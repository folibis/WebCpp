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

bool ICommunicationClient::Init()
{
    bool retval;
    ClearError();

    try
    {
        if(m_sockets.Create(true) == false)
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

bool ICommunicationClient::Connect(const std::string &address)
{
    ClearError();

    try
    {
        if(m_sockets.Connect(address) == false)
        {
            SetLastError(std::string("socket connect error: ") + m_sockets.GetLastError());
            throw std::runtime_error(GetLastError());
        }

        return true;
    }

    catch(...)
    {
        CloseConnection();
        DebugPrint() << "ICommunicationClient::Connect error: " << GetLastError() << std::endl;
        return false;
    }
}

bool ICommunicationClient::CloseConnection()
{
    return m_sockets.CloseSocket(0);
}

bool ICommunicationClient::Run()
{
    ClearError();

    if(m_initialized)
    {
        if(pthread_create(&m_thread, nullptr, &ICommunicationClient::ReadThreadWrapper, this) != 0)
        {
            SetLastError(strerror(errno), errno);
            m_running = false;
        }
    }

    return m_running;
}

bool ICommunicationClient::Close(bool wait)
{
    if(m_initialized)
    {
        CloseConnection();
    }

    if(m_running)
    {
        m_running = false;
        if(wait)
        {
            pthread_join(m_thread, nullptr);
        }
    }

    return true;
}

bool ICommunicationClient::WaitFor()
{
    if(m_running)
    {
        pthread_join(m_thread, nullptr);
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

void *ICommunicationClient::ReadThreadWrapper(void *ptr)
{
    ICommunicationClient *instance = static_cast<ICommunicationClient *>(ptr);
    if(instance)
    {
        instance->ReadThread();
    }

    return nullptr;
}

void *ICommunicationClient::ReadThread()
{
    ClearError();

    while(m_running)
    {
        try
        {
            if(m_sockets.Poll())
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
        catch(...)
        {
            SetLastError("read thread unexpected error");
        }
    }

    return nullptr;
}
