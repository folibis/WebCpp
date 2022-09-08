#include "AuthProvider.h"
#include "AuthFactory.h"


using namespace WebCpp;

AuthProvider::AuthProvider(Type type):
    m_type(type)
{

}

bool AuthProvider::Init(const std::string &str)
{
    return ((m_type == Type::Client) ? InitClient(str) : InitServer(str));
}

bool AuthProvider::Parse(const std::string &str)
{
    return ((m_type == Type::Client) ? ParseFromServer(str) : ParseFromClient(str));
}

bool AuthProvider::Check(const std::string &str)
{
    return ((m_type == Type::Client) ? CheckFromServer(str) : CheckFromClient(str));
}

bool AuthProvider::IsInitialized() const
{
    return m_initialized;
}

IAuth *AuthProvider::GetPreferred() const
{
    for(auto &authMethod: m_auth)
    {
        if(authMethod->IsPreferred())
        {
            return authMethod.get();
        }
    }

    return nullptr;
}

const std::vector<std::unique_ptr<IAuth>> &AuthProvider::Get() const
{
    return m_auth;
}

void AuthProvider::Clear()
{
    m_auth.clear();
}

bool AuthProvider::InitServer(const std::string &str)
{
    m_initialized = false;
    try
    {
        auto &list = AuthFactory::Instance().Get();
        for(auto &pair: list)
        {
            auto ptr = pair.second();
            if(ptr != nullptr)
            {
                if(ptr->Init() == true)
                {
                    m_auth.push_back(std::unique_ptr<IAuth>(ptr));
                    m_initialized = true;
                }
            }
        }
    }
    catch(...)
    {
        m_auth.clear();
    }

    return m_initialized;
}

bool AuthProvider::InitClient(const std::string &)
{
    m_initialized = true;
    return m_initialized;
}

bool AuthProvider::ParseFromServer(const std::string &str)
{
    bool retval = false;
    auto ptr = AuthFactory::Instance().CreateFromString(str);
    if(ptr != nullptr)
    {
        if(ptr->ParseFromServer(str) == true)
        {
            m_auth.push_back(std::unique_ptr<IAuth>(ptr));
            retval = true;
        }
    }

    return retval;
}

bool AuthProvider::ParseFromClient(const std::string &str)
{
    return false;
}

bool AuthProvider::CheckFromServer(const std::string &str)
{
    bool retval = false;
    ClearPreferred();
    for(auto &authMethod: m_auth)
    {
        if(authMethod->ParseFromServer(str) == true)
        {
            authMethod->SetPreferred(true);
            retval = true;
        }
    }

    return retval;
}

bool AuthProvider::CheckFromClient(const std::string &str)
{
    bool retval = false;
    ClearPreferred();
    for(auto &authMethod: m_auth)
    {
        if(authMethod->ParseFromClient(str) == true)
        {
            authMethod->SetPreferred(true);
            retval = true;
        }
    }

    return retval;
}

void AuthProvider::ClearPreferred()
{
    for(auto &authMethod: m_auth)
    {
        authMethod->SetPreferred(false);
    }
}

