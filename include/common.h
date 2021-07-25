#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <vector>


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
std::string &trim(std::string &str, const std::string &chars);
bool string2int(const std::string &str, int &value);


using byte_array = std::vector<char>;

std::vector<byte_array> split(const byte_array &str, const char delimiter);


#endif // COMMON_H
