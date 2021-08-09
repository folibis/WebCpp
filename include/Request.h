#ifndef REQUEST_H
#define REQUEST_H

#include <map>
#include "common.h"
#include "HttpConfig.h"
#include "RequestBody.h"


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

    Request(int connID, const ByteArray &request, const HttpConfig &config);

    int GetConnectionID() const;
    Request::Method GetMethod() const;
    std::string GetUri() const;
    std::string GetPath() const;
    const std::vector<Header>& GetHeaders() const;
    std::string GetHeader(Request::HeaderType headerType) const;
    std::string GetHeader(const std::string& headerType) const;
    std::string GetVersion() const;
    std::string GetHost() const;
    const ByteArray &GetData() const;
    const RequestBody& GetRequestBody() const;
    std::string GetArg(const std::string &name) const;
    void SetArg(const std::string &name, const std::string &value);
    bool IsKeepAlive() const;

    static std::string Method2String(Request::Method method);
    static Request::Method String2Method(const std::string &str);
    static std::string HeaderType2String(Request::HeaderType headerType);
    static Request::HeaderType String2HeaderType(const std::string &str);    

protected:
    void Init(const ByteArray &data);
    bool ParseHeaders(std::vector<ByteArray> &arr);    
    void ParseQuery();
    void ParseBody(const ByteArray &data);

private:
    int m_connID;
    const HttpConfig &m_config;
    Request::Method m_method = Request::Method::Undefined;
    std::string m_uri = "";
    std::string m_path = "";
    std::vector<Header> m_headers = {};
    std::string m_version = "";
    std::string m_host = "";
    ByteArray m_data;
    std::map<std::string, std::string> m_args;
    std::map<std::string, std::string> m_query;
    RequestBody m_requestBody;
};

}

#endif // REQUEST_H
