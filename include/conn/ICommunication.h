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

#ifndef ICOMMUNICATION_H
#define ICOMMUNICATION_H

#include <string>
#include <vector>
#include "IError.h"
#include "IRunnable.h"

#define DEFAULT_ADDRESS "0.0.0.0"
#define DEFAULT_PORT 80


namespace WebCpp
{

class ICommunication : public IError, public IRunnable
{
public:
    enum class CommunicationProtocol
    {
        Undefined = 0,
        TCP,
        UnixDoamin,
    };

    enum class ComminicationType
    {
        Undefined = 0,
        Server,
        Client
    };

    virtual bool Connect(const std::string &address = "") = 0;

    CommunicationProtocol GetProtocol() const { return m_protocol; }
    ComminicationType GetType() const { return m_type; }

    void SetPort(int port) { m_port = port; }
    int GetPort() const { return m_port; }
    void SetAddress(const std::string &address) { m_address = address; };
    std::string GetAddress() const { return m_address; }

protected:
    int m_socket = (-1);
    CommunicationProtocol m_protocol = CommunicationProtocol::Undefined;
    ComminicationType m_type = ComminicationType::Undefined;
    int m_port = DEFAULT_PORT;
    std::string m_address = DEFAULT_ADDRESS;

    void ParseAddress(const std::string &address);
};

}

#endif // ICOMMUNICATION_H
