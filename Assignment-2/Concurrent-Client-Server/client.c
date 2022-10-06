#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#define PORT 8080
#define NUM_CLIENT 3

int main()
{
    pthread_t sniffer_thread;
    for (int client_num = 1; client_num <= NUM_CLIENT; client_num++)
    {
        if (pthread_create(&sniffer_thread, NULL, client_connection, (void *)client_num) < 0)
        {
            printf("Thread creation for CLient-%d failed.\n", client_num);
            return 0;
        }
        // sleep(3);
    }
    pthread_exit(NULL);
    return 0;
}

int client_connection(int client_num)
{
    // Client socket initialization
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0)
    {
        printf("Client-%d socket creation failed. \n", client_num);
        exit(0);
    }
    printf("Client-%d socket created.\n", client_num);

    // Server socket address
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Connect with the server
    int connect_status = connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (connect_status < 0)
    {
        printf("Client-%d connection with Server failed.\n", client_num);
        exit(0);
    }
    printf("Client-%d connected to Server.\n", client_num);

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