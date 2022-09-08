#ifndef AUTHFACTORY_H
#define AUTHFACTORY_H

#include <vector>
#include <map>
#include <string>
#include "IAuth.h"


namespace WebCpp {

class AuthFactory
{
public:
    static AuthFactory& Instance();
    using CreateCallback = IAuth *(*)();
    bool Register(const char *name, CreateCallback callback);
    IAuth* Create(const std::string &name);
    IAuth* CreateFromString(const std::string &data);
    const std::map<std::string, CreateCallback> &Get();

private:
    std::map<std::string, CreateCallback> m_auth;
};

}

#define IAUTH_IMPL(T) IAuth *T::CreateMethod() { return new T(); }

#define REGISTER_AUTH(N, T) bool T::m_registered = AuthFactory::Instance().Register(N, &T::CreateMethod);\
IAUTH_IMPL(T)

#define IAUTH_DECL protected: \
static IAuth *CreateMethod(); \
static bool m_registered; \

#endif // AUTHFACTORY_H
