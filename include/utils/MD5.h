/*
Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD5 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.

License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.

https://www.rfc-editor.org/rfc/rfc1321

*/

#ifndef MD5_H
#define MD5_H

#include <inttypes.h>
#include  <stdio.h>

class MD5
{
public:
    MD5();
    void MD5Init();
    void MD5Update(const uint8_t *input, size_t inputLen);
    void MD5Final (uint8_t digest[16]);

protected:
    void Encode(uint8_t *output, uint32_t *input, size_t len);
    void Decode(uint32_t *output, const uint8_t *input, size_t len);
    void MD5Transform(uint32_t state[], const uint8_t block[]);

private:
    typedef struct
    {
        uint32_t state[4];
        uint32_t count[2];
        uint8_t buffer[64];
    } MD5_CTX;

    MD5_CTX context;
};

#endif // MD5_H
