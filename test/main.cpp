#include <signal.h>
#include "HttpServer.h"


static  WebCpp::HttpServer server;

void handle_sigint(int)
{
    server.Close();
}

int main()
{
    signal(SIGINT, handle_sigint);

    if(server.Init())
    {
        server.Run();
        return 0;
    }

    return 1;
}
