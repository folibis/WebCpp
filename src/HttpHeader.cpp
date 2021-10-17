#include <algorithm>
#include "defines_webcpp.h"
#include "StringUtil.h"
#include "HttpHeader.h"


using namespace WebCpp;

HttpHeader::Header HttpHeader::Header::defaultHeader = HttpHeader::Header();

HttpHeader::HttpHeader(HeaderRole role)
{

}

bool HttpHeader::Parse(const ByteArray &data, size_t start)
{
    m_complete = false;

    size_t pos = StringUtil::SearchPosition(data, { CRLFCRLF }, start);
    if(pos != SIZE_MAX)
    {
        auto arr = StringUtil::Split(data, { CRLF }, start, pos);
        if(ParseHeaders(data, arr))
        {
            m_headerSize = pos - start;
            m_complete = true;
        }
    }

    return m_complete;
}

bool HttpHeader::ParseHeader(const ByteArray &data)
{
    auto arr = StringUtil::Split(data, { CRLF }, 0, data.size());
    return ParseHeaders(data, arr);
}

ByteArray HttpHeader::ToByteArray() const
{
    std::string headers;
    for(auto const &header: m_headers)
    {
        headers += header.name + ": " + header.value + CR + LF;
    }

    return ByteArray(headers.begin(), headers.end());
}

bool HttpHeader::IsComplete() const
{
    return m_complete;
}

size_t HttpHeader::GetHeaderSize() const
{
    return m_headerSize;
}

size_t HttpHeader::GetBodySize() const
{
    size_t size = 0;

    if(m_complete)
    {
        auto str = GetHeader(HeaderType::ContentLength);
        int num;
        if(StringUtil::String2int(str, num))
        {
            size = num;
        }
    }

    return size;
}

size_t HttpHeader::GetRequestSize() const
{
    return GetHeaderSize() + GetBodySize() + 4; // header + delimiter(CRLFCRLF, 4 bytes) + body
}

HttpHeader::HeaderRole HttpHeader::GetRole() const
{
    return m_role;
}

void HttpHeader::SetVersion(const std::string &version)
{
    m_version = version;
}

bool HttpHeader::ParseHeaders(const ByteArray &data, const StringUtil::Ranges &ranges)
{
    bool statusLineParsed = false;

    for(auto &range: ranges)
    {
        auto headerDelimiter = StringUtil::SearchPosition(data, { ':' }, range.start, range.end);
        if(headerDelimiter != SIZE_MAX)
        {
            std::string name = std::string(data.begin() + range.start, data.begin() + headerDelimiter);
            std::string value = std::string(data.begin() + headerDelimiter + 1, data.begin() + range.end + 1);
            StringUtil::Trim(name);
            StringUtil::Trim(value);

            SetHeader(name, value);
        }
    }

    return true;
}



HttpHeader::HeaderType HttpHeader::String2HeaderType(const std::string &str)
{
    switch(_(str.c_str()))
    {
        case _("Accept"):              return HttpHeader::HeaderType::Accept;
        case _("Accept-Charset"):      return HttpHeader::HeaderType::AcceptCharset;
        case _("Accept-Encoding"):     return HttpHeader::HeaderType::AcceptEncoding;
        case _("Accept-Datetime"):     return HttpHeader::HeaderType::AcceptDatetime;
        case _("Accept-Language"):     return HttpHeader::HeaderType::AcceptLanguage;
        case _("Authorization"):       return HttpHeader::HeaderType::Authorization;
        case _("Cache-Control"):       return HttpHeader::HeaderType::CacheControl;
        case _("Connection"):          return HttpHeader::HeaderType::Connection;
        case _("Content-Encoding"):    return HttpHeader::HeaderType::ContentEncoding;
        case _("Content-Length"):      return HttpHeader::HeaderType::ContentLength;
        case _("Content-MD5"):         return HttpHeader::HeaderType::ContentMD5;
        case _("Content-Type"):        return HttpHeader::HeaderType::ContentType;
        case _("Cookie"):              return HttpHeader::HeaderType::Cookie;
        case _("Date"):                return HttpHeader::HeaderType::Date;
        case _("Expect"):              return HttpHeader::HeaderType::Expect;
        case _("Forwarded"):           return HttpHeader::HeaderType::Forwarded;
        case _("From"):                return HttpHeader::HeaderType::From;
        case _("HTTP2-Settings"):      return HttpHeader::HeaderType::HTTP2Settings;
        case _("Host"):                return HttpHeader::HeaderType::Host;
        case _("If-Match"):            return HttpHeader::HeaderType::IfMatch;
        case _("If-Modified-Since"):   return HttpHeader::HeaderType::IfModifiedSince;
        case _("If-None-Match"):       return HttpHeader::HeaderType::IfNoneMatch;
        case _("If-Range"):            return HttpHeader::HeaderType::IfRange;
        case _("If-Unmodified-Since"): return HttpHeader::HeaderType::IfUnmodifiedSince;
        case _("Max-Forwards"):        return HttpHeader::HeaderType::MaxForwards;
        case _("Origin"):              return HttpHeader::HeaderType::Origin;
        case _("Pragma"):              return HttpHeader::HeaderType::Pragma;
        case _("Prefer"):              return HttpHeader::HeaderType::Prefer;
        case _("Proxy-Authorization"): return HttpHeader::HeaderType::ProxyAuthorization;
        case _("Range"):               return HttpHeader::HeaderType::Range;
        case _("Referer"):             return HttpHeader::HeaderType::Referer;
        case _("TE"):                  return HttpHeader::HeaderType::TE;
        case _("Trailer"):             return HttpHeader::HeaderType::Trailer;
        case _("TransferEncoding"):    return HttpHeader::HeaderType::TransferEncoding;
        case _("User-Agent"):          return HttpHeader::HeaderType::UserAgent;
        case _("Upgrade"):             return HttpHeader::HeaderType::Upgrade;
        case _("Via"):                 return HttpHeader::HeaderType::Via;
        case _("Warning"):             return HttpHeader::HeaderType::Warning;

        case _("Accept-CH"):                       return HeaderType::AcceptCH;
        case _("Access-Control-Allow-Origin"):     return HeaderType::AccessControlAllowOrigin;
        case _("Access-Control-Allow-Credential"): return HeaderType::AccessControlAllowCredentials;
        case _("Access-Control-Expose-Headers"):   return HeaderType::AccessControlExposeHeaders;
        case _("Access-Control-Max-Age"):          return HeaderType::AccessControlMaxAge;
        case _("Access-Control-Allow-Methods"):    return HeaderType::AccessControlAllowMethods;
        case _("Access-Control-Allow-Headers"):    return HeaderType::AccessControlAllowHeaders;
        case _("Accept-Patch"):                    return HeaderType::AcceptPatch;
        case _("Accept-Ranges"):                   return HeaderType::AcceptRanges;
        case _("Age"):                             return HeaderType::Age;
        case _("Allow"):                           return HeaderType::Allow;
        case _("Alt-Svc"):                         return HeaderType::AltSvc;
        case _("Content-Disposition"):             return HeaderType::ContentDisposition;
        case _("Content-Language"):                return HeaderType::ContentLanguage;
        case _("Content-Location"):                return HeaderType::ContentLocation;
        case _("Content-Range"):                   return HeaderType::ContentRange;
        case _("Delta-Base"):                      return HeaderType::DeltaBase;
        case _("ETag"):                            return HeaderType::ETag;
        case _("Expires"):                         return HeaderType::Expires;
        case _("IM"):                              return HeaderType::IM;
        case _("Last-Modified"):                   return HeaderType::LastModified;
        case _("Link"):                            return HeaderType::Link;
        case _("Location"):                        return HeaderType::Location;
        case _("P3P"):                             return HeaderType::P3P;
        case _("Preference-Applied"):              return HeaderType::PreferenceApplied;
        case _("Proxy-Authenticate"):              return HeaderType::ProxyAuthenticate;
        case _("Public-Key-Pins"):                 return HeaderType::PublicKeyPins;
        case _("Retry-After"):                     return HeaderType::RetryAfter;
        case _("Server"):                          return HeaderType::Server;
        case _("Set-Cookie"):                      return HeaderType::SetCookie;
        case _("Strict-Transport-Security"):       return HeaderType::StrictTransportSecurity;
        case _("Transfer-Encoding"):               return HeaderType::TransferEncoding;
        case _("Tk"):                              return HeaderType::Tk;
        case _("Vary"):                            return HeaderType::Vary;
        case _("WWW-Authenticate"):                return HeaderType::WWWAuthenticate;
        case _("X-Frame-Options"):                 return HeaderType::XFrameOptions;

        default: break;
    }

    return HttpHeader::HeaderType::Undefined;
}

std::string HttpHeader::HeaderType2String(HttpHeader::HeaderType headerType)
{
    switch(headerType)
    {
        case HttpHeader::HeaderType::Accept:              return "Accept";
        case HttpHeader::HeaderType::AcceptCharset:       return "Accept-Charset";
        case HttpHeader::HeaderType::AcceptEncoding:      return "Accept-Encoding";
        case HttpHeader::HeaderType::AcceptDatetime:      return "Accept-Datetime";
        case HttpHeader::HeaderType::AcceptLanguage:      return "Accept-Language";
        case HttpHeader::HeaderType::Authorization:       return "Authorization";
        case HttpHeader::HeaderType::CacheControl:        return "Cache-Control";
        case HttpHeader::HeaderType::Connection:          return "Connection";
        case HttpHeader::HeaderType::ContentEncoding:     return "Content-Encoding";
        case HttpHeader::HeaderType::ContentLength:       return "Content-Length";
        case HttpHeader::HeaderType::ContentMD5:          return "Content-MD5";
        case HttpHeader::HeaderType::ContentType:         return "Content-Type";
        case HttpHeader::HeaderType::Cookie:              return "Cookie";
        case HttpHeader::HeaderType::Date:                return "Date";
        case HttpHeader::HeaderType::Expect:              return "Expect";
        case HttpHeader::HeaderType::Forwarded:           return "Forwarded";
        case HttpHeader::HeaderType::From:                return "From";
        case HttpHeader::HeaderType::HTTP2Settings:       return "HTTP2-Settings";
        case HttpHeader::HeaderType::Host:                return "Host";
        case HttpHeader::HeaderType::IfMatch:             return "If-Match";
        case HttpHeader::HeaderType::IfModifiedSince:     return "If-Modified-Since";
        case HttpHeader::HeaderType::IfNoneMatch:         return "If-None-Match";
        case HttpHeader::HeaderType::IfRange:             return "If-Range";
        case HttpHeader::HeaderType::IfUnmodifiedSince:   return "If-Unmodified-Since";
        case HttpHeader::HeaderType::MaxForwards:         return "Max-Forwards";
        case HttpHeader::HeaderType::Origin:              return "Origin";
        case HttpHeader::HeaderType::Pragma:              return "Pragma";
        case HttpHeader::HeaderType::Prefer:              return "Prefer";
        case HttpHeader::HeaderType::ProxyAuthorization:  return "Proxy-Authorization";
        case HttpHeader::HeaderType::Range:               return "Range";
        case HttpHeader::HeaderType::Referer:             return "Referer";
        case HttpHeader::HeaderType::TE:                  return "TE";
        case HttpHeader::HeaderType::Trailer:             return "Trailer";
        case HttpHeader::HeaderType::TransferEncoding:    return "Transfer-Encoding";
        case HttpHeader::HeaderType::UserAgent:           return "User-Agent";
        case HttpHeader::HeaderType::Upgrade:             return "Upgrade";
        case HttpHeader::HeaderType::Via:                 return "Via";
        case HttpHeader::HeaderType::Warning:             return "Warning";

        case HeaderType::AcceptCH :                     return "Accept-CH";
        case HeaderType::AccessControlAllowOrigin:      return "Access-Control-Allow-Origin";
        case HeaderType::AccessControlAllowCredentials: return "Access-Control-Allow-Credential";
        case HeaderType::AccessControlExposeHeaders:    return "Access-Control-Expose-Headers";
        case HeaderType::AccessControlMaxAge:           return "Access-Control-Max-Age";
        case HeaderType::AccessControlAllowMethods:     return "Access-Control-Allow-Methods";
        case HeaderType::AccessControlAllowHeaders:     return "Access-Control-Allow-Headers";
        case HeaderType::AcceptPatch:                   return "Accept-Patch";
        case HeaderType::AcceptRanges:                  return "Accept-Ranges";
        case HeaderType::Age :                          return "Age";
        case HeaderType::Allow:                         return "Allow";
        case HeaderType::AltSvc:                        return "Alt-Svc";
        case HeaderType::ContentDisposition:            return "Content-Disposition";
        case HeaderType::ContentLanguage:               return "Content-Language";
        case HeaderType::ContentLocation:               return "Content-Location";
        case HeaderType::ContentRange:                  return "Content-Range";
        case HeaderType::DeltaBase:                     return "Delta-Base";
        case HeaderType::ETag:                          return "ETag";
        case HeaderType::Expires:                       return "Expires";
        case HeaderType::IM:                            return "IM";
        case HeaderType::LastModified:                  return "Last-Modified";
        case HeaderType::Link:                          return "Link";
        case HeaderType::Location:                      return "Location";
        case HeaderType::P3P:                           return "P3P";
        case HeaderType::PreferenceApplied:             return "Preference-Applied";
        case HeaderType::ProxyAuthenticate:             return "Proxy-Authenticate";
        case HeaderType::PublicKeyPins:                 return "Public-Key-Pins";
        case HeaderType::RetryAfter:                    return "Retry-After";
        case HeaderType::Server:                        return "Server";
        case HeaderType::SetCookie:                     return "Set-Cookie";
        case HeaderType::StrictTransportSecurity:       return "Strict-Transport-Security";
        case HeaderType::Tk:                            return "Tk";
        case HeaderType::Vary:                          return "Vary";
        case HeaderType::WWWAuthenticate:               return "WWW-Authenticate";
        case HeaderType::XFrameOptions:                 return "X-Frame-Options";
        default: break;
    }

    return "";
}

std::string HttpHeader::ToString() const
{
    return "Header (" + std::to_string(m_headers.size()) + " records, ver. " + m_version + ", size: " + std::to_string(m_headerSize) + ")";
}

const std::vector<HttpHeader::Header> &HttpHeader::GetHeaders() const
{
    return m_headers;
}

void HttpHeader::SetHeader(HeaderType type, const std::string &value)
{
    SetHeader(HeaderType2String(type), value);
}

void HttpHeader::SetHeader(const std::string &name, const std::string &value)
{
    for(auto &header: m_headers)
    {
        if(header.name == name)
        {
            header.value = value;
            return;
        }
    }
    HttpHeader::Header header;
    header.type = String2HeaderType(name);
    header.name = name;
    header.value = value;
    m_headers.push_back(std::move(header));
}

std::string HttpHeader::GetHeader(HeaderType headerType) const
{
    return GetHeader(HttpHeader::HeaderType2String(headerType));
}

std::string HttpHeader::GetHeader(const std::string &headerType) const
{
    for(auto &header: m_headers)
    {
        if(header.name == headerType)
        {
            return header.value;
        }
    }

    return "";
}

std::string HttpHeader::GetVersion() const
{
    return m_version;
}

std::string HttpHeader::GetRemote() const
{
    if(m_remotePort != (-1))
    {
        return m_remoteAddress + ":" + std::to_string(m_remotePort);
    }

    return m_remoteAddress;
}

void HttpHeader::SetRemote(const std::string &address)
{
    auto list = StringUtil::Split(address,':');
    if(list.size() == 2)
    {
        m_remoteAddress = list[0];
        int port;
        if(StringUtil::String2int(list[0], port))
        {
            m_remotePort = port;
        }
    }
}

std::string HttpHeader::GetRemoteAddress() const
{
    return m_remoteAddress;
}

int HttpHeader::GetRemotePort() const
{
    return m_remotePort;
}

int HttpHeader::GetCount() const
{
    return m_headers.size();
}
