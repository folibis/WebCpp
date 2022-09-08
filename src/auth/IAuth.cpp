#include "IAuth.h"


using namespace WebCpp;

std::map<std::string, IAuth::CreateCallback> IAuth::m_auth;

bool IAuth::Register(const char *name, CreateCallback callback)
{
    std::string n(name);
    if(n.size() > 0)
    {
        StringUtil::ToLower(n);
        n[0] = toupper(n[0]);

        if(m_auth.find(n) == m_auth.end())
        {
            m_auth.insert(std::pair<std::string, IAuth::CreateCallback>(n, callback));
            return true;
        }
    }

    return false;
}

IAuth *IAuth::Create(const std::string &name)
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

const std::map<std::string, IAuth::CreateCallback>& IAuth::Get()
{
    return m_auth;
}
