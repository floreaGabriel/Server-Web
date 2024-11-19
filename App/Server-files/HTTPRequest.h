
#ifndef HTTPRequest_h
#define HTTPRequest_h

#include "../DataStructures/Dictionary/Dictionary.h"
#include <string.h>
#include <stdio.h>





struct HTTPRequest {

    // requestul este de forma 
    // linie_de_request
    // header_content
    // body_content

    struct Dictionary request_line;
    struct Dictionary header_content;
    struct Dictionary body_content;
};


struct HTTPRequest request_constructor(char* request);
void extract_request_line(struct HTTPRequest*, char*);
void extract_header_content(struct HTTPRequest*, char*);
void extract_body_content(struct HTTPRequest*, char*);

#endif /* HTTPRequest_h */