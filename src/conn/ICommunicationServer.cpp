#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <cstring>
#include <stdexcept>
#include "DebugPrint.h"
#include "Lock.h"
#include "ICommunicationServer.h"


using namespace WebCpp;

ICommunicationServer::ICommunicationServer(SocketPool::Domain domain,
                                           SocketPool::Type type,
                                           SocketPool::Options options):
    m_sockets(MAX_CLIENTS + 1, SocketPool::Service::Server, domain, type, options)
{

}

void ICommunicationServer::SetPort(int port)
{
    m_sockets.SetPort(port);
}

int ICommunicationServer::GetPort() const
{
    return m_sockets.GetPort();
}

void ICommunicationServer::SetHost(const std::string &host)
{
    m_sockets.SetHost(host);
}

std::string ICommunicationServer::GetHost() const
{
    return m_sockets.GetHost();
}

bool ICommunicationServer::Init()
{
    ClearError();
    bool retval;

    try
    {
        if(m_sockets.Create(true) == ERROR)
        {
            SetLastError(std::string("server socket create error: ") + m_sockets.GetLastError());
            throw std::runtime_error(GetLastError());
        }

        retval = true;
    }

    catch(...)
    {
        CloseConnection(0);
        DebugPrint() << "CommunicationServer::Init error: " << GetLastError() << std::endl;
        retval = false;
    }

    return retval;
}

bool ICommunicationServer::Run()
{
    ClearError();

    try
    {
        auto f = std::bind(&ICommunicationServer::ReadThread, this, std::placeholders::_1);
        m_readThread.SetFunction(f);
        m_running = m_readThread.Start();
        if(m_running == false)
        {
            SetLastError(m_readThread.GetLastError());
        }
    }
    catch(...)
    {
        m_running = false;
    }

    return m_running;
}

bool ICommunicationServer::WaitFor()
{
    m_readThread.Wait();
    return true;
}

bool ICommunicationServer::Connect(const std::string &host, int port)
{
    ClearError();

    try
    {
        if(m_sockets.Bind(host, port) == false)
        {
            SetLastError(std::string("socket bind error: ") + m_sockets.GetLastError());
            throw std::runtime_error(GetLastError());
        }

        if(m_sockets.Listen() == false)
        {
            SetLastError(std::string("socket listen error: ") + m_sockets.GetLastError());
            throw std::runtime_error(GetLastError());
        }

        return true;
    }

    catch(...)
    {
        CloseConnection(0);
        DebugPrint() << "CommunicationServer::Connect error: " << GetLastError() << std::endl;
        return false;
    }
}

bool ICommunicationServer::Close(bool wait)
{
    if(m_running == true)
    {
        m_running = false;
        if(wait)
        {
            m_readThread.Wait();
        }

        CloseConnections();
    }

    return true;
}

bool ICommunicationServer::CloseConnection(int connID)
{   
    bool retval = m_sockets.CloseSocket(connID);
    if(m_closeConnectionCallback != nullptr)
    {
        m_closeConnectionCallback(connID);
    }

    return retval;
}

void ICommunicationServer::CloseConnections()
{  
    m_sockets.CloseSockets();
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

    try
    {
        auto pos = m_sockets.Write(data.data(), size, connID);
        retval = (pos == size);
        if(retval == false)
        {
            SetLastError("send " + std::to_string(pos) + " of " + std::to_string(size) + " bytes");
        }
    }
    catch(const std::exception &ex)
    {
        SetLastError(std::string("CommunicationServer::Write() exception: ") + ex.what());
        retval = false;
    }

    return retval;
}

void *ICommunicationServer::ReadThread(bool &running)
{
    int retval = (-1);

    try
    {
        m_sockets.SetPollRead();
        while(running)
        {
            if(m_sockets.Poll())
            {
                for (int i = 0; i < m_sockets.GetCount(); i++)
                {
                    if(m_sockets.IsPollError(i))
                    {
                        CloseConnection(i);
                    }
                    else if(m_sockets.HasData(i))
                    {
                        if (i == 0) // new client connected
                        {
                            int id = m_sockets.Accept();
                            if(id != ERROR)
                            {
                                if(m_newConnectionCallback != nullptr)
                                {
                                    m_newConnectionCallback(id, m_sockets.GetRemoteAddress(id));
                                }
                            }
                        }
                        else // existing socket data received
                        {
                            auto readBytes = m_sockets.Read(m_readBuffer, READ_BUFFER_SIZE, i);
                            if(readBytes != ERROR)
                            {
                                if(m_dataReadyCallback != nullptr)
                                {
                                    ByteArray data;
                                    data.insert(data.end(), m_readBuffer, m_readBuffer + readBytes);
                                    m_dataReadyCallback(i, data);
                                }
                            }
                            else
                            {
                                CloseConnection(i);
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
