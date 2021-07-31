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
        server.Get("/", [](const WebCpp::Request &request, WebCpp::Response &response) -> bool
        {
            response.SendFile("index.html");
            return true;
        });

        server.Get("/user/{user:alpha}/[{action:alpha}/]", [](const WebCpp::Request &request, WebCpp::Response &response) -> bool
        {
            std::string user = request.GetArg("user");
            if(user.empty())
            {
                user = "Unknown";
            }
            std::string action = request.GetArg("action");
            if(action.empty())
            {
                action = "Hello!";
            }


            response.SetHeader("Content-Type","text/html");
            response.Write(std::string("<h2>") + user + ", " + action + "</h2>");
            return true;
        });

        server.Run();
        return 0;
    }

    return 1;
}
