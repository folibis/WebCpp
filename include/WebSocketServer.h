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

#ifndef WEBCPP_WEBSOCKETSERVER_H
#define WEBCPP_WEBSOCKETSERVER_H

#include <memory>
#include <deque>
#include "HttpConfig.h"
#include "RouteHttp.h"
#include "RouteWebSocket.h"
#include "IErrorable.h"
#include "IRunnable.h"
#include "Request.h"
#include "ICommunicationServer.h"
#include "RequestWebSocket.h"
#include "ResponseWebSocket.h"
#include "ThreadWorker.h"
#include "Mutex.h"
#include "Signal.h"


namespace WebCpp
{

class WebSocketServer: public IErrorable, public IRunnable
{
public:
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

    bool SendResponse(const ResponseWebSocket &response);

    Http::Protocol GetProtocol() const;
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
        std::vector<RequestWebSocket> requestList;
        bool handshake;
        bool readyForDispatch;
    };

    void OnConnected(int connID, const std::string& remote);
    void OnDataReady(int connID, ByteArray &data);
    void OnClosed(int connID);

    bool StartRequestThread();
    bool StopRequestThread();
    void* RequestThread(bool &running);

    void SendSignal();
    void WaitForSignal();
    void InitConnection(int connID, const std::string &remote);
    void PutToQueue(int connID, ByteArray &data);

    bool IsQueueEmpty();
    bool CheckData();
    void ProcessRequests();
    void RemoveFromQueue(int connID);
    bool ProcessRequest(Request &request);
    bool CheckWsHeader(RequestData& requestData);
    bool CheckWsFrame(RequestData &requestData);
    bool ProcessWsRequest(Request &request, const RequestWebSocket &wsRequest);
    RouteWebSocket* GetRoute(const std::string &path);

private:
    std::shared_ptr<ICommunicationServer> m_server = nullptr;
    Http::Protocol m_protocol = Http::Protocol::Undefined;
    ThreadWorker m_requestThread;
    Mutex m_queueMutex;
    Mutex m_signalMutex;
    Mutex m_requestMutex;
    Signal m_signalCondition;
    std::deque<RequestData> m_requestQueue;
    HttpConfig m_config;
    std::vector<RouteWebSocket> m_routes;
};

}

#endif // WEBCPP_WEBSOCKETSERVER_H

#endif
