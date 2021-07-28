#include <iostream>
#include "common.h"
#include "Lock.h"
#include "Request.h"
#include "HttpServer.h"


using namespace WebCpp;

HttpServer::HttpServer()
{

}

bool WebCpp::HttpServer::Init()
{
    m_server.SetPort(8080);

    if(!m_server.Init())
    {
        return false;
    }

    auto f1 = std::bind(&HttpServer::OnConnected, this, std::placeholders::_1);
    m_server.SetNewConnectionCallback(f1);
    auto f2 = std::bind(&HttpServer::OnDataReady, this, std::placeholders::_1, std::placeholders::_2);
    m_server.SetDataReadyCallback(f2);
    auto f3 = std::bind(&HttpServer::OnClosed, this, std::placeholders::_1);
    m_server.SetCloseConnectionCallback(f3);

    m_requestThreadRunning = true;
    if(pthread_create(&m_requestThread, nullptr, &HttpServer::RequestThreadWrapper, this) != 0)
    {
        m_requestThreadRunning = false;
        return false;
    }

    return true;
}

bool HttpServer::Run()
{
    if(!m_server.Connect())
    {
        return false;
    }

    m_server.WaitFor();
    return true;
}

bool HttpServer::Close()
{
    m_server.Close();
    return true;
}

HttpServer &HttpServer::Get(const std::string &path, HttpServer::RouteFunc &f)
{
    return *this;
}

HttpServer &HttpServer::Post(const std::string &path, HttpServer::RouteFunc &f)
{
    return *this;
}

void HttpServer::OnConnected(int connID)
{
    std::cout << "client connected: #" << connID << std:: endl;
}

void HttpServer::OnDataReady(int connID, ByteArray &data)
{
    PutToQueue(connID, data);
    SendSignal();
}

void HttpServer::OnClosed(int connID)
{    
    std::cout << "client disconnected: #" << connID << std:: endl;
}

void *HttpServer::RequestThreadWrapper(void *ptr)
{
    HttpServer *instance = static_cast<HttpServer *>(ptr);
    if(instance != nullptr)
    {
        return instance->RequestThread();
    }

    return nullptr;
}

void *HttpServer::RequestThread()
{
    while(m_requestThreadRunning)
    {
        WaitForSignal();
        while(!IsQueueEmpty())
        {
            Request request = GetNextRequest();
            ProcessRequest(request);
        }
    }

    return nullptr;
}

void HttpServer::SendSignal()
{
    Lock lock(m_signalMutex);
    pthread_cond_signal(&m_signalCondition);
}

void HttpServer::WaitForSignal()
{
    Lock lock(m_signalMutex);
    while(IsQueueEmpty() && m_requestThreadRunning)
    {
        pthread_cond_wait(& m_signalCondition, &m_signalMutex);
    }
}

void HttpServer::PutToQueue(int connID, ByteArray &data)
{
    Lock lock(m_queueMutex);
    m_requestQueue.push(RequestData(connID, data));
}

bool HttpServer::IsQueueEmpty()
{
    Lock lock(m_queueMutex);
    return m_requestQueue.empty();
}

Request HttpServer::GetNextRequest()
{
    Lock lock(m_queueMutex);
    RequestData requestData = m_requestQueue.front();
    m_requestQueue.pop();

    return Request(requestData.connID, requestData.data);
}

void HttpServer::ProcessRequest(const Request &request)
{
    for(auto &route: m_routes)
    {
        if(route.IsMatch(request))
        {
            Response response;
            break;
        }
    }
}
