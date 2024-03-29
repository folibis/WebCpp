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
 * FileServer - a simple implementation of a file server that provides access to the
 * appropriade directory using simple folders/files list
*/

#include <csignal>
#include "common_webcpp.h"
#include "HttpServer.h"
#include "Request.h"
#include <iostream>
#include "StringUtil.h"
#include "FileSystem.h"
#include "example_common.h"
#include "DebugPrint.h"


static WebCpp::HttpServer *ptr = nullptr;

static const char* pageTpl =
#include "templates/fileserverpage.txt"
;
static const char* tableTpl =
#include "templates/fileservertable.txt"
;
static const char* rowTpl =
#include "templates/fileservertablerow.txt"
;

void handle_sigint(int)
{
    if(ptr != nullptr)
    {
        ptr->Close(false);
    }
}

int main(int argc, char *argv[])
{
    WebCpp::HttpServer httpServer;
    ptr = &httpServer;

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

    WebCpp::HttpConfig &config = WebCpp::HttpConfig::Instance();
    config.SetHttpProtocol(http_protocol);
    config.SetHttpServerPort(port_http);
    config.SetSslSertificate(SSL_CERT);
    config.SetSslKey(SSL_KEY);

    if(httpServer.Init())
    {
        WebCpp::DebugPrint() << "HTTP file server" << std::endl;
        httpServer.OnGet("/*", [&](const WebCpp::Request &request, WebCpp::Response &response) -> bool
        {
            std::string root = WebCpp::FileSystem::NormalizePath("~/");
            std::string url = request.GetUrl().GetPath();
            std::string local = (url == "/") ? root : (root + url);
            std::string parent = "";
            auto pos = url.rfind(WebCpp::FileSystem::PathDelimiter(), url.size() - 2);
            if(pos != std::string::npos)
            {
                parent = std::string(url.begin(), url.begin() + pos + 1);
            }

            WebCpp::DebugPrint() << "OnGet(), url: " << url << std::endl;

            if(WebCpp::FileSystem::IsFileExist(local))
            {
                if(WebCpp::FileSystem::IsDir(local))
                {
                    std::string table = tableTpl;
                    StringUtil::Replace(table, "[folder]", request.GetUrl().GetPath());

                    if(!parent.empty())
                    {
                        StringUtil::Replace(table, "[backlink]", WebCpp::FileSystem::NormalizePath(parent));
                    }
                    else
                    {
                        StringUtil::Replace(table, "[backlink]", "#");
                    }


                    std::string rows = "";
                    auto list = WebCpp::FileSystem::GetFolder(local);
                    for(auto &entry: list)
                    {
                        std::string file = entry.name;
                        if(file == "." || file == "..")
                        {
                            continue;
                        }

                        std::string row = rowTpl;
                        StringUtil::Replace(row, "[link]", WebCpp::FileSystem::NormalizePath(request.GetUrl().GetPath()) + file);
                        StringUtil::Replace(row, "[name]", entry.folder ? ("<strong>" + file + "/</strong>") : file);
                        StringUtil::Replace(row, "[size]", std::to_string(entry.size));
                        StringUtil::Replace(row, "[modified]", entry.lastModified);
                        rows += row;
                    }
                    StringUtil::Replace(table, "[rows]", rows);
                    std::string page = pageTpl;
                    StringUtil::Replace(page, "[body]", table);

                    response.AddHeader("Content-Type","text/html;charset=utf-8");
                    response.Write(page);
                }
                else
                {
                    response.AddFile(local);
                }
            }
            else
            {
                response.NotFound();
            }

            return true;
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
