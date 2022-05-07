#ifdef WITH_OPENSSL
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>
#include <cstring>
#include <iostream>
#include <signal.h>
#include "Lock.h"
#include "CommunicationSslClient.h"


using namespace WebCpp;

CommunicationSslClient::CommunicationSslClient(const std::string &cert, const std::string &key) noexcept:

    ICommunicationClient(SocketPool::Domain::Inet,
                         SocketPool::Type::Stream,
                         SocketPool::Options::ReuseAddr | SocketPool::Options::Ssl)
{
    m_sockets.SetSslCredentials(cert, key);
    m_sockets.SetPort(DEFAULT_SSL_PORT);
}

CommunicationSslClient::~CommunicationSslClient()
{
    Close();
}


bool CommunicationSslClient::Init()
{
    signal(SIGPIPE, SIG_IGN);
    if(m_initialized == true)
    {
        SetLastError("already initialized");
        return false;
    }

    m_initialized = ICommunicationClient::Init();

    return m_initialized;
}

#endif
