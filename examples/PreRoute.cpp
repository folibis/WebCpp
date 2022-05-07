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
 * PreRoute - a simple HTTP server that uses a pre-routing handler
 * for authentication purposes.
*/

#include <csignal>
#include "common_webcpp.h"
#include "FileSystem.h"
#include "HttpServer.h"
#include "example_common.h"
#include "DebugPrint.h"

#define DEFAULT_USER "test"
#define DEFAULT_PASSWORD "test"


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

    bool authenticated = false;

    if(httpServer.Init())
    {
        WebCpp::DebugPrint() << "HTTP prerouting test server" << std::endl;

        httpServer.SetPreRouteFunc([&](const WebCpp::Request &request, WebCpp::Response &response) -> bool
        {
            // skip the common files
            auto ext = WebCpp::FileSystem::ExtractFileExtension(request.GetUrl().GetPath());
            if(ext == "css" || ext == "js")
            {
                return false;
            }

            // skip the form POST
            if(request.GetMethod() == WebCpp::Http::Method::POST &&
                    request.GetUrl().GetPath() == "/auth")
            {
                return false;
            }

            bool retval = false;
            if(authenticated == false)
            {
                retval = response.AddFile("auth.html");
                if(retval == false)
                {
                    response.NotFound();
                }
            }
            return !authenticated;
        });

        httpServer.OnPost("/auth", [&](const WebCpp::Request &request, WebCpp::Response &response) -> bool
        {
            auto user = request.GetRequestBody().GetValue("user").GetDataString();
            auto password = request.GetRequestBody().GetValue("password").GetDataString();
            if(user == DEFAULT_USER && password == DEFAULT_PASSWORD)
            {
                authenticated = true;
            }

            response.Redirect("/");
            return true;
        });

        httpServer.OnGet("/[{file}]", [](const WebCpp::Request &request, WebCpp::Response &response) -> bool
        {
            std::string file = request.GetArg("file");
            bool retval = false;

            if(file.empty())
            {
                response.AddHeader("Content-Type","text/html;charset=utf-8");
                response.Write("<h3>Congratulations! You've accessed the secured area!</h3>");
            }
            else
            {
                retval = response.AddFile(file);
                if(retval == false)
                {
                    response.NotFound();
                }
                else
                {
                    response.AddHeader(WebCpp::HttpHeader::HeaderType::CacheControl,"no-cache");
                }
            }

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
