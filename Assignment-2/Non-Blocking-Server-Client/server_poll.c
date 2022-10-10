#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <poll.h>

#define NUM_CLIENTS 10
#define PORT 8080

int initialize_TCP_server_socket();

int main()
{
    // Initialize the server socket
    int server_socket_fd = initialize_TCP_server_socket();
    if (server_socket_fd <= 0)
    {
        printf("Server socket couldn't be created \n");
        exit(0);
    }

    // Client socket address
    struct sockaddr_in client_addr;

    // Poll structure to initiate server client structure
    struct pollfd *pdfs;
    struct pollfd poll_fds[NUM_CLIENTS + 1];

    // Initialize the connections
    poll_fds[0].fd = server_socket_fd;
    poll_fds[0].events = POLLIN;
    int clients_used = 0;
    for (int i = 1; i < NUM_CLIENTS; i++)
    {
        poll_fds[i].fd = 0;
        poll_fds[i].events = POLLIN;
    }

    // Compute factorial for server-client request
    long long int factorial[21];
    factorial[0] = 1;
    for (int i = 1; i <= 20; i++)
        factorial[i] = factorial[i - 1] * i;

    // Opening a file
    FILE *file_ptr = fopen("client_requests_poll.txt", "w");

    while (true)
    {
        // Call poll() method and wait for client connection
        int poll_status = poll(poll_fds, clients_used + 1, 5000);

        // poll() call wake up. Find the cllient FD with available requests
        if (poll_status >= 0)
        {
            // Check if the FD with the events is the server FD
            if (poll_fds[0].revents & POLLIN)
            {
                // Accept the new client connection
                int client_addr_len = sizeof(client_addr);
                int client_socket_fd = accept(server_socket_fd, (struct sockaddr *)&client_addr, &client_addr_len);
                if (client_socket_fd >= 0)
                {
                    printf("\n***** Accepted a new Connection with client fd %d ******* \n\n", client_socket_fd);
                    for (int i = 1; i < NUM_CLIENTS; i++)
                    {
                        if (poll_fds[i].fd == 0)
                        {
                            poll_fds[i].fd = client_socket_fd;
                            poll_fds[i].events = POLLIN;
                            clients_used++;
                            break;
                        }
                    }
                }
                else
                    printf("Client connection Accept failed\n");

                poll_status--;
                if (!poll_status)
                    continue;
            }

            int client_message_read_status;
            // Check if the FD with the events is the non-server (client) FD
            for (int i = 1; i < NUM_CLIENTS; i++)
            {
                if (poll_fds[i].fd > 0 && poll_fds[i].revents & POLLIN)
                {
                    // Message received from the client
                    int message_from_client;
                    client_message_read_status = recv(poll_fds[i].fd, &message_from_client, sizeof(message_from_client), 0);
                    if (client_message_read_status <= 0)
                    {
                        poll_fds[i].fd = 0;
                        poll_fds[i].events = 0;
                        poll_fds[i].revents = 0;
                        clients_used--;
                    }
                    else
                    {
                        printf("Client replied: %d\n", message_from_client);

                        // Server's message to the client
                        long long int message_from_server = factorial[message_from_client];
                        send(poll_fds[i].fd, &message_from_server, sizeof(message_from_server), 0);

                        // Writing into the file
                        char request[200];
                        sprintf(request, "Client with IP-address='%s' and Port-no.='%u' \nClient request = %d \nServer response = %lli \n\n",
                                inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), message_from_client, message_from_server);
                        fputs(request, file_ptr);
                    }
                }
            }
        }
    }

    // Close server socket
    close(server_socket_fd);

    // Close file
    fclose(file_ptr);
}

int initialize_TCP_server_socket()
{
    // Server socket initialization
    int server_socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket_fd < 0)
    {
        printf("Server socket creation failed. \n");
        return -1;
    }
    printf("Server socket created.\n");

    // Server socket address
    struct sockaddr_in server_addr, client_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the server
    int bind_status = bind(server_socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (bind_status < 0)
    {
        printf("Server Bind failed. \n");
        return -1;
    }

    // Server listening...
    int listen_status = listen(server_socket_fd, 15);
    if (listen_status < 0)
    {
        printf("Server listening failed.\n");
        return -1;
    }
    return server_socket_fd;
}