#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>

#define PORT 8080

int main()
{
    // Client socket initialization
    int socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_fd < 0)
    {
        printf("Client socket creation failed. \n");
        exit(0);
    }
    printf("Client socket created.\n");

    // Server socket address
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    // TODO
    server_addr.sin_port = htons(PORT);

    // Connect with the server
    int connect_status = connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (connect_status < 0)
    {
        printf("Connection with Server failed. \n");
        exit(0);
    }
    printf("Client connected to Server.\n");

    // "Write and Read" to and from the server
    for (int i = 1; i <= 20; i++)
    {
        int client_message = i;
        send(socket_fd, &client_message, sizeof(client_message), 0);

        long long int server_message;
        recv(socket_fd, &server_message, sizeof(server_message), 0);
        printf("Server replied: Factorial of %d = %lli \n", i, server_message);
    }

    // Close the socket
    close(socket_fd);

    return 0;
}