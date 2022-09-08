#include "AuthBasic.h"


using namespace WebCpp;
REGISTER_AUTH("Basic", AuthBasic)

AuthBasic::AuthBasic()
{

}

bool AuthBasic::Init()
{
    return false;
}

bool AuthBasic::Parse(const Request &request)
{
    return false;
}

std::string AuthBasic::GetChallenge() const
{
    return "";
}
