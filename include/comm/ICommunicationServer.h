#ifndef ICOMMUNICATION_SERVER_H
#define ICOMMUNICATION_SERVER_H

#include "ICommunication.h"
#include "functional"


namespace WebCpp
{

class ICommunicationServer : public ICommunication
{
public:
    virtual bool CloseClient(int connID) = 0;

    virtual bool SetNewConnectionCallback(const std::function<void(int, const std::string&)> &callback) { m_newConnectionCallback = callback; return true; };
    virtual bool SetDataReadyCallback(const std::function<void(int, std::vector<char> &data)> &callback) { m_dataReadyCallback = callback; return true; };
    virtual bool SetCloseConnectionCallback(const std::function<void(int)> &callback) { m_closeConnectionCallback = callback; return true; };

protected:
    std::function<void(int, const std::string&)> m_newConnectionCallback = nullptr;
    std::function<void(int, std::vector<char> &data)> m_dataReadyCallback = nullptr;
    std::function<void(int)> m_closeConnectionCallback = nullptr;

};

}

#endif // ICOMMUNICATION_SERVER_H
