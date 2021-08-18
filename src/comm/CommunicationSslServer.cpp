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
#include "common.h"
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
    bool retval = false;
    try
    {
        retval = InitSSL();
        if(retval == false)
        {
            SetLastError(std::string("SSL init error"));
            throw std::runtime_error(GetLastError());
        }

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
        std::cout << "CommunicationSslServer::Init error: " << ex.what() << std::endl;
        CloseConnections();
        retval = false;
    }
    catch(...)
    {
        std::cout << "CommunicationSslServer::Init unexpected error" << std::endl;
        CloseConnections();
        retval = false;
    }

    return retval;
}

bool CommunicationSslServer::Connect(const std::string &address)
{
    try
    {
        if(!address.empty())
        {
            auto addr_arr = StringUtil::Split(address, ':');
            if(addr_arr.size() == 2)
            {
                m_address = addr_arr[0];
                int port;
                if(StringUtil::String2int(addr_arr[1], port))
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
        m_fds[0].revents = 0;

        m_running = true;
        if(pthread_create(&m_readThread, nullptr, &CommunicationSslServer::ReadThreadWrapper, this) != 0)
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

        SSL_CTX_free(m_ctx);
        EVP_cleanup();
    }

    return true;
}

bool CommunicationSslServer::Write(int connID, const std::vector<char> &data)
{
    return Write(connID, data, data.size());
}

bool CommunicationSslServer::Write(int connID, const std::vector<char> &data, size_t size)
{
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
                    CloseClient(connID);
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
        std::cout << "CommunicationTcpServer::Write: " << ex.what() << std::endl;
        retval = false;
    }

    return retval;
}

bool CommunicationSslServer::WaitFor()
{
    pthread_join(m_readThread, nullptr);
    return true;
}

bool CommunicationSslServer::CloseClient(int connID)
{
    if(m_fds[connID].fd != (-1))
    {
        close(m_fds[connID].fd);
        m_fds[connID].fd = (-1);
        m_fds[connID].events = 0;
        m_fds[connID].revents = 0;

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
        SSL_load_error_strings();
        OpenSSL_add_ssl_algorithms();

        const SSL_METHOD *method;

        method = SSLv23_server_method();

        m_ctx = SSL_CTX_new(method);
        if (!m_ctx)
        {
            throw std::runtime_error("Unable to create SSL context");
        }

        SSL_CTX_set_ecdh_auto(ctx, 1);

        if (SSL_CTX_use_certificate_file(m_ctx, m_cert.c_str(), SSL_FILETYPE_PEM) <= 0)
        {
            throw std::runtime_error("Unable to init certificate file");
        }

        if (SSL_CTX_use_PrivateKey_file(m_ctx, m_key.c_str(), SSL_FILETYPE_PEM) <= 0 )
        {
            throw std::runtime_error("Unable to init private key file");
        }

        retval = true;
    }
    catch(...)
    {
        retval = false;
    }

    return retval;
}

void CommunicationSslServer::CloseConnections()
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
                                        CloseClient(i);
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
