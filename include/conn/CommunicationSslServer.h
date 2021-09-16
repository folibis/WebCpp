/*
*
* Copyright (c) 2021 ruslan@muhlinin.com
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
*/

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
    bool Write(int connID, ByteArray &data) override;
    bool Write(int connID, ByteArray &data, size_t size) override;
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
