#include "libmini_server.h" // librarie principala

#include <stdio.h>
#include "Server-files/ServerConfig.h"

#include <signal.h>
#include <stdlib.h>


int main(int argc, char* argv[])
{
    //signal(SIGPIPE, SIG_IGN);
    ServerConfig config;
    
    // Încarcă configurarea din fișierul JSON
    load_config("config.json", &config);

    printf("~~~~~~~~~~~~~~~~~~~~\n");
    printf("Setările serverului:\n");
    printf("Domain: %d\n", config.domain);
    printf("Service: %d\n", config.service);
    printf("Protocol: %d\n", config.protocol);
    printf("Interface: %d\n", config.interface);
    printf("Port: %d\n", config.port);
    printf("Backlog: %d\n", config.backlog);
    printf("ThreadPool Size: %d\n", config.thread_pool_size);
    printf("~~~~~~~~~~~~~~~~~~~~\n\n");

    // Inițializează serverul cu setările din configurare
    struct Server httpserver = server_constructor(config.domain, config.service, config.protocol ,config.interface, config.port, config.backlog);
    

    printf("Serverul este pornit și ascultă pe portul %d\n", httpserver.port);
    launch(&httpserver, config.thread_pool_size);

    server_destructor(&httpserver);
    return 0;
}
