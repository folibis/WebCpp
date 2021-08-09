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

bool string2int(const std::string &str, int &value, int base)
{
    try
    {
        value = std::stoi(str, 0, base);
        return true;
    }
    catch(...)
    {
        return false;
    }
}

std::vector<ByteArray> split(const ByteArray &str, const char delimiter)
{
    std::vector<ByteArray> retval;

    auto it = str.begin();
    ByteArray::const_iterator i;

    while((i = std::find(it, str.end(), delimiter)) != str.end())
    {
        retval.push_back(ByteArray(it, i));
        it = i + 1;
    }

    if(it != str.end())
    {
        retval.push_back(ByteArray(it, str.end()));
    }

    return retval;
}

std::vector<ByteArray> split(const ByteArray &str, const ByteArray delimiter, size_t max)
{
    std::vector<ByteArray> retval;
    size_t pos = 0;
    size_t start = 0;

    while(look_for(str, delimiter, pos, start))
    {
        if(pos > 0)
        {
            retval.push_back(ByteArray(str.begin() + start, str.begin() + pos));
        }
        start = pos + delimiter.size();
        if(start >= max)
        {
            break;
        }
    }

    if(start < max)
    {
        retval.push_back(ByteArray(str.begin() + start, str.end()));
    }

    return retval;
}

ByteArray trim(ByteArray &str, const ByteArray &chars)
{
    if(str.empty())
    {
        return str;
    }

    ByteArray::const_iterator b;
    ByteArray::const_iterator e;

    for(b = str.begin(); b != str.end(); b ++)
    {
        if(contains(chars, *b))
        {
            continue;
        }
        break;
    }

    for(e = str.end() - 1; e != b; e --)
    {
        if(contains(chars, *e))
        {
            continue;
        }
        break;
    }

    e += 1;

    if(b != str.begin() || e != str.end())
    {
        str = ByteArray(b, e);
    }
    return str;
}

bool contains(const ByteArray &str, char ch)
{
    for(auto &c: str)
    {
        if(c == ch)
        {
            return true;
        }
    }

    return false;
}

bool look_for(const ByteArray &str, const ByteArray &search, size_t &position, size_t start)
{
    auto ptr1 = str.data();
    auto ptr2 = search.data();

    auto size1 = str.size();
    auto size2 = search.size();

    if(size1 < size2)
    {
        return false;
    }

    for(size_t pos1 = start; pos1 < (size1 - size2); pos1 ++)
    {        
        for(size_t pos2 = 0; pos2 < size2; pos2 ++)
        {
            if(ptr1[pos1 + pos2] != ptr2[pos2])
            {
                break;
            }

            if(pos2 == (size2 - 1))
            {
                position = pos1;
                return true;
            }
        }        
    }

    return false;
}

bool look_for(const std::string &str, const std::string &search, size_t &position, size_t start)
{
    if(search.empty() || str.empty())
    {
        return false;
    }

    auto pos = str.find(search, start);
    if(pos != std::string::npos)
    {
        position = pos;
        return true;
    }

    return false;
}

bool look_for(const ByteArray &str, const std::string &search, size_t &position, size_t start)
{
    if(search.empty() || str.empty())
    {
        return false;
    }

    auto arr = ByteArray(search.begin(), search.end());
    return look_for(str, arr, position, start);
}

void toLower(std::string &str)
{
    std::transform(str.begin(), str.end(), str.begin(),[](unsigned char c)
    {
        return std::tolower(c);
    });
}

void toUpper(std::string &str)
{
    std::transform(str.begin(), str.end(), str.begin(),[](unsigned char c)
    {
        return std::toupper(c);
    });
}


bool compare(const char *ch1, const char *ch2, size_t size)
{
    for(size_t i = 0; i < size;i ++)
    {
        if(ch1[i] != ch2[i])
        {
            return false;
        }
    }

    return true;
}
