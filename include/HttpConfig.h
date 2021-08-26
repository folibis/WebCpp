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

#ifndef HTTPCONFIG_H
#define HTTPCONFIG_H

#include <string>
#include <vector>
#include "common.h"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define PROPERTY(TYPE,NAME,DEFAULT) \
    public: \
    TYPE Get##NAME() const { return m_##NAME; } \
    void Set##NAME(const TYPE& value) { m_##NAME = value; OnChanged(TOSTRING(NAME)); } \
    private: \
    TYPE m_##NAME = DEFAULT; \


namespace WebCpp
{

class HttpConfig
{
public:
    HttpConfig();
    void Init();
    bool Load();

    std::string RootFolder() const;
    std::string ToString() const;

protected:
    void SetRootFolder();
    void OnChanged(const std::string &value);
private:
    std::string m_rootFolder;

    PROPERTY(std::string, ServerName, WEBCPP_CANONICAL_NAME)
    PROPERTY(std::string, Root, "public")
    PROPERTY(std::string, IndexFile, "index.html")
    PROPERTY(bool, HttpEnabled, true)
    PROPERTY(std::string, HttpServerAddress, "")
    PROPERTY(int, HttpServerPort, 8080)
    PROPERTY(std::string, HttpProtocol, "HTTP")
    PROPERTY(int, KeepAliveTimeout, 2000)
    PROPERTY(std::string, SslSertificate, "cert.pem")
    PROPERTY(std::string, SslKey, "key.pem")
    PROPERTY(bool, TempFile, false)
    PROPERTY(bool, WsProcessDefault, true)
    PROPERTY(int, WsServerPort, 8081)
    PROPERTY(std::string, WsProtocol, "ws")
};

}

#endif // HTTPCONFIG_H
