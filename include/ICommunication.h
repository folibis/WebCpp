/**************************************************************************
/// brief The ICommunication class
/// author ruslan@muhlinin.com
/// date July 25, 2021
/// details The network communication interface
**************************************************************************/

#ifndef ICOMMUNICATION_H
#define ICOMMUNICATION_H

#include <string>
#include "IError.h"

#define DEFAULT_ADDRESS "0.0.0.0"
#define DEFAULT_PORT 80


namespace WebCpp
{

class ICommunication : public IError
{
public:
    enum class CommunicationProtocol
    {
        Undefined = 0,        
        TCP
    };

    enum class ComminicationType
    {
        Undefined = 0,
        Server,
        Client
    };

    virtual bool Init() = 0;
    virtual bool Connect(const std::string &address = "") = 0;
    virtual bool Close(bool wait) = 0;

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
};

}

#endif // ICOMMUNICATION_H
