#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <cstring>
#include <stdexcept>
#include "Socket.h"

#define QUEUE_SIZE 10


using namespace WebCpp;

Socket::Socket()
{

}

Socket::Socket(Domain domain, Type type, Options options) noexcept:
    m_domain(domain),
    m_type(type),
    m_options(options)
{
    Create();
}

Socket::Socket(int fd, Domain domain, Type type, Options options) noexcept:
    m_domain(domain),
    m_type(type),
    m_options(options)
{
    m_socket = fd;
}

bool Socket::Create()
{
    try
    {
        int domain = Socket::Domain2Domain(m_domain);
        int type = Socket::Type2Type(m_type);

        m_socket = socket(domain, type, 0);
        if(m_socket == ERROR)
        {
            throw std::runtime_error(std::string("socket create error: ") + strerror(errno));
        }

        if((m_options & Options::ReuseAddr) == Options::ReuseAddr)
        {
            int opt = 1;
            if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == ERROR)
            {
                throw std::runtime_error(std::string("set socket option error: ") + strerror(errno));
            }
        }
    }
    catch(const std::runtime_error &err)
    {
        m_socket = (-1);
        SetLastError(err.what());
    }
    catch(...)
    {
        m_socket = (-1);
        SetLastError("error creating socket");
    }

    return IsValid();
}

bool Socket::IsValid() const
{
    return (m_socket >= 0);
}

bool Socket::Bind(const std::string &address, int port)
{
    m_address = address;
    m_port = port;

    try
    {
        int domain = Socket::Domain2Domain(m_domain);
        struct sockaddr_in server_sockaddr;
        server_sockaddr.sin_family = domain;
        server_sockaddr.sin_port = htons(port);
        if(address.empty() || address == "*")
        {
            server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        }
        else
        {
            server_sockaddr.sin_addr.s_addr = inet_addr(m_address.c_str());
        }

        if(bind(m_socket, (struct sockaddr* ) &server_sockaddr, sizeof(server_sockaddr)) == ERROR)
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

bool Socket::Listen()
{
    try
    {
        if(listen(m_socket, QUEUE_SIZE) == ERROR)
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

size_t Socket::Write(const void *buffer, size_t size, bool &again)
{
    ssize_t sent = (-1);
    try
    {
        again = false;
        sent = send(m_socket, buffer, size, MSG_NOSIGNAL);
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

size_t Socket::Read(void *buffer, size_t size, bool &again)
{
    ssize_t read = (-1);
    try
    {
        again = false;
        read = recv(m_socket, buffer, size, MSG_DONTWAIT);
        if (read < 0)
        {
            if (errno == EWOULDBLOCK)
            {
                again = true;
            }
        }
        else
        {
            throw std::runtime_error(std::string("socket read error: ") + strerror(errno));
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

bool Socket::Close()
{
    if(m_socket >= 0)
    {
        close(m_socket);
        m_socket = (-1);
        return true;
    }

    return false;
}


Socket Socket::Accept()
{
    try
    {
        int new_socket = accept(m_socket, NULL, NULL);
        if (new_socket == ERROR)
        {
            throw std::runtime_error(std::string("socket accept error: ") + strerror(errno));
        }

        fcntl(new_socket, F_SETFL, O_NONBLOCK);
        return Socket(new_socket, m_domain, m_type, m_options);
    }
    catch(const std::runtime_error &err)
    {
        SetLastError(err.what());
    }
    catch(...)
    {
        SetLastError("socket accept error");
    }

    return Socket();
}

std::string Socket::GetPeer()
{
    try
    {
        struct sockaddr_in client_sockaddr = {};
        socklen_t len = sizeof(client_sockaddr);
        if (getpeername(m_socket, reinterpret_cast<struct sockaddr *>(&client_sockaddr), &len ) != -1)
        {
            return std::string(inet_ntoa(client_sockaddr.sin_addr)) + ":" + std::to_string(ntohs(client_sockaddr.sin_port));
        }
        else
        {
            throw std::runtime_error(std::string("socket get peer error: ") + strerror(errno));
        }
    }
    catch(const std::runtime_error &err)
    {
        SetLastError(err.what());
    }
    catch(...)
    {
        SetLastError("socket get peer error");
    }
    return "";
}

std::string Socket::GetAddress() const
{
    return m_address;
}

int Socket::GetPort() const
{
    return m_port;
}

int Socket::GetHander() const
{
    return m_socket;
}

int Socket::Domain2Domain(Domain domain)
{
    switch(domain)
    {
        case Domain::Inet:
            return AF_INET;
        case Domain::Local:
            return AF_UNIX;
        default: break;
    }
    return PF_UNSPEC;
}

int Socket::Type2Type(Type type)
{
    switch(type)
    {
        case Type::Stream:
            return SOCK_STREAM;
        case Type::Datagram:
            return SOCK_DGRAM;
        case Type::Raw:
            return SOCK_RAW;
        default: break;
    }
    return SOCK_STREAM;
}
