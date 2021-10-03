#include <signal.h>
#include "common_webcpp.h"
#include "HttpServer.h"
#include "Request.h"
#include <string>
#include <iostream>
#include "StringUtil.h"
#include "FileSystem.h"


static WebCpp::HttpServer *ptr = nullptr;

static const char* pageTpl = "<html> \
        <head> \
        <title>File server</title> \
        <meta content=\"text/html;charset=utf-8\" http-equiv=\"Content-Type\"> \
        <meta content=\"utf-8\" http-equiv=\"encoding\"> \
        <style> \
        .row span { display: inline-block; padding:5px 10px; width:300px; } \
        </style> \
        </head> \
        <body> \
        [body] \
        </body> \
        </html>";

static const char* tableTpl = "<h4>Index of: [folder]</h4><hr><div><a href='[backlink]'>Go parent</a></div><div>[rows]</div>";
static const char* rowTpl = "<div class='row'><span><a href='[link]'>[name]</a></span><span>[size]</span><span>[modified]</span></div>";

void handle_sigint(int)
{
    if(ptr != nullptr)
    {
        ptr->Close(false);
    }
}

int main()
{
    WebCpp::HttpServer httpServer;
    ptr = &httpServer;

    signal(SIGINT, handle_sigint);

    WebCpp::HttpConfig config;
    config.SetHttpProtocol("HTTP");
    config.SetHttpServerPort(8080);

    if(httpServer.Init(config))
    {
        httpServer.OnGet("/*", [&](const WebCpp::Request &request, WebCpp::Response &response) -> bool
        {
            std::string root = WebCpp::FileSystem::NormalizePath(getenv("HOME"));
            std::string url = request.GetUrl().GetPath();
            std::string local = (url == "/") ? root : (root + url);
            std::string parent = "";
            auto pos = url.rfind(WebCpp::FileSystem::PathDelimiter(), url.size() - 2);
            if(pos != std::string::npos)
            {
                parent = std::string(url.begin(), url.begin() + pos + 1);
            }
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
