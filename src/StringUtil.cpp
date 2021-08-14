#include "StringUtil.h"


size_t StringUtil::SearchPosition(const ByteArray &str, const ByteArray&substring, size_t start, size_t end)
{
    const char* pstr = str.data();
    const char* psubstring = substring.data();
    size_t substringLen = substring.size();
    if(end == SIZE_MAX)
    {
        end = str.size() - 1;
    }

    for(size_t pos1 = start;pos1 <= end - substringLen; pos1++)
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

    return retval;
}
