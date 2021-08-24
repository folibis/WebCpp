#ifndef RESPONSEWEBSOCKET_H
#define RESPONSEWEBSOCKET_H

#include "common.h"
#include "ICommunication.h"


namespace WebCpp
{

class ResponseWebSocket
{
public:
    enum class MessageType
    {
        Undefined = 0,
        Text = 1,
        Binary = 2,
        Ping = 9,
        Pong = 10,
    };

    ResponseWebSocket(int connID);
    ResponseWebSocket(const ResponseWebSocket& other) = delete;
    ResponseWebSocket& operator=(const ResponseWebSocket& other) = delete;
    ResponseWebSocket(ResponseWebSocket&& other) = delete;
    ResponseWebSocket& operator=(ResponseWebSocket&& other) = delete;

    bool IsEmpty() const;
    void WriteText(const ByteArray &data);
    void WriteText(const std::string &data);
    void WriteBinary(const ByteArray &data);

    bool Send(ICommunication *communication);



private:
    int m_connID = (-1);
    ByteArray m_data;
    MessageType m_messageType = MessageType::Undefined;
};

}

#endif // RESPONSEWEBSOCKET_H
