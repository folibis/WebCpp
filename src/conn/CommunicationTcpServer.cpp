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
#include "CommunicationTcpServer.h"

#define POLL_TIMEOUT 1000 // milliseconds
#define READ_FAIL_COUNT 10


using namespace WebCpp;

CommunicationTcpServer::CommunicationTcpServer() noexcept
{
    m_protocol = ICommunication::CommunicationProtocol::TCP;
    m_type = ICommunication::ComminicationType::Server;
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

bool CommunicationTcpServer::Run()
{
    try
    {
        std::memset(m_fds, 0, sizeof(m_fds));
        for (int i = 0; i < (MAX_CLIENTS + 1); i++)
        {
            m_fds[i].fd = (-1);
        }

        // index 0 is always the main socket
        m_fds[0].fd = m_socket;
        m_fds[0].events = POLLIN;
        m_fds[0].revents = 0;

        m_running = true;
        if(pthread_create(&m_readThread, nullptr, &CommunicationTcpServer::ReadThreadWrapper, this) != 0)
        {
            SetLastError(strerror(errno), errno);
            m_running = false;
        }

        return true;
    }
    catch(...)
    {
        return false;
    }
}

bool CommunicationTcpServer::WaitFor()
{
    pthread_join(m_readThread, nullptr);
    return true;
}

bool CommunicationTcpServer::Close(bool wait)
{
    if(m_running == true)
    {
        m_running = false;
        if(wait)
        {
            pthread_join(m_readThread, nullptr);
        }

        CloseConnections();
    }

    return true;
}

bool CommunicationTcpServer::CloseConnection(int connID)
{
    bool retval = ICommunicationServer::CloseConnection(connID);
    if(retval)
    {
        if(m_closeConnectionCallback != nullptr)
        {
            m_closeConnectionCallback(connID);
        }
    }

    return retval;
}

void *CommunicationTcpServer::ReadThreadWrapper(void *ptr)
{
    CommunicationTcpServer *instance = static_cast<CommunicationTcpServer *>(ptr);
    if(instance != nullptr)
    {
        return instance->ReadThread();
    }

    return nullptr;
}

void *CommunicationTcpServer::ReadThread()
{
    int retval = (-1);

    try
    {
        while(m_running)
        {
            retval = poll(m_fds, MAX_CLIENTS + 1, POLL_TIMEOUT);
            if(retval > 0)
            {
                for (int i = 0; i < MAX_CLIENTS + 1; i++)
                {
                    if(m_fds[i].revents == 0)
                    {
                        continue;
                    }

                    if(m_fds[i].revents == POLLIN)
                    {
                        if (i == 0) // new client connected
                        {
                            int new_socket = accept(m_socket, NULL, NULL);
                            if (new_socket < 0)
                            {
                                close(new_socket);
                                continue;
                            }

                            fcntl(new_socket, F_SETFL, O_NONBLOCK);

                            for(int j = 1;j < MAX_CLIENTS + 1;j ++) // looks for free space for the new socket
                            {
                                if(m_fds[j].fd == (-1))
                                {
                                    m_fds[j].fd = new_socket;
                                    m_fds[j].events = POLLIN;
                                    if(m_newConnectionCallback != nullptr)
                                    {
                                        struct sockaddr_in client_sockaddr = {};
                                        socklen_t len = sizeof(client_sockaddr);
                                        std::string remote;
                                        if (getpeername(new_socket, reinterpret_cast<struct sockaddr *>(&client_sockaddr), &len ) != -1)
                                        {
                                            remote = std::string(inet_ntoa(client_sockaddr.sin_addr)) + ":" + std::to_string(ntohs(client_sockaddr.sin_port));
                                        }
                                        m_newConnectionCallback(j,remote);
                                    }
                                    break;
                                }
                            }
                        }
                        else // existing socket data received
                        {
                            bool readMore = true;
                            bool isError = false;

                            ByteArray data;
                            do
                            {
                                retval = recv(m_fds[i].fd, m_readBuffer, READ_BUFFER_SIZE, MSG_DONTWAIT);
                                if (retval < 0)
                                {
                                    if (errno == EWOULDBLOCK)
                                    {
                                        readMore = false;
                                    }
                                    else
                                    {
                                        isError = true;
                                        CloseConnection(i);
                                        readMore = false;
                                    }
                                }
                                else if(retval > 0)
                                {
                                    data.insert(data.end(), m_readBuffer, m_readBuffer + retval);
                                }
                                else // the peer connection probably has closed
                                {
                                    readMore = false;
                                    isError = true;
                                    CloseConnection(i);
                                }
                            }
                            while(readMore == true);

                            if(isError == false)
                            {
                                if(m_dataReadyCallback != nullptr)
                                {
                                    m_dataReadyCallback(i, data);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    catch(...)
    {
        DebugPrint() << "critical unexpected error occured in the read thread" << std::endl;
    }

    return nullptr;
}
