#include <signal.h>
#include "common_webcpp.h"
#include "HttpServer.h"
#include "example_common.h"
#include "DebugPrint.h"


static WebCpp::HttpServer *httpServerPtr;

void handle_sigint(int)
{
    httpServerPtr->Close(false);
}

int main(int argc, char *argv[])
{
    int port_http = DEFAULT_HTTP_PORT;
    std::string http_protocol = DEFAULT_HTTP_PROTOCOL;

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

    cmdline.Set("-rh", http_protocol);

    signal(SIGINT, handle_sigint);

    WebCpp::HttpServer httpServer;
    httpServerPtr = &httpServer;

    WebCpp::HttpConfig config;
    config.SetRoot(PUB);
    config.SetHttpProtocol(http_protocol);
    config.SetHttpServerPort(port_http);

    WebCpp::DebugPrint() << config.ToString() << std::endl;

    if(httpServer.Init(config))
    {
        WebCpp::DebugPrint() << "Simple HTTP static test server" << std::endl;
        httpServer.OnGet("/", [](const WebCpp::Request &, WebCpp::Response &response) -> bool
        {
            WebCpp::DebugPrint() << "OnGet()" << std::endl;
            response.AddHeader("Content-Type","text/html;charset=utf-8");
            response.Write("<h3>WebCpp works!</h3>");

            return true;
        });

        WebCpp::DebugPrint() << "Starting... Press Ctrl-C to terminate" << std::endl;
        httpServer.Run();
        httpServer.WaitFor();
    }
    else
    {
        WebCpp::DebugPrint() << "HTTP server Init() failed: " << httpServer.GetLastError() << std::endl;
    }

    return 1;
}
