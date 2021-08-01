#ifndef HTTPCONFIG_H
#define HTTPCONFIG_H

#include <string>
#include "common.h"


#define PROPERTY(TYPE,NAME,DEFAULT) \
    public: \
    TYPE Get##NAME() const { return m_##NAME; } \
    void Set##NAME(const TYPE& value) { m_##NAME = value; } \
    private: \
    TYPE m_##NAME = DEFAULT;

namespace WebCpp
{

class HttpConfig
{
public:
    HttpConfig();
    void Init();

    PROPERTY(std::string, ServerName, WEBCPP_CANONICAL_NAME)
    PROPERTY(std::string, Root, "public")
    PROPERTY(int, KeepAliveTimeout, 5)
};

}

#endif // HTTPCONFIG_H
