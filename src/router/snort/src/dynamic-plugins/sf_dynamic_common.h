/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2005-2013 Sourcefire, Inc.
 *
 */
#ifndef _SF_DYNAMIC_COMMON_H_
#define _SF_DYNAMIC_COMMON_H_

#ifndef WIN32
#include <sys/types.h>
#else
#include <stdint.h>
#endif

typedef enum {
    SF_FLAG_ALT_DECODE         = 0x0001,
    SF_FLAG_ALT_DETECT         = 0x0002,
    SF_FLAG_DETECT_ALL         = 0xffff
} SFDetectFlagType;

#ifdef SF_WCHAR
#include <wchar.h>
typedef void (*DebugWideMsgFunc)(uint64_t, const wchar_t *, ...);
#endif
typedef uint32_t (*GetSnortInstance)(void);

#define STD_BUF 1024

#ifndef DECODE_BLEN
#define DECODE_BLEN 65535
#define MAX_URI 8192
/* must be defined the same as in detection_util.h */
typedef enum
{
    HTTP_BUFFER_NONE,
    HTTP_BUFFER_URI,
    HTTP_BUFFER_HEADER,
    HTTP_BUFFER_CLIENT_BODY,
    HTTP_BUFFER_METHOD,
    HTTP_BUFFER_COOKIE,
    HTTP_BUFFER_STAT_CODE,
    HTTP_BUFFER_STAT_MSG,
    HTTP_BUFFER_RAW_URI,
    HTTP_BUFFER_RAW_HEADER,
    HTTP_BUFFER_RAW_COOKIE,
    HTTP_BUFFER_MAX
} HTTP_BUFFER;
#endif

/* Reasons for packet verdict indicating snort/preprocessor module that blocks a packet.
 * Any change here should also be reflected in pkt_trace.c file. */
typedef enum {
    VERDICT_REASON_INFO,         /* Not blocking packet; used by module to send packet information to tracer */
    VERDICT_REASON_SSNSTR,       /* Not blocking packet; used to update session debug info */
    VERDICT_REASON_NO_BLOCK,     /* Not blocking packet; all enum defined after this indicates blocking */
    VERDICT_REASON_UNKNOWN,      /* Module blocking the packet is not known or traced */
    VERDICT_REASON_DAQRETRY,     /* Wait for another re-transmitted packet from DAQ */
    VERDICT_REASON_SNORT,
    VERDICT_REASON_APPID,
    VERDICT_REASON_SFSSL,
    VERDICT_REASON_FIREWALL,
    VERDICT_REASON_CPORTAL,
    VERDICT_REASON_SAFESEARCH,
    VERDICT_REASON_SI,
    VERDICT_REASON_PREFILTER,
    VERDICT_REASON_FTP,
    VERDICT_REASON_STREAM,
    VERDICT_REASON_SESSION,
    VERDICT_REASON_DEFRAG,
    VERDICT_REASON_REACT,
    VERDICT_REASON_RESPONSE,
    VERDICT_REASON_REPUTATION,
    VERDICT_REASON_XLINK2STATE,
    VERDICT_REASON_BO,
    VERDICT_REASON_SMB,
    VERDICT_REASON_FILE,
    VERDICT_REASON_IPS,
    VERDICT_REASON_FASTBLOCK,

    MAX_VERDICT_REASON
} Verdict_Reason;

typedef struct {
    const uint8_t *data;
    uint16_t len;
} SFDataPointer;

typedef struct {
    uint8_t data[DECODE_BLEN];
    uint16_t len;
} SFDataBuffer;

typedef void (*LogMsgFunc)(const char *, ...);
typedef void (*DebugMsgFunc)(uint64_t, const char *, ...);
typedef int (*GetAltDetectFunc)(uint8_t **, uint16_t *);
typedef void (*SetAltDetectFunc)(uint8_t *,uint16_t );
typedef int (*IsDetectFlagFunc)(SFDetectFlagType);
typedef void (*DetectFlagDisableFunc)(SFDetectFlagType);
typedef void (*SetHttpBufferFunc)(HTTP_BUFFER, const uint8_t*, unsigned);
typedef const uint8_t* (*GetHttpBufferFunc)(HTTP_BUFFER, unsigned*);

#endif /* _SF_DYNAMIC_COMMON_H_ */
