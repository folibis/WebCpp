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

#ifndef RESPONSE_H
#define RESPONSE_H

#include <string>
#include <map>
#include "ICommunication.h"
#include "common.h"
#include "HttpConfig.h"


namespace WebCpp
{

class Response
{
public:
    enum class HeaderType
    {
        Undefined = 0,
        AcceptCH,
        AccessControlAllowOrigin,
        AccessControlAllowCredentials,
        AccessControlExposeHeaders,
        AccessControlMaxAge,
        AccessControlAllowMethods,
        AccessControlAllowHeaders,
        AcceptPatch,
        AcceptRanges,
        Age,
        Allow,
        AltSvc,
        CacheControl,
        Connection,
        ContentDisposition,
        ContentEncoding,
        ContentLanguage,
        ContentLength,
        ContentLocation,
        ContentMD5,
        ContentRange,
        ContentType,
        Date,
        DeltaBase,
        ETag,
        Expires,
        IM,
        LastModified,
        Link,
        Location,
        P3P,
        Pragma,
        PreferenceApplied,
        ProxyAuthenticate,
        PublicKeyPins,
        RetryAfter,
        Server,
        SetCookie,
        StrictTransportSecurity,
        Trailer,
        TransferEncoding,
        Tk,
        Upgrade,
        Vary,
        Via,
        Warning,
        WWWAuthenticate,
        XFrameOptions,
    };

    Response(int connID, const HttpConfig& config);
    Response(const Response& other) = delete;
    Response& operator=(const Response& other) = delete;
    Response(Response&& other) = delete;
    Response& operator=(Response&& other) = delete;

    void SetHeader(Response::HeaderType header, const std::string &value);
    void SetHeader(const std::string &name, const std::string &value);
    void Write(const ByteArray &data);
    void Write(const std::string &data);
    bool AddFile(const std::string &file, const std::string &charset = "utf-8");
    bool SendNotFound();
    bool SendRedirect(const std::string &url);

    void SetResponseCode(uint16_t code);
    void SetResponseCode(uint16_t code, const std::string &phrase);
    uint16_t GetResponseCode() const;

    bool Send(ICommunication *communication);
    static std::string HeaderType2String(Response::HeaderType headerType);
    static Response::HeaderType String2HeaderType(const std::string &str);
    static std::string ResponseCode2String(int code);
    static std::string Extension2MimeType(const std::string &extension);

protected:
    void InitDefault();
    ByteArray BuildStatusLine() const;
    ByteArray BuildHeaders() const;

private:
    int m_connID;
    const HttpConfig &m_config;
    std::string m_version;
    std::map<std::string, std::string> m_headers;
    ByteArray m_body;
    uint16_t m_responseCode = 200;
    std::string m_responsePhrase = "";
    std::string m_mimeType = "";   

    std::string  m_file;
};

}

#endif // RESPONSE_H
