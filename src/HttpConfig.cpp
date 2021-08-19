#include "HttpConfig.h"
#include "Print.h"


using namespace WebCpp;

HttpConfig::HttpConfig()
{
    Init();
}

void HttpConfig::Init()
{
    if(Load() == false)
    {
        Print() << "error while loading settings" << std::endl;
    }
}

bool HttpConfig::Load()
{
    return true;
}

std::string HttpConfig::ToString() const
{
    return "HttpConfig (name: " + m_ServerName + ", protocol: " + m_HttpProtocol + ", port: " + std::to_string(m_HttpServerPort) + ")";
}
