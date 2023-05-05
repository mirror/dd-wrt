/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2003-2013 Sourcefire, Inc.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 ****************************************************************************/

/**
**  @file       hi_server_norm.c
**
**  @author     Daniel Roelker <droelker@sourcefire.com>
**
**  @brief      HTTP Server normalization routines
**
**  We deal with the normalization of HTTP client requests headers and
**  URI.
**
**  In this file, we handle all the different HTTP request/response URI evasions.  The
**  list is:
**      - ASCII decoding
**      - UTF-8 decoding
**      - IIS Unicode decoding
**      - Directory traversals (self-referential and traversal)
**      - Multiple Slashes
**      - Double decoding
**      - %U decoding
**      - Bare Byte Unicode decoding
**
**      Base 36 is deprecated and essentially a noop
**      - Base36 decoding
**
**  NOTES:
**      - Initial development.  DJR
*/
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <ctype.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "hi_norm.h"
#include "hi_util.h"
#include "hi_return_codes.h"
#include "hi_eo_log.h"

#include "snort_bounds.h"
#include "detection_util.h"


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

            if (charset == CHARSET_UNKNOWN)
            {
                /* Got a text content type but no charset.
                 * Look for potential BOM (Byte Order Mark) */
                if (ServerResp->body_size >= 4)
                {
                    uint8_t size = 0;

                    if (!memcmp(ServerResp->body, "\x00\x00\xFE\xFF", 4))
                    {
                        charset = CHARSET_UTF32BE;
                        size = 4;
                    }
                    else if (!memcmp(ServerResp->body, "\xFF\xFE\x00\x00", 4))
                    {
                        charset = CHARSET_UTF32LE;
                        size = 4;
                    }
                    else if (!memcmp(ServerResp->body, "\xFE\xFF", 2))
                    {
                        charset = CHARSET_UTF16BE;
                        size = 2;
                    }
                    else if (!memcmp(ServerResp->body, "\xFF\xFE", 2))
                    {
                        charset = CHARSET_UTF16LE;
                        size = 2;
                    }

                    //  BOM (Byte Order Mark) was missing. Try to guess
                    //  the encoding.
                    else if (ServerResp->body[0]  == '\0' &&
                        ServerResp->body[2]  == '\0' &&
                        ServerResp->body[3])
                    {
                        if (ServerResp->body[1])
                            charset = CHARSET_UTF16BE;  // \0C\0C
                        else
                            charset = CHARSET_UTF32BE;  // \0\0\0C
                    }
                    else if (ServerResp->body[0] &&
                        ServerResp->body[1] == '\0' &&
                        ServerResp->body[3] == '\0')
                    {
                        if (ServerResp->body[2])
                            charset = CHARSET_UTF16LE;  // C\0C\0
                        else
                            charset = CHARSET_UTF32LE;  // C\0\0\0
                    }
                    else
                    {
                        //  NOTE: The UTF-8 BOM (Byte Order Mark) does not
                        //  match the above cases, so we end up here when
                        //  parsing UTF-8. That works out for the moment
                        //  because the first 128 characters of UTF-8 are
                        //  identical to ASCII. We may want to handle
                        //  other UTF-8 characters beyond 0x7f in the future.

                        charset = CHARSET_DEFAULT; // ensure we don't try again
                    }

                    // FIXIT-M We are not currently handling the case
                    // where some characters are not ASCII and
                    // some are ASCII. This is a problem because some
                    // UTF-16 characters have no NUL bytes (so won't
                    // be identified as UTF-16.)

                    // FIXIT-L We also do not handle multiple levels
                    // of encoding (where unicode becomes %u0020 for
                    // example).

                    ServerResp->body += size;
                    ServerResp->body_size -= size;
                }
                else
                    charset = CHARSET_DEFAULT; // ensure we don't try again

                set_decode_utf_state_charset(&(hsd->utf_state), charset);
            }

            /* Normalize server responses with utf-16le, utf-16be, utf-32le,
               or utf-32be charsets.*/
            switch (charset)
            {
                case CHARSET_UTF16LE:
                case CHARSET_UTF16BE:
                case CHARSET_UTF32LE:
                case CHARSET_UTF32BE:
                    result = DecodeUTF((char *)ServerResp->body, ServerResp->body_size,
                            (char *)HttpDecodeBuf.data, sizeof(HttpDecodeBuf.data),
                            &bytes_copied,
                            &(hsd->utf_state));

                    if (result == DECODE_UTF_FAILURE)
                    {
                        if(hi_eo_generate_event(Session, HI_EO_SERVER_UTF_NORM_FAIL))
                        {
                            hi_eo_server_event_log(Session, HI_EO_SERVER_UTF_NORM_FAIL, NULL, NULL);
                        }
                    }
                    SetHttpDecode((uint16_t)bytes_copied);
                    ServerResp->body = HttpDecodeBuf.data;
                    ServerResp->body_size = HttpDecodeBuf.len;
                    break;
                default:
                    break;
            }
        }
    }

    if (Session->server_conf->normalize_javascript && (ServerResp->body_size > 0))
    {
        int js_present, status, index;
        char *ptr, *start, *end;
        JSState js;

        js.allowed_spaces = Session->server_conf->max_js_ws;
        js.allowed_levels = MAX_ALLOWED_OBFUSCATION;
        js.alerts = 0;

        js_present = status = index = 0;
        start = (char *)ServerResp->body;
        ptr = start;
        end = start + ServerResp->body_size;
        
        while(ptr < end)
        {
            char *angle_bracket, *js_start;
            int type_js, bytes_copied, script_found;
            bytes_copied = 0;
            type_js = 0;
            hi_current_search = &hi_js_search[0];
            script_found = search_api->search_instance_find(hi_javascript_search_mpse, (const char *)ptr,
                                            (end-ptr), 0 , HI_SearchStrFound);
            if (script_found > 0)
            {
                js_start = ptr + hi_search_info.index;
                angle_bracket = (char *)SnortStrnStr((const char *)(js_start), (end - js_start), ">");
                if(!angle_bracket)
                    break;

                if(angle_bracket > js_start)
                {
                    script_found = search_api->search_instance_find(hi_htmltype_search_mpse, (const char *)js_start,
                                                               (angle_bracket-js_start), 0 , HI_SearchStrFound); 
                    js_start = angle_bracket;
                    if(script_found > 0)
                    {
                        switch (hi_search_info.id)
                        {
                            case HTML_JS:
                                js_present = 1;
                                type_js = 1;
                                break;
                            default:
                                type_js = 0;
                                break;
                        }
                    }
                    else
                    {
                        //if no type or language is found we assume its a javascript
                        js_present = 1;
                        type_js = 1;
                    }

                }
                //Save before the <script> begins
                if(js_start > ptr)
                {
                    status = SafeBoundsMemmove(HttpDecodeBuf.data+index, ptr, (js_start - ptr), HttpDecodeBuf.data, HttpDecodeBuf.data + sizeof(HttpDecodeBuf.data));
                    if(status == SAFEMEM_SUCCESS)
                        index += (js_start - ptr);
                    else
                        break;
                }

                ptr = js_start;
                if(!type_js)
                    continue;

                if(Session->server_conf->iis_unicode.on)
                {
                    JSNormalizeDecode(js_start, (uint16_t)(end-js_start), (char *)HttpDecodeBuf.data+index, (uint16_t)(sizeof(HttpDecodeBuf.data) - index), 
                            &ptr, &bytes_copied, &js, Session->server_conf->iis_unicode_map);
                }
                else
                {
                    JSNormalizeDecode(js_start, (uint16_t)(end-js_start), (char *)HttpDecodeBuf.data+index, (uint16_t)(sizeof(HttpDecodeBuf.data) - index), 
                                                    &ptr, &bytes_copied, &js, NULL);
                }
                index += bytes_copied;
            }
            else
                break;
        }

        if(js_present)    
        {
            if( ptr < end )
            {
                status = SafeBoundsMemmove(HttpDecodeBuf.data+index, ptr, (end - ptr), HttpDecodeBuf.data, HttpDecodeBuf.data + sizeof(HttpDecodeBuf.data));
                if(status == SAFEMEM_SUCCESS)
                    index += (end - ptr);
            }
            SetHttpDecode((uint16_t)index);
            ServerResp->body = HttpDecodeBuf.data;
            ServerResp->body_size = index;
            if(js.alerts) 
            {
                if((js.alerts & ALERT_LEVELS_EXCEEDED) && (hi_eo_generate_event(Session, HI_EO_SERVER_JS_OBFUSCATION_EXCD)))
                {
                    hi_eo_server_event_log(Session, HI_EO_SERVER_JS_OBFUSCATION_EXCD, NULL, NULL);
                }
                if( (js.alerts & ALERT_SPACES_EXCEEDED) && (hi_eo_generate_event(Session, HI_EO_SERVER_JS_EXCESS_WS)))
                {
                    hi_eo_server_event_log(Session, HI_EO_SERVER_JS_EXCESS_WS, NULL, NULL);
                }
                if((js.alerts & ALERT_MIXED_ENCODINGS) && (hi_eo_generate_event(Session, HI_EO_SERVER_MIXED_ENCODINGS)))
                {
                    hi_eo_server_event_log(Session, HI_EO_SERVER_MIXED_ENCODINGS, NULL, NULL);
                }
            }

            if(hsd)
                hsd->log_flags |= HTTP_LOG_JSNORM_DATA;
        }
    }

    return HI_SUCCESS;
}


/* This function assumes that only text input is passed to it always.
 * It parses the input buffer for randomized UTF encoded characters
 * and normalizes them to UTF 8  whether they are accompanied with BOM or not.
 * In effect, it boils down to removing nulls from the passed text.
 * It returns the number of valid bytes in the text after normalization.
 */
uint32_t NormalizeRandomNulls (u_char *src, uint32_t src_len, u_char *dst)
{
    uint16_t bytes = 0;

    while (src_len)
    {
        if(*src)
        {
            *dst++ = *src++;
            bytes++;
        }
        else
        {
            src++;
        }
        src_len--;
    }
    return bytes;
}

