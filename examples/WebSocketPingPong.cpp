#include <signal.h>
#include "common_webcpp.h"
#include "defines_webcpp.h"
#include "HttpServer.h"
#include "WebSocketServer.h"
#include "Request.h"
#include <string>
#include "DebugPrint.h"
#include "StringUtil.h"
#include "ResponseWebSocket.h"
#include "FcgiClient.h"
#include "StringUtil.h"
#include "ThreadWorker.h"
#include "example_common.h"


static WebCpp::HttpServer *httpServerPtr = nullptr;
static WebCpp::WebSocketServer *wsServerPtr = nullptr;

void handle_sigint(int)
{
    httpServerPtr->Close(false);
    wsServerPtr->Close(false);
}

int main(int argc, char *argv[])
{
    int port_http = DEFAULT_HTTP_PORT;
    int port_ws = DEFAULT_WS_PORT;
    std::string http_protocol = DEFAULT_HTTP_PROTOCOL;
    std::string ws_protocol = DEFAULT_WS_PROTOCOL;

    auto cmdline = CommandLine::Parse(argc, argv);

    if(cmdline.Exists("-h"))
    {
        cmdline.PrintUsage(true, true);
        exit(0);
    }

    if(cmdline.Exists("-v"))
    {
        WebCpp::DebugPrint::AllowPrint = true;
    }

    int v;
    if(StringUtil::String2int(cmdline.Get("-ph"), v))
    {
        port_http = v;
    }
    if(StringUtil::String2int(cmdline.Get("-pw"), v))
    {
        port_ws = v;
    }

    cmdline.Set("-rh", http_protocol);
    cmdline.Set("-rw", ws_protocol);

    signal(SIGINT, handle_sigint);

    int connID = (-1);
    int min = 1,max = 100;
    WebCpp::HttpServer httpServer;
    WebCpp::WebSocketServer wsServer;
    httpServerPtr = &httpServer;
    wsServerPtr = &wsServer;

    WebCpp::HttpConfig config;
    config.SetRoot(PUB);
    config.SetHttpProtocol(http_protocol);
    config.SetHttpServerPort(port_http);
    config.SetWsProtocol(ws_protocol);
    config.SetWsServerPort(port_ws);
    config.SetSslSertificate(SSL_CERT);
    config.SetSslKey(SSL_KEY);

    if(httpServer.Init(config))
    {
        WebCpp::DebugPrint() << "WebSocket ping-pong test server" << std::endl;
        WebCpp::DebugPrint() << "HTTP server Init(): ok " << std::endl;
        httpServer.OnGet("/[{file}]", [](const WebCpp::Request &request, WebCpp::Response &response) -> bool
        {
            std::string file = request.GetArg("file");
            bool retval = false;
            WebCpp::DebugPrint() << "OnGet(), file: " << file << std::endl;

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
    else
    {
        WebCpp::DebugPrint() << "HTTP server Init() failed: " << httpServer.GetLastError() << std::endl;
    }

    if(wsServer.Init(config))
    {
        WebCpp::DebugPrint() << "WS server Init(): ok " << std::endl;
        wsServer.OnMessage("/ws", [&](const WebCpp::Request &request, WebCpp::ResponseWebSocket &response, const ByteArray &data) -> bool
        {
            response.WriteText(data);
            return true;
        });

        wsServer.Run();
    }
    else
    {
        WebCpp::DebugPrint() << "WS server Init() failed: " << wsServer.GetLastError() << std::endl;
    }

    if(httpServer.IsRunning() && wsServer.IsRunning())
    {
        WebCpp::DebugPrint() << "Starting... Press Ctrl-C to terminate" << std::endl;
        httpServer.WaitFor();
        wsServer.WaitFor();
    }

    return 0;
}
