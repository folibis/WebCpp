#include <fstream>
#include "common.h"
#include "FileSystem.h"
#include "Response.h"

#define WRITE_BIFFER_SIZE 1024


#ifdef WITH_ZLIB
int inflate(const void *src, int srcLen, void *dst, int dstLen) {
    z_stream strm  = {0};
    strm.total_in  = strm.avail_in  = srcLen;
    strm.total_out = strm.avail_out = dstLen;
    strm.next_in   = (Bytef *) src;
    strm.next_out  = (Bytef *) dst;

    strm.zalloc = Z_NULL;
    strm.zfree  = Z_NULL;
    strm.opaque = Z_NULL;

    int err = -1;
    int ret = -1;

    err = inflateInit2(&strm, (15 + 32)); //15 window bits, and the +32 tells zlib to to detect if using gzip or zlib
    if (err == Z_OK) {
        err = inflate(&strm, Z_FINISH);
        if (err == Z_STREAM_END) {
            ret = strm.total_out;
        }
        else {
            inflateEnd(&strm);
            return err;
        }
    }
    else {
        inflateEnd(&strm);
        return err;
    }

    inflateEnd(&strm);
    return ret;
}
#endif


using namespace WebCpp;

Response::Response(int connID, const HttpConfig& config) :
    m_connID(connID),
    m_config(config)
{
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
    SetHeader(Response::HeaderType::ContentLength, std::to_string(m_body.size()));
}

void Response::Write(const std::string &data)
{
    Write(ByteArray(data.begin(), data.end()));
}

bool Response::AddFile(const std::string &file, const std::string &charset)
{
    bool retval = false;
    std::string path;

    if(FileSystem::IsFileExist(file) == false)
    {
        std::string root = FileSystem::NormalizePath(m_config.GetRoot());
        std::string root_full = FileSystem::NormalizePath(FileSystem::GetFullPath(root));
        if(root != root_full)
        {
            path = FileSystem::NormalizePath(FileSystem::GetApplicationFolder()) + root;
        }
        else
        {
            path = root;
        }
        path = path + file;
    }
    else
    {
        path = file;
    }

    if(FileSystem::IsFileExist(path))
    {
        std::string ext = FileSystem::ExtractFileExtension(path);
        SetHeader(Response::HeaderType::ContentType, Response::Extension2MimeType(ext) + ";charset=" + charset);
        SetHeader(Response::HeaderType::ContentLength, std::to_string(FileSystem::GetFileSize(path)));
        SetHeader(Response::HeaderType::LastModified, FileSystem::GetFileModifiedTime(path));
        m_file = path;
        retval = true;
    }

    return retval;
}

bool Response::SendNotFound()
{
    m_responseCode = 404;
    m_responsePhrase = Response::ResponseCode2String(m_responseCode);
    SetHeader(Response::HeaderType::ContentLength, "0");
    return true;
}

bool Response::SendRedirect(const std::string &url)
{
    m_responseCode = 301;
    m_responsePhrase = Response::ResponseCode2String(m_responseCode);
    SetHeader(Response::HeaderType::Location, url);
    SetHeader(Response::HeaderType::ContentLength, "0");
    return true;
}

void Response::SetResponseCode(uint16_t code)
{
    m_responseCode = code;
    m_responsePhrase = Response::ResponseCode2String(m_responseCode);
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

bool Response::Send(ICommunication *communication)
{
    ByteArray header;

    const ByteArray &sl = BuildStatusLine();
    header.insert(header.end(), sl.begin(), sl.end());

    const ByteArray &hdr = BuildHeaders();
    header.insert(header.end(), hdr.begin(), hdr.end());
    header.push_back(CR);
    header.push_back(LF);

    communication->Write(m_connID, header);

    if(!m_file.empty())
    {
        ByteArray buffer(WRITE_BIFFER_SIZE);
        std::ifstream stream(m_file, std::ios::binary);
        do
        {
            stream.read(buffer.data(), WRITE_BIFFER_SIZE);
            communication->Write(m_connID, buffer, stream.gcount());
        }
        while(stream);
    }
    else if(m_body.size() > 0)
    {
        communication->Write(m_connID, m_body);
    }

    return true;
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

std::string Response::ResponseCode2String(int code)
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

std::string Response::Extension2MimeType(const std::string &extension)
{
    switch(_(extension.c_str()))
    {
        case _("aac"):    return "audio/aac";
        case _("abw"):    return "application/x-abiword";
        case _("arc"):    return "application/x-freearc";
        case _("avi"):    return "video/x-msvideo";
        case _("azw"):    return "application/vnd.amazon.ebook";
        case _("bin"):    return "application/octet-stream";
        case _("bmp"):    return "image/bmp";
        case _("bz"):     return "application/x-bzip";
        case _("bz2"):    return "application/x-bzip2";
        case _("csh"):    return "application/x-csh";
        case _("css"):    return "text/css";
        case _("csv"):    return "text/csv";
        case _("doc"):    return "application/msword";
        case _("docx"):   return "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
        case _("eot"):    return "application/vnd.ms-fontobject";
        case _("epub"):   return "application/epub+zip";
        case _("gz"):     return "application/gzip";
        case _("gif"):    return "image/gif";
        case _("htm"):    return "text/html";
        case _("html"):   return "text/html";
        case _("ico"):    return "image/vnd.microsoft.icon";
        case _("ics"):    return "text/calendar";
        case _("jar"):    return "application/java-archive";
        case _("jpeg"):   return "image/jpeg";
        case _("jpg"):    return "image/jpeg";
        case _("js"):     return "text/javascript";
        case _("json"):   return "application/json";
        case _("jsonld"): return "application/ld+json";
        case _("mid"):    return "audio/midi audio/x-midi";
        case _("mjs"):    return "text/javascript";
        case _("mp3"):    return "audio/mpeg";
        case _("mpeg"):   return "video/mpeg";
        case _("mpkg"):   return "application/vnd.apple.installer+xml";
        case _("odp"):    return "application/vnd.oasis.opendocument.presentation";
        case _("ods"):    return "application/vnd.oasis.opendocument.spreadsheet";
        case _("odt"):    return "application/vnd.oasis.opendocument.text";
        case _("oga"):    return "audio/ogg";
        case _("ogv"):    return "video/ogg";
        case _("ogx"):    return "application/ogg";
        case _("opus"):   return "audio/opus";
        case _("otf"):    return "font/otf";
        case _("png"):    return "image/png";
        case _("pdf"):    return "application/pdf";
        case _("php"):    return "application/x-httpd-php";
        case _("ppt"):    return "application/vnd.ms-powerpoint";
        case _("pptx"):   return "application/vnd.openxmlformats-officedocument.presentationml.presentation";
        case _("rar"):    return "application/vnd.rar";
        case _("rtf"):    return "application/rtf";
        case _("sh"):     return "application/x-sh";
        case _("svg"):    return "image/svg+xml";
        case _("swf"):    return "application/x-shockwave-flash";
        case _("tar"):    return "application/x-tar";
        case _("tif"):    return "image/tiff";
        case _("tiff"):   return "image/tiff";
        case _("ts"):     return "video/mp2t";
        case _("ttf"):    return "font/ttf";
        case _("txt"):    return "text/plain";
        case _("vsd"):    return "application/vnd.visio";
        case _("wav"):    return "audio/wav";
        case _("weba"):   return "audio/webm";
        case _("webm"):   return "video/webm";
        case _("webp"):   return "image/webp";
        case _("woff"):   return "font/woff";
        case _("woff2"):  return "font/woff2";
        case _("xhtml"):  return "application/xhtml+xml";
        case _("xls"):    return "application/vnd.ms-excel";
        case _("xlsx"):   return "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
        case _("xml"):    return "application/xml";
        case _("xul"):    return "application/vnd.mozilla.xul+xml";
        case _("zip"):    return "application/zip";
        case _("3gp"):    return "video/3gpp";
        case _("3g2"):    return "video/3gpp2";
        case _("7z"):     return "application/x-7z-compressed";
    }

    return "application/octet-stream";
}
void Response::InitDefault()
{
    m_version = "HTTP/1.1";
    m_responseCode = 200;
    m_mimeType = "text/plain";
    SetHeader(Response::HeaderType::Server, m_config.GetServerName());
}

ByteArray Response::BuildStatusLine() const
{
    std::string statusLine = m_version + " " + std::to_string(m_responseCode) + " " + m_responsePhrase + CR + LF;
    return ByteArray(statusLine.begin(), statusLine.end());
}

ByteArray Response::BuildHeaders() const
{
    std::string headers;
    for(auto &pair: m_headers)
    {
        headers += pair.first + ": " + pair.second + CR + LF;
    }

    return ByteArray(headers.begin(), headers.end());
}
