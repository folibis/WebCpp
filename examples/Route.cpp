#include <signal.h>
#include "common.h"
#include "HttpServer.h"


static WebCpp::HttpServer *httpServerPtr = nullptr;

void handle_sigint(int)
{
    httpServerPtr->Close(false);
}

int main()
{
    signal(SIGINT, handle_sigint);

    WebCpp::HttpServer httpServer;
    httpServerPtr = &httpServer;

    WebCpp::HttpConfig config;
    config.SetRoot(PUBLIC_DIR);
    config.SetHttpProtocol("HTTP");
    config.SetHttpServerPort(8080);

    if(httpServer.Init(config))
    {
        httpServer.OnGet("/[{file}]", [](const WebCpp::Request &request, WebCpp::Response &response) -> bool
        {
            std::string file = request.GetArg("file");
            bool retval = false;

            if(file.empty())
            {
                retval = response.AddFile("route.html");
            }
            else
            {
                retval = response.AddFile(file);
            }
            if(retval == false)
            {
                response.SendNotFound();
            }
            else
            {
                response.AddHeader(WebCpp::HttpHeader::HeaderType::CacheControl,"no-cache");
            }

            return retval;
        });

        httpServer.OnGet("/route/{param1:alpha}[/{param2:string}][/{param3:numeric}/]", [](const WebCpp::Request &request, WebCpp::Response &response) -> bool
        {
            std::string param1 = request.GetArg("param1");
            std::string param2 = request.GetArg("param2");
            std::string param3 = request.GetArg("param3");

            response.AddHeader("Content-Type","text/plain;charset=utf-8");
            response.Write("param1: " + param1 + ", param2: " + param2 + ",param3: " + param3);
            return true;
        });

        httpServer.Run();
        httpServer.WaitFor();
    }

    return 1;
}
