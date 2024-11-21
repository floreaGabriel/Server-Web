#include "HTTPServer.h"

#define MAX_LENGTH 30000

#include <stdarg.h>
#include "ThreadPool.h"
#include <stdlib.h>
#include <sys/socket.h> 

void register_routes(struct HTTPServer *server, 
        char * (*route_function)(struct HTTPServer *server, struct HTTPRequest *request),
        char *uri, int num_methods, ...);   

void * handler(void *arg);
void launch(struct HTTPServer *server);

struct HTTPServer http_server_constructor(void) {

    struct HTTPServer server;

    server.server = server_constructor(AF_INET, SOCK_STREAM, 0, INADDR_ANY, 8081, 255);
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
        char *(*route_function)(struct HTTPServer *server, struct HTTPRequest *request),
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
    route.uri = malloc(strlen(uri) + 1); // pentru a păstra string-ul corect
    if (!route.uri) {
        perror("Eroare alocare memorie pentru route.uri");
        return;
    }
    strcpy(route.uri, uri);

    printf("register router uri: %s , %s\n",uri,route.uri);


    // functia rutei + salvam ruta in server
    route.route_function = route_function;
    server->routes.insert(&server->routes, uri, sizeof(char[strlen(uri)]), &route, sizeof(route));

} 


void launch(struct HTTPServer *server)
{
    // Creăm un ThreadPool cu 20 de thread-uri
    ThreadPool *thread_pool = threadPoolCreate(20);
    if (!thread_pool) {
        perror("Eroare la crearea ThreadPool-ului");
        return;
    }

    struct sockaddr *sock_addr = (struct sockaddr *)&server->server.address;
    int address_length = sizeof(server->server.address);

    while (1) {
        printf("~~~~~~~~~~ WAITING FOR CONNECTION ~~~~~~~~~~~\n\n");

        // Acceptăm conexiuni de la clienți
        int client_socket = accept(server->server.socket, sock_addr, (socklen_t *)&address_length);
        if (client_socket < 0) {
            perror("Eroare la acceptarea conexiunii");
            continue;
        }

        printf("Client accepted!\n");

        // Creăm structura pentru argumentele handler-ului
        struct ClientServer *client = malloc(sizeof(struct ClientServer));
        if (!client) {
            perror("Eroare alocare memorie pentru ClientServer");
            close(client_socket);
            continue;
        }
        client->client = client_socket;
        client->server = server;

        // Adăugăm handler-ul în ThreadPool
        threadPoolEnqueue(thread_pool, handler, client);
    }

    // Distrugem ThreadPool-ul când terminăm
    threadPoolDestroy(thread_pool);
}

void *handler(void *arg)
{
    struct ClientServer *client = (struct ClientServer *)arg;

    char request_string[MAX_LENGTH];

    ssize_t bytes_read = read(client->client, request_string, sizeof(request_string));
    if (bytes_read <= 0) {
        perror("Eroare la citirea requestului");
        close(client->client);
        free(client);
        return NULL;
    }

    request_string[bytes_read] = '\0';
    printf("%s\n\n", request_string);

    // Creăm HTTPRequest din request-ul primit
    struct HTTPRequest request = request_constructor(request_string);

    char *uri = request.request_line.search(&request.request_line, "uri", sizeof("uri"));

    printf("testCRAPAT 2: %s\n",uri);

    // Căutăm ruta corespunzătoare
    struct Route *route = client->server->routes.search(&client->server->routes, uri, strlen(uri) + 1);
    if (!route) {
        printf("Ruta nu a fost găsită pentru URI: %s\n", uri);
        return NULL;  // Tratați corespunzător cazul în care ruta nu este găsită
    }
    printf("testCRAPAT 3\n");

    if (route) {
        printf("testCRAPAT 4.1\n");
        char *response = route->route_function(client->server, &request);
        printf("testCRAPAT 4\n");

        // Creăm răspunsul HTTP complet (header + conținut)
        char *response_header = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\nContent-Length: %ld\r\n\r\n";
        int response_header_len = snprintf(NULL, 0, response_header, strlen(response));
        char *final_response = malloc(response_header_len + strlen(response) + 1);
        
        if (final_response == NULL) {
            perror("Eroare la alocarea memoriei pentru final_response");
            free(response);
            return NULL;
        }

        // Concatenează header-ul și răspunsul
        snprintf(final_response, response_header_len + 1, response_header, strlen(response));
        strcat(final_response, response);  // Adaugă conținutul HTML

        // Trimitem răspunsul complet
        ssize_t bytes_sent = send(client->client, final_response, strlen(final_response), 0);
        if (bytes_sent == -1) {
            perror("Eroare la trimiterea răspunsului");
        } else {
            printf("Răspuns trimis: \n\n%s\n", final_response);
        }

        free(response);
        free(final_response);
    } else {
        const char *not_found_response = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
        printf("testCRAPAT 6\n");
        write(client->client, not_found_response, strlen(not_found_response));
        printf("testCRAPAT 7\n");
    }

    // Curățăm resursele
    close(client->client);
    free(client);
    printf("testCRAPAT 8\n");
    return NULL;
}


// de continuat !!!




// Joins the contents of multiple files into one.
char *render_template(int num_templates, ...)
{
    // Create a buffer to store the data in.
    char *buffer = malloc(30000);
    int buffer_position = 0;
    char c;
    FILE *file;
    // Iterate over the files given as arguments.
    va_list files;
    va_start(files, num_templates);
    // Read the contents of each file into the buffer.
    for (int i = 0; i < num_templates; i++)
    {
        char *path = va_arg(files, char*);
        file = fopen(path, "r");
        while ((c = fgetc(file)) != EOF)
        {
            buffer[buffer_position] = c;
            buffer_position += 1;
        }
    }
    va_end(files);
    return buffer;
}

void http_server_destructor(struct HTTPServer *server)
{
    printf("Closing server socket...\n");
    server_destructor(&server->server);
    printf("Server socket closed!\n");
}