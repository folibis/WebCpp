#ifndef REQUEST_H
#define REQUEST_H

#include "common.h"

namespace WebCpp
{

class Request
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
        Request::HeaderType type = Request::HeaderType::Undefined;
        std::string name = "";
        std::string value = "";
        static Request::Header defaultHeader;
    };

    Request(int connID, const ByteArray &request);

    int GetConnectionID() const;
    Request::Method GetMethod() const;
    std::string GetUri() const;
    const std::vector<Header>& GetHeaders() const;
    const Header& GetHeader(Request::HeaderType headerType) const;
    const Header& GetHeader(const std::string& headerType) const;
    std::string GetVersion() const;
    std::string GetHost() const;
    const ByteArray &GetData() const;
    std::string Param(const std::string &name) const;

    static std::string Method2String(Request::Method method);
    static Request::Method String2Method(const std::string &str);
    static std::string HeaderType2String(Request::HeaderType headerType);
    static Request::HeaderType String2HeaderType(const std::string &str);

protected:
    void Init(const ByteArray &data);
    bool ParseHeaders(std::vector<ByteArray> &arr);

private:
    int m_connID;
    Request::Method m_method = Request::Method::Undefined;
    std::string m_uri = "";
    std::vector<Header> m_headers = {};
    std::string m_version = "";
    std::string m_host = "";
    ByteArray m_data;
};

}

#endif // REQUEST_H
