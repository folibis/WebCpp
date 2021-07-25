#include <sstream>
#include "common.h"
#include <algorithm>


std::vector<std::string> split(const std::string &str, const char delimiter)
{
    std::vector<std::string> strings;
    std::istringstream f(str);
    std::string s;
    while (getline(f, s, delimiter))
    {
        strings.push_back(s);
    }

    return strings;
}

std::string &ltrim(std::string &str, const std::string &chars)
{
    str.erase(0, str.find_first_not_of(chars));
    return str;
}

std::string &rtrim(std::string &str, const std::string &chars)
{
    str.erase(str.find_last_not_of(chars) + 1);
    return str;
}

std::string &trim(std::string &str, const std::string &chars)
{
    return ltrim(rtrim(str, chars), chars);
}

bool string2int(const std::string &str, int &value)
{
    try
    {
        value = std::stoi(str);
        return true;
    }
    catch(...)
    {
        return false;
    }
}

std::vector<byte_array> split(const byte_array &str, const char delimiter)
{
    std::vector<byte_array> retval;

    auto it = str.begin();
    byte_array::const_iterator i;

    while((i = std::find(it, str.end(), delimiter)) != str.end())
    {
        retval.push_back(byte_array(it, i));
        it = i;
    }

    return retval;
}
