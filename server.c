#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

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

    // Initialize structure
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind
    if (bind(server_socket,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) == -1)
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

    printf("Listening on port 8080...\n");
    printf("Waiting for a client...\n");

    // Accept
    client_socket = accept(server_socket, NULL, NULL);

    if (client_socket == -1)
    {
        perror("Accept failed");
        close(server_socket);
        return 1;
    }

    printf("Client connected!\n");

    close(client_socket);
    close(server_socket);

    return 0;
}
