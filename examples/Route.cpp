#include <signal.h>
#include "common_webcpp.h"
#include "HttpServer.h"
#include "example_common.h"
#include "DebugPrint.h"


static WebCpp::HttpServer *httpServerPtr = nullptr;

void handle_sigint(int)
{
    httpServerPtr->Close(false);
}

int main(int argc, char *argv[])
{
    signal(SIGINT, handle_sigint);

    int port_http = DEFAULT_HTTP_PORT;
    std::string http_protocol = DEFAULT_HTTP_PROTOCOL;

    auto cmdline = CommandLine::Parse(argc, argv);

    if(cmdline.Exists("-h"))
    {
        cmdline.PrintUsage(false, true);
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

    WebCpp::HttpServer httpServer;
    httpServerPtr = &httpServer;

    WebCpp::HttpConfig config;
    config.SetRoot(PUB);
    config.SetHttpProtocol(http_protocol);
    config.SetHttpServerPort(port_http);
    config.SetSslSertificate(SSL_CERT);
    config.SetSslKey(SSL_KEY);

    if(httpServer.Init(config))
    {
        WebCpp::DebugPrint() << "HTTP routing test server" << std::endl;
        httpServer.OnGet("/[{file}]", [](const WebCpp::Request &request, WebCpp::Response &response) -> bool
        {
            std::string file = request.GetArg("file");
            bool retval = false;

            WebCpp::DebugPrint() << "OnGet(), file: " << file << std::endl;

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

            WebCpp::DebugPrint() << "OnGet(), route/: param1: " << param1 << ", param2: " << param2 << ",param3: " << param3 << std::endl;

            response.AddHeader("Content-Type","text/plain;charset=utf-8");
            response.Write("param1: " + param1 + ", param2: " + param2 + ",param3: " + param3);
            return true;
        });

        httpServer.Run();
        WebCpp::DebugPrint() << "Starting... Press Ctrl-C to terminate" << std::endl;
        httpServer.WaitFor();
    }
    else
    {
        WebCpp::DebugPrint() << "HTTP server Init() failed: " << httpServer.GetLastError() << std::endl;
    }

    return 1;
}
