#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <cstring>
#include <stdexcept>
#include "Print.h"
#include "Lock.h"
#include "ICommunicationServer.h"


using namespace WebCpp;

bool ICommunicationServer::Init()
{
    bool retval;

    try
    {
        m_socket = socket(AF_INET, SOCK_STREAM, 0);
        if(m_socket == ERROR)
        {
            SetLastError(std::string("socket create error: ") + strerror(errno), errno);
            throw;
        }

        int opt = 1;
        if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == ERROR)
        {
            SetLastError(std::string("set socket option error: ") + strerror(errno), errno);
            throw;
        }

        retval = true;
    }

    catch(...)
    {
        CloseConnection(0);
        Print() << "CommunicationServer::Init error: " << GetLastError() << std::endl;
        retval = false;
    }

    return retval;
}

bool ICommunicationServer::Connect(const std::string &address)
{
    try
    {
        ParseAddress(address);
        struct sockaddr_in server_sockaddr;
        server_sockaddr.sin_family = AF_INET;
        server_sockaddr.sin_port = htons(m_port);
        if(m_address.empty() || m_address == "*")
        {
            server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        }
        else
        {
            server_sockaddr.sin_addr.s_addr = inet_addr(m_address.c_str());
        }

        if(bind(m_socket, (struct sockaddr* ) &server_sockaddr, sizeof(server_sockaddr)) == ERROR)
        {
            SetLastError(std::string("socket bind error: ") + strerror(errno), errno);
            throw;
        }

        if(listen(m_socket, QUEUE_SIZE) == -1)
        {
            SetLastError(std::string("socket listen error: ") + strerror(errno), errno);
            throw;
        }

        return true;
    }

    catch(...)
    {
        CloseConnection(0);
        Print() << "CommunicationServer::Connect error: " << GetLastError() << std::endl;
        return false;
    }
}

bool ICommunicationServer::CloseConnection(int connID)
{
    if(connID < 0 || connID > (MAX_CLIENTS + 1))
    {
        return false;
    }

    try
    {
        if(m_fds[connID].fd != (-1))
        {
            close(m_fds[connID].fd);
            m_fds[connID].fd = (-1);
            m_fds[connID].events = 0;
            m_fds[connID].revents = 0;

            return true;
        }
    }
    catch(...) { }

    return false;
}

void ICommunicationServer::CloseConnections()
{
    try
    {
        for (int i = 0; i < MAX_CLIENTS + 1; i++)
        {
            CloseConnection(i);
        }
    }
    catch(...)
    { }
}

bool ICommunicationServer::Write(int connID, ByteArray &data)
{
    return Write(connID, data, data.size());
}

bool ICommunicationServer::Write(int connID, ByteArray &data, size_t size)
{
    ClearError();

    if(m_initialized == false || m_connected == false)
    {
        SetLastError("not initialized or not connected");
        return false;
    }

    bool retval = false;
    Lock lock(m_writeMutex);

    size_t written = 0;
    try
    {
        if(connID < 0 || connID > MAX_CLIENTS)
        {
            SetLastError("connection ID isn't valid");
            throw;
        }

        int fd = m_fds[connID].fd;
        if(fd != (-1))
        {
            size_t pos = 0;
            while(pos < size)
            {
                ssize_t sent = send(fd, data.data() + pos, (size - pos), MSG_NOSIGNAL);
                if(sent == ERROR)
                {
                    if(errno != EAGAIN)
                    {
                        CloseConnection(connID);
                        break;
                    }
                }
                else
                {
                    pos += sent;
                }
            }

            retval = (pos == size);
        }
    }
    catch(const std::exception &ex)
    {
        SetLastError(std::string("CommunicationServer::Write() exception: ") + ex.what());
        retval = false;
    }

    return retval;
}
