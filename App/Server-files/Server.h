#ifndef Server_h
#define Server_h 

#include <sys/socket.h>
#include <stdint.h>
#include <netinet/in.h>

struct Server {
    int domain;
    int service;
    int protocol;
    uint32_t interface; 
    int port;
    int backlog;

    struct sockaddr_in address;

    int socket;

    void (*launch)(struct Server*);
};

struct Server server_constructor(int domain, int service, int protocol, uint32_t interface, 
    int port, int backlog);


#endif /* Server_h */