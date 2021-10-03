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
#ifndef WEBCPP_COMMUNICATIONSSLCLIENT_H
#define WEBCPP_COMMUNICATIONSSLCLIENT_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "ICommunicationClient.h"

#define BUFFER_SIZE 1024


namespace WebCpp
{

class CommunicationSslClient: public ICommunicationClient
{
public:
    CommunicationSslClient(const std::string &cert, const std::string &key) noexcept;
    virtual ~CommunicationSslClient();
    bool Init() override;
    bool Run() override;
    bool Close(bool wait = true) override;
    bool WaitFor() override;

    bool Connect(const std::string &address = "") override;
    bool Write(const ByteArray &data) override;
    ByteArray Read(size_t length) override;

protected:
    static void* ReadThreadWrapper(void *ptr);
    void* ReadThread();    
    bool InitSSL();

private:
    struct hostent *hostinfo;
    struct sockaddr_in dest_addr = {};
    const std::string m_cert;
    const std::string m_key;
    pollfd m_poll;
    bool m_running = false;
    pthread_t m_thread;
    pthread_mutex_t m_writeMutex = PTHREAD_MUTEX_INITIALIZER;
    SSL_CTX *m_ctx = nullptr;
    SSL *m_ssl = nullptr;
    char m_readBuffer[BUFFER_SIZE];
};

}

#endif // WEBCPP_COMMUNICATIONSSLCLIENT_H

#endif // WITH_OPENSSL
