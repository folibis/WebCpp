#ifdef WITH_OPENSSL
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <signal.h>
#include "common_webcpp.h"
#include "Lock.h"
#include "StringUtil.h"
#include "CommunicationSslServer.h"


using namespace WebCpp;

CommunicationSslServer::CommunicationSslServer(const std::string &cert, const std::string &key) noexcept:
    ICommunicationServer(SocketPool::Domain::Inet,
                         SocketPool::Type::Stream,
                         SocketPool::Options::ReuseAddr | SocketPool::Options::Ssl)
{
    m_sockets.SetSslCredentials(cert, key);
}

bool CommunicationSslServer::Init()
{
    ClearError();
    signal(SIGPIPE, SIG_IGN);

    if(m_initialized == true)
    {
        SetLastError("already initialized");
        return false;
    }

    m_initialized = ICommunicationServer::Init();

    return m_initialized;
}

bool CommunicationSslServer::Connect(const std::string &address)
{
    ClearError();

    if(m_initialized == false)
    {
        SetLastError("not initialized");
        return false;
    }

    m_connected = ICommunicationServer::Connect(address);
    return m_connected;
}

#endif
