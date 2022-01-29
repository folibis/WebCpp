#include "common_webcpp.h"
#include "defines_webcpp.h"
#include "FileSystem.h"
#include "File.h"
#include "Response.h"
#include "IHttp.h"
#include "Data.h"
#include "DebugPrint.h"

#define WRITE_BIFFER_SIZE 1024


using namespace WebCpp;

Response::Response(int connID, const HttpConfig& config) :
    m_connID(connID),
    m_config(config),
    m_header(HttpHeader::HeaderRole::Response)
{
    InitDefault();
}

HttpHeader &Response::GetHeader()
{
    return m_header;
}

void Response::AddHeader(const std::string &name, const std::string &value)
{
    m_header.SetHeader(name, value);
}

void Response::AddHeader(HttpHeader::HeaderType header, const std::string &value)
{
    AddHeader(HttpHeader::HeaderType2String(header), value);
}

void Response::Write(const ByteArray &data, size_t start)
{
    m_body.insert(m_body.end(), data.begin() + start, data.end());
    AddHeader(HttpHeader::HeaderType::ContentLength, std::to_string(m_body.size()));
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
        path = root + file;
    }
    else
    {
        path = file;
    }

    if(FileSystem::IsFileExist(path))
    {
        std::string ext = FileSystem::ExtractFileExtension(path);
        AddHeader(HttpHeader::HeaderType::ContentType, Response::Extension2MimeType(ext) + ";charset=" + charset);
        AddHeader(HttpHeader::HeaderType::ContentLength, std::to_string(FileSystem::GetFileSize(path)));
        AddHeader(HttpHeader::HeaderType::LastModified, FileSystem::GetFileModifiedTime(path));
        m_file = path;
        retval = true;
    }
    else
    {
        SetLastError("file not exist");
    }

    return retval;
}

bool Response::SendNotFound()
{
    m_responseCode = 404;
    m_responsePhrase = Response::ResponseCode2String(m_responseCode);
    AddHeader(HttpHeader::HeaderType::ContentLength, "0");
    return true;
}

bool Response::SendRedirect(const std::string &url)
{
    m_responseCode = 301;
    m_responsePhrase = Response::ResponseCode2String(m_responseCode);
    AddHeader(HttpHeader::HeaderType::Location, url);
    AddHeader(HttpHeader::HeaderType::ContentLength, "0");
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

std::string Response::GetResponsePhrase() const
{
    return m_responsePhrase;
}

const ByteArray &Response::GetBody() const
{
    return m_body;
}

const HttpHeader &Response::GetHeader() const
{
    return m_header;
}

std::string Response::GetHttpVersion() const
{
    return m_version;
}

bool Response::IsShouldSend() const
{
    return m_shouldSend;
}

void Response::SetShouldSend(bool value)
{
    m_shouldSend = value;
}

bool Response::Send(ICommunicationServer *communication)
{
    ByteArray header;

    const ByteArray &sl = BuildStatusLine();
    header.insert(header.end(), sl.begin(), sl.end());

    const ByteArray &hdr = BuildHeaders();
    header.insert(header.end(), hdr.begin(), hdr.end());
    header.push_back(CR);
    header.push_back(LF);

    if(communication->Write(m_connID, header) == false)
    {
        SetLastError("error sending header: " + communication->GetLastError());
        return false;
    }

    if(!m_file.empty())
    {
        ByteArray buffer(WRITE_BIFFER_SIZE);
        if(FileSystem::IsFileExist(m_file))
        {
            size_t size = FileSystem::GetFileSize(m_file);
            File file(m_file, File::Mode::Read);
            if(file.IsOpened())
            {
                size_t pos = 0;
                while(pos < size)
                {
                    ssize_t bytes = file.Read(reinterpret_cast<char *>(buffer.data()), WRITE_BIFFER_SIZE);
                    if(bytes == ERROR)
                    {
                        break;
                    }
                    pos += bytes;
                    if(communication->Write(m_connID, buffer, bytes) == false)
                    {
                        SetLastError("error sending file: " + communication->GetLastError());
                        return false;
                    }
                }
            }
            else
            {
                SetLastError("file " + m_file + " failed to open");
                return false;
            }
        }
        else
        {
            SetLastError("file " + m_file + " not exists");
            return false;
        }
    }
    else if(m_body.size() > 0)
    {
        if(communication->Write(m_connID, m_body) == false)
        {
            SetLastError("error sending body: " + communication->GetLastError());
            return false;
        }
    }

    return true;
}

bool Response::Parse(const ByteArray &data, size_t* all, size_t* downoaded)
{
    ClearError();
    size_t pos;

    if(ParseStatusLine(data, pos) == false)
    {
        SetLastError("error parsing status line: " + GetLastError());
        return false;
    }

    if(m_header.IsComplete() == false)
    {
        if(m_header.Parse(data, pos + 2) == false)
        {
            SetLastError("error parsing headers");
            return false;
        }

        size_t allSize = pos + 2 + m_header.GetRequestSize();
        if(all != nullptr)
        {
            *all = m_header.GetBodySize();
        }
        if(downoaded != nullptr)
        {
            *downoaded = data.size() - (m_header.GetHeaderSize() + pos + 2);
        }

        if(data.size() >= allSize)
        {
            auto transferEncoding = m_header.GetHeader(HttpHeader::HeaderType::TransferEncoding);
            auto contentEncoding = String2EncodingType(m_header.GetHeader(HttpHeader::HeaderType::ContentEncoding));

            /* some servers send 'chunked' inside Content-Encoding but according to the
             * https://datatracker.ietf.org/doc/html/rfc2616#section-3.5 it's incorrect
             * and should only be sent in the Transfer-Encoding, so we ignore that here */
            if(transferEncoding == "chunked")
            {
                try
                {
                    ByteArray tail(data.end() - 5, data.end());
                    if(StringUtil::Compare(tail, { '0', CR, LF, CR, LF})) // all data received
                    {
                        DecodeBody(EncodingType::Chunked, data, pos);
                        if(contentEncoding != EncodingType::Undefined)
                        {
                            if(DecodeBody(contentEncoding, m_body, 0) == false)
                            {
                                return false;
                            }
                        }

                        return true;
                    }
                }
                catch(...)
                {
                    return false;
                }
            }

            else
            {
                if(pos + 2 + m_header.GetRequestSize() == data.size())
                {
                    size_t dataStart = pos + 2 + m_header.GetHeaderSize() + 4;
                    if(data.size() > dataStart)
                    {
                        if(DecodeBody(contentEncoding, data, dataStart) == false)
                        {
                            return false;
                        }
                    }
                    return true;
                }
            }
        }
    }

    return false;
}

bool Response::DecodeBody(EncodingType type, const ByteArray& data, size_t pos)
{
    switch(type)
    {
        case EncodingType::Chunked:
            {
                m_body.clear();
                size_t dataStart = pos + 2 + m_header.GetHeaderSize() + 4; // status line + CRLF (2 bytes) + headers + CRLFCRLF (4 bytes)
                size_t dataLength = data.size() - 5; // data w/o trailing chunk
                while(dataStart < dataLength)
                {
                    auto pos = StringUtil::SearchPosition(data, {CR, LF}, dataStart); // end of length string
                    std::string temp(data.begin() + dataStart,data.begin() + pos);
                    int n = std::stoi(temp, nullptr, 16);
                    size_t chunkEnd = pos + 2 + n; // end of lenght string + 2 bytes(CRLF) + data length
                    if(chunkEnd <= dataLength)
                    {
                        m_body.insert(m_body.end(), data.begin() + pos + 2, data.begin() + chunkEnd);
                        dataStart = chunkEnd + 2; // end of data + 2 bytes (CRLF)
                    }
                    else
                    {
                        return false;
                    }
                }
            }
            break;
#ifdef WITH_ZLIB
        case EncodingType::Gzip:
            {
                ByteArray encoded(data.begin() + pos, data.end());
                ByteArray decoded = Data::Unzip(encoded);
                m_body.clear();
                m_body.insert(m_body.begin(), decoded.begin(), decoded.end());
            }
            break;
        case EncodingType::Deflate:
            {
                ByteArray encoded(data.begin() + pos, data.end());
                ByteArray decoded = Data::Uncompress(encoded);
                m_body.clear();
                m_body.insert(m_body.begin(), decoded.begin(), decoded.end());
            }
            break;
#endif
        default:
            m_body.insert(m_body.end(), data.begin() + pos, data.end());
            break;
    }

    return true;
}

Response::EncodingType Response::String2EncodingType(const std::string &str)
{
    switch(_(str.c_str()))
    {
        case _("gzip"): return Response::EncodingType::Gzip;
        case _("deflate"): return Response::EncodingType::Deflate;
        case _("chunked"): return Response::EncodingType::Chunked;
        case _("compress"): return Response::EncodingType::Compress;
        case _("br"): return Response::EncodingType::Brotli;
        default: break;
    }

    return Response::EncodingType::Undefined;
}

bool Response::ParseStatusLine(const ByteArray &data, size_t &pos)
{
    pos = StringUtil::SearchPosition(data, { CR, LF });
    if(pos != SIZE_MAX) // status line presents
    {
        auto ranges = StringUtil::Split(data, { ' ' }, 0, pos);
        if(ranges.size() >= 3)
        {
            m_version = std::string(data.begin() + ranges[0].start, data.begin() + ranges[0].end + 1);
            StringUtil::Trim(m_version);

            std::string temp = std::string(data.begin() + ranges[1].start, data.begin() + ranges[1].end + 1);
            int i;
            if(StringUtil::String2int(temp, i))
            {
                m_responseCode = i;
            }
            else
            {
                SetLastError("response code parsing error");
                return false;
            }

            for(size_t i = 2;i < ranges.size();i ++)
            {
                m_responsePhrase += (m_responsePhrase.empty() ? "" : " ") + std::string(data.begin() + ranges[i].start, data.begin() + ranges[i].end + 1);
            }

            StringUtil::Trim(m_responsePhrase);

            return true;
        }
    }
    else
    {
        SetLastError("status line not found");
    }

    return false;
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
    AddHeader(HttpHeader::HeaderType::Server, m_config.GetServerName());
}

ByteArray Response::BuildStatusLine() const
{
    std::string statusLine = m_version + " " + std::to_string(m_responseCode) + " " + m_responsePhrase + CR + LF;
    return ByteArray(statusLine.begin(), statusLine.end());
}

ByteArray Response::BuildHeaders() const
{
    return m_header.ToByteArray();
}
