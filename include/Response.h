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

#ifndef WEBCPP_RESPONSE_H
#define WEBCPP_RESPONSE_H

#include <string>
#include <map>
#include "ICommunicationServer.h"
#include "common_webcpp.h"
#include "HttpConfig.h"
#include "HttpHeader.h"
#include "IErrorable.h"


namespace WebCpp
{

class Session;
class Response: public IErrorable
{
public:
    enum class HeaderType
    {
        Undefined = 0,

    };

    Response(int connID, const HttpConfig& config);
    Response(const Response& other) = delete;
    Response& operator=(const Response& other) = delete;
    Response(Response&& other) = delete;
    Response& operator=(Response&& other) = delete;

    HttpHeader& GetHeader();
    void AddHeader(HttpHeader::HeaderType header, const std::string &value);
    void AddHeader(const std::string &name, const std::string &value);
    void Write(const ByteArray &data, size_t start = 0);
    void Write(const std::string &data);
    bool AddFile(const std::string &file, const std::string &charset = "utf-8");
    bool NotFound();
    bool Redirect(const std::string &url);
    bool Unauthorized();
    bool NotAuthenticated();
    void SetResponseCode(uint16_t code);
    void SetResponseCode(uint16_t code, const std::string &phrase);
    uint16_t GetResponseCode() const;
    std::string GetResponsePhrase() const;
    const ByteArray& GetBody() const;
    const HttpHeader& GetHeader() const;
    std::string GetHttpVersion() const;

    bool IsShouldSend() const;
    void SetShouldSend(bool value);
    bool Send(ICommunicationServer *communication);
    bool Parse(const ByteArray &data, size_t *all = nullptr, size_t *downoaded = nullptr);

    void SetSession(Session *session);
    Session* GetSession() const;

    static std::string HeaderType2String(Response::HeaderType headerType);
    static Response::HeaderType String2HeaderType(const std::string &str);
    static std::string ResponseCode2String(int code);
    static std::string Extension2MimeType(const std::string &extension);

protected:
    enum class EncodingType
    {
        Undefined = 0,
        Gzip,
        Deflate,
        Chunked,
        Compress,
        Brotli,
    };

    void InitDefault();
    ByteArray BuildStatusLine() const;
    ByteArray BuildHeaders() const;    
    bool ParseStatusLine(const ByteArray &data, size_t &pos);
    bool DecodeBody(EncodingType type, const ByteArray &data, size_t pos);
    static EncodingType String2EncodingType(const std::string &str);

private:
    int m_connID;
    const HttpConfig &m_config;
    std::string m_version;
    HttpHeader m_header;
    ByteArray m_body;
    uint16_t m_responseCode = 200;
    std::string m_responsePhrase = "";
    std::string m_mimeType = "";   
    std::string  m_file;
    bool m_shouldSend = true;
    Session *m_session = nullptr;
};

}

#endif // WEBCPP_RESPONSE_H
