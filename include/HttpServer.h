#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <CommunicationTcpServer.h>


namespace WebCpp
{

class HttpServer
{
public:
    HttpServer();
    bool Init();
    bool Run();
    bool Close();

protected:
    void OnConnected(int connID);
    void OnDataReady(int connID, const std::vector<char> &data);
    void OnClosed(int connID);

private:
    CommunicationTcpServer m_server;
};

}

#endif // HTTPSERVER_H
