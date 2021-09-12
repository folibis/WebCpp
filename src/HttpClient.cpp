#include "CommunicationTcpClient.h"
#include "CommunicationSslClient.h"
#include "LogWriter.h"
#include "HttpClient.h"
#include "Response.h"
#include <iostream>

using namespace WebCpp;

HttpClient::HttpClient()
{
    //m_address = address;
    //m_port = port;
}

bool HttpClient::Init()
{
    HttpConfig config;
    return Init(config);
}

bool HttpClient::Init(const HttpConfig &config)
{
    m_config = config;

    return true;
}

bool HttpClient::Run()
{
    return true;
    //return m_connection->Run();
}

bool HttpClient::Close(bool wait)
{
    return m_connection->Close(wait);
}

bool HttpClient::WaitFor()
{
    return m_connection->WaitFor();
}

bool HttpClient::Open(Request &request)
{
    if(request.Send(m_connection.get()) == false)
    {
        SetLastError("request sending error: " + request.GetLastError());
        LOG(GetLastError(), LogWriter::LogType::Error);
        return false;
    }
    return true;
}

bool HttpClient::Open(Http::Method method, const std::string &address, const std::map<std::string, std::string> &headers)
{
    ClearError();

    Request request;
    auto &url = request.GetUrl();
    url.Parse(address);
    if(url.IsInitiaized())
    {
        if(InitConnection(url))
        {
            m_connection->Connect();
            request.SetMethod(method);
            for(auto &header: headers)
            {
                request.GetHeader().SetHeader(header.first, header.second);
            }
            return Open(request);
        }
    }
    else
    {
        SetLastError("Url parsing error");
        LOG(GetLastError(), LogWriter::LogType::Error);
    }

    return false;
}

void HttpClient::SetResponseCallback(const std::function<bool (const Response &)> &func)
{
    m_responseCallback = func;
}

void HttpClient::OnDataReady(ByteArray &data)
{
    m_buffer.insert(m_buffer.end(), data.begin(), data.end());

    Response response(0, m_config);
    if(response.Parse(m_buffer))
    {
        if(m_responseCallback != nullptr)
        {
            m_responseCallback(response);
        }
        m_buffer.clear();
    }
}

void HttpClient::OnClosed()
{
    m_buffer.clear();
}

bool HttpClient::InitConnection(const Url &url)
{
    if(m_connection != nullptr)
    {
        m_connection->Close();
    }

    switch(url.GetScheme())
    {
        case Url::Scheme::HTTP:
            m_connection.reset(new CommunicationTcpClient());
            break;
#ifdef WITH_OPENSSL
        case Url::Scheme::HTTPS:
            m_server.reset(new CommunicationSslServer(m_config.GetSslSertificate(), m_config.GetSslKey()));
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

    m_connection->SetAddress(url.GetHost());
    m_connection->SetPort(url.GetPort());

    if(!m_connection->Init())
    {
        SetLastError("HttpServer init failed");
        LOG(GetLastError(), LogWriter::LogType::Error);
        return false;
    }

    auto f1 = std::bind(&HttpClient::OnDataReady, this, std::placeholders::_1);
    m_connection->SetDataReadyCallback(f1);
    auto f2 = std::bind(&HttpClient::OnClosed, this);
    m_connection->SetCloseConnectionCallback(f2);

    m_connection->Run();

    return true;
}
