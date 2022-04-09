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
 * WebSocketClient - a simple WebSocket client that connects
 * to a specified WebSocket server and prints out some debug info
*/

#include <unistd.h>
#include <csignal>
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
    WebCpp::Http::Protocol ws_protocol = DEFAULT_WS_PROTOCOL;

    std::string address = "ws://127.0.0.1:8081/ws";
    auto cmdline = CommandLine::Parse(argc, argv);

    if(cmdline.Exists("-h"))
    {
        std::vector<std::string> adds;
        adds.push_back("-a: address to connect");
        cmdline.PrintUsage(false, true, adds);
        exit(0);
    }
    cmdline.Set("-a", address);
    int v;
    if(StringUtil::String2int(cmdline.Get("-pw"), v))
    {
        port_ws = v;
    }
    std::string s;
    cmdline.Set("-rw", s);
    if(!s.empty())
    {
        ws_protocol = WebCpp::Http::String2Protocol(s);
    }

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

    task.SetFunction([&wsClient, &counter](bool &running) -> void*
    {
        while(running)
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
