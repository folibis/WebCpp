#include <stdexcept>
#include "Sha1.h"
#include "Data.h"

static const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";


static inline bool is_base64(unsigned char c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string Data::Base64Encode(const unsigned char* bytes_to_encode, size_t in_len)
{
    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while (in_len--) {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for(i = 0; (i <4) ; i++)
                ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i)
    {
        for(j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; (j < i + 1); j++)
            ret += base64_chars[char_array_4[j]];

        while((i++ < 3))
            ret += '=';

    }

    return ret;

}

std::string Data::Base64Encode(const std::string &str)
{
    return Data::Base64Encode(reinterpret_cast<unsigned char const*>(str.c_str()), str.size());
}

std::string Data::Base64Decode(std::string const& encoded_string)
{
    int in_len = encoded_string.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::string ret;

    while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
        char_array_4[i++] = encoded_string[in_]; in_++;
        if (i ==4) {
            for (i = 0; i <4; i++)
                char_array_4[i] = base64_chars.find(char_array_4[i]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
                ret += char_array_3[i];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j <4; j++)
            char_array_4[j] = 0;

        for (j = 0; j <4; j++)
            char_array_4[j] = base64_chars.find(char_array_4[j]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
    }

    return ret;
}

std::string Data::Sha1(const std::string &string)
{
    SHA1 checksum;
    checksum.update(string);
    return checksum.final();
}

uint8_t * Data::Sha1Digest(const std::string &string)
{    
    SHA1 checksum;
    checksum.update(string);
    return checksum.digest();
}


#ifdef WITH_ZLIB
#include "zlib.h"
#define CHUNK 0x4000

ByteArray Data::Compress(const ByteArray &data)
{
    return data;
}

ByteArray Data::Uncompress(const ByteArray &data)
{
    ByteArray retval;

    try
    {
        z_stream strm = {0};
        unsigned char out[CHUNK];

        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;
        strm.next_in = nullptr;
        strm.avail_in = 0;

        if(inflateInit2(&strm, -MAX_WBITS) != Z_OK)
        {
            throw std::runtime_error("");
        }

        bool done = false;
        size_t pos = 0;
        size_t sizeIn;
        size_t sizeOut = 0;
        uint8_t *ptr = const_cast<uint8_t *>(data.data());
        size_t size = data.size();

        while(!done)
        {
            sizeIn = ((size - pos) > CHUNK ? CHUNK : (size - pos));
            strm.avail_in = sizeIn;
            strm.next_in = ptr + pos;
            strm.avail_out = CHUNK;
            strm.next_out = out;
            int err = inflate(&strm, Z_NO_FLUSH);
            if (err == Z_STREAM_END)
            {
                done = true;
            }
            else if (err != Z_OK)
            {
                throw std::runtime_error("");
            }
            size_t chunkSize = strm.total_out - sizeOut;
            sizeOut = strm.total_out;
            retval.insert(retval.end(), out, out + chunkSize);
            pos = strm.total_in;
            if(pos >= size)
            {
                done = true;
            }
        }

        inflateEnd(&strm);
    }
    catch(...)
    {
        return ByteArray();
    }

    return retval;
}

ByteArray Data::Zip(const ByteArray &data)
{
    return data;
}

ByteArray Data::Unzip(const ByteArray &data)
{
    ByteArray retval;

    try
    {
        z_stream strm = {0};
        unsigned char out[CHUNK];

        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;
        strm.next_in = nullptr;
        strm.avail_in = 0;

        if(inflateInit2(&strm, 16 + MAX_WBITS) != Z_OK)
        {
            throw std::runtime_error("");
        }

        bool done = false;
        size_t pos = 0;
        size_t sizeIn;
        size_t sizeOut = 0;
        uint8_t *ptr = const_cast<uint8_t *>(data.data());
        size_t size = data.size();

        while(!done)
        {
            sizeIn = ((size - pos) > CHUNK ? CHUNK : (size - pos));
            strm.avail_in = sizeIn;
            strm.next_in = ptr + pos;
            strm.avail_out = CHUNK;
            strm.next_out = out;
            int err = inflate(&strm, Z_SYNC_FLUSH);
            if (err == Z_STREAM_END)
            {
                done = true;
            }
            else if (err != Z_OK)
            {
                throw std::runtime_error("");
            }
            size_t chunkSize = strm.total_out - sizeOut;
            sizeOut = strm.total_out;
            retval.insert(retval.end(), out, out + chunkSize);
            pos = strm.total_in;
            if(pos >= size)
            {
                done = true;
            }
        }

        inflateEnd(&strm);
    }
    catch(...)
    {
        return ByteArray();
    }

    return retval;
}

#endif
