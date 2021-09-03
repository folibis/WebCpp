#include <sstream>
#include <algorithm>
#include "StringUtil.h"
#include "iostream"
#include <iomanip>


size_t StringUtil::SearchPosition(const ByteArray &str, const ByteArray&substring, size_t start, size_t end)
{
    const char* pstr = str.data();
    const char* psubstring = substring.data();
    size_t substringLen = substring.size();
    if(end == SIZE_MAX)
    {
        end = str.size() - 1;
    }

    for(size_t pos1 = start;pos1 <= end - substringLen + 1; pos1++)
    {
        size_t pos2;
        for(pos2 = 0; pos2 < substringLen; pos2++)
        {
            if(pstr[pos1 + pos2] == psubstring[pos2])
            {
                continue;
            }
            break;
        }

        if(pos2 == substringLen)
        {
            return pos1;
        }
    }

    return SIZE_MAX;
}

StringUtil::Ranges StringUtil::Split(const ByteArray &str, const ByteArray &delimiter, size_t start, size_t end)
{
    Ranges retval;
    size_t pos = SIZE_MAX;
    if(end == SIZE_MAX)
    {
        end = str.size() - 1;
    }

    while( (pos = SearchPosition(str, delimiter,start, end)) != SIZE_MAX)
    {
        size_t p1 = start;
        size_t p2 = pos - 1;
        retval.push_back(Range {p1, p2});

        if(pos >= end - delimiter.size())
        {
            retval.push_back(Range {pos + delimiter.size(), end});
            break;
        }
        start = pos + delimiter.size();
    }

    if(start < end)
    {
        retval.push_back( Range{ start, end } );
    }

    return retval;
}

size_t StringUtil::SearchPositionReverse(const ByteArray &str, const ByteArray &substring, size_t start, size_t end)
{
    const char* pstr = str.data();
    const char* psubstring = substring.data();
    size_t substringLen = substring.size();
    if(end == SIZE_MAX)
    {
        end = str.size() - 1;
    }

    if((end - start) < substringLen)
    {
        return SIZE_MAX;
    }

    for(size_t pos1 = end; pos1 >= start + substringLen - 1; pos1--)
    {
        size_t pos2;
        for(pos2 = substringLen; pos2 > 0; pos2--)
        {
            if(pstr[pos1 - (substringLen - pos2)] == psubstring[pos2 - 1])
            {
                continue;
            }
            break;
        }

        if(pos2 == 0)
        {
            return pos1 - substringLen + 1;
        }
    }

    return SIZE_MAX;
}

StringUtil::Ranges StringUtil::SplitReverse(const ByteArray &str, const ByteArray &delimiter, size_t start, size_t end)
{
    Ranges retval;
    size_t pos = SIZE_MAX;
    if(end == SIZE_MAX)
    {
        end = str.size();
    }

    while( (pos = SearchPositionReverse(str, delimiter,start, end)) != SIZE_MAX)
    {
        size_t p1 = pos + delimiter.size();
        size_t p2 = end;
        retval.push_back(Range{p1, p2});

        if(pos <= start + delimiter.size())
        {
            retval.push_back(Range{start, pos});
            break;
        }
        end = pos - 1;
    }

    if(end > 0)
    {
        retval.push_back( Range{ start, end } );
    }

    return retval;
}

ByteArray StringUtil::Trim(ByteArray &str, const ByteArray &chars)
{
    if(str.empty())
    {
        return str;
    }

    ByteArray::const_iterator b;
    ByteArray::const_iterator e;

    for(b = str.begin(); b != str.end(); b ++)
    {
        if(StringUtil::Contains(chars, *b))
        {
            continue;
        }
        break;
    }

    if(b == str.end())
    {
        return ByteArray();
    }

    for(e = str.end() - 1; e != b; e --)
    {
        if(StringUtil::Contains(chars, *e))
        {
            continue;
        }
        break;
    }

    if(e == b)
    {
        return ByteArray();
    }

    e += 1;

    if(b != str.begin() || e != str.end())
    {
        str = ByteArray(b, e);
    }
    return str;
}

bool StringUtil::Contains(const ByteArray &str, char ch)
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

// ----------------------  std::string -------------------------

std::vector<std::string> StringUtil::Split(const std::string &str, const char delimiter)
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

std::string& StringUtil::LTrim(std::string &str, const std::string &chars)
{
    str.erase(0, str.find_first_not_of(chars));
    return str;
}

std::string& StringUtil::RTrim(std::string &str, const std::string &chars)
{
    str.erase(str.find_last_not_of(chars) + 1);
    return str;
}

std::string& StringUtil::Trim(std::string &str, const std::string &chars)
{
    return LTrim(RTrim(str, chars), chars);
}

bool StringUtil::String2int(const std::string &str, int &value, int base)
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

void StringUtil::ToLower(std::string &str)
{
    std::transform(str.begin(), str.end(), str.begin(),[](unsigned char c)
    {
        return std::tolower(c);
    });
}

void StringUtil::ToUpper(std::string &str)
{
    std::transform(str.begin(), str.end(), str.begin(),[](unsigned char c)
    {
        return std::toupper(c);
    });
}

std::string StringUtil::ByteArray2String(const StringUtil::ByteArray &array)
{
    return std::string(array.begin(), array.end());
}

StringUtil::ByteArray StringUtil::String2ByteArray(const std::string &string)
{
    return ByteArray(string.begin(), string.end());
}

void StringUtil::UrlDecode(std::string &str)
{
    size_t from = 0;
    while((from = str.find('%', from)) != std::string::npos)
    {
        std::string value(str.begin() + from + 1, str.begin() + from + 3);
        int ascii;
        if(StringUtil::String2int(value, ascii, 16))
        {
            str.erase(from, 3);
            str.insert(from, 1, static_cast<char>(ascii));
        }
    }

    std::replace(str.begin(), str.end(), '+', ' ');
}

#ifndef NDEBUG
void StringUtil::Print(const StringUtil::ByteArray &array)
{
    size_t pos = 0;
    size_t size = array.size();

    while(pos < size)
    {
        std::string txt = "";
        std::cout << std::hex << std::setfill('0') << std::setw(6) << pos << " ";
        for(size_t i = 0;i < 16;i ++)
        {
            if(pos + i < size)
            {
                uint8_t ch = array[pos + i];

                if(ch >= 32 && ch <= 126)
                {
                    txt += static_cast<char>(ch);
                }
                else
                {
                    txt += ".";
                }
                std::cout << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(ch) << " ";
            }
            else
            {
                std::cout << "   ";
            }
        }
        std::cout << "| " << txt << std::endl;
        pos += 16;
    }
}
#endif

void StringUtil::Replace(std::string &str, const std::string &find, const std::string &replace)
{
    size_t index = 0;
    while (true)
    {
         index = str.find(find, index);
         if(index == std::string::npos)
         {
             break;
         }

         str.replace(index, find.size(), replace);
         index += replace.size();
    }
}

void StringUtil::RandInit()
{
    srand(static_cast<unsigned int>(time(nullptr)));
}

uint32_t StringUtil::GetRand(uint32_t min, uint32_t max)
{
    return static_cast<uint32_t>(rand()) % (max - min) + min;
}

bool StringUtil::Compare(const ByteArray &arr1, const ByteArray &arr2)
{
    if(arr1.size() != arr2.size())
    {
        return false;
    }

    for(size_t i = 0;i < arr1.size();i ++)
    {
        if(arr1[i] != arr2[i])
        {
            return false;
        }
    }

    return true;
}
