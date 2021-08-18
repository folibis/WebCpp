#ifndef HTTPHEADER_H
#define HTTPHEADER_H

#include <string>
#include <vector>
#include <map>
#include "common.h"
#include "StringUtil.h"


namespace WebCpp
{

class HttpHeader
{
public:
    enum class Method
    {
        Undefined = 0,
        OPTIONS,
        GET,
        HEAD,
        POST,
        PUT,
        DELETE,
        TRACE,
        CONNECT
    };

    enum class HeaderType
    {
        Undefined = 0,
        Accept,
        AcceptCharset,
        AcceptEncoding,
        AcceptDatetime,
        AcceptLanguage,
        Authorization,
        CacheControl,
        Connection,
        ContentEncoding,
        ContentLength,
        ContentMD5,
        ContentType,
        Cookie,
        Date,
        Expect,
        Forwarded,
        From,
        Host,
        HTTP2Settings,
        IfMatch,
        IfModifiedSince,
        IfNoneMatch,
        IfRange,
        IfUnmodifiedSince,
        MaxForwards,
        Origin,
        Pragma,
        Prefer,
        ProxyAuthorization,
        Range,
        Referer,
        TE,
        Trailer,
        TransferEncoding,
        UserAgent,
        Upgrade,
        Via,
        Warning,
    };

    struct Header
    {
        HttpHeader::HeaderType type = HttpHeader::HeaderType::Undefined;
        std::string name = "";
        std::string value = "";
        static HttpHeader::Header defaultHeader;
    };

    HttpHeader();
    HttpHeader(const HttpHeader& other) = delete;
    HttpHeader& operator=(const HttpHeader& other) = delete;
    HttpHeader(HttpHeader&& other) = default;
    HttpHeader& operator=(HttpHeader&& other) = default;

    bool Parse(const ByteArray &data);
    bool IsComplete() const;
    size_t GetHeaderSize() const;
    size_t GetBodySize() const;
    size_t GetRequestSize() const;

    HttpHeader::Method GetMethod() const;
    std::string GetPath() const;
    std::string GetUri() const;
    std::string GetVersion() const;
    std::string GetHost() const;

    std::string GetHeader(HeaderType headerType) const;
    std::string GetHeader(const std::string &headerType) const;
    const std::vector<HttpHeader::Header> &GetHeaders() const;

    static HttpHeader::Method String2Method(const std::string &str);
    static std::string Method2String(HttpHeader::Method method);
    static HttpHeader::HeaderType String2HeaderType(const std::string &str);
    static std::string HeaderType2String(HttpHeader::HeaderType headerType);

protected:
    bool ParseHeaders(const ByteArray &data, const StringUtil::Ranges &ranges);
    void ParseQuery();

private:
    bool m_complete = false;
    HttpHeader::Method m_method = HttpHeader::Method::Undefined;
    std::string m_uri = "";
    std::string m_path = "";
    std::vector<Header> m_headers = {};
    std::string m_version = "";
    std::string m_host = "";
    size_t m_headerSize = 0;
    std::map<std::string, std::string> m_query;
};

}

#endif // HTTPHEADER_H
