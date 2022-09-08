#ifndef IAUTH_H
#define IAUTH_H

#include <vector>
#include "Request.h"


namespace WebCpp {

class IAuth
{
public:
    virtual bool Init() = 0;
    virtual bool Parse(const Request &request) = 0;
    virtual std::string GetChallenge() const = 0;

    using CreateCallback = IAuth *(*)();
    static bool Register(const char *name, CreateCallback callback);
    static IAuth* Create(const std::string &name);
    static const std::map<std::string, CreateCallback>& Get();

private:
    static std::map<std::string, CreateCallback> m_auth;
};

}

#define IAUTH_IMPL(T) IAuth *T::CreateMethod() { return new T(); }

#define REGISTER_AUTH(N, T) bool T::m_registered = IAuth::Register(N, &T::CreateMethod);\
IAUTH_IMPL(T)

#define IAUTH_DECL protected: \
static IAuth *CreateMethod(); \
static bool m_registered; \

#endif // IAUTH_H
