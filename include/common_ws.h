#ifndef COMMON_WS_H
#define COMMON_WS_H

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

#endif // COMMON_WS_H
