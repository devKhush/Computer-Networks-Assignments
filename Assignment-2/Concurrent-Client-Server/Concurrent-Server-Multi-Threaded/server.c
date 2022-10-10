#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/socket.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#define PORT 8080

struct sockaddr_in server_addr, client_addr;
long long int factorial[21];
FILE *file_ptr;

void *server_client_connection(void *client_socket);

int main()
{
    // Server socket initialization
    int server_socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket_fd < 0)
    {
        printf("Server socket creation failed. \n");
        exit(0);
    }
    printf("Server socket created.\n");

    // Server socket address
    // struct sockaddr_in server_addr, client_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the server
    int bind_status = bind(server_socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (bind_status < 0)
    {
        printf("Server Bind failed. \n");
        exit(0);
    }

    // Server listening...
    int listen_status = listen(server_socket_fd, 15);
    if (listen_status < 0)
    {
        printf("Server listening failed.\n");
        exit(0);
    }

    // Compute factorial for server-client request
    factorial[0] = 1;
    for (int i = 1; i <= 20; i++)
        factorial[i] = factorial[i - 1] * i;

    // Opening a file
    file_ptr = fopen("client_requests.txt", "w");

    // Connecting to client
    while (true)
    {
        // Connection with clients
        int client_addr_len = sizeof(client_addr);
        int client_socket_fd = client_socket_fd = accept(server_socket_fd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket_fd < 0)
        {
            printf("Client accept failed \n");
            continue;
        }
        printf("***** Connection accepted from %s:%u *****\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // Pointer to the client socket fd
        int *client_socket_fd_ptr = &client_socket_fd;

        // Creating a thread for Server
        pthread_t sniffer_thread;
        int thread_status = pthread_create(&sniffer_thread, NULL, server_client_connection, (void *)client_socket_fd_ptr);
        if (thread_status < 0)
        {
            printf("Thread creation for Server failed.\n");
            continue;
        }
    }

    // Close the Threads
    pthread_exit(NULL);

    // Closing the file
    fclose(file_ptr);

    // Close the socket
    close(server_socket_fd);

    return 0;
}

void *server_client_connection(void *client_socket)
{
    int client_socket_fd = *(int *)client_socket;

    // Read and write from and to the client
    for (int i = 1; i <= 20; i++)
    {
        // Message received from the client
        int message_from_client;
        recv(client_socket_fd, &message_from_client, sizeof(message_from_client), 0);
        if (message_from_client < 0)
        {
            printf("Reading from Client socket failed \n");
            exit(0);
        }
        printf("Client replied: %d\n", message_from_client);

        // Server's message to the client
        long long int message_from_server = factorial[message_from_client];
        send(client_socket_fd, &message_from_server, sizeof(message_from_server), 0);

        // Writing into the file
        char request[200];
        sprintf(request, "Client with IP-address='%s' and Port-no.='%u' \nClient request = %d \nServer response = %lli \n\n",
                inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), message_from_client, message_from_server);
        fputs(request, file_ptr);
    }
}