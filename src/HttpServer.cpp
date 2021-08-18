#include <iostream>
#include "common.h"
#include "CommunicationTcpServer.h"
#include "CommunicationSslServer.h"
#include "LogWriter.h"
#include "Lock.h"
#include "FileSystem.h"
#include "StringUtil.h"
#include "Request.h"
#include "KeepAliveTimer.h"
#include "HttpServer.h"


using namespace WebCpp;

HttpServer::HttpServer()
{

}

bool WebCpp::HttpServer::Init(WebCpp::HttpConfig config)
{
    ClearError();
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
        default:
            break;
    }

    if(m_server == nullptr)
    {
        SetLastError("protocol isn't set or not implemented");
        LOG(GetLastError(), LogWriter::LogType::Error);
        return false;
    }

    m_server->SetPort(m_config.GetServerPort());

    if(!m_server->Init())
    {
        SetLastError("HttpServer init failed");
        LOG(GetLastError(), LogWriter::LogType::Error);
        return false;
    }

    LOG(ToString(), LogWriter::LogType::Info);

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
        SetLastError("failed to run request thread");
        LOG(GetLastError(), LogWriter::LogType::Error);
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
    Route route(path, HttpHeader::Method::GET);
    LOG("register route: " + route.ToString(), LogWriter::LogType::Info);
    route.SetFunction(f);
    m_routes.push_back(std::move(route));

    return *this;
}

HttpServer &HttpServer::Post(const std::string &path, const Route::RouteFunc &f)
{
    Route route(path, HttpHeader::Method::POST);
    LOG("register route: " + route.ToString(), LogWriter::LogType::Info);
    route.SetFunction(f);
    m_routes.push_back(std::move(route));
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
    StringUtil::ToUpper(s);

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

std::string HttpServer::ToString() const
{
    return m_config.ToString();
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
            if(CheckDataFullness())
            {
                Request request = GetNextRequest();
                ProcessRequest(request);
            }
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

    bool alreadyExists = false;

    for(auto &req: m_requestQueue)
    {
        if(req.connID == connID)
        {
            req.data.insert(req.data.end(), data.begin(), data.end());
            alreadyExists = true;
            break;
        }
    }

    if(alreadyExists == false)
    {
        m_requestQueue.push_back(RequestData(connID, data));
    }
}

bool HttpServer::IsQueueEmpty()
{
    Lock lock(m_queueMutex);
    return m_requestQueue.empty();
}

bool HttpServer::CheckDataFullness()
{
    Lock lock(m_queueMutex);
    bool retval = false;

    for(RequestData& requestData: m_requestQueue)
    {
        if(requestData.header.IsComplete() == false)
        {
            requestData.header.Parse(requestData.data);
        }

        if(requestData.header.IsComplete())
        {
            size_t size = requestData.header.GetRequestSize();
            if(requestData.data.size() >= size)
            {
                requestData.readyForDispatch = true;
                retval = true;
                break;
            }
        }
    }

    return retval;
}

Request HttpServer::GetNextRequest()
{
    Lock lock(m_queueMutex);

    for (auto it = m_requestQueue.begin(); it != m_requestQueue.end(); ++it)
    {
        if(it->readyForDispatch == true)
        {
            RequestData data = std::move(*it);
            m_requestQueue.erase(it);
            return Request(data.connID, data.data, std::move(data.header), m_config);
        }
    }

    return Request(m_config); // should never be called
}

void HttpServer::RemoveFromQueue(int connID)
{
    Lock lock(m_queueMutex);
    for (auto it = m_requestQueue.begin(); it != m_requestQueue.end(); ++it)
    {
        if(it->connID == connID)
        {
            m_requestQueue.erase(it);
            break;
        }
    }
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
                    try
                    {
                        if((processed = f(request, response)))
                        {
                            break;
                        }
                    }
                    catch(...) { }
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


    LOG(request.GetHeader().ToString() + (processed ? ", processed" : ", not processed"), LogWriter::LogType::Access);

    response.SetHeader(Response::HeaderType::Date, FileSystem::GetDateTime());
    response.Send(m_server.get());
}

void HttpServer::ProcessKeepAlive(int connID)
{
    m_server->CloseClient(connID);
    RemoveFromQueue(connID);
}
