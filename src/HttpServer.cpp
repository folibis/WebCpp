#include <iostream>
#include "common.h"
#include "HttpServer.h"


using namespace WebCpp;

HttpServer::HttpServer()
{

}

bool WebCpp::HttpServer::Init()
{
    m_server.SetPort(8080);

    if(!m_server.Init())
    {
        return false;
    }

    auto f1 = std::bind(&HttpServer::OnConnected, this, std::placeholders::_1);
    m_server.SetNewConnectionCallback(f1);
    auto f2 = std::bind(&HttpServer::OnDataReady, this, std::placeholders::_1, std::placeholders::_2);
    m_server.SetDataReadyCallback(f2);
    auto f3 = std::bind(&HttpServer::OnClosed, this, std::placeholders::_1);
    m_server.SetCloseConnectionCallback(f3);

    return true;
}

bool HttpServer::Run()
{
    if(!m_server.Connect())
    {
        return false;
    }

    m_server.WaitFor();
    return true;
}

bool HttpServer::Close()
{
    m_server.Close();
    return true;
}

void HttpServer::OnConnected(int connID)
{
    std::cout << "client connected: #" << connID << std:: endl;
}

void HttpServer::OnDataReady(int connID, const byte_array &data)
{
    //std::cout << data << std::endl;
}

void HttpServer::OnClosed(int connID)
{
    std::cout << "client disconnected: #" << connID << std:: endl;
}
