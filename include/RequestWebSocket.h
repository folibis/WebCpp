#ifndef REQUESTWEBSOCKET_H
#define REQUESTWEBSOCKET_H

#include "common_webcpp.h"
#include "common_ws.h"
#include "ICommunicationClient.h"


namespace WebCpp
{

class RequestWebSocket
{
public:
    RequestWebSocket();
    bool Parse(const ByteArray &data);
    bool IsFinal() const;
    MessageType GetType() const;
    void SetType(MessageType type);
    size_t GetSize() const;
    const ByteArray& GetData() const;
    void SetData(const ByteArray& data);

    bool Send(ICommunicationClient *communication) const;

private:
    ByteArray m_data;
    bool m_final = false;
    size_t m_size = 0;
    MessageType m_messageType = MessageType::Undefined;
};

}

#endif // REQUESTWEBSOCKET_H
