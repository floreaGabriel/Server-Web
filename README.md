# HTTP Server Instructions

# Configurations
- This is a HTTP Server, it listens only for HTTP Requests.
- Server runs on port __8080__
- IP Address: __127.0.0.1__


# Usage
## Step 1
- Add the library in your main project file. ( "libmini_server.h" ):
```C
#include "libmini_server.h" // main library

int main(int argc, char* argv[])
{
    return 0;
}
```
## Step 2
- Include Server.h header from "Server-files" :
```C
#include "libmini_server.h" // main library
#include "Server-files/HTTPServer.h"
int main(int argc, char* argv[])
{
    return 0;
}
```
## Step 3
- Initialize your HTTTP Server and add routes.
  Adding a route has the following template.
    - Create a function for adding a route. (Give it a suggestive name!)
        - **char * name(struct HTTPServer *server, struct HTTPRequest *request);**
    - Inside the function define the functionality of your route.
    - You can use the function
      - __render_template(int num_of_files, "filename1", "filename2", ...);__
        - this function returns in a ** char * ** variable the content of thoese files.
    - Use function from your HTTP Server struct
      - __register.routes(struct HTTPServer *server, 
        char * (*route_function)(struct HTTPServer *server, struct HTTPRequest *request),
        char *uri, int num_methods, ...);__ to add that route to your server.
    - And then launch the server.
    - Dont't forget to add the serer destructor.

```C
#include "libmini_server.h" // main library
#include "Server-files/HTTPServer.h"

char *home(struct HTTPServer *server, struct HTTPRequest *request)
{
    char *response = render_template(1, "Resources/index.html");
    if (!response) {
        return strdup("HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n");
    }
    return response;
}

char *demo(struct HTTPServer *server, struct HTTPRequest *request)
{
    char *response = render_template(1, "Resources/demo.html");
    if (!response) {
        return strdup("HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n");
    }
    return response;
}

int main(int argc, char* argv[])
{
    struct HTTPServer httpserver = http_server_constructor();

    httpserver.register_routes(&httpserver, home, "/", 0);

    httpserver.register_routes(&httpserver, demo, "/demo?cautare=masina", 0);

    launch(&httpserver);

    http_server_destructor(&httpserver);
    return 0;
}
```

# Step 4

- Run the makefile to compile the program and the library.
  - run **make** in the terminal
- And then run the server:
  - __./main__ in the terminal
