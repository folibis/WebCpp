#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <netdb.h>
#include <cstring>
#include <stdexcept>
#include "SocketPool.h"
#include "StringUtil.h"

#define QUEUE_SIZE 10


using namespace WebCpp;

SocketPool::SocketPool(size_t count, Service service, Domain domain, Type type, Options options):
    m_count(count),
    m_service(service),
    m_domain(domain),
    m_type(type),
    m_options(options)
{
    m_fds = new struct pollfd[count] { };
    for(auto i = 0;i < count;i ++)
    {
        m_fds[i].fd = (-1);
    }
#ifdef WITH_OPENSSL
    if((m_options & Options::Ssl) == Options::Ssl)
    {
        m_sslClient = new SSL*[count];
    }
#endif
}

void SocketPool::SetPort(int port)
{
    m_port = port;
}

int SocketPool::GetPort() const
{
    return m_port;
}

void SocketPool::SetHost(const std::string &host)
{
    m_host = host;
}

std::string SocketPool::GetHost() const
{
    return m_host;
}

SocketPool::~SocketPool()
{
    if(m_fds != nullptr)
    {
        delete []m_fds;
        m_fds = nullptr;
    }
#ifdef WITH_OPENSSL
    if((m_options & Options::Ssl) == Options::Ssl)
    {
        {
        if(m_sslClient != nullptr)
            delete []m_sslClient;
            m_sslClient = nullptr;
        }
    }
#endif
}

int SocketPool::Create(bool main)
{
    ClearError();
    int sock = (-1);

    try
    {
        size_t index = main ? 0 : FindEmpty();
        if(index == (-1))
        {
            SetLastError("No free room for socket");
            return (-1);
        }

#ifdef WITH_OPENSSL
        if((m_options & Options::Ssl) == Options::Ssl)
        {
            if(InitSSL() == false)
            {
                throw std::runtime_error(std::string("SSL init error: ") + GetLastError());
            }
        }
#endif

        int d = SocketPool::Domain2Domain(m_domain);
        int t = SocketPool::Type2Type(m_type);

        sock = socket(d, t, 0);
        if(sock == ERROR)
        {
            throw std::runtime_error(std::string("socket create error: ") + strerror(errno));
        }

        if((m_options & Options::ReuseAddr) == Options::ReuseAddr)
        {
            int opt = 1;
            if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == ERROR)
            {
                throw std::runtime_error(std::string("set socket option error: ") + strerror(errno));
            }
        }

        m_fds[index].fd = sock;
        m_fds[index].events = POLLIN;

#ifdef WITH_OPENSSL
        if((m_options & Options::Ssl) == Options::Ssl)
        {
            auto ssl = SSL_new(m_ctx);
            SSL_set_fd(ssl, sock);
            m_sslClient[index] = ssl;
        }
#endif
        return index;
    }
    catch(const std::runtime_error &err)
    {
        SetLastError(err.what());
    }
    catch(...)
    {
        SetLastError("error creating socket");
    }

    if(sock >= 0)
    {
        close(sock);
    }
    return (-1);
}

bool SocketPool::CloseSocket(size_t index)
{
    if(index < m_count)
    {
        if(m_fds[index].fd != (-1))
        {
            close(m_fds[index].fd);
            m_fds[index].fd = (-1);
            m_fds[index].events = 0;
#ifdef WITH_OPENSSL
            if((m_options & Options::Ssl) == Options::Ssl)
            {
                SSL *ssl = m_sslClient[index];
                if(ssl != nullptr)
                {
                    SSL_shutdown(ssl);
                    SSL_free(ssl);
                }
                m_sslClient[index] = nullptr;
            }
#endif
            return true;
        }
    }

    return false;
}

bool SocketPool::CloseSockets()
{
    for(auto i = 0;i < m_count;i ++)
    {
        CloseSocket(i);
    }

    return true;
}

bool SocketPool::Bind(const std::string &host, int port)
{
    ClearError();

    try
    {
        if(m_fds[0].fd == (-1))
        {
            SetLastError("create main socket first");
            return false;
        }

        if(!host.empty())
        {
            m_host = host;
        }
        if(port > 0)
        {
            m_port = port;
        }

        int d = SocketPool::Domain2Domain(m_domain);
        struct sockaddr_in server_sockaddr;
        server_sockaddr.sin_family = d;
        server_sockaddr.sin_port = htons(m_port);
        if(m_host.empty() || m_host == "*")
        {
            server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        }
        else
        {
            server_sockaddr.sin_addr.s_addr = inet_addr(m_host.c_str());
        }

        if(bind(m_fds[0].fd, (struct sockaddr* ) &server_sockaddr, sizeof(server_sockaddr)) == ERROR)
        {
            throw std::runtime_error(std::string("socket bind error: ") + strerror(errno));
        }

        return true;
    }
    catch(const std::runtime_error &err)
    {
        SetLastError(err.what());
    }
    catch(...)
    {
        SetLastError("socket bind error");
    }

    return false;
}

bool SocketPool::Listen()
{
    ClearError();

    try
    {
        if(m_fds[0].fd == (-1))
        {
            SetLastError("create main socket first");
            return false;
        }

        if(listen(m_fds[0].fd, QUEUE_SIZE) == ERROR)
        {
            throw std::runtime_error(std::string("socket listen error: ") + strerror(errno));
        }

        return true;
    }
    catch(const std::runtime_error &err)
    {
        SetLastError(err.what());
    }
    catch(...)
    {
        SetLastError("socket listen error");
    }

    return false;
}

size_t SocketPool::Accept()
{
    ClearError();

    try
    {
        if(m_fds[0].fd == (-1))
        {
            throw std::runtime_error("create main socket first");
        }

        int new_socket = accept(m_fds[0].fd, NULL, NULL);
        if(new_socket != ERROR)
        {
            int index = FindEmpty();
            if(index != ERROR)
            {
                fcntl(new_socket, F_SETFL, O_NONBLOCK);
                m_fds[index].fd = new_socket;
                m_fds[index].events = POLLIN;
#ifdef WITH_OPENSSL
                if((m_options & Options::Ssl) == Options::Ssl)
                {
                    if(AcceptSsl(new_socket, index) == false)
                    {
                        CloseSocket(index);
                        throw std::runtime_error(GetLastError());
                    }
                }
#endif
                return index;
            }
            else
            {
                throw std::runtime_error("no room for new connction");
            }
        }
        else
        {
            throw std::runtime_error(std::string("socket accept error: ") + strerror(errno));
        }
    }
    catch(const std::runtime_error &err)
    {
        SetLastError(err.what());
    }
    catch(...)
    {
        SetLastError("socket listen error");
    }

    return ERROR;
}

bool SocketPool::Connect(const std::string &host, int port)
{
    ClearError();

    if(m_fds[0].fd == (-1))
    {
        SetLastError("create main socket first");
        return false;
    }

    switch(m_domain)
    {
    case Domain::Inet:
        return ConnectTcp(host, port);
    case Domain::Local:
        return ConnectUnix(host);
    default: break;
    }

    return false;
}

bool SocketPool::ConnectTcp(const std::string &host, int port)
{
    ParseAddress(host);
    if(port != 0)
    {
        m_port = port;
    }

    struct hostent *hostinfo;
    struct sockaddr_in dest_addr;

    try
    {
        if((hostinfo = gethostbyname(m_host.c_str())) == nullptr)
        {
            SetLastError(std::string("Error resolving the host name") + strerror(errno), errno);
            throw std::runtime_error(GetLastError());
        }
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(m_port);
        dest_addr.sin_addr = *((struct in_addr *)hostinfo->h_addr);
        memset(&(dest_addr.sin_zero), 0, 8);

        if(connect(m_fds[0].fd, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr)) == -1)
        {
            SetLastError(std::string("Socket connecting error: ") + strerror(errno), errno);
            throw std::runtime_error(GetLastError());
        }

#ifdef WITH_OPENSSL
        if((m_options & Options::Ssl) == Options::Ssl)
        {
            SSL *ssl = m_sslClient[0];
            const int status = SSL_connect(ssl);
            if(status <= 0)
            {
                int errorCode = SSL_get_error(ssl, status);
                SetLastError(ERR_error_string(errorCode, nullptr));
                throw std::runtime_error(std::string("SSL connect error: ") + GetLastError());
            }
        }
#endif
        return true;
    }
    catch(const std::runtime_error &err)
    {
        SetLastError(err.what());
    }
    catch(...)
    {
        SetLastError("socket connect error");
    }

    return false;
}

bool SocketPool::ConnectUnix(const std::string &host)
{
    socklen_t len;
    struct sockaddr_un addr;

    if(!host.empty())
    {
        m_host = host;
    }

    try
    {
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, m_host.c_str(), m_host.size());
        //*addr.sun_path = '\0';

        len = static_cast<socklen_t>(__builtin_offsetof(struct sockaddr_un, sun_path) + m_host.length());
        if(connect(m_fds[0].fd, reinterpret_cast<struct sockaddr *>(&addr), len) == (-1))
        {
            SetLastError(std::string("Socket connecting error: ") + strerror(errno), errno);
            throw std::runtime_error(GetLastError());
        }

        return true;
    }
    catch(const std::runtime_error &err)
    {
        SetLastError(err.what());
    }
    catch(...)
    {
        SetLastError("socket connect error");
    }

    return false;
}

size_t SocketPool::Write(const void *buffer, size_t size, size_t index)
{
    ClearError();

    ssize_t sent = (-1);
    try
    {
        int fd = m_fds[index].fd;
        if(fd == (-1))
        {
            SetLastError("wrong socket");
            return (-1);
        }

        bool again = false;

        if((m_options & Options::Ssl) == Options::Ssl)
        {
#ifdef WITH_OPENSSL
            SSL *ssl = m_sslClient[index];
            do
            {
                sent = SSL_write(ssl, buffer, size);
                if(sent <= 0)
                {
                    int errorCode = SSL_get_error(ssl, sent);
                    if(errorCode == SSL_ERROR_WANT_WRITE)
                    {
                        again = true;
                    }
                    else
                    {
                        SetLastError(ERR_error_string(errorCode, nullptr));
                        throw std::runtime_error(std::string("get SSL handler error: ") + GetLastError());
                    }
                }
                else
                {
                    again = false;
                }
            }
            while(again);
#endif
        }
        else
        {

            do
            {
                sent = send(fd, buffer, size, MSG_NOSIGNAL);
                if(sent == ERROR)
                {
                    if(errno == EAGAIN)
                    {
                        again = true;
                    }
                    else
                    {
                        throw std::runtime_error(std::string("socket write error: ") + strerror(errno));
                    }
                }
                else
                {
                    again = false;
                }
            }
            while(again);
        }
    }
    catch(const std::runtime_error &err)
    {
        SetLastError(err.what());
    }
    catch(...)
    {
        SetLastError("socket write error");
    }

    return sent;
}

size_t SocketPool::Read(void *buffer, size_t size, size_t index)
{
    ClearError();
    ssize_t read = (-1);

    try
    {
        int fd = m_fds[index].fd;
        if(fd == (-1))
        {
            SetLastError("wrong socket");
            return (-1);
        }

        if((m_options & Options::Ssl) == Options::Ssl)
        {
#ifdef WITH_OPENSSL
            SSL *ssl = m_sslClient[index];
            if(ssl == nullptr)
            {
                SetLastError(ERR_error_string(ERR_get_error(), nullptr));
                throw std::runtime_error(std::string("get SSL handler error: ") + GetLastError());
            }

            read = SSL_read(ssl, buffer, size);
            if (read <= 0)
            {
                int errorCode = SSL_get_error(ssl, read);
                if (errorCode == SSL_ERROR_WANT_READ)
                {
                    read = 0;
                }
                else
                {
                    SetLastError(ERR_error_string(errorCode, nullptr));
                    throw std::runtime_error(std::string("SSL read error: ") + GetLastError());
                }
            }
#endif
        }
        else
        {
            bool again = false;
            do
            {
                read = recv(fd, buffer, size, MSG_DONTWAIT);
                if (read < 0)
                {
                    if (errno == EWOULDBLOCK)
                    {
                        again = true;
                    }
                    else
                    {
                        throw std::runtime_error(std::string("socket read error: ") + strerror(errno));
                    }
                }
            }
            while(again);
        }
    }
    catch(const std::runtime_error &err)
    {
        SetLastError(err.what());
    }
    catch(...)
    {
        SetLastError("socket read error");
    }

    return read;
}

bool SocketPool::Poll()
{
    auto retval = poll(m_fds, m_count, POLL_TIMEOUT);
    return (retval > 0);
}

size_t SocketPool::GetCount() const
{
    return m_count;
}

bool SocketPool::HasData(size_t index) const
{
    return (m_fds[index].revents == POLLIN);
}

std::string SocketPool::GetRemoteAddress(size_t index) const
{
    int fd = m_fds[index].fd;
    if(fd == (-1))
    {
        return "";
    }

    struct sockaddr_in client_sockaddr = {};
    socklen_t len = sizeof(client_sockaddr);
    std::string remote;
    if (getpeername(fd, reinterpret_cast<struct sockaddr *>(&client_sockaddr), &len ) != ERROR)
    {
        return std::string(inet_ntoa(client_sockaddr.sin_addr)) + ":" + std::to_string(ntohs(client_sockaddr.sin_port));
    }

    return "";
}

#ifdef WITH_OPENSSL
void SocketPool::SetSslCredentials(const std::string &cert, const std::string &key)
{
    m_cert = cert;
    m_key = key;
}

std::string SocketPool::ToString() const
{
    return std::string("SocketPool: " +
                       Service2String(m_service) + ", " +
                       Domain2String(m_domain) + ", "  +
                       Type2String(m_type) +
                       std::string(((m_options & Options::Ssl) == Options::Ssl) ? ", Ssl" : ""));
}

bool SocketPool::InitSSL()
{
    bool retval = false;
    try
    {
        OpenSSL_add_all_algorithms();
        SSL_load_error_strings();

        if(m_service == Service::Client)
        {
            const SSL_METHOD *method = TLS_client_method();
            m_ctx = SSL_CTX_new(method);
        }
        else if(m_service == Service::Server)
        {
            const SSL_METHOD *method = TLS_server_method();
            m_ctx = SSL_CTX_new(method);
        }

        if(m_ctx == nullptr)
        {
            SetLastError(ERR_error_string(ERR_get_error(), nullptr));
            retval = false;
        }
        if(m_service == Service::Server)
        {
            if (SSL_CTX_use_certificate_file(m_ctx, m_cert.c_str(), SSL_FILETYPE_PEM) <= 0)
            {
                SetLastError(ERR_error_string(ERR_get_error(), nullptr));
                throw std::runtime_error(GetLastError());
            }

            if (SSL_CTX_use_PrivateKey_file(m_ctx, m_key.c_str(), SSL_FILETYPE_PEM) <= 0 )
            {
                SetLastError(ERR_error_string(ERR_get_error(), nullptr));
                throw std::runtime_error(GetLastError());
            }
        }

        retval = true;
    }
    catch(const std::runtime_error &err)
    {
        SetLastError(err.what());
    }
    catch(...)
    {
        SetLastError("SSL init error");
    }

    return retval;
}

bool SocketPool::AcceptSsl(int fd, int index)
{
    ClearError();

    bool isContinue = true;
    bool isError = false;

    try
    {
        SSL *ssl = nullptr;
        ssl = SSL_new(m_ctx);
        SSL_set_fd(ssl, fd);

        while(isContinue)
        {
            int ret = SSL_accept(ssl);
            if(ret <= 0)
            {
                int errorCode = SSL_get_error(ssl, ret);
                if (errorCode == SSL_ERROR_WANT_READ)
                {
                    isContinue = true;
                }
                else
                {
                    SetLastError(ERR_error_string(errorCode, nullptr));
                    isContinue = false;
                    SSL_shutdown(ssl);
                    SSL_free(ssl);
                    close(fd);
                    isError = true;
                }
            }
            else
            {
                isError = false;
                isContinue = false;
                m_sslClient[index] = ssl;
            }
        }
    }
    catch(const std::runtime_error &err)
    {
        SetLastError(err.what());
    }
    catch(...)
    {
        SetLastError("SSL socket accept error");
    }

    return (isError == false);
}
#endif

int SocketPool::FindEmpty()
{
    for(int i = 1;i < m_count;i ++)
    {
        if(m_fds[i].fd == (-1))
        {
            return i;
        }
    }

    return ERROR;
}

void SocketPool::ParseAddress(const std::string &address)
{
    if(!address.empty())
    {
        auto addr_arr = StringUtil::Split(address, ':');
        if(addr_arr.size() >= 1)
        {
            m_host = addr_arr[0];
            if(addr_arr.size() >= 2)
            {
                int port;
                if(StringUtil::String2int(addr_arr[1], port))
                {
                    m_port = port;
                }
            }
        }
    }
}

int SocketPool::Domain2Domain(SocketPool::Domain domain)
{
    switch(domain)
    {
    case SocketPool::Domain::Inet:
        return AF_INET;
    case SocketPool::Domain::Local:
        return AF_UNIX;
    default: break;
    }
    return PF_UNSPEC;
}

int SocketPool::Type2Type(SocketPool::Type type)
{
    switch(type)
    {
    case SocketPool::Type::Stream:
        return SOCK_STREAM;
    case SocketPool::Type::Datagram:
        return SOCK_DGRAM;
    case SocketPool::Type::Raw:
        return SOCK_RAW;
    default: break;
    }
    return SOCK_STREAM;
}

std::string SocketPool::Domain2String(Domain domain)
{
    switch(domain)
    {
    case Domain::Inet:
        return "Inet";
    case Domain::Local:
        return "Local";
    default:
        break;
    }

    return "Undefined";
}

std::string SocketPool::Type2String(Type type)
{
    switch(type)
    {
    case Type::Stream:
        return "Stream";
    case Type::Datagram:
        return "Datagram";
    case Type::Raw:
        return "Raw";
    default:
        break;
    }

    return "Undefined";
}

std::string SocketPool::Service2String(Service service)
{
    switch(service)
    {
    case Service::Server:
        return "Server";
    case Service::Client:
        return "Client";
    default:
        break;
    }

    return "Undefined";
}
