#ifdef WITH_WEBSOCKET

#include <limits>
#include "common_ws.h"
#include "ResponseWebSocket.h"


using namespace WebCpp;

ResponseWebSocket::ResponseWebSocket(int connID)
{
    m_connID = connID;
}

bool ResponseWebSocket::IsEmpty() const
{
    return (m_messageType == MessageType::Undefined || m_data.empty());
}

void ResponseWebSocket::WriteText(const ByteArray &data)
{
    m_data = data;
    m_messageType = MessageType::Text;
}

void ResponseWebSocket::WriteText(const std::string &data)
{
    WriteText(ByteArray(data.begin(), data.end()));
}

void ResponseWebSocket::WriteBinary(const ByteArray &data)
{
    m_data = data;
    m_messageType = MessageType::Binary;
}

bool ResponseWebSocket::Send(ICommunicationServer *communication) const
{
    try
    {
        ByteArray response;

        WebSocketHeader header;
        header.flags1.FIN = 1;
        header.flags1.opcode = static_cast<uint8_t>(m_messageType);
        header.flags2.Mask = 0;
        size_t dataSize = m_data.size();

        if(dataSize < 126)
        {
            header.flags2.PayloadLen = m_data.size();
        }
        else
        {
            if(dataSize <= std::numeric_limits<uint16_t>::max())
            {
                header.flags2.PayloadLen = 126;
            }
            else
            {
                header.flags2.PayloadLen = 127;
            }
        }

        auto const ptr = reinterpret_cast<char*>(&header);
        ByteArray buffer(ptr, ptr + sizeof(header));
        response.insert(response.end(), buffer.begin(), buffer.end());

        if(dataSize >= 126 && dataSize <= std::numeric_limits<uint16_t>::max())
        {
            WebSocketHeaderLength2 lengthHeader;
            lengthHeader.length.value = dataSize;
            ByteArray buffer(sizeof(lengthHeader));
            buffer[0] = lengthHeader.length.bytes[1];
            buffer[1] = lengthHeader.length.bytes[0];
            response.insert(response.end(), buffer.begin(), buffer.end());
        }
        else if(dataSize > std::numeric_limits<uint16_t>::max())
        {
            WebSocketHeaderLength3 lengthHeader;
            lengthHeader.length.value = dataSize;
            ByteArray buffer(sizeof(lengthHeader));
            for(int i = 0;i < 8;i ++)
            {
                buffer[i] = lengthHeader.length.bytes[7 - i];
            }
            response.insert(response.end(), buffer.begin(), buffer.end());
        }

        response.insert(response.end(), m_data.begin(), m_data.end());

        communication->Write(m_connID, response);

        return true;
    }
    catch(...)
    {
        return false;
    }
}

#endif
