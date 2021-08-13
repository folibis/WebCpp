#ifndef STRINGUTIL_H
#define STRINGUTIL_H

#include "common.h"


class StringUtil
{
public:
    using ByteArray = std::vector<char>;

    struct Range
    {
        size_t p1;
        size_t p2;
    };

    using Ranges = std::vector<Range>;

    static size_t SearchPosition(const ByteArray &str, const ByteArray &substring, size_t start, size_t end);
    static Ranges Split(const ByteArray &str, const ByteArray &delimiter, size_t start, size_t end);
    static size_t SearchPositionReverse(const ByteArray &str, const ByteArray &substring, size_t start, size_t end);
    static Ranges SplitReverse(const ByteArray &str, const ByteArray &delimiter, size_t start, size_t end);
};

#endif // STRINGUTIL_H
