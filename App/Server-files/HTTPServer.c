#include "HTTPServer.h"

#define MAX_LENGTH 30000

#include <stdarg.h>

void register_routes(struct HTTPServer *server, 
        void (*route_function)(struct HTTPServer *server, struct HTTPRequest *request),
        char *uri, int num_methods, ...);   

void * handler(void *arg);

struct HTTPServer http_server_constructor() {

    struct HTTPServer server;

    server.server = server_constructor(AF_INET, SOCK_STREAM, 0, INADDR_ANY, 8080, 255);
    server.register_routes = register_routes;
    server.routes = dictionary_constructor(compare_string_keys);
    server.launch = launch;

    return server;
}

struct ClientServer {
    int client;
    struct HTTPServer *server;
};

struct Route {
    int methods[9];
    char *uri;
    char *(*route_function)(struct HTTPServer *server, struct HTTPRequest *request);
};

void register_routes(struct HTTPServer *server, 
        void (*route_function)(struct HTTPServer *server, struct HTTPRequest *request),
        char *uri, int num_methods, ...)
{
    struct Route route;
    va_list methods;
    va_start(methods,num_methods);
    // iteram prin lista de argumente ...
    for (int i = 0; i < num_methods; i++)
    {
        route.methods[i] = va_arg(methods, int);
    }

    // inregistram uri rutei
    char buffer[strlen(uri)];
    route.uri = buffer;
    strcpy(route.uri, uri);

    // functia rutei + salvam ruta in server
    route.route_function = route_function;
    server->routes.insert(&server->routes, uri, sizeof(char[strlen(uri)]), &route, sizeof(route));

} 



void launch(struct HTTPServer *server)
{

    // obtin lungimea adresei pentru a putea face accept la conexiuni
    int address_length = sizeof(server->server.address);

    while(1){
        printf("~~~~~~~~~~ WAITING FOR CONNECTION ~~~~~~~~~~~\n\n");
        
        struct ClientServer *client = malloc(sizeof(struct ClientServer));
        client->client = accept(server->server.socket, (struct sockaddr *)&server->server.address,(socklen_t *)&address_length);
        client->server = server;
        printf("Client accepted!\n");
        
    }
}


void * handler(void *arg)
{
    struct ClientServer *client = (struct ClientServer*)arg;
    
    char request_string[MAX_LENGTH];

    ssize_t bytes_read = read(client->client, request_string, sizeof(request_string));

    struct HTTPRequest request = request_constructor(request_string);
    char *uri = request.request_line.search(&request, "uri", sizeof("uri"));

    struct Route *route = client->server->routes.search(&client->server->routes, uri, sizeof(char[strlen(uri)]));

    char *response = route->route_function(client->server, &request);

    write(client->client, response, sizeof(char[strlen(response)]));
    close(client->client);
    free(client);

    http_request_destructor(&request);
    return NULL;
}

// de continuat !!!