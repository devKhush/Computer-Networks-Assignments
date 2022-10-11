#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <poll.h>
#include <sys/epoll.h>

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

    // E-Poll structure to initiate server client structure
    struct poll_fd *poll_fd;
    struct epoll_event epoll_event;
    struct epoll_event *epoll_events;
    epoll_events = (struct epoll_event *)calloc(NUM_CLIENTS, sizeof(epoll_event));

    // Initialize the connections
    int efd = epoll_create1(0);
    epoll_event.data.fd = server_socket_fd;
    epoll_event.events = EPOLLIN;
    int clients_used = 0;
    if (epoll_ctl(efd, EPOLL_CTL_ADD, server_socket_fd, &epoll_event) == -1)
    {
        printf("Error in epoll_ctl() \n");
        exit(0);
    }

    // Initialize empty connectiosn
    for (int i = 0; i < NUM_CLIENTS; i++)
    {
        epoll_events[i].data.fd = 0;
        epoll_events[i].events = EPOLLIN;
    }

    // Compute factorial for server-client request
    long long int factorial[21];
    factorial[0] = 1;
    for (int i = 1; i <= 20; i++)
        factorial[i] = factorial[i - 1] * i;

    // Opening a file
    FILE *file_ptr = fopen("client_requests_epoll.txt", "w");

    // Making all the connections
    while (true)
    {
        // Call epoll() method and wait for client connection
        int epoll_status = epoll_wait(efd, epoll_events, NUM_CLIENTS, -1);
        if (epoll_status <= 0)
            break;

        for (int i = 0; i < epoll_status; i++)
        {
            if ((epoll_events[i].events & EPOLLIN) == EPOLLIN)
            {
                if (epoll_events[i].data.fd == server_socket_fd)
                {
                    int client_addr_len = sizeof(client_addr);
                    int client_socket_fd = accept(server_socket_fd, (struct sockaddr *)&client_addr, &client_addr_len);
                    if (client_socket_fd < 0)
                    {
                        printf("Error in accept() \n");
                        continue;
                    }
                    epoll_event.events = EPOLLIN;
                    epoll_event.data.fd = client_socket_fd;

                    printf("***** New Client connected successfully with fd %d *****\n", client_socket_fd);
                    // Writing into the file_ptr
                    char empty[200];
                    sprintf(empty, "********** New Client connected successfully with IP-address='%s', FD='%d' and Port-no.='%u' **********\n\n", inet_ntoa(client_addr.sin_addr), client_socket_fd, ntohs(client_addr.sin_port));
                    fputs(empty, file_ptr);
                    fflush(file_ptr);

                    if (epoll_ctl(efd, EPOLL_CTL_ADD, client_socket_fd, &epoll_event) == -1)
                    {
                        printf("Error in epoll_ctl() \n");
                        continue;
                    }
                }

                else
                {
                    // Message received from the client
                    int message_from_client;
                    int client_message_read_status = recv(epoll_events[i].data.fd, &message_from_client, sizeof(message_from_client), 0);
                    if (client_message_read_status <= 0)
                    {
                        epoll_events[i].data.fd = 0;
                        epoll_events[i].events = 0;
                        clients_used--;
                        if (client_message_read_status == 0)
                            break;
                    }
                    else
                    {
                        printf("Client replied: %d\n", message_from_client);

                        // Server's message to the client
                        long long int message_from_server = factorial[message_from_client];
                        send(epoll_events[i].data.fd, &message_from_server, sizeof(message_from_server), 0);

                        // Writing into the file
                        char request[200];
                        sprintf(request, "Client with IP-address='%s' and Port-no.='%u' \nClient request = %d \nServer response = %lli \n\n",
                                inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), message_from_client, message_from_server);
                        fputs(request, file_ptr);
                        fflush(file_ptr);
                    }
                }
            }
        }
    }

    for (int i = 0; i < NUM_CLIENTS; i++)
        close(epoll_events[i].data.fd);

    free(epoll_events);

    // Close server socket
    close(server_socket_fd);
    return 0;
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
        printf("Server Bind failed. Port already in use.\n");
        return -1;
    }

    // Server listening...
    int listen_status = listen(server_socket_fd, 15);
    if (listen_status < 0)
    {
        printf("Server listening failed.\n");
        return -1;
    }
    printf("Listening for connections on Port %d\n\n", PORT);

    return server_socket_fd;
}