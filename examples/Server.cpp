#include <signal.h>
#include "common_webcpp.h"
#include "defines_webcpp.h"
#include "HttpServer.h"
#include "Request.h"
#include <string>
#include "StringUtil.h"
#include <iostream>


static WebCpp::HttpServer *httpServerPtr = nullptr;


void handle_sigint(int)
{
    httpServerPtr->Close(false);
}

int main()
{
    signal(SIGINT, handle_sigint);

    int connID = (-1);
    int min = 1,max = 100;
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

            std::cout << "file: " << file << std::endl << std::endl;

            if(file.empty())
            {
                retval = response.AddFile("server.html");
            }
            else
            {
                retval = response.AddFile(file);
            }
            if(retval == false)
            {
                response.SendNotFound();
            }

            return retval;
        });

        httpServer.Run();
    }

    httpServer.WaitFor();

    return 0;
}
