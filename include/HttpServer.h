#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <ICommunicationServer.h>
#include "common.h"
#include <deque>
#include <vector>
#include <memory>
#include <pthread.h>
#include "IError.h"
#include "Request.h"
#include "Response.h"
#include "Route.h"
#include "HttpConfig.h"
#include "HttpHeader.h"


namespace WebCpp
{

class HttpServer: public IError
{
public:
    enum class Protocol
    {
        Undefined = 0,
        HTTP,
        HTTPS,
    };

    HttpServer();
    bool Init(WebCpp::HttpConfig config);
    bool Run();
    bool Close();

    HttpServer& Get(const std::string &path, const Route::RouteFunc &f);
    HttpServer& Post(const std::string &path, const Route::RouteFunc &f);

    void SetPreRouteFunc(const Route::RouteFunc &callback);
    void SetPostRouteFunc(const Route::RouteFunc &callback);

    Protocol GetProtocol() const;

    static Protocol String2Protocol(const std::string &str);
    static std::string Protocol2String(Protocol protocol);

protected:
    void OnConnected(int connID);
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
    std::vector<Route> m_routes;

    HttpConfig m_config;
    Route::RouteFunc m_preRoute = nullptr;
    Route::RouteFunc m_postRoute = nullptr;
};

}

#endif // HTTPSERVER_H
