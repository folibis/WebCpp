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

ICommunicationClient::ICommunicationClient(SocketPool::Domain domain,
                                           SocketPool::Type type,
                                           SocketPool::Options options):
    m_sockets(1, SocketPool::Service::Client, domain, type, options)
{

}

void ICommunicationClient::SetPort(int port)
{
    m_sockets.SetPort(port);
}

int ICommunicationClient::GetPort() const
{
    return m_sockets.GetPort();
}

void ICommunicationClient::SetHost(const std::string &host)
{
    m_sockets.SetHost(host);
}

std::string ICommunicationClient::GetHost() const
{
    return m_sockets.GetHost();
}

bool ICommunicationClient::Init()
{
    bool retval;
    ClearError();

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
        CloseConnection();
        DebugPrint() << "ICommunicationClient::Init error: " << GetLastError() << std::endl;
        retval = false;
    }

    return retval;
}

bool ICommunicationClient::Connect(const std::string &host, int port)
{
    ClearError();

    try
    {
        if(m_sockets.Connect(host, port) == false)
        {
            SetLastError(std::string("socket connect error: ") + m_sockets.GetLastError());
            throw std::runtime_error(GetLastError());
        }

        m_connected = true;
        return m_connected;
    }
    catch(...)
    {
        m_connected = false;
        CloseConnection();
        DebugPrint() << "ICommunicationClient::Connect error: " << GetLastError() << std::endl;
        return false;
    }
}

bool ICommunicationClient::CloseConnection()
{
    bool retval = false;
    if(m_sockets.IsSocketValid(0))
    {
        retval = m_sockets.CloseSocket(0);
        if(m_closeConnectionCallback != nullptr)
        {
            m_closeConnectionCallback();
        }
    }
    m_connected = false;
    m_initialized = false;

    return retval;
}

bool ICommunicationClient::Run()
{
    ClearError();

    if(m_initialized)
    {
        auto f = std::bind(&ICommunicationClient::ReadThread, this, std::placeholders::_1);
        m_thread.SetFunction(f);
        m_running = m_thread.Start();
        if(m_running == false)
        {
            SetLastError(m_thread.GetLastError());
        }
    }

    return m_running;
}

bool ICommunicationClient::Close(bool wait)
{
    if(m_initialized && m_connected)
    {
        CloseConnection();
    }

    if(m_running)
    {
        m_running = false;
        m_thread.Stop(wait);
    }

    return true;
}

bool ICommunicationClient::WaitFor()
{
    if(m_running)
    {
        m_thread.Wait();
    }
    return true;
}

bool ICommunicationClient::Write(const ByteArray &data)
{
    ClearError();
    bool retval = false;

    if(m_initialized == false || m_connected == false)
    {
        SetLastError("not initialized ot not connected");
        return false;
    }

    try
    {
        size_t sentBytes = m_sockets.Write(data.data(), data.size());
        if(data.size() != sentBytes)
        {
            SetLastError(std::string("Send error: ") + strerror(errno), errno);
            throw GetLastError();
        }
        retval = true;
    }
    catch(...)
    {
        SetLastError("Write failed: " + GetLastError());
    }

    return retval;
}

ByteArray ICommunicationClient::Read(size_t length)
{
    ClearError();
    bool readMore = true;
    ByteArray data(BUFFER_SIZE);

    try
    {
        size_t readSize = length > BUFFER_SIZE ? BUFFER_SIZE : length;
        size_t all = 0;
        size_t toRead;
        do
        {
            toRead = length - all;
            if(toRead > readSize)
            {
                toRead = readSize;
            }
            auto readBytes = m_sockets.Read(&m_readBuffer, toRead);
            if(readBytes > 0)
            {
                data.insert(data.end(), m_readBuffer, m_readBuffer + readBytes);
                all += readBytes;
                if(all >= length)
                {
                    readMore = false;
                }
            }
            else
            {
                readMore = false;
            }
        }
        while(readMore);
    }
    catch(...)
    {

    }

    return data;
}

void *ICommunicationClient::ReadThread(bool &running)
{
    ClearError();

    while(running)
    {
        m_sockets.SetPollRead();
        try
        {
            if(m_sockets.Poll())
            {
                if(m_sockets.IsPollError(0))
                {
                    CloseConnection();
                }
                else
                {
                    auto readBytes = m_sockets.Read(m_readBuffer, BUFFER_SIZE);
                    if(readBytes == ERROR)
                    {
                        CloseConnection();
                    }
                    else if(readBytes > 0)
                    {
                        if(m_dataReadyCallback != nullptr)
                        {
                            ByteArray data;
                            data.insert(data.end(), m_readBuffer, m_readBuffer + readBytes);
                            m_dataReadyCallback(data);
                        }
                    }
                }
            }
        }
        catch(...)
        {
            SetLastError("read thread unexpected error");
        }
    }

    return nullptr;
}
