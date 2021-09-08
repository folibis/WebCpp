#ifndef IHTTP_H
#define IHTTP_H

#include <string>


namespace WebCpp {

class Http
{
public:
    enum class Protocol
    {
        Undefined = 0,
        HTTP,
        HTTPS,
        WS,
        WSS,
    };

    enum class Method
    {
        Undefined = 0,
        OPTIONS,
        GET,
        HEAD,
        POST,
        PUT,
        DELETE,
        TRACE,
        CONNECT,
        WEBSOCKET,
    };

    static std::string Protocol2String(Protocol protocol);
    static Protocol String2Protocol(const std::string &str);
    static Method String2Method(const std::string &str);
    static std::string Method2String(Method method);
};

}

#endif // IHTTP_H
