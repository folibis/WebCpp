#include <signal.h>
#include "common_webcpp.h"
#include "HttpServer.h"


static WebCpp::HttpServer *httpServerPtr;

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
        httpServer.OnGet("/", [](const WebCpp::Request &, WebCpp::Response &response) -> bool
        {
            response.AddHeader("Content-Type","text/html;charset=utf-8");
            response.Write("<h3>WebCpp works!</h3>");

            return true;
        });

        httpServer.Run();
        httpServer.WaitFor();
    }

    return 1;
}
