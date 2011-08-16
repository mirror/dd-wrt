/****************************************************************************
 *
 * Copyright (C) 2003-2011 Sourcefire, Inc.
 *
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
 ****************************************************************************/
 
/**
**  @file       hi_client_norm.c
**  
**  @author     Daniel Roelker <droelker@sourcefire.com>
**  
**  @brief      HTTP client normalization routines
**  
**  We deal with the normalization of HTTP client requests headers and 
**  URI.
**  
**  In this file, we handle all the different HTTP request URI evasions.  The
**  list is:
**      - ASCII decoding
**      - UTF-8 decoding
**      - IIS Unicode decoding
**      - Directory traversals (self-referential and traversal)
**      - Multiple Slashes
**      - Double decoding
**      - %U decoding
**      - Bare Byte Unicode decoding
**      - Base36 decoding
**  
**  NOTES:
**      - Initial development.  DJR
*/
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <ctype.h>

#include "hi_norm.h"
#include "hi_util.h"
#include "hi_return_codes.h"
#include "hi_eo_log.h"

#include "bounds.h"
#include "detection_util.h"

#define MAX_URI 4096

extern int hi_split_header_cookie(HI_SESSION *, u_char *, int *, u_char *, int *, const u_char *, int , COOKIE_PTR *);

int hi_server_norm(HI_SESSION *Session, HttpSessionData *hsd)
{
    static u_char HeaderBuf[MAX_URI];
    static u_char CookieBuf[MAX_URI];
    static u_char RawHeaderBuf[MAX_URI];
    static u_char RawCookieBuf[MAX_URI];
    HI_SERVER_RESP    *ServerResp;
    int iRet;
    int iRawHeaderBufSize = MAX_URI;
    int iRawCookieBufSize = MAX_URI;
    int iHeaderBufSize = MAX_URI;
    int iCookieBufSize = MAX_URI;
    uint16_t encodeType = 0;

    if(!Session || !Session->server_conf)
    {
        return HI_INVALID_ARG;
    }

    ServerResp = &Session->server.response;
    ServerResp->header_encode_type = 0;
    ServerResp->cookie_encode_type = 0;

    if (ServerResp->cookie.cookie)
    {
        /* There is an HTTP header with a cookie, look for the cookie &
         * separate the two buffers */
        iRet = hi_split_header_cookie(Session,
            RawHeaderBuf, &iRawHeaderBufSize,
            RawCookieBuf, &iRawCookieBufSize,
            ServerResp->header_raw, ServerResp->header_raw_size,
            &ServerResp->cookie);
        if( iRet == HI_SUCCESS)
        {
             ServerResp->cookie.cookie = RawCookieBuf;
             ServerResp->cookie.cookie_end = RawCookieBuf + iRawCookieBufSize;
        }
    }
    else
    {
        if (ServerResp->header_raw_size)
        {
            if (ServerResp->header_raw_size > MAX_URI)
            {
                ServerResp->header_raw_size = MAX_URI;
            }
            /* Limiting to MAX_URI above should cause this to always return SAFEMEM_SUCCESS */
            SafeMemcpy(RawHeaderBuf, ServerResp->header_raw, ServerResp->header_raw_size,
                    &RawHeaderBuf[0], &RawHeaderBuf[0] + iRawHeaderBufSize);
        }
        iRawHeaderBufSize = ServerResp->header_raw_size;
        iRawCookieBufSize = 0;
    }

    if(ServerResp->header_norm && Session->server_conf->normalize_headers)
    {
        Session->norm_flags &= ~HI_BODY;
        iRet = hi_norm_uri(Session, HeaderBuf, &iHeaderBufSize, 
                       RawHeaderBuf, iRawHeaderBufSize, &encodeType);
        if (iRet == HI_NONFATAL_ERR)
        {
            /* There was a non-fatal problem normalizing */
            ServerResp->header_norm = NULL;
            ServerResp->header_norm_size = 0;
            ServerResp->header_encode_type = 0;
        }
        else 
        {
            /* Client code is expecting these to be set to non-NULL if 
             * normalization occurred. */
            ServerResp->header_norm      = HeaderBuf;
            ServerResp->header_norm_size = iHeaderBufSize;
            ServerResp->header_encode_type = encodeType;
        }
        encodeType = 0;
    }
    else
    {
        /* Client code is expecting these to be set to non-NULL if 
         * normalization occurred. */
        if (iRawHeaderBufSize)
        {
            ServerResp->header_norm      = RawHeaderBuf;
            ServerResp->header_norm_size = iRawHeaderBufSize;
            ServerResp->header_encode_type = 0;
        }
    }

    if(ServerResp->cookie.cookie && Session->server_conf->normalize_cookies)
    {
        Session->norm_flags &= ~HI_BODY;
        iRet = hi_norm_uri(Session, CookieBuf, &iCookieBufSize, 
                       RawCookieBuf, iRawCookieBufSize, &encodeType);
        if (iRet == HI_NONFATAL_ERR)
        {
            /* There was a non-fatal problem normalizing */
            ServerResp->cookie_norm = NULL;
            ServerResp->cookie_norm_size = 0;
            ServerResp->cookie_encode_type = 0;
        }
        else 
        {
            /* Client code is expecting these to be set to non-NULL if 
             * normalization occurred. */
            ServerResp->cookie_norm      = CookieBuf;
            ServerResp->cookie_norm_size = iCookieBufSize;
            ServerResp->cookie_encode_type = encodeType;
        }
        encodeType = 0;
    }
    else
    {
        /* Client code is expecting these to be set to non-NULL if 
         * normalization occurred. */
        if (iRawCookieBufSize)
        {
            ServerResp->cookie_norm      = RawCookieBuf;
            ServerResp->cookie_norm_size = iRawCookieBufSize;
            ServerResp->cookie_encode_type = 0;
        }
    }

    if (Session->server_conf->normalize_utf && (ServerResp->body_size > 0))
    {
        int bytes_copied, result, charset;

        if (hsd)
        {
            charset = get_decode_utf_state_charset(&(hsd->utf_state));

            /* Normalize server responses with utf-16le, utf-16be, utf-32le,
               or utf-32be charsets.*/
            if ((charset != CHARSET_DEFAULT) && (charset != CHARSET_UTF7))
            {
                result = DecodeUTF((char *)ServerResp->body, ServerResp->body_size,
                                   (char *)DecodeBuffer.data, sizeof(DecodeBuffer.data),
                                   &bytes_copied,
                                   &(hsd->utf_state));

                if (result == DECODE_UTF_FAILURE)
                {
                    if(hi_eo_generate_event(Session, HI_EO_SERVER_UTF_NORM_FAIL))
                    {
                        hi_eo_server_event_log(Session, HI_EO_SERVER_UTF_NORM_FAIL, NULL, NULL);
                    }
                }
                DecodeBuffer.len = bytes_copied;
            }
        }
    }


    return HI_SUCCESS;
}
