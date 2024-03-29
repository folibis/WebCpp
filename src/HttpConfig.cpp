#include "common_webcpp.h"
#include "defines_webcpp.h"
#include "DebugPrint.h"
#include "FileSystem.h"
#include "HttpConfig.h"

using namespace WebCpp;


HttpConfig::HttpConfig()
{

}

HttpConfig &HttpConfig::Instance()
{
    static HttpConfig instance;
    if(instance.m_initialized == false)
    {
        instance.Init();
    }

    return instance;
}

bool HttpConfig::Init()
{
    if(Load() == false)
    {
        DebugPrint() << "error while loading settings" << std::endl;
        m_initialized = false;
    }
    else
    {
        SetRootFolder();
        m_initialized = true;
    }

    return m_initialized;
}

bool HttpConfig::Load()
{
    return true;
}

std::string HttpConfig::RootFolder() const
{
    return m_rootFolder;
}

std::string HttpConfig::ToString() const
{
    return std::string("HttpConfig :") + "\n" +
            "\tname: " + m_ServerName + "\n" +
            "\tHTTP protocol: " + Http::Protocol2String(m_HttpProtocol) + "\n" +
            "\tHTTP port: " + std::to_string(m_HttpServerPort) + "\n" +
            "\tWebSocket protocol: " + Http::Protocol2String(m_WsProtocol) + "\n" +
            "\tWebSocket port: " + std::to_string(m_WsServerPort) + "\n" +
            "\tRoot : " + m_rootFolder + "\n";
}

void HttpConfig::SetRootFolder()
{
    std::string root = FileSystem::NormalizePath(GetRoot());
    std::string root_full = FileSystem::NormalizePath(FileSystem::GetFullPath(root));
    if(root != root_full)
    {
        m_rootFolder = FileSystem::NormalizePath(FileSystem::GetApplicationFolder()) + root;
    }
    else
    {
        m_rootFolder = root;
    }
}

void HttpConfig::OnChanged(const std::string &value)
{
    switch(_(value.c_str()))
    {
        case _("Root"):
            SetRootFolder();
            break;
    }
}
