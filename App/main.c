#include "libmini_server.h" // librarie principala


#include "Server-files/HTTPServer.h"

int main(int argc, char* argv[])
{
    struct HTTPServer server = http_server_constructor();


    server.launch(&server);

}