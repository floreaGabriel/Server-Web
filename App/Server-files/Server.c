#include "Server.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define MAX_LENGTH 30000

void handler(void *arg);
void execute_cgi_script(const char *script_path, const char *body, int client_socket);
int authenticate(const char *auth_header);
void send_response(int client_socket, const char *status, const char *body, const char *content_type);
void parse_post_data(const char *body, const char *content_type, int client_socket);

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


// Extrage valoarea unui header dat (de exemplu, "Content-Type" sau "Authorization")
char* get_header_value(const char *request, const char *header) {
    char *start = strstr(request, header);
    if (start) {
        start += strlen(header);
        while (*start == ' ' || *start == ':') {
            start++;
        }

        // Căutăm terminatorul de linie
        char *end = strstr(start, "\r\n");
        if (end) {
            size_t length = end - start;
            char *value = (char *)malloc(length + 1);
            if (value) {
                strncpy(value, start, length);
                value[length] = '\0';
                return value;
            }
        }
    }
    return NULL;
}


char* get_method_from_request(const char *request) {
    static char method[16];
    int i = 0;

    while (request[i] != ' ' && request[i] != '\0' && i < sizeof(method) - 1) {
        method[i] = request[i];
        i++;
    }
    method[i] = '\0';

    return method;
}
char* get_uri_from_request(const char *request) {
    static char uri[256];
    int i = 0;
    int j = 0;
    int state = 0; // 0 = înainte de metodă, 1 = între metodă și URI, 2 = după URI

    while (request[i] != '\0') {
        // Începe extragerea URI-ului după ce am găsit spațiul între metodă și URI
        if (state == 1 && request[i] != ' ' && request[i] != '\r' && request[i] != '\n') {
            uri[j++] = request[i];
        } else if (state == 0 && request[i] == ' ') {
            state = 1; // am trecut de metodă, începem să citim URI-ul
        } else if (state == 1 && (request[i] == ' ' || request[i] == '\r' || request[i] == '\n')) {
            break; // Stop extragerea când ajungem la sfârșitul URI-ului
        }
        i++;
    }

    uri[j] = '\0'; // Asigurăm că string-ul este terminat corect
    return uri;
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



void handler(void *arg) {
    struct ClientServer *client = (struct ClientServer *)arg;
    int client_socket = client->client;

    char request_string[MAX_LENGTH];
    ssize_t bytes_read = read(client_socket, request_string, sizeof(request_string) - 1);
    if (bytes_read < 0) {
        perror("Eroare la citirea requestului");
        close(client_socket);
        return;
    }

    if (bytes_read >= sizeof(request_string)) {
        fprintf(stderr, "Request prea mare pentru buffer.\n");
        close(client_socket);
        return ;
    }

    request_string[bytes_read] = '\0';


    // Extragem metoda și URI-ul direct din request_string
    char *method = get_method_from_request(request_string);
    char *uri = get_uri_from_request(request_string);
    char *content_type = get_header_value(request_string, "Content-Type");
    char *auth_header = get_header_value(request_string, "Authorization");

    //printf("\n\nrequest uri:\n%s\n\n",uri); //in uri am calea
    //printf("\n\nrequest:\n\n%s\n\n",request_string); //in request string este tot string ul de il primesc


    printf("PRINTEAZA DATE:\n");
    printf("tot requestul:\n\n%s\n\n\nmethod:\n%s\nuri:\n%s\ncontent_type:\n%s\nauth_header:\n%s\n\n"
            ,request_string,method,uri,content_type,auth_header);


    if (auth_header) {
        if (!authenticate(auth_header)) {
            send_response(client_socket, "401 Unauthorized", "Invalid credentials", "text/plain");
            close(client_socket);
            free(client);
            return;
        }
    }

    char path[500];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len != -1) {
        path[len] = '\0'; // Terminăm string-ul
        int lungime = strlen(path);
        strcpy(path + lungime - 4, "Resources");  // Adăugăm directorul resurselor
        strcat(path, uri);  // Adăugăm URI-ul pentru a forma calea completă
        printf("Calea finală: %s\n", path);
    } else {
        perror("readlink");
    }

    // Analizăm cererea în funcție de metoda HTTP
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

        char *body = strstr(request_string, "\r\n\r\n");
        if (body) {
            body += 4; // Salt peste separatorul "\r\n\r\n"
        } else {
            printf("Nu s-a găsit un body în cerere.\n");
        }

        if (strstr(uri, ".cgi") != NULL) {
            printf("a intrat in .cgi la post");
            execute_cgi_script(path, body, client_socket);
        } else {
            printf("nu a intrat in .cgi la post");
            parse_post_data(body, content_type, client_socket);
        }
    } else if (strcmp(method, "PUT") == 0) {
        printf("Procesăm un request PUT pentru: %s\n", uri);

        char *body = strstr(request_string, "\r\n\r\n");
        if (body) {
            body += 4;
        } else {
            printf("Nu s-a găsit un body în cerere.\n");
        }

        printf("body:%s \n\n",body);

        if (strstr(uri, ".cgi") != NULL) {
            printf("a intrat in .cgi la put");
            execute_cgi_script(path, body, client_socket);
        } else {
            printf("nu a intrat in .cgi la put");
            parse_post_data(body, content_type, client_socket); // Poate fi folosit și pentru PUT
        }

    } else {
        printf("Metodă HTTP necunoscută: %s\n", method);
        send_response(client_socket, "405 Method Not Allowed", "Metoda HTTP nu este permisă", "text/plain");
    }

    close(client_socket);
    free(client);
}

int authenticate(const char *auth_header) {
    // Simplu exemplu de autentificare bazată pe token. Poți schimba logica aici
    if (strncmp(auth_header, "Bearer token123", 16) == 0) {
        return 1; // Autentificare validă
    }
    return 0; // Autentificare invalidă
}

void send_response(int client_socket, const char *status, const char *body, const char *content_type) {
    char response_header[512];
    snprintf(response_header, sizeof(response_header),
            "HTTP/1.1 %s\r\nContent-Type: %s; charset=UTF-8\r\nContent-Length: %ld\r\n\r\n",
            status, content_type, strlen(body));

    write(client_socket, response_header, strlen(response_header));
    write(client_socket, body, strlen(body));
}

void parse_post_data(const char *body, const char *content_type, int client_socket) {
    if (strcmp(content_type, "application/x-www-form-urlencoded") == 0) {
        // Procesare form-uri
        printf("Procesăm form-ul: %s\n", body);
        // Logică de salvare date într-o bază de date sau fișier
    } else if (strcmp(content_type, "application/json") == 0) {
        // Procesare JSON
        printf("Procesăm JSON: %s\n", body);
        // Logică de parsare JSON
    } else {
        send_response(client_socket, "415 Unsupported Media Type", "Tipul de conținut nu este acceptat", "text/plain");
    }
}

void execute_cgi_script(const char *script_path, const char *body, int client_socket) {
    pid_t pid = fork();
    if (pid == 0) {
        // Suntem în procesul copil

        // Trimiterea capetelor HTTP înainte de a executa CGI
        // De exemplu, trimitem capetele pentru un răspuns valid
        char *response_header = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n";
        write(client_socket, response_header, strlen(response_header));

        // Redirecționăm stdout pentru a scrie în client_socket
        dup2(client_socket, STDOUT_FILENO);
        dup2(client_socket, STDERR_FILENO); // Redirecționăm și stderr

        // Executăm scriptul CGI
        execl(script_path, script_path, NULL);
        
        // Dacă ajungem aici, execuția a eșuat
        perror("Exec failed");
        exit(1);
    } else if (pid < 0) {
        // Eroare la crearea procesului
        perror("Fork failed");
    }
}


