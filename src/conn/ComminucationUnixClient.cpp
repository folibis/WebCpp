#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include "ComminucationUnixClient.h"

#define MAX_CLIENTS 10
#define POLL_TIMEOUT 1000


using namespace WebCpp;

ComminucationUnixClient::ComminucationUnixClient(const std::string& path)
{
    m_address = path;
    m_protocol = CommunicationProtocol::UnixDoamin;
    m_type = ComminicationType::Client;
}

bool ComminucationUnixClient::Init()
{
    try
    {
        ClearError();
        m_socket = socket(AF_UNIX, SOCK_STREAM,0);
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

bool ComminucationUnixClient::Run()
{
    if(m_initialized)
    {
        m_poll.fd = m_socket;
        m_poll.events = POLLIN;

        m_running = true;
        if(pthread_create(&m_thread, nullptr, &ComminucationUnixClient::ReadThreadWrapper, this) != 0)
        {
            SetLastError(strerror(errno), errno);
            m_running = false;
        }
    }

    return m_running;
}

bool ComminucationUnixClient::Close(bool wait)
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

bool ComminucationUnixClient::WaitFor()
{
    pthread_join(m_thread, nullptr);
    return true;
}

bool ComminucationUnixClient::Connect(const std::string &address)
{
    if(m_connected)
    {
        return true;
    }

    socklen_t len;
    struct sockaddr_un addr;

    try
    {
        ClearError();

        if(!address.empty())
        {
            m_address = address;
        }

        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, m_address.c_str(), m_address.size());
        //*addr.sun_path = '\0';

        len = static_cast<socklen_t>(__builtin_offsetof(struct sockaddr_un, sun_path) + m_address.length());
        if(connect(m_socket, reinterpret_cast<struct sockaddr *>(&addr), len) == (-1))
        {
            SetLastError(std::string("Socket creating error: ") + strerror(errno), errno);
            throw;
        }

        m_connected = true;
    }
    catch(...)
    {
        m_connected = false;
    }

    return m_connected;
}

bool ComminucationUnixClient::Write(const ByteArray &data)
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
            throw;
        }
        retval = true;
    }
    catch(...)
    {
        SetLastError("Write failed: " + GetLastError());
    }


    return retval;
}

ByteArray ComminucationUnixClient::Read(size_t length)
{
    length = (length == 0 ? BUFFER_SIZE : length);
    bool readMore = true;
    ByteArray data;

    try
    {
        do
        {
            ssize_t readBytes = recv(m_socket, &m_readBuffer, length, MSG_DONTWAIT);
            if(readBytes > 0)
            {
                data.insert(data.end(), m_readBuffer, m_readBuffer + readBytes);
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

void *ComminucationUnixClient::ReadThreadWrapper(void *ptr)
{
    ComminucationUnixClient *instance = static_cast<ComminucationUnixClient *>(ptr);
    if(instance)
    {
        instance->ReadThread();
    }

    return nullptr;
}

void *ComminucationUnixClient::ReadThread()
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
                bool isError = false;

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
                            isError = true;
                            readMore = false;
                        }
                    }
                    else if(readBytes > 0)
                    {
                        data.insert(data.end(), m_readBuffer, m_readBuffer + readBytes);
                    }
                    else // the peer connection probably has closed
                    {
                        close(m_socket);
                        readMore = false;
                        isError = true;
                        m_connected = false;
                        if(m_closeConnectionCallback != nullptr)
                        {
                            m_closeConnectionCallback();
                        }
                    }
                }
                while(readMore == true);

                if(isError == false)
                {
                    if(m_dataReadyCallback != nullptr)
                    {
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
