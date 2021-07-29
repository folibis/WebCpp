#include "common.h"
#include "Response.h"


using namespace WebCpp;

Response::Response(const std::string &version)
{
    m_version = version;
    InitDefault();
}

void Response::SetHeader(const std::string &name, const std::string &value)
{
    m_headers[name] = value;
}

void Response::SetHeader(Response::HeaderType header, const std::string &value)
{
    SetHeader(HeaderType2String(header), value);
}

void Response::Write(const ByteArray &data)
{
    m_body.insert(m_body.end(), data.begin(), data.end());
}

void Response::Write(const std::string &data)
{
    Write(ByteArray(data.begin(), data.end()));
}

void Response::SetResponseCode(uint16_t code, const std::string &phrase)
{
    m_responseCode = code;
    m_responsePhrase = phrase;
}

uint16_t Response::GetResponseCode() const
{
    return m_responseCode;
}

std::string Response::HeaderType2String(Response::HeaderType headerType)
{
    switch(headerType)
    {
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
        case HeaderType::CacheControl:                  return "Cache-Control";
        case HeaderType::Connection:                    return "Connection";
        case HeaderType::ContentDisposition:            return "Content-Disposition";
        case HeaderType::ContentEncoding:               return "Content-Encoding";
        case HeaderType::ContentLanguage:               return "Content-Language";
        case HeaderType::ContentLength:                 return "Content-Length";
        case HeaderType::ContentLocation:               return "Content-Location";
        case HeaderType::ContentMD5:                    return "Content-MD5";
        case HeaderType::ContentRange:                  return "Content-Range";
        case HeaderType::ContentType:                   return "Content-Type";
        case HeaderType::Date:                          return "Date";
        case HeaderType::DeltaBase:                     return "Delta-Base";
        case HeaderType::ETag:                          return "ETag";
        case HeaderType::Expires:                       return "Expires";
        case HeaderType::IM:                            return "IM";
        case HeaderType::LastModified:                  return "Last-Modified";
        case HeaderType::Link:                          return "Link";
        case HeaderType::Location:                      return "Location";
        case HeaderType::P3P:                           return "P3P";
        case HeaderType::Pragma:                        return "Pragma";
        case HeaderType::PreferenceApplied:             return "Preference-Applied";
        case HeaderType::ProxyAuthenticate:             return "Proxy-Authenticate";
        case HeaderType::PublicKeyPins:                 return "Public-Key-Pins";
        case HeaderType::RetryAfter:                    return "Retry-After";
        case HeaderType::Server:                        return "Server";
        case HeaderType::SetCookie:                     return "Set-Cookie";
        case HeaderType::StrictTransportSecurity:       return "Strict-Transport-Security";
        case HeaderType::Trailer:                       return "Trailer";
        case HeaderType::TransferEncoding:              return "Transfer-Encoding";
        case HeaderType::Tk:                            return "Tk";
        case HeaderType::Upgrade:                       return "Upgrade";
        case HeaderType::Vary:                          return "Vary";
        case HeaderType::Via:                           return "Via";
        case HeaderType::Warning:                       return "Warning";
        case HeaderType::WWWAuthenticate:               return "WWW-Authenticate";
        case HeaderType::XFrameOptions:                 return "X-Frame-Options";
        default: break;
    }

    return "";
}

Response::HeaderType Response::String2HeaderType(const std::string &str)
{
    switch(_(str.c_str()))
    {
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
        case _("Cache-Control"):                   return HeaderType::CacheControl;
        case _("Connection"):                      return HeaderType::Connection;
        case _("Content-Disposition"):             return HeaderType::ContentDisposition;
        case _("Content-Encoding"):                return HeaderType::ContentEncoding;
        case _("Content-Language"):                return HeaderType::ContentLanguage;
        case _("Content-Length"):                  return HeaderType::ContentLength;
        case _("Content-Location"):                return HeaderType::ContentLocation;
        case _("Content-MD5"):                     return HeaderType::ContentMD5;
        case _("Content-Range"):                   return HeaderType::ContentRange;
        case _("Content-Type"):                    return HeaderType::ContentType;
        case _("Date"):                            return HeaderType::Date;
        case _("Delta-Base"):                      return HeaderType::DeltaBase;
        case _("ETag"):                            return HeaderType::ETag;
        case _("Expires"):                         return HeaderType::Expires;
        case _("IM"):                              return HeaderType::IM;
        case _("Last-Modified"):                   return HeaderType::LastModified;
        case _("Link"):                            return HeaderType::Link;
        case _("Location"):                        return HeaderType::Location;
        case _("P3P"):                             return HeaderType::P3P;
        case _("Pragma"):                          return HeaderType::Pragma;
        case _("Preference-Applied"):              return HeaderType::PreferenceApplied;
        case _("Proxy-Authenticate"):              return HeaderType::ProxyAuthenticate;
        case _("Public-Key-Pins"):                 return HeaderType::PublicKeyPins;
        case _("Retry-After"):                     return HeaderType::RetryAfter;
        case _("Server"):                          return HeaderType::Server;
        case _("Set-Cookie"):                      return HeaderType::SetCookie;
        case _("Strict-Transport-Security"):       return HeaderType::StrictTransportSecurity;
        case _("Trailer"):                         return HeaderType::Trailer;
        case _("Transfer-Encoding"):               return HeaderType::TransferEncoding;
        case _("Tk"):                              return HeaderType::Tk;
        case _("Upgrade"):                         return HeaderType::Upgrade;
        case _("Vary"):                            return HeaderType::Vary;
        case _("Via"):                             return HeaderType::Via;
        case _("Warning"):                         return HeaderType::Warning;
        case _("WWW-Authenticate"):                return HeaderType::WWWAuthenticate;
        case _("X-Frame-Options"):                 return HeaderType::XFrameOptions;
        default: break;
    }

    return HeaderType::Undefined;
}

std::string Response::ResponseCode(int code)
{
    switch(code)
    {
        case 100: return "Continue";
        case 101: return "Switching Protocols";
        case 200: return "OK";
        case 201: return "Created";
        case 202: return "Accepted";
        case 203: return "Non-Authoritative Information";
        case 204: return "No Content";
        case 205: return "Reset Content";
        case 206: return "Partial Content";
        case 300: return "Multiple Choices";
        case 301: return "Moved Permanently";
        case 302: return "Found";
        case 303: return "See Other";
        case 304: return "Not Modified";
        case 305: return "Use Proxy";
        case 307: return "Temporary Redirect";
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 402: return "Payment Required";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 407: return "Proxy Authentication Required";
        case 408: return "Request Time-out";
        case 409: return "Conflict";
        case 410: return "Gone";
        case 411: return "Length Required";
        case 412: return "Precondition Failed";
        case 413: return "Request Entity Too Large";
        case 414: return "Request-URI Too Large";
        case 415: return "Unsupported Media Type";
        case 416: return "Requested range not satisfiable";
        case 417: return "Expectation Failed";
        case 500: return "Internal Server Error";
        case 501: return "Not Implemented";
        case 502: return "Bad Gateway";
        case 503: return "Service Unavailable";
        case 504: return "Gateway Time-out";
        case 505: return "HTTP Version not supported";
        default: break;
    }

    return "";
}

void Response::InitDefault()
{
    m_responseCode = 200;
}
