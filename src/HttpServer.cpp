#include <iostream>
#include "common_webcpp.h"
#include "CommunicationTcpServer.h"
#include "CommunicationSslServer.h"
#include "LogWriter.h"
#include "Lock.h"
#include "FileSystem.h"
#include "StringUtil.h"
#include "Request.h"
#include "KeepAliveTimer.h"
#include "Data.h"
#include "HttpServer.h"
#include "IHttp.h"


using namespace WebCpp;

HttpServer::HttpServer()
{

}

bool HttpServer::Init()
{
    WebCpp::HttpConfig config;
    return Init(config);
}

bool WebCpp::HttpServer::Init(const WebCpp::HttpConfig& config)
{
    ClearError();
    m_config = config;

    m_protocol = Http::String2Protocol(m_config.GetHttpProtocol());

    switch(m_protocol)
    {
        case Http::Protocol::HTTP:
            m_server = std::make_shared<CommunicationTcpServer>();
            break;
#ifdef WITH_OPENSSL
        case Http::Protocol::HTTPS:
            m_server = std::make_shared<CommunicationSslServer>(m_config.GetSslSertificate(), m_config.GetSslKey());
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

    m_server->SetPort(m_config.GetHttpServerPort());

    if(!m_server->Init())
    {
        SetLastError("HttpServer init failed");
        LOG(GetLastError(), LogWriter::LogType::Error);
        return false;
    }

    LOG(ToString(), LogWriter::LogType::Info);

    FileSystem::ChangeDir(FileSystem::GetApplicationFolder());

    auto f1 = std::bind(&HttpServer::OnConnected, this, std::placeholders::_1, std::placeholders::_2);
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

    if(!m_server->Run())
    {
        return false;
    }

    return true;
}

bool HttpServer::Close(bool wait)
{
    m_server->Close(wait);
    KeepAliveTimer::stop();
    return true;
}

bool HttpServer::WaitFor()
{
    return m_server->WaitFor();
}

HttpServer &HttpServer::OnGet(const std::string &path, const RouteHttp::RouteFunc &f)
{
    RouteHttp route(path, Http::Method::GET);
    LOG("register route: " + route.ToString(), LogWriter::LogType::Info);
    route.SetFunction(f);
    m_routes.push_back(std::move(route));

    return *this;
}

HttpServer &HttpServer::OnPost(const std::string &path, const RouteHttp::RouteFunc &f)
{
    RouteHttp route(path, Http::Method::POST);
    LOG("register route: " + route.ToString(), LogWriter::LogType::Info);
    route.SetFunction(f);
    m_routes.push_back(std::move(route));
    return *this;
}

void HttpServer::SetPreRouteFunc(const RouteHttp::RouteFunc &callback)
{
    m_preRoute = callback;
}

void HttpServer::SetPostRouteFunc(const RouteHttp::RouteFunc &callback)
{
    m_postRoute = callback;
}

bool HttpServer::SendResponse(Response &response)
{
    if(response.IsShouldSend())
    {
        response.AddHeader(HttpHeader::HeaderType::Date, FileSystem::GetDateTime());
        if(response.Send(m_server.get()) == false)
        {
            LOG("Error sending response: " + response.GetLastError(), LogWriter::LogType::Error);
        }
    }

    return true;
}

std::string HttpServer::ToString() const
{
    return m_config.ToString();
}

void HttpServer::OnConnected(int connID, const std::string &remote)
{
    LOG(std::string("client connected: #") + std::to_string(connID) + ", " + remote, LogWriter::LogType::Access);
    PutToQueue(connID, remote);
}

void HttpServer::OnDataReady(int connID, ByteArray &data)
{
    AppendData(connID, data);
    SendSignal();
}

void HttpServer::OnClosed(int connID)
{    
    LOG(std::string("client disconnected: #") + std::to_string(connID), LogWriter::LogType::Access);
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
        if(CheckDataFullness())
        {
            auto request = GetNextRequest();
            ProcessRequest(*request);
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
    pthread_cond_wait(& m_signalCondition, &m_signalMutex);
}

void HttpServer::PutToQueue(int connID, const std::string &remote)
{
    Lock lock(m_queueMutex);
    m_requestQueue.push_back(RequestData(connID, remote));
}

void HttpServer::AppendData(int connID, const ByteArray &data)
{
    for(auto &req: m_requestQueue)
    {
        if(req.connID == connID)
        {
            req.data.insert(req.data.end(), data.begin(), data.end());
            if(req.request == nullptr)
            {
                req.request.reset(new Request(req.connID, m_config, req.remote));
            }
            break;
        }
    }
}

bool HttpServer::IsQueueEmpty()
{
    Lock lock(m_queueMutex);
    bool retval = false;
    for(RequestData& requestData: m_requestQueue)
    {
        if(requestData.readyForDispatch == true)
        {
            return true;
        }
    }
    return false;
}

bool HttpServer::CheckDataFullness()
{
    Lock lock(m_queueMutex);
    bool retval = false;

    for(RequestData& requestData: m_requestQueue)
    {
        if(requestData.request != nullptr)
        {
            if(requestData.request->Parse(requestData.data))
            {
                size_t size = requestData.request->GetRequestSize();
                if(requestData.data.size() >= size)
                {
                    requestData.readyForDispatch = true;
                    requestData.data.erase(requestData.data.begin(), requestData.data.begin() + size);
                    retval = true;
                    break;
                }
            }
        }
    }

    return retval;
}

std::unique_ptr<Request> HttpServer::GetNextRequest()
{
    Lock lock(m_queueMutex);

    for (auto it = m_requestQueue.begin(); it != m_requestQueue.end(); ++it)
    {
        if(it->readyForDispatch == true)
        {
            RequestData &data = (*it);
            data.readyForDispatch = false;
            return std::unique_ptr<Request>(std::move(it->request));
        }
    }

    return nullptr; // should never be called
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

    LOG(request.GetUrl().GetPath() + (processed ? ", processed" : ", not processed"), LogWriter::LogType::Access);

    SendResponse(response);
}

void HttpServer::ProcessKeepAlive(int connID)
{
    m_server->CloseConnection(connID);
    RemoveFromQueue(connID);
}

