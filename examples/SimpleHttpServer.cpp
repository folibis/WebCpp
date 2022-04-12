/*
*
* Copyright (c) 2021 ruslan@muhlinin.com
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
*/

/*
 * SimpleHttpServer - a simple HTTP server that response for each GET request
 * with a message
*/

#include <csignal>
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
    WebCpp::Http::Protocol http_protocol = DEFAULT_HTTP_PROTOCOL;

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

    std::string s;
    if(cmdline.Set("-rh", s) == true)
    {
        http_protocol = WebCpp::Http::String2Protocol(s);
    }

    signal(SIGINT, handle_sigint);

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
