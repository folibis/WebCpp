#include "AuthFactory.h"
#include "StringUtil.h"


using namespace WebCpp;

AuthFactory &AuthFactory::Instance()
{
    static AuthFactory instance;
    return instance;
}

bool AuthFactory::Register(const char *name, CreateCallback callback)
{
    std::string n(name);
    if(n.size() > 0)
    {
        StringUtil::ToLower(n);
        n[0] = toupper(n[0]);

        if(m_auth.find(n) == m_auth.end())
        {
            auto pair = std::pair<std::string, CreateCallback>(n, callback);
            m_auth.insert(pair);
            return true;
        }
    }

    return false;
}

IAuth *AuthFactory::Create(const std::string &name)
{
    std::string n = name;
    StringUtil::ToLower(n);
    n[0] = toupper(n[0]);

    auto it = m_auth.find(n);
    if(it == m_auth.end())
    {
        return nullptr;
    }

    return it->second();
}

IAuth *AuthFactory::CreateFromString(const std::string &data)
{
    size_t pos = data.find(' ');
    std::string scheme;
    if(pos != std::string::npos)
    {
        scheme = data.substr(0, pos);
    }
    else
    {
        scheme = data;
    }

    StringUtil::Trim(scheme);
    auto ptr = Create(scheme);
    return ptr;
}

const std::map<std::string, AuthFactory::CreateCallback>& AuthFactory::Get()
{
    return m_auth;
}
