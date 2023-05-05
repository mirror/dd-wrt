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

#ifndef __HI_EO_EVENTS_H__
#define __HI_EO_EVENTS_H__

#include "hi_include.h"

/*
**  Client Events
*/
typedef enum _HI_CLI_EVENTS 
{
    HI_EO_CLIENT_ASCII =       0,
    HI_EO_CLIENT_DOUBLE_DECODE  , 
    HI_EO_CLIENT_U_ENCODE       , 
    HI_EO_CLIENT_BARE_BYTE      , 
    /* Base36 is deprecated - leave here so events keep the same number */
    HI_EO_CLIENT_BASE36         ,
    HI_EO_CLIENT_UTF_8          , 
    HI_EO_CLIENT_IIS_UNICODE    , 
    HI_EO_CLIENT_MULTI_SLASH    , 
    HI_EO_CLIENT_IIS_BACKSLASH  , 
    HI_EO_CLIENT_SELF_DIR_TRAV  , 
    HI_EO_CLIENT_DIR_TRAV       ,
    HI_EO_CLIENT_APACHE_WS      ,
    HI_EO_CLIENT_IIS_DELIMITER  ,
    HI_EO_CLIENT_NON_RFC_CHAR   ,
    HI_EO_CLIENT_OVERSIZE_DIR   ,
    HI_EO_CLIENT_LARGE_CHUNK    ,
    HI_EO_CLIENT_PROXY_USE      ,
    HI_EO_CLIENT_WEBROOT_DIR    ,
    HI_EO_CLIENT_LONG_HDR       ,
    HI_EO_CLIENT_MAX_HEADERS    ,
    HI_EO_CLIENT_MULTIPLE_CONTLEN,
    HI_EO_CLIENT_CHUNK_SIZE_MISMATCH,
    HI_EO_CLIENT_INVALID_TRUEIP ,
    HI_EO_CLIENT_MULTIPLE_HOST_HDRS,
    HI_EO_CLIENT_LONG_HOSTNAME  ,
    HI_EO_CLIENT_EXCEEDS_SPACES ,
    HI_EO_CLIENT_CONSECUTIVE_SMALL_CHUNKS,
    HI_EO_CLIENT_UNBOUNDED_POST,
    HI_EO_CLIENT_MULTIPLE_TRUEIP_IN_SESSION,
    HI_EO_CLIENT_BOTH_TRUEIP_XFF_HDRS,
    HI_EO_CLIENT_UNKNOWN_METHOD,
    HI_EO_CLIENT_SIMPLE_REQUEST,
    HI_EO_CLIENT_UNESCAPED_SPACE_URI,
    HI_EO_CLIENT_PIPELINE_MAX,
    HI_EO_CLIENT_MULTIPLE_COLON_BETN_KEY_VALUE,
    HI_EO_CLIENT_INVALID_RANGE_UNIT_FMT,
    HI_EO_CLIENT_RANGE_NON_GET_METHOD,
    HI_EO_CLIENT_RANGE_FIELD_ERROR,   
    HI_EO_CLIENT_EVENT_NUM
} HI_CLI_EVENTS;

typedef enum _HI_EVENTS
{
    HI_EO_ANOM_SERVER =         0,
    HI_EO_SERVER_INVALID_STATCODE,
    HI_EO_SERVER_NO_CONTLEN,
    HI_EO_SERVER_UTF_NORM_FAIL,
    HI_EO_SERVER_UTF7,
    HI_EO_SERVER_DECOMPR_FAILED,
    HI_EO_SERVER_CONSECUTIVE_SMALL_CHUNKS,
    HI_EO_CLISRV_MSG_SIZE_EXCEPTION,
    HI_EO_SERVER_JS_OBFUSCATION_EXCD,
    HI_EO_SERVER_JS_EXCESS_WS,
    HI_EO_SERVER_MIXED_ENCODINGS,
    HI_EO_SERVER_SWF_ZLIB_FAILURE,
    HI_EO_SERVER_SWF_LZMA_FAILURE,
    HI_EO_SERVER_PDF_DEFL_FAILURE,
    HI_EO_SERVER_PDF_UNSUP_COMP_TYPE,
    HI_EO_SERVER_PDF_CASC_COMP,
    HI_EO_SERVER_PDF_PARSE_FAILURE,
    HI_EO_SERVER_PROTOCOL_OTHER,
    HI_EO_SERVER_MULTIPLE_CONTLEN,
    HI_EO_SERVER_MULTIPLE_CONTENT_ENCODING,
    HI_EO_SERVER_MULTIPLE_COLON_BETN_KEY_VALUE,
    HI_EO_SERVER_INVALID_CHAR_BETN_KEY_VALUE,
    HI_EO_CLISRV_INVALID_CHUNKED_ENCODING,
    HI_EO_SERVER_PARTIAL_DECOMPRESSION_FAIL,
    HI_EO_SERVER_INVALID_HEADER_FOLDING,
    HI_EO_SERVER_JUNK_LINE_BEFORE_RESP_HEADER,
    HI_EO_SERVER_NO_RESP_HEADER_END,
    HI_EO_SERVER_INVALID_CHUNK_SIZE,
    HI_EO_SERVER_INVALID_VERSION_RESP_HEADER,
    HI_EO_SERVER_INVALID_CONTENT_RANGE_UNIT_FMT,
    HI_EO_SERVER_RANGE_FIELD_ERROR,
    HI_EO_SERVER_NON_RANGE_GET_PARTIAL_METHOD,
    HI_EO_SERVER_EVENT_NUM
}HI_EVENTS;

/*
**  These defines are the alert names for each event
*/
#define HI_EO_CLIENT_ASCII_STR                          \
    "(http_inspect) ASCII ENCODING"
#define HI_EO_CLIENT_DOUBLE_DECODE_STR                  \
    "(http_inspect) DOUBLE DECODING ATTACK"
#define HI_EO_CLIENT_U_ENCODE_STR                       \
    "(http_inspect) U ENCODING"
#define HI_EO_CLIENT_BARE_BYTE_STR                      \
    "(http_inspect) BARE BYTE UNICODE ENCODING"
/* Base36 is deprecated - leave here so events keep the same number */
#define HI_EO_CLIENT_BASE36_STR                         \
    "(http_inspect) BASE36 ENCODING"
#define HI_EO_CLIENT_UTF_8_STR                          \
    "(http_inspect) UTF-8 ENCODING"
#define HI_EO_CLIENT_IIS_UNICODE_STR                    \
    "(http_inspect) IIS UNICODE CODEPOINT ENCODING"
#define HI_EO_CLIENT_MULTI_SLASH_STR                    \
    "(http_inspect) MULTI_SLASH ENCODING"
#define HI_EO_CLIENT_IIS_BACKSLASH_STR                  \
    "(http_inspect) IIS BACKSLASH EVASION"
#define HI_EO_CLIENT_SELF_DIR_TRAV_STR                  \
    "(http_inspect) SELF DIRECTORY TRAVERSAL"
#define HI_EO_CLIENT_DIR_TRAV_STR                       \
    "(http_inspect) DIRECTORY TRAVERSAL"
#define HI_EO_CLIENT_APACHE_WS_STR                      \
    "(http_inspect) APACHE WHITESPACE (TAB)"
#define HI_EO_CLIENT_IIS_DELIMITER_STR                  \
    "(http_inspect) NON-RFC HTTP DELIMITER"
#define HI_EO_CLIENT_NON_RFC_CHAR_STR                   \
    "(http_inspect) NON-RFC DEFINED CHAR"
#define HI_EO_CLIENT_OVERSIZE_DIR_STR                   \
    "(http_inspect) OVERSIZE REQUEST-URI DIRECTORY"
#define HI_EO_CLIENT_LARGE_CHUNK_STR                    \
    "(http_inspect) OVERSIZE CHUNK ENCODING"
#define HI_EO_CLIENT_PROXY_USE_STR                      \
    "(http_inspect) UNAUTHORIZED PROXY USE DETECTED"
#define HI_EO_CLIENT_WEBROOT_DIR_STR                    \
    "(http_inspect) WEBROOT DIRECTORY TRAVERSAL"
#define HI_EO_CLIENT_LONG_HDR_STR                       \
    "(http_inspect) LONG HEADER"
#define HI_EO_CLIENT_MAX_HEADERS_STR                    \
    "(http_inspect) MAX HEADER FIELDS"
#define HI_EO_CLIENT_MULTIPLE_CONTLEN_STR               \
    "(http_inspect) MULTIPLE CONTENT LENGTH"
#define HI_EO_CLIENT_CHUNK_SIZE_MISMATCH_STR            \
    "(http_inspect) CHUNK SIZE MISMATCH DETECTED"
#define HI_EO_CLIENT_MULTIPLE_HOST_HDRS_STR             \
    "(http_inspect) MULTIPLE HOST HDRS DETECTED"
#define HI_EO_CLIENT_INVALID_TRUEIP_STR                 \
    "(http_inspect) INVALID IP IN TRUE-CLIENT-IP/XFF HEADER"
#define HI_EO_CLIENT_LONG_HOSTNAME_STR                  \
    "(http_inspect) HOSTNAME EXCEEDS 255 CHARACTERS"
#define HI_EO_CLIENT_EXCEEDS_SPACES_STR                 \
    "(http_inspect) HEADER PARSING SPACE SATURATION"
#define HI_EO_CLIENT_CONSECUTIVE_SMALL_CHUNKS_STR       \
    "(http_inspect) CLIENT CONSECUTIVE SMALL CHUNK SIZES"
#define HI_EO_CLIENT_UNBOUNDED_POST_STR                 \
    "(http_inspect) POST W/O CONTENT-LENGTH OR CHUNKS"
#define HI_EO_CLIENT_MULTIPLE_TRUEIP_IN_SESSION_STR     \
    "(http_inspect) MULTIPLE TRUE IPS IN A SESSION"
#define HI_EO_CLIENT_BOTH_TRUEIP_XFF_HDRS_STR           \
    "(http_inspect) BOTH TRUE_CLIENT_IP AND XFF HDRS PRESENT"
#define HI_EO_CLIENT_UNKNOWN_METHOD_STR                 \
    "(http_inspect) UNKNOWN METHOD"
#define HI_EO_CLIENT_SIMPLE_REQUEST_STR                 \
    "(http_inspect) SIMPLE REQUEST"
#define HI_EO_CLIENT_UNESCAPED_SPACE_URI_STR            \
    "(http_inspect) UNESCAPED SPACE IN HTTP URI"
#define HI_EO_CLIENT_PIPELINE_MAX_STR                   \
    "(http_inspect) TOO MANY PIPELINED REQUESTS"
#define HI_EO_CLIENT_MULTIPLE_COLON_BETN_KEY_VALUE_STR  \
    "(http_inspect) MULTIPLE COLON BETWEEN KEY AND VALUE IN HTTP REQUEST HEADER"
#define HI_EO_CLIENT_INVALID_RANGE_UNIT_FMT_STR         \
    "(http_inspect) INVALID RANGE UNIT FORMAT"
#define HI_EO_CLIENT_RANGE_NON_GET_METHOD_STR           \
    "(http_inspect) RANGE FIELD PRESENT IN NON GET METHOD"
#define HI_EO_CLIENT_RANGE_FIELD_ERROR_STR              \
    "(http_inspect) ERROR IN RANGE FIELD OF REQUEST HEADER"

/*
**  Server Events
*/

#define HI_EO_ANOM_SERVER_STR                           \
    "(http_inspect) ANOMALOUS HTTP SERVER ON UNDEFINED HTTP PORT"
#define HI_EO_SERVER_INVALID_STATCODE_STR               \
    "(http_inspect) INVALID STATUS CODE IN HTTP RESPONSE"
#define HI_EO_SERVER_NO_CONTLEN_STR                     \
    "(http_inspect) NO CONTENT-LENGTH OR TRANSFER-ENCODING IN HTTP RESPONSE"
#define HI_EO_SERVER_UTF_NORM_FAIL_STR                  \
    "(http_inspect) HTTP RESPONSE HAS UTF CHARSET WHICH FAILED TO NORMALIZE"
#define HI_EO_SERVER_UTF7_STR                           \
    "(http_inspect) HTTP RESPONSE HAS UTF-7 CHARSET"
#define HI_EO_SERVER_DECOMPR_FAILED_STR                 \
    "(http_inspect) HTTP RESPONSE GZIP DECOMPRESSION FAILED"
#define HI_EO_SERVER_CONSECUTIVE_SMALL_CHUNKS_STR       \
    "(http_inspect) SERVER CONSECUTIVE SMALL CHUNK SIZES"
#define HI_EO_CLISRV_MSG_SIZE_EXCEPTION_STR             \
    "(http_inspect) INVALID CONTENT-LENGTH OR CHUNK SIZE"
#define HI_EO_SERVER_JS_OBFUSCATION_EXCD_STR            \
    "(http_inspect) JAVASCRIPT OBFUSCATION LEVELS EXCEEDS 1"
#define HI_EO_SERVER_JS_EXCESS_WS_STR                   \
    "(http_inspect) JAVASCRIPT WHITESPACES EXCEEDS MAX ALLOWED"
#define HI_EO_SERVER_MIXED_ENCODINGS_STR                \
    "(http_inspect) MULTIPLE ENCODINGS WITHIN JAVASCRIPT OBFUSCATED DATA"
#define HI_EO_SERVER_SWF_ZLIB_FAILURE_STR               \
    "(http_inspect) HTTP_RESPONSE SWF FILE ZLIB DECOMPRESSION FAILURE"
#define HI_EO_SERVER_SWF_LZMA_FAILURE_STR               \
    "(http_inspect) HTTP_RESPONSE SWF FILE LZMA DECOMPRESSION FAILURE"
#define HI_EO_SERVER_PDF_DEFL_FAILURE_STR               \
    "(http_inspect) HTTP_RESPONSE PDF FILE DEFLATE DECOMPRESSION FAILURE"
#define HI_EO_SERVER_PDF_UNSUP_COMP_TYPE_STR            \
    "(http_inspect) HTTP_RESPONSE PDF FILE UNSUPPORTED COMPRESSION TYPE"
#define HI_EO_SERVER_PDF_CASC_COMP_STR                  \
    "(http_inspect) HTTP_RESPONSE PDF FILE CASCADED COMPRESSION"
#define HI_EO_SERVER_PDF_PARSE_FAILURE_STR              \
    "(http_inspect) HTTP_RESPONSE PDF FILE PARSE FAILURE"
#define HI_EO_SERVER_PROTOCOL_OTHER_STR			\
    "(http_inspect) PROTOCOL-OTHER HTTP server response before client request "
#define HI_EO_SERVER_MULTIPLE_CONTLEN_STR               \
    "(http_inspect) MULTIPLE CONTENT LENGTH IN HTTP RESPONSE"
#define HI_EO_SERVER_MULTIPLE_CONTENT_ENCODING_STR      \
    "(http_inspect) MULTIPLE CONTENT ENCODING IN HTTP RESPONSE"
#define HI_EO_SERVER_MULTIPLE_COLON_BETN_KEY_VALUE_STR  \
    "(http_inspect) MULTIPLE COLON BETWEEN KEY AND VALUE IN HTTP RESPONSE HEADER"
#define HI_EO_SERVER_INVALID_CHAR_BETN_KEY_VALUE_STR  \
    "(http_inspect) INVALID CHARACTER BETWEEN KEY AND VALUE IN HTTP RESPONSE HEADER"
#define HI_EO_CLISRV_INVALID_CHUNKED_EXCEPTION_STR \
    "(http_inspect) TRANSFER ENCODING:CHUNKED IN HTTP 1.0 REQUEST/RESPONSE HEADER"
#define HI_EO_SERVER_PARTIAL_DECOMPRESSION_FAIL_STR  \
    "(http_inspect) HTTP RESPONSE PARTIAL DECOMPRESSION FAILURE"
#define HI_EO_SERVER_INVALID_HEADER_FOLDING_STR \
    "(http_inspect) INVALID HEADER FOLDING"
#define HI_EO_SERVER_JUNK_LINE_BEFORE_RESP_HEADER_STR \
    "(http_inspect) JUNK LINE BEFORE HTTP RESPONSE HEADER"
#define HI_EO_SERVER_NO_RESP_HEADER_END_STR \
    "(http_inspect) NO END OF HEADER IN RESPONSE"
#define HI_EO_SERVER_INVALID_CHUNK_SIZE_STR \
    "(http_inspect) INVALID CHUNK SIZE OR CHUNK SIZE FOLLOWED BY JUNK CHARACTERS"
#define HI_EO_SERVER_INVALID_VERSION_RESP_HEADER_STR \
    "(http_inspect) INVALID VERSION IN HTTP RESPONSE HEADER"
#define HI_EO_SERVER_INVALID_CONTENT_RANGE_UNIT_FMT_STR \
    "(http_inspect) INVALID CONTENT RANGE UNIT FORMAT"
#define HI_EO_SERVER_RANGE_FIELD_ERROR_STR \
    "(http_inspect) ERROR IN RANGE FIELD OF RESPONSE HEADER"
#define HI_EO_SERVER_NON_RANGE_GET_PARTIAL_METHOD_STR   \
    "(http_inspect) RANGE FIELD NOT PRESENT IN GET METHOD, BUT RESPONSE WITH PARTIAL CONTENT"

/*
**  Event Priorities
*/
#define HI_EO_HIGH_PRIORITY 0
#define HI_EO_MED_PRIORITY  1
#define HI_EO_LOW_PRIORITY  2

#endif
