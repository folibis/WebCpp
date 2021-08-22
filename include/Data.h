#ifndef DATA_H
#define DATA_H

#include <string>

class Data
{
public:
    static std::string Base64Encode(const unsigned char *bytes_to_encode, size_t in_len);
    static std::string Base64Encode(const std::string& str);
    static std::string Base64Decode(const std::string& str);
    static std::string Sha1(const std::string &string);
    static uint8_t *Sha1Digest(const std::string &string);

private:
    static unsigned int pos_of_char(const unsigned char chr);
};

#endif // DATA_H
