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

#ifndef WEBCPP_DATA_H
#define WEBCPP_DATA_H

#include <string>
#include "common_webcpp.h"


class Data
{
public:
    static std::string Base64Encode(const unsigned char *bytes_to_encode, size_t in_len);
    static std::string Base64Encode(const std::string& str);
    static std::string Base64Decode(const std::string& str);
    static std::string Sha1(const std::string &string);
    static uint8_t *Sha1Digest(const std::string &string);
    static std::string Sha256(const std::string &string);

#ifdef WITH_ZLIB
    static ByteArray Compress(const ByteArray &data);
    static ByteArray Uncompress(const ByteArray &data);
    static ByteArray Zip(const ByteArray &data);
    static ByteArray Unzip(const ByteArray &data);
#endif

private:
    static unsigned int pos_of_char(const unsigned char chr);
};

#endif // WEBCPP_DATA_H
