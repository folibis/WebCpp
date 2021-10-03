#include <signal.h>
#include "common_webcpp.h"
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

    if(httpServer.Init(config))
    {
        httpServer.OnGet("/[{file}]", [](const WebCpp::Request &request, WebCpp::Response &response) -> bool
        {
            bool retval = false;
            std::string file = request.GetArg("file");
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

            response.AddHeader("Content-Type","text/html;charset=utf-8");
            response.Write("<div>Hello, " + body.GetValue("name").GetDataString() + " " + body.GetValue("surname").GetDataString() + "</div>");
            response.Write("<div>file 1 '" + file1.fileName + "'" + " with length: " + std::to_string(file1.data.size()) + " and mimetype: '" + file1.contentType + "' was successfully uploaded</div>");
            response.Write("<div>file 2 '" + file2.fileName + "'" + " with length: " + std::to_string(file2.data.size()) + " and mimetype: '" + file2.contentType + "' was successfully uploaded</div>");

            return retval;
        });

        httpServer.Run();
        httpServer.WaitFor();
        return 0;
    }

    return 1;
}
