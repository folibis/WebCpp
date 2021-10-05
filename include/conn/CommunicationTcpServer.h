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

#ifndef WEBCPP_COMMUNICATION_TCP_SERVER_H
#define WEBCPP_COMMUNICATION_TCP_SERVER_H

#include <poll.h>
#include <pthread.h>
#include <functional>
#include <vector>
#include "ICommunicationServer.h"

#define READ_BUFFER_SIZE 1024


namespace WebCpp
{

class CommunicationTcpServer : public ICommunicationServer
{
public:
    CommunicationTcpServer() noexcept;    
    virtual ~CommunicationTcpServer();

    bool Init() override final;
    bool Connect(const std::string &address = "") override;
    bool Run() override;
    bool WaitFor() override;
    bool Close(bool wait = true) override;
    bool CloseConnection(int connID) override;

protected:
    static void* ReadThreadWrapper(void *ptr);
    void* ReadThread();

private:
    bool m_running = false;
    pthread_t m_readThread;

    char m_readBuffer[READ_BUFFER_SIZE];
};

}

#endif // WEBCPP_COMMUNICATION_TCP_SERVER_H
