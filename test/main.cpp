#include <signal.h>
#include "HttpServer.h"
#include "Request.h"
#include <string>

static WebCpp::HttpServer server;

void handle_sigint(int)
{
    server.Close();
}

int main()
{
    signal(SIGINT, handle_sigint);

    std::string q = "home?a=2&value=%7Bxyz%7D&name=Ruslan%20Muhlinin";
    WebCpp::Request::UrlDecode(q);

    WebCpp::HttpConfig config;
    config.SetRoot("/home/ruslan/source/webcpp/test/public");
    config.SetProtocol("HTTPS");
    config.SetServerPort(8888);
    config.SetSslSertificate("/home/ruslan/source/webcpp/test/ssl/server.cert");
    config.SetSslKey("/home/ruslan/source/webcpp/test/ssl/server.key");

    if(server.Init(config))
    {
        server.Get("/form", [](const WebCpp::Request &request, WebCpp::Response &response) -> bool
        {
            bool retval = false;

            retval = response.AddFile("form.html");

            if(retval == false)
            {
                response.SendNotFound();
            }

            return retval;
        });

        server.Post("/form", [](const WebCpp::Request &request, WebCpp::Response &response) -> bool
        {
            bool retval = true;

            response.SetHeader("Content-Type","text/html;charset=utf-8");
            response.Write("<div>Ok</div>");

            return retval;
        });


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

        server.Get("/(user|users)/{user:alpha}/[{action:alpha}/]", [](const WebCpp::Request &request, WebCpp::Response &response) -> bool
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
