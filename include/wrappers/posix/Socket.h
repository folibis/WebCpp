#ifndef SOCKET_H
#define SOCKET_H

#include "IErrorable.h"


namespace WebCpp
{

class Socket: public IErrorable
{
public:
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
    };

    Socket();
    Socket(Domain domain, Type type, Options options = Options::None) noexcept;
    Socket(const Socket& other) = delete;
    Socket& operator=(const Socket& other) = delete;
    Socket(Socket&& other) = default;
    Socket& operator=(Socket&& other) = default;

    inline bool IsValid() const;
    bool Bind(const std::string &address, int port);
    bool Listen();
    size_t Write(const void *buffer, size_t size, bool &again);
    size_t Read(void *buffer, size_t size, bool &again);
    bool Close();

    Socket Accept();
    std::string GetPeer();
    inline std::string GetAddress() const;
    inline int GetPort() const;
    int GetHander() const;

    static int Domain2Domain(Domain domain);
    static int Type2Type(Type type);

protected:
    Socket(int fd, Domain domain, Type type, Options options = Options::None) noexcept;
    bool Create();

private:
    Domain m_domain = Domain::Undefined;
    Type m_type = Type::Undefined;
    Options m_options = Options::None;
    int m_socket = (-1);
    std::string m_address;
    int m_port;
};

inline Socket::Options operator |(Socket::Options a, Socket::Options b)
{
    return static_cast<Socket::Options>(static_cast<int>(a) | static_cast<int>(b));
}
inline Socket::Options operator &(Socket::Options a, Socket::Options b)
{
    return static_cast<Socket::Options>(static_cast<int>(a) & static_cast<int>(b));
}
inline bool operator ==(Socket::Options a, Socket::Options b)
{
    return (static_cast<int>(a) == static_cast<int>(b));
}

}

#endif // SOCKET_H
