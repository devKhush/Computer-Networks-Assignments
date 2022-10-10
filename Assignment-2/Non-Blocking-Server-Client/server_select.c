#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <inttypes.h>

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

    // Initialize all the client connections
    int all_client_connections[NUM_CLIENTS];
    for (int i = 0; i < NUM_CLIENTS; i++)
    {
        all_client_connections[i] = -1;
    }
    all_client_connections[0] = server_socket_fd;

    // Client socket address
    struct sockaddr_in client_addr;

    // Compute factorial for server-client request
    long long int factorial[21];
    factorial[0] = 1;
    for (int i = 1; i <= 20; i++)
        factorial[i] = factorial[i - 1] * i;

    // Opening a file
    FILE *file_ptr = fopen("client_requests_select.txt", "w");

    // Open client connections
    fd_set server_read_fd_set;

    // Connect Client to Server
    while (true)
    {
        FD_ZERO(&server_read_fd_set);

        // Set the fd_set before passing it to the select call
        for (int i = 0; i < NUM_CLIENTS; i++)
        {
            if (all_client_connections[i] >= 0)
                FD_SET(all_client_connections[i], &server_read_fd_set);
        }
        /* Invoke select() and then wait! */
        int select_return_value = select(FD_SETSIZE, &server_read_fd_set, NULL, NULL, NULL);

        // Select woke up. Identify the fd_set that has events   I
        if (select_return_value >= 0)
        {
            // printf("==> Select() returned with %d\n", select_return_value);

            // Check if fd with event is the server fd
            if (FD_ISSET(server_socket_fd, &server_read_fd_set))
            {
                int client_addr_len = sizeof(client_addr);
                int client_socket_fd = accept(server_socket_fd, (struct sockaddr *)&client_addr, &client_addr_len);
                if (client_socket_fd >= 0)
                {
                    printf("***** Accepted a new Connection with fd %d *****\n", client_socket_fd);
                    for (int i = 0; i < NUM_CLIENTS; i++)
                    {
                        if (all_client_connections[i] < 0)
                        {
                            all_client_connections[i] = client_socket_fd;
                            break;
                        }
                    }
                }
                else
                {
                    printf("Client connection Accept failed\n");
                }
                select_return_value--;
                if (!select_return_value)
                    continue;
            }

            // Check if fd with event is the non-server fd
            for (int i = 0; i < NUM_CLIENTS; i++)
            {
                int return_val;
                if ((all_client_connections[i] > 0) && FD_ISSET(all_client_connections[i], &server_read_fd_set))
                {
                    // Message received from the client
                    int message_from_client;
                    return_val = recv(all_client_connections[i], &message_from_client, sizeof(message_from_client), 0);
                    if (return_val < 0)
                    {
                        printf("Reading from Client socket failed \n");
                        break;
                    }
                    if (return_val == 0)
                    {
                        printf("*******Closing connection for client FD: %d ******* \n", all_client_connections[i]);
                        close(all_client_connections[i]);
                        all_client_connections[i] = -1;
                    }
                    if (return_val > 0)
                    {
                        printf("\nClient replied: %d\n", message_from_client);

                        // Server's message to the client
                        long long int message_from_server = factorial[message_from_client];
                        send(all_client_connections[i], &message_from_server, sizeof(message_from_server), 0);

                        // Writing into the file
                        char request[200];
                        sprintf(request, "Client with IP-address='%s' and Port-no.='%u' \nClient request = %d \nServer response = %lli \n\n",
                                inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), message_from_client, message_from_server);
                        fputs(request, file_ptr);
                    }
                }
                return_val--;
                if (!return_val)
                    continue;
            }
        }
    }

    // Close all the sockets
    for (int i = 0; i < NUM_CLIENTS; i++)
    {
        if (all_client_connections[i] > 0)
            close(all_client_connections[i]);
    }

    // Close server socket
    close(server_socket_fd);

    // Close the file
    fclose(file_ptr);

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