#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <string>

namespace WebCpp
{

class FileSystem
{
public:
    static std::string GetFullPath(const std::string &path);
    static std::string GetApplicationFolder();
    static bool ChangeDir(const std::string &path);
    static std::string NormalizePath(const std::string &path);
    static std::string ExtractFileName(const std::string &path);
    static std::string ExtractFileExtension(const std::string &path);
    static bool IsFileExist(const std::string &path);
    static int GetFileSize(const std::string &path);
    static char PathDelimiter();
    static bool CreateFolder(const std::string &path);
    static bool DeleteFolder(const std::string &path);
    static std::string GetDateTime();
    static std::string GetFileModifiedTime(const std::string &file);
    static std::string TempFolder();
    static std::string Root();
};

}

#endif // FILESYSTEM_H
