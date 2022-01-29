#ifdef WITH_WEBSOCKET

#include <cstring>
#include "CommunicationTcpServer.h"
#include "CommunicationSslServer.h"
#include "LogWriter.h"
#include "FileSystem.h"
#include "Lock.h"
#include "Data.h"
#include "common_ws.h"
#include "defines_webcpp.h"
#include "WebSocketServer.h"
#include "IHttp.h"


using namespace WebCpp;

WebSocketServer::WebSocketServer()
{

}

WebSocketServer::~WebSocketServer()
{

}

bool WebSocketServer::Init()
{
    WebCpp::HttpConfig config;
    return Init(config);
}

bool WebSocketServer::Init(WebCpp::HttpConfig config)
{
    ClearError();
    m_config = config;

    m_protocol = WebSocketServer::String2Protocol(m_config.GetWsProtocol());
    switch(m_protocol)
    {
        case Protocol::WS:
            m_server = std::make_shared<CommunicationTcpServer>();
            break;
#ifdef WITH_OPENSSL
        case Protocol::WSS:
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

    m_server->SetPort(m_config.GetWsServerPort());
    if(!m_server->Init())
    {
        SetLastError("WebSocketServer init failed");
        LOG(GetLastError(), LogWriter::LogType::Error);
        return false;
    }

    LOG(ToString(), LogWriter::LogType::Info);

    FileSystem::ChangeDir(FileSystem::GetApplicationFolder());

    auto f1 = std::bind(&WebSocketServer::OnConnected, this, std::placeholders::_1, std::placeholders::_2);
    m_server->SetNewConnectionCallback(f1);
    auto f2 = std::bind(&WebSocketServer::OnDataReady, this, std::placeholders::_1, std::placeholders::_2);
    m_server->SetDataReadyCallback(f2);
    auto f3 = std::bind(&WebSocketServer::OnClosed, this, std::placeholders::_1);
    m_server->SetCloseConnectionCallback(f3);

    if(StartRequestThread() == false)
    {
        return false;
    }

    return true;
}

bool WebSocketServer::Run()
{
    if(!m_server->Connect())
    {
        return false;
    }

    if(!m_server->Run())
    {
        return false;
    }

    m_running = true;
    return m_running;
}

bool WebSocketServer::Close(bool wait)
{
    m_server->Close(wait);
    StopRequestThread();
    return true;
}

bool WebSocketServer::WaitFor()
{
    return m_server->WaitFor();
}

void WebSocketServer::OnRequest(const std::string &path, const WebCpp::RouteHttp::RouteFunc &func)
{
    RouteWebSocket *route = GetRoute(path);
    if(route == nullptr)
    {
        RouteWebSocket route(path);
        LOG("register route: " + route.ToString(), LogWriter::LogType::Info);
        route.SetFunctionRequest(func);
        m_routes.push_back(std::move(route));
    }
    else
    {
        route->SetFunctionRequest(func);
        LOG("register request function for route: " + route->ToString(), LogWriter::LogType::Info);
    }
}

void WebSocketServer::OnMessage(const std::string &path, const std::function<bool(const Request &, ResponseWebSocket &, const ByteArray &)> &func)
{

    RouteWebSocket *route = GetRoute(path);
    if(route == nullptr)
    {
        RouteWebSocket route(path);
        LOG("register route: " + route.ToString(), LogWriter::LogType::Info);
        route.SetFunctionMessage(func);
        m_routes.push_back(std::move(route));
    }
    else
    {
        route->SetFunctionMessage(func);
        LOG("register message function for route: " + route->ToString(), LogWriter::LogType::Info);
    }
}

bool WebSocketServer::SendResponse(const ResponseWebSocket &response)
{
    if(!response.IsEmpty())
    {
        return response.Send(m_server.get());
    }

    return false;
}

WebSocketServer::Protocol WebSocketServer::GetProtocol() const
{
    return m_protocol;
}

WebSocketServer::Protocol WebSocketServer::String2Protocol(const std::string &str)
{
    std::string s = str;
    StringUtil::ToUpper(s);

    switch(_(s.c_str()))
    {
        case _("WS"): return WebSocketServer::Protocol::WS;
        case _("WSS"): return WebSocketServer::Protocol::WSS;
        default: break;
    }

    return WebSocketServer::Protocol::Undefined;
}

std::string WebSocketServer::Protocol2String(WebSocketServer::Protocol protocol)
{
    switch(protocol)
    {
        case WebSocketServer::Protocol::WS: return "HTTP";
        case WebSocketServer::Protocol::WSS: return "HTTPS";
        default: break;
    }

    return "";
}

std::string WebSocketServer::ToString() const
{
    return m_config.ToString();
}

void WebSocketServer::OnConnected(int connID, const std::string &remote)
{
    LOG(std::string("client connected: #") + std::to_string(connID) + ", " + remote, LogWriter::LogType::Access);
    InitConnection(connID, remote);
}

void WebSocketServer::OnDataReady(int connID, ByteArray &data)
{
    PutToQueue(connID, data);
    SendSignal();
}

void WebSocketServer::OnClosed(int connID)
{
    LOG(std::string("websocket connection closed: #") + std::to_string(connID), LogWriter::LogType::Access);
    RemoveFromQueue(connID);
}

bool WebSocketServer::StartRequestThread()
{
    auto f = std::bind(&WebSocketServer::RequestThread, this, std::placeholders::_1);
    m_requestThread.SetFunction(f);

    if(m_requestThread.Start() == false)
    {
        SetLastError("failed to run request thread: " + m_requestThread.GetLastError());
        LOG(GetLastError(), LogWriter::LogType::Error);
        return false;
    }

    return true;
}

bool WebSocketServer::StopRequestThread()
{
    if(m_requestThread.IsRunning())
    {
        m_requestThread.Stop();
        SendSignal();
        m_requestThread.Wait();
    }
    return true;
}

void *WebSocketServer::RequestThread(bool &running)
{
    while(m_requestThread.IsRunning())
    {
        WaitForSignal();
        if(CheckData())
        {
            ProcessRequests();
        }
    }

    return nullptr;
}

void WebSocketServer::SendSignal()
{
    Lock lock(m_signalMutex);
    m_signalCondition.Fire();
}

void WebSocketServer::WaitForSignal()
{
    Lock lock(m_signalMutex);
    m_signalCondition.Wait(m_signalMutex);
}

void WebSocketServer::PutToQueue(int connID, ByteArray &data)
{
    Lock lock(m_queueMutex);

    for(auto &req: m_requestQueue)
    {
        if(req.connID == connID)
        {
            req.data.insert(req.data.end(), data.begin(), data.end());
            break;
        }
    }
}

bool WebSocketServer::IsQueueEmpty()
{
    Lock lock(m_queueMutex);
    return m_requestQueue.empty();
}

void WebSocketServer::InitConnection(int connID, const std::string &remote)
{
    Lock lock(m_queueMutex);

    for(auto &req: m_requestQueue)
    {
        if(req.connID == connID)
        {
            return;
        }
    }

    m_requestQueue.push_back(RequestData(connID, remote, m_config));
}

bool WebSocketServer::CheckData()
{
    bool retval = false;
    Lock lock(m_queueMutex);

    for(RequestData& requestData: m_requestQueue)
    {
        if(requestData.readyForDispatch == false)
        {
            if(requestData.handshake == false)
            {
                retval |= CheckWsHeader(requestData);
            }
            else
            {
                bool frameParsed;
                while((frameParsed = CheckWsFrame(requestData)))
                {
                    retval |= frameParsed;
                }
            }
        }
    }

    return retval;
}

bool WebSocketServer::CheckWsHeader(RequestData& requestData)
{
    bool retval = false;

    if(requestData.request.Parse(requestData.data))
    {
        size_t size = requestData.request.GetRequestSize();
        if(requestData.data.size() >= size)
        {
            requestData.request.SetMethod(Http::Method::WEBSOCKET);
            requestData.data.erase(requestData.data.begin(), requestData.data.begin() + size);
            requestData.readyForDispatch = true;
            requestData.handshake = false;
            retval = true;
        }
    }

    return retval;
}

bool WebSocketServer::CheckWsFrame(RequestData& requestData)
{
    bool retval = false;
    Lock lock(m_requestMutex);

    RequestWebSocket request;
    if(request.Parse(requestData.data))
    {
        size_t size = request.GetSize();
        requestData.data.erase(requestData.data.begin(), requestData.data.begin() + size);
        requestData.requestList.push_back(std::move(request));
        requestData.readyForDispatch = true;
        requestData.handshake = true;
        retval = true;
    }

    return retval;
}

void WebSocketServer::ProcessRequests()
{
    Lock lock(m_queueMutex);

    for(auto &entry: m_requestQueue)
    {
        if(entry.readyForDispatch)
        {
            if(entry.handshake == false)
            {
                if(ProcessRequest(entry.request))
                {
                    entry.handshake = true;
                    entry.readyForDispatch = false;
                }
            }
            else
            {
                Lock lock(m_requestMutex);
                if(entry.requestList.size() > 0)
                {
                    auto it = entry.requestList.begin();
                    while (it != entry.requestList.end())
                    {
                        if(it->IsFinal())
                        {
                            ProcessWsRequest(entry.request, *it);
                            it = entry.requestList.erase(it);
                        }
                        else
                        {
                            ++it;
                        }
                    }

                    entry.readyForDispatch = false;
                }
            }
        }
    }
}

void WebSocketServer::RemoveFromQueue(int connID)
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

bool WebSocketServer::ProcessRequest(Request &request)
{
    Response response(request.GetConnectionID(), m_config);
    bool processed = false;
    bool matched = false;

    for(auto &route: m_routes)
    {
        if(route.IsMatch(request))
        {
            matched = true;
            auto &f = route.GetFunctionRequest();
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

    // the uri is matched but not request handler is provided or request is not processed
    if(processed == false && matched == true)
    {
        if(m_config.GetWsProcessDefault() == true)
        {
            std::string key = request.GetHeader().GetHeader("Sec-WebSocket-Key");
            key = key + WEBSOCKET_KEY_TOKEN;

            uint8_t *buffer = Data::Sha1Digest(key);
            key = Data::Base64Encode(buffer, 20);

            response.SetResponseCode(101);
            response.AddHeader(HttpHeader::HeaderType::Date, FileSystem::GetDateTime());
            response.AddHeader(HttpHeader::HeaderType::Upgrade, "websocket");
            response.AddHeader(HttpHeader::HeaderType::Connection, "upgrade");
            response.AddHeader("Sec-WebSocket-Accept", key);
            response.AddHeader("Sec-WebSocket-Version", WS_VERSION);
        }
        else
        {
            response.SendNotFound();
        }
    }

    return response.Send(m_server.get());
}

bool WebSocketServer::ProcessWsRequest(Request &request, const RequestWebSocket &wsRequest)
{
    ResponseWebSocket response(request.GetConnectionID());
    bool processed = false;

    auto type = wsRequest.GetType();
    switch(type)
    {
        case MessageType::Text:
        case MessageType::Binary:
            for(auto &route: m_routes)
            {
                if(route.IsMatch(request))
                {
                    auto &f = route.GetFunctionMessage();
                    if(f != nullptr)
                    {
                        try
                        {
                            if(f(request, response, wsRequest.GetData()) == true)
                            {
                                break;
                            }
                        }
                        catch(...) { }
                    }
                }
            }
            break;
        case MessageType::Ping:
            response.WriteBinary(wsRequest.GetData());
            response.SetMessageType(MessageType::Pong);
            break;
        case MessageType::Close:
            break;
        default:
            break;
    }

    if(!response.IsEmpty())
    {
        response.Send(m_server.get());
    }

    return true;
}

RouteWebSocket *WebSocketServer::GetRoute(const std::string &path)
{
    for(size_t i = 0;i < m_routes.size();i ++)
    {
        if(m_routes.at(i).GetPath() == path)
        {
            return &(m_routes.at(i));
        }
    }

    return nullptr;
}

#endif
