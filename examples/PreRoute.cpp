#include <signal.h>
#include "common_webcpp.h"
#include "FileSystem.h"
#include "StringUtil.h"
#include "HttpServer.h"


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

    bool authenticated = false;

    if(httpServer.Init(config))
    {
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
                    response.SendNotFound();
                }
            }
            return !authenticated;
        });

        httpServer.OnPost("/auth", [&](const WebCpp::Request &request, WebCpp::Response &response) -> bool
        {
            auto user = request.GetRequestBody().GetValue("user").GetDataString();
            auto password = request.GetRequestBody().GetValue("password").GetDataString();
            if(user == "test" && password == "test")
            {
                authenticated = true;
            }

            response.SendRedirect("/");
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
                    response.SendNotFound();
                }
                else
                {
                    response.AddHeader(WebCpp::HttpHeader::HeaderType::CacheControl,"no-cache");
                }
            }

            return true;
        });

        httpServer.Run();
        httpServer.WaitFor();
    }

    return 1;
}
