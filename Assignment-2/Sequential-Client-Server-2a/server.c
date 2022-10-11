#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#define PORT 8080

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
    struct sockaddr_in server_addr, client_addr;
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
    int listen_status = listen(server_socket_fd, 5);
    if (listen_status < 0)
    {
        printf("Server listening failed.\n");
        exit(0);
    }
    printf("Listening for connections on Port %d\n\n", PORT);

    // Compute factorial for server-client request
    long long int factorial[21];
    factorial[0] = 1;
    for (int i = 1; i <= 20; i++)
        factorial[i] = factorial[i - 1] * i;

    // Opening a file
    FILE *file_ptr = fopen("client_requests.txt", "w");

    while (true)
    {
        // Connect the Server socket with the client
        int client_addr_len = sizeof(client_addr);
        int client_socket_fd = accept(server_socket_fd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket_fd < 0)
        {
            printf("Client accept failed \n");
            continue;
        }

        // Client IP and port information
        printf("Connection accepted from client with IP-address and Port '%s:%u'\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // Writing into the file_ptr
        char empty[200];
        sprintf(empty, "***** New Client connected successfully with IP-address='%s' and Port-no.='%u' *****\n\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        fprintf(file_ptr, empty, NULL);
        fflush(file_ptr);

        // Read and write from and to the client
        for (int i = 1; i <= 20; i++)
        {
            // Message received from the client
            int message_from_client;
            recv(client_socket_fd, &message_from_client, sizeof(message_from_client), 0);
            if (message_from_client < 0)
            {
                printf("Reading from Client socket failed \n");
                continue;
            }
            printf("Client replied: %d\n", message_from_client);

            // Server's message to the client
            long long int message_from_server = factorial[message_from_client];
            send(client_socket_fd, &message_from_server, sizeof(message_from_server), 0);

            // Writing into the file
            char request[200];
            sprintf(request, "Client with IP-address='%s' and Port-no.='%u' \nClient request = %d \nServer response = %lli \n\n",
                    inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), message_from_client, message_from_server);
            fprintf(file_ptr, request, NULL);
            fflush(file_ptr);
        }
        printf("\n");
    }

    // Closing the file
    fclose(file_ptr);

    // Close the socket
    close(server_socket_fd);
    return 0;
}