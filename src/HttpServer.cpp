#include <iostream>
#include "common.h"
#include "Lock.h"
#include "FileSystem.h"
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

    FileSystem::ChangeDir(FileSystem::GetApplicationFolder());

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

HttpServer &HttpServer::Get(const std::string &path, const Route::RouteFunc &f)
{
    Route route(path, Request::Method::GET);
    route.SetFunction(f);
    m_routes.push_back(route);

    return *this;
}

HttpServer &HttpServer::Post(const std::string &path, const Route::RouteFunc &f)
{
    Route route(path, Request::Method::POST);
    route.SetFunction(f);
    m_routes.push_back(route);
    return *this;
}

void HttpServer::SetConfig(const HttpConfig &config)
{
    m_config = config;
}

void HttpServer::SetPreRouteFunc(const Route::RouteFunc &callback)
{
    m_preRoute = callback;
}

void HttpServer::SetPostRouteFunc(const Route::RouteFunc &callback)
{
    m_postRoute = callback;
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

    return Request(requestData.connID, requestData.data, m_config);
}

void HttpServer::ProcessRequest(Request &request)
{
    bool processed = false;

    Response response(request.GetConnectionID(), m_config);

    if(m_preRoute != nullptr)
    {
        processed = m_preRoute(request, response);
    }

    if(processed == false)
    {
        for(auto &route: m_routes)
        {
            if(route.IsMatch(request))
            {
                auto &f = route.GetFunction();
                if(f != nullptr)
                {
                    if((processed = f(request, response)))
                    {
                        break;
                    }
                }
            }
        }
    }

    if(m_postRoute != nullptr)
    {
        processed = m_postRoute(request, response);
    }

    if(processed == false)
    {
        response.SendNotFound();
    }

    response.SetHeader(Response::HeaderType::Date, FileSystem::GetDateTime());
    response.Send(&m_server);
}
