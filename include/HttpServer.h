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

#ifndef WEBCPP_HTTPSERVER_H
#define WEBCPP_HTTPSERVER_H

#include <ICommunicationServer.h>
#include "common_webcpp.h"
#include <deque>
#include <vector>
#include <memory>
#include "IErrorable.h"
#include "IRunnable.h"
#include "ThreadWorker.h"
#include "Mutex.h"
#include "Signal.h"
#include "Session.h"
#include "Request.h"
#include "Response.h"
#include "RouteHttp.h"
#include "HttpConfig.h"
#include "HttpHeader.h"


namespace WebCpp
{

class HttpServer: public IErrorable, public IRunnable
{
public:
    HttpServer();
    HttpServer(const HttpServer& other) = delete;
    HttpServer& operator=(const HttpServer& other) = delete;
    HttpServer(HttpServer&& other) = delete;
    HttpServer& operator=(HttpServer&& other) = delete;

    bool Init() override;
    bool Init(const WebCpp::HttpConfig& config);
    bool Run() override;
    bool Close(bool wait = true) override;
    bool WaitFor() override;

    HttpServer& OnGet(const std::string &path, const RouteHttp::RouteFunc &f);
    HttpServer& OnPost(const std::string &path, const RouteHttp::RouteFunc &f);

    void SetPreRouteFunc(const RouteHttp::RouteFunc &callback);
    void SetPostRouteFunc(const RouteHttp::RouteFunc &callback);

    bool SendResponse(Response &response);

    std::string ToString() const;

protected:
    void OnConnected(int connID, const std::string& remote);
    void OnDataReady(int connID, ByteArray &data);
    void OnClosed(int connID);

    bool StartRequestThread();
    bool StopRequestThread();
    void* RequestThread(bool &running);

    void SendSignal();
    void WaitForSignal();
    void PutToQueue(int connID, const std::string &remote);
    void AppendData(int connID, const ByteArray &data);
    bool IsQueueEmpty();
    bool CheckDataFullness();
    std::unique_ptr<Request> GetNextRequest();
    void RemoveFromQueue(int connID);

    void ProcessRequest(Request &request);
    void ProcessKeepAlive(int connID);    

private:
    std::shared_ptr<ICommunicationServer> m_server = nullptr;
    Http::Protocol m_protocol = Http::Protocol::Undefined;
    Session m_sessions;
    ThreadWorker m_requestThread;
    Mutex m_queueMutex;
    Mutex m_signalMutex;
    Signal m_signalCondition;
    std::vector<RouteHttp> m_routes;
    HttpConfig &m_config;
    RouteHttp::RouteFunc m_preRoute = nullptr;
    RouteHttp::RouteFunc m_postRoute = nullptr;
};

}

#endif // WEBCPP_HTTPSERVER_H
