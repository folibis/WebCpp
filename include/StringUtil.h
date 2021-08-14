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
};

#endif // STRINGUTIL_H
