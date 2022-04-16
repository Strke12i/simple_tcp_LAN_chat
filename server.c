#include "config.h"
#include <stdio.h>

// This function needs to received a port to start listen
SOCKET create_socket(const char *port)
{
    printf("Configuring local address..\n");
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET; // works only on the ipv4 
    hints.ai_socktype = SOCK_STREAM;// TCP protocol
    hints.ai_flags = AI_PASSIVE; // Wild card ip address

    struct addrinfo *bind_address;
    getaddrinfo(0, port, &hints, &bind_address);

    printf("initializing socket...\n");
    // Create the socket with the bind attributes
    SOCKET socket_listen = socket(bind_address->ai_family,
                                  bind_address->ai_socktype, bind_address->ai_protocol);

    if (!ISVALIDSOCKET(socket_listen))
    {
        fprintf(stderr, "Error on create socket..\n");
        exit(1);
    }
    if (bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen))
    {
        fprintf(stderr, "Error on binding socket..\n");
        exit(1);
    }

    freeaddrinfo(bind_address);

    if (listen(socket_listen, 10) < 0)
    {
        fprintf(stderr, "Error on listening socket..\n");
        exit(1);
    }

    return socket_listen;
}

int main(int argc, char const *argv[])
{
// Check if the enviroment is set to windows to start the socket
#if defined(_WIN32)
    WSADATA d;
    if (WSAStartup(MAKEWORD(2, 2), &d))
    {
        fprintf(stderr, "Error on startup..\n");
        return 1;
    }
#endif

    // The main program requires a port as the second argument
    if(argc != 2)
    {
        fprintf(stderr, "Invalid number of arguments\n");
        return 1;   
    }

    SOCKET socket_listen = create_socket(argv[1]);

    fd_set master;
    FD_ZERO(&master);
    FD_SET(socket_listen, &master);
    SOCKET max_sockets = socket_listen;

    // implements the logic around the multiple sockets connections
    while (1)
    {
        fd_set reads = master;
        if (select(max_sockets + 1, &reads, 0, 0, 0) < 0)
        {
            fprintf(stderr, "Error on select..\n");
            exit(1);
        }

        for (SOCKET i = 1; i < max_sockets + 1; i++)
        {
            if (FD_ISSET(i, &reads))
            {
                if (i == socket_listen)
                {
                    struct sockaddr_storage client_address;
                    socklen_t client_len = sizeof(client_address);
                    SOCKET socket_client = accept(i, (struct sockaddr *)&client_address, &client_len);
                    if (!ISVALIDSOCKET(socket_client))
                    {
                        fprintf(stderr, "Error on accept socket..\n");
                        return 1;
                    }

                    FD_SET(socket_client, &master);
                    if (socket_client > socket_listen)
                    {
                        max_sockets = socket_client;
                    }

                    // the nameinfo function print the new connection established
                    char address_buffer[100];
                    char service_buffer[100];
                    getnameinfo((struct sockaddr *)&client_address, client_len,
                                address_buffer, sizeof(address_buffer),
                                service_buffer, sizeof(service_buffer), NI_NUMERICHOST);

                    printf("New connection from: %s:%s\n", address_buffer, service_buffer);
                }
                else 
                {
                    char buffer[4096];
                    int bytes_received = recv(i,buffer,4096,0);
                    if(bytes_received<1)
                    {
                        FD_CLR(i,&master);
                        CLOSESOCKET(i);
                        continue;
                    }

                    for (SOCKET j = 0; j < max_sockets+1; j++)
                    {
                        if(FD_ISSET(j,&master))
                        {
                            if(socket_listen == j || j == i) continue;
                            else send(j,buffer,bytes_received,0);
                        }
                    }
                    
                }
            }
        }
    }
    printf("Closing socket..\n");
    CLOSESOCKET(socket_listen);

#if defined(_WIN32)
    WSACleanup();
#endif
    return 0;
}