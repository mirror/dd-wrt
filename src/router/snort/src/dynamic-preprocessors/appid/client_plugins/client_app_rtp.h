#ifndef CLIENT_APP_RTP_H_
#define CLIENT_APP_RTP_H_

#include <stdint.h>

#pragma pack(1)
typedef struct
{
#if defined(WORDS_BIGENDIAN)
    uint8_t vers:2,
             padding:1,
             extension:1,
             count:4;
    uint8_t marker:1,
             payloadtype:7;
#else
    uint8_t count:4,
             extension:1,
             padding:1,
             vers:2;
    uint8_t payloadtype:7,
             marker:1;
#endif
    uint16_t seq;
    uint32_t timestamp;
    uint32_t ssrc;
} ClientRTPMsg;
#pragma pack()

#endif // !CLIENT_APP_RTP_H_
