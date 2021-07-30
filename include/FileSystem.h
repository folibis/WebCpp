#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <string>

namespace WebCpp
{

class FileSystem
{
public:
    static std::string ExtractFileName(const std::string &path);
    static std::string ExtractFileExtension(const std::string &path);
    static bool IsFileExist(const std::string &path);
    static int GetFileSize(const std::string &path);
    static char PathDelimiter();
    static std::string GetDateTime();
};

}

#endif // FILESYSTEM_H
