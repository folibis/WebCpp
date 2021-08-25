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

#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <ICommunicationServer.h>
#include "common.h"
#include <deque>
#include <vector>
#include <memory>
#include <pthread.h>
#include "IError.h"
#include "IRunnable.h"
#include "Request.h"
#include "Response.h"
#include "RouteHttp.h"
#include "HttpConfig.h"
#include "HttpHeader.h"


namespace WebCpp
{

class HttpServer: public IError, public IRunnable
{
public:
    enum class Protocol
    {
        Undefined = 0,
        HTTP,
        HTTPS,
    };

    HttpServer();
    HttpServer(const HttpServer&other) = delete;
    HttpServer& operator=(const HttpServer& other) = delete;
    HttpServer(HttpServer&& other) = delete;
    HttpServer& operator=(HttpServer&& other) = delete;

    bool Init(WebCpp::HttpConfig config);
    bool Run() override;
    bool Close(bool wait = true) override;
    bool WaitFor() override;

    HttpServer& OnGet(const std::string &path, const RouteHttp::RouteFunc &f);
    HttpServer& OnPost(const std::string &path, const RouteHttp::RouteFunc &f);

    void SetPreRouteFunc(const RouteHttp::RouteFunc &callback);
    void SetPostRouteFunc(const RouteHttp::RouteFunc &callback);


    Protocol GetProtocol() const;

    static Protocol String2Protocol(const std::string &str);
    static std::string Protocol2String(Protocol protocol);

    std::string ToString() const;

protected:
    void OnConnected(int connID, const std::string& remote);
    void OnDataReady(int connID, std::vector<char> &data);
    void OnClosed(int connID);

    static void* RequestThreadWrapper(void *ptr);
    void* RequestThread();

    void SendSignal();
    void WaitForSignal();
    void PutToQueue(int connID, ByteArray &data);
    bool IsQueueEmpty();
    bool CheckDataFullness();
    Request GetNextRequest();
    void RemoveFromQueue(int connID);
    void ProcessRequest(Request &request);
    void ProcessKeepAlive(int connID);    

private:
    struct RequestData
    {
        RequestData(int connID, ByteArray& data)
        {
            this->connID= connID;
            this->data = std::move(data);
            readyForDispatch = false;
        }
        int connID;
        ByteArray data;
        HttpHeader header;
        bool readyForDispatch;
    };

    std::shared_ptr<ICommunicationServer> m_server = nullptr;
    Protocol m_protocol = Protocol::Undefined;

    pthread_t m_requestThread;

    pthread_mutex_t m_queueMutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t m_signalMutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t m_signalCondition = PTHREAD_COND_INITIALIZER;

    bool m_requestThreadRunning = false;
    std::deque<RequestData> m_requestQueue;
    std::vector<RouteHttp> m_routes;

    HttpConfig m_config;
    RouteHttp::RouteFunc m_preRoute = nullptr;
    RouteHttp::RouteFunc m_postRoute = nullptr;
};

}

#endif // HTTPSERVER_H
