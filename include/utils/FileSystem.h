/*
*
* Copyright (c) 2021 ruslan@muhlinin.com
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
*/

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
    static std::string GetTimeStamp();
    static std::string GetFileModifiedTime(const std::string &file);
    static std::string TempFolder();
    static std::string Root();
};

}

#endif // FILESYSTEM_H
