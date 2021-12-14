#include <signal.h>
#include "common_webcpp.h"
#include "HttpServer.h"
#include "Request.h"
#include <iostream>
#include "StringUtil.h"
#include "Data.h"
#include "FileSystem.h"
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
        WebCpp::DebugPrint() << "HTTP POST server" << std::endl;
        httpServer.OnGet("/[{file}]", [](const WebCpp::Request &request, WebCpp::Response &response) -> bool
        {
            bool retval = false;
            std::string file = request.GetArg("file");
            WebCpp::DebugPrint() << "OnGet(), file: " << file << std::endl;

            if(!file.empty())
            {
                retval = response.AddFile(file);
            }
            else
            {
                retval = response.AddFile("form.html");
            }

            if(retval == false)
            {
                response.SendNotFound();
            }

            return retval;
        });

        httpServer.OnPost("/form", [](const WebCpp::Request &request, WebCpp::Response &response) -> bool
        {
            bool retval = true;

            auto &body = request.GetRequestBody();
            auto file1 = body.GetValue("file1");
            auto file2 = body.GetValue("file2");
            WebCpp::DebugPrint() << "OnPost()" << std::endl;

            response.AddHeader("Content-Type","text/html;charset=utf-8");
            response.Write("<div>Hello, " + body.GetValue("name").GetDataString() + " " + body.GetValue("surname").GetDataString() + "</div>");
            response.Write("<div>file 1 '" + file1.fileName + "'" + " with length: " + std::to_string(file1.data.size()) + " and mimetype: '" + file1.contentType + "' was successfully uploaded</div>");
            response.Write("<div>file 2 '" + file2.fileName + "'" + " with length: " + std::to_string(file2.data.size()) + " and mimetype: '" + file2.contentType + "' was successfully uploaded</div>");

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
