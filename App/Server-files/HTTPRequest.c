#include "HTTPRequest.h"
#include "../DataStructures/Lists/Queue.h"

struct HTTPRequest request_constructor(char* request)
{
        struct HTTPRequest http_request;

        
        char requested[strlen(request)];
        strcpy(requested, request);

        for(int i = 0; i < strlen(requested); ++i)
        {
            if (requested[i] == '\r' && requested[i + 1] == '\n' 
                && requested[i + 2] == '\r' && requested[i + 3] == '\n')
                {
                    requested[i + 2]= '#';
                }
        }

        char *request_line = strtok(requested, "\n");
        char *request_header = strtok(NULL, "#");
        char *request_body = strtok(NULL, "\r\n");

        // construire dictionare request

        extract_request_line(&http_request, request_line);
        extract_header_content(&http_request, request_header);
        extract_body_content(&http_request, request_body);

        printf("Buffer modificat:\n\n");

        return http_request;

        
}   

void extract_request_line(struct HTTPRequest* http_request, char* request_line) {

    // GET / HTTP/1.1

    char fields[strlen(request_line)];
    strcpy(fields, request_line);

    char* method = strtok(fields, " ");
    char* uri = strtok(NULL, " ");
    char* http_version = strtok(NULL, " ");

    struct Dictionary rq_line_d = dictionary_constructor(compare_string_keys);

    rq_line_d.insert(&rq_line_d, "method", sizeof("method"), method, sizeof(method));
    rq_line_d.insert(&rq_line_d, "uri", sizeof(uri), uri, sizeof(uri));
    rq_line_d.insert(&rq_line_d, "http_version", sizeof("http_version"), http_version, sizeof(http_version));

    http_request->request_line = rq_line_d;
}

void extract_header_content(struct HTTPRequest* http_request, char* header_content)
{
    char fields[strlen(header_content)];
    strcpy(fields, header_content);

    struct Queue headers = queue_constructor();

    char* field = strtok(fields, "\n");

    while(field)
    {
        headers.push(&headers, field, sizeof(char[strlen(field)]));
        field = strtok(NULL, "\n");
    }

    http_request->header_content = dictionary_constructor(compare_string_keys);

    char *header = headers.peek(&headers);

    while(header)
    {

        char *key = strtok(header, ":");
        char *value = strtok(NULL, "\0");

        if (value)
        {
            if (value[0] == ' ')
            {
                value++;
            }

            http_request->header_content.insert(&http_request->header_content, key, sizeof(key), value, sizeof(value));

        }

        headers.pop(&headers);
        header = headers.peek(&headers);
    }

    queue_destructor(&headers);
}

void extract_body_content(struct HTTPRequest* http_request, char* body)
{
    // verificam de tip de content este in request
    char *content_type = http_request->header_content.search(&http_request->header_content, "Content-Type", sizeof("Content-Type"));

    if (content_type)
    {
        struct Dictionary body_fields = dictionary_constructor(compare_string_keys);

        // verificam daca content-type este application/x-www-form-urlencoded
        // asta inseamna ca este de forma cheie=valoare& ... 

        if (strcmp(content_type, "application/x-www-form-urlencoded") == 0)
        {
            struct Queue fields = queue_constructor();
            char *field = strtok(body, "&");

            while(field)
            {
                fields.push(&fields, field, sizeof(char[strlen(field)]));
            }

            field = fields.peek(&fields);

            while(field)
            {
                char* key = strtok(field, "=");
                char* val = strtok(NULL, "\0");

                if (val[0] == ' ')
                {
                    val++;
                }

                body_fields.insert(&body_fields, key, sizeof(key), val, sizeof(val));

                fields.pop(&fields);
                field = fields.peek(&fields);
            }

            
        }
        else
        {
            body_fields.insert(&body_fields, "data", sizeof("data"), body, sizeof(body));

        }
    }
}


