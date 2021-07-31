#include <algorithm>
#include "Request.h"

using namespace WebCpp;

Request::Header Request::Header::defaultHeader = Request::Header();

Request::Request(int connID, const ByteArray &request, const HttpConfig& config):
    m_connID(connID),
    m_config(config)
{
    Init(request);
}

int Request::GetConnectionID() const
{
    return m_connID;
}

void Request::Init(const ByteArray &data)
{
    size_t pos;
    if(look_for(data, { CRLFCRLF }, pos))
    {
        m_data = ByteArray(pos, data.size());
        auto arr = split(data, { CRLF }, pos);
        ParseHeaders(arr);
    }
    else
    {
        auto arr = split(data, { CRLF });
        ParseHeaders(arr);
    }
}

bool Request::ParseHeaders(std::vector<ByteArray> &arr)
{
    for(auto &line: arr)
    {
        if(m_method == Request::Method::Undefined)
        {
            std::string s(line.begin(), line.end());
            auto methodArr = split(s, ' ');
            if(methodArr.size() < 3)
            {
                return false;
            }

            trim(methodArr[0]);
            m_method = Request::String2Method(methodArr[0]);
            if(m_method == Request::Method::Undefined)
            {
                return false;
            }

            trim(methodArr[1]);
            m_uri = methodArr[1];
            ParseQuery();

            trim(methodArr[2]);
            m_version = methodArr[2];
        }
        else
        {
            auto it = std::find(line.begin(), line.end(), ':');
            if(it != line.end())
            {
                std::string name = std::string(line.begin(), it);
                std::string value = std::string(it + 1, line.end());
                trim(name);
                trim(value);

                Request::Header header;
                header.name = name;
                header.type = String2HeaderType(name);
                header.value = value;

                m_headers.push_back(std::move(header));
            }
        }
    }

    return true;
}

void Request::ParseQuery()
{
    auto pos = m_uri.find('?');

    if(pos != std::string::npos)
    {
        m_path = std::string(m_uri.begin(), m_uri.begin() + pos);
        std::string query = std::string(m_uri.begin() + pos + 1, m_uri.end());
        auto q = split(query, '&');
        for(auto &token: q)
        {
            auto parr = split(token, '=');
            if(parr.size() == 2)
            {
                m_query[parr[0]] = parr[1];
            }
        }
    }
}

Request::Method Request::GetMethod() const
{
    return m_method;
}

std::string Request::GetUri() const
{
    return m_uri;
}

std::string Request::GetPath() const
{
    return m_path;
}

const std::vector<Request::Header> &Request::GetHeaders() const
{
    return m_headers;
}

const Request::Header &Request::GetHeader(Request::HeaderType headerType) const
{
    for(auto &header: m_headers)
    {
        if(header.type == headerType)
        {
            return header;
        }
    }

    return Request::Header::defaultHeader;
}

const Request::Header &Request::GetHeader(const std::string &headerType) const
{
    for(auto &header: m_headers)
    {
        if(header.name == headerType)
        {
            return header;
        }
    }

    return Request::Header::defaultHeader;
}

std::string Request::GetVersion() const
{
    return m_version;
}

std::string Request::GetHost() const
{
    return m_host;
}

const ByteArray& Request::GetData() const
{
    return m_data;
}

void Request::SetArg(const std::string &name, const std::string &value)
{
    m_args[name] = value;
}

std::string Request::GetArg(const std::string &name) const
{
    if(m_args.find(name) == m_args.end())
    {
        return "";
    }

    return m_args.at(name);
}

std::string Request::Method2String(Request::Method method)
{
    switch(method)
    {
        case Request::Method::OPTIONS: return "OPTIONS";
        case Request::Method::GET:     return "GET";
        case Request::Method::HEAD:    return "HEAD";
        case Request::Method::POST:    return "POST";
        case Request::Method::PUT:     return "PUT";
        case Request::Method::DELETE:  return "DELETE";
        case Request::Method::TRACE:   return "TRACE";
        case Request::Method::CONNECT: return "CONNECT";
        default:
            break;
    }

    return "";
}

Request::Method Request::String2Method(const std::string &str)
{
    std::string s = str;
    toUpper(s);

    switch(_(s.c_str()))
    {
        case _("OPTIONS"): return Request::Method::OPTIONS;
        case _("GET"):     return Request::Method::GET;
        case _("HEAD"):    return Request::Method::HEAD;
        case _("POST"):    return Request::Method::POST;
        case _("PUT"):     return Request::Method::PUT;
        case _("DELETE"):  return Request::Method::DELETE;
        case _("TRACE"):   return Request::Method::TRACE;
        case _("CONNECT"): return Request::Method::CONNECT;
        default:
            break;
    }

    return Request::Method::Undefined;
}

std::string Request::HeaderType2String(Request::HeaderType headerType)
{
    switch(headerType)
    {
        case Request::HeaderType::Accept:              return "Accept";
        case Request::HeaderType::AcceptCharset:       return "Accept-Charset";
        case Request::HeaderType::AcceptEncoding:      return "Accept-Encoding";
        case Request::HeaderType::AcceptDatetime:      return "Accept-Datetime";
        case Request::HeaderType::AcceptLanguage:      return "Accept-Language";
        case Request::HeaderType::Authorization:       return "Authorization";
        case Request::HeaderType::CacheControl:        return "Cache-Control";
        case Request::HeaderType::Connection:          return "Connection";
        case Request::HeaderType::ContentEncoding:     return "Content-Encoding";
        case Request::HeaderType::ContentLength:       return "Content-Length";
        case Request::HeaderType::ContentMD5:          return "Content-MD5";
        case Request::HeaderType::ContentType:         return "Content-Type";
        case Request::HeaderType::Cookie:              return "Cookie";
        case Request::HeaderType::Date:                return "Date";
        case Request::HeaderType::Expect:              return "Expect";
        case Request::HeaderType::Forwarded:           return "Forwarded";
        case Request::HeaderType::From:                return "From";
        case Request::HeaderType::HTTP2Settings:       return "HTTP2-Settings";
        case Request::HeaderType::Host:                return "Host";
        case Request::HeaderType::IfMatch:             return "If-Match";
        case Request::HeaderType::IfModifiedSince:     return "If-Modified-Since";
        case Request::HeaderType::IfNoneMatch:         return "If-None-Match";
        case Request::HeaderType::IfRange:             return "If-Range";
        case Request::HeaderType::IfUnmodifiedSince:   return "If-Unmodified-Since";
        case Request::HeaderType::MaxForwards:         return "Max-Forwards";
        case Request::HeaderType::Origin:              return "Origin";
        case Request::HeaderType::Pragma:              return "Pragma";
        case Request::HeaderType::Prefer:              return "Prefer";
        case Request::HeaderType::ProxyAuthorization:  return "Proxy-Authorization";
        case Request::HeaderType::Range:               return "Range";
        case Request::HeaderType::Referer:             return "Referer";
        case Request::HeaderType::TE:                  return "TE";
        case Request::HeaderType::Trailer:             return "Trailer";
        case Request::HeaderType::TransferEncoding:    return "TransferEncoding";
        case Request::HeaderType::UserAgent:           return "User-Agent";
        case Request::HeaderType::Upgrade:             return "Upgrade";
        case Request::HeaderType::Via:                 return "Via";
        case Request::HeaderType::Warning:             return "Warning";
        default: break;
    }

    return "";
}

Request::HeaderType Request::String2HeaderType(const std::string &str)
{
    switch(_(str.c_str()))
    {
        case _("Accept"):              return Request::HeaderType::Accept;
        case _("Accept-Charset"):      return Request::HeaderType::AcceptCharset;
        case _("Accept-Encoding"):     return Request::HeaderType::AcceptEncoding;
        case _("Accept-Datetime"):     return Request::HeaderType::AcceptDatetime;
        case _("Accept-Language"):     return Request::HeaderType::AcceptLanguage;
        case _("Authorization"):       return Request::HeaderType::Authorization;
        case _("Cache-Control"):       return Request::HeaderType::CacheControl;
        case _("Connection"):          return Request::HeaderType::Connection;
        case _("Content-Encoding"):    return Request::HeaderType::ContentEncoding;
        case _("Content-Length"):      return Request::HeaderType::ContentLength;
        case _("Content-MD5"):         return Request::HeaderType::ContentMD5;
        case _("Content-Type"):        return Request::HeaderType::ContentType;
        case _("Cookie"):              return Request::HeaderType::Cookie;
        case _("Date"):                return Request::HeaderType::Date;
        case _("Expect"):              return Request::HeaderType::Expect;
        case _("Forwarded"):           return Request::HeaderType::Forwarded;
        case _("From"):                return Request::HeaderType::From;
        case _("HTTP2-Settings"):      return Request::HeaderType::HTTP2Settings;
        case _("Host"):                return Request::HeaderType::Host;
        case _("If-Match"):            return Request::HeaderType::IfMatch;
        case _("If-Modified-Since"):   return Request::HeaderType::IfModifiedSince;
        case _("If-None-Match"):       return Request::HeaderType::IfNoneMatch;
        case _("If-Range"):            return Request::HeaderType::IfRange;
        case _("If-Unmodified-Since"): return Request::HeaderType::IfUnmodifiedSince;
        case _("Max-Forwards"):        return Request::HeaderType::MaxForwards;
        case _("Origin"):              return Request::HeaderType::Origin;
        case _("Pragma"):              return Request::HeaderType::Pragma;
        case _("Prefer"):              return Request::HeaderType::Prefer;
        case _("Proxy-Authorization"): return Request::HeaderType::ProxyAuthorization;
        case _("Range"):               return Request::HeaderType::Range;
        case _("Referer"):             return Request::HeaderType::Referer;
        case _("TE"):                  return Request::HeaderType::TE;
        case _("Trailer"):             return Request::HeaderType::Trailer;
        case _("TransferEncoding"):    return Request::HeaderType::TransferEncoding;
        case _("User-Agent"):          return Request::HeaderType::UserAgent;
        case _("Upgrade"):             return Request::HeaderType::Upgrade;
        case _("Via"):                 return Request::HeaderType::Via;
        case _("Warning"):             return Request::HeaderType::Warning;
        default: break;
    }

    return Request::HeaderType::Undefined;
}
