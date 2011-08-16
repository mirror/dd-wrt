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
#ifdef ZLIB
#include <zlib.h>
#include "mempool.h"
extern MemPool *hi_gzip_mempool;
#endif

#include "hi_server.h"
#include "hi_ui_config.h"
#include "hi_return_codes.h"
#include "hi_si.h"
#include "hi_eo_log.h"
#include "bounds.h"
#include "detection_util.h"
#include "stream_api.h"

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

typedef int (*LOOKUP_FCN)(HI_SESSION *, const u_char *, const u_char *, const u_char **,
            URI_PTR *);
extern LOOKUP_FCN lookup_table[256];
extern int NextNonWhiteSpace(HI_SESSION *, const u_char *, const u_char *, const u_char **, URI_PTR *);
extern int CheckChunkEncoding(HI_SESSION *, const u_char *, const u_char *, const u_char **, u_char *, int , int, int *, int *);
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
extern const u_char *extract_http_content_length(HI_SESSION *, HTTPINSPECT_CONF *, const u_char *, const u_char *, const u_char *, HEADER_PTR *, HEADER_FIELD_PTR *) ;

static INLINE void ApplyFlowDepth(HTTPINSPECT_CONF *, Packet *, HttpSessionData *, int, int, uint32_t);
#define CLR_SERVER_HEADER(Server) \
    do { \
            Server->response.header_raw = NULL;\
            Server->response.header_raw_size = 0;\
            Server->response.header_norm = NULL; \
            Server->response.header_norm_size = 0 ;\
            Server->response.cookie.cookie = NULL;\
            Server->response.cookie.cookie_end = NULL;\
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

#define CLR_SERVER_BODY(Server)\
    do { \
            Server->response.body = NULL;\
            Server->response.body_size = 0;\
    }while(0);

static INLINE void clearHttpRespBuffer(HI_SERVER *Server)
{
    CLR_SERVER_HEADER(Server);
    CLR_SERVER_STAT(Server);
    CLR_SERVER_BODY(Server);
}

static INLINE const u_char *MovePastDelims(const u_char *start, const u_char *end,const u_char *ptr)
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
                sd->resp_state.is_max_seq = 1;
                sd->resp_state.max_seq = seq_num + ServerConf->server_flow_depth;
            }
            p->packet_flags |= PKT_HTTP_DECODE;
            ApplyFlowDepth(ServerConf, p, sd, 0, 1, seq_num);
            return HI_SUCCESS;
        }
    }
    else
    {
        return HI_SUCCESS;
    }


    return HI_SUCCESS;
}

static INLINE int hi_server_extract_status_msg( const u_char *start, const u_char *ptr, 
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


static INLINE int hi_server_extract_status_code(HI_SESSION *Session, const u_char *start, const u_char *ptr, 
        const u_char *end, URI_PTR *result)
{
    int iRet = HI_SUCCESS;
    SkipBlankSpace(start,end,&ptr);

    result->uri = ptr;

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

/* Grab the argument of "charset=foo" from a Content-Type header */
static INLINE const u_char *extract_http_content_type_charset(HI_SESSION *Session,
        HttpSessionData *hsd, const u_char *p, const u_char *start, const u_char *end )
{
    const char *crlf;

    if (hsd == NULL)
        return p;

    /* find end of Content-Type header */
    crlf = SnortStrnStr((const char*)p, (int)(end - p), "\n");

    if (crlf)
    {
        char *charset;

        /* search for "charset=utf-" within the header */
        charset = (char *) SnortStrnStr((const char*)p, (int)(crlf - (const char*)p), "charset=utf-");
        /* if found, check the argument after "charset=utf-" */
        if (charset)
        {
            size_t cmplen;

            charset += 12; /* length of "charset=utf-" */
            cmplen = (size_t) ((char *) crlf - charset);

            if (cmplen > 4)
                cmplen = 4;

            if ( !strncmp(charset, "16le", cmplen) )
                set_decode_utf_state_charset(&(hsd->utf_state), CHARSET_UTF16LE);
            else if ( !strncmp(charset, "16be", cmplen) )
                set_decode_utf_state_charset(&(hsd->utf_state), CHARSET_UTF16BE);
            else if ( !strncmp(charset, "32le", cmplen) )
                set_decode_utf_state_charset(&(hsd->utf_state), CHARSET_UTF32LE);
            else if ( !strncmp(charset, "32be", cmplen) )
                set_decode_utf_state_charset(&(hsd->utf_state), CHARSET_UTF32BE);
            else
            {
                if (cmplen > 1)
                    cmplen = 1;

                if ( !strncmp(charset, "7", cmplen) )
                {
                    set_decode_utf_state_charset(&(hsd->utf_state), CHARSET_UTF7);
                    if(hi_eo_generate_event(Session, HI_EO_SERVER_UTF7))
                    {
                        hi_eo_server_event_log(Session, HI_EO_SERVER_UTF7, NULL, NULL);
                    }
                }
            }
        }
        else
            set_decode_utf_state_charset(&(hsd->utf_state), CHARSET_DEFAULT);

        p = (const u_char*)crlf;
    }

    return p;
}

#ifdef ZLIB
static INLINE const u_char *extract_http_content_encoding(HTTPINSPECT_CONF *ServerConf, 
        const u_char *p, const u_char *start, const u_char *end, HEADER_PTR *header_ptr, 
        HEADER_FIELD_PTR *header_field_ptr)
{
    const u_char *crlf;
    int space_present = 0;
    if (header_ptr->content_encoding.cont_encoding_start)
    {
        header_ptr->header.uri_end = p;
        header_ptr->content_encoding.compress_fmt = 0;
        return p;
    }
    else
    {
        header_field_ptr->content_encoding = &header_ptr->content_encoding;
        p = p + HTTPRESP_HEADER_LENGTH__CONTENT_ENCODING;
    }
    SkipBlankSpace(start,end,&p);
    if(hi_util_in_bounds(start, end, p) && *p == ':')
    {
        p++;
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
                                {
                                    header_field_ptr->content_encoding->cont_encoding_start=
                                        header_field_ptr->content_encoding->cont_encoding_end = NULL;
                                    header_field_ptr->content_encoding->compress_fmt = 0;
                                    return p;
                                }
                            }
                            else
                            {
                                header_field_ptr->content_encoding->cont_encoding_start=
                                    header_field_ptr->content_encoding->cont_encoding_end = NULL;
                                header_field_ptr->content_encoding->compress_fmt = 0;
                                return p;
                            }
                        }
                        else
                            break;
                    }
                }
                else if(isalpha((int)*p))
                {
                    header_field_ptr->content_encoding->cont_encoding_start = p;
                    while(hi_util_in_bounds(start, end, p) && *p!='\n' )
                    {
                        if(IsHeaderFieldName(p, end, HTTPRESP_HEADER_NAME__GZIP, HTTPRESP_HEADER_LENGTH__GZIP) ||
                                IsHeaderFieldName(p, end, HTTPRESP_HEADER_NAME__XGZIP, HTTPRESP_HEADER_LENGTH__XGZIP))
                        {
                            header_field_ptr->content_encoding->compress_fmt |= HTTP_RESP_COMPRESS_TYPE__GZIP;
                            p = p + HTTPRESP_HEADER_LENGTH__GZIP;
                            continue;
                        }
                        else if(IsHeaderFieldName(p, end, HTTPRESP_HEADER_NAME__DEFLATE, HTTPRESP_HEADER_LENGTH__DEFLATE))
                        {
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
                {
                    header_field_ptr->content_encoding->cont_encoding_start=
                        header_field_ptr->content_encoding->cont_encoding_end = NULL;
                    header_field_ptr->content_encoding->compress_fmt = 0;
                    return p;
                }
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
#endif


static INLINE const u_char *extractHttpRespHeaderFieldValues(HTTPINSPECT_CONF *ServerConf, 
        const u_char *p, const u_char *offset, const u_char *start, 
        const u_char *end, HEADER_PTR *header_ptr, 
        HEADER_FIELD_PTR *header_field_ptr, int parse_cont_encoding, HttpSessionData *hsd,
        HI_SESSION *Session)
{
    if (((p - offset) == 0) && ((*p == 'S') || (*p == 's')))
    {
        /* Search for 'Cookie' at beginning, starting from current *p */
        if ( ServerConf->enable_cookie && 
                IsHeaderFieldName(p, end, HTTPRESP_HEADER_NAME__COOKIE, 
                    HTTPRESP_HEADER_LENGTH__COOKIE))
        {
            p = extract_http_cookie(p, end, header_ptr, header_field_ptr);
        }
    }
    else if (((p - offset) == 0) && ((*p == 'C') || (*p == 'c')))
    {
        if ( IsHeaderFieldName(p, end, HTTPRESP_HEADER_NAME__CONTENT_TYPE,
                               HTTPRESP_HEADER_LENGTH__CONTENT_TYPE) && ServerConf->normalize_utf)
        {
            p = extract_http_content_type_charset(Session, hsd, p, start, end);
        }

#ifdef ZLIB
        else if ( IsHeaderFieldName(p, end, HTTPRESP_HEADER_NAME__CONTENT_ENCODING, 
                    HTTPRESP_HEADER_LENGTH__CONTENT_ENCODING) && ServerConf->extract_gzip &&
                    parse_cont_encoding) 
        {
            p = extract_http_content_encoding(ServerConf, p, start, end, header_ptr, header_field_ptr );
        }
#endif
        else if ( IsHeaderFieldName(p, end, HTTPRESP_HEADER_NAME__CONTENT_LENGTH, 
                HTTPRESP_HEADER_LENGTH__CONTENT_LENGTH) )
        {
            p = extract_http_content_length(Session, ServerConf, p, start, end, header_ptr, header_field_ptr );
        }
    }
    return p;
}


static INLINE const u_char *hi_server_extract_header(
        HI_SESSION *Session, HTTPINSPECT_CONF *ServerConf, 
            HEADER_PTR *header_ptr, const u_char *start, 
            const u_char *end, int parse_cont_encoding,
            HttpSessionData *hsd)
{
    const u_char *p;
    const u_char *offset;
    HEADER_FIELD_PTR header_field_ptr ;

    if(!start || !end)
        return NULL;

    p = start;

    offset = (u_char*)p;

    header_ptr->header.uri = p;
    header_ptr->header.uri_end = end;
    header_ptr->content_encoding.compress_fmt = 0;
    header_ptr->content_len.len = 0;

    while (hi_util_in_bounds(start, end, p))
    {
        if(*p == '\n')
        {
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
                        return p;
                    }
                }
                else if(*p == '\n')
                {
                    p++;
                    header_ptr->header.uri_end = p;
                    return p;
                }
            }
            else if ( (p = extractHttpRespHeaderFieldValues(ServerConf, p, offset, 
                            start, end, header_ptr, &header_field_ptr, 
                            parse_cont_encoding, hsd, Session)) == end)
            {
                return end;
            }

        }
        else if( (p == header_ptr->header.uri) && 
                (p = extractHttpRespHeaderFieldValues(ServerConf, p, offset, 
                          start, end, header_ptr, &header_field_ptr,
                          parse_cont_encoding, hsd, Session)) == end)
        {
            return end;
        }
        if ( *p == '\n') continue;
        p++;
    }

    header_ptr->header.uri_end = p;
    return p;
}

static INLINE int hi_server_extract_body(
                        HI_SESSION *Session, HttpSessionData *sd,
                        const u_char *ptr, const u_char *end, URI_PTR *result)
{
    HTTPINSPECT_CONF *ServerConf;
    const u_char *start = ptr;
    int iRet = HI_SUCCESS;
    const u_char *post_end = end; 
    int chunk_size = 0;
    int chunk_read = 0;
    int bytes_to_read = 0;
    ServerConf = Session->server_conf;

    switch(ServerConf->server_flow_depth)
    {
        case -1: 
            result->uri = result->uri_end = NULL;
            return iRet;
        case 0:
            break;
        default:
            if(sd->resp_state.flow_depth_read < ServerConf->server_flow_depth)
            {
                bytes_to_read = ServerConf->server_flow_depth - sd->resp_state.flow_depth_read;
                if((end-ptr) > bytes_to_read )
                {
                    end = ptr + bytes_to_read;
                }
                sd->resp_state.flow_depth_read +=bytes_to_read;
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
        if( ServerConf->chunk_length )
        {
            if(CheckChunkEncoding(Session, start, end, &post_end, (u_char *)DecodeBuffer.data, sizeof(DecodeBuffer.data),
                                    sd->resp_state.last_chunk_size, &chunk_size, &chunk_read ) == 1)
            {
                sd->resp_state.last_chunk_size = chunk_size;
                sd->resp_state.last_pkt_chunked = 1;
                result->uri = (u_char *)DecodeBuffer.data;
                result->uri_end = result->uri + chunk_read;
                return iRet;
            }
            else
            {
                if(!(sd->resp_state.last_pkt_chunked))
                {
                    if(hi_eo_generate_event(Session, HI_EO_SERVER_NO_CONTLEN))
                    {
                        hi_eo_server_event_log(Session, HI_EO_SERVER_NO_CONTLEN, NULL, NULL);
                    }
                }
                else
                {
                    sd->resp_state.last_pkt_chunked = 0;
                    sd->resp_state.last_chunk_size = 0;
                }
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

#ifdef ZLIB
static void SetGzipBuffers(HttpSessionData *hsd, HI_SESSION *session)
{
    if ((hsd != NULL) && (hsd->decomp_state == NULL)
            && (session != NULL) && (session->server_conf != NULL)
            && (session->global_conf != NULL) && session->server_conf->extract_gzip)
    {
        MemBucket *bkt = mempool_alloc(hi_gzip_mempool);

        if (bkt != NULL)
        {
            hsd->decomp_state = (DECOMPRESS_STATE *)SnortAlloc(sizeof(DECOMPRESS_STATE));
            hsd->decomp_state->gzip_bucket = bkt;
            hsd->decomp_state->compr_depth = session->global_conf->compr_depth;
            hsd->decomp_state->decompr_depth = session->global_conf->decompr_depth;
            hsd->decomp_state->compr_buffer = (unsigned char *)bkt->data;
            hsd->decomp_state->decompr_buffer = (unsigned char *)bkt->data + session->global_conf->compr_depth;
            hsd->decomp_state->inflate_init = 0;
        }
    }
}

int uncompress_gzip ( u_char *dest, int destLen, u_char *source, 
        int sourceLen, HttpSessionData *sd, int *total_bytes_read, int compr_fmt)
{
    z_stream stream;
    int err;
    int iRet = HI_SUCCESS;

   stream = sd->decomp_state->d_stream;

   stream.next_in = (Bytef*)source;
   stream.avail_in = (uInt)sourceLen;
   if ((uLong)stream.avail_in != (uLong)sourceLen) 
   {
       sd->decomp_state->d_stream = stream;
       return HI_FATAL_ERR;
   }

   stream.next_out = dest;
   stream.avail_out = (uInt)destLen;
   if ((uLong)stream.avail_out != (uLong)destLen)
   { 
       sd->decomp_state->d_stream = stream;
       return HI_FATAL_ERR;
   }


   if(!sd->decomp_state->inflate_init)
   {
       sd->decomp_state->inflate_init = 1;
       stream.zalloc = (alloc_func)0;
       stream.zfree = (free_func)0;
       if(compr_fmt & HTTP_RESP_COMPRESS_TYPE__DEFLATE)
           err = inflateInit2(&stream, DEFLATE_WBITS);
       else 
           err = inflateInit2(&stream, GZIP_WBITS);
       if (err != Z_OK) 
       {
           sd->decomp_state->d_stream = stream;
           return HI_FATAL_ERR;
       }
   }
   else
   {
       stream.total_in = 0;
       stream.total_out =0;
   }


   err = inflate(&stream, Z_STREAM_END);
   if ((err != Z_STREAM_END) && (err !=Z_OK)) {
       /* If some of the compressed data is decompressed we need to provide that for detection */
       if( stream.total_out > 0)
       {
           *total_bytes_read = stream.total_out;
           iRet = HI_NONFATAL_ERR;
       }
       else
           iRet = HI_FATAL_ERR;
       inflateEnd(&stream);
       sd->decomp_state->d_stream = stream;
       return iRet;
   }
   *total_bytes_read = stream.total_out;
   sd->decomp_state->d_stream = stream;
   return HI_SUCCESS;
}

static INLINE int hi_server_decompress(HI_SESSION *Session, HttpSessionData *sd, const u_char *ptr, 
        const u_char *end, URI_PTR *result)
{
    const u_char *start = ptr;
    int rawbuf_size = end - ptr;
    int iRet = HI_SUCCESS;
    int zRet = HI_FATAL_ERR;
    int compr_depth, decompr_depth;
    int compr_bytes_read, decompr_bytes_read;
    int compr_avail, decompr_avail;
    int total_bytes_read = 0;
    int chunk_size = 0;
    int chunk_read = 0;

    u_char *compr_buffer;
    u_char *decompr_buffer;
    compr_depth = sd->decomp_state->compr_depth;
    decompr_depth = sd->decomp_state->decompr_depth;
    compr_bytes_read = sd->decomp_state->compr_bytes_read;
    decompr_bytes_read = sd->decomp_state->decompr_bytes_read;
    compr_buffer = sd->decomp_state->compr_buffer;
    decompr_buffer = sd->decomp_state->decompr_buffer;

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

    /* Apply the server flow depth
     * If the server flow depth is set then we need to decompress only upto the 
     * server flow depth
     */
    switch ( Session->server_conf->server_flow_depth)
    {
        case -1:
            decompr_avail=0;
            break;
        case 0:
            break;
        default:
            if(sd->resp_state.flow_depth_read < Session->server_conf->server_flow_depth)
            {
                if(decompr_avail > (Session->server_conf->server_flow_depth - sd->resp_state.flow_depth_read))
                    decompr_avail = Session->server_conf->server_flow_depth - sd->resp_state.flow_depth_read;
            }
            else
            {
                decompr_avail = 0;
            }
            break;
    }

    if(compr_avail <=0 || decompr_avail <=0 ||
            (!compr_buffer) || (!decompr_buffer))
    {
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
        if(CheckChunkEncoding(Session, start, end, NULL, compr_buffer, compr_avail, 
                    sd->resp_state.last_chunk_size, &chunk_size, &chunk_read ) == 1)
        {
            sd->resp_state.last_chunk_size = chunk_size;
            compr_avail = chunk_read;
            zRet = uncompress_gzip(decompr_buffer,decompr_avail,compr_buffer, compr_avail, sd, &total_bytes_read,
                                    sd->decomp_state->compress_fmt);
        }
        else
        {
            /* No Content-Length or Transfer-Encoding : chunked */
            if(hi_eo_generate_event(Session, HI_EO_SERVER_NO_CONTLEN))
            {
                hi_eo_server_event_log(Session, HI_EO_SERVER_NO_CONTLEN, NULL, NULL);
            }

            memcpy(compr_buffer, ptr, compr_avail);
            zRet = uncompress_gzip(decompr_buffer,decompr_avail,compr_buffer, compr_avail, sd, 
                    &total_bytes_read, sd->decomp_state->compress_fmt);
        }
    }
    else
    {
        memcpy(compr_buffer, ptr, compr_avail);
        zRet = uncompress_gzip(decompr_buffer,decompr_avail,compr_buffer, compr_avail, sd, 
                &total_bytes_read, sd->decomp_state->compress_fmt);
    }
    
    sd->decomp_state->compr_bytes_read += compr_avail;
    hi_stats.compr_bytes_read += compr_avail;

    if((zRet == HI_SUCCESS) || (zRet == HI_NONFATAL_ERR))
    {
        if(decompr_buffer)
        {
            result->uri = decompr_buffer;
            if ( total_bytes_read < decompr_avail )
            {
                result->uri_end = decompr_buffer + total_bytes_read;
                sd->decomp_state->decompr_bytes_read += total_bytes_read;
                sd->resp_state.flow_depth_read += total_bytes_read;
                hi_stats.decompr_bytes_read += total_bytes_read;
            }
            else
            {
                result->uri_end = decompr_buffer + decompr_avail;
                sd->decomp_state->decompr_bytes_read += decompr_avail;
                sd->resp_state.flow_depth_read += decompr_avail;
                hi_stats.decompr_bytes_read += decompr_avail;
            }
        }
    }
    else
    {
        ResetGzipState(sd->decomp_state);
        ResetRespState(&(sd->resp_state));
    }

    return iRet;


}
#endif

static INLINE int hi_server_inspect_body(HI_SESSION *Session, HttpSessionData *sd, const u_char *ptr,
                        const u_char *end, URI_PTR *result)
{
    int iRet = HI_SUCCESS;

    result->uri = ptr;
    result->uri_end = end;
    if(!Session || !sd )
    {
        if ((sd != NULL))
        {
#ifdef ZLIB
            ResetGzipState(sd->decomp_state);
#endif
            ResetRespState(&(sd->resp_state));
            return HI_INVALID_ARG;
        }
    }

#ifdef ZLIB
    if((sd->decomp_state != NULL) && sd->decomp_state->decompress_data)
    {
        iRet = hi_server_decompress(Session, sd, ptr, end, result);
    }
    else
#endif
    {
        result->uri = ptr;
        result->uri_end = end;
        iRet = hi_server_extract_body(Session, sd, ptr, end, result);
    }

    return iRet;

}
static INLINE void ApplyFlowDepth(HTTPINSPECT_CONF *ServerConf, Packet *p, 
        HttpSessionData *sd, int resp_header_size, int expected, uint32_t seq_num)
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
            if(sd->resp_state.is_max_seq )
            {
                if(SEQ_GEQ((sd->resp_state.max_seq), seq_num))
                {
                    if(p->dsize > (sd->resp_state.max_seq- seq_num))
                    {
                        SetDetectLimit(p, (sd->resp_state.max_seq-seq_num));
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
                    SetDetectLimit(p, resp_header_size);
                    return;
                }
            }
            else
            {
                if(expected)
                {
                    if(p->dsize > ServerConf->server_flow_depth)
                    {
                        SetDetectLimit(p, ServerConf->server_flow_depth);
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
                    SetDetectLimit(p, 0);
                    return;
                }
            }

        }
        else
        {
            SetDetectLimit(p, p->dsize);
        }
    }
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
#ifdef ZLIB
    int parse_cont_encoding = 1;
    int status;
#endif
    int expected_pkt = 0;
    int alt_dsize;
    uint32_t seq_num = 0;

    if (!Session || !p || !data || (dsize == 0))
        return HI_INVALID_ARG;

    ServerConf = Session->server_conf;
    if(!ServerConf)
        return HI_INVALID_ARG;


    Server = &(Session->server);
    clearHttpRespBuffer(Server);

    seq_num = GET_PKT_SEQ(p);

    if ( (sd != NULL) )
    {
        /* If the previously inspected packet in this session identified as a body
         * and if the packets are stream inserted wait for reassembled */
        if (sd->resp_state.inspect_reassembled)
        {
            if(p->packet_flags & PKT_STREAM_INSERT)
            {
#ifdef ZLIB
                parse_cont_encoding = 0;
#endif
                not_stream_insert = 0;
            }
        }
        /* If this packet is the next expected packet to be inspected and is out of sequence
         * clear out the resp state*/
#ifdef ZLIB
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
                ResetGzipState(sd->decomp_state);
                ResetRespState(&(sd->resp_state));
            }
        }
        else
#endif
        if(sd->resp_state.inspect_body && not_stream_insert)
        {
            /* If the server flow depth is 0 then we need to check if the packet
             * is in sequence
             */
            if(!ServerConf->server_flow_depth)
            {
                if( sd->resp_state.next_seq &&
                        (seq_num == sd->resp_state.next_seq) )
                {
                    sd->resp_state.next_seq = seq_num + p->dsize;
                    expected_pkt = 1;
                }
                else
                {
#ifdef ZLIB
                    ResetGzipState(sd->decomp_state);
#endif
                    ResetRespState(&(sd->resp_state));
                }
            }
            else 
            {
                /*Check if the sequence number of the packet is within the allowed
                 * flow_depth
                 */
                if( (sd->resp_state.is_max_seq) && 
                        SEQ_LT(seq_num, (sd->resp_state.max_seq)))
                {
                    expected_pkt = 1;
                }
                else
                {
#ifdef ZLIB
                    ResetGzipState(sd->decomp_state);
#endif
                    ResetRespState(&(sd->resp_state));
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

    len = end - ptr;
    if ( len > 4 )
    {
        if(!IsHttpVersion(&ptr, end))
        { 
            if(expected_pkt)
            {
                ptr = start;
                p->packet_flags |= PKT_HTTP_DECODE;
            }
            else
            {
                p->packet_flags |= PKT_HTTP_DECODE;
                ApplyFlowDepth(ServerConf, p, sd, resp_header_size, 0, seq_num);
                if ( not_stream_insert && (sd != NULL))
                {
#ifdef ZLIB
                    ResetGzipState(sd->decomp_state);
#endif
                    ResetRespState(&(sd->resp_state));
                }
                CLR_SERVER_HEADER(Server);
                return HI_SUCCESS;
            }
        }
        else
        {
            p->packet_flags |= PKT_HTTP_DECODE;
            /* This is a next expected packet to be decompressed but the packet is a
             * valid HTTP response. So the gzip decompression ends here */
            if(expected_pkt)
            {
                expected_pkt = 0;
#ifdef ZLIB
                ResetGzipState(sd->decomp_state);
#endif
                ResetRespState(&(sd->resp_state));
            }
            while(hi_util_in_bounds(start, end, ptr))
            {
                if (isspace((int)*ptr))
                    break;
                ptr++;
            }

        }
    }
    else if (!expected_pkt)
    {
        return HI_SUCCESS;
    }

    /*If this is the next expected packet to be decompressed, send this packet 
     * decompression */

    if (expected_pkt)
    {
        if (hi_util_in_bounds(start, end, ptr))
        {
            iRet = hi_server_inspect_body(Session, sd, ptr, end, &body_ptr);
        }
    }
    else
    {
        iRet = hi_server_extract_status_code(Session, start,ptr,end , &stat_code_ptr);

        if ( iRet == STAT_END )
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
                        CLR_SERVER_STAT(Server);
                    }
                    else
                    {
#ifdef ZLIB
                        ptr =  hi_server_extract_header(Session, ServerConf, &header_ptr,
                                            stat_msg_ptr.uri_end , end, parse_cont_encoding, sd );
#else
                        /* We dont need the content-encoding header when zlib is not enabled */
                        ptr =  hi_server_extract_header(Session, ServerConf, &header_ptr,
                                    stat_msg_ptr.uri_end , end, 0, sd );
#endif
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
#ifdef ZLIB
                        if( header_ptr.content_encoding.compress_fmt )
                        {
                            hi_stats.gzip_pkts++;

                            /* We've got gzip data - grab buffer from mempool and attach
                             * to session data if server is configured to do so */
                            if (sd->decomp_state == NULL)
                                SetGzipBuffers(sd, Session);

                            if (sd->decomp_state != NULL)
                            {
                                sd->decomp_state->decompress_data = 1;
                                sd->decomp_state->compress_fmt =
                                                            header_ptr.content_encoding.compress_fmt;
                            }

                        }
                        else
#endif
                        {
                            sd->resp_state.inspect_body = 1;
                        }

                        sd->resp_state.last_pkt_contlen = header_ptr.content_len.len;
                        if(ServerConf->server_flow_depth == -1)
                            sd->resp_state.is_max_seq = 0;
                        else
                        {
                            sd->resp_state.is_max_seq = 1;
                            sd->resp_state.max_seq = seq_num +
                                        (header_ptr.header.uri_end - start)+ ServerConf->server_flow_depth;
                        }

                        if (p->packet_flags & PKT_STREAM_INSERT)
                        {
                            if(header_ptr.content_len.cont_len_start && ((end - (header_ptr.header.uri_end)) >= header_ptr.content_len.len))
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
                                                                end, &body_ptr);
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
        if( Server->response.body_size > 0)
        {
            if ( Server->response.body_size < sizeof(DecodeBuffer.data) )
            {
                alt_dsize = Server->response.body_size;
            }
            else
            {
                alt_dsize = sizeof(DecodeBuffer.data);
            }
#ifdef ZLIB
            if(sd->decomp_state && sd->decomp_state->decompress_data)
            {
                status = SafeMemcpy(DecodeBuffer.data, Server->response.body,
                                            alt_dsize, DecodeBuffer.data, DecodeBuffer.data + sizeof(DecodeBuffer.data));
                if( status != SAFEMEM_SUCCESS  )
                    return HI_MEM_ALLOC_FAIL;
                p->data_flags |= DATA_FLAGS_GZIP;
                SetAltDecode(p, alt_dsize);
                SetDetectLimit(p, alt_dsize);
            }
            else
#endif
            {
                if(sd->resp_state.last_pkt_chunked)
                {
                    p->data_flags |= DATA_FLAGS_RESP_BODY;
                    SetAltDecode(p, alt_dsize);
                    SetDetectLimit(p, alt_dsize);
                }
                else
                {
                    p->data_flags |= DATA_FLAGS_RESP_BODY;
                    p->packet_flags |= PKT_HTTP_RESP_BODY;
                    SetDetectLimit(p, (alt_dsize + resp_header_size));
                }
            }

            if (get_decode_utf_state_charset(&(sd->utf_state)) != CHARSET_DEFAULT)
            {
                if ( Server->response.body_size < sizeof(DecodeBuffer.data) )
                {
                    alt_dsize = Server->response.body_size;
                }           
                else
                {
                    alt_dsize = sizeof(DecodeBuffer.data);
                }
                SetDetectLimit(p, alt_dsize);
                SetAltDecode(p, alt_dsize);
            }
        }
        
    }
    else
    {
        /* There is no body to the HTTP response.
         * In this case we need to inspect the entire HTTP response header.
         */
        ApplyFlowDepth(ServerConf, p, sd, resp_header_size, 1, seq_num);
    }

    return HI_SUCCESS;
}

int ServerInspection(HI_SESSION *Session, Packet *p, HttpSessionData *hsd)
{
    int iRet;

    if ((p->data == NULL) || (p->dsize == 0))
    {
        return HI_INVALID_ARG;
    }

    if ( Session->server_conf->inspect_response )
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
