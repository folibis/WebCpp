#include <signal.h>
#include "common_webcpp.h"
#include "defines_webcpp.h"
#include "HttpServer.h"
#include "WebSocketServer.h"
#include "Request.h"
#include <string>
#include "StringUtil.h"
#include "ResponseWebSocket.h"
#include "FcgiClient.h"
#include "StringUtil.h"
#include "ThreadWorker.h"


static WebCpp::HttpServer *httpServerPtr = nullptr;
static WebCpp::WebSocketServer *wsServerPtr = nullptr;

void handle_sigint(int)
{
    httpServerPtr->Close(false);
    wsServerPtr->Close(false);
}

int main(int argc, char *argv[])
{
    int port_http = 8080;
    int port_ws = 8081;
    int t;
    std::string action;

    for(int i = 1;i < argc;i ++)
    {
        switch(_(argv[i]))
        {
            case _("-ph"):
                action = "port_http";
                break;
            case _("-pw"):
                action = "port_ws";
                break;
            default:
                switch(_(action.c_str()))
                {
                    case _("port_http"):
                        if(StringUtil::String2int(argv[i], t))
                        {
                            port_http = t;
                        }
                        break;
                    case _("port_ws"):
                        if(StringUtil::String2int(argv[i], t))
                        {
                            port_ws = t;
                        }
                        break;
                }

                action = "";
                break;
        }
    }

    signal(SIGINT, handle_sigint);

    int connID = (-1);
    int min = 1,max = 100;
    WebCpp::HttpServer httpServer;
    WebCpp::WebSocketServer wsServer;
    httpServerPtr = &httpServer;
    wsServerPtr = &wsServer;

    WebCpp::HttpConfig config;
    config.SetRoot(PUBLIC_DIR);
    config.SetHttpProtocol("HTTP");
    config.SetHttpServerPort(port_http);
    config.SetWsProtocol("WS");
    config.SetWsServerPort(port_ws);

    if(httpServer.Init(config))
    {
        httpServer.OnGet("/[{file}]", [](const WebCpp::Request &request, WebCpp::Response &response) -> bool
        {
            std::string file = request.GetArg("file");
            bool retval = false;

            if(file.empty())
            {
                retval = response.AddFile("ws3.html");
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

    if(wsServer.Init(config))
    {
        wsServer.OnMessage("/ws", [&](const WebCpp::Request &request, WebCpp::ResponseWebSocket &response, const ByteArray &data) -> bool
        {
            response.WriteText(data);
            return true;
        });

        wsServer.Run();
    }

    httpServer.WaitFor();
    wsServer.WaitFor();

    return 0;
}
