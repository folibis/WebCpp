#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdexcept>
#include <fcntl.h>
#include <cstring>
#include <iostream>
#include "common.h"
#include "CommunicationTcpServer.h"

#define QUEUE_SIZE 10
#define POLL_TIMEOUT 1000 // milliseconds


using namespace WebCpp;

CommunicationTcpServer::CommunicationTcpServer() noexcept
{
    m_protocol = ICommunication::CommunicationProtocol::TCP;
    m_type = ICommunication::ComminicationType::Server;
}

bool CommunicationTcpServer::Init()
{
    bool retval;

    try
    {
        m_socket = socket(AF_INET, SOCK_STREAM, 0);
        if(m_socket == ERROR)
        {
            SetLastError(std::string("socket create error: ") + strerror(errno), errno);
            throw std::runtime_error(GetLastError());
        }

        int opt = 1;
        if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == ERROR)
        {
            SetLastError(std::string("set socket option error: ") + strerror(errno), errno);
            throw std::runtime_error(GetLastError());
        }

        retval = true;
    }
    catch(const std::exception &ex)
    {
        std::cout << "CommunicationTcpServer::Init error: " << ex.what() << std::endl;
        CloseConnections();
        retval = false;
    }
    catch(...)
    {
        std::cout << "CommunicationTcpServer::Init unexpected error" << std::endl;
        CloseConnections();
        retval = false;
    }

    return retval;
}

bool CommunicationTcpServer::Connect(const std::string &address)
{
    try
    {
        if(!address.empty())
        {
            auto addr_arr = split(address, ':');
            if(addr_arr.size() == 2)
            {
                m_address = addr_arr[0];
                int port;
                if(string2int(addr_arr[1], port))
                {
                    m_port = port;
                }
            }
        }

        struct sockaddr_in server_sockaddr;
        server_sockaddr.sin_family = AF_INET;
        server_sockaddr.sin_port = htons(m_port);
        server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        if(bind(m_socket, (struct sockaddr* ) &server_sockaddr, sizeof(server_sockaddr)) == ERROR)
        {
            SetLastError(std::string("socket bind error: ") + strerror(errno), errno);
            throw std::runtime_error(GetLastError());
        }

        if(listen(m_socket, QUEUE_SIZE) == -1)
        {
            SetLastError(std::string("socket listen error: ") + strerror(errno), errno);
            throw std::runtime_error(GetLastError());
        }

        memset(m_fds, 0, sizeof(m_fds));
        for (int i = 0; i < (MAX_CLIENTS + 1); i++)
        {
            m_fds[i].fd = (-1);
        }

        // index 0 is always the main socket
        m_fds[0].fd = m_socket;
        m_fds[0].events = POLLIN;

        m_running = true;
        if(pthread_create(&m_readThread, nullptr, &CommunicationTcpServer::ReadThreadWrapper, this) != 0)
        {
            SetLastError(strerror(errno), errno);
            m_running = false;
        }

        return true;
    }
    catch(const std::exception &ex)
    {
        std::cout << "CommunicationTcpServer::Connect error: " << ex.what() << std::endl;
        CloseConnections();
        return false;
    }
    catch(...)
    {
        std::cout << "CommunicationTcpServer::Connect unexpected error" << std::endl;
        CloseConnections();
        return false;
    }
}

bool CommunicationTcpServer::Close( bool wait)
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

bool CommunicationTcpServer::WaitFor()
{
    pthread_join(m_readThread, nullptr);
    return true;
}


void CommunicationTcpServer::CloseConnections()
{
    try
    {
        for (int i = 0; i < MAX_CLIENTS + 1; i++)
        {
            if(m_fds[i].fd != (-1))
            {
                close(m_fds[i].fd);
                m_fds[i].fd = (-1);
                m_fds->events = 0;
            }
        }
    }
    catch(...)
    { }
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
                                        m_newConnectionCallback(j);
                                    }
                                    break;
                                }
                            }
                        }
                        else // existing socket data received
                        {
                            bool readMore = true;
                            bool isError = false;
                            byte_array data;
                            do
                            {
                                retval = recv(m_fds[i].fd, m_readBuffer, READ_BUFFER_SIZE, 0);
                                if (retval < 0)
                                {
                                    readMore = false;
                                    if (errno != EWOULDBLOCK)
                                    {
                                        isError = true;
                                        m_fds[i].fd = (-1);
                                        m_fds[i].events = 0;
                                        if(m_closeConnectionCallback != nullptr)
                                        {
                                            m_closeConnectionCallback(i);
                                        }
                                    }
                                    break;
                                }
                                else if(retval == 0) // the peer connection probably has closed
                                {
                                    m_fds[i].fd = (-1);
                                    m_fds[i].events = 0;
                                    readMore = false;
                                    isError = true;
                                    if(m_closeConnectionCallback != nullptr)
                                    {
                                        m_closeConnectionCallback(i);
                                    }
                                    break;
                                }
                                else
                                {
                                    data.insert(data.end(), m_readBuffer, m_readBuffer + retval);
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
        std::cout << "critical unexpected error occured in the read thread" << std::endl;
    }

    return nullptr;
}

bool CommunicationTcpServer::SetNewConnectionCallback(const std::function<void (int)> &callback)
{
    m_newConnectionCallback = callback;
    return true;
}

bool CommunicationTcpServer::SetDataReadyCallback(const std::function<void(int, const std::vector<char> &)> &callback)
{
    m_dataReadyCallback = callback;
    return true;
}

bool CommunicationTcpServer::SetCloseConnectionCallback(const std::function<void (int)> &callback)
{
    m_closeConnectionCallback = callback;
    return true;
}
