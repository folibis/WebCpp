#ifndef COMMON_H
#define COMMON_H

/*
*
* Copyright (c) 2021 ruslan@muhlinin.com
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
*/

#include <string>
#include <vector>

#define CR '\r'
#define LF '\n'
#define CRLF '\r','\n'
#define CRLFCRLF '\r','\n','\r','\n'

#define WEBCPP_VERSION "0.2"
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

using ByteArray = std::vector<uint8_t>;

struct point
{
    size_t p1;
    size_t p2;
};
using PointArray = std::vector<point>;


#endif // COMMON_H
