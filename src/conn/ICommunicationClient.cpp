#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <fcntl.h>
#include <cstring>
#include <stdexcept>
#include "DebugPrint.h"
#include "ICommunicationClient.h"


using namespace WebCpp;

bool ICommunicationClient::Init()
{
    bool retval = false;

    try
    {
        ClearError();
        m_socket = socket(AF_INET, SOCK_STREAM, 0);
        if(m_socket == (-1))
        {
            SetLastError(std::string("Socket creating error: ") + strerror(errno), errno);
            throw;
        }

        retval = true;
    }
    catch(...)
    {
        retval = false;
        if(m_socket >= 0)
        {
            close(m_socket);
            m_socket = (-1);
        }
        DebugPrint() << "CommunicationClient::Init error: " << GetLastError() << std::endl;
    }

    return retval;
}

bool ICommunicationClient::Connect(const std::string &address)
{
    struct hostent *hostinfo;
    struct sockaddr_in dest_addr;

    if(m_initialized == false)
    {
        SetLastError("Connect failed: connection not initialized");
        return false;
    }

    try
    {
        ParseAddress(address);

        if((hostinfo = gethostbyname(m_address.c_str())) == nullptr)
        {
            SetLastError(std::string("Error resolving the host name") + strerror(errno), errno);
            throw;
        }
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(m_port);
        dest_addr.sin_addr = *((struct in_addr *)hostinfo->h_addr);
        memset(&(dest_addr.sin_zero), 0, 8);

        if(connect(m_socket, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr)) == -1)
        {
            SetLastError(std::string("Connect error") + strerror(errno), errno);
            throw;
        }

        m_connected = true;
    }
    catch (...)
    {
        m_connected = false;
    }

    return m_connected;
}
