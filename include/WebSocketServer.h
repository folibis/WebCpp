#ifdef WITH_WEBSOCKET

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

#ifndef WEBSOCKETSERVER_H
#define WEBSOCKETSERVER_H

#include <memory>
#include <deque>
#include "HttpConfig.h"
#include "RouteHttp.h"
#include "RouteWebSocket.h"
#include "IError.h"
#include "IRunnable.h"
#include "Request.h"
#include "ICommunicationServer.h"
#include "ResponseWebSocket.h"


namespace WebCpp
{

class WebSocketServer: public IError, public IRunnable
{
public:
    enum class Protocol
    {
        Undefined = 0,
        WS,
        WSS,
    };
    WebSocketServer();
    virtual ~WebSocketServer();
    WebSocketServer(const WebSocketServer& other) = delete;
    WebSocketServer& operator=(const WebSocketServer& other) = delete;
    WebSocketServer(WebSocketServer&& other) = delete;
    WebSocketServer& operator=(WebSocketServer&& other) = delete;

    bool Init() override;
    bool Init(WebCpp::HttpConfig config);
    bool Run() override;
    bool Close(bool wait = true) override;
    bool WaitFor() override;

    void OnRequest(const std::string &path, const RouteHttp::RouteFunc &func);
    void OnMessage(const std::string &path, const std::function<bool(const Request& request, ResponseWebSocket &response, const ByteArray &data)>& func);

    Protocol GetProtocol() const;

    static Protocol String2Protocol(const std::string &str);
    static std::string Protocol2String(Protocol protocol);

    std::string ToString() const;

protected:
    struct RequestData
    {
        RequestData(int connID, const std::string &remote, const HttpConfig &config)
        {
            this->connID= connID;
            readyForDispatch = false;
            handshake = false;
            request.SetConnectionID(connID);
            request.SetConfig(config);
            request.GetHeader().SetRemote(remote);
        }

        int connID;
        Request request;
        ByteArray data;
        ByteArray encodedData;
        bool handshake;
        bool readyForDispatch;
    };

    void OnConnected(int connID, const std::string& remote);
    void OnDataReady(int connID, std::vector<char> &data);
    void OnClosed(int connID);

    static void* RequestThreadWrapper(void *ptr);
    void* RequestThread();
    void SendSignal();
    void WaitForSignal();
    void InitConnection(int connID, const std::string &remote);
    void PutToQueue(int connID, ByteArray &data);

    bool IsQueueEmpty();
    bool CheckDataFullness();
    void ProcessRequests();
    void RemoveFromQueue(int connID);
    bool ProcessRequest(Request &request);
    bool CheckWsHeader(RequestData& requestData);
    bool CheckWsFrame(RequestData &requestData);
    bool ProcessWsRequest(Request &request, const ByteArray &data);
    //bool IsRouteExist(const std::string &path);
    RouteWebSocket* GetRoute(const std::string &path);

private:
    std::shared_ptr<ICommunicationServer> m_server = nullptr;
    Protocol m_protocol = Protocol::Undefined;
    pthread_t m_requestThread;
    pthread_mutex_t m_queueMutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t m_signalMutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t m_signalCondition = PTHREAD_COND_INITIALIZER;
    bool m_requestThreadRunning = false;
    std::deque<RequestData> m_requestQueue;
    HttpConfig m_config;
    //RouteHttp::RouteFunc m_webSocketRequest = nullptr;
    std::vector<RouteWebSocket> m_routes;
    //std::function<bool(const Request& request, ResponseWebSocket &response, const ByteArray& data)> m_dataFunc;
};

}

#endif // WEBSOCKETSERVER_H

#endif
