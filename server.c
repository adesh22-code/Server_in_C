#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 4096

char* read_file_to_buffer(const char *filename, long *out_size);

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

        
        
        close(client_socket);
    }

    close(server_socket);
    return 0;
}


char* read_file_to_buffer(const char *filename, long *out_size) {
    // --- STEP 1: OPEN FILE FROM DISK ---
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        return NULL;
    }

    // Measure size
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    // Allocate memory on Heap
    char *content = malloc(size + 1);
    if (content != NULL) {
        fread(content, 1, size, file);
        content[size] = '\0';
        *out_size = size; // Pass the size back via pointer reference
    }

    fclose(file);
    return content;
    // Close transaction with browser
}
