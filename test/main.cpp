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
        server.Get("/", [](const WebCpp::Request &request, WebCpp::Response &response)
        {
            response.SetHeader("Content-Type","text/html");
            response.Write("<h2>Main page</h2>");
            return true;
        });
        server.Get("/hello/[{user:string}/]", [](const WebCpp::Request &request, WebCpp::Response &response)
        {
            std::string user = request.Param("user");
            if(user.empty())
            {
                user = "Unknown";
            }
            response.SetHeader("Content-Type","text/html");
            response.Write(std::string("<h2>Hello, ") + user + "</h2>");
            return true;
        });

        server.Run();
        return 0;
    }

    return 1;
}
