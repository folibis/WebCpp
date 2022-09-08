#include "CommunicationTcpClient.h"
#include "CommunicationSslClient.h"
#include "LogWriter.h"
#include "Data.h"
#include "Response.h"
#include "RequestWebSocket.h"
#include "WebSocketClient.h"


using namespace WebCpp;

WebSocketClient::WebSocketClient():
    m_config(WebCpp::HttpConfig::Instance())
{

}

bool WebSocketClient::Init()
{
    return true;
}

bool WebSocketClient::Run()
{
    return true;
}

bool WebSocketClient::Close(bool wait)
{
    return m_connection->Close(wait);
}

bool WebSocketClient::WaitFor()
{
    return m_connection->WaitFor();
}

bool WebSocketClient::Open(Request &request)
{
    ClearError();

    if(m_connection == nullptr)
    {
        if(InitConnection(request.GetUrl()) == false)
        {
            SetLastError("init failed: " + GetLastError());
            LOG(GetLastError(), LogWriter::LogType::Error);
            return false;
        }
    }

    if(m_connection->IsConnected() == false)
    {
        if(m_connection->Connect() == false)
        {
            SetLastError("connection faied: " + m_connection->GetLastError());
            LOG(GetLastError(), LogWriter::LogType::Error);
            return false;
        }

        SetState(State::Connected);
    }

    if(m_connection->Run() == false)
    {
        SetLastError("read routine failed: " + m_connection->GetLastError());
        LOG(GetLastError(), LogWriter::LogType::Error);
        return false;
    }

    auto &header = request.GetHeader();
    header.SetHeader(HttpHeader::HeaderType::Host, request.GetUrl().GetHost());
    header.SetHeader(HttpHeader::HeaderType::Upgrade, "websocket");
    header.SetHeader(HttpHeader::HeaderType::Connection, "Upgrade");
    m_key = Data::Base64Encode(StringUtil::GenerateRandomString(16));
    header.SetHeader("Sec-WebSocket-Key", m_key);
    header.SetHeader("Sec-WebSocket-Version", WS_VERSION);
    if(request.Send(m_connection) == false)
    {
        SetLastError("request sending error: " + request.GetLastError());
        LOG(GetLastError(), LogWriter::LogType::Error);
        return false;
    }

    SetState(State::HandShake);

    return true;
}

bool WebSocketClient::Open(const std::string &address)
{
    ClearError();

    Request request;
    auto &p_url = request.GetUrl();
    p_url.Parse(address);
    if(p_url.IsInitiaized())
    {
        request.SetMethod(Http::Method::GET);
        return Open(request);
    }
    else
    {
        SetLastError("Url parsing error");
        LOG(GetLastError(), LogWriter::LogType::Error);
    }

    return false;
}

bool WebSocketClient::SendText(const ByteArray &data)
{
    RequestWebSocket request;
    request.SetType(MessageType::Text);
    request.SetData(data);
    return request.Send(m_connection.get());
}

bool WebSocketClient::SendText(const std::string &data)
{
    return SendText(StringUtil::String2ByteArray(data));
}

bool WebSocketClient::SendBinary(const ByteArray &data)
{
    RequestWebSocket request;
    request.SetType(MessageType::Binary);
    request.SetData(data);
    return request.Send(m_connection.get());
}

bool WebSocketClient::SendBinary(const std::string &data)
{
    return SendBinary(StringUtil::String2ByteArray(data));
}

bool WebSocketClient::SendPing()
{
    RequestWebSocket request;
    request.SetType(MessageType::Ping);
    return request.Send(m_connection.get());
}

void WebSocketClient::SetOnConnect(const std::function<void (bool)> &callback)
{
    m_connectCallback = callback;
}

void WebSocketClient::SetOnClose(const std::function<void ()> &callback)
{
    m_closeCallback = callback;
}

void WebSocketClient::SetOnError(const std::function<void (const std::string &)> &callback)
{
    m_errorCallback = callback;
}

void WebSocketClient::SetOnMessage(const std::function<void (ResponseWebSocket &)> &callback)
{
    m_messageCallback = callback;
}

void WebSocketClient::SetProgressCallback(const std::function<void (size_t, size_t)> &callback)
{
    m_progressCallback = callback;
}

void WebSocketClient::SetOnStateChanged(const std::function<void (State)> &callback)
{
    m_stateCallback = callback;
}

void WebSocketClient::OnDataReady(const ByteArray &data)
{
    if(m_state == State::HandShake)
    {
        Response response(0, m_config);
        size_t all, downloaded;
        if(response.Parse(data, &all, &downloaded))
        {
            if(response.GetResponseCode() == 101)
            {
                auto &header = response.GetHeader();
                std::string h = header.GetHeader(HttpHeader::HeaderType::Upgrade);
                StringUtil::ToLower(h);
                if(h == "websocket")
                {
                    h = header.GetHeader(HttpHeader::HeaderType::Connection);
                    StringUtil::ToLower(h);
                    if(h == "upgrade")
                    {
                        h = header.GetHeader("Sec-WebSocket-Accept");
                        std::string key = m_key + WEBSOCKET_KEY_TOKEN;
                        uint8_t *buffer = Data::Sha1Digest(key);
                        key = Data::Base64Encode(buffer, 20);
                        if(h == key)
                        {
                            SetState(State::BinaryMessage);
                            if(m_connectCallback != nullptr)
                            {
                                m_connectCallback(true);
                            }
                            return;
                        }
                        else
                        {
                            SetLastError("incorrect response key");
                        }
                    }
                    else
                    {
                        SetLastError("incorrect response header");
                    }
                }
                else
                {
                    SetLastError("incorrect response header");
                }
            }
            else
            {
                SetLastError("incorrect response code");
            }
        }
        else
        {
            SetLastError("error while response parsing");
        }

        SetState(State::Closed);
        if(m_connectCallback != nullptr)
        {
            m_connectCallback(false);
        }

        Close();
    }
    else if(m_state == State::BinaryMessage)
    {
        m_data.insert(m_data.end(), data.begin(), data.end());
        ResponseWebSocket response(0);
        if(response.Parse(m_data) == true)
        {
            if(m_messageCallback != nullptr)
            {
                m_messageCallback(response);
            }

            m_data.clear();
        }
    }
}

void WebSocketClient::OnClosed()
{
    SetState(State::Closed);

    if(m_closeCallback != nullptr)
    {
        m_closeCallback();
    }
}

bool WebSocketClient::InitConnection(const Url &url)
{
    if(m_connection != nullptr)
    {
        m_connection->Close();
    }

    switch(url.GetScheme())
    {
        case Url::Scheme::WS:
            m_connection = std::make_shared<CommunicationTcpClient>();
            break;
#ifdef WITH_OPENSSL
        case Url::Scheme::WSS:
            m_connection = std::make_shared<CommunicationSslClient>(m_config.GetSslSertificate(), m_config.GetSslKey());
            break;
#endif
        default:
            break;
    }

    if(m_connection == nullptr)
    {
        SetLastError("provided scheme is incorrect or not supported");
        LOG(GetLastError(), LogWriter::LogType::Error);
        return false;
    }

    m_connection->SetHost(url.GetHost());
    m_connection->SetPort(url.GetPort());

    if(!m_connection->Init())
    {
        SetLastError("WebSocketClient init failed");
        LOG(GetLastError(), LogWriter::LogType::Error);
        return false;
    }

    if(!m_connection->Run())
    {
        SetLastError("WebSocketClient run failed");
        LOG(GetLastError(), LogWriter::LogType::Error);
        return false;
    }

    SetState(State::Initialized);

    auto f1 = std::bind(&WebSocketClient::OnDataReady, this, std::placeholders::_1);
    m_connection->SetDataReadyCallback(f1);
    auto f2 = std::bind(&WebSocketClient::OnClosed, this);
    m_connection->SetCloseConnectionCallback(f2);

    return true;
}

void WebSocketClient::SetState(State state)
{
    m_state = state;
    if(m_stateCallback != nullptr)
    {
        m_stateCallback(m_state);
    }
}
