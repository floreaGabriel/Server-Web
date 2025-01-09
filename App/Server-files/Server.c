#include "Server.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/wait.h>
#include <zlib.h>


#define MAX_LENGTH 30000

void handler(void *arg);
void execute_cgi_script(const char *script_path, const char *body, int client_socket);
void execute_php_script(const char *script_path, const char *method, const char *data, int client_socket);
int authenticate(const char *auth_header);
void send_response(int client_socket, const char *status, const char *body, const char *content_type);
void parse_post_data(const char *body, const char *content_type, int client_socket);
void process_json(const char *body, int client_socket);
void process_form_urlencoded(const char *body, int client_socket, char* path);
void process_text_plain(const char *body, int client_socket);
void process_unknown(const char *body, int client_socket);
void process_multipart_form_data(const char *body, int client_socket);

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



void launch(struct Server *server, int thread_pool_size)
{
    // Creăm un ThreadPool cu 20 de thread-uri
    ThreadPool *thread_pool = threadPoolCreate(thread_pool_size);
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


int compress_gzip(const char *input, size_t input_len, char *output, size_t *output_len) {
    z_stream stream;
    memset(&stream, 0, sizeof(stream));

    // Initializează stream-ul pentru compresie
    if (deflateInit2(&stream, Z_BEST_COMPRESSION, Z_DEFLATED, 15 | 16, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
        return -1;
    }

    stream.next_in = (Bytef *)input;
    stream.avail_in = input_len;

    stream.next_out = (Bytef *)output;
    stream.avail_out = *output_len;

    // Compresia datelor
    if (deflate(&stream, Z_FINISH) != Z_STREAM_END) {
        deflateEnd(&stream);
        return -1;
    }

    *output_len = stream.total_out;

    // Curăță stream-ul
    deflateEnd(&stream);
    return 0;
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
        fprintf(stderr, "Request prea mare pentru fbuffer.\n");
        close(client_socket);
        return ;
    }

    request_string[bytes_read] = '\0';


    // Extragem metoda și URI-ul direct din request_string
    char *method = get_method_from_request(request_string);
    char *uri = get_uri_from_request(request_string);
    char *content_type = get_header_value(request_string, "Content-Type");
    char *auth_header = get_header_value(request_string, "Authorization");
    char *accept_encoding = get_header_value(request_string,"Accept-Encoding");


    int supports_gzip=0;
    if(accept_encoding && strstr(accept_encoding,"gzip"))
    {
        supports_gzip=1;
    }


    printf("PRINTEAZA DATE:\n");
    printf("tot requestul:\n\n%s\n\n\nmethod:\n%s\nuri:\n%s\ncontent_type:\n%s\nauth_header:\n%s\n\n"
            ,request_string,method,uri,content_type,auth_header);

    // verificare pentru accesare de subdirectoare... bad request
    if (strstr(uri, "..")) {
        send_response(client_socket, "400 Bad Request", "Invalid file path", "text/plain");
        return;
    }


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

    // setare functie initiala pentru /
    if (strcmp(uri, "/") == 0) {
        strcat(path, "index.html");
    }
    // Analizăm cererea în funcție de metoda HTTP
    if (strcmp(method, "GET") == 0) {
        printf("Procesăm un request GET pentru: %s\n", uri);


        if (strstr(uri, ".php") != NULL) {
        char *query_string = strchr(uri, '?'); // Găsim începutul query string-ului (parametrii)

        char script_path[500];
        for(int i=0;i<strlen(path);i++)
        {
            if(path[i]!='?')
            {
                script_path[i]=path[i];
            } else
            {
                i=strlen(path);
            }
        }

        if (query_string) {
            query_string++; // Sărim peste caracterul `?`
        } else {
            query_string = ""; // Nu există query string
        }

        // Apelăm funcția pentru execuția scriptului PHP
        execute_php_script(script_path, method,query_string, client_socket);
        close(client_socket);
        free(client);
        return;
    }
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

                    printf("am ajuns inainte de supports_gzip%d\n",supports_gzip);
                    if (supports_gzip) {
                        printf("executa compresia acuma\n\n");
                        // Aplicăm compresia Gzip
                        char compressed_content[1024 * 10]; // Buffer pentru conținut comprimat
                        size_t compressed_size = sizeof(compressed_content);

                        if (compress_gzip(file_content, file_size, compressed_content, &compressed_size) == 0) {
                            // Trimitem răspunsul comprimat
                            char header[256];
                            snprintf(header, sizeof(header),
                                    "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\nContent-Encoding: gzip\r\nContent-Length: %ld\r\n\r\n",
                                    compressed_size);
                            printf("heade=\n%s",header);
                            write(client_socket, header, strlen(header));
                            write(client_socket, compressed_content, compressed_size);
                        } else {
                            // Eroare la compresie
                            const char *server_error_response = "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n";
                            write(client_socket, server_error_response, strlen(server_error_response));
                        }
                    } else {                    
                        char header[256];
                        snprintf(header, sizeof(header),
                                "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\nContent-Length: %ld\r\n\r\n",
                                file_size);
                        write(client_socket, header, strlen(header));
                        write(client_socket, file_content, file_size);
                    }
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
            send_response(client_socket, "400 Bad Request", "Body missing in POST request", "text/plain");
            close(client_socket);
            free(client);
            return;

        }

        if (!content_type) {
            printf("Antetul Content-Type lipsește.\n");
            send_response(client_socket, "400 Bad Request", "Content-Type missing", "text/plain");
            close(client_socket);
            free(client);
            return;
        } 


        printf("Body-ul cererii:\n%s\n", body);
        printf("Content-Type-ul cererii: %s\n", content_type);

        // Detectare și procesare fișiere CGI
        if (strstr(uri, ".php") != NULL) {
            execute_php_script(path, method,body, client_socket);
            close(client_socket);
            free(client);
            return;
        } else if (strstr(uri, ".cgi") != NULL) {
            execute_cgi_script(path, body, client_socket);
        } else if (strstr(content_type, "application/json") != NULL) {
            process_json(body, client_socket);
        } else if (strstr(content_type, "multipart/form-data") != NULL) {
            process_multipart_form_data(body, client_socket);
        } else if (strstr(content_type, "application/x-www-form-urlencoded") != NULL) { 
            process_form_urlencoded(body, client_socket, path);
        } else if (strstr(content_type, "text/plain") != NULL) {
            process_text_plain(body, client_socket);
        } else {
            process_unknown(body, client_socket);
        }
    } else if (strcmp(method, "PUT") == 0) {
        printf("Procesăm un request PUT pentru: %s\n", uri);

        // Găsim corpul cererii
        char *body = strstr(request_string, "\r\n\r\n");
        if (body) {
            body += 4; // Sărim peste separatorul "\r\n\r\n"
        } else {
            printf("Nu s-a găsit un body în cerere.\n");
            send_response(client_socket, "400 Bad Request", "Body missing in PUT request", "text/plain");
            close(client_socket);
            free(client);
            return;
        }

        if (!content_type) {
            printf("Antetul Content-Type lipsește.\n");
            send_response(client_socket, "400 Bad Request", "Content-Type missing", "text/plain");
            close(client_socket);
            free(client);
            return;
        }

        printf("Body-ul cererii (PUT):\n%s\n", body);
        printf("Content-Type-ul cererii (PUT): %s\n", content_type);

        // Procesare requesturi PUT pentru fișiere
        if (strstr(content_type, "text/plain") != NULL) {
                // Salvăm corpul cererii ca fișier text pe server
            FILE *file = fopen(path, "w");
            if (!file) {
                perror("Eroare la deschiderea fișierului pentru scriere");
                send_response(client_socket, "500 Internal Server Error", "Error saving file", "text/plain");
                close(client_socket);
                free(client);
                return;
            }
            fprintf(file, "%s", body);
            fclose(file);

            printf("Fișier actualizat cu succes: %s\n", path);
            send_response(client_socket, "200 OK", "File updated successfully", "text/plain");
        } else if (strstr(content_type, "application/json") != NULL) {
            // Salvăm JSON-ul primit în corpul cererii ca fișier JSON
            FILE *file = fopen(path, "w");
            if (!file) {
                perror("Eroare la deschiderea fișierului pentru scriere");
                send_response(client_socket, "500 Internal Server Error", "Error saving JSON file", "text/plain");
                close(client_socket);
                free(client);
                return;
            }
            fprintf(file, "%s", body);
            fclose(file);

            printf("Fișier JSON actualizat cu succes: %s\n", path);
            send_response(client_socket, "200 OK", "JSON updated successfully", "text/plain");
        } else {
            printf("Content-Type-ul nu este suportat pentru PUT: %s\n", content_type);
            send_response(client_socket, "415 Unsupported Media Type", "Content-Type not supported for PUT", "text/plain");
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

// ~~~~~~~~~~~~
// CGI + POST DATA functions 

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

void execute_php_script(const char *script_path, const char *method, const char *data, int client_socket) {
    int pipe_stdin[2], pipe_stdout[2];
    int use_stdin = strcmp(method, "POST") == 0; // Folosim stdin doar pentru POST

    // Creăm pipe-urile necesare
    if ((use_stdin && pipe(pipe_stdin) == -1) || pipe(pipe_stdout) == -1) {
        perror("Eroare la crearea pipe-urilor");
        send_response(client_socket, "500 Internal Server Error", "Failed to execute PHP script", "text/plain");
        return;
    }

    pid_t pid = fork();
    if (pid == 0) {
        // Procesul copil
        if (use_stdin) close(pipe_stdin[1]);
        close(pipe_stdout[0]);

        if (use_stdin) dup2(pipe_stdin[0], STDIN_FILENO); // Redirecționăm STDIN pentru POST
        dup2(pipe_stdout[1], STDOUT_FILENO);             // Redirecționăm STDOUT
        close(pipe_stdout[1]);

        if (use_stdin) close(pipe_stdin[0]);

        // Setăm variabilele de mediu necesare
        setenv("REQUEST_METHOD", method, 1);
        setenv("SCRIPT_FILENAME", script_path, 1);
        setenv("REDIRECT_STATUS", "200", 1);

        if (strcmp(method, "POST") == 0) {
            // Setăm variabilele specifice POST
            setenv("CONTENT_TYPE", "application/x-www-form-urlencoded", 1);
            char content_length[16];
            snprintf(content_length, sizeof(content_length), "%zu", strlen(data));
            setenv("CONTENT_LENGTH", content_length, 1);
        } else if (strcmp(method, "GET") == 0) {
            // Setăm variabilele specifice GET
            setenv("QUERY_STRING", data, 1); // `data` este query string-ul în cazul GET
        }

        // Executăm php-cgi
        execlp("php-cgi", "php-cgi", NULL);

        // Dacă ajungem aici, execuția a eșuat
        perror("Execuția PHP a eșuat");
        exit(1);
    } else if (pid > 0) {
        // Procesul părinte
        if (use_stdin) close(pipe_stdin[0]);
        close(pipe_stdout[1]);

        // Scriem body-ul în STDIN-ul procesului copil pentru POST
        if (use_stdin) {
            write(pipe_stdin[1], data, strlen(data));
            close(pipe_stdin[1]);
        }

        // Citim răspunsul din STDOUT-ul procesului copil
        char buffer[4096];
        ssize_t bytes_read;

        // Trimitere capete HTTP
        char *response_header = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n";
        write(client_socket, response_header, strlen(response_header));

        // Trimitere conținut
        while ((bytes_read = read(pipe_stdout[0], buffer, sizeof(buffer))) > 0) {
            write(client_socket, buffer, bytes_read);
        }

        close(pipe_stdout[0]);
        waitpid(pid, NULL, 0);
    } else {
        perror("Eroare la fork");
        send_response(client_socket, "500 Internal Server Error", "Failed to fork process", "text/plain");
    }
}




// ~~~~~~~~~~~~~~~~~~
// POST processing functions 

void process_json(const char *body, int client_socket) {
    printf("Procesăm JSON: %s\n", body);
    send_response(client_socket, "200 OK", "Json processed successfully", "application/json");
}
void process_form_urlencoded(const char *body, int client_socket, char* file_path) {
    printf("Procesăm form-urlencoded: %s\n", body);

    // Fișierul care conține datele
    printf("FISIER: %s\n", file_path);
    FILE *file = fopen(file_path, "r+");
    if (!file) {
        perror("Eroare la deschiderea fișierului");
        send_response(client_socket, "500 Internal Server Error", "Could not open data file", "application/json");
        return;
    }

    // Extragem numele și emailul din body
    char name_to_check[128] = {0};
    char email_to_check[128] = {0};

    const char *name_key = "name=";
    const char *email_key = "email=";

    char *name_start = strstr(body, name_key);
    char *email_start = strstr(body, email_key);

    if (name_start) {
        name_start += strlen(name_key);
        char *name_end = strchr(name_start, '&'); // Numele se termină înainte de '&'
        if (name_end) {
            size_t name_length = name_end - name_start;
            strncpy(name_to_check, name_start, name_length);
            name_to_check[name_length] = '\0';
        } else {
            strcpy(name_to_check, name_start); // Dacă nu există '&', citim până la sfârșitul body-ului
        }
    }

    if (email_start) {
        email_start += strlen(email_key);
        char *email_end = strchr(email_start, '\0'); // Emailul este până la sfârșitul body-ului
        if (email_end) {
            size_t email_length = email_end - email_start;
            strncpy(email_to_check, email_start, email_length);
            email_to_check[email_length] = '\0';
        }
    }

    // Verificăm dacă linia completă (nume și email) există deja în fișier
    int entry_exists = 0;
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // Eliminăm newline-ul de la finalul liniei
        line[strcspn(line, "\n")] = '\0';

        char existing_name[128];
        char existing_email[128];
        if (sscanf(line, "%127s %127s", existing_name, existing_email) == 2) {
            if (strcmp(existing_name, name_to_check) == 0 && strcmp(existing_email, email_to_check) == 0) {
                entry_exists = 1;
                break;
            }
        }
    }

    if (entry_exists) {
        printf("Numele și emailul %s %s există deja.\n", name_to_check, email_to_check);
        send_response(client_socket, "200 OK", "Name and email already exist", "application/json");
    } else {
        // Adăugăm linia la finalul fișierului
        fseek(file, 0, SEEK_END);
        fprintf(file, "%s %s\n", name_to_check, email_to_check);
        printf("Date adăugate: %s %s\n", name_to_check, email_to_check);
        send_response(client_socket, "200 OK", "Name and email added successfully", "application/json");
    }

    fclose(file);
}


void process_text_plain(const char *body, int client_socket) {
    printf("Procesăm text/plain: %s\n", body);
    send_response(client_socket, "200 OK", "Text processed successfully", "text/plain");
}
void process_unknown(const char *body, int client_socket) {
    printf("Tip de conținut necunoscut: %s\n", body);
    send_response(client_socket, "415 Unsupported Media Type", "Unsupported Content-Type", "text/plain");
} 

void process_multipart_form_data(const char *body, int client_socket) {
    printf("Procesăm imaginea încărcată...\n");

    // 6. Răspuns către client
    send_response(client_socket, "200 OK", "Image uploaded successfully", "text/plain");
}