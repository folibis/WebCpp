#ifdef WITH_WEBSOCKET

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

#ifndef WEBCPP_RESPONSEWEBSOCKET_H
#define WEBCPP_RESPONSEWEBSOCKET_H

#include "common_webcpp.h"
#include "ICommunicationServer.h"
#include "common_ws.h"

namespace WebCpp
{

class ResponseWebSocket
{
public:

    ResponseWebSocket(int connID);
    ResponseWebSocket(const ResponseWebSocket& other) = delete;
    ResponseWebSocket& operator=(const ResponseWebSocket& other) = delete;
    ResponseWebSocket(ResponseWebSocket&& other) = delete;
    ResponseWebSocket& operator=(ResponseWebSocket&& other) = delete;

    bool IsEmpty() const;
    void WriteText(const ByteArray &data);
    void WriteText(const std::string &data);
    void WriteBinary(const ByteArray &data);
    void WriteBinary(const std::string &data);

    MessageType GetMessageType() const;
    void SetMessageType(MessageType type);

    const ByteArray& GetData() const;

    bool Send(ICommunicationServer *communication) const;
    bool Parse(const ByteArray &data);

private:
    int m_connID = (-1);
    ByteArray m_data;
    MessageType m_messageType = MessageType::Undefined;
};

}

#endif // WEBCPP_RESPONSEWEBSOCKET_H

#endif
