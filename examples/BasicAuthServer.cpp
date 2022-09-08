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
 * BasicAuthServer - a simple HTTP server with Basic authentication
 *
*/

#include <csignal>
#include "common_webcpp.h"
#include "defines_webcpp.h"
#include "HttpServer.h"
#include "Request.h"
#include <iostream>
#include "example_common.h"
#include "DebugPrint.h"

#define USER "foo"
#define PASSWORD "bar"


static WebCpp::HttpServer *httpServerPtr = nullptr;

void handle_sigint(int)
{
    httpServerPtr->Close(false);
}

int main(int argc, char *argv[])
{
    signal(SIGINT, handle_sigint);

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

    WebCpp::HttpServer httpServer;
    httpServerPtr = &httpServer;

    WebCpp::HttpConfig &config = WebCpp::HttpConfig::Instance();
    config.SetRoot(PUB);
    config.SetHttpProtocol(http_protocol);
    config.SetHttpServerPort(port_http);
    config.SetSslSertificate(SSL_CERT);
    config.SetSslKey(SSL_KEY);

    if(httpServer.Init())
    {
        WebCpp::DebugPrint() << "HTTP simple test server with Basic auth" << std::endl;
        httpServer.SetAuthHandler([](const WebCpp::Request &request, WebCpp::IAuth *authMethod) -> bool
        {
            WebCpp::DebugPrint() << "authenticated using "
                                 << authMethod->GetSchemeName() << " scheme"
                                 << " with credentials ("
                                 << "user: " << authMethod->GetUser()
                                 << ". password: " << authMethod->GetPassword() << ")"
                                 << std::endl;
            if(authMethod->GetUser() == USER &&
                    authMethod->GetPassword() == PASSWORD)
            {
                return true;
            }

            return false;
        });

        httpServer.OnGet("/[{file}]", [](const WebCpp::Request &request, WebCpp::Response &response) -> bool
        {
            std::string file = request.GetArg("file");
            WebCpp::DebugPrint() << "OnGet(), file: " << file << std::endl;
            response.Write("<h3>Congratulations, you're successfully autenticated!</h3>");

            return true;
        }, true);

        httpServer.Run();
    }
    else
    {
        WebCpp::DebugPrint() << "HTTP server Init() failed: " << httpServer.GetLastError() << std::endl;
    }

    WebCpp::DebugPrint() << "Starting... Press Ctrl-C to terminate" << std::endl;
    httpServer.WaitFor();

    return 0;
}
