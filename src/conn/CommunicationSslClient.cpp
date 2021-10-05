#ifdef WITH_OPENSSL
#include <sys/types.h>


#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>
#include <cstring>
#include <iostream>
#include "Lock.h"
#include "CommunicationSslClient.h"

#define POLL_TIMEOUT 1000


using namespace WebCpp;

CommunicationSslClient::CommunicationSslClient(const std::string &cert, const std::string &key) noexcept:
    m_cert(cert),
    m_key(key)
{
    m_protocol = ICommunication::CommunicationProtocol::TCP;
    m_type = ICommunication::ComminicationType::Client;
}

WebCpp::CommunicationSslClient::~CommunicationSslClient()
{

}

bool CommunicationSslClient::Init()
{
    if(m_initialized == true)
    {
        SetLastError("already initialized");
        return false;
    }

    try
    {
        ClearError();
        if(InitSSL() == false)
        {
            SetLastError(std::string("SSL init error"));
            throw std::runtime_error(GetLastError());
        }

        m_initialized = ICommunicationClient::Init();
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

bool CommunicationSslClient::Run()
{
    if(m_initialized)
    {
        m_poll.fd = m_socket;
        m_poll.events = POLLIN;

        m_running = true;
        if(pthread_create(&m_thread, nullptr, &CommunicationSslClient::ReadThreadWrapper, this) != 0)
        {
            SetLastError(strerror(errno), errno);
            m_running = false;
        }
    }

    return m_running;
}

bool CommunicationSslClient::Close(bool wait)
{
    if(m_initialized)
    {
        if(m_ssl != nullptr)
        {
            SSL_free(m_ssl);
        }
        close(m_socket);
        if(m_ctx != nullptr)
        {
            SSL_CTX_free(m_ctx);
        }

        m_socket = (-1);
        m_connected = false;
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

bool CommunicationSslClient::WaitFor()
{
    if(m_running)
    {
        pthread_join(m_thread, nullptr);
    }
    return true;
}

bool CommunicationSslClient::Connect(const std::string &address)
{
    ClearError();

    try
    {
        if(ICommunicationClient::Connect(address) == false)
        {
            return false;
        }

        m_ssl = SSL_new(m_ctx);
        SSL_set_fd(m_ssl, m_socket);
        if(SSL_connect(m_ssl) <= 0)
        {
            SetLastError(ERR_error_string(ERR_get_error(), NULL));
            throw;
        }
    }
    catch(...)
    {
        m_connected = false;
    }

    return m_connected;
}

bool CommunicationSslClient::Write(const ByteArray &data)
{
    bool retval = false;
    Lock lock(m_writeMutex);
    ClearError();

    try
    {
        if(m_initialized == false || m_connected == false)
        {
            SetLastError("not initialized ot not connected");
            throw;
        }

        int sent = SSL_write(m_ssl, data.data(), data.size());

        if(sent <= 0)
        {
            int errorCode = SSL_get_error(m_ssl, sent);
            SetLastError("write failed", errorCode);
            Close();
            retval = false;
        }
        else
        {
            retval = true;
        }

    }
    catch(const std::exception &ex)
    {
        retval = false;
    }

    return retval;
}

ByteArray CommunicationSslClient::Read(size_t length)
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
            int readBytes = SSL_read(m_ssl, m_readBuffer, toRead);
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

void *CommunicationSslClient::ReadThreadWrapper(void *ptr)
{
    CommunicationSslClient *instance = static_cast<CommunicationSslClient *>(ptr);
    if(instance)
    {
        instance->ReadThread();
    }

    return nullptr;
}

void *CommunicationSslClient::ReadThread()
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
                    int retval = SSL_read(m_ssl, m_readBuffer, BUFFER_SIZE);
                    if (retval <= 0)
                    {
                        int errorCode = SSL_get_error(m_ssl, retval);
                        if (errorCode == SSL_ERROR_WANT_READ)
                        {
                            readMore = false;
                        }
                        else
                        {
                            readMore = false;
                            Close();
                            if(m_closeConnectionCallback != nullptr)
                            {
                                m_closeConnectionCallback();
                            }
                        }
                    }
                    else
                    {
                        if(m_dataReadyCallback != nullptr)
                        {
                            m_dataReadyCallback(ByteArray(m_readBuffer, m_readBuffer + retval));
                        }
                        if(retval < BUFFER_SIZE)
                        {
                            readMore = false;
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

bool CommunicationSslClient::InitSSL()
{
    bool retval = true;

    try
    {
        OpenSSL_add_all_algorithms();
        SSL_load_error_strings();
        const SSL_METHOD *method = TLS_client_method();
        m_ctx = SSL_CTX_new(method);
        if(m_ctx == nullptr)
        {
            SetLastError(ERR_error_string(ERR_get_error(), nullptr));
            retval = false;
        }
    }
    catch(...)
    {
        SetLastError("SSL init failed");
        retval = false;
    }

    return retval;
}

#endif
