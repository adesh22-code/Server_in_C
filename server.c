#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 4096

char *read_file_to_buffer(const char *filename, long *out_size);
void setup_server(int server_socket, struct sockaddr_in *server_addr);
void handle_client(int client_socket, const char *html_file);

int main()
{
    int server_socket;
    int client_socket;
    struct sockaddr_in server_addr;

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        perror("Socket creation failed");
        return 1;
    }

    printf("Socket created successfully!\n");

    // Allow address reuse
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR,
                   &opt, sizeof(opt)) == -1)
    {
        perror("setsockopt failed");
        close(server_socket);
        return 1;
    }

    // Setup server
    setup_server(server_socket, &server_addr);

    while (1)
    {
        printf("\nWaiting for client...\n");

        client_socket = accept(server_socket, NULL, NULL);

        if (client_socket == -1)
        {
            perror("Accept failed");
            continue;
        }

        printf("Client connected!\n");

        // Serve index.html
        handle_client(client_socket, "about.html");

        close(client_socket);
    }

    close(server_socket);
    return 0;
}

void setup_server(int server_socket, struct sockaddr_in *server_addr)
{
    memset(server_addr, 0, sizeof(struct sockaddr_in));

    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(PORT);
    server_addr->sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket,
             (struct sockaddr *)server_addr,
             sizeof(struct sockaddr_in)) == -1)
    {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Bind successful!\n");

    if (listen(server_socket, 5) == -1)
    {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Listening on port %d...\n", PORT);
}

void handle_client(int client_socket, const char *html_file)
{
    char buffer[BUFFER_SIZE];

    memset(buffer, 0, BUFFER_SIZE);

    int bytes_received = recv(client_socket,
                              buffer,
                              BUFFER_SIZE - 1,
                              0);

    if (bytes_received <= 0)
    {
        return;
    }

    buffer[bytes_received] = '\0';

    printf("\n===== HTTP REQUEST =====\n");
    printf("%s\n", buffer);

    long file_size;
    char *html = read_file_to_buffer(html_file, &file_size);

    if (html == NULL)
    {
        const char *response =
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 13\r\n"
            "Connection: close\r\n"
            "\r\n"
            "404 Not Found";

        send(client_socket, response, strlen(response), 0);
        return;
    }

    char header[512];

    sprintf(header,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: %ld\r\n"
            "Connection: close\r\n"
            "\r\n",
            file_size);

    send(client_socket, header, strlen(header), 0);
    send(client_socket, html, file_size, 0);

    free(html);
}

char *read_file_to_buffer(const char *filename, long *out_size)
{
    FILE *file = fopen(filename, "rb");

    if (file == NULL)
    {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    char *content = malloc(size + 1);

    if (content == NULL)
    {
        fclose(file);
        return NULL;
    }

    fread(content, 1, size, file);
    content[size] = '\0';

    *out_size = size;

    fclose(file);

    return content;
}
