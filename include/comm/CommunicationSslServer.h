#ifdef WITH_OPENSSL
#ifndef COMMUNICATION_SSL_SERVER_H
#define COMMUNICATION_SSL_SERVER_H

#include <poll.h>
#include <pthread.h>
#include <functional>
#include <vector>
#include "ICommunicationServer.h"
#include <poll.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define MAX_CLIENTS 10
#define READ_BUFFER_SIZE 1024


namespace WebCpp
{

class CommunicationSslServer: public ICommunicationServer
{
public:
    CommunicationSslServer(const std::string &cert, const std::string &key) noexcept;

    bool Init() override;
    bool Connect(const std::string &address = "") override;
    bool Close(bool wait = false) override;
    bool Write(int connID, const std::vector<char> &data) override;
    bool Write(int connID, const std::vector<char> &data, size_t size) override;
    bool Run() override;
    bool WaitFor() override;
    bool CloseClient(int connID) override;

protected:
    bool InitSSL();
    void CloseConnections();
    static void* ReadThreadWrapper(void *ptr);
    void* ReadThread();

private:
    struct pollfd m_fds[MAX_CLIENTS + 1];
    bool m_running = false;
    pthread_t m_readThread;
    pthread_mutex_t m_writeMutex = PTHREAD_MUTEX_INITIALIZER;

    SSL_CTX *m_ctx = nullptr;
    SSL *m_sslClient[MAX_CLIENTS + 1] = {};
    const std::string m_cert;
    const std::string m_key;
    char m_readBuffer[READ_BUFFER_SIZE];
};

}

#endif // COMMUNICATION_SSL_SERVER_H
#endif // WITH_OPENSSL
