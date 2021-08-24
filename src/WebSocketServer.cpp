#include <cstring>
#include <iostream>
#include "CommunicationTcpServer.h"
#include "CommunicationSslServer.h"
#include "LogWriter.h"
#include "FileSystem.h"
#include "Lock.h"
#include "Data.h"
#include "common_ws.h"
#include "WebSocketServer.h"

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
            m_server.reset(new CommunicationTcpServer());
            break;
#ifdef WITH_OPENSSL
        case Protocol::WSS:
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

void WebSocketServer::SetWebSocketRequestFunc(const WebCpp::Route::RouteFunc &callback)
{
    m_webSocketRequest = callback;
}

void WebSocketServer::Data(const std::function<bool(const HttpHeader &, ResponseWebSocket &, const ByteArray &)> &func)
{
    m_dataFunc = func;
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

void WebSocketServer::OnDataReady(int connID, std::vector<char> &data)
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

    m_requestQueue.push_back(RequestData(connID, remote));
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
        }
    }

    return retval;
}
bool WebSocketServer::CheckWsFrame(RequestData& requestData)
{
    bool retval = false;

    WebSocketHeader header;
    size_t dataSize = requestData.data.size();
    size_t headerSize = sizeof(WebSocketHeader);

    if(dataSize >= headerSize)
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
                    std::memcpy(&length, requestData.data.data() + headerSize, sizeof(WebSocketHeaderLength2));
                    payloadSize = length.length;
                }
                break;
            case 127:
                sizeHeaderSize = sizeof(WebSocketHeaderLength3);
                if(dataSize >= headerSize + sizeHeaderSize)
                {
                    WebSocketHeaderLength3 length;
                    std::memcpy(&length, requestData.data.data() + headerSize, sizeof(WebSocketHeaderLength3));
                    payloadSize = length.length;
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
            if(header.flags2.Mask == 1)
            {
                maskHeaderSize = sizeof(WebSocketHeaderMask);
                if(dataSize >= headerSize + sizeHeaderSize + maskHeaderSize)
                {                    
                    std::memcpy(&mask, requestData.data.data() + headerSize + sizeHeaderSize, maskHeaderSize);
                }
            }

            if(dataSize == (headerSize + sizeHeaderSize + maskHeaderSize + payloadSize))
            {
                requestData.encodedData = ByteArray(payloadSize);
                for(size_t i = 0;i < payloadSize;i ++)
                {
                    requestData.encodedData[i] = requestData.data[headerSize + sizeHeaderSize + maskHeaderSize + i] ^ mask.bytes[i % 4];
                }

                requestData.data.empty();
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

    for (auto it = m_requestQueue.begin(); it != m_requestQueue.end(); ++it)
    {
        if(it->readyForDispatch)
        {
            if(it->handshake == false)
            {
                if(ProcessRequest(Request(it->connID, it->data, std::move(it->header), m_config)))
                {
                    it->data.clear();
                    it->handshake = true;
                    it->readyForDispatch = false;
                }
            }
            else
            {
                ProcessWsRequest(it->connID, it->header, it->encodedData);
                it->readyForDispatch = false;
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

bool WebSocketServer::ProcessRequest(const Request &request)
{
    Response response(request.GetConnectionID(), m_config);

    if(m_webSocketRequest != nullptr)
    {
        m_webSocketRequest(request, response);
    }

    std::string key = request.GetHeader().GetHeader("Sec-WebSocket-Key");
    key = key + WEBSOCKET_KEY_TOKEN;

    uint8_t *buffer = Data::Sha1Digest(key);
    key = Data::Base64Encode(buffer, 20);

    response.SetResponseCode(101);
    response.SetHeader(Response::HeaderType::Date, FileSystem::GetDateTime());
    response.SetHeader(Response::HeaderType::Upgrade, "websocket");
    response.SetHeader(Response::HeaderType::Connection, "upgrade");
    response.SetHeader("Sec-WebSocket-Accept", key);    

    return response.Send(m_server.get());
}

bool WebSocketServer::ProcessWsRequest(int connID, const HttpHeader& header, const ByteArray &data)
{
    if(m_dataFunc != nullptr)
    {
        ResponseWebSocket response(connID);
        m_dataFunc(header, response, data);
        if(!response.IsEmpty())
        {
            response.Send(m_server.get());
        }
    }

    return true;
}
