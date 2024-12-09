#ifndef SERVERCONFIG_H
#define SERVERCONFIG_H

#include "../cJSON/cJSON.h"
#include <stdint.h>

typedef struct {
    int domain;
    int service;
    int protocol;
    uint32_t interface;
    int port;
    int backlog;
    int thread_pool_size;
} ServerConfig;



void load_config(const char *config_file, ServerConfig *config);

#endif /* SERVERCONFIG_H */