/*
*
* Copyright (c) 2021 ruslan@muhlinin.com
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
*/

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
        CONNECT,
        WEBSOCKET,
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

    bool Parse(const ByteArray &data, bool withRequestList = true);
    bool ParseHeader(const ByteArray &data);
    bool IsComplete() const;
    size_t GetHeaderSize() const;
    size_t GetBodySize() const;
    size_t GetRequestSize() const;

    void SetMethod(HttpHeader::Method method);
    HttpHeader::Method GetMethod() const;
    std::string GetPath() const;
    std::string GetUri() const;
    std::string GetVersion() const;
    std::string GetQuery() const;
    std::string GetQueryValue(const std::string &name) const;

    std::string GetRemote() const;
    void SetRemote(const std::string &remote);
    std::string GetRemoteAddress() const;
    int GetRemotePort() const;

    int GetCount() const;
    std::string GetHeader(HeaderType headerType) const;
    std::string GetHeader(const std::string &headerType) const;
    const std::vector<HttpHeader::Header> &GetHeaders() const;

    static HttpHeader::Method String2Method(const std::string &str);
    static std::string Method2String(HttpHeader::Method method);
    static HttpHeader::HeaderType String2HeaderType(const std::string &str);
    static std::string HeaderType2String(HttpHeader::HeaderType headerType);

    std::string ToString() const;

protected:
    bool ParseHeaders(const ByteArray &data, const StringUtil::Ranges &ranges, bool requestList = true);
    void ParseQuery();

private:
    bool m_complete = false;
    HttpHeader::Method m_method = HttpHeader::Method::Undefined;
    std::string m_uri = "";
    std::string m_path = "";
    std::string m_query = "";
    std::vector<Header> m_headers = {};
    std::string m_version = "";
    size_t m_headerSize = 0;
    std::map<std::string, std::string> m_queryValue;
    std::string m_remoteAddress;
    int m_remotePort = (-1);
};

}

#endif // HTTPHEADER_H
