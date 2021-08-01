#include <signal.h>
#include "HttpServer.h"


static WebCpp::HttpServer server;

void handle_sigint(int)
{
    server.Close();
}

int main()
{
    signal(SIGINT, handle_sigint);

    if(server.Init())
    {
        WebCpp::HttpConfig config;
        config.SetRoot("/home/ruslan/source/webcpp/test/public");
        server.SetConfig(config);

        server.Get("/[{file}]", [](const WebCpp::Request &request, WebCpp::Response &response) -> bool
        {
            bool retval = false;
            std::string file = request.GetArg("file");
            if(!file.empty())
            {
                retval = response.AddFile(file);
            }
            else
            {
                retval = response.AddFile("index.html");
            }

            if(retval == false)
            {
                response.SendNotFound();
            }

            return retval;
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

            response.SetHeader("Content-Type","text/html;charset=utf-8");
            response.Write(std::string("<h2>") + user + ", " + action + "</h2>");
            return true;
        });

        server.Run();
        return 0;
    }

    return 1;
}
