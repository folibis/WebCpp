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

#ifndef ICOMMUNICATIONCLIENT_H
#define ICOMMUNICATIONCLIENT_H

#include "ICommunication.h"
#include "functional"
#include "common.h"


namespace WebCpp
{

class ICommunicationClient : public ICommunication
{
public:
    virtual bool Write(const ByteArray &data) = 0;
    virtual ByteArray Read(size_t length) = 0;
    virtual bool SetDataReadyCallback(const std::function<void(std::vector<char> &data)> &callback) { m_dataReadyCallback = callback; return true; };
    virtual bool SetCloseConnectionCallback(const std::function<void()> &callback) { m_closeConnectionCallback = callback; return true; };
    bool IsConnected() const { return m_isConnected; }

protected:    
    bool m_isConnected = false;
    std::function<void(std::vector<char> &data)> m_dataReadyCallback = nullptr;
    std::function<void()> m_closeConnectionCallback = nullptr;
};

}

#endif // ICOMMUNICATIONCLIENT_H