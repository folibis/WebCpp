#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <vector>

#define CR '\r'
#define LF '\n'
#define CRLF '\r','\n'
#define CRLFCRLF '\r','\n','\r','\n'

#define WEBCPP_VERSION "0.1"
#define WEBCPP_NAME "WebCpp"
#define WEBCPP_CANONICAL_NAME WEBCPP_NAME " " WEBCPP_VERSION


constexpr uint64_t mix(char m, uint64_t s)
{
    return ((s << 7) + ~(s >> 3)) + static_cast<uint64_t>(~m);
}

constexpr uint64_t _(const char* str)
{
    return (*str) ? mix(*str,_(str + 1)) : 0;
}

constexpr uint64_t operator "" _(const char* str)
{
    return _(str);
}

std::vector<std::string> split(const std::string &str, const char delimiter);
std::string &ltrim(std::string &str, const std::string &chars);
std::string &rtrim(std::string &str, const std::string &chars);
std::string &trim(std::string &str, const std::string &chars = " \r\n\t");
bool string2int(const std::string &str, int &value, int base = 10);
void toLower(std::string &str);
void toUpper(std::string &str);

using ByteArray = std::vector<char>;

std::vector<ByteArray> split(const ByteArray &str, const char delimiter);
std::vector<ByteArray> split(const ByteArray &str, const ByteArray delimiter, size_t max = SIZE_MAX);
ByteArray trim(ByteArray &str, const ByteArray &chars = { ' ',CRLF,'\t' });
bool contains(const ByteArray &str, char ch);
bool look_for(const ByteArray &str, const ByteArray &search, size_t &position, size_t start = 0);
bool look_for(const std::string &str, const std::string &search, size_t &position, size_t start = 0);

bool compare(const char *ch1, const char *ch2, size_t size);

#endif // COMMON_H
