#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdexcept>
#include <fcntl.h>
#include <cstring>
#include <iostream>
#include "common_webcpp.h"
#include "Lock.h"
#include "Print.h"
#include "StringUtil.h"
#include "CommunicationTcpServer.h"

#define QUEUE_SIZE 10
#define POLL_TIMEOUT 1000 // milliseconds
#define WRITE_MAX_SIZE 1500
#define READ_FAIL_COUNT 10


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
        Print() << "CommunicationTcpServer::Init error: " << ex.what() << std::endl;
        CloseConnections();
        retval = false;
    }
    catch(...)
    {
        Print() << "CommunicationTcpServer::Init unexpected error" << std::endl;
        CloseConnections();
        retval = false;
    }

    return retval;
}

bool CommunicationTcpServer::Connect(const std::string &address)
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
            throw std::runtime_error(GetLastError());
        }

        if(listen(m_socket, QUEUE_SIZE) == -1)
        {
            SetLastError(std::string("socket listen error: ") + strerror(errno), errno);
            throw std::runtime_error(GetLastError());
        }

        return true;
    }
    catch(const std::exception &ex)
    {
        Print() << "CommunicationTcpServer::Connect error: " << ex.what() << std::endl;
        CloseConnections();
        return false;
    }
    catch(...)
    {
        Print() << "CommunicationTcpServer::Connect unexpected error" << std::endl;
        CloseConnections();
        return false;
    }
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

bool CommunicationTcpServer::Write(int connID, ByteArray &data)
{
    return Write(connID, data, data.size());
}

bool CommunicationTcpServer::Write(int connID, ByteArray &data, size_t size)
{
    bool retval = false;
    Lock lock(m_writeMutex);

    size_t written = 0;
    try
    {
        if(connID < 0 || connID > MAX_CLIENTS)
        {
            throw std::runtime_error("connection ID isn't valid");
        }

        int fd = m_fds[connID].fd;
        bool finished = false;
        if(fd != (-1))
        {
            while(!finished)
            {
                size_t s = (size - written) > WRITE_MAX_SIZE ? WRITE_MAX_SIZE : size;
                ssize_t sent = send(fd, data.data() + written, s, MSG_NOSIGNAL);
                std::cout << "write " << sent << " bytes" << std::endl;
                if(sent == ERROR)
                {
                    CloseClient(connID);
                    retval = false;
                    finished = true;
                }
                else
                {
                    written += sent;
                    if(written >= size)
                    {
                        finished = true;
                    }
                    retval = true;
                }
            }
        }
    }
    catch(const std::exception &ex)
    {
        Print() << "CommunicationTcpServer::Write() exception: " << ex.what() << std::endl;
        retval = false;
    }

    return retval;
}

bool CommunicationTcpServer::CloseClient(int connID)
{
    if(connID < 0 || connID > (MAX_CLIENTS + 1))
    {
        return false;
    }

    if(m_fds[connID].fd != (-1))
    {
        close(m_fds[connID].fd);
        m_fds[connID].fd = (-1);
        m_fds[connID].events = 0;
        m_fds[connID].revents = 0;

        if(m_closeConnectionCallback != nullptr)
        {
            m_closeConnectionCallback(connID);
        }

        return true;
    }

    return false;
}

void CommunicationTcpServer::CloseConnections()
{
    try
    {
        for (int i = 0; i < MAX_CLIENTS + 1; i++)
        {
            CloseClient(i);
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
                                        CloseClient(i);
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
                                    CloseClient(i);
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
        Print() << "critical unexpected error occured in the read thread" << std::endl;
    }

    return nullptr;
}
