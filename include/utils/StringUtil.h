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

#ifndef STRINGUTIL_H
#define STRINGUTIL_H

#include "common.h"


class StringUtil
{
public:
    using ByteArray = std::vector<char>;

    struct Range
    {
        size_t start;
        size_t end;
    };

    using Ranges = std::vector<Range>;

    static size_t SearchPosition(const ByteArray &str, const ByteArray &substring, size_t start = 0, size_t end = SIZE_MAX);
    static Ranges Split(const ByteArray &str, const ByteArray &delimiter, size_t start = 0, size_t end = SIZE_MAX);
    static size_t SearchPositionReverse(const ByteArray &str, const ByteArray &substring, size_t start = 0, size_t end = SIZE_MAX);
    static Ranges SplitReverse(const ByteArray &str, const ByteArray &delimiter, size_t start = 0, size_t end = SIZE_MAX);
    static ByteArray Trim(ByteArray &str, const ByteArray &chars = { '\r','\n','\t' });
    static bool Contains(const ByteArray &str, char ch);
    static std::vector<std::string> Split(const std::string &str, const char delimiter);
    static std::string &LTrim(std::string &str, const std::string &chars);
    static std::string &RTrim(std::string &str, const std::string &chars);
    static std::string &Trim(std::string &str, const std::string &chars = " \r\n\t");
    static bool String2int(const std::string &str, int &value, int base = 10);
    static void ToLower(std::string &str);
    static void ToUpper(std::string &str);
    static std::string ByteArray2String(const ByteArray &array);
    static ByteArray String2ByteArray(const std::string &string);
    static void UrlDecode(std::string &str);
    static void Print(const StringUtil::ByteArray &array);
    static void Replace(std::string &str, const std::string &find, const std::string &replace);
};

#endif // STRINGUTIL_H
