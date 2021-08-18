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

using ByteArray = std::vector<char>;

struct point
{
    size_t p1;
    size_t p2;
};
using PointArray = std::vector<point>;


#endif // COMMON_H
