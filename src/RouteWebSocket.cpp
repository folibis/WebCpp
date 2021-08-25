#include "RouteWebSocket.h"


using namespace WebCpp;

RouteWebSocket::RouteWebSocket(const std::string &path) :
    Route(path, HttpHeader::Method::WEBSOCKET)
{

}

bool RouteWebSocket::SetFunctionRequest(const RouteWebSocket::RouteFuncRequest &f)
{
    m_funcRequest = f;
    return true;
}

const RouteWebSocket::RouteFuncRequest &RouteWebSocket::GetFunctionRequest() const
{
    return m_funcRequest;
}

bool RouteWebSocket::SetFunctionMessage(const RouteWebSocket::RouteFuncMessage &f)
{
    m_funcMessage = f;
    return true;
}

const RouteWebSocket::RouteFuncMessage &RouteWebSocket::GetFunctionMessage() const
{
    return m_funcMessage;
}
