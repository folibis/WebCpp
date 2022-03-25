#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>
#include <cstring>
#include "CommunicationTcpClient.h"


using namespace WebCpp;

CommunicationTcpClient::CommunicationTcpClient():
    ICommunicationClient(SocketPool::Domain::Inet,
                         SocketPool::Type::Stream,
                         SocketPool::Options::ReuseAddr)
{

}

CommunicationTcpClient::~CommunicationTcpClient()
{
    Close();
}

bool CommunicationTcpClient::Init()
{
    if(m_initialized == true)
    {
        SetLastError("already initialized");
        return false;
    }

    m_initialized = ICommunicationClient::Init();
    return m_initialized;
}

