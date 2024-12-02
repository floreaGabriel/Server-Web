#include "Server.h"

#include <stdio.h>
#include <stdlib.h>
#include "HTTPRequest.h"
#include <pthread.h>

#define MAX_LENGTH 30000

void * handler(void *arg);

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



void launch(struct Server *server)
{
    // Creăm un ThreadPool cu 20 de thread-uri
    ThreadPool *thread_pool = threadPoolCreate(20);
    if (!thread_pool) {
        perror("Eroare la crearea ThreadPool-ului");
        
        return;
    }

    //struct sockaddr *sock_addr = (struct sockaddr *)&server->server.address;
    int address_length = sizeof(server->address);



    printf("~~~~~~~~~~ WAITING FOR CONNECTION ~~~~~~~~~~~\n\n");

    while (1) {

        // Acceptăm conexiuni de la clienți
        int client_socket = accept(server->socket, (struct sockaddr *)&server->address, (socklen_t *)&address_length);
        
        if (client_socket < 0) {
            perror("Eroare la acceptarea conexiunii");
            continue;
        }

        //printf("Client accepted!\n");

        printf("Conexiune acceptată pe socketul %d si pe portul %d\n", client_socket, server->port);

        // Creăm structura pentru argumentele handler-ului
        struct ClientServer* client = malloc(sizeof(struct ClientServer));
        if (!client) {
            perror("Eroare alocare memorie pentru ClientServer");
            return;
        }


        client->client = client_socket;

        // Adăugăm handler-ul în ThreadPool
        threadPoolEnqueue(thread_pool, handler, client);


        printf("Dupa handler!\n");
    }

    // Distrugem ThreadPool-ul când terminăm
    threadPoolDestroy(thread_pool);
}

void* handler(void *arg)
{
    struct ClientServer *client = (struct ClientServer *)arg;
    int client_socket = client->client;

    char request_string[MAX_LENGTH];

    ssize_t bytes_read = read(client_socket, request_string, sizeof(request_string) - 1);
    if (bytes_read < 0) {
        perror("Eroare la citirea requestului");
        close(client_socket);
        return NULL;
    }
    if (bytes_read >= sizeof(request_string)) {
        fprintf(stderr, "Request prea mare pentru buffer.\n");
        close(client_socket);
        return NULL;
    }


    request_string[bytes_read] = '\0';
    //printf("\n\nHttp primit:\n%s",request_string);
    // Creăm HTTPRequest din request-ul primit
    struct HTTPRequest request = request_constructor(request_string);


    char *method = request.request_line.search(&request.request_line, "method", sizeof("method"));
    char *uri = request.request_line.search(&request.request_line, "uri", sizeof("uri"));

    //obtin calea fisierului acre este cerut in http request
    char path[500];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len != -1) {
        path[len] = '\0'; // Terminăm string-ul

        int lungime=strlen(path);

        strcpy(path+lungime-4,"Resources");

        strcat(path,uri);

        printf("Calea finală: %s\n", path);
    } else {
        perror("readlink");
    }

    //printf("\n\nrequest uri:\n%s\n\n",uri); //in uri am calea
    printf("\n\nrequest:\n\n%s\n\n",request_string); //in request string este tot string ul de il primesc
    
        
    if (strcmp(method, "GET") == 0) {
            printf("Procesăm un request GET pentru: %s\n", uri);

            if (access(path, R_OK) == 0) {
                // Fișierul există, trimitem conținutul
                FILE *file = fopen(path, "r");
                if (!file) {
                    perror("Eroare la deschiderea fișierului");
                    const char *server_error_response = "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n";
                    write(client_socket, server_error_response, strlen(server_error_response));
                } else {
                    fseek(file, 0, SEEK_END);
                    long file_size = ftell(file);
                    rewind(file);

                    char *file_content = malloc(file_size + 1);
                    if (file_content) {
                        fread(file_content, 1, file_size, file);
                        file_content[file_size] = '\0';
                        fclose(file);

                        char header[256];
                        snprintf(header, sizeof(header),
                                "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\nContent-Length: %ld\r\n\r\n",
                                file_size);
                        write(client_socket, header, strlen(header));
                        write(client_socket, file_content, file_size);
                        free(file_content);
                    } else {
                        fclose(file);
                        const char *server_error_response = "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n";
                        write(client_socket, server_error_response, strlen(server_error_response));
                    }
                }
            } else {
                // Fișierul nu există
                printf("Fișierul nu există: %s\n", path);
                const char *not_found_response = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
                write(client_socket, not_found_response, strlen(not_found_response));
            }

        } else if (strcmp(method, "POST") == 0) {
            printf("Procesăm un request POST pentru: %s\n", uri);

            // Preluăm corpul cererii
            char *body = NULL;
            body = strstr(request_string, "\r\n\r\n"); // Găsim separatorul dintre antet și corp
            if (body) {
                body += 4; // Salt peste separatorul "\r\n\r\n"
            } else {
                printf("Nu s-a găsit un body în cerere.\n");
            }
            
            printf("Body-ul cererii POST:\n%s\n", body);

            // Exemplu de răspuns pentru POST
            const char *response_body = "Cererea POST a fost procesată cu succes!";
            char response_header[256];
            snprintf(response_header, sizeof(response_header),
                    "HTTP/1.1 200 OK\r\nContent-Type: text/plain; charset=UTF-8\r\nContent-Length: %ld\r\n\r\n",
                    strlen(response_body));
            write(client_socket, response_header, strlen(response_header));
            write(client_socket, response_body, strlen(response_body));

        } else if (strcmp(method, "PUT") == 0) {
            printf("Procesăm un request PUT pentru: %s\n", uri);

            // Preluăm corpul cererii
            char *body = NULL;
            body = strstr(request_string, "\r\n\r\n"); // Găsim separatorul dintre antet și corp
            if (body) {
                body += 4; // Salt peste separatorul "\r\n\r\n"
            } else {
                printf("Nu s-a găsit un body în cerere.\n");
            }
            printf("Body-ul cererii PUT:\n%s\n", body);

            // Exemplu de răspuns pentru PUT
            const char *response_body = "Cererea PUT a fost procesată cu succes!";
            char response_header[256];
            snprintf(response_header, sizeof(response_header),
                    "HTTP/1.1 200 OK\r\nContent-Type: text/plain; charset=UTF-8\r\nContent-Length: %ld\r\n\r\n",
                    strlen(response_body));
            write(client_socket, response_header, strlen(response_header));
            write(client_socket, response_body, strlen(response_body));

        } else {
            printf("Metodă HTTP necunoscută: %s\n", method);
            const char *method_not_allowed_response = "HTTP/1.1 405 Method Not Allowed\r\nContent-Length: 0\r\n\r\n";
            write(client_socket, method_not_allowed_response, strlen(method_not_allowed_response));
        }

    printf("Inchidem socketul %d ...\n", client_socket);
    // Curățăm resursele
    close(client_socket);
    free(client);

    return NULL;
}


