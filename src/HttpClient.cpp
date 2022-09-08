#include "CommunicationTcpClient.h"
#include "CommunicationSslClient.h"
#include "LogWriter.h"
#include "HttpClient.h"
#include "Response.h"
#include "AuthFactory.h"


using namespace WebCpp;

HttpClient::HttpClient():
    m_config(WebCpp::HttpConfig::Instance()),
    m_authProvider(AuthProvider::Type::Client)
{

}

HttpClient::~HttpClient()
{
    HttpClient::Close();
}

bool HttpClient::Init()
{
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

    if(m_connection->Close(wait) == false)
    {
        SetLastError("close failed: " + m_connection->GetLastError());
        retval = false;
    }

    SetState(State::Closed);

    return retval;
}

bool HttpClient::WaitFor()
{
    return m_connection->WaitFor();
}

bool HttpClient::Open()
{
    ClearError();

    if(InitConnection(m_request.GetUrl()) == false)
    {
        SetLastError("connection init failed: " + GetLastError());
        LOG(GetLastError(), LogWriter::LogType::Error);
        SetState(State::Undefined);
        return false;
    }

    if(m_connection->IsConnected() == false)
    {
        if(m_connection->Connect() == false)
        {
            SetLastError("connection faied: " + m_connection->GetLastError());
            LOG(GetLastError(), LogWriter::LogType::Error);
            SetState(State::Undefined);
            return false;
        }

        SetState(State::Connected);
    }

    if(m_connection->Run() == false)
    {
        SetLastError("read routine failed: " + m_connection->GetLastError());
        LOG(GetLastError(), LogWriter::LogType::Error);
        SetState(State::Undefined);
        return false;
    }

    if(m_request.Send(m_connection) == false)
    {
        SetLastError("request sending error: " + m_request.GetLastError());
        LOG(GetLastError(), LogWriter::LogType::Error);
        SetState(State::Undefined);
        return false;
    }

    SetState(State::DataSent);
    return true;
}

bool HttpClient::Open(Http::Method method, const std::string &url, const std::map<std::string, std::string> &headers)
{
    ClearError();

    if(m_state == State::DataSent)
    {
        SetLastError("Already opened");
        return false;
    }

    m_request.Clear();

    auto &p_url = m_request.GetUrl();
    p_url.Parse(url);
    if(p_url.IsInitiaized())
    {
        m_request.SetMethod(method);
        for(auto &header: headers)
        {
            m_request.GetHeader().SetHeader(header.first, header.second);
        }

        if(m_authRequired)
        {
            AddAuthHeaders();
        }

        return Open();
    }
    else
    {
        SetLastError("Url parsing error");
        LOG(GetLastError(), LogWriter::LogType::Error);
    }

    return false;
}

void HttpClient::SetKeepOpen(bool value)
{
    m_keepOpen = value;
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

void HttpClient::SetAuthCallback(const std::function<bool (const Request &, AuthProvider &)> &func)
{
    m_authCallback = func;
}

HttpClient::State HttpClient::GetState() const
{
    return m_state;
}

void HttpClient::ClearAuth()
{
    m_authProvider.Clear();
}

void HttpClient::OnDataReady(const ByteArray &data)
{
    m_buffer.insert(m_buffer.end(), data.begin(), data.end());

    Response response(0, m_config);
    size_t all, downloaded;
    if(response.Parse(m_buffer, &all, &downloaded))
    {
        if(response.GetResponseCode() == 401)
        {
            LOG("Authentication required", LogWriter::LogType::Info);
            m_authRequired = true;
            m_authProvider.Clear();

            if(m_authCallback != nullptr)
            {
                auto list = response.GetHeader().GetAllHeaders("WWW-Authenticate");
                for(auto &scheme: list)
                {
                    m_authProvider.Parse(scheme);
                }

                if(m_authCallback(m_request, m_authProvider) == true)
                {
                    AddAuthHeaders();
                    if(Open() == false)
                    {
                        SetLastError("request with auth sending error: " + m_request.GetLastError());
                        LOG(GetLastError(), LogWriter::LogType::Error);
                    }

                    SetState(State::DataSent);
                }
            }
        }
        else
        {
            SetState(State::DataReady);
            if(m_responseCallback != nullptr)
            {
                m_responseCallback(response);
            }

            if(m_keepOpen == false)
            {
                Close(false);
            }
            else
            {
                SetState(State::Undefined);
            }
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
    SetState(State::Closed);
}

bool HttpClient::InitConnection(const Url &url)
{
    if(m_connection == nullptr || m_connection->IsInitialized() == false)
    {
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

bool HttpClient::AddAuthHeaders()
{
    IAuth *preferredScheme = nullptr;
    for(auto &scheme: m_authProvider.Get())
    {
        if(scheme->IsPreferred())
        {
            preferredScheme = scheme.get();
            break;
        }
    }

    if(preferredScheme != nullptr)
    {
        preferredScheme->AddAuthHeaders(m_request);
        return true;
    }

    return false;
}
