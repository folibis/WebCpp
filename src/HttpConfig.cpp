#include "common_webcpp.h"
#include "defines_webcpp.h"
#include "DebugPrint.h"
#include "FileSystem.h"
#include "HttpConfig.h"

using namespace WebCpp;

HttpConfig::HttpConfig()
{
    Init();
}

void HttpConfig::Init()
{
    if(Load() == false)
    {
        DebugPrint() << "error while loading settings" << std::endl;
    }

    SetRootFolder();
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
    return "HttpConfig (name: " + m_ServerName + ", protocol: " + m_HttpProtocol + ", port: " + std::to_string(m_HttpServerPort) + ")";
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
