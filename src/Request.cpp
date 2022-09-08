#include <algorithm>
#include "Request.h"
#include "IHttp.h"
#include "Session.h"


#define EOL_LENGTH 2
#define ENTRY_DELIMITER_LENGTH 4


using namespace WebCpp;

Request::Request():
    m_header(HttpHeader::HeaderRole::Request)
{

}

bool Request::Parse(const ByteArray &data)
{
    ClearError();

    if(m_requestLineLength == 0)
    {
        if(ParseRequestLine(data, m_requestLineLength) == false)
        {
            SetLastError("Request: error parsing request line: " + GetLastError());
            return false;
        }
    }
    if(m_header.IsComplete() == false)
    {
        if(m_header.Parse(data, m_requestLineLength + EOL_LENGTH) == false)
        {
            SetLastError("Request: error parsing header: " + GetLastError());
            return false;
        }
    }

    if(m_header.GetBodySize() > 0)
    {
        return ParseBody(data, m_requestLineLength + EOL_LENGTH + m_header.GetHeaderSize() + ENTRY_DELIMITER_LENGTH);
    }

    return true;
}

bool Request::ParseRequestLine(const ByteArray &data, size_t &pos)
{
    pos = StringUtil::SearchPosition(data, { CR, LF });
    if(pos != SIZE_MAX) // request line presents
    {
        auto ranges = StringUtil::Split(data, { ' ' }, 0, pos);
        if(ranges.size() == 3)
        {                  
            m_method = Http::String2Method(std::string(data.begin() + ranges[0].start, data.begin() + ranges[0].end + 1));
            if(m_method == Http::Method::Undefined)
            {
                SetLastError("wrong method");
                return false;
            }
            m_url.Parse(std::string(data.begin() + ranges[1].start, data.begin() + ranges[1].end + 1), false);
            if(m_url.IsInitiaized() == false)
            {
                SetLastError("wrong URL");
                return false;
            }
            m_httpVersion = std::string(data.begin() + ranges[2].start, data.begin() + ranges[2].end + 1);
            StringUtil::Trim(m_httpVersion);
            return true;
        }
    }

    return false;
}


Request::Request(int connID, const std::string &remote):
    m_connID(connID),
    m_header(HttpHeader::HeaderRole::Request)
{
}

int Request::GetConnectionID() const
{
    return m_connID;
}

void Request::SetConnectionID(int connID)
{
    m_connID = connID;
}

const Url &Request::GetUrl() const
{
    return m_url;
}

Url &Request::GetUrl()
{
    return m_url;
}

const HttpHeader &Request::GetHeader() const
{
    return m_header;
}

HttpHeader &Request::GetHeader()
{
    return m_header;
}

Http::Method Request::GetMethod() const
{
    return m_method;
}

void Request::SetMethod(Http::Method method)
{
    m_method = method;
}

std::string Request::GetHttpVersion() const
{
    return m_httpVersion;
}


bool Request::ParseBody(const ByteArray &data, size_t headerSize)
{
    WebCpp::HttpConfig &config = WebCpp::HttpConfig::Instance();
    auto contentType = m_header.GetHeader(HttpHeader::HeaderType::ContentType);
    if(m_requestBody.Parse(data, headerSize, ByteArray(contentType.begin(), contentType.end()), config.GetTempFile()) == false)
    {
        SetLastError("body parsing error: " + m_requestBody.GetLastError());
        return false;
    }

    return true;
}

const RequestBody &Request::GetRequestBody() const
{
    return m_requestBody;
}

RequestBody &Request::GetRequestBody()
{
    return m_requestBody;
}

void Request::SetArg(const std::string &name, const std::string &value)
{
    m_args[name] = value;
}

Http::Protocol Request::GetProtocol() const
{
    if(m_header.GetHeader(HttpHeader::HeaderType::Upgrade) == "websocket")
    {
        return Http::Protocol::WS;
    }

    return Http::Protocol::HTTP;
}

size_t Request::GetRequestLineLength() const
{
    return m_requestLineLength;
}

size_t Request::GetRequestSize() const
{
    // Request line + CRLF (2 bytes) + Header + CRLFCRLF (4 bytes) + Body
    return m_requestLineLength + EOL_LENGTH + m_header.GetHeaderSize() + ENTRY_DELIMITER_LENGTH + m_header.GetBodySize();
}

std::string Request::GetRemote() const
{
    return m_remote;
}

void Request::SetRemote(const std::string &remote)
{
    m_remote = remote;
}

bool Request::Send(const std::shared_ptr<ICommunicationClient> &communication)
{
    ClearError();

    if(communication == nullptr)
    {
        SetLastError("connection not set");
        return false;
    }

    ByteArray header;

    const ByteArray &body = m_requestBody.ToByteArray();

    const ByteArray &rl = BuildRequestLine();
    header.insert(header.end(), rl.begin(), rl.end());

    auto &hd = GetHeader();
    if(body.size() > 0)
    {
        hd.SetHeader(HttpHeader::HeaderType::ContentType, m_requestBody.BuildContentType());
        hd.SetHeader(HttpHeader::HeaderType::ContentLength, std::to_string(body.size()));
    }

    hd.SetHeader(HttpHeader::HeaderType::UserAgent, WEBCPP_CANONICAL_NAME);
    hd.SetHeader(HttpHeader::HeaderType::Host, m_url.GetHost());
    hd.SetHeader(HttpHeader::HeaderType::Accept, "*/*");

    std::string encoding = "";
#ifdef WITH_ZLIB
    encoding += "gzip, deflate";
#endif
    if(!encoding.empty())
    {
        hd.SetHeader(HttpHeader::HeaderType::AcceptEncoding, encoding);
    }

    const ByteArray &h = m_header.ToByteArray();
    header.insert(header.end(), h.begin(), h.end());

    header.push_back(CR);
    header.push_back(LF);

    if(communication->Write(header) == false)
    {
        SetLastError("error sending header: " + communication->GetLastError());
        return false;
    }
    if(body.size() > 0)
    {
        if(communication->Write(body))
        {
            SetLastError("error sending body: " + communication->GetLastError());
            return false;
        }
    }

    return true;
}

void Request::Clear()
{
    m_connID = (-1);
    m_url.Clear();
    m_header.Clear();
    m_requestLineLength = 0;
    m_args.clear();
    m_requestBody.Clear();
    m_remote = "";
    m_session = nullptr;
}

void Request::SetSession(Session *session)
{
    m_session = session;
}

Session *Request::GetSession() const
{
    return m_session;
}

bool Request::CheckAuth()
{
    bool retval = false;
    if(m_session != nullptr)
    {
        auto &auth = m_session->authProvider;
        auto header = m_header.GetHeader(HttpHeader::HeaderType::Authorization);
        retval = auth.Check(header);
    }

    return retval;
}

ByteArray Request::BuildRequestLine() const
{
    const HttpHeader &header = GetHeader();
    std::string line = Http::Method2String(m_method) + " " + m_url.GetNormalizedPath() + (m_url.HasQuery() ? ("?" + m_url.Query2String()) : "") + " " + m_httpVersion + CR + LF;
    return StringUtil::String2ByteArray(line);
}

ByteArray Request::BuildHeaders() const
{
    auto &header = GetHeader();

    std::string headers;
    for(auto &entry: header.GetHeaders())
    {
        headers += entry.name + ": " + entry.value + CR + LF;
    }

    return ByteArray(headers.begin(), headers.end());
}

std::string Request::ToString() const
{
    return std::string("connID: ") + std::to_string(m_connID) + ", "
            + std::to_string(m_header.GetCount()) + " headers, body size: " + std::to_string(m_header.GetBodySize());
}

std::string Request::GetArg(const std::string &name) const
{
    if(m_args.find(name) == m_args.end())
    {
        return "";
    }

    return m_args.at(name);
}
