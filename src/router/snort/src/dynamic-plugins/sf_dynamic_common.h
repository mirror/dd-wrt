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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Copyright (C) 2005-2011 Sourcefire, Inc.
 *
 */
#ifndef _SF_DYNAMIC_COMMON_H_
#define _SF_DYNAMIC_COMMON_H_

#ifndef WIN32
#include <sys/types.h>
#else
#include <stdint.h>
#endif

typedef void (*LogMsgFunc)(const char *, ...);
typedef void (*DebugMsgFunc)(int, char *, ...);
#ifdef HAVE_WCHAR_H
typedef void (*DebugWideMsgFunc)(int, wchar_t *, ...);
#endif

#define STD_BUF 1024

#ifndef DECODE_BLEN
#define DECODE_BLEN 65535
typedef enum
{
    HTTP_BUFFER_URI,
    HTTP_BUFFER_RAW_URI,
    HTTP_BUFFER_HEADER,
    HTTP_BUFFER_RAW_HEADER,
    HTTP_BUFFER_CLIENT_BODY,
    HTTP_BUFFER_METHOD,
    HTTP_BUFFER_COOKIE,
    HTTP_BUFFER_RAW_COOKIE,
    HTTP_BUFFER_STAT_CODE,
    HTTP_BUFFER_STAT_MSG,
    HTTP_BUFFER_MAX
} HTTP_BUFFER;
#endif

typedef struct _UriInfo
{
    uint8_t *uriBuffer;
    uint16_t uriLength;
    uint32_t uriDecodeFlags;

} UriInfo;

typedef struct {
    uint8_t data[DECODE_BLEN];
    uint16_t len;
} SFDataBuffer;

#define SetAltBuffer(pktPtr, altLen) \
{ \
    pktPtr->flags |= FLAG_ALT_DECODE; \
    _dpd.altBuffer->len = altLen; \
}

#define ResetAltBuffer(pktPtr) \
{ \
    pktPtr->flags &= ~FLAG_ALT_DECODE; \
    _dpd.altBuffer->len = 0; \
}

#endif /* _SF_DYNAMIC_COMMON_H_ */
