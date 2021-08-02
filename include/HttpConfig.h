#ifndef HTTPCONFIG_H
#define HTTPCONFIG_H

#include <string>
#include <vector>
#include "common.h"

#define PROPERTY(TYPE,NAME,DEFAULT) \
    public: \
    TYPE Get##NAME() const { return m_##NAME; } \
    void Set##NAME(const TYPE& value) { m_##NAME = value; } \
    private: \
    TYPE m_##NAME = DEFAULT; \


namespace WebCpp
{

class HttpConfig
{
public:
    HttpConfig();
    void Init();
    bool Load();

    PROPERTY(std::string, ServerName, WEBCPP_CANONICAL_NAME)
    PROPERTY(std::string, Root, "public")
    PROPERTY(std::string, IndexFile, "index.html")
    PROPERTY(int, ServerPort, 8080)
    PROPERTY(std::string, Protocol, "HTTP")
    PROPERTY(int, KeepAliveTimeout, 2000)
    PROPERTY(std::string, SslSertificate, "cert.pem")
    PROPERTY(std::string, SslKey, "key.pem")
};

}

#endif // HTTPCONFIG_H
