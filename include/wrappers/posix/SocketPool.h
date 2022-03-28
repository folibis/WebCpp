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

#ifndef WEBCPP_SOCKET_POOL_H
#define WEBCPP_SOCKET_POOL_H

#include <poll.h>
#include <stddef.h>
#include "IErrorable.h"
#ifdef WITH_OPENSSL
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif

#define POLL_TIMEOUT 500
#define DEFAULT_HOST "*"
#define DEFAULT_PORT 80


namespace WebCpp
{

class SocketPool: public IErrorable
{
public:
    enum class Service
    {
        Undefined = 0,
        Server,
        Client
    };
    enum class Domain
    {
        Undefined = 0,
        Inet,
        Local,
    };
    enum class Type
    {
        Undefined = 0,
        Stream,
        Datagram,
        Raw,
    };
    enum class Options
    {
        None = 0,
        ReuseAddr = 1,
        Ssl = 2,
    };

    SocketPool(size_t count, Service service, Domain domain, Type type, Options options = Options::None);
    ~SocketPool();
    SocketPool(const SocketPool& other) = delete;
    SocketPool& operator=(const SocketPool& other) = delete;
    SocketPool(SocketPool&& other) = delete;
    SocketPool& operator=(SocketPool&& other) = delete;

    void SetPort(int port);
    int GetPort() const;
    void SetHost(const std::string &host);
    std::string GetHost() const;
    int Create(bool main = false);
    bool CloseSocket(size_t index);
    bool CloseSockets();
    bool Bind(const std::string &host, int port);
    bool Listen();
    size_t Accept();
    bool Connect(const std::string &host, int port = 0);
    size_t Write(const uint8_t *buffer, size_t size, size_t index = 0);
    size_t Read(void *buffer, size_t size, size_t index = 0);
    bool Poll();
    size_t GetCount() const;
    bool HasData(size_t index) const;
    std::string GetRemoteAddress(size_t index) const;
#ifdef WITH_OPENSSL
    void SetSslCredentials(const std::string &cert, const std::string &key);
#endif

    std::string ToString() const;
    static int Domain2Domain(SocketPool::Domain domain);
    static int Type2Type(SocketPool::Type type);
    static std::string Domain2String(SocketPool::Domain domain);
    static std::string Type2String(SocketPool::Type type);
    static std::string Service2String(SocketPool::Service service);

protected:
    int FindEmpty();
    void ParseAddress(const std::string &address);
    bool ConnectTcp(const std::string &host, int port);
    bool ConnectUnix(const std::string &host);

#ifdef WITH_OPENSSL
    bool InitSSL();
    bool AcceptSsl(int fd, int index);
#endif

private:
    size_t m_count;
    Service m_service = Service::Undefined;
    Domain m_domain = Domain::Undefined;
    Type m_type = Type::Undefined;
    Options m_options = Options::None;
    struct pollfd *m_fds = nullptr;
#ifdef WITH_OPENSSL
    std::string m_cert;
    std::string m_key;
    SSL_CTX *m_ctx = nullptr;
    SSL **m_sslClient = nullptr;
#endif
    std::string m_host = DEFAULT_HOST;
    int m_port = DEFAULT_PORT;
};

inline SocketPool::Options operator |(SocketPool::Options a, SocketPool::Options b)
{
    return static_cast<SocketPool::Options>(static_cast<int>(a) | static_cast<int>(b));
}
inline SocketPool::Options operator &(SocketPool::Options a, SocketPool::Options b)
{
    return static_cast<SocketPool::Options>(static_cast<int>(a) & static_cast<int>(b));
}
inline bool operator ==(SocketPool::Options a, SocketPool::Options b)
{
    return (static_cast<int>(a) == static_cast<int>(b));
}

}

#endif // WEBCPP_SOCKET_POOL_H
