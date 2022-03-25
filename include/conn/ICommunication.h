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
#include "IErrorable.h"
#include "IRunnable.h"


namespace WebCpp
{

class ICommunication : public IErrorable, public IRunnable
{
public:
    virtual bool Connect(const std::string &host = "", int port = 0) = 0;
    virtual void SetPort(int) { }
    virtual int GetPort() const { return 0; }
    virtual void SetHost(const std::string &) { };
    virtual std::string GetHost() const { return std::string(); }

    bool IsInitialized() const { return m_initialized; }
    bool IsConnected() const { return m_connected; }

protected:
    bool m_initialized = false;
    bool m_connected = false;
};

}

#endif // WEBCPP_ICOMMUNICATION_H
