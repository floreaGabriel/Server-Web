#include "Server.h"

#include <stdio.h>
#include <stdlib.h>
#include "HTTPRequest.h"
#include <pthread.h>

#define MAX_LENGTH 30000

struct Server server_constructor(int domain, int  service, int protocol, uint32_t interface,
    int port, int backlog) 
{

    struct Server server;

    server.domain = domain;
    server.service = service;
    server.protocol = protocol;
    server.port = port;
    server.backlog = backlog;


    server.address.sin_family = domain;
    server.address.sin_port = htons(port);
    server.address.sin_addr.s_addr = htonl(interface);

    server.socket = socket(domain, service, protocol);

    if (server.socket == 0)
    {
        perror("Failed to connect socket...\n");
        exit(-1);
    }

    int opt = 1;
    if (setsockopt(server.socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    if (bind(server.socket, (struct sockaddr*)&server.address, sizeof(server.address)) < 0)
    {
        perror("Failed to bind ... \n");
        exit(-1);
    }

    if (listen(server.socket, server.backlog) < 0)
    {
        perror("Failed to listen ... \n");
        exit(-1);
    }

    
    return server;

}

void server_destructor(struct Server *server) {
    printf("Closing server socket %d ... \n\n", server->socket);
    close(server->socket);
}
