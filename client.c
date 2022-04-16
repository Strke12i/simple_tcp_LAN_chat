#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "config.h"

#if defined(_WIN32)
#include <conio.h>
#endif


SOCKET create_socket(const char *host,const char *port)
{
    printf("Configuring local address..\n");
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo *peer_address;
    getaddrinfo(host, port, &hints, &peer_address);

    char address_buffer[100];
    char service_buffer[100];

    getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen,
                address_buffer, sizeof(address_buffer),
                service_buffer, sizeof(service_buffer), NI_NUMERICHOST);
    printf("Remote address is: %s:%s\n", address_buffer, service_buffer);

    SOCKET socket_peer = socket(peer_address->ai_family, peer_address->ai_socktype, peer_address->ai_protocol);
    if (!ISVALIDSOCKET(socket_peer))
    {
        fprintf(stderr, "error on create socket.");
        exit(1);
    }
    printf("Connecting..\n");
    if (connect(socket_peer, peer_address->ai_addr, peer_address->ai_addrlen))
    {
        fprintf(stderr, "error on connect.");
        exit(1);
    }
    freeaddrinfo(peer_address);
    printf("Connected!\n");

    return socket_peer;
}

int main(int argc, char const *argv[])
{
#if defined(_WIN32)
    WSADATA d;
    if (WSAStartup(MAKEWORD(2, 2), &d))
    {
        fprintf(stderr, "error on startup.");
        return 1;
    }
#endif

    if (argc != 4)
    {
        fprintf(stderr, "Invalid quantity of parameters.");
        return 1;
    }
    const char *username = argv[3];
    if(strlen(username) > 15)
    {
        fprintf(stderr, "Username too long. Only 15 symbols");
        return 1;
    }

    SOCKET socket_peer = create_socket(argv[1], argv[2]);
    printf("Send data:\n");
    while (1)
    {
        fd_set reads;
        FD_ZERO(&reads);
        FD_SET(socket_peer, &reads);
#if !defined(_WIN32)
        FD_SET(0, &reads);
#endif

        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;

        if (select(socket_peer + 1, &reads, 0, 0, &timeout) < 0)
        {
            fprintf(stderr, "error on select.");
            return 1;
        }

        if (FD_ISSET(socket_peer, &reads))
        {
            char read[4096];
            int bytes_received = recv(socket_peer, read, 4096, 0);
            if (bytes_received < 1)
            {
                printf("Connection closed by peer\n");
                break;
            }
            printf("--%.*s", bytes_received, read);
        }
#if defined(_WIN32)
        if (_kbhit())
        {
#else
        if (FD_ISSET(0, &reads))
        {
#endif

            char read[4096];
            if (!fgets(read, 4080, stdin))break;
            char buf[4096];
            strcpy(buf,username);
            strcat(buf,":");
            strcat(buf,read);
           
            int bytes_send = send(socket_peer, buf, strlen(buf), 0);
            
        }
    }
    printf("closing socket..\n");
#if defined(_WIN32)
    WSACleanup();
#endif


    
    return 0;
}
