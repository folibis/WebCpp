#include <iostream>
#include <unistd.h>
#include <signal.h>
#include "defines_webcpp.h"
#include "common_webcpp.h"
#include "StringUtil.h"
#include "WebSocketClient.h"
#include "ResponseWebSocket.h"


int main(int argc, char *argv[])
{
    std::string address = "ws://127.0.0.1:8081/ws";
    std::string action;

    for(int i = 1;i < argc;i ++)
    {
        switch(_(argv[i]))
        {
            case _("-a"):
                action = "address";
                break;
            default:
                address = argv[i];
                action = "";
                break;
        }
    }

    WebCpp::HttpConfig config;
    config.SetRoot(PUBLIC_DIR);
    config.SetWsProtocol("WS");
    config.SetWsServerPort(8081);

    WebCpp::WebSocketClient wsClient;
    wsClient.SetOnConnect([](bool connected)
    {
        std::cout << "connected: " << (connected ? "true" : "false") << std::endl;
    });

    wsClient.SetOnClose([]()
    {
        std::cout << "closed" << std::endl;
    });

    wsClient.SetOnError([](const std::string &error)
    {
        std::cout << "error: " << error << std::endl;
    });

    wsClient.SetOnMessage([](WebCpp::ResponseWebSocket &response)
    {
        std::cout << "message received: " << StringUtil::ByteArray2String(response.GetData()) << std::endl;
    });

    if(wsClient.Init(config) == false)
    {
        std::cout << "init error: " << wsClient.GetLastError() << std::endl;
        return 1;
    }

    if(wsClient.Open(address) == false)
    {
        std::cout << "open error: " << wsClient.GetLastError() << std::endl;
        return 1;
    }

    wsClient.SendText("Hello!");
    wsClient.WaitFor();
    wsClient.Close();

    return 0;
}
