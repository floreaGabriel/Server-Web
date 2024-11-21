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
    struct HTTPServer server = http_server_constructor();


    server.register_routes(&server, home, "/", 0);

    server.register_routes(&server, demo, "/demo?cautare=masina", 0);


    printf("Serverul este pornit și ascultă pe portul 8081\n");
    server.launch(&server);

    http_server_destructor(&server);
    return 0;
}
