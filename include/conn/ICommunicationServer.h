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

#ifndef WEBCPP_ICOMMUNICATION_SERVER_H
#define WEBCPP_ICOMMUNICATION_SERVER_H

#include "ICommunication.h"
#include "functional"
#include "common_webcpp.h"


namespace WebCpp
{

class ICommunicationServer : public ICommunication
{
public:
    virtual bool CloseClient(int connID) = 0;
    virtual bool Write(int connID, ByteArray &data) = 0;
    virtual bool Write(int connID, ByteArray &data, size_t size) = 0;

    virtual bool SetNewConnectionCallback(const std::function<void(int, const std::string&)> &callback) { m_newConnectionCallback = callback; return true; };
    virtual bool SetDataReadyCallback(const std::function<void(int, ByteArray &data)> &callback) { m_dataReadyCallback = callback; return true; };
    virtual bool SetCloseConnectionCallback(const std::function<void(int)> &callback) { m_closeConnectionCallback = callback; return true; };

protected:
    std::function<void(int, const std::string&)> m_newConnectionCallback = nullptr;
    std::function<void(int, ByteArray &data)> m_dataReadyCallback = nullptr;
    std::function<void(int)> m_closeConnectionCallback = nullptr;

};

}

#endif // WEBCPP_ICOMMUNICATION_SERVER_H
