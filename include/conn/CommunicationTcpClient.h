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

#ifndef COMMUNICATIONTCPCLIENT_H
#define COMMUNICATIONTCPCLIENT_H

#include <poll.h>
#include "ICommunicationClient.h"

#define BUFFER_SIZE 1024


namespace WebCpp {

class CommunicationTcpClient: public ICommunicationClient
{
public:
    CommunicationTcpClient();

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

private:
    bool m_initialized = false;
    pollfd m_poll;
    bool m_running = false;
    pthread_t m_thread;
    char m_readBuffer[BUFFER_SIZE];
};

}

#endif // COMMUNICATIONTCPCLIENT_H
