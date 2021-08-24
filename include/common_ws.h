#ifndef COMMON_WS_H
#define COMMON_WS_H
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

#include <inttypes.h>


#pragma pack(push, 1)
    struct Flag1
    {
        uint8_t opcode: 4;
        uint8_t RSV3:   1;
        uint8_t RSV2:   1;
        uint8_t RSV1:   1;
        uint8_t FIN:    1;
    };
    struct Flag2
    {
        uint8_t PayloadLen: 7;
        uint8_t Mask:       1;
    };

    struct WebSocketHeader
    {
        Flag1 flags1;
        Flag2 flags2;
    };
    struct WebSocketHeaderLength2
    {
        uint16_t length;
    };
    struct WebSocketHeaderLength3
    {
        uint64_t length;
    };
    struct WebSocketHeaderMask
    {
        uint8_t bytes[4];
    };
#pragma pack(pop)

    enum class MessageType
    {
        Undefined = 0,
        Text = 1,
        Binary = 2,
        Ping = 9,
        Pong = 10,
    };

#endif // COMMON_WS_H
