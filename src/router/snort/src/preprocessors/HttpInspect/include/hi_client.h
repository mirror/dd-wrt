/* $Id$ */
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

#ifndef __HI_CLIENT_H__
#define __HI_CLIENT_H__


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>

#include "snort_httpinspect.h"
#include "hi_include.h"
#include "hi_eo.h"
#include "hi_eo_events.h"
#define URI_END  99
#define POST_END 100
#define NO_URI   101

#define XFF_MODE_MASK      (0x000f)
#define XFF_EXFF_MASK      (0x000c)

#define TRUE_CLIENT_IP_HDR (0x01)
#define XFF_HDR            (0x02)
#define HDRS_BOTH          (0x03)
#define XFF_HEADERS        (0x04)  // Using xff_headers list
#define XFF_HEADERS_ACTIVE (0x08)  // Looking for highest precedence xff header
#define XFF_INIT (XFF_HEADERS | XFF_HEADERS_ACTIVE)

#define XFF_TOP_PRECEDENCE (1)
#define XFF_BOT_PRECEDENCE (255)

#define HTTP_RANGE_NONE                  0
#define HTTP_RANGE_WITH_FULL_CONTENT_REQ 1
#define RANGE_WITH_PARTIAL_CONTENT_REQ   2
#define RANGE_WITH_REQ_ERROR             3

typedef struct s_COOKIE_PTR
{
    const u_char *cookie;
    const u_char *cookie_end;
    struct s_COOKIE_PTR *next;
} COOKIE_PTR;


typedef struct s_CONTLEN_PTR
{
    const u_char *cont_len_start;
    const u_char *cont_len_end;
    uint32_t len;
}CONTLEN_PTR;

typedef struct s_CONT_ENCODING_PTR
{
    const u_char *cont_encoding_start;
    const u_char *cont_encoding_end;
    uint16_t compress_fmt;
}CONT_ENCODING_PTR;

typedef struct s_HEADER_FIELD_PTR
{
    COOKIE_PTR *cookie;
    CONTLEN_PTR *content_len;
    CONT_ENCODING_PTR *content_encoding;
} HEADER_FIELD_PTR;

/* These numbers were chosen to avoid conflicting with
 * the return codes in hi_return_codes.h */

/**
 **  This structure holds pointers to the different sections of an HTTP
 **  request.  We need to track where whitespace begins and ends, so we
 **  can evaluate the placement of the URI correctly.
 **
 **  For example,
 **
 **  GET     / HTTP/1.0
 **     ^   ^
 **   start end
 **
 **  The end space pointers are set to NULL if there is space until the end
 **  of the buffer.
 */

typedef struct s_URI_PTR
{
    const u_char *uri;                /* the beginning of the URI */
    const u_char *uri_end;            /* the end of the URI */
    const u_char *norm;               /* ptr to first normalization occurence */
    const u_char *ident;              /* ptr to beginning of the HTTP identifier */
    const u_char *first_sp_start;     /* beginning of first space delimiter */
    const u_char *first_sp_end;       /* end of first space delimiter */
    const u_char *second_sp_start;    /* beginning of second space delimiter */
    const u_char *second_sp_end;      /* end of second space delimiter */
    const u_char *param;              /* '?' (beginning of parameter field) */
    const u_char *delimiter;          /* HTTP URI delimiter (\r\n\) */
    const u_char *last_dir;           /* ptr to last dir, so we catch long dirs */
    const u_char *proxy;              /* ptr to the absolute URI */
}  URI_PTR;

typedef struct s_HEADER_PTR
{
    URI_PTR header;
    COOKIE_PTR cookie;
    CONTLEN_PTR content_len;
    CONT_ENCODING_PTR content_encoding;
    uint8_t range_flag;
    bool is_chunked;
#if defined(FEAT_OPEN_APPID)
    HEADER_LOCATION userAgent, referer, method, via, responseCode, server, xWorkingWith, contentType;
#endif /* defined(FEAT_OPEN_APPID) */
} HEADER_PTR;


typedef struct s_HI_CLIENT_REQ
{
    /*
    u_char *method;
    int  method_size;
    */

    const u_char *uri;
    const u_char *uri_norm;
    const u_char *post_raw;
    const u_char *post_norm;
    const u_char *header_raw;
    const u_char *header_norm;
    COOKIE_PTR cookie;
    const u_char *cookie_norm;
    const u_char *method_raw;

    u_int uri_size;
    u_int uri_norm_size;
    u_int post_raw_size;
    u_int post_norm_size;
    u_int header_raw_size;
    u_int header_norm_size;
    u_int cookie_norm_size;
    u_int method_size;

    /*
    u_char *param;
    u_int  param_size;
    u_int  param_norm;
    */

    /*
    u_char *ver;
    u_int  ver_size;

    u_char *hdr;
    u_int  hdr_size;

    u_char *payload;
    u_int  payload_size;
    */

    const u_char *pipeline_req;
    u_char method;
    uint16_t uri_encode_type;
    uint16_t header_encode_type;
    uint16_t cookie_encode_type;
    uint16_t post_encode_type;
    const u_char *content_type;
    const u_char *content_disp;
    uint8_t range_flag;

}  HI_CLIENT_REQ;

typedef struct s_HI_CLIENT
{
    HI_CLIENT_REQ request;
    int (*state)(void *, unsigned char, int);
    HI_CLIENT_EVENTS event_list;

}  HI_CLIENT;

typedef struct s_HI_CLIENT_HDR_ARGS
{
    HEADER_PTR *hdr_ptr;
    HEADER_FIELD_PTR *hdr_field_ptr;
    HttpSessionData *sd; 
    int strm_ins; 
    int hst_name_hdr;
    uint8_t true_clnt_xff;
    uint8_t prev_true_clnt_xff;
    uint8_t top_precedence;
    uint8_t new_precedence;
} HI_CLIENT_HDR_ARGS;

int hi_client_inspection(Packet *p, void *Session, HttpSessionData *hsd, int stream_ins);
int hi_client_init(HTTPINSPECT_GLOBAL_CONF *GlobalConf);

char **hi_client_get_field_names();

extern const u_char *proxy_start;
extern const u_char *proxy_end;

#endif
