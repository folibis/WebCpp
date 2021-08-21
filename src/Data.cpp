#include <stdexcept>
#include "Sha1.h"
#include "Data.h"


static const char* base64_chars[2] = {
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789"
    "+/",

    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789"
    "-_"};


std::string Data::Base64Encode(unsigned char const* bytes_to_encode, size_t in_len)
{
    size_t len_encoded = (in_len +2) / 3 * 4;

    unsigned char trailing_char = '=';

    const char* base64_chars_ = base64_chars[1];

    std::string ret;
    ret.reserve(len_encoded);

    unsigned int pos = 0;

    while (pos < in_len)
    {
        ret.push_back(base64_chars_[(bytes_to_encode[pos + 0] & 0xfc) >> 2]);

        if (pos+1 < in_len)
        {
            ret.push_back(base64_chars_[((bytes_to_encode[pos + 0] & 0x03) << 4) + ((bytes_to_encode[pos + 1] & 0xf0) >> 4)]);

            if (pos+2 < in_len)
            {
                ret.push_back(base64_chars_[((bytes_to_encode[pos + 1] & 0x0f) << 2) + ((bytes_to_encode[pos + 2] & 0xc0) >> 6)]);
                ret.push_back(base64_chars_[  bytes_to_encode[pos + 2] & 0x3f]);
            }
            else
            {
                ret.push_back(base64_chars_[(bytes_to_encode[pos + 1] & 0x0f) << 2]);
                ret.push_back(trailing_char);
            }
        }
        else
        {
            ret.push_back(base64_chars_[(bytes_to_encode[pos + 0] & 0x03) << 4]);
            ret.push_back(trailing_char);
            ret.push_back(trailing_char);
        }

        pos += 3;
    }

    return ret;
}

std::string Data::Base64Encode(const std::string &str)
{
    return Data::Base64Encode(reinterpret_cast<unsigned char const*>(str.c_str()), str.size());
}

std::string Data::Base64Decode(const std::string &str)
{
    if (str.empty()) return std::string();

    size_t length_of_string = str.length();
    size_t pos = 0;

    size_t approx_length_of_decoded_string = length_of_string / 4 * 3;
    std::string ret;
    ret.reserve(approx_length_of_decoded_string);

    while (pos < length_of_string)
    {
        size_t pos_of_char_1 = pos_of_char(str[pos+1] );

        ret.push_back(static_cast<std::string::value_type>(((pos_of_char(str[pos+0])) << 2) + ((pos_of_char_1 & 0x30 ) >> 4)));

        if ( ( pos + 2 < length_of_string  )       &&
             str[pos+2] != '='        &&
             str[pos+2] != '.'
             )
        {
            unsigned int pos_of_char_2 = pos_of_char(str[pos+2] );
            ret.push_back(static_cast<std::string::value_type>( (( pos_of_char_1 & 0x0f) << 4) + (( pos_of_char_2 & 0x3c) >> 2)));

            if ( ( pos + 3 < length_of_string )     &&
                 str[pos+3] != '='     &&
                 str[pos+3] != '.'
                 )
            {
                ret.push_back(static_cast<std::string::value_type>( ( (pos_of_char_2 & 0x03 ) << 6 ) + pos_of_char(str[pos+3])   ));
            }
        }

        pos += 4;
    }

    return ret;
}

unsigned int Data::pos_of_char(const unsigned char chr)
{
    if      (chr >= 'A' && chr <= 'Z') return chr - 'A';
    else if (chr >= 'a' && chr <= 'z') return chr - 'a' + ('Z' - 'A')               + 1;
    else if (chr >= '0' && chr <= '9') return chr - '0' + ('Z' - 'A') + ('z' - 'a') + 2;
    else if (chr == '+' || chr == '-') return 62;
    else if (chr == '/' || chr == '_') return 63;
    else
        throw std::runtime_error("Input is not valid base64-encoded data.");
}

std::string Data::Sha1(const std::string &string)
{
    SHA1 checksum;
    checksum.update(string);
    return checksum.final();
}

bool Data::HexString2Array(const std::string &str, unsigned char *data)
{
    for(size_t i = 0;i < str.size();i += 2)
    {
        std::string val;
        val += str.at(i);
        val += str.at(i + 1);
        data[i / 2] = std::stoi(val, nullptr, 16);
    }

    return true;
}
