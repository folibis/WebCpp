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
};

#endif // STRINGUTIL_H
