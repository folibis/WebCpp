#ifndef SHA256_H
#define SHA256_H

#include <inttypes.h>
#include <string>

class SHA256
{
public:
    std::string Hash(const char *data);

protected:
    void Init();
    void Transform(uint8_t data[]);
    void Update(uint8_t data[], uint32_t len);
    void Final(uint8_t hash[]);

private:
    typedef struct
    {
        uint8_t data[64];
        uint32_t datalen;
        uint32_t bitlen[2];
        uint32_t state[8];
    } SHA256_CTX;

    SHA256_CTX ctx;
};

#endif // SHA256_H
