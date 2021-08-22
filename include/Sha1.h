#ifndef SHA1_H
#define SHA1_H

#include <iostream>
#include <string>
#include <cstdint>


class SHA1
{
public:
    SHA1();
    void update(const std::string &s);
    void update(std::istream &is);
    std::string final();
    uint8_t* digest();

    static std::string from_file(const std::string &filename);

private:
    typedef uint64_t uint32;   /* just needs to be at least 32bit */
    typedef uint64_t uint64;  /* just needs to be at least 64bit */

    static const unsigned int DIGEST_INTS = 5;  /* number of 32bit integers per SHA1 digest */
    static const unsigned int BLOCK_INTS = 16;  /* number of 32bit integers per SHA1 block */
    static const unsigned int BLOCK_BYTES = BLOCK_INTS * 4;

    uint64 m_digest[DIGEST_INTS];
    std::string m_buffer;
    uint64 m_transforms;

    void prepare();
    void reset();
    void transform(uint32 block[BLOCK_BYTES]);

    static void buffer_to_block(const std::string &buffer, uint32 block[BLOCK_BYTES]);
    static void read(std::istream &is, std::string &s, int max);
};

std::string sha1(const std::string &string);

#endif /* SHA1_H */
