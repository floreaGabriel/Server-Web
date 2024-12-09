#include "ServerConfig.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../cJSON/cJSON.h"
#include <sys/socket.h>
#include <arpa/inet.h>

void load_config(const char *config_file, ServerConfig *config) {
    
    printf("CONFIG: %s\n", config_file);
    FILE *file = fopen(config_file, "r");
    if (file == NULL) {
        perror("Eroare la deschiderea fișierului de configurare");
        exit(EXIT_FAILURE);
    }

    // Citim conținutul fișierului
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    char *json_data = (char *)malloc(file_size + 1);
    fread(json_data, 1, file_size, file);
    json_data[file_size] = '\0';
    fclose(file);

    // Parsăm JSON-ul
    cJSON *json = cJSON_Parse(json_data);
    if (!json) {
        fprintf(stderr, "Eroare la parsarea JSON: %s\n", cJSON_GetErrorPtr());
        free(json_data);
        exit(EXIT_FAILURE);
    }

    // Citim setările serverului
    cJSON *server = cJSON_GetObjectItemCaseSensitive(json, "server");
    if (server) {
        cJSON* domain = cJSON_GetObjectItemCaseSensitive(server, "domain");
        if (cJSON_IsString(domain) && (domain->valuestring != NULL)) {
            if (strcmp(domain->valuestring, "AF_INET") == 0)
            {
                 config->domain = AF_INET;
            } else if (strcmp(domain->valuestring, "AF_INET6") == 0) {
                config->domain = AF_INET6;
            } else if (strcmp(domain->valuestring, "AF_UNIX") == 0) {
                config->domain = AF_UNIX;
            } else {
                printf("Valoare necunoscută pentru domain: %s\n", domain->valuestring);
            }
        }
        
        cJSON* service = cJSON_GetObjectItemCaseSensitive(server, "service");
        if (cJSON_IsString(service) && (service->valuestring != NULL)) {
            if (strcmp(service->valuestring, "SOCK_STREAM") == 0) {
                config->service = SOCK_STREAM;
            } else if (strcmp(service->valuestring, "SOCK_DGRAM") == 0) {
                config->service = SOCK_DGRAM;
            } else {
                printf("Valoare necunoscută pentru service: %s\n", service->valuestring);
            }        
        }
        config->protocol = cJSON_GetObjectItemCaseSensitive(server, "protocol")->valueint;
        cJSON* interface = cJSON_GetObjectItemCaseSensitive(server, "interface");
        if (cJSON_IsString(interface) && (interface->valuestring != NULL)) {
            if (strcmp(interface->valuestring, "INADDR_ANY") == 0) {
                config->interface = INADDR_ANY;
            } else {
                printf("Valoare necunoscută pentru interface: %s\n", interface->valuestring);
            }        
        }
        config->port = cJSON_GetObjectItemCaseSensitive(server, "port")->valueint;
        config->backlog = cJSON_GetObjectItemCaseSensitive(server, "backlog")->valueint;
        config->thread_pool_size = cJSON_GetObjectItemCaseSensitive(server, "thread_pool_size")->valueint;
    
    }

    // Eliberăm memoria
    cJSON_Delete(json);
    free(json_data);
}
