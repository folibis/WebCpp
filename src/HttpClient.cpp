#include "CommunicationTcpClient.h"
#include "CommunicationSslClient.h"
#include "LogWriter.h"
#include "HttpClient.h"
#include "Response.h"


using namespace WebCpp;

HttpClient::HttpClient()
{

}

HttpClient::~HttpClient()
{
    HttpClient::Close();
}

bool HttpClient::Init()
{
    HttpConfig config;
    return Init(config);
}

bool HttpClient::Init(const HttpConfig &config)
{
    ClearError();

    m_config = config;

    return true;
}

bool HttpClient::Run()
{
    return true;
}

bool HttpClient::Close(bool wait)
{
    ClearError();
    bool retval = true;

    if(m_connection->Close(wait) == true)
    {
        SetLastError("close failed: " + m_connection->GetLastError());
        retval = false;
    }

    m_stateCallback = nullptr;
    SetState(State::Closed);

    return retval;
}

bool HttpClient::WaitFor()
{
    return m_connection->WaitFor();
}

bool HttpClient::Open(Request &request)
{
    ClearError();

    if(m_connection == nullptr)
    {
        if(InitConnection(request.GetUrl()) == false)
        {
            SetLastError("connection init failed: " + GetLastError());
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

    if(request.Send(m_connection) == false)
    {
        SetLastError("request sending error: " + request.GetLastError());
        LOG(GetLastError(), LogWriter::LogType::Error);
        return false;
    }

    SetState(State::DataSent);

    return true;
}

bool HttpClient::Open(Http::Method method, const std::string &url, const std::map<std::string, std::string> &headers)
{
    ClearError();

    Request request;
    auto &p_url = request.GetUrl();
    p_url.Parse(url);
    if(p_url.IsInitiaized())
    {
        request.SetMethod(method);
        for(auto &header: headers)
        {
            request.GetHeader().SetHeader(header.first, header.second);
        }

        return Open(request);
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

void HttpClient::SetStateCallback(const std::function<void (State)> &func)
{
    m_stateCallback = func;
}

void HttpClient::SetProgressCallback(const std::function<void (size_t, size_t)> &func)
{
    m_progressCallback = func;
}

HttpClient::State HttpClient::GetState() const
{
    return m_state;
}

void HttpClient::OnDataReady(const ByteArray &data)
{
    m_buffer.insert(m_buffer.end(), data.begin(), data.end());

    Response response(0, m_config);
    size_t all, downloaded;
    if(response.Parse(m_buffer, &all, &downloaded))
    {
        SetState(State::DataReady);

        if(m_responseCallback != nullptr)
        {
            m_responseCallback(response);
        }

        m_buffer.clear();
    }

    if(m_progressCallback)
    {
        m_progressCallback(all, downloaded);
    }
}

void HttpClient::OnClosed()
{
    m_buffer.clear();
    m_connection->Close();
    SetState(State::Closed);
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
            m_connection = std::make_shared<CommunicationTcpClient>();
            break;
#ifdef WITH_OPENSSL
        case Url::Scheme::HTTPS:
            m_connection = std::make_shared<CommunicationSslClient>(m_config.GetSslSertificate(), m_config.GetSslKey());
            break;
#endif
        default:
            break;
    }

    if(m_connection == nullptr)
    {
        SetLastError("requested scheme (" + Url::Scheme2String(url.GetScheme()) +  ") is incorrect or not supported");
        LOG(GetLastError(), LogWriter::LogType::Error);
        return false;
    }

    m_connection->SetHost(url.GetHost());
    m_connection->SetPort(url.GetPort());

    if(m_connection->Init() == false)
    {
        SetLastError("HttpServer init failed: " + m_connection->GetLastError());
        LOG(GetLastError(), LogWriter::LogType::Error);
        return false;
    }

    SetState(State::Initialized);

    auto f1 = std::bind(&HttpClient::OnDataReady, this, std::placeholders::_1);
    m_connection->SetDataReadyCallback(f1);
    auto f2 = std::bind(&HttpClient::OnClosed, this);
    m_connection->SetCloseConnectionCallback(f2);

    return true;
}

void HttpClient::SetState(State state)
{
    m_state = state;

    if(m_stateCallback != nullptr)
    {
        m_stateCallback(m_state);
    }
}
