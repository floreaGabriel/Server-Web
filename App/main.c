#include "libmini_server.h" // librarie principala


#include "Server-files/HTTPServer.h"


char * home(struct HTTPServer *server, struct HTTPRequest *request)
{
    return render_template(1, "/home/floreagabriel/PSO-PROIECT/Server-Web/App/Resources/index.html");
}

int main(int argc, char* argv[])
{
    struct HTTPServer server = http_server_constructor();

    server.register_routes(&server, home, "/", 0);
    
    server.launch(&server);

    http_server_destructor(&server);
}