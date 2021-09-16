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

#define POLL_TIMEOUT 1000


using namespace WebCpp;

CommunicationTcpClient::CommunicationTcpClient()
{
    m_protocol = ICommunication::CommunicationProtocol::TCP;
    m_type = ICommunication::ComminicationType::Client;
}

bool CommunicationTcpClient::Init()
{
    try
    {
        ClearError();
        m_socket = socket(AF_INET, SOCK_STREAM, 0);
        if(m_socket == (-1))
        {
            SetLastError(std::string("Socket creating error: ") + strerror(errno), errno);
            throw;
        }

        m_initialized = true;
    }
    catch(...)
    {
        m_initialized = false;
        if(m_socket >= 0)
        {
            close(m_socket);
            m_socket = (-1);
        }
    }
    return m_initialized;
}

bool CommunicationTcpClient::Run()
{
    if(m_initialized)
    {
        m_poll.fd = m_socket;
        m_poll.events = POLLIN;

        m_running = true;
        if(pthread_create(&m_thread, nullptr, &CommunicationTcpClient::ReadThreadWrapper, this) != 0)
        {
            SetLastError(strerror(errno), errno);
            m_running = false;
        }
    }

    return m_running;
}

bool CommunicationTcpClient::Close(bool wait)
{
    if(m_initialized)
    {
        close(m_socket);
        m_connected = false;
        m_socket = (-1);
        m_initialized = false;
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

bool CommunicationTcpClient::WaitFor()
{
    if(m_running)
    {
        pthread_join(m_thread, nullptr);
    }
    return true;
}

bool CommunicationTcpClient::Connect(const std::string &address)
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

bool CommunicationTcpClient::Write(const ByteArray &data)
{
    ClearError();

    if(m_initialized == false || m_connected == false)
    {
        SetLastError("not initialized ot not connected");
        return false;
    }

    bool retval = false;
    try
    {
        size_t sentBytes = send(m_socket, data.data(), data.size(), MSG_NOSIGNAL);
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

ByteArray CommunicationTcpClient::Read(size_t length)
{
    bool readMore = true;
    ByteArray data;

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
            ssize_t readBytes = recv(m_socket, &m_readBuffer, toRead, MSG_DONTWAIT);
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

void *CommunicationTcpClient::ReadThreadWrapper(void *ptr)
{
    CommunicationTcpClient *instance = static_cast<CommunicationTcpClient *>(ptr);
    if(instance)
    {
        instance->ReadThread();
    }

    return nullptr;
}

void *CommunicationTcpClient::ReadThread()
{
    ClearError();

    while(m_running)
    {
        try
        {
            int status = poll(&m_poll, 1, POLL_TIMEOUT);
            if(status > 0 && (m_poll.revents & POLLIN))
            {
                bool readMore = true;

                ByteArray data;
                do
                {
                    ssize_t readBytes = recv(m_poll.fd, &m_readBuffer, BUFFER_SIZE, MSG_DONTWAIT);
                    if (readBytes < 0)
                    {
                        if (errno == EWOULDBLOCK)
                        {
                            readMore = false;
                        }
                        else
                        {
                            readMore = false;
                        }
                    }
                    else if(readBytes > 0)
                    {
                        if(m_dataReadyCallback != nullptr)
                        {
                            m_dataReadyCallback(ByteArray(m_readBuffer, m_readBuffer + readBytes));
                        }
                    }
                    else // the peer connection probably has closed
                    {
                        close(m_socket);
                        readMore = false;
                        m_connected = false;
                        if(m_closeConnectionCallback != nullptr)
                        {
                            m_closeConnectionCallback();
                        }
                    }
                }
                while(readMore == true);
            }
        }
        catch(...)
        {
            SetLastError("read thread unexpected error");
        }
    }

    return nullptr;
}
