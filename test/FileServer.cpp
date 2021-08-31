#include <signal.h>
#include "common.h"
#include "HttpServer.h"
#include "Request.h"
#include <string>
#include <iostream>
#include "StringUtil.h"
#include "FileSystem.h"


static WebCpp::HttpServer *ptr = nullptr;

void handle_sigint(int)
{
    ptr->Close(false);
}

int main()
{
    WebCpp::HttpServer httpServer;
    ptr = &httpServer;

    signal(SIGINT, handle_sigint);

    WebCpp::HttpConfig config;
    config.SetRoot(PUBLIC_DIR);
    config.SetHttpProtocol("HTTP");
    config.SetHttpServerPort(8080);

    bool httpServerRun = false;

    if(httpServer.Init(config))
    {
        httpServer.OnGet("/*", [&](const WebCpp::Request &request, WebCpp::Response &response) -> bool
        {
            std::string root = WebCpp::FileSystem::NormalizePath(getenv("HOME"));
            std::string url = request.GetHeader().GetPath();
            std::string local = (url == "/") ? root : WebCpp::FileSystem::NormalizePath(root + url);
            std::string parent = "";
            auto pos = url.rfind(WebCpp::FileSystem::PathDelimiter());
            if(pos != std::string::npos)
            {
                parent = std::string(url.begin(), url.begin() + pos);
            }
            if(WebCpp::FileSystem::IsFileExist(local))
            {
                if(WebCpp::FileSystem::IsDir(local))
                {
                    auto list = WebCpp::FileSystem::GetFolder(local);
                    std::string content = "<h4>" + request.GetHeader().GetPath() + "</h4><hr>";
                    if(!parent.empty())
                    {
                        content += "<p><a href=\"" + WebCpp::FileSystem::NormalizePath(parent) + "\">Go parent</a></div>";
                    }
                    for(auto &entry: list)
                    {
                        std::string file = entry.first;
                        if(file == "." || file == "..")
                        {
                            continue;
                        }
                        content += "<div><a href=\"" +
                                (WebCpp::FileSystem::NormalizePath(request.GetHeader().GetPath()) + file) + "\">" +
                                (entry.second ? ("<strong>" + file + "/</strong>") : file) + "</a></div>";
                    }
                    response.SetHeader("Content-Type","text/html;charset=utf-8");
                    response.Write(content);
                }
                else
                {
                    response.AddFile(url);
                }
            }
            else
            {
                response.SendNotFound();
            }

            return true;
        });


        httpServer.Run();
        httpServer.WaitFor();

        return 0;
    }

    return 1;
}
