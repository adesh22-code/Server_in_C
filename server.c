#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 4096

int main()
{
    int server_socket;
    int client_socket;
    char buffer[BUFFER_SIZE];

    struct sockaddr_in server_addr;

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        perror("Socket creation failed");
        return 1;
    }
    printf("Socket created successfully!\n");

    // Reuse port to prevent "Address already in use" bugs on frequent restarts
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Initialize structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Bind failed");
        close(server_socket);
        return 1;
    }
    printf("Bind successful!\n");

    // Listen
    if (listen(server_socket, 5) == -1)
    {
        perror("Listen failed");
        close(server_socket);
        return 1;
    }
    printf("Listening on port %d...\n", PORT);

    // Infinite Loop so the server doesn't exit after serving one request
    while (1)
    {
        printf("\nWaiting for a client...\n");

        client_socket = accept(server_socket, NULL, NULL);
        if (client_socket == -1)
        {
            perror("Accept failed");
            continue; 
        }
        printf("Client connected!\n");

        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);

        if (bytes_received == -1)
        {
            perror("recv failed");
            close(client_socket);
            continue;
        }

        buffer[bytes_received] = '\0';
        printf("Received HTTP Request.\n");

        // --- STEP 1: OPEN FILE FROM DISK ---
        FILE *html_file = fopen("index.html", "r");
        
        if (html_file == NULL)
        {
            // Fallback error response if index.html is missing
            char error_response[] =
                "HTTP/1.1 404 Not Found\r\n"
                "Content-Type: text/plain\r\n"
                "Content-Length: 26\r\n"
                "Connection: close\r\n\r\n"
                "Error 404: File Not Found";
            send(client_socket, error_response, strlen(error_response), 0);
            printf("File index.html missing! Sent 404.\n");
        }
        else
        {
            // --- STEP 2: MEASURE FILE CARGO SIZE ---
            fseek(html_file, 0, SEEK_END);      // Move cursor to the end
            long file_size = ftell(html_file);   // Find position byte offset
            rewind(html_file);                  // Move cursor back to beginning

            // Allocate dynamic memory block matching file size + null terminator
            char *file_content = malloc(file_size + 1);
            
            // Read all characters from disk straight into our layout variable
            fread(file_content, 1, file_size, html_file);
            file_content[file_size] = '\0'; // Seal the string string safely
            fclose(html_file);              // Close disk stream cleanly

            // --- STEP 3: ASSEMBLE PROTOCOL HEADERS ---
            char response_header[512];
            sprintf(response_header,
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: text/html\r\n"
                    "Content-Length: %ld\r\n"
                    "Connection: close\r\n"
                    "\r\n", 
                    file_size);

            // Transmit HTTP wrapper headers first
            send(client_socket, response_header, strlen(response_header), 0);
            
            // Transmit HTML webpage cargo payload immediately after
            send(client_socket, file_content, file_size, 0);
            
            printf("Served index.html file successfully (%ld bytes)!\n", file_size);

            // Free dynamic memory allocation block
            free(file_content);
        }

        // Close transaction with browser
        close(client_socket);
    }

    close(server_socket);
    return 0;
}
