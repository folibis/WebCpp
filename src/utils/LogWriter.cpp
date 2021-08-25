#include "FileSystem.h"
#include "LogWriter.h"
#include <iostream>
#include <cstring>
#define DEFAULT_FOLDER "/var/log/webcpp"


LogWriter::~LogWriter()
{
    Close();
}

LogWriter& LogWriter::Instance()
{
    static LogWriter instance;
    return instance;
}

void LogWriter::Write(const std::string &text, LogWriter::LogType type)
{
    auto &stream = m_streams[static_cast<int>(type)];
    if(stream.is_open())
    {
        std::string s = "[" + WebCpp::FileSystem::GetDateTime() + "] " + text;
        stream << s << std::endl;
    }
}

LogWriter::LogWriter()
{
    m_opened = Open();
}

bool LogWriter::Open()
{
    try
    {
        if(WebCpp::FileSystem::CreateFolder(DEFAULT_FOLDER))
        {
            for(int i = 0;i < 3;i ++)
            {
                LogType type = static_cast<LogType>(i);
                std::string file = std::string(DEFAULT_FOLDER) + "/" + LogWriter::LogType2String(type) + ".log";
                m_streams[i].open(file, std::ofstream::binary | std::ios::app);
            }
        }

        return true;
    }
    catch(...)
    {
        return false;
    }
}

bool LogWriter::Close()
{
    try
    {
        for(int i = 0;i < 3;i ++)
        {
            auto &stream = m_streams[i];
            if(stream)
            {
                stream.close();
            }
        }

        return true;
    }
    catch(...)
    {
        return false;
    }
}

std::string LogWriter::LogType2String(LogWriter::LogType type)
{
    switch(type)
    {
        case LogWriter::LogType::Info:
            return "info";
        case LogWriter::LogType::Error:
            return "error";
        case LogWriter::LogType::Access:
            return "access";
        default: break;
    }

    return "";
}
