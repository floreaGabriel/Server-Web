#include "libmini_server.h" // librarie principala


#include "Server-files/HTTPServer.h"


char *home(struct HTTPServer *server, struct HTTPRequest *request)
{
    char *response = render_template(1, "/home/aless/Desktop/proiect2/Server-Web/App/Resources/index.html");
    if (!response) {
        return strdup("HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n");
    }
    return response;
}

char *demo(struct HTTPServer *server, struct HTTPRequest *request)
{
    char *response = render_template(1, "/home/aless/Desktop/proiect2/Server-Web/App/Resources/demo.html");
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


    printf("Serverul este pornit și ascultă pe portul 8081\n");
    launch(&httpserver);

    http_server_destructor(&httpserver);
    return 0;
}
