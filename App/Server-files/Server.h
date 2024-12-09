#ifndef Server_h
#define Server_h 

#include <sys/socket.h>
#include <stdint.h>
#include <netinet/in.h>
#include <unistd.h>
#include "ThreadPool.h"


struct Server {
    int domain;
    int service;
    int protocol;
    uint32_t interface; 
    int port; 
    int backlog;

    struct sockaddr_in address;

    int socket;
};


struct ClientServer {
    int client;
};

struct Server server_constructor(int domain, int service, int protocol, uint32_t interface, 
    int port, int backlog);

void launch(struct Server*, int);

void server_destructor(struct Server *server);
#endif /* Server_h */






