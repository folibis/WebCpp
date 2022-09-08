#include "AuthBasic.h"
#include "StringUtil.h"
#include "Data.h"
#include "Request.h"


using namespace WebCpp;
REGISTER_AUTH("Basic", AuthBasic)

AuthBasic::AuthBasic()
{

}

bool AuthBasic::Init()
{
    return true;
}

bool AuthBasic::ParseFromClient(const std::string &token)
{
    auto list = StringUtil::Split(token, ' ');
    if(list.size() >= 2)
    {
        std::string method = StringUtil::Trim(list[0]);
        if(method == "Basic")
        {
            std::string data = StringUtil::Trim(list[1]);
            std::string decoded = Data::Base64Decode(data);
            auto credentials = StringUtil::Split(decoded, ':');
            if(credentials.size() >= 2)
            {
                m_user = credentials[0];
                m_password = credentials[1];
                return true;
            }
        }
    }

    return false;
}

bool AuthBasic::ParseFromServer(const std::string &token)
{
    auto list = StringUtil::Split(token, ' ');
    if(list.size() >= 2)
    {
        std::string method = StringUtil::Trim(list[0]);
        if(method == "Basic")
        {
            auto tokens = StringUtil::ParseParamString(token, 5);
            auto it = tokens.find("realm");
            if(it != tokens.end())
            {
                m_realm = it->second;
            }
            it = tokens.find("charset");
            if(it != tokens.end())
            {
                m_charset = it->second;
            }

            return true;
        }
    }

    return false;
}

std::string AuthBasic::GetChallenge() const
{
    return GetSchemeName() + " realm=\"" + m_realm + "\"";
}

std::string AuthBasic::GetSchemeName() const
{
    return "Basic";
}

bool AuthBasic::AddAuthHeaders(Request &request)
{
    std::string token = Data::Base64Encode(m_user + ":" + m_password);
    std::string header = GetSchemeName() + " " + token;
    request.GetHeader().SetHeader(HttpHeader::HeaderType::Authorization, header);

    return true;
}
