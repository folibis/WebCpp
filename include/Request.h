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

namespace WebCpp
{

class Request
{
public:
    enum class Protocol
    {
        Undefined,
        HTTP1,
        WebSocket,
    };
    Request();
    Request(HttpConfig &config);
    Request(int connID, const ByteArray &request, HttpHeader &&header, HttpConfig &config);
    Request(const Request& other) = delete;
    Request& operator=(const Request& other) = delete;
    Request(Request&& other) = default;
    Request& operator=(Request&& other) = default;


    int GetConnectionID() const;
    void SetConnectionID(int connID);
    const HttpConfig& GetConfig() const;
    void SetConfig(const HttpConfig& config);
    const HttpHeader& GetHeader() const;
    HttpHeader& GetHeader();
    const ByteArray &GetData() const;
    const RequestBody& GetRequestBody() const;
    std::string GetArg(const std::string &name) const;
    void SetArg(const std::string &name, const std::string &value);
    bool IsKeepAlive() const;
    Protocol GetProtocol() const;

protected:
    void Init(const ByteArray &data);    
    void ParseBody(const ByteArray &data, size_t headerSize);

private:
    int m_connID;
    HttpConfig m_config;
    HttpHeader m_header;
    ByteArray m_data;
    std::map<std::string, std::string> m_args;    
    RequestBody m_requestBody;
};

}

#endif // REQUEST_H
