#include "libmini_server.h"

int main(int argc, char* argv[])
{

    struct Server server = server_constructor(
        AF_INET, SOCK_STREAM, 0, INADDR_ANY, 8080, 10
    );

    server.launch(&server);


    return 0;
}