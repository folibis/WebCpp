#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <CommunicationTcpServer.h>
#include "common.h"
#include <queue>
#include <vector>
#include <pthread.h>
#include "Request.h"
#include "Response.h"
#include "Route.h"


namespace WebCpp
{

class HttpServer
{
public:
    HttpServer();
    bool Init();
    bool Run();
    bool Close();

    HttpServer& Get(const std::string &path, const Route::RouteFunc &f);
    HttpServer& Post(const std::string &path, const Route::RouteFunc &f);

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
    Request GetNextRequest();
    void ProcessRequest(Request &request);

private:
    struct RequestData
    {
        RequestData(int connID, ByteArray& data)
        {
            this->connID= connID;
            this->data = std::move(data);
        }
        int connID;
        ByteArray data;
    };

    CommunicationTcpServer m_server;
    pthread_t m_requestThread;

    pthread_mutex_t m_queueMutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t m_signalMutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t m_signalCondition = PTHREAD_COND_INITIALIZER;

    bool m_requestThreadRunning = false;
    std::queue<RequestData> m_requestQueue;
    std::vector<Route> m_routes;
};

}

#endif // HTTPSERVER_H
