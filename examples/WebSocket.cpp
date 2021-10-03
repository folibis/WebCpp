#include <signal.h>
#include "common_webcpp.h"
#include "defines_webcpp.h"
#include "HttpServer.h"
#include "WebSocketServer.h"
#include "Request.h"
#include <string>
#include "StringUtil.h"
#include "ResponseWebSocket.h"
#include "FcgiClient.h"
#include "StringUtil.h"
#include "PeriodicalTask.h"


static WebCpp::HttpServer *httpServerPtr = nullptr;
static WebCpp::WebSocketServer *wsServerPtr = nullptr;

void handle_sigint(int)
{
    httpServerPtr->Close(false);
    wsServerPtr->Close(false);
}

int main()
{
    signal(SIGINT, handle_sigint);

    int connID = (-1);
    int min = 1,max = 100;
    WebCpp::HttpServer httpServer;
    WebCpp::WebSocketServer wsServer;
    httpServerPtr = &httpServer;
    wsServerPtr = &wsServer;
    WebCpp::PeriodicalTask task;
    task.SetFunction([&](bool *running) -> void*
    {
        WebCpp::ResponseWebSocket response(connID);
        StringUtil::RandInit();
        while(*running)
        {
            int num = StringUtil::GetRand(min, max);
            ByteArray data = StringUtil::String2ByteArray(std::to_string(num));
            response.WriteText(data);
            wsServer.SendResponse(response);
            sleep(1);
        }

        return nullptr;
    });

    WebCpp::HttpConfig config;
    config.SetRoot(PUBLIC_DIR);
    config.SetHttpProtocol("HTTP");
    config.SetHttpServerPort(8080);
    config.SetWsProtocol("WS");
    config.SetWsServerPort(8081);

    if(httpServer.Init(config))
    {
        httpServer.OnGet("/[{file}]", [](const WebCpp::Request &request, WebCpp::Response &response) -> bool
        {
            std::string file = request.GetArg("file");
            bool retval = false;

            if(file.empty())
            {
                retval = response.AddFile("ws.html");
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
    }

    if(wsServer.Init(config))
    {
        wsServer.OnMessage("/ws", [&](const WebCpp::Request &request, WebCpp::ResponseWebSocket &response, const ByteArray &data) -> bool
        {
            std::string d = StringUtil::ByteArray2String(data);
            switch(_(d.c_str()))
            {
                case _("start"):
                    connID = request.GetConnectionID();
                    task.Start();
                    break;
                case _("stop"):
                    task.Stop();
                    break;
                default:
                    {
                        auto arr = StringUtil::Split(d,',');
                        if(arr.size() == 2)
                        {
                            int i = 0;
                            if(StringUtil::String2int(arr[0],i))
                            {
                                min = i;
                            }
                            if(StringUtil::String2int(arr[1],i))
                            {
                                max = i;
                            }
                        }
                    }
                    break;
            }

            return true;
        });

        wsServer.Run();
    }

    httpServer.WaitFor();
    wsServer.WaitFor();

    return 0;
}
