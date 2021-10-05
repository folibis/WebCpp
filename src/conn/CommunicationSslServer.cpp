#ifdef WITH_OPENSSL
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include "common_webcpp.h"
#include "Lock.h"
#include "StringUtil.h"
#include "CommunicationSslServer.h"


#define QUEUE_SIZE 10
#define POLL_TIMEOUT 1000 // milliseconds
#define WRITE_MAX_SIZE 1500


using namespace WebCpp;

CommunicationSslServer::CommunicationSslServer(const std::string &cert, const std::string &key) noexcept:
    m_cert(cert),
    m_key(key)
{

}

bool CommunicationSslServer::Init()
{
    ClearError();

    if(m_initialized == true)
    {
        SetLastError("already initialized");
        return false;
    }

    try
    {
        if(InitSSL() == false)
        {
            SetLastError(std::string("SSL init error"));
            throw;
        }

        m_initialized = ICommunicationServer::Init();
    }
    catch(...)
    {
        CloseConnections();
        m_initialized = false;
    }

    return m_initialized;
}

bool CommunicationSslServer::Connect(const std::string &address)
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

bool CommunicationSslServer::Run()
{
    try
    {
        memset(m_fds, 0, sizeof(m_fds));
        for (int i = 0; i < (MAX_CLIENTS + 1); i++)
        {
            m_fds[i].fd = (-1);
        }

        // index 0 is always the main socket
        m_fds[0].fd = m_socket;
        m_fds[0].events = POLLIN;
        m_fds[0].revents = 0;

        m_running = true;
        if(pthread_create(&m_readThread, nullptr, &CommunicationSslServer::ReadThreadWrapper, this) != 0)
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

bool CommunicationSslServer::Close(bool wait)
{
    if(m_running == true)
    {
        m_running = false;
        if(wait)
        {
            pthread_join(m_readThread, nullptr);
        }

        CloseConnections();

        close(m_socket);
        if(m_ctx != nullptr)
        {
            SSL_CTX_free(m_ctx);
        }
        EVP_cleanup();
    }

    return true;
}

bool CommunicationSslServer::Write(int connID, ByteArray &data, size_t size)
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
        int fd = m_fds[connID].fd;
        if(fd != (-1))
        {
            while(size > 0)
            {
                size_t s = size > WRITE_MAX_SIZE ? WRITE_MAX_SIZE : size;
                SSL *ssl = m_sslClient[connID];
                int sent = SSL_write(ssl, data.data() + written, s);

                if(sent <= 0)
                {
                    CloseConnection(connID);
                    retval = false;
                }
                else
                {
                    written += sent;
                    size -= sent;
                    retval = true;
                }
            }
        }
    }
    catch(const std::exception &ex)
    {
        retval = false;
    }

    return retval;
}

bool CommunicationSslServer::WaitFor()
{
    pthread_join(m_readThread, nullptr);
    return true;
}

bool CommunicationSslServer::CloseConnection(int connID)
{
    if(ICommunicationServer::CloseConnection(connID))
    {
        SSL *ssl = m_sslClient[connID];
        SSL_shutdown(ssl);
        SSL_free(ssl);
        m_sslClient[connID] = nullptr;

        if(m_closeConnectionCallback != nullptr)
        {
            m_closeConnectionCallback(connID);
        }

        return true;
    }

    return false;
}

bool CommunicationSslServer::InitSSL()
{
    bool retval = false;
    try
    {
        OpenSSL_add_ssl_algorithms();
        SSL_load_error_strings();

        const SSL_METHOD *method = TLS_server_method();

        m_ctx = SSL_CTX_new(method);
        if (!m_ctx)
        {
            SetLastError(ERR_error_string(ERR_get_error(), nullptr));
            throw;
        }

        if (SSL_CTX_use_certificate_file(m_ctx, m_cert.c_str(), SSL_FILETYPE_PEM) <= 0)
        {
            SetLastError(ERR_error_string(ERR_get_error(), nullptr));
            throw;
        }

        if (SSL_CTX_use_PrivateKey_file(m_ctx, m_key.c_str(), SSL_FILETYPE_PEM) <= 0 )
        {
            SetLastError(ERR_error_string(ERR_get_error(), nullptr));
            throw;
        }

        retval = true;
    }
    catch(...)
    {
        retval = false;
    }

    return retval;
}

void *CommunicationSslServer::ReadThreadWrapper(void *ptr)
{
    CommunicationSslServer *instance = static_cast<CommunicationSslServer *>(ptr);
    if(instance != nullptr)
    {
        return instance->ReadThread();
    }

    return nullptr;
}

void *CommunicationSslServer::ReadThread()
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
                                    SSL *ssl;
                                    ssl = SSL_new(m_ctx);
                                    SSL_set_fd(ssl, new_socket);
                                    bool isContinue = true;
                                    bool isError = false;

                                    while(isContinue)
                                    {
                                        int ret = SSL_accept(ssl);
                                        if(ret <= 0)
                                        {
                                            int errorCode = SSL_get_error(ssl, ret);
                                            if (errorCode == SSL_ERROR_WANT_READ)
                                            {
                                                isContinue = true;
                                            }
                                            else
                                            {
                                                isContinue = false;
                                                SSL_shutdown(ssl);
                                                SSL_free(ssl);
                                                close(new_socket);
                                                isError = true;
                                                break;
                                            }
                                        }
                                        else
                                        {
                                            isError = false;
                                            break;
                                        }
                                    }

                                    m_fds[j].fd = new_socket;
                                    m_fds[j].events = POLLIN;
                                    m_sslClient[j] = ssl;

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
                                SSL *ssl = m_sslClient[i];
                                retval = SSL_read(ssl, m_readBuffer, READ_BUFFER_SIZE);
                                if (retval <= 0)
                                {
                                    int errorCode = SSL_get_error(ssl, retval);
                                    if (errorCode == SSL_ERROR_WANT_READ)
                                    {
                                        readMore = false;
                                        isError = false;
                                    }
                                    else
                                    {
                                        readMore = false;
                                        isError = true;
                                        CloseConnection(i);
                                    }
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

#endif
