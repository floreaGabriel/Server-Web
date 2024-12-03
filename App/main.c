#include "libmini_server.h" // librarie principala

#include<stdio.h>
#include "Server-files/Server.h"
#include <signal.h>

int main(int argc, char* argv[])
{
    signal(SIGPIPE, SIG_IGN);

    struct Server httpserver = server_constructor(AF_INET, SOCK_STREAM, 0, INADDR_ANY, 8080, 255);

    // char path[500];
    // ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    // if (len != -1) {
    //     path[len] = '\0'; // Asigurăm terminatorul de string
    //     printf("Calea completă către executabil: %s\n", path);
    // } else {
    //     perror("readlink");
    // }



    printf("Serverul este pornit și ascultă pe portul 8081\n");
    launch(&httpserver);

    server_destructor(&httpserver);
    return 0;
}
