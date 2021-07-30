#include <sys/stat.h>
#include <iostream>
#include <ctime>
#include "FileSystem.h"

#define MAX_EXT_LENGTH 8

using namespace WebCpp;

std::string FileSystem::ExtractFileName(const std::string& path)
{
    size_t i = path.rfind(FileSystem::PathDelimiter(), path.length());
    if (i != std::string::npos)
    {
        return(path.substr(i+1, path.length() - i));
    }

    return path;
}

std::string FileSystem::ExtractFileExtension(const std::string &path)
{
    size_t i = path.rfind('.', path.length());
    if (i != std::string::npos && i < MAX_EXT_LENGTH)
    {
        return(path.substr(i + 1, path.length() - i));
    }

    return path;
}

bool FileSystem::IsFileExist(const std::string &path)
{
    bool retval = false;
    struct stat sb;

    try
    {
        if (stat(path.c_str(), &sb) == 0)
        {
            retval = true;
        }
    }
    catch (std::exception &ex)
    {
        std::cout << "reading file attributes failed: " << ex.what() << std::endl;
    }
    return retval;
}

int FileSystem::GetFileSize(const std::string &path)
{
    int fileSize = -1;
    struct stat sb;
    try
    {
        if (stat(path.c_str(), &sb) == 0)
            fileSize = sb.st_size;
    }
    catch (std::exception &ex)
    {
        std::cout << "reading file size failed: " << ex.what() << std::endl;
    }

    return fileSize;
}

char FileSystem::PathDelimiter()
{
#ifdef _WIN32
    return '\\';
#else
    return '/';
#endif
}

std::string FileSystem::GetDateTime()
{
    time_t tm;
    struct tm * timeinfo;
    char buffer[30];

    time(&tm);
    const time_t *t = &tm;
    timeinfo = gmtime(t);
    strftime(buffer, 30, "%a, %d %b %Y %H:%M:%S GMT", timeinfo);
    return buffer;
}
