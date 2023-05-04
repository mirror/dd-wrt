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
**  @file       hi_server.c
**
**  @author     Daniel Roelker <droelker@sourcefire.com>
**
**  @brief      Handles inspection of HTTP server responses.
**
**  HttpInspect handles server responses in a stateless manner because we
**  are really only interested in the first response packet that contains
**  the HTTP response code, headers, and the payload.
**
**  The first big thing is to incorporate the HTTP protocol flow
**  analyzer.
**
**  NOTES:
**      - Initial development.  DJR
*/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include "mempool.h"
#include "hi_paf.h"
extern MemPool *hi_gzip_mempool;
extern uint8_t decompression_buffer[];

extern uint8_t dechunk_buffer[];
static bool simple_response = false;

#include "hi_server.h"
#include "hi_ui_config.h"
#include "hi_return_codes.h"
#include "hi_si.h"
#include "hi_eo_log.h"
#include "snort_bounds.h"
#include "detection_util.h"
#include "stream_api.h"
#include "sfutil/util_unfold.h"
#include "memory_stats.h"

#if defined(FEAT_OPEN_APPID)
#include "spp_stream6.h"
#endif /* defined(FEAT_OPEN_APPID) */

#define STAT_END 100
#define HTTPRESP_HEADER_NAME__COOKIE "Set-Cookie"
#define HTTPRESP_HEADER_LENGTH__COOKIE 10
#define HTTPRESP_HEADER_NAME__CONTENT_ENCODING "Content-Encoding"
#define HTTPRESP_HEADER_LENGTH__CONTENT_ENCODING 16
#define HTTPRESP_HEADER_NAME__GZIP "gzip"
#define HTTPRESP_HEADER_NAME__XGZIP "x-gzip"
#define HTTPRESP_HEADER_LENGTH__GZIP 4
#define HTTPRESP_HEADER_LENGTH__XGZIP 6
#define HTTPRESP_HEADER_NAME__DEFLATE "deflate"
#define HTTPRESP_HEADER_LENGTH__DEFLATE 7
#define HTTPRESP_HEADER_NAME__CONTENT_LENGTH "Content-length"
#define HTTPRESP_HEADER_LENGTH__CONTENT_LENGTH 14
#define HTTPRESP_HEADER_NAME__CONTENT_TYPE "Content-Type"
#define HTTPRESP_HEADER_LENGTH__CONTENT_TYPE 12
#define HTTPRESP_HEADER_NAME__TRANSFER_ENCODING "Transfer-Encoding"
#define HTTPRESP_HEADER_LENGTH__TRANSFER_ENCODING 17
#define HTTPRESP_HEADER_NAME__CONTENT_RANGE "Content-Range"
#define HTTPRESP_HEADER_LENGTH__CONTENT_RANGE 13
#define HTTPRESP_HEADER_NAME__ACCEPT_RANGES "Accept-Ranges"
#define HTTPRESP_HEADER_LENGTH__ACCEPT_RANGES 13
#if defined(FEAT_OPEN_APPID)
#define HEADER_NAME__VIA "Via"
#define HEADER_LENGTH__VIA sizeof(HEADER_NAME__VIA)-1
#define HEADER_NAME__SERVER "Server"
#define HEADER_LENGTH__SERVER sizeof(HEADER_NAME__SERVER)-1
#define HEADER_NAME__X_WORKING_WITH "X-Working-With"
#define HEADER_LENGTH__X_WORKING_WITH sizeof(HEADER_NAME__X_WORKING_WITH)-1
#endif /* defined(FEAT_OPEN_APPID) */

extern fd_config_t hi_fd_conf;

typedef int (*LOOKUP_FCN)(HI_SESSION *, const u_char *, const u_char *, const u_char **,
            URI_PTR *);
extern LOOKUP_FCN lookup_table[256];
extern int NextNonWhiteSpace(HI_SESSION *, const u_char *, const u_char *, const u_char **, URI_PTR *);
extern int CheckChunkEncoding(HI_SESSION *, const u_char *, const u_char *, const u_char **, u_char *,
                              uint32_t , uint32_t, uint32_t *, uint32_t *, HttpSessionData *, int);
extern int IsHttpVersion(const u_char **, const u_char *);
extern int find_rfc_delimiter(HI_SESSION *, const u_char *, const u_char *, const u_char **, URI_PTR *);
extern int find_non_rfc_delimiter(HI_SESSION *, const u_char *, const u_char *, const u_char **, URI_PTR *);
extern int NextNonWhiteSpace(HI_SESSION *, const u_char *, const u_char *, const u_char **, URI_PTR *);
extern int SetPercentNorm(HI_SESSION *, const u_char *, const u_char *, const u_char **, URI_PTR *);
extern int SetSlashNorm(HI_SESSION *, const u_char *, const u_char *, const u_char **, URI_PTR *);
extern int SetBackSlashNorm(HI_SESSION *, const u_char *, const u_char *, const u_char **, URI_PTR *);
extern int SetPlusNorm(HI_SESSION *, const u_char *, const u_char *, const u_char **, URI_PTR *);
extern int SetBinaryNorm(HI_SESSION *, const u_char *, const u_char *, const u_char **, URI_PTR *);
extern int SetParamField(HI_SESSION *, const u_char *, const u_char *, const u_char **, URI_PTR *);
extern int SetProxy(HI_SESSION *, const u_char *, const u_char *, const u_char **, URI_PTR *);
extern const u_char *extract_http_cookie(const u_char *p, const u_char *end, HEADER_PTR *, HEADER_FIELD_PTR *);
extern const u_char *extract_http_content_length(HI_SESSION *, HTTPINSPECT_CONF *, const u_char *, const u_char *, const u_char *, HEADER_PTR *, HEADER_FIELD_PTR *, int) ;

#define CLR_SERVER_HEADER(Server) \
    do { \
            Server->response.header_raw = NULL;\
            Server->response.header_raw_size = 0;\
            Server->response.header_norm = NULL; \
            Server->response.header_norm_size = 0 ;\
            Server->response.cookie.cookie = NULL;\
            Server->response.cookie.cookie_end = NULL;\
            if(Server->response.cookie.next) {\
                COOKIE_PTR *cookie = Server->response.cookie.next; \
                do { \
                    Server->response.cookie.next = Server->response.cookie.next->next; \
                    SnortPreprocFree(cookie, sizeof(COOKIE_PTR), PP_HTTPINSPECT, \
                         PP_MEM_CATEGORY_SESSION); \
                    cookie = Server->response.cookie.next; \
                }while(cookie);\
            }\
            Server->response.cookie.next = NULL;\
            Server->response.cookie_norm = NULL;\
            Server->response.cookie_norm_size = 0;\
    } while(0);

#define CLR_SERVER_STAT(Server) \
    do { \
            Server->response.status_msg = NULL;\
            Server->response.status_code = NULL;\
            Server->response.status_code_size = 0;\
            Server->response.status_msg_size = 0;\
    }while(0);

#define CLR_SERVER_STAT_MSG(Server) \
    do { \
            Server->response.status_msg = NULL;\
            Server->response.status_msg_size = 0;\
        }while(0);

#define CLR_SERVER_BODY(Server)\
    do { \
            Server->response.body = NULL;\
            Server->response.body_size = 0;\
            Server->response.body_raw = NULL;\
            Server->response.body_raw_size = 0;\
    }while(0);

static inline void clearHttpRespBuffer(HI_SERVER *Server)
{
    CLR_SERVER_HEADER(Server);
    CLR_SERVER_STAT(Server);
    CLR_SERVER_BODY(Server);
}

static inline const u_char *MovePastDelims(const u_char *start, const u_char *end,const u_char *ptr)
{

    while(hi_util_in_bounds(start, end, ptr))
    {
       if(*ptr < 0x21)
        {
            if(*ptr < 0x0E && *ptr > 0x08)
            {
                ptr++;
                continue;
            }
            else
            {
                if(*ptr == 0x20)
                {
                    ptr++;
                    continue;
                }
            }
        }

        break;
    }

    return ptr;
}

void CheckSkipAlertMultipleColon(HI_SESSION *Session, const u_char *start, const u_char *end, const u_char **ptr, int iInspectMode)
{
    if (hi_util_in_bounds(start, end, *ptr) && **ptr == ':')
    {
        if (iInspectMode == HI_SI_SERVER_MODE && hi_eo_generate_event(Session, HI_EO_SERVER_MULTIPLE_COLON_BETN_KEY_VALUE))
        {
            hi_eo_server_event_log(Session, HI_EO_SERVER_MULTIPLE_COLON_BETN_KEY_VALUE, NULL, NULL);
        }
        else if (iInspectMode == HI_SI_CLIENT_MODE && hi_eo_generate_event(Session, HI_EO_CLIENT_MULTIPLE_COLON_BETN_KEY_VALUE))
        {
            hi_eo_client_event_log(Session, HI_EO_CLIENT_MULTIPLE_COLON_BETN_KEY_VALUE, NULL, NULL);
        }
        while (hi_util_in_bounds(start, end, *ptr) && **ptr == ':') (*ptr)++;
    }
}

/**
 ** CheckSkipAlertCharsBeforeColon:
 **
 ** This function skips any invalid characters between
 ** http response header and ':'.
 **/
static inline void CheckSkipAlertCharsBeforeColon(HI_SESSION *Session, const u_char *start,
                         const u_char *end, const u_char **ptr)
{
    bool invalid_char = false;

    while ( (hi_util_in_bounds(start, end, *ptr)) && (**ptr != ':')) {
        switch(**ptr) {
        case ' ':
	case '\t':
	    (*ptr)++;
            break;

	case '\0':
	case '\r':
	case '\n':
	case '\v':
	case '\f':
	    invalid_char = true;
	    (*ptr)++;
	    break;

        default:
	    return;
        }

	if (invalid_char &&
		hi_eo_generate_event(Session, HI_EO_SERVER_INVALID_CHAR_BETN_KEY_VALUE)) {
	    hi_eo_server_event_log(Session, HI_EO_SERVER_INVALID_CHAR_BETN_KEY_VALUE, NULL, NULL);
        }
    }
}

/**
**  NAME
**    IsHttpServerData::
*/
/**
**  Inspect an HTTP server response packet to determine the state.
**
**  We inspect this packet and determine whether we are in the beginning
**  of a response header or if we are looking at payload.  We limit the
**  amount of inspection done on responses by only inspecting the HTTP header
**  and some payload.  If the whole packet is a payload, then we just ignore
**  it, since we inspected the previous header and payload.
**
**  We limit the amount of the payload by adjusting the Server structure
**  members, header and header size.
**
**  @param Server      the server structure
**  @param data        pointer to the beginning of payload
**  @param dsize       the size of the payload
**  @param flow_depth  the amount of header and payload to inspect
**
**  @return integer
**
**  @retval HI_INVALID_ARG invalid argument
**  @retval HI_SUCCESS     function success
*/
static int IsHttpServerData(HI_SESSION *Session, Packet *p, HttpSessionData *sd)
{
    const u_char *start;
    const u_char *end;
    const u_char *ptr;
    int len;
    uint32_t seq_num = 0;
    HI_SERVER *Server;
    HTTPINSPECT_CONF *ServerConf;

    ServerConf = Session->server_conf;
    if(!ServerConf)
        return HI_INVALID_ARG;

    Server = &(Session->server);

    clearHttpRespBuffer(Server);
    /*
    ** HTTP:Server-Side-Session-Performance-Optimization
    ** This drops Server->Client packets which are not part of the
    ** HTTP Response header. It can miss part of the response header
    ** if the header is sent as multiple packets.
    */
    if(!(p->data))
    {
        return HI_INVALID_ARG;
    }

    seq_num = GET_PKT_SEQ(p);

    /*
    **  Let's set up the data pointers.
    */
    Server->response.header_raw      = p->data;
    Server->response.header_raw_size = p->dsize;

    start = p->data;
    end = p->data + p->dsize;
    ptr = start;

    ptr = MovePastDelims(start,end,ptr);

    len = end - ptr;
    if ( len > 4 )
    {
        if(!IsHttpVersion(&ptr, end))
        {
            p->packet_flags |= PKT_HTTP_DECODE;
            ApplyFlowDepth(ServerConf, p, sd, 0, 0, seq_num);
            return HI_SUCCESS;
        }
        else
        {
            if(ServerConf->server_flow_depth > 0)
            {
                if(sd)
                {
                    sd->resp_state.flow_depth_excd = false;
                    sd->resp_state.max_seq = seq_num + ServerConf->server_flow_depth;
                }
            }
            p->packet_flags |= PKT_HTTP_DECODE;
            ApplyFlowDepth(ServerConf, p, sd, 0, 0, seq_num);
            return HI_SUCCESS;
        }
    }
    else
    {
        return HI_SUCCESS;
    }


    return HI_SUCCESS;
}

static inline int hi_server_extract_status_msg( const u_char *start, const u_char *ptr,
        const u_char *end, URI_PTR *result)
{
    int iRet = HI_SUCCESS;
    SkipBlankSpace(start,end,&ptr);

    if (  hi_util_in_bounds(start, end, ptr) )
    {
        const u_char *crlf = (u_char *)SnortStrnStr((const char *)ptr, end - ptr, "\n");
        result->uri = ptr;
        if (crlf)
        {
            if(crlf[-1] == '\r')
                result->uri_end = crlf - 1;
            else
                result->uri_end = crlf;
            ptr = crlf;
        }
        else
        {
            result->uri_end =end;
        }

        if(result->uri < result->uri_end)
            iRet = STAT_END;
        else
            iRet = HI_OUT_OF_BOUNDS;
    }
    else
        iRet = HI_OUT_OF_BOUNDS;

    return iRet;
}


static inline int hi_server_extract_status_code(HI_SESSION *Session, const u_char *start, const u_char *ptr,
        const u_char *end, URI_PTR *result)
{
    int iRet = HI_SUCCESS;
    SkipBlankSpace(start,end,&ptr);

    result->uri = ptr;
    result->uri_end = ptr;

    while(  hi_util_in_bounds(start, end, ptr) )
    {
        if(isdigit((int)*ptr))
        {
            SkipDigits(start, end, &ptr);
            if (  hi_util_in_bounds(start, end, ptr) )
            {
                if(isspace((int)*ptr))
                {
                    result->uri_end = ptr;
                    iRet = STAT_END;
                    return iRet;
                }
                else
                {
                    result->uri_end = ptr;
                    iRet = HI_NONFATAL_ERR;
                    return iRet;
                }

            }
            else
            {
                iRet = HI_OUT_OF_BOUNDS;
                return iRet;
            }

        }
        else
        {

            if(hi_eo_generate_event(Session, HI_EO_SERVER_INVALID_STATCODE))
            {
                hi_eo_server_event_log(Session, HI_EO_SERVER_INVALID_STATCODE, NULL, NULL);
            }
            ptr++;
        }
    }

    iRet = HI_OUT_OF_BOUNDS;

    return iRet;
}

/* Given a string, removes header folding (\r\n followed by linear whitespace)
 * and exits when the end of a header is found, defined as \n followed by a
 * non-whitespace.  This is especially helpful for HTML.
 *  Returns -1 if the header is too long and cant normalise completely */

int sf_unfold_http_header(HI_SESSION *Session, const uint8_t *inbuf,
        uint32_t inbuf_size, uint8_t *outbuf,
        uint32_t outbuf_size, uint32_t *output_bytes,
        int *folded)
{
    int num_spaces = 0;
    bool line_folding = false;
    const uint8_t *cursor = NULL, *endofinbuf = NULL;
    uint8_t *outbuf_ptr = NULL;
    uint32_t n = 0;
    enum CursorState {CURSOR_STATE_NORMAL, CURSOR_STATE_NEWLINE};
    uint8_t state = CURSOR_STATE_NORMAL;
    cursor = inbuf;
    endofinbuf = inbuf + inbuf_size;
    outbuf_ptr = outbuf;

    /* Keep adding chars until we get to the end of the line.  If we get to the
     * end of the line and the next line starts with a tab or space, add the space
     * to the buffer and keep reading.  If the next line does not start with a
     * tab or space, stop reading because that's the end of the header. */
    while((cursor < endofinbuf) && (n < outbuf_size))
    {
        switch (state)
        {
            case CURSOR_STATE_NORMAL:
            if(*cursor == '\r')
            {
                if( (cursor + 1 < endofinbuf && *++cursor == '\n'))
                {
                    state = CURSOR_STATE_NEWLINE;
                }
                else
                {
                    *outbuf_ptr++ = *cursor;
                    n++;
                }
            }
            else if( *cursor  == '\n')
            {
                state = CURSOR_STATE_NEWLINE;
            }
            else
            {
                *outbuf_ptr++ = *cursor;
                n++;
            }
            break;

            case CURSOR_STATE_NEWLINE:
            if((*cursor == ' ') || (*cursor == '\t'))
            {
                num_spaces++;
                line_folding = true;
            }
            else if( line_folding )
            {
                if(*cursor != '\n')
                    state = CURSOR_STATE_NORMAL;
                line_folding = false;
                if(hi_server_is_known_header(cursor, endofinbuf))
                {
                    if(hi_eo_generate_event(Session, HI_EO_SERVER_INVALID_HEADER_FOLDING))
                    {
                        hi_eo_server_event_log(Session,
                                HI_EO_SERVER_INVALID_HEADER_FOLDING, NULL, NULL);
                    }
                }
                *outbuf_ptr++ = *cursor;
                n++;
            }
            else if((*cursor == 0x0b) || (*cursor == 0x0c))
            {
                if(hi_eo_generate_event(Session, HI_EO_SERVER_INVALID_HEADER_FOLDING))
                {
                    hi_eo_server_event_log(Session,
                            HI_EO_SERVER_INVALID_HEADER_FOLDING, NULL, NULL);
                }
               goto exit_loop;
            }
            else
                goto exit_loop;
            break;

        }
        cursor++;
    }
exit_loop:

    if(n < outbuf_size)
        *outbuf_ptr = '\0';

    *output_bytes = outbuf_ptr - outbuf;
    if(folded)
        *folded = num_spaces;
    if( n < outbuf_size)
        return 0;
    return -1;
}


/* Grab the argument of "charset=foo" from a Content-Type header */
static inline const u_char *extract_http_content_type_charset(HI_SESSION *Session,
        HttpSessionData *hsd, const u_char *p, const u_char *start, const u_char *end )
{
    size_t cmplen;
    uint8_t unfold_buf[DECODE_BLEN];
    uint32_t unfold_size =0;
    const char *ptr, *ptr_end;

    if (hsd == NULL)
        return p;

    /* Don't trim spaces so p is set to end of header */
    sf_unfold_http_header(Session, p, end-p, unfold_buf,
              sizeof(unfold_buf), &unfold_size, 0);
    if (!unfold_size)
    {
        set_decode_utf_state_charset(&(hsd->utf_state), CHARSET_DEFAULT);
        return p;
    }
    p += unfold_size;

    ptr = (const char *)unfold_buf;
    ptr_end = (const char *)(ptr + strlen((const char *)unfold_buf));

    ptr = SnortStrcasestr(ptr, (int)(ptr_end - ptr), "text");
    if (!ptr)
    {
        set_decode_utf_state_charset(&(hsd->utf_state), CHARSET_DEFAULT);
        return p;
    }

    ptr = SnortStrcasestr(ptr, (int)(ptr_end - ptr), "utf-");
    if (!ptr)
    {
        set_decode_utf_state_charset(&(hsd->utf_state), CHARSET_UNKNOWN);
        return p;
    }
    ptr += 4; /* length of "utf-" */
    cmplen = ptr_end - ptr;

    if ((cmplen > 0) && (*ptr == '8'))
    {
        set_decode_utf_state_charset(&(hsd->utf_state), CHARSET_DEFAULT);
    }
    else if ((cmplen > 0) && (*ptr == '7'))
    {
        set_decode_utf_state_charset(&(hsd->utf_state), CHARSET_UTF7);
        if(hi_eo_generate_event(Session, HI_EO_SERVER_UTF7))
            hi_eo_server_event_log(Session, HI_EO_SERVER_UTF7, NULL, NULL);
    }
    else if (cmplen >= 4)
    {
        if ( !strncasecmp(ptr, "16le", 4) )
            set_decode_utf_state_charset(&(hsd->utf_state), CHARSET_UTF16LE);
        else if ( !strncasecmp(ptr, "16be", 4) )
            set_decode_utf_state_charset(&(hsd->utf_state), CHARSET_UTF16BE);
        else if ( !strncasecmp(ptr, "32le", 4) )
            set_decode_utf_state_charset(&(hsd->utf_state), CHARSET_UTF32LE);
        else if ( !strncasecmp(ptr, "32be", 4) )
            set_decode_utf_state_charset(&(hsd->utf_state), CHARSET_UTF32BE);
        else
            set_decode_utf_state_charset(&(hsd->utf_state), CHARSET_UNKNOWN);
    }
    else
        set_decode_utf_state_charset(&(hsd->utf_state), CHARSET_UNKNOWN);

    return p;
}

static inline const u_char *extract_http_content_encoding(HTTPINSPECT_CONF *ServerConf,
        const u_char *p, const u_char *start, const u_char *end, HEADER_PTR *header_ptr,
        HEADER_FIELD_PTR *header_field_ptr,HI_SESSION *Session)
{
    const u_char *crlf;
    int space_present = 0;
    if (header_ptr->content_encoding.cont_encoding_start)
    {
        if(hi_eo_generate_event(Session, HI_EO_SERVER_MULTIPLE_CONTENT_ENCODING))
        {
            hi_eo_server_event_log(Session, HI_EO_SERVER_MULTIPLE_CONTENT_ENCODING, NULL, NULL);
        }

        header_ptr->header.uri_end = p;
        header_ptr->content_encoding.compress_fmt = 0;
        return p;
    }
    else
    {
        header_field_ptr->content_encoding = &header_ptr->content_encoding;
        header_field_ptr->content_encoding->cont_encoding_start =
            header_field_ptr->content_encoding->cont_encoding_end = NULL;
        header_field_ptr->content_encoding->compress_fmt = 0;
        p = p + HTTPRESP_HEADER_LENGTH__CONTENT_ENCODING;
    }
    CheckSkipAlertCharsBeforeColon(Session, start, end, &p);
    if(hi_util_in_bounds(start, end, p) && *p == ':')
    {
        p++;
        CheckSkipAlertMultipleColon(Session, start, end, &p, HI_SI_SERVER_MODE);

        if (  hi_util_in_bounds(start, end, p) )
        {
            if ( ServerConf->profile == HI_APACHE || ServerConf->profile == HI_ALL)
            {
                SkipWhiteSpace(start,end,&p);
            }
            else
            {
                SkipBlankAndNewLine(start,end,&p);
            }
            if( hi_util_in_bounds(start, end, p))
            {
                if ( *p == '\n' )
                {
                    while(hi_util_in_bounds(start, end, p))
                    {
                        if ( *p == '\n')
                        {
                            p++;
                            while( hi_util_in_bounds(start, end, p) && ( *p == ' ' || *p == '\t'))
                            {
                                space_present = 1;
                                p++;
                            }
                            if ( space_present )
                            {
                                if ( isalpha((int)*p))
                                    break;
                                else if(isspace((int)*p) && (ServerConf->profile == HI_APACHE || ServerConf->profile == HI_ALL) )
                                {
                                    SkipWhiteSpace(start,end,&p);
                                }
                                else
                                    return p;
                            }
                            else
                                return p;
                        }
                        else
                            break;
                    }
                }
                if(isalpha((int)*p))
                {
                    header_field_ptr->content_encoding->cont_encoding_start = p;
                    while(hi_util_in_bounds(start, end, p) && *p!='\n' )
                    {
                        if(IsHeaderFieldName(p, end, HTTPRESP_HEADER_NAME__GZIP, HTTPRESP_HEADER_LENGTH__GZIP) ||
                                IsHeaderFieldName(p, end, HTTPRESP_HEADER_NAME__XGZIP, HTTPRESP_HEADER_LENGTH__XGZIP))
                        {
                            if (header_ptr->content_encoding.compress_fmt)
                            {
                                 if(hi_eo_generate_event(Session, HI_EO_SERVER_MULTIPLE_CONTENT_ENCODING))
                                 {
                                     hi_eo_server_event_log(Session, HI_EO_SERVER_MULTIPLE_CONTENT_ENCODING, NULL, NULL);
                                 }
                            }
                            header_field_ptr->content_encoding->compress_fmt |= HTTP_RESP_COMPRESS_TYPE__GZIP;
                            p = p + HTTPRESP_HEADER_LENGTH__GZIP;
                            continue;
                        }
                        else if(IsHeaderFieldName(p, end, HTTPRESP_HEADER_NAME__DEFLATE, HTTPRESP_HEADER_LENGTH__DEFLATE))
                        {
                            if (header_ptr->content_encoding.compress_fmt)
                            {
                                if(hi_eo_generate_event(Session, HI_EO_SERVER_MULTIPLE_CONTENT_ENCODING))
                                {
                                    hi_eo_server_event_log(Session, HI_EO_SERVER_MULTIPLE_CONTENT_ENCODING, NULL, NULL);
                                }
                            }
                            header_field_ptr->content_encoding->compress_fmt |= HTTP_RESP_COMPRESS_TYPE__DEFLATE;
                            p = p + HTTPRESP_HEADER_LENGTH__DEFLATE;
                            continue;
                        }
                        else
                            p++;
                    }

                    /*crlf = (u_char *)SnortStrnStr((const char *)p, end - p, "\n");
                    if(crlf)
                    {
                        p = crlf;
                    }
                    else
                    {
                        header_ptr->header.uri_end = end ;
                        return end;
                    }*/
                }
                else
                    return p;
            }
        }
    }
    else
    {
        if(hi_util_in_bounds(start, end, p))
        {
            crlf = (u_char *)SnortStrnStr((const char *)p, end - p, "\n");
            if(crlf)
            {
                p = crlf;
            }
            else
            {
                header_ptr->header.uri_end = end ;
                return end;
            }
        }
    }
    if(!p || !hi_util_in_bounds(start, end, p))
        p = end;

    return p;
}

/*
 * extract_http_content_range() will extract required data from content-range
 * field. Focus is for, when the units is "bytes".
 *  The possible syntax as follows,
 *    content-range: <units> <start_pos>-<end_pos>/<total_len>
 *    content-range: <units> * /<total_len>
 *    content-range: <units> <start_pos>-<end_pos>/ *
 */
static const u_char *extract_http_content_range(HI_SESSION *Session,
             const u_char *p, const u_char *start, const u_char *end,
             HEADER_PTR *header_ptr)
{
    u_char *crlf = NULL;
    const u_char *unit_start = NULL;
    int is_byte_range = false;

    SkipBlankSpace(start,end,&p);
    if (hi_util_in_bounds(start, end, p) && *p == ':')
    {
        p++;
        CheckSkipAlertMultipleColon(Session, start, end, &p, HI_SI_SERVER_MODE);
        while (hi_util_in_bounds(start, end, p))
        {
            if ((*p == ' ') || (*p == '\t'))
            {
                p++;
                break;
            }
            p++;
        }
        if (hi_util_in_bounds(start, end, p))
        {
            /* extract unit */
            unit_start = p;
            while (hi_util_in_bounds(start, end, p) && ( *p != ' '))
            {
                p++;
            }

            if ((*p != ' ') || ((*(p+1) == ' ')))
            {
                if (hi_eo_generate_event(Session, HI_EO_SERVER_INVALID_CONTENT_RANGE_UNIT_FMT))
                {
                    hi_eo_server_event_log(Session, HI_EO_SERVER_INVALID_CONTENT_RANGE_UNIT_FMT, NULL, NULL);
                }
                header_ptr->range_flag = RANGE_WITH_RESP_ERROR;
                return end;
            }

            /* Set flag, when unit is bytes. Based on unit, flow will change */
            if (!strncasecmp((const char *)unit_start, RANGE_UNIT_BYTE, 5))
            {
                is_byte_range = true;
            }
            else
            {
                is_byte_range = false;
            }
            p++;

            if (hi_util_in_bounds(start, end, p))
            {
                if (is_byte_range == true)
                {
                    if (isdigit((int)*p))
                    {
                        /*if start with digit, then expectation is 
                          "<start_pos>-<end_pos>/<total_len>" or "<start_pos>-<end_pos>/ *"
                          There is no whitespace between / and * To make it comment added the space
                          pattern other than above mentioned format will flag as error */
                        uint64_t start_range = 0;
                        uint64_t end_range = 0;
                        uint64_t len = 0;
                        const u_char *start_ptr = NULL;
                        const u_char *end_ptr = NULL;
                        const u_char *len_ptr = NULL;
                        const u_char *ret_ptr = NULL;
                        int is_valid_range = false;
                        int delim_1 = false;
                        int delim_2 = false;
                        int assign = false;

                        start_ptr = p;
                        p++;
                        while (hi_util_in_bounds(start, end, p))
                        {
                            if (!isdigit((int)*p))
                            {
                                if (*p == '-')
                                {
                                    if ((delim_2 == true) || (delim_1 == true))
                                    {
                                        break;
                                    }
                                    delim_1 = true;
                                    assign = true;
                                    p++;
                                    continue;
                                } 
                                else if (*p == '/')
                                {
                                    if ((delim_1 == false) || (delim_2 == true))
                                    {
                                        break;
                                    }
                                    delim_2 = true;
                                    assign = true;
                                    p++;
                                    continue;
                                }
                                else if ((*p == '*') || (*p == '\r') || (*p == '\n'))
                                {
                                    if ((delim_1 == false) || (delim_2 == false))
                                    {
                                        break;
                                    }
                                    is_valid_range = true;
                                    break;
                                }
                                break;
                            } 

                            /* Here digit only come */
                            if (assign == true)
                            {
                                if ((delim_2 == false) && (delim_1 == true))
                                {
                                    end_ptr = p;
                                }
                                else if ((delim_1 == true) && (delim_2 == true))
                                {
                                    len_ptr = p;
                                }
                                else
                                {
                                    assign = false;
                                    break;
                                }
                                assign = false;
                            }
                            p++;       
                        }

                        if (is_valid_range == true)
                        {
                            /* Now check total_len, * means set the flag accordingly
                               or convert str to value and flag as full content
                               when differnce between start_pos and end_pos and sum with 1
                               equals to total_len, otherwise partial flag set */
                            if (*p == '*')
                            {
                                header_ptr->range_flag = RANGE_WITH_RESP_UNKNOWN_CONTENT_SIZE;
                            }
                            else if ((start_ptr) && (end_ptr) && (len_ptr))
                            {
                                int error = false;

                                start_range = (uint64_t)SnortStrtol((char *)start_ptr, (char**)&ret_ptr, 10);
                                if ((errno == ERANGE) || (start_ptr == ret_ptr) || (start_range > 0xFFFFFFFF))
                                {
                                    error = true;
                                }

                                ret_ptr = NULL;
                                end_range = (uint64_t)SnortStrtol((char *)end_ptr, (char**)&ret_ptr, 10);
                                if ((errno == ERANGE) || (end_ptr == ret_ptr) || (end_range > 0xFFFFFFFF))
                                {
                                    error = true;
                                }

                                ret_ptr = NULL;
                                len = (uint64_t)SnortStrtol((char *)len_ptr, (char**)&ret_ptr, 10);
                                if ((errno == ERANGE) || (len_ptr == ret_ptr) || (len > 0xFFFFFFFF))
                                {
                                    error = true;
                                }

                                if (error == false)
                                {
                                    if (((end_range - start_range) + 1) == len)
                                    {
                                        header_ptr->range_flag = RANGE_WITH_RESP_FULL_CONTENT;
                                    }
                                    else
                                    {
                                        header_ptr->range_flag = RANGE_WITH_RESP_PARTIAL_CONTENT;
                                    }
                                }
                                else
                                {
                                    header_ptr->range_flag = RANGE_WITH_RESP_ERROR;
                                }
                            }
                            else
                            { 
                                header_ptr->range_flag = RANGE_WITH_RESP_ERROR;
                            }

                            crlf = (u_char *)SnortStrnStr((const char *)p, end - p, "\n");
                            if (crlf)
                            {
                                return p;
                            }
                            else
                            {
                                header_ptr->header.uri_end = end;
                                return end;
                            }
                        }
                    }
                    else if (*p == '*')
                    {
                        /* To check pattern, "* /<total_len>" and set flag accordingly */
                        header_ptr->range_flag = RANGE_WITH_UNKNOWN_CONTENT_RANGE;
                        p++;
                        if (hi_util_in_bounds(start, end, p))
                        {
                            if (*p == '/')
                            {
                                p++;
                                if (hi_util_in_bounds(start, end, p))
                                {
                                    if (isdigit((int)*p))
                                    {
                                        p++;
                                        while (hi_util_in_bounds(start, end, p))
                                        {
                                            if (isdigit((int)*p))
                                            {
                                                p++;
                                                continue;
                                            }
                                            else if ((*p == '\r') || (*p == '\n')) /* digit followed by \r or \n */
                                            {
                                                crlf = (u_char *)SnortStrnStr((const char *)p, end - p, "\n");
                                                if (crlf)
                                                {
                                                    return p;
                                                }
                                                else
                                                {
                                                    header_ptr->header.uri_end = end;
                                                    return end;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    header_ptr->range_flag = RANGE_WITH_RESP_ERROR;
                    crlf = (u_char *)SnortStrnStr((const char *)p, end - p, "\n");
                    if (crlf)
                    {
                        p = crlf;
                        return p;
                    }
                    else
                    {
                        header_ptr->header.uri_end = end;
                        return end;
                    }
                }
                else
                {
                    /* other than byte range is out of scope for snort */
                    while (hi_util_in_bounds(start, end, p))
                    {
                        if ((*p == '\r') || (*p == '\n'))
                        {
                            header_ptr->range_flag = RANGE_WITH_RESP_NON_BYTE;
                            crlf = (u_char *)SnortStrnStr((const char *)p, end - p, "\n");
                            if (crlf)
                            {
                                p = crlf;
                                return p;
                            }
                            else
                            {
                                header_ptr->header.uri_end = end;
                                return end;
                            }
                        }
                        p++;
                    }
                } 
            }
        }
    }
    header_ptr->range_flag = RANGE_WITH_RESP_ERROR;
    crlf = (u_char *)SnortStrnStr((const char *)p, end - p, "\n");
    if (crlf)
    {
        p = crlf;
        return p;
    }
    else
    {
        header_ptr->header.uri_end = end;
        return end;
    }
}

/* extract_http_accept_ranges will extract and set flag based on bytes unit */ 
static const u_char *extract_http_accept_ranges(HI_SESSION *Session,
             const u_char *p, const u_char *start, const u_char *end,
             HEADER_PTR *header_ptr, uint16_t *accept_range_flag)
{
    const u_char *start_ptr = NULL;
    const u_char *end_ptr = NULL;
    u_char *crlf = NULL;
    int len = 0;

    *accept_range_flag = ACCEPT_RANGE_UNKNOWN;
    SkipBlankSpace(start,end,&p);

    if (hi_util_in_bounds(start, end, p) && (*p == ':'))
    {
        p++;
        CheckSkipAlertMultipleColon(Session, start, end, &p, HI_SI_SERVER_MODE);
        while (hi_util_in_bounds(start, end, p))
        {
            if ((*p == ' ') || (*p == '\t') || (*p == ','))
            {
                p++;
                break;
            }
            p++;
        }

        if (hi_util_in_bounds(start, end, p))
        {
            start_ptr = p;
            while (hi_util_in_bounds(start, end, p))
            {
                if ((*p == '\r') || (*p == '\n'))
                {
                    end_ptr = (p - 1);
                    break;
                }
                p++;
            }

            if (start_ptr && end_ptr)
            {
                len = end_ptr - start_ptr;
                if ((len >= 5) && (SnortStrcasestr((const char *)start, len, "bytes")))
                {
                    *accept_range_flag = ACCEPT_RANGE_BYTES;
                }
                else if ((len >= 4) && (!strncasecmp((const char *)start, "none", 4)))
                {
                    *accept_range_flag = ACCEPT_RANGE_NONE;
                }
                else
                {
                    *accept_range_flag = ACCEPT_RANGE_OTHER;
                }
            }
        }
    }    

    crlf = (u_char *)SnortStrnStr((const char *)p, end - p, "\n");
    if (crlf)
    {
        p = crlf;
        return p;
    }
    else
    {
        header_ptr->header.uri_end = end;
        return end;
    }
}

const u_char *extract_http_transfer_encoding(HI_SESSION *Session, HttpSessionData *hsd,
     const u_char *p, const u_char *start, const u_char *end,
     HEADER_PTR *header_ptr, int iInspectMode)
{
    uint8_t unfold_buf[DECODE_BLEN];
    uint32_t unfold_size =0;
    const u_char *start_ptr, *end_ptr, *cur_ptr;


    SkipBlankSpace(start,end,&p);

    if(hi_util_in_bounds(start, end, p) && *p == ':')
    {
        p++;
        CheckSkipAlertMultipleColon(Session, start, end, &p, iInspectMode);

        if(hi_util_in_bounds(start, end, p))
            sf_unfold_http_header(Session, p, end-p, unfold_buf,
            sizeof(unfold_buf), &unfold_size, 0);

        if(!unfold_size)
        {
            header_ptr->header.uri_end = end;
            return end;
        }

        p = p + unfold_size;

        start_ptr = unfold_buf;
        cur_ptr = unfold_buf;
        end_ptr = unfold_buf + unfold_size;
        SkipBlankSpace(start_ptr,end_ptr,&cur_ptr);

        start_ptr = cur_ptr;

        start_ptr = (u_char *)SnortStrcasestr((const char *)start_ptr, (end_ptr - start_ptr), "chunked");
        if (start_ptr)
        {
            if ((iInspectMode == HI_SI_SERVER_MODE) && hsd)
            {
                hsd->resp_state.last_pkt_chunked = 1;
                hsd->resp_state.last_pkt_contlen = 0;
            }
            header_ptr->content_len.len = 0 ;
            header_ptr->content_len.cont_len_start = NULL;
            header_ptr->is_chunked = true;
        }
    }
    else
    {
        header_ptr->header.uri_end = end;
        return end;
    }

    return p;
}

#if defined(FEAT_OPEN_APPID)
static const u_char *extract_http_server_header(HI_SESSION *Session, const u_char *p, const u_char *start,
        const u_char *end, HEADER_PTR  *header_ptr, HEADER_LOCATION *headerLoc)
{
    int num_spaces = 0;
    uint8_t unfold_buf[DECODE_BLEN];
    uint32_t unfold_size =0;
    const u_char *end_ptr, *cur_ptr;

    SkipBlankSpace(start,end,&p);

    if(hi_util_in_bounds(start, end, p) && *p == ':')
    {
        p++;
        CheckSkipAlertMultipleColon(Session, start, end, &p, HI_SI_SERVER_MODE);

        if(hi_util_in_bounds(start, end, p))
            sf_unfold_http_header(Session, p, end-p, unfold_buf,
        sizeof(unfold_buf), &unfold_size, &num_spaces);

        if(!unfold_size)
        {
            header_ptr->header.uri_end = end;
            return end;
        }

        p = p + unfold_size;

        cur_ptr = unfold_buf;
        end_ptr = unfold_buf + unfold_size;
        SkipBlankSpace(unfold_buf,end_ptr,&cur_ptr);

        {
            unsigned field_strlen = (unsigned)(end_ptr - cur_ptr); 
            if (field_strlen)
            {
                headerLoc->start = (u_char *)strndup((const char *)cur_ptr, field_strlen);
                if (NULL == headerLoc->start)
                {
                    /* treat this out-of-memory error as a parse failure */
                    header_ptr->header.uri_end = end;
                    return end;
                }
                /* now that we have the memory, fill in len. */
                headerLoc->len = field_strlen;
            }
        }
    }
    else
    {
        header_ptr->header.uri_end = end;
        return end;
    }

    return p;

}
#endif /* defined(FEAT_OPEN_APPID) */

static inline const u_char *extractHttpRespHeaderFieldValues(HTTPINSPECT_CONF *ServerConf,
        const u_char *p, const u_char *offset, const u_char *start,
        const u_char *end, HEADER_PTR *header_ptr,
        HEADER_FIELD_PTR *header_field_ptr, int parse_cont_encoding, bool parse_trans_encoding,
        HttpSessionData *hsd, HI_SESSION *Session)
{
    if (((p - offset) == 0) && ((*p == 'S') || (*p == 's')))
    {
        /* Search for 'Cookie' at beginning, starting from current *p */
        if ( ServerConf->enable_cookie &&
                IsHeaderFieldName(p, end, HTTPRESP_HEADER_NAME__COOKIE,
                    HTTPRESP_HEADER_LENGTH__COOKIE))
        {
            p = extract_http_cookie((p + HTTPRESP_HEADER_LENGTH__COOKIE), end, header_ptr, header_field_ptr);
        }
#if defined(FEAT_OPEN_APPID)
        else if ((ServerConf->appid_enabled) && (IsHeaderFieldName(p, end, HEADER_NAME__SERVER, HEADER_LENGTH__SERVER)))
        {
            p = p + HEADER_LENGTH__SERVER;
            p = extract_http_server_header(Session, p, start, end, header_ptr, &header_ptr->server);
        }
#endif /* defined(FEAT_OPEN_APPID) */
    }
    else if (((p - offset) == 0) && ((*p == 'C') || (*p == 'c')))
    {
        if ( IsHeaderFieldName(p, end, HTTPRESP_HEADER_NAME__CONTENT_TYPE,
                               HTTPRESP_HEADER_LENGTH__CONTENT_TYPE) && ServerConf->normalize_utf)
        {
#if defined(FEAT_OPEN_APPID)
            const u_char *ptr;
            ptr = p + HTTPRESP_HEADER_LENGTH__CONTENT_TYPE;
#endif /* defined(FEAT_OPEN_APPID) */
            p = extract_http_content_type_charset(Session, hsd, p, start, end);
#if defined(FEAT_OPEN_APPID)
            header_ptr->contentType.start = ptr;
            SkipBlankColon(ptr, p, &header_ptr->contentType.start);
            header_ptr->contentType.len = p - header_ptr->contentType.start;
#endif /* defined(FEAT_OPEN_APPID) */
        }

        else if ( IsHeaderFieldName(p, end, HTTPRESP_HEADER_NAME__CONTENT_ENCODING,
                    HTTPRESP_HEADER_LENGTH__CONTENT_ENCODING) && ServerConf->extract_gzip &&
                    parse_cont_encoding)
        {
            p = extract_http_content_encoding(ServerConf, p, start, end, header_ptr, header_field_ptr,Session);
        }
        else if ( IsHeaderFieldName(p, end, HTTPRESP_HEADER_NAME__CONTENT_LENGTH,
                HTTPRESP_HEADER_LENGTH__CONTENT_LENGTH) )
        {
            if(hsd && !hsd->resp_state.last_pkt_chunked)
                p = extract_http_content_length(Session, ServerConf, p, start, end, header_ptr, header_field_ptr, HI_SI_SERVER_MODE );
        }
        else if (IsHeaderFieldName(p, end, HTTPRESP_HEADER_NAME__CONTENT_RANGE, HTTPRESP_HEADER_LENGTH__CONTENT_RANGE))
        {
            p = p + HTTPRESP_HEADER_LENGTH__CONTENT_RANGE;
            p = extract_http_content_range(Session, p, start, end, header_ptr);
        }
    }
    else if (((p - offset) == 0) && ((*p == 'T') || (*p == 't')))
    {
        if ( IsHeaderFieldName(p, end, HTTPRESP_HEADER_NAME__TRANSFER_ENCODING,
                               HTTPRESP_HEADER_LENGTH__TRANSFER_ENCODING) && parse_trans_encoding )
        {
            p = p + HTTPRESP_HEADER_LENGTH__TRANSFER_ENCODING;
            p = extract_http_transfer_encoding(Session, hsd, p, start, end, header_ptr, HI_SI_SERVER_MODE);
        }
    }
    else if (((p - offset) == 0) && ((*p == 'A') || (*p == 'a')))
    {
        if (IsHeaderFieldName(p, end, HTTPRESP_HEADER_NAME__ACCEPT_RANGES, 
                               HTTPRESP_HEADER_LENGTH__ACCEPT_RANGES))
        {
            uint16_t accept_range_flag = 0;
            p = p + HTTPRESP_HEADER_LENGTH__ACCEPT_RANGES;
            p = extract_http_accept_ranges(Session, p, start, end, header_ptr, &accept_range_flag);
            Session->server.response.accept_range_flag = accept_range_flag;
        }
    }
#if defined(FEAT_OPEN_APPID)
    else if(((p - offset) == 0) && ((*p == 'V') || (*p == 'v')))
    {
        if((ServerConf->appid_enabled) && (IsHeaderFieldName(p, end, HEADER_NAME__VIA, HEADER_LENGTH__VIA)))
        {
            p = p + HEADER_LENGTH__VIA;
            p = extract_http_server_header(Session, p, start, end, header_ptr, &header_ptr->via);
        }
    }
    else if(((p - offset) == 0) && ((*p == 'X') || (*p == 'x')))
    {
        if((ServerConf->appid_enabled) && (IsHeaderFieldName(p, end, HEADER_NAME__X_WORKING_WITH, HEADER_LENGTH__X_WORKING_WITH)))
        {
            p = p + HEADER_LENGTH__X_WORKING_WITH;
            p = extract_http_server_header(Session, p, start, end, header_ptr, &header_ptr->xWorkingWith);
        }
    }
#endif /* defined(FEAT_OPEN_APPID) */
    return p;
}

int hi_server_is_known_header(
    const u_char *p, const u_char *end)
{

    if ((*p == 'C') || (*p == 'c'))
    {
        if (IsHeaderFieldName(p, end, HTTPRESP_HEADER_NAME__CONTENT_TYPE,
                               HTTPRESP_HEADER_LENGTH__CONTENT_TYPE) ||
           IsHeaderFieldName(p, end, HTTPRESP_HEADER_NAME__CONTENT_ENCODING,
                    HTTPRESP_HEADER_LENGTH__CONTENT_ENCODING) ||
           IsHeaderFieldName(p, end, HTTPRESP_HEADER_NAME__CONTENT_LENGTH,
                HTTPRESP_HEADER_LENGTH__CONTENT_LENGTH))
        {
            return 1;
        }
    }

    if ((*p == 'T') || (*p == 't'))
    {
        if (IsHeaderFieldName(p, end, HTTPRESP_HEADER_NAME__TRANSFER_ENCODING,
                               HTTPRESP_HEADER_LENGTH__TRANSFER_ENCODING))
        {
            return 1;
        }
    }

    return 0;
}

static inline const u_char *hi_server_extract_header(
        HI_SESSION *Session, HTTPINSPECT_CONF *ServerConf,
            HEADER_PTR *header_ptr, const u_char *start,
            const u_char *end, int parse_cont_encoding,
            bool parse_trans_encoding, HttpSessionData *hsd)
{
    const u_char *p;
    const u_char *offset;
    HEADER_FIELD_PTR header_field_ptr ;
    bool newline = false;

    if(!start || !end)
        return NULL;

    p = start;

    offset = (u_char*)p;

    header_ptr->header.uri = p;
    header_ptr->header.uri_end = end;
    header_ptr->content_encoding.compress_fmt = 0;
    header_ptr->content_len.len = 0;
    header_ptr->is_chunked = false;

    while (hi_util_in_bounds(start, end, p))
    {
        if(*p == '\n')
        {
            newline = true;
            p++;

            offset = (u_char*)p;

            if (!hi_util_in_bounds(start, end, p))
            {
                header_ptr->header.uri_end = p;
                return p;
            }

            if (*p < 0x0E)
            {
                if(*p == '\r')
                {
                    p++;

                    if(hi_util_in_bounds(start, end, p) && (*p == '\n'))
                    {
                        p++;
                        header_ptr->header.uri_end = p;
                        hsd->resp_state.eoh_found = true;
                        return p;
                    }
                }
                else if(*p == '\n')
                {
                    p++;
                    header_ptr->header.uri_end = p;
                    hsd->resp_state.eoh_found = true;
                    return p;
                }
            }
            else if ( (p = extractHttpRespHeaderFieldValues(ServerConf, p, offset,
                            start, end, header_ptr, &header_field_ptr,
                            parse_cont_encoding, parse_trans_encoding, hsd, Session)) == end)
            {
                return end;
            }

        }
        else if( (p == header_ptr->header.uri) &&
                (p = extractHttpRespHeaderFieldValues(ServerConf, p, offset,
                          start, end, header_ptr, &header_field_ptr,
                          parse_cont_encoding, parse_trans_encoding, hsd, Session)) == end)
        {
            return end;
        }
        if ( *p == '\n')
        {
            newline = true;
            continue;
        }
        if (newline && (*p == '\t' || *p == ' '))
        {
            while(*p == '\t' ||  *p == ' ')
                p++;

            if (hi_util_in_bounds(start, end, p) &&
                    hi_server_is_known_header(p, end))
            {
                if(hi_eo_generate_event(Session, HI_EO_SERVER_INVALID_HEADER_FOLDING))
                {
                    hi_eo_server_event_log(Session, HI_EO_SERVER_INVALID_HEADER_FOLDING,
                            NULL, NULL);
                }
                newline = false;
            }
        }
        else
        {
            newline = false;
            p++;
        }
    }

    header_ptr->header.uri_end = p;
    return p;
}

static inline int hi_server_extract_body(
                        HI_SESSION *Session, HttpSessionData *sd,
                        const u_char *ptr, const u_char *end, URI_PTR *result)
{
    HTTPINSPECT_CONF *ServerConf;
    const u_char *start = ptr;
    int iRet = HI_SUCCESS;
    const u_char *post_end = end;
    uint32_t updated_chunk_remainder = 0;
    uint32_t chunk_read = 0;
    int64_t bytes_to_read = 0;
    ServerConf = Session->server_conf;

    switch(ServerConf->server_extract_size)
    {
        case -1:
            result->uri = result->uri_end = NULL;
            return iRet;
        case 0:
            break;
        default:
            if(sd->resp_state.data_extracted < ServerConf->server_extract_size)
            {
                bytes_to_read = ServerConf->server_extract_size - sd->resp_state.data_extracted;
                if((end-ptr) > bytes_to_read )
                {
                    end = ptr + bytes_to_read;
                }
                else
                    bytes_to_read = (end-ptr);
                sd->resp_state.data_extracted += (int)bytes_to_read;
            }
            else
            {
                result->uri = result->uri_end = NULL;
                return iRet;
            }
    }

/*    if( ServerConf->server_flow_depth && ((end - ptr) > ServerConf->server_flow_depth) )
    {
        end = ptr + ServerConf->server_flow_depth;
    }*/

    if (!(sd->resp_state.last_pkt_contlen))
    {
        if( ServerConf->chunk_length || ServerConf->small_chunk_length.size )
        {
            if (sd->resp_state.last_pkt_chunked
                && CheckChunkEncoding(Session, start, end, &post_end,
                                      (u_char *)HttpDecodeBuf.data, sizeof(HttpDecodeBuf.data),
                                      sd->resp_state.chunk_remainder, &updated_chunk_remainder, &chunk_read,
                                      sd, HI_SI_SERVER_MODE) == 1)
            {
                sd->resp_state.chunk_remainder = updated_chunk_remainder;
                sd->resp_state.last_pkt_chunked = 1;
                result->uri = (u_char *)HttpDecodeBuf.data;
                result->uri_end = result->uri + chunk_read;
                return iRet;
            }
            else
            {
                if(!(sd->resp_state.last_pkt_chunked) && !simple_response)
                {
                    if(hi_eo_generate_event(Session, HI_EO_SERVER_NO_CONTLEN))
                    {
                        hi_eo_server_event_log(Session, HI_EO_SERVER_NO_CONTLEN, NULL, NULL);
                    }
                }
                else
                    sd->resp_state.last_pkt_chunked = 0;
                result->uri = start;
                result->uri_end = end;
            }
        }
        else
        {
            result->uri = start;
            result->uri_end = end;
            return iRet;
        }
    }

    result->uri = start;
    result->uri_end = end;

    return STAT_END;
}

static void LogFileDecomp( void *Context, int Event )
{
    if( Context != NULL )
        if(hi_eo_generate_event((HI_SESSION *)Context, Event))
            hi_eo_server_event_log((HI_SESSION *)Context, Event, NULL, NULL);
}

static void InitFileDecomp(HttpSessionData *hsd, HI_SESSION *session, void* ssnptr)
{
    fd_session_p_t fd_session;

    if((hsd == NULL) || (session == NULL) || (session->server_conf == NULL) ||
       (session->global_conf == NULL) || (hi_fd_conf.fd_MemPool == NULL) )
        return;

    if( (fd_session = File_Decomp_New(ssnptr)) == (fd_session_p_t)NULL )
        return;

    hsd->fd_state = fd_session;
    fd_session->Modes = session->server_conf->file_decomp_modes;

    fd_session->Alert_Callback = LogFileDecomp;
    fd_session->Alert_Context = session;

    if( (session->server_conf->unlimited_decompress) != 0 )
    {
        fd_session->Compr_Depth = 0;
        fd_session->Decompr_Depth = 0;
    }
    else
    {
        fd_session->Compr_Depth = session->global_conf->compr_depth;
        fd_session->Decompr_Depth = session->global_conf->decompr_depth;
    }

    (void)File_Decomp_Init( fd_session );
}

static void SetGzipBuffers(HttpSessionData *hsd, HI_SESSION *session, void* scbPtr)
{
    if ((hsd != NULL) && (hsd->decomp_state == NULL)
            && (session != NULL) && (session->server_conf != NULL)
            && (session->global_conf != NULL) && session->server_conf->extract_gzip)
    {
        MemBucket *bkt = mempool_alloc(hi_gzip_mempool);

        if (bkt != NULL)
        {
            bkt->scbPtr = scbPtr;
            hsd->decomp_state = bkt->data;
            hsd->decomp_state->bkt = bkt;
            if (session->server_conf->unlimited_decompress)
            {
                hsd->decomp_state->compr_depth = MAX_GZIP_DEPTH;
                hsd->decomp_state->decompr_depth = MAX_GZIP_DEPTH;
            }
            else
            {
                hsd->decomp_state->compr_depth = session->global_conf->compr_depth;
                hsd->decomp_state->decompr_depth = session->global_conf->decompr_depth;
            }
            hsd->decomp_state->inflate_init = 0;
            hsd->decomp_state->stage = HTTP_DECOMP_START;
        }
        else
        {
            mempool_free(hi_gzip_mempool, bkt);
        }
    }
}

int uncompress_gzip ( HI_SESSION *Session, u_char *dest, int destLen, const u_char *source,
        int sourceLen, HttpSessionData *sd, int *total_bytes_read, int compr_fmt)
{
    z_streamp streamp;
    int err;
    int iRet = HI_SUCCESS;
    int bytes_read_so_far;

    streamp = &sd->decomp_state->d_stream;

    /* Are we starting a new packet or continuing on the current one? */
    if (sd->decomp_state->stage == HTTP_DECOMP_START)
    {
        streamp->next_in = (Bytef*)source;
        streamp->avail_in = (uInt)sourceLen;
        if ((uLong)streamp->avail_in != (uLong)sourceLen)
        {
            sd->decomp_state->stage = HTTP_DECOMP_FIN;
            return HI_FATAL_ERR;
        }
        bytes_read_so_far = 0;
    }
    else 
    {
        bytes_read_so_far = streamp->total_out;
    }

    streamp->next_out = dest;
    streamp->avail_out = (uInt)destLen;
    if ((uLong)streamp->avail_out != (uLong)destLen)
    {
        return HI_FATAL_ERR;
    }


    if(!sd->decomp_state->inflate_init)
    {
        sd->decomp_state->inflate_init = 1;
        streamp->zalloc = (alloc_func)0;
        streamp->zfree = (free_func)0;
        if(compr_fmt & HTTP_RESP_COMPRESS_TYPE__DEFLATE)
            err = inflateInit(streamp);
        else
            err = inflateInit2(streamp, GZIP_WBITS);
        if (err != Z_OK)
        {
            return HI_FATAL_ERR;
        }
    }
    else if (sd->decomp_state->stage != HTTP_DECOMP_MID)
    {
        streamp->total_in = 0;
        streamp->total_out =0;
    }

    err = inflate(streamp, Z_SYNC_FLUSH);
    if ((!sd->decomp_state->deflate_initialized)
            && (err == Z_DATA_ERROR)
            && (compr_fmt & HTTP_RESP_COMPRESS_TYPE__DEFLATE))
    {
        inflateEnd(streamp);
        err = inflateInit2(streamp,DEFLATE_RAW_WBITS);
        if (err != Z_OK)
        {
            return HI_FATAL_ERR;
        }

        sd->decomp_state->deflate_initialized = true;

        streamp->next_in = (Bytef*)source;
        streamp->avail_in = (uInt)sourceLen;

        err = inflate(streamp, Z_SYNC_FLUSH);
    }

    if ((err != Z_STREAM_END) && (err !=Z_OK))
    {

        /* If some of the compressed data is decompressed we need to provide that for detection */
        if (( streamp->total_out > 0) && (err != Z_DATA_ERROR))
        {
            *total_bytes_read = streamp->total_out;
            iRet = HI_NONFATAL_ERR;
        }
        else
            iRet = HI_FATAL_ERR;
        inflateEnd(streamp);
        sd->decomp_state->stage = HTTP_DECOMP_FIN;
        return iRet;
    }
    *total_bytes_read = streamp->total_out - bytes_read_so_far;

    /* Check if we need to decompress more */
    if(err == Z_STREAM_END)
    {
        if(streamp->avail_in != 0 )
        {
            /* We have a case here where  uncompression returned END, yet data is available for compression */
            if(streamp->total_out > 0)
            {
                //Partial decompression case, log an alert
                if(hi_eo_generate_event(Session, HI_EO_SERVER_PARTIAL_DECOMPRESSION_FAIL))
                {
                    hi_eo_server_event_log(Session, HI_EO_SERVER_PARTIAL_DECOMPRESSION_FAIL, NULL, NULL);
                }
            }
            else
            {
                //Proceed with non-fatal error and generate decompression failed event
                iRet = HI_NONFATAL_ERR;
            }
        }
        sd->decomp_state->stage = HTTP_DECOMP_FIN;
        return iRet;
    }
    else if (sd->decomp_state->d_stream.total_in < sourceLen && *total_bytes_read != 0)
        sd->decomp_state->stage = HTTP_DECOMP_MID;
    else
        sd->decomp_state->stage = HTTP_DECOMP_FIN;
    return HI_SUCCESS;
}

static inline int hi_server_decompress(HI_SESSION *Session, HttpSessionData *sd, const u_char *ptr,
        const u_char *end, URI_PTR *result, bool start_of_body)
{
    const u_char *start = ptr;
    int rawbuf_size = end - ptr;
    int iRet = HI_SUCCESS;
    int zRet = HI_SUCCESS;
    int compr_depth, decompr_depth;
    int compr_bytes_read, decompr_bytes_read;
    int compr_avail, decompr_avail;
    int total_bytes_read = 0;
    static uint32_t updated_chunk_remainder = 0;
    static uint32_t chunk_read = 0;
    uint32_t saved_chunk_size = 0;

    compr_depth = sd->decomp_state->compr_depth;
    decompr_depth = sd->decomp_state->decompr_depth;
    compr_bytes_read = sd->decomp_state->compr_bytes_read;
    decompr_bytes_read = sd->decomp_state->decompr_bytes_read;
    saved_chunk_size = sd->resp_state.chunk_remainder;

    if(Session->server_conf->unlimited_decompress)
    {
        compr_avail = compr_depth;
        decompr_avail = decompr_depth;
    }
    else
    {
        compr_avail = compr_depth-compr_bytes_read;
        decompr_avail = decompr_depth - decompr_bytes_read;
    }

    /* Apply the server extract size
     * If the server extract size is set then we need to decompress only upto the
     * server flow depth
     */
    switch ( Session->server_conf->server_extract_size)
    {
        case -1:
            decompr_avail=0;
            break;
        case 0:
            break;
        default:
            if(sd->resp_state.data_extracted < Session->server_conf->server_extract_size)
            {
                if(decompr_avail > (Session->server_conf->server_extract_size - sd->resp_state.data_extracted))
                    decompr_avail = (int)(Session->server_conf->server_extract_size - sd->resp_state.data_extracted);
            }
            else
            {
                decompr_avail = 0;
            }
            break;
    }

    if ((compr_avail <= 0) || (decompr_avail <= 0))
    {
        (void)File_Decomp_Reset(sd->fd_state);
        ResetGzipState(sd->decomp_state);
        ResetRespState(&(sd->resp_state));
        return iRet;
    }


    if(rawbuf_size < compr_avail)
    {
        compr_avail = rawbuf_size;
    }

    if(!(sd->resp_state.last_pkt_contlen))
    {
        if(sd->resp_state.last_pkt_chunked)
        {
            int cRet = 1;
            if(!(sd->decomp_state && sd->decomp_state->stage == HTTP_DECOMP_MID))
            {
                chunk_read = 0;
                updated_chunk_remainder = 0;
                cRet = CheckChunkEncoding(Session, start, end, NULL, dechunk_buffer, compr_avail,
                           sd->resp_state.chunk_remainder, &updated_chunk_remainder, &chunk_read,
                           sd, HI_SI_SERVER_MODE);
            }
            if(cRet == 1)
            {
                sd->resp_state.chunk_remainder = updated_chunk_remainder;
                compr_avail = chunk_read;
                zRet = uncompress_gzip(Session, decompression_buffer, decompr_avail, dechunk_buffer,
                           compr_avail, sd, &total_bytes_read, sd->decomp_state->compress_fmt);
            }
        }
        else
        {
            /* No Content-Length or Transfer-Encoding : chunked */
            if(hi_eo_generate_event(Session, HI_EO_SERVER_NO_CONTLEN))
            {
                hi_eo_server_event_log(Session, HI_EO_SERVER_NO_CONTLEN, NULL, NULL);
            }

            zRet = uncompress_gzip(Session, decompression_buffer, decompr_avail, ptr, compr_avail,
                    sd, &total_bytes_read, sd->decomp_state->compress_fmt);
        }
    }
    else
    {
        zRet = uncompress_gzip(Session, decompression_buffer, decompr_avail, ptr, compr_avail,
                sd, &total_bytes_read, sd->decomp_state->compress_fmt);
    }
    if(!Session->server_conf->unlimited_decompress)
        sd->decomp_state->stage = HTTP_DECOMP_FIN;

    if((zRet == HI_SUCCESS) || (zRet == HI_NONFATAL_ERR))
    {
        sd->decomp_state->compr_bytes_read += compr_avail;
        hi_stats.compr_bytes_read += compr_avail;

        result->uri = decompression_buffer;
        if ( total_bytes_read < decompr_avail )
        {
            result->uri_end = decompression_buffer + total_bytes_read;
            sd->decomp_state->decompr_bytes_read += total_bytes_read;
            sd->resp_state.data_extracted += total_bytes_read;
            hi_stats.decompr_bytes_read += total_bytes_read;
        }
        else
        {
            result->uri_end = decompression_buffer + decompr_avail;
            sd->decomp_state->decompr_bytes_read += decompr_avail;
            sd->resp_state.data_extracted += decompr_avail;
            hi_stats.decompr_bytes_read += decompr_avail;
        }
    }
    else
    {
        if(!sd->decomp_state->decompr_bytes_read)
        {
            sd->resp_state.chunk_remainder = saved_chunk_size;
            iRet = HI_NONFATAL_ERR;
        }
        else
            ResetRespState(&(sd->resp_state));
        (void)File_Decomp_Reset(sd->fd_state);
        ResetGzipState(sd->decomp_state);
        sd->decomp_state->stage = HTTP_DECOMP_FIN;
    }

    if(zRet!=HI_SUCCESS && start_of_body)
    {
        if(hi_eo_generate_event(Session, HI_EO_SERVER_DECOMPR_FAILED))
        {
            hi_eo_server_event_log(Session, HI_EO_SERVER_DECOMPR_FAILED, NULL, NULL);
        }
    }

    return iRet;


}

static inline int hi_server_inspect_body(HI_SESSION *Session, HttpSessionData *sd, const u_char *ptr,
                        const u_char *end, URI_PTR *result, bool start_of_body)
{
    int iRet = HI_SUCCESS;

    result->uri =ptr;
    result->uri_end = end;
    if(!Session || !sd )
    {
        if ((sd != NULL))
        {
            (void)File_Decomp_Reset(sd->fd_state);
            ResetGzipState(sd->decomp_state);
            ResetRespState(&(sd->resp_state));
        }
        return HI_INVALID_ARG;
    }

    if((sd->decomp_state != NULL) && sd->decomp_state->decompress_data)
    {
        iRet = hi_server_decompress(Session, sd, ptr, end, result, start_of_body);
        if(iRet == HI_NONFATAL_ERR)
        {
            sd->resp_state.inspect_body = 1;
            result->uri = ptr;
            result->uri_end = end;
            iRet = hi_server_extract_body(Session, sd, ptr, end, result);
        }
    }
    else
    {
        result->uri = ptr;
        result->uri_end = end;
        iRet = hi_server_extract_body(Session, sd, ptr, end, result);
    }

    return iRet;

}
void ApplyFlowDepth(HTTPINSPECT_CONF *ServerConf, Packet *p,
        HttpSessionData *sd, int resp_header_size, int read_only, uint32_t seq_num)
{
    if(!ServerConf->server_flow_depth)
    {
        SetDetectLimit(p, p->dsize);
    }
    else if(ServerConf->server_flow_depth == -1)
    {
        SetDetectLimit(p, resp_header_size);
    }
    else
    {
        if(sd != NULL)
        {
            if(!(sd->resp_state.flow_depth_excd ))
            {
                if(sd->resp_state.max_seq)
                {
                    if(SEQ_GEQ((sd->resp_state.max_seq), seq_num))
                    {
                        if(((uint32_t)p->dsize) > (sd->resp_state.max_seq- seq_num))
                        {
                            SetDetectLimit(p, (uint16_t)(sd->resp_state.max_seq-seq_num));
                            if( !read_only )
                                sd->resp_state.flow_depth_excd = true;
                            return;
                        }
                        else
                        {
                            SetDetectLimit(p, p->dsize);
                            return;
                        }
                    }
                    else
                    {
                        if( !read_only )
                            sd->resp_state.flow_depth_excd = true;
                        SetDetectLimit(p, resp_header_size);
                        return;
                    }
                }
                else
                {
                    if( !read_only )
                        sd->resp_state.flow_depth_excd = false;
                    SetDetectLimit(p, (((ServerConf->server_flow_depth) < p->dsize)? ServerConf->server_flow_depth: p->dsize));
                }
            }
            else
            {
                SetDetectLimit(p, 0);
                return;
            }

        }
        else
        {

            SetDetectLimit(p, (((ServerConf->server_flow_depth) < p->dsize)? (ServerConf->server_flow_depth): (p->dsize)));
        }
    }
}

static inline void ResetState (HttpSessionData* sd)
{
    (void)File_Decomp_Reset(sd->fd_state);
    ResetGzipState(sd->decomp_state);
    ResetRespState(&(sd->resp_state));
}

int HttpResponseInspection(HI_SESSION *Session, Packet *p, const unsigned char *data,
        int dsize, HttpSessionData *sd)
{
    HTTPINSPECT_CONF *ServerConf;
    URI_PTR stat_code_ptr;
    URI_PTR stat_msg_ptr;
    HEADER_PTR header_ptr;
    URI_PTR body_ptr;
    HI_SERVER *Server;

    const u_char *start;
    const u_char *end;
    const u_char *ptr;
    int len;
    int iRet = 0;
    int resp_header_size = 0;
    /* Refers to the stream reassembled packets when reassembly is turned on.
     * Refers to all packets when reassembly is turned off.
     */
    int not_stream_insert = 1;
    int parse_cont_encoding = 1;
    int status;
    int expected_pkt = 0;
    int alt_dsize;
    uint32_t seq_num = 0;

    static uint32_t paf_bytes_total = 0;
    static int paf_bytes_curr = 0;
    uint32_t paf_bytes_processed = 0;
    bool parse_trans_encoding = true;

    if (ScPafEnabled())
    {
	paf_bytes_processed = hi_paf_resp_bytes_processed(p->ssnptr);
	paf_bytes_curr = paf_bytes_processed - paf_bytes_total;
	paf_bytes_total = paf_bytes_processed;
	if (paf_bytes_curr < 0)
	    paf_bytes_curr = paf_bytes_processed;
    }

    if (!Session || !p || !data || (dsize == 0))
        return HI_INVALID_ARG;

    ServerConf = Session->server_conf;
    if(!ServerConf)
        return HI_INVALID_ARG;


    Server = &(Session->server);
    clearHttpRespBuffer(Server);

    seq_num = GET_PKT_SEQ(p);

    if ( ScPafEnabled() )
    {
        expected_pkt = !PacketHasStartOfPDU(p);
        parse_cont_encoding = !expected_pkt;
        not_stream_insert = PacketHasPAFPayload(p);
        parse_trans_encoding = !hi_paf_disable_te(p->ssnptr, false);

        if ( !expected_pkt )
        {
            simple_response = false;
            if ( sd )
            {
                if (!sd->decomp_state || (sd->decomp_state->stage != HTTP_DECOMP_MID))
                    ResetState(sd);
            }
        }
        else if ( sd )
        {
            if(hi_paf_simple_request(p->ssnptr))
            {
                simple_response = true;
                if(!(sd->resp_state.next_seq))
                {
                    /*first simple response packet */
                    sd->resp_state.next_seq = seq_num + p->dsize;
                    if(ServerConf->server_flow_depth == -1)
                            sd->resp_state.flow_depth_excd = true;
                    else
                    {
                        sd->resp_state.flow_depth_excd = false;
                        sd->resp_state.max_seq = seq_num + ServerConf->server_flow_depth;
                    }

                }
            }
            else
                simple_response = false;

            if(ServerConf->server_extract_size)
            {
                /*Packet is beyond the extract limit*/
                if ( sd && (sd->resp_state.data_extracted > ServerConf->server_extract_size ))
                {
                    expected_pkt = 0;
                    ResetState(sd);
                    if (sd->decomp_state)
                        sd->decomp_state->stage = HTTP_DECOMP_FIN;
                }
            }
        }
    }
    // when PAF is hardened, the following can be removed
    else if ( (sd != NULL) )
    {
        /* If the previously inspected packet in this session identified as a body
         * and if the packets are stream inserted wait for reassembled */
        if (sd->resp_state.inspect_reassembled)
        {
            if(p->packet_flags & PKT_STREAM_INSERT)
            {
                parse_cont_encoding = 0;
                not_stream_insert = 0;
            }
        }
        /* If this packet is the next expected packet to be inspected and is out of sequence
         * clear out the resp state*/
        if(( sd->decomp_state && sd->decomp_state->decompress_data) && parse_cont_encoding)
        {
            if( sd->resp_state.next_seq &&
                    (seq_num == sd->resp_state.next_seq) )
            {
                sd->resp_state.next_seq = seq_num + p->dsize;
                expected_pkt = 1;
            }
            else
            {
                (void)File_Decomp_Reset(sd->fd_state);
                ResetRespState(&(sd->resp_state));
                if(sd->decomp_state && sd->decomp_state->stage != HTTP_DECOMP_START)
                {
                    ResetGzipState(sd->decomp_state);
                    sd->decomp_state->stage = HTTP_DECOMP_FIN;
                }
                else
                    ResetGzipState(sd->decomp_state);
            }
        }
        else
        if(sd->resp_state.inspect_body && not_stream_insert)
        {
            /* If the server extrtact size is 0 then we need to check if the packet
             * is in sequence
             */
            if(!ServerConf->server_extract_size)
            {
                if( sd->resp_state.next_seq &&
                        (seq_num == sd->resp_state.next_seq) )
                {
                    sd->resp_state.next_seq = seq_num + p->dsize;
                    expected_pkt = 1;
                }
                else
                {
                    (void)File_Decomp_Reset(sd->fd_state);
                    ResetRespState(&(sd->resp_state));
                    if(sd->decomp_state && sd->decomp_state->stage != HTTP_DECOMP_START)
                    {
                        ResetGzipState(sd->decomp_state);
                        sd->decomp_state->stage = HTTP_DECOMP_FIN;
                    }
                    else
                        ResetGzipState(sd->decomp_state);
                }
            }
            else
            {

                if( (ServerConf->server_extract_size > 0) &&(sd->resp_state.data_extracted > ServerConf->server_extract_size))
                {
                    expected_pkt = 1;
                }
                else
                {
                    (void)File_Decomp_Reset(sd->fd_state);
                    ResetRespState(&(sd->resp_state));
                    if(sd->decomp_state && sd->decomp_state->stage != HTTP_DECOMP_START)
                    {
                        ResetGzipState(sd->decomp_state);
                        sd->decomp_state->stage = HTTP_DECOMP_FIN;
                    }
                    else
                        ResetGzipState(sd->decomp_state);
                }

            }

        }
    }

    memset(&stat_code_ptr, 0x00, sizeof(URI_PTR));
    memset(&stat_msg_ptr, 0x00, sizeof(URI_PTR));
    memset(&header_ptr, 0x00, sizeof(HEADER_PTR));
    memset(&body_ptr, 0x00, sizeof(URI_PTR));

    start = data;
    end = data + dsize;
    ptr = start;

    /* moving past the CRLF */

    while(hi_util_in_bounds(start, end, ptr))
    {
        if(*ptr < 0x21)
        {
            if(*ptr < 0x0E && *ptr > 0x08)
            {
                ptr++;
                continue;
            }
            else
            {
                if(*ptr == 0x20)
                {
                    ptr++;
                    continue;
                }
            }
        }

        break;
    }

    /*after doing this we need to basically check for version, status code and status message*/
    if(( PacketHasStartOfPDU(p)) &&
        ((*ptr != 'H') || *(ptr+1) != 'T' || *(ptr+2) != 'T'
          || *(ptr+3) != 'P' || *(ptr+4) != '/'))
    {
        if( !(ptr = (u_char *)SnortStrcasestr((const char *)ptr, end-ptr, "HTTP/")))
        {
            CLR_SERVER_HEADER(Server);
            return HI_SUCCESS;
        }
    }

    len = end - ptr;

    if ( len > 4 )
    {
        bool header = false;

        p->packet_flags |= PKT_HTTP_DECODE;
        if(!sd->resp_state.inspect_reassembled ||
           !PacketHasPAFPayload(p) ||
           PacketHasStartOfPDU(p) ||
           !ScPafEnabled())
            header = IsHttpVersion(&ptr, end);

        if(!header)
        {
            if(expected_pkt)
                ptr = start;
            else
            {
                ApplyFlowDepth(ServerConf, p, sd, resp_header_size, 0, seq_num);
                if ( not_stream_insert && (sd != NULL))
                {
                    (void)File_Decomp_Reset(sd->fd_state);
                    ResetGzipState(sd->decomp_state);
                    ResetRespState(&(sd->resp_state));
                }
                CLR_SERVER_HEADER(Server);
                return HI_SUCCESS;
            }
        }
        else
        {
            simple_response = false;
            /* This is a next expected packet to be decompressed but the packet is a
             * valid HTTP response. So the gzip decompression ends here */
            if(expected_pkt)
            {
                expected_pkt = 0;
                (void)File_Decomp_Reset(sd->fd_state);
                ResetGzipState(sd->decomp_state);
                ResetRespState(&(sd->resp_state));
                sd->resp_state.flow_depth_excd = false;
            }
            while(hi_util_in_bounds(start, end, ptr))
            {
                if (isspace((int)*ptr))
                    break;
                ptr++;
            }

        }
    }
    else if (expected_pkt)
    {
        ptr = start;
    }
    else
    {
        return HI_SUCCESS;
    }

    /*If this is the next expected packet to be decompressed, send this packet
     * decompression */

    if (expected_pkt)
    {
        bool resp_eoh = false;
	if (ScPafEnabled() && !sd->resp_state.eoh_found)
	{
	    // check for EOH in this packet
            if ((resp_eoh = hi_paf_resp_eoh(p->ssnptr)))
	    {
	        sd->resp_state.eoh_found = true;
	        ptr += paf_bytes_curr; // jump to body offset
	        if (ptr == end)
	           return HI_SUCCESS;
	    }
        }

        if (hi_util_in_bounds(start, end, ptr))
        {
            iRet = hi_server_inspect_body(Session, sd, ptr, end, &body_ptr, resp_eoh | !sd->resp_state.data_extracted); 
        }
    }
    else
    {
        iRet = hi_server_extract_status_code(Session, start,ptr,end , &stat_code_ptr);

        if ( iRet != HI_OUT_OF_BOUNDS )
        {
            Server->response.status_code = stat_code_ptr.uri;
            Server->response.status_code_size = stat_code_ptr.uri_end - stat_code_ptr.uri;
            if ( (int)Server->response.status_code_size <= 0)
            {
                CLR_SERVER_STAT(Server);
            }
            else
            {
                iRet = hi_server_extract_status_msg(start, stat_code_ptr.uri_end ,
                        end, &stat_msg_ptr);

                if ( stat_msg_ptr.uri )
                {
                    Server->response.status_msg = stat_msg_ptr.uri;
                    Server->response.status_msg_size = stat_msg_ptr.uri_end - stat_msg_ptr.uri;
                    if ((int)Server->response.status_msg_size <= 0)
                    {
                        CLR_SERVER_STAT_MSG(Server);
                    }
                    {
                        header_ptr.range_flag = HTTP_RANGE_NONE;
                        ptr =  hi_server_extract_header(Session, ServerConf, &header_ptr,
                                            stat_msg_ptr.uri_end , end, parse_cont_encoding, parse_trans_encoding, sd );

                        if (header_ptr.range_flag != HTTP_RANGE_NONE)
                        {
                            Server->response.range_flag = header_ptr.range_flag;
                        } 
                        else
                        {
                            Server->response.range_flag = HTTP_RESP_RANGE_NONE;
                        }
                    }
                }
                else
                {
                    CLR_SERVER_STAT(Server);
                }
            }

            if (header_ptr.header.uri)
            {
                Server->response.header_raw = header_ptr.header.uri;
                Server->response.header_raw_size =
                    header_ptr.header.uri_end - header_ptr.header.uri;
                if(!Server->response.header_raw_size)
                {
                    CLR_SERVER_HEADER(Server);
                }
                else
                {
                    resp_header_size = (header_ptr.header.uri_end - p->data);
                    hi_stats.resp_headers++;
                    Server->response.header_norm = header_ptr.header.uri;
                    if (header_ptr.cookie.cookie)
                    {
                        hi_stats.resp_cookies++;
                        Server->response.cookie.cookie = header_ptr.cookie.cookie;
                        Server->response.cookie.cookie_end = header_ptr.cookie.cookie_end;
                        Server->response.cookie.next = header_ptr.cookie.next;
                    }
                    else
                    {
                        Server->response.cookie.cookie = NULL;
                        Server->response.cookie.cookie_end = NULL;
                        Server->response.cookie.next = NULL;
                    }
                    if (sd != NULL)
                    {
                        if( header_ptr.content_encoding.compress_fmt )
                        {
                            hi_stats.gzip_pkts++;

                            /* We've got gzip data - grab buffer from mempool and attach
                             * to session data if server is configured to do so */
                            if (sd->decomp_state == NULL)
                                SetGzipBuffers(sd, Session, p->ssnptr);

                            if (sd->decomp_state != NULL)
                            {
                                sd->decomp_state->decompress_data = 1;
                                sd->decomp_state->compress_fmt =
                                                            header_ptr.content_encoding.compress_fmt;
                            }

                        }
                        else
                        {
                            sd->resp_state.inspect_body = 1;
                        }
                                  
                        /*InitFileDecomp should not be called twice when decomp_state_stage is
                          MID as the objects getting initialized twice*/ 
                        if(( ServerConf->file_decomp_modes != 0 ) && (sd->fd_state == NULL))
                        {
                            InitFileDecomp(sd, Session,p->ssnptr);
                        }

                        sd->resp_state.last_pkt_contlen = (header_ptr.content_len.len != 0);
                        if(ServerConf->server_flow_depth == -1)
                            sd->resp_state.flow_depth_excd = true;
                        else
                        {
                            sd->resp_state.flow_depth_excd = false;
                            sd->resp_state.max_seq = seq_num +
                                        (header_ptr.header.uri_end - start)+ ServerConf->server_flow_depth;
                        }

                        if (p->packet_flags & PKT_STREAM_INSERT)
                        {
                            if ( ScPafEnabled() )
                            {
                                if ( p->packet_flags & PKT_PDU_TAIL )
                                    expected_pkt = 1;
                                else
                                    sd->resp_state.inspect_reassembled = 1;
                            }
                            else if (
                                header_ptr.content_len.cont_len_start &&
                                ((uint32_t)(end - (header_ptr.header.uri_end)) >= header_ptr.content_len.len))
                            {
                                /* change this when the api is fixed to flush correctly */
                                //stream_api->response_flush_stream(p);
                                expected_pkt = 1;
                            }
                            else
                                sd->resp_state.inspect_reassembled = 1;
                        }
                        else
                        {
                            if(p->packet_flags & PKT_REBUILT_STREAM)
                                sd->resp_state.inspect_reassembled = 1;

                            expected_pkt = 1;
                        }
                        if(expected_pkt)
                        {
                            sd->resp_state.next_seq = seq_num + p->dsize;

                            if(hi_util_in_bounds(start, end, header_ptr.header.uri_end))
                            {
                                iRet = hi_server_inspect_body(Session, sd, header_ptr.header.uri_end,
                                                                end, &body_ptr, PacketHasStartOfPDU(p) || !ScPafEnabled());
                            }
                        }
                    }
                }
            }
            else
            {
                CLR_SERVER_HEADER(Server);

            }
        }
        else
        {
            CLR_SERVER_STAT(Server);
        }
    }

    if( body_ptr.uri )
    {
        Server->response.body = body_ptr.uri;
        Server->response.body_size = body_ptr.uri_end - body_ptr.uri;
        Server->response.body_raw = body_ptr.uri;
        Server->response.body_raw_size = body_ptr.uri_end - body_ptr.uri;
        if( Server->response.body_size > 0)
        {
            if ( Server->response.body_size < sizeof(HttpDecodeBuf.data) )
            {
                alt_dsize = Server->response.body_size;
            }
            else
            {
                alt_dsize = sizeof(HttpDecodeBuf.data);
            }
            /* not checking if sd== NULL as the body_ptr.uri = NULL when sd === NULL in hi_server_inspect_body */
            if(sd->decomp_state && sd->decomp_state->decompress_data)
            {
                status = SafeMemcpy(HttpDecodeBuf.data, Server->response.body,
                                            alt_dsize, HttpDecodeBuf.data, HttpDecodeBuf.data + sizeof(HttpDecodeBuf.data));
                if( status != SAFEMEM_SUCCESS  )
                {
                    CLR_SERVER_HEADER(Server);
                    CLR_SERVER_STAT_MSG(Server);
                    CLR_SERVER_STAT(Server);
                    return HI_MEM_ALLOC_FAIL;
                }

                SetHttpDecode((uint16_t)alt_dsize);
                Server->response.body = HttpDecodeBuf.data;
                Server->response.body_size = HttpDecodeBuf.len;
                sd->log_flags |= HTTP_LOG_GZIP_DATA;
            }
            else
            {
                if(sd->resp_state.last_pkt_chunked)
                {
                    SetHttpDecode((uint16_t)alt_dsize);
                    Server->response.body = HttpDecodeBuf.data;
                    Server->response.body_size = HttpDecodeBuf.len;
                }
                else
                {
                    Server->response.body_size = alt_dsize;
                }
            }

            if ((get_decode_utf_state_charset(&(sd->utf_state)) != CHARSET_DEFAULT)
                    || (ServerConf->normalize_javascript && Server->response.body_size))
            {
                if ( Server->response.body_size < sizeof(HttpDecodeBuf.data) )
                {
                    alt_dsize = Server->response.body_size;
                }
                else
                {
                    alt_dsize = sizeof(HttpDecodeBuf.data);
                }
                Server->response.body_size = alt_dsize;
                SetHttpDecode((uint16_t)alt_dsize);
            }
        }

    }

#if defined(FEAT_OPEN_APPID)
    //copy over extracted headers for appId
    if ((ServerConf->appid_enabled))
    {
        HttpParsedHeaders headers;
        memset(&headers, 0, sizeof(headers));
        headers.via = header_ptr.via;
        headers.server = header_ptr.server;
        headers.xWorkingWith = header_ptr.xWorkingWith;
        headers.contentType = header_ptr.contentType;
        if (Server->response.status_code)
        {
            headers.responseCode.start = Server->response.status_code;
            headers.responseCode.len = Server->response.status_code_size;
        }

        /*callback into appId with header values extracted. */
        CallHttpHeaderProcessors(p, &headers);
        SnortPreprocFree((void*) headers.server.start, sizeof(uint8_t),
             PP_HTTPINSPECT, PP_MEM_CATEGORY_SESSION);
        SnortPreprocFree((void*) headers.via.start, sizeof(uint8_t),
             PP_HTTPINSPECT, PP_MEM_CATEGORY_SESSION);
        SnortPreprocFree((void*) headers.xWorkingWith.start, sizeof(uint8_t),
             PP_HTTPINSPECT, PP_MEM_CATEGORY_SESSION);
    }

#endif /* defined(FEAT_OPEN_APPID) */

    ApplyFlowDepth(ServerConf, p, sd, resp_header_size, 0, seq_num);

    return HI_SUCCESS;
}

int ServerInspection(HI_SESSION *Session, Packet *p, HttpSessionData *hsd)
{
    int iRet;

    if ((p->data == NULL) || (p->dsize == 0))
    {
        return HI_INVALID_ARG;
    }

#if defined(FEAT_OPEN_APPID)
    if ( Session->server_conf->inspect_response || Session->server_conf->appid_enabled)
#else
    if ( Session->server_conf->inspect_response )
#endif /* defined(FEAT_OPEN_APPID) */
    {
        iRet = HttpResponseInspection(Session, p, p->data, p->dsize, hsd);
    }
    else
    {
        iRet = IsHttpServerData(Session, p, hsd);
    }

    if (iRet)
    {
        return iRet;
    }

    return HI_SUCCESS;
}

int hi_server_inspection(void *S, Packet *p, HttpSessionData *hsd)
{
    HI_SESSION *Session;

    int iRet;

    if(!S )
    {
        return HI_INVALID_ARG;
    }

    Session = (HI_SESSION *)S;

    /*  IF Server->enable_xff is not enabled, we are not going to do anything related to
        Extradata list update. if XFF is not enabled req_id and resp_id will be zero always .
    */
    if (Session->server_conf->enable_xff)
    {
       if( ScPafEnabled() )
       {
	   uint8_t find_id;

           if(hsd->http_resp_id == XFF_MAX_PIPELINE_REQ)
              hsd->http_resp_id = 0;
           if( PacketHasStartOfPDU(p) )
           {
              hsd->http_resp_id++;
              if( hsd->tList_count != 0 )
                 hsd->tList_count--;
           }
           hsd->is_response = 1;

           find_id = (hsd->http_resp_id - 1 );
           if( !find_id )
               find_id = XFF_MAX_PIPELINE_REQ;

           if( (hsd->tList_start != NULL) && ( hsd->tList_start->tID == find_id ) )
               deleteNode_tList(hsd);
       }
    }

    /*
    **  Let's inspect the server response.
    */
    iRet = ServerInspection(Session, p, hsd);
    if (iRet)
    {
        return iRet;
    }

    return HI_SUCCESS;
}
