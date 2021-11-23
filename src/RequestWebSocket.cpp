#include <cstring>
#include <limits>
#include "StringUtil.h"
#include "RequestWebSocket.h"


using namespace WebCpp;

RequestWebSocket::RequestWebSocket()
{

}

bool RequestWebSocket::Parse(const ByteArray &data)
{
    bool retval = false;

    WebSocketHeader header;
    size_t dataSize = data.size();
    size_t headerSize = sizeof(WebSocketHeader);

    if(dataSize >= headerSize)
    {
        std::memcpy(&header, data.data(), headerSize);
        m_messageType = static_cast<MessageType>(header.flags1.opcode);

        uint64_t payloadSize = 0;
        size_t sizeHeaderSize = 0;
        switch(header.flags2.PayloadLen)
        {
            case 126:
                sizeHeaderSize = sizeof(WebSocketHeaderLength2);
                if(dataSize >= headerSize + sizeHeaderSize)
                {
                    WebSocketHeaderLength2 length;
                    const uint8_t* ptr = data.data() + headerSize;
                    length.length.bytes[0] = *(ptr + 1);
                    length.length.bytes[1] = *ptr;
                    payloadSize = length.length.value;
                }
                else
                {
                    return false;
                }
                break;
            case 127:
                sizeHeaderSize = sizeof(WebSocketHeaderLength3);
                if(dataSize >= headerSize + sizeHeaderSize)
                {
                    WebSocketHeaderLength3 length;
                    const uint8_t* ptr = data.data() + headerSize;
                    for(int i = 0;i < 8;i ++)
                    {
                        length.length.bytes[i] = *(ptr + 7 - i);
                    }
                    payloadSize = length.length.value;
                }
                else
                {
                    return false;
                }
                break;
            default:
                payloadSize = header.flags2.PayloadLen;
                break;
        }

        if(payloadSize > 0)
        {
            size_t maskHeaderSize = 0;
            WebSocketHeaderMask mask;
            size_t headers_size = headerSize + sizeHeaderSize;
            if(header.flags2.Mask == 1)
            {
                maskHeaderSize = sizeof(WebSocketHeaderMask);
                headers_size += maskHeaderSize;
                if(dataSize >= headers_size)
                {
                    std::memcpy(&mask, data.data() + headerSize + sizeHeaderSize, maskHeaderSize);
                }
                else
                {
                    return false;
                }
            }

            size_t messageFullSize = headers_size + payloadSize;
            if(dataSize >= messageFullSize)
            {
                // according to rfc6455#section-5.3 server must ignore unmasked data
                // but anyway we support such unstandard clients
                if(header.flags2.Mask == 1)
                {
                    ByteArray encoded(payloadSize);
                    for(size_t i = 0;i < payloadSize;i ++)
                    {
                        encoded[i] = data[headers_size + i] ^ mask.bytes[i % 4];
                    }
                    m_data.insert(m_data.end(), encoded.begin(), encoded.end());
                }
                else
                {
                    m_data.insert(m_data.end(),
                                  data.begin() + headers_size,
                                  data.begin() + messageFullSize
                                  );
                }

                if(header.flags1.FIN == 1)
                {
                    m_final = true;
                    retval = true;
                }

                m_size = messageFullSize;
            }
        }
    }

    return retval;
}

bool RequestWebSocket::IsFinal() const
{
    return m_final;
}

MessageType RequestWebSocket::GetType() const
{
    return m_messageType;
}

void RequestWebSocket::SetType(MessageType type)
{
    m_messageType = type;
}

size_t RequestWebSocket::GetSize() const
{
    return m_size;
}

const ByteArray &RequestWebSocket::GetData() const
{
    return m_data;
}

void RequestWebSocket::SetData(const ByteArray &data)
{
    m_data = data;
}

bool RequestWebSocket::Send(ICommunicationClient *communication) const
{
    try
    {
        ByteArray response;

        WebSocketHeader header = {};
        header.flags1.FIN = 1;
        header.flags1.opcode = static_cast<uint8_t>(m_messageType);
        header.flags2.Mask = 1;
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
            WebSocketHeaderLength2 lengthHeader = {};
            lengthHeader.length.value = dataSize;
            ByteArray buffer(sizeof(lengthHeader));
            buffer[0] = lengthHeader.length.bytes[1];
            buffer[1] = lengthHeader.length.bytes[0];
            response.insert(response.end(), buffer.begin(), buffer.end());
        }
        else if(dataSize > std::numeric_limits<uint16_t>::max())
        {
            WebSocketHeaderLength3 lengthHeader = {};
            lengthHeader.length.value = dataSize;
            ByteArray buffer(sizeof(lengthHeader));
            for(int i = 0;i < 8;i ++)
            {
                buffer[i] = lengthHeader.length.bytes[7 - i];
            }
            response.insert(response.end(), buffer.begin(), buffer.end());
        }

        StringUtil::RandInit();
        WebSocketHeaderMask mask;
        ByteArray maskBuffer;
        for(auto i = 0;i < 4;i ++)
        {
            mask.bytes[i] = StringUtil::GetRand(0, 0xFF);
            maskBuffer.push_back(mask.bytes[i]);
        }
        response.insert(response.end(), maskBuffer.begin(), maskBuffer.end());

        ByteArray encoded(m_data.size());
        for(auto i = 0;i < m_data.size();i ++)
        {
            encoded[i] = m_data[i] ^ mask.bytes[i % 4];
        }

        response.insert(response.end(), encoded.begin(), encoded.end());

        communication->Write(response);

        return true;
    }
    catch(...)
    {
        return false;
    }
}
