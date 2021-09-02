#include <signal.h>
#include "common.h"
#include "HttpServer.h"
#include "WebSocketServer.h"
#include "Request.h"
#include <string>
#include <iostream>
#include "StringUtil.h"
#include "Data.h"
#include "ResponseWebSocket.h"
#include "FcgiClient.h"
#include "FileSystem.h"

#define FPM "/run/php/php7.4-fpm.sock"


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
        httpServer.OnGet("/*.php", [&](const WebCpp::Request &request, WebCpp::Response &response) -> bool
        {
            bool retval = false;

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
        httpServer.WaitFor();
        return 0;
    }

    return 1;
}
