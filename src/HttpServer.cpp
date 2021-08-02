#include <iostream>
#include "common.h"
#include "CommunicationTcpServer.h"
#include "CommunicationSslServer.h"
#include "Lock.h"
#include "FileSystem.h"
#include "Request.h"
#include "KeepAliveTimer.h"
#include "HttpServer.h"


using namespace WebCpp;

HttpServer::HttpServer()
{

}

bool WebCpp::HttpServer::Init(WebCpp::HttpConfig config)
{
    m_config = config;

    m_protocol = HttpServer::String2Protocol(m_config.GetProtocol());

    switch(m_protocol)
    {
        case Protocol::HTTP:
            m_server.reset(new CommunicationTcpServer());
            break;
#ifdef WITH_OPENSSL
        case Protocol::HTTPS:
            m_server.reset(new CommunicationSslServer(m_config.GetSslSertificate(), m_config.GetSslKey()));
            break;
#endif
        default: break;
    }

    if(m_server == nullptr)
    {
        return false;
    }

    m_server->SetPort(m_config.GetServerPort());

    if(!m_server->Init())
    {
        return false;
    }

    FileSystem::ChangeDir(FileSystem::GetApplicationFolder());

    auto f1 = std::bind(&HttpServer::OnConnected, this, std::placeholders::_1);
    m_server->SetNewConnectionCallback(f1);
    auto f2 = std::bind(&HttpServer::OnDataReady, this, std::placeholders::_1, std::placeholders::_2);
    m_server->SetDataReadyCallback(f2);
    auto f3 = std::bind(&HttpServer::OnClosed, this, std::placeholders::_1);
    m_server->SetCloseConnectionCallback(f3);

    m_requestThreadRunning = true;
    if(pthread_create(&m_requestThread, nullptr, &HttpServer::RequestThreadWrapper, this) != 0)
    {
        m_requestThreadRunning = false;
        return false;
    }

    if(m_config.GetKeepAliveTimeout() != 0)
    {
        auto f = std::bind(&HttpServer::ProcessKeepAlive, this, std::placeholders::_1);
        KeepAliveTimer::SetCallback(f);
        KeepAliveTimer::run();
    }

    return true;
}

bool HttpServer::Run()
{
    if(!m_server->Connect())
    {
        return false;
    }

    m_server->WaitFor();
    return true;
}

bool HttpServer::Close()
{
    m_server->Close();
    KeepAliveTimer::stop();
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

void HttpServer::SetPreRouteFunc(const Route::RouteFunc &callback)
{
    m_preRoute = callback;
}

void HttpServer::SetPostRouteFunc(const Route::RouteFunc &callback)
{
    m_postRoute = callback;
}

HttpServer::Protocol HttpServer::GetProtocol() const
{
    return m_protocol;
}

HttpServer::Protocol HttpServer::String2Protocol(const std::string &str)
{
    std::string s = str;
    toUpper(s);

    switch(_(s.c_str()))
    {
        case _("HTTP"): return HttpServer::Protocol::HTTP;
        case _("HTTPS"): return HttpServer::Protocol::HTTPS;
        default: break;
    }

    return HttpServer::Protocol::Undefined;
}

std::string HttpServer::Protocol2String(HttpServer::Protocol protocol)
{
    switch(protocol)
    {
        case HttpServer::Protocol::HTTP: return "HTTP";
        case HttpServer::Protocol::HTTPS: return "HTTPS";
        default: break;
    }

    return "";
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

    if(m_config.GetKeepAliveTimeout() > 0)
    {
        KeepAliveTimer::SetTimer(m_config.GetKeepAliveTimeout(), request.GetConnectionID());
    }

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
    response.Send(m_server.get());
}

void HttpServer::ProcessKeepAlive(int connID)
{
    m_server->CloseClient(connID);
}
