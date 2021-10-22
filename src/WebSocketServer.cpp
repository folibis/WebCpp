#ifdef WITH_WEBSOCKET

#include <cstring>
#include <iostream>
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

#define WEBSOCKET_KEY_TOKEN "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"


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

    m_requestThreadRunning = true;
    if(pthread_create(&m_requestThread, nullptr, &WebSocketServer::RequestThreadWrapper, this) != 0)
    {
        SetLastError("failed to run request thread");
        LOG(GetLastError(), LogWriter::LogType::Error);
        m_requestThreadRunning = false;
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

    return true;
}

bool WebSocketServer::Close(bool wait)
{
    m_server->Close(wait);
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
    LOG(std::string("client disconnected: #") + std::to_string(connID), LogWriter::LogType::Access);
    RemoveFromQueue(connID);
}

void *WebSocketServer::RequestThreadWrapper(void *ptr)
{
    WebSocketServer *instance = static_cast<WebSocketServer *>(ptr);
    if(instance != nullptr)
    {
        return instance->RequestThread();
    }

    return nullptr;
}

void *WebSocketServer::RequestThread()
{
    while(m_requestThreadRunning)
    {
        WaitForSignal();
        if(CheckDataFullness())
        {
            ProcessRequests();
        }
    }

    return nullptr;
}

void WebSocketServer::SendSignal()
{
    Lock lock(m_signalMutex);
    pthread_cond_signal(&m_signalCondition);
}

void WebSocketServer::WaitForSignal()
{
    Lock lock(m_signalMutex);
    pthread_cond_wait(& m_signalCondition, &m_signalMutex);
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

bool WebSocketServer::CheckDataFullness()
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
                retval |= CheckWsFrame(requestData);
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
            requestData.readyForDispatch = true;
            retval = true;
        }
    }

    return retval;
}

bool WebSocketServer::CheckWsFrame(RequestData& requestData)
{
    bool retval = false;

    WebSocketHeader header;
    size_t dataSize;
    size_t headerSize = sizeof(WebSocketHeader);

    while((dataSize = requestData.data.size()) >= headerSize)
    {
        std::memcpy(&header, requestData.data.data(), headerSize);

        uint64_t payloadSize = 0;
        size_t sizeHeaderSize = 0;
        switch(header.flags2.PayloadLen)
        {
            case 126:
                sizeHeaderSize = sizeof(WebSocketHeaderLength2);
                if(dataSize >= headerSize + sizeHeaderSize)
                {
                    WebSocketHeaderLength2 length;
                    uint8_t* ptr = requestData.data.data() + headerSize;
                    length.length.bytes[0] = *(ptr + 1);
                    length.length.bytes[1] = *ptr;
                    payloadSize = length.length.value;
                }
                else
                {
                    return false;
                }
                break;
            case 127:
                sizeHeaderSize = sizeof(WebSocketHeaderLength3);
                if(dataSize >= headerSize + sizeHeaderSize)
                {
                    WebSocketHeaderLength3 length;
                    uint8_t* ptr = requestData.data.data() + headerSize;
                    for(int i = 0;i < 8;i ++)
                    {
                        length.length.bytes[i] = *(ptr + 7 - i);
                    }
                    payloadSize = length.length.value;
                }
                else
                {
                    return false;
                }
                break;
            default:
                payloadSize = header.flags2.PayloadLen;
                break;
        }

        if(payloadSize > 0)
        {
            size_t maskHeaderSize = 0;
            WebSocketHeaderMask mask;
            size_t headers_size = headerSize + sizeHeaderSize;
            if(header.flags2.Mask == 1)
            {
                maskHeaderSize = sizeof(WebSocketHeaderMask);
                headers_size += maskHeaderSize;
                if(dataSize >= headers_size)
                {
                    std::memcpy(&mask, requestData.data.data() + headerSize + sizeHeaderSize, maskHeaderSize);
                }
                else
                {
                    return false;
                }
            }

            size_t messageFullSize = headers_size + payloadSize;
            if(dataSize >= messageFullSize)
            {
                // according to rfc6455#section-5.3 server must ignore unmasked data
                // but anyway I decided to support such unstandard clients
                if(header.flags2.Mask == 1)
                {
                    ByteArray encodedData(payloadSize);
                    for(size_t i = 0;i < payloadSize;i ++)
                    {
                        encodedData[i] = requestData.data[headers_size + i] ^ mask.bytes[i % 4];
                    }
                    requestData.encodedData.push_back(std::move(encodedData));
                }
                else
                {
                    requestData.encodedData.push_back(
                                ByteArray(
                                    requestData.data.begin() + headers_size,
                                    requestData.data.begin() + messageFullSize)
                                );
                }
                requestData.data.erase(requestData.data.begin(), requestData.data.begin() + messageFullSize);
                requestData.data.shrink_to_fit();

                if(header.flags1.FIN == 1)
                {
                    requestData.readyForDispatch = true;
                    retval = true;
                }
            }
        }
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
                    entry.data.clear();
                    entry.handshake = true;
                    entry.readyForDispatch = false;
                }
            }
            else
            {
                if(entry.encodedData.size() > 0)
                {
                    for(const auto&data: entry.encodedData)
                    {
                        ProcessWsRequest(entry.request, data);
                    }

                    entry.readyForDispatch = false;
                    entry.encodedData.clear();
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
        }
        else
        {
            response.SendNotFound();
        }
    }

    return response.Send(m_server.get());
}

bool WebSocketServer::ProcessWsRequest(Request &request, const ByteArray &data)
{
    ResponseWebSocket response(request.GetConnectionID());
    bool processed = false;

    for(auto &route: m_routes)
    {
        if(route.IsMatch(request))
        {
            auto &f = route.GetFunctionMessage();
            if(f != nullptr)
            {
                try
                {
                    if((processed = f(request, response, data)))
                    {
                        break;
                    }
                }
                catch(...) { }
            }
        }
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
