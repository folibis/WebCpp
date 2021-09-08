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

#ifndef REQUEST_H
#define REQUEST_H

#include <map>
#include "common.h"
#include "HttpConfig.h"
#include "RequestBody.h"
#include "HttpHeader.h"
#include "ICommunicationClient.h"
#include "Url.h"
#include "IHttp.h"
#include "IError.h"


namespace WebCpp
{

class Request: public IError
{
public:
    Request();
    Request(const HttpConfig &config);
    Request(int connID, HttpConfig &config, const std::string &remote);
    Request(const Request& other) = delete;
    Request& operator=(const Request& other) = delete;
    Request(Request&& other) = default;
    Request& operator=(Request&& other) = default;

    bool Parse(const ByteArray &data);
    int GetConnectionID() const;
    void SetConnectionID(int connID);
    const HttpConfig& GetConfig() const;
    void SetConfig(const HttpConfig& config);
    const Url& GetUrl() const;
    Url& GetUrl();
    const HttpHeader& GetHeader() const;
    HttpHeader& GetHeader();
    Http::Method GetMethod() const;
    void SetMethod(Http::Method method);
    std::string GetHttpVersion() const;
    const RequestBody& GetRequestBody() const;
    RequestBody& GetRequestBody();
    std::string GetArg(const std::string &name) const;
    void SetArg(const std::string &name, const std::string &value);
    bool IsKeepAlive() const;
    Http::Protocol GetProtocol() const;
    size_t GetRequestLineLength() const;
    size_t GetRequestSize() const;
    std::string GetRemote() const;
    void SetRemote(const std::string &remote);
    bool Send(ICommunicationClient *comm);
    std::string ToString() const;

protected:
    bool Init(const ByteArray &data);
    bool ParseRequestLine(const ByteArray &data, size_t &pos);
    bool ParseBody(const ByteArray &data, size_t headerSize);
    ByteArray BuildRequestLine() const;
    ByteArray BuildHeaders() const;
private:
    int m_connID;
    Url m_url;
    HttpConfig m_config;
    HttpHeader m_header;
    Http::Method m_method = Http::Method::Undefined;
    std::string m_httpVersion = "HTTP/1.1";
    size_t m_requestLineLength = 0;
    std::map<std::string, std::string> m_args;    
    RequestBody m_requestBody;
    std::string m_remote;
};

}

#endif // REQUEST_H
