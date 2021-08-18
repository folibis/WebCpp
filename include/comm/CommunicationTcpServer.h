#ifndef COMMUNICATION_TCP_SERVER_H
#define COMMUNICATION_TCP_SERVER_H

#include <poll.h>
#include <pthread.h>
#include <functional>
#include <vector>
#include "ICommunicationServer.h"

#define MAX_CLIENTS 10
#define READ_BUFFER_SIZE 1024


namespace WebCpp
{

class CommunicationTcpServer : public ICommunicationServer
{
public:
    CommunicationTcpServer() noexcept;    

    bool Init() override;
    bool Connect(const std::string &address = "") override;
    bool Close(bool wait = false) override;
    bool Write(int connID, const std::vector<char> &data) override;
    bool Write(int connID, const std::vector<char> &data, size_t size) override;

    bool WaitFor() override;
    bool CloseClient(int connID) override;

protected:
    void CloseConnections();
    static void* ReadThreadWrapper(void *ptr);
    void* ReadThread();

private:
    struct pollfd m_fds[MAX_CLIENTS + 1];
    bool m_running = false;
    pthread_t m_readThread;
    pthread_mutex_t m_writeMutex = PTHREAD_MUTEX_INITIALIZER;

    char m_readBuffer[READ_BUFFER_SIZE];
};

}

#endif // COMMUNICATION_TCP_SERVER_H
