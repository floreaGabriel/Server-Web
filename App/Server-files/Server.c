#include "Server.h"

#include <stdio.h>
#include <stdlib.h>
#include "HTTPRequest.h"

#define MAX_LENGTH 30000

void launch(struct Server *server) 
{
    // obtin lungimea adresei pentru a putea face accept la conexiuni
    int address_length = sizeof(server->address);
    int new_socket;
    char buffer[MAX_LENGTH];
    ssize_t bytes_read;
    while(1){
        printf("~~~~~~~~~~ WAITING FOR CONNECTION ~~~~~~~~~~~\n\n");
        new_socket = accept(server->socket, (struct sockaddr *)&server->address,(socklen_t *)&address_length);
        printf("Client accepted!\n");
        bytes_read = read(new_socket, buffer, sizeof(buffer));
        struct HTTPRequest request = request_constructor(buffer);

        
        
    }
}

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

    server.launch = launch;
    
    return server;

}

