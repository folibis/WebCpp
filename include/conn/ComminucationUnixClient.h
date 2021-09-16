#ifndef COMMINUCATIONUNIXCLIENT_H
#define COMMINUCATIONUNIXCLIENT_H

#include "poll.h"
#include "pthread.h"
#include "ICommunicationClient.h"

#define BUFFER_SIZE 1024


namespace WebCpp
{

class ComminucationUnixClient: public ICommunicationClient
{
public:
    ComminucationUnixClient(const std::string &path);
    bool Init() override;
    bool Run() override;
    bool Close(bool wait = true) override;
    bool WaitFor() override;
    bool Connect(const std::string &address = "") override;
    bool Write(const ByteArray &data) override;
    ByteArray Read(size_t length = 0) override;

protected:
    static void* ReadThreadWrapper(void *ptr);
    void* ReadThread();

private:
    pollfd m_poll;
    bool m_running = false;
    pthread_t m_thread;
    char m_readBuffer[BUFFER_SIZE];
};

}

#endif // COMMINUCATIONUNIXCLIENT_H
