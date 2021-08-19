#include "CommunicationTcpServer.h"
#include "CommunicationSslServer.h"
#include "LogWriter.h"
#include "FileSystem.h"
#include "WebSocketServer.h"


using namespace WebCpp;

WebSocketServer::WebSocketServer()
{

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

    m_server->WaitFor();
    return true;
}

bool WebSocketServer::Close()
{
    m_server->Close();
    return true;
}

void WebSocketServer::SetWebSocketRequestFunc(const WebCpp::Route::RouteFunc &callback)
{
    m_webSocketRequest = callback;
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
}

void WebSocketServer::OnDataReady(int connID, std::vector<char> &data)
{
    PutToQueue(connID, data);
    SendSignal();
}

void WebSocketServer::OnClosed(int connID)
{

}

void *WebSocketServer::RequestThreadWrapper(void *ptr)
{

}

void *WebSocketServer::RequestThread()
{

}

void WebSocketServer::SendSignal()
{

}

void WebSocketServer::WaitForSignal()
{

}

void WebSocketServer::PutToQueue(int connID, ByteArray &data)
{

}

bool WebSocketServer::IsQueueEmpty()
{

}

bool WebSocketServer::CheckDataFullness()
{

}

Request WebSocketServer::GetNextRequest()
{

}

void WebSocketServer::RemoveFromQueue(int connID)
{

}

void WebSocketServer::ProcessRequest(Request &request)
{

}
