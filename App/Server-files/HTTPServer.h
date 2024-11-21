#ifndef HTTPServer_h
#define HTTPServer_h

#include "HTTPRequest.h"
#include "Server.h"
#include "../DataStructures/Dictionary/Dictionary.h"

struct HTTPServer {

    struct Server server;
    struct Dictionary routes;
    pthread_mutex_t routes_mutex; // Mutex pentru protecția `routes`
    void (*register_routes)(struct HTTPServer *server, 
        char * (*route_function)(struct HTTPServer *server, struct HTTPRequest *request),
        char *uri, int num_methods, ...);
};

enum HTTPMethods
{
    CONNECT,
    DELETE,
    GET,
    HEAD,
    OPTIONS,
    PATCH,
    POST,
    PUT,
    TRACE
};

void launch(struct HTTPServer *server);
struct HTTPServer http_server_constructor(void);
void http_server_destructor(struct HTTPServer *server);

char *render_template(int num_templates, ...);

#endif