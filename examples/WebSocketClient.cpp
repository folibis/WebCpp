#include <iostream>
#include <unistd.h>
#include <signal.h>
#include "defines_webcpp.h"
#include "common_webcpp.h"
#include "StringUtil.h"
#include "WebSocketClient.h"
#include "ResponseWebSocket.h"
#include "ThreadWorker.h"
#include "example_common.h"
#include "DebugPrint.h"

#define SEND_ATTEMPTS 10
#define SEND_DELAY 1000 // ms.


int main(int argc, char *argv[])
{
    int port_ws = DEFAULT_WS_PORT;
    std::string ws_protocol = DEFAULT_WS_PROTOCOL;

    std::string address = "ws://127.0.0.1:8081/ws";
    auto cmdline = CommandLine::Parse(argc, argv);

    if(cmdline.Exists("-h"))
    {
        cmdline.PrintUsage(false, true, "-a: address to connect");
        exit(0);
    }
    cmdline.Set("-a", address);
    int v;
    if(StringUtil::String2int(cmdline.Get("-pw"), v))
    {
        port_ws = v;
    }
    cmdline.Set("-rw", ws_protocol);

    WebCpp::HttpConfig config;
    config.SetRoot(PUB);
    config.SetWsProtocol(ws_protocol);
    config.SetWsServerPort(port_ws);

    WebCpp::WebSocketClient wsClient;
    WebCpp::ThreadWorker task;
    int counter = 0;

    wsClient.SetOnConnect([&wsClient, &task](bool connected)
    {
        WebCpp::DebugPrint() << "connected: " << (connected ? "true" : "false") << std::endl;
        task.Start();
    });

    wsClient.SetOnClose([]()
    {
        WebCpp::DebugPrint() << "closed" << std::endl;
    });

    wsClient.SetOnError([](const std::string &error)
    {
        WebCpp::DebugPrint() << "error: " << error << std::endl;
    });

    wsClient.SetOnMessage([&](WebCpp::ResponseWebSocket &response)
    {
        WebCpp::DebugPrint() << "message received: ";
        std::cout << StringUtil::ByteArray2String(response.GetData()) << std::endl << std::endl;
        if(counter >= SEND_ATTEMPTS)
        {
            task.Stop();
            wsClient.Close();
        }
    });

    if(wsClient.Init(config) == false)
    {
        WebCpp::DebugPrint() << "init error: " << wsClient.GetLastError() << std::endl;
        return 1;
    }

    WebCpp::DebugPrint() << "WebSocket test client" << std::endl;

    if(wsClient.Open(address) == false)
    {
        WebCpp::DebugPrint() << "open error: " << wsClient.GetLastError() << std::endl;
        return 1;
    }

    task.SetFunction([&wsClient, &counter](bool *running) -> void*
    {
        while(*running)
        {
            auto str = StringUtil::GenerateRandomString();
            WebCpp::DebugPrint() << "message send:     " << str << std::endl;
            wsClient.SendText(str);
            counter ++;

            usleep(SEND_DELAY * 1000);
        }
        return nullptr;
    });

    WebCpp::DebugPrint() << "Starting... Press Ctrl-C to terminate" << std::endl;
    wsClient.WaitFor();
    wsClient.Close();

    return 0;
}
