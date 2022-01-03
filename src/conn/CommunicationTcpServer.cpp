#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include "common_webcpp.h"
#include "Lock.h"
#include "DebugPrint.h"
#include "StringUtil.h"
#include "SocketPool.h"
#include "CommunicationTcpServer.h"


using namespace WebCpp;

CommunicationTcpServer::CommunicationTcpServer() noexcept:
    ICommunicationServer(SocketPool::Domain::Inet,
                         SocketPool::Type::Stream,
                         SocketPool::Options::ReuseAddr)
{
}

CommunicationTcpServer::~CommunicationTcpServer()
{
    CommunicationTcpServer::Close();
}

bool CommunicationTcpServer::Init()
{
    if(m_initialized == true)
    {
        SetLastError("already initialized");
        return false;
    }

    m_initialized = ICommunicationServer::Init();
    return m_initialized;
}

bool CommunicationTcpServer::Connect(const std::string &address)
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
