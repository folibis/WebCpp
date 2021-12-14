#include <signal.h>
#include "common_webcpp.h"
#include "HttpServer.h"
#include "FcgiClient.h"
#include "example_common.h"
#include "DebugPrint.h"

#define FPM "/run/php/php7.4-fpm.sock"


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

    WebCpp::FcgiClient fcgi(FPM, config);
    if(fcgi.Init())
    {
        fcgi.SetParam(WebCpp::FcgiClient::FcgiParam::QUERY_STRING, "QUERY_STRING");
        fcgi.SetParam(WebCpp::FcgiClient::FcgiParam::REQUEST_METHOD, "REQUEST_METHOD");
        fcgi.SetParam(WebCpp::FcgiClient::FcgiParam::SCRIPT_FILENAME, "SCRIPT_FILENAME");
        fcgi.SetParam(WebCpp::FcgiClient::FcgiParam::SCRIPT_NAME, "SCRIPT_NAME");
        fcgi.SetParam(WebCpp::FcgiClient::FcgiParam::PATH_INFO, "PATH_INFO");
        fcgi.SetParam(WebCpp::FcgiClient::FcgiParam::REQUEST_URI, "REQUEST_URI");
        fcgi.SetParam(WebCpp::FcgiClient::FcgiParam::DOCUMENT_ROOT, "DOCUMENT_ROOT");
        fcgi.SetParam(WebCpp::FcgiClient::FcgiParam::SERVER_PROTOCOL, "SERVER_PROTOCOL");
        fcgi.SetParam(WebCpp::FcgiClient::FcgiParam::GATEWAY_INTERFACE, "GATEWAY_INTERFACE");
        fcgi.SetParam(WebCpp::FcgiClient::FcgiParam::REMOTE_ADDR, "REMOTE_ADDR");
        fcgi.SetParam(WebCpp::FcgiClient::FcgiParam::REMOTE_PORT, "REMOTE_PORT");
        fcgi.SetParam(WebCpp::FcgiClient::FcgiParam::SERVER_ADDR, "SERVER_ADDR");
        fcgi.SetParam(WebCpp::FcgiClient::FcgiParam::SERVER_PORT, "SERVER_PORT");
        fcgi.SetParam(WebCpp::FcgiClient::FcgiParam::SERVER_NAME, "SERVER_NAME");

        fcgi.SetOnResponseCallback([&](WebCpp::Response &response) {
            httpServer.SendResponse(response);
        });
    }

    if(httpServer.Init(config))
    {
        WebCpp::DebugPrint() << "HTTP fcgi server" << std::endl;
        WebCpp::DebugPrint() << "Note: php-fcgi server must be run" << std::endl;
        httpServer.OnGet("/*.php", [&](const WebCpp::Request &request, WebCpp::Response &response) -> bool
        {
            bool retval = false;
            WebCpp::DebugPrint() << "OnGet(*.php), sends fcgi request..." << std::endl;

            retval = fcgi.SendRequest(request);
            if(retval == false)
            {
                response.SendNotFound();
            }
            else
            {
                response.SetShouldSend(false);
            }
            return true;
        });

        httpServer.OnGet("/[{file}]", [](const WebCpp::Request &request, WebCpp::Response &response) -> bool
        {
            std::string file = request.GetArg("file");
            bool retval = false;
            WebCpp::DebugPrint() << "OnGet(), file: " << file << std::endl;

            if(file.empty())
            {
                retval = response.AddFile("fcgi.html");
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
        WebCpp::DebugPrint() << "Starting... Press Ctrl-C to terminate" << std::endl;
        httpServer.WaitFor();
        return 0;
    }
    else
    {
        WebCpp::DebugPrint() << "HTTP server Init() failed: " << httpServer.GetLastError() << std::endl;
    }

    return 1;
}
