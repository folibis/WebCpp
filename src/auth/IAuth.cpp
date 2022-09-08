#include "IAuth.h"
#include "StringUtil.h"


using namespace WebCpp;

std::string IAuth::GetUser() const
{
    return m_user;
}

bool IAuth::SetUser(const std::string &user)
{
    m_user = user;
    m_preferred = true;

    return true;
}

std::string IAuth::GetPassword() const
{
    return m_password;
}

bool IAuth::SetPassword(const std::string &password)
{
    m_password = password;
    m_preferred = true;

    return true;
}

void IAuth::SetPreferred(bool value)
{
    m_preferred = value;
}

bool IAuth::IsPreferred() const
{
    return m_preferred;
}
