#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include "ComminucationUnixClient.h"


using namespace WebCpp;

ComminucationUnixClient::ComminucationUnixClient(const std::string& path):
    ICommunicationClient(SocketPool::Domain::Inet,
                         SocketPool::Type::Stream,
                         SocketPool::Options::ReuseAddr)
{
    m_sockets.SetHost(path);
}

bool ComminucationUnixClient::Init()
{
    if(m_initialized == true)
    {
        SetLastError("already initialized");
        return false;
    }

    m_initialized = ICommunicationClient::Init();
    return m_initialized;
}

