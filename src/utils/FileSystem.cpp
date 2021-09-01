#include <sys/stat.h>
#include <unistd.h>
#include <linux/limits.h>
#include <dirent.h>
#include <iostream>
#include <cstring>
#include <ctime>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <vector>
#include "FileSystem.h"
#include "StringUtil.h"
#include "common.h"

#define MAX_EXT_LENGTH 8


using namespace WebCpp;

std::string FileSystem::GetFullPath(const std::string &path)
{
    char buffer[PATH_MAX];
    char *retval = realpath(path.c_str(), buffer);

    if(retval == nullptr)
    {
        return "";
    }

    return std::string(retval);
}

std::string FileSystem::GetApplicationFolder()
{
    char buffer[PATH_MAX];
    int len = readlink("/proc/self/exe", buffer, 250);
    if(len >= 0)
    {
        return std::string(buffer, len);
    }

    return "";
}

bool FileSystem::ChangeDir(const std::string &path)
{
    return (chdir(path.c_str()) == 0);
}

std::string FileSystem::NormalizePath(const std::string &path)
{
    std::string tmp = path;
    StringUtil::Replace(tmp, "//", "/");
    if(tmp.rfind(FileSystem::PathDelimiter()) != (tmp.length() - 1))
    {
        return tmp + FileSystem::PathDelimiter();
    }

    return tmp;
}

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
    if (i != std::string::npos && (path.length() - i) < MAX_EXT_LENGTH)
    {
        return(path.substr(i + 1, path.length() - i));
    }

    return "";
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

std::string FileSystem::Root()
{
#ifdef _WIN32
    char const *folder = getenv("SystemDrive");
    if(folder == nullptr)
    {
        return "\\";
    }
    std::string root(folder);
    if(root.at(root.size() - 1) != FileSystem::PathDelimiter())
    {
        root += FileSystem::PathDelimiter();
    }
    return root;
#else
    return "/";
#endif
}

bool FileSystem::IsDir(const std::string &path)
{
    struct stat result;
    if(stat(path.c_str(), &result) == 0)
    {
        if((result.st_mode & S_IFDIR) != 0)
        {
            return true;
        }
    }

    return false;
}

std::vector<FileSystem::FileInfo> FileSystem::GetFolder(const std::string &path)
{
    std::vector<FileSystem::FileInfo> retval;

    DIR *dir; struct dirent *diread; struct stat info;

    if ((dir = opendir(path.c_str())) != nullptr)
    {
        while ((diread = readdir(dir)) != nullptr)
        {
            long size = (-1);
            std::string mod;
            if(stat(std::string(NormalizePath(path) + diread->d_name).c_str(), &info) == 0)
            {
                auto mod_time = info.st_mtime;
                const time_t *t = &mod_time;
                struct tm * timeinfo = gmtime(t);
                char buffer[30];
                strftime(buffer, 30, "%a, %d %b %Y %H:%M:%S GMT", timeinfo);
                mod = buffer;
                size = info.st_size;
            }

            retval.push_back(FileInfo { diread->d_name, (diread->d_type == DT_DIR), size, mod });
        }
        closedir(dir);
    }

    return retval;
}

bool FileSystem::CreateFolder(const std::string &path)
{
    struct stat s;
    std::string fullpath = "";
    bool retval = false;

    try
    {
        auto list = StringUtil::Split(path, FileSystem::PathDelimiter());
        for(auto &folder: list)
        {
            if(fullpath.empty() && folder.empty())
            {
                fullpath = FileSystem::Root();
                continue;
            }
            fullpath += ((fullpath.empty() || fullpath == FileSystem::Root()) ? "" : std::string(1, FileSystem::PathDelimiter())) + folder;
            if(stat(fullpath.c_str(), &s) == 0)
            {
                if (S_ISDIR(s.st_mode))
                {
                    retval = true;
                    continue;
                }
            }
            else
            {
#ifdef __linux__
                if(mkdir(fullpath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0)
#else
                if(mkdir(fullpath.c_str()) != 0)
#endif
                {
                    retval = false;
                }
                else
                {
                    retval = true;
                }
            }
        }

        return retval;
    }
    catch(std::exception &ex)
    {
        std::cout << "exception while creating a folder: " << ex.what() << std::endl;
        return false;
    }
}

bool FileSystem::DeleteFolder(const std::string &path)
{
    DIR *dir;
    struct stat s;
    struct dirent *entry;

    try
    {
        if(stat(path.c_str(), &s) == 0)
        {
            dir = opendir(path.c_str());
            while((entry = readdir(dir)))
            {
                std::string file(entry->d_name);
                if (file == "." || file == "..")
                {
                    continue;
                }

                file = FileSystem::NormalizePath(path) + file;
                if(stat(file.c_str(), &s) == 0)
                {
                    if(S_ISDIR(s.st_mode))
                    {
                        DeleteFolder(file);
                    }
                    else
                    {
                        unlink(file.c_str());
                    }
                }
            }

            rmdir(path.c_str());

            return true;
        }
        else
        {
            throw "cannot access the folder";
        }
    }
    catch(std::exception &ex)
    {
        std::cout << "exception while removing the folder: " << ex.what() << std::endl;
        return false;
    }
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

std::string FileSystem::GetTimeStamp()
{
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) -
            std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch());

    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time), "%H:%M:%S");
    oss << ":" << ms.count() <<  std::endl;

    return oss.str();
}

std::string FileSystem::GetFileModifiedTime(const std::string &file)
{
    struct stat result;
    if(stat(file.c_str(), &result) == 0)
    {
        auto mod_time = result.st_mtime;
        const time_t *t = &mod_time;
        struct tm * timeinfo = gmtime(t);
        char buffer[30];
        strftime(buffer, 30, "%a, %d %b %Y %H:%M:%S GMT", timeinfo);
        return buffer;
    }

    return "";
}

void RandInit()
{
    srand(static_cast<unsigned int>(time(nullptr)));
}

uint32_t GetRand(uint32_t min, uint32_t max)
{
    return static_cast<uint32_t>(rand()) % (max - min) + min;
}

std::string FileSystem::TempFolder()
{
    std::vector<std::string> tmpFolders = { "TMPDIR","TEMP","TMP", "TEMPDIR" };

    char const *folder = nullptr;
    for(auto &env: tmpFolders)
    {
        folder = getenv(env.c_str());
        if(folder == nullptr)
        {
            continue;
        }
    }

    if(folder == nullptr)
    {
        folder = "/tmp";
    }

    std::string path(folder);

    if(path.at(path.size() - 1) != FileSystem::PathDelimiter())
    {
        path += FileSystem::PathDelimiter();
    }

    RandInit();
    for(int i = 0;i < 6;i ++)
    {
        char ch = GetRand(static_cast<int>('a'), static_cast<int>('z'));
        path.push_back(ch);
    }

    return path;
}
