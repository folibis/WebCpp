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

#ifndef WEBCPP_ICOMMUNICATION_H
#define WEBCPP_ICOMMUNICATION_H

#include <string>
#include <vector>
#include "Socket.h"
#include "IErrorable.h"
#include "IRunnable.h"

#define DEFAULT_ADDRESS "*"
#define DEFAULT_PORT 80


namespace WebCpp
{

class ICommunication : public IErrorable, public IRunnable
{
public:
    virtual bool Connect(const std::string &address = "") = 0;
    void SetPort(int port) { m_port = port; }
    int GetPort() const { return m_port; }
    void SetAddress(const std::string &address) { m_address = address; };
    std::string GetAddress() const { return m_address; }
    bool IsInitialized() const { return m_initialized; }
    bool IsConnected() const { return m_connected; }

protected:
    bool m_initialized = false;
    bool m_connected = false;
    int m_port = DEFAULT_PORT;
    std::string m_address = DEFAULT_ADDRESS;
};

}

#endif // WEBCPP_ICOMMUNICATION_H
