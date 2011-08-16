/*
 ** Copyright (C) 2002-2011 Sourcefire, Inc.
 ** Copyright (C) 1998-2002 Martin Roesch <roesch@sourcefire.com>
 **
 ** This program is free software; you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License Version 2 as
 ** published by the Free Software Foundation.  You may not use, modify or
 ** distribute this program under any other version of the GNU General
 ** Public License.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program; if not, write to the Free Software
 ** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 ** 
 ** Description
 ** This file contains the utility functions used by rule options.
 **
 */

#ifndef __DETECTION_UTIL_H__
#define __DETECTION_UTIL_H__

#include "sf_types.h"
#include "debug.h"

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

#define DOE_BUF_URI     0x01
#define DOE_BUF_STD     0x02

#define HTTPURI_PIPELINE_REQ 0x01

#define HTTP_ENCODE_TYPE__UTF8_UNICODE   0x00000001
#define HTTP_ENCODE_TYPE__DOUBLE_ENCODE  0x00000002
#define HTTP_ENCODE_TYPE__NONASCII       0x00000004
#define HTTP_ENCODE_TYPE__BASE36         0x00000008
#define HTTP_ENCODE_TYPE__UENCODE        0x00000010
#define HTTP_ENCODE_TYPE__BARE_BYTE      0x00000020
#define HTTP_ENCODE_TYPE__IIS_UNICODE    0x00000040
#define HTTP_ENCODE_TYPE__ASCII          0x00000080

typedef struct _HttpUri
{ 
    const uint8_t *uri;
    uint16_t length; 
    uint32_t encode_type;
} HttpUri;

typedef struct {
    uint8_t data[DECODE_BLEN];
    uint16_t len;
} DataBuffer;
    
extern const uint8_t *file_data_ptr;

extern uint8_t base64_decode_buf[DECODE_BLEN];
extern uint32_t base64_decode_size;

extern uint32_t mime_decode_size;
extern uint8_t mime_present;

extern uint8_t doe_buf_flags;
extern const uint8_t *doe_ptr;

extern HttpUri UriBufs[HTTP_BUFFER_MAX];
extern DataBuffer DecodeBuffer;

#ifdef DEBUG
const char* uri_buffer_name[HTTP_BUFFER_MAX];
#endif

#define SetDetectLimit(pktPtr, altLen) \
{ \
    pktPtr->alt_dsize = altLen; \
}

#define SetAltDecode(pktPtr, altLen) \
{ \
    pktPtr->packet_flags |= PKT_ALT_DECODE; \
    DecodeBuffer.len = altLen; \
}
#define IsLimitedDetect(pktPtr) (pktPtr->packet_flags & PKT_HTTP_DECODE)
/*
 * Function: setFileDataPtr
 *
 * Purpose: Sets the file data pointer used by 
 *          file_data rule option.
 *
 * Arguments: ptr => pointer to the body data
 *
 * Returns: void
 *
 */

static INLINE void setFileDataPtr(const u_char *ptr, uint32_t decode_size)
{
    file_data_ptr = ptr;
    mime_decode_size = decode_size;

}

/*
 * Function: IsBase64DecodeBuf
 *
 * Purpose: Checks if there is base64 decoded buffer.
 *
 * Arguments: p => doe_ptr
 *
 * Returns: Returns 1 if there is base64 decoded data
 *          and if the doe_ptr is within the buffer.
 *          Returns 0 otherwise.
 *
 */

static INLINE int IsBase64DecodeBuf(const uint8_t *p)
{
    if( base64_decode_size && p )
    {
        if ((p >= base64_decode_buf) && 
                (p < (base64_decode_buf + base64_decode_size)))
        {
            return 1;
        }
        else
            return 0;
    }
    else
        return 0;
}

static INLINE int IsMimeDecodeBuf(const uint8_t *p)
{
    if( mime_present && file_data_ptr)
    {
        if ((p >= file_data_ptr) &&
                (p < (file_data_ptr + mime_decode_size)))
        {
            return 1;
        }
        else
            return 0;
    }
    else
        return 0;
}

/*
 * Function: SetDoePtr(const uint8_t *ptr, uint8_t type)
 *
 * Purpose: This function set the doe_ptr and sets the type of
 *          buffer to which doe_ptr points.
 * 
 * Arguments: ptr       => pointer
 *            type      => type of buffer
 * 
 * Returns: void
 *
*/

static INLINE void SetDoePtr(const uint8_t *ptr, uint8_t type)
{
    doe_ptr = ptr;
    doe_buf_flags = type;
}

/*
 * Function: UpdateDoePtr(const uint8_t *ptr, uint8_t update)
 *
 * Purpose: This function updates the doe_ptr and resets the type of
 *          buffer to which doe_ptr points based on the update value.
 * 
 * Arguments: ptr       => pointer
 *            update    => reset the buf flag if update is not zero.
 * 
 * Returns: void
 *
*/

static INLINE void UpdateDoePtr(const uint8_t *ptr, uint8_t update)
{
    doe_ptr = ptr;
    if(update)
        doe_buf_flags = DOE_BUF_STD;
}

static INLINE void DetectReset(void)
{
    file_data_ptr  = NULL;
    base64_decode_size = 0;
    doe_buf_flags = 0;
    mime_decode_size = 0;
    mime_present = 0;
    DecodeBuffer.len = 0;
}

#endif

