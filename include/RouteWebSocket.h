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

#ifndef WEBCPP_ROUTEWEBSOCKET_H
#define WEBCPP_ROUTEWEBSOCKET_H

#include "Route.h"
#include "Request.h"
#include "ResponseWebSocket.h"
#include "HttpHeader.h"


namespace WebCpp
{

class RouteWebSocket: public Route
{
public:    
    using RouteFuncRequest = std::function<bool(const Request&request, Response &response)>;
    using RouteFuncMessage = std::function<bool(const Request& request, ResponseWebSocket &response, const ByteArray& data)>;

    RouteWebSocket(const std::string &path);

    bool SetFunctionRequest(const RouteFuncRequest& f);
    const RouteFuncRequest& GetFunctionRequest() const;

    bool SetFunctionMessage(const RouteFuncMessage& f);
    const RouteFuncMessage& GetFunctionMessage() const;

private:
    RouteFuncRequest m_funcRequest;
    RouteFuncMessage m_funcMessage;
};

}

#endif // WEBCPP_ROUTEWEBSOCKET_H

#endif
