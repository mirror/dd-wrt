/****************************************************************************
 *
 * Copyright (C) 2014 Cisco and/or its affiliates. All rights reserved.
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

#ifndef __SNORT_HTTPINSPECT_H__
#define __SNORT_HTTPINSPECT_H__

#include "decode.h"
#include "session_api.h"
#include "stream_api.h"
#include "hi_ui_config.h"
#include "util_utf.h"
#include "detection_util.h"
#include "mempool.h"
#include "str_search.h"
#include "util_jsnorm.h"

#include <zlib.h>

extern MemPool *http_mempool;
extern MemPool *mime_decode_mempool;
extern MemPool *mime_log_mempool;

extern DataBuffer HttpDecodeBuf;

/**
**  The definition of the configuration separators in the snort.conf
**  configure line.
*/
#define CONF_SEPARATORS " \t\n\r"
#define MAX_METHOD_LEN  256

/*
**  These are the definitions of the parser section delimiting
**  keywords to configure HttpInspect.  When one of these keywords
**  are seen, we begin a new section.
*/
#define GLOBAL        "global"
#define GLOBAL_SERVER "global_server"
#define SERVER        "server"
#define CLIENT        "client"

#define DEFAULT_HTTP_MEMCAP 150994944 /* 144 MB */
#define MIN_HTTP_MEMCAP     2304
#define MAX_HTTP_MEMCAP     603979776 /* 576 MB */
#define MAX_URI_EXTRACTED   2048
#define MAX_HOSTNAME        256


#define DEFAULT_MAX_GZIP_MEM 838860
#define GZIP_MEM_MIN    3276
#define MAX_GZIP_DEPTH    65535
#define DEFAULT_COMP_DEPTH 1460
#define DEFAULT_DECOMP_DEPTH 2920

#define DEFLATE_RAW_WBITS -15
#define DEFLATE_WBITS   15
#define GZIP_WBITS      31


typedef enum _HttpRespCompressType
{
    HTTP_RESP_COMPRESS_TYPE__GZIP     = 0x00000001,
    HTTP_RESP_COMPRESS_TYPE__DEFLATE  = 0x00000002

} _HttpRespCompressType;

typedef struct s_DECOMPRESS_STATE
{
    uint8_t inflate_init;
    int compr_bytes_read;
    int decompr_bytes_read;
    int compr_depth;
    int decompr_depth;
    uint16_t compress_fmt;
    uint8_t decompress_data;
    z_stream d_stream;
    MemBucket *bkt;
    bool deflate_initialized;

} DECOMPRESS_STATE;

typedef struct s_HTTP_RESP_STATE
{
    uint8_t inspect_body;
    uint8_t inspect_reassembled;
    uint8_t last_pkt_contlen;
    uint8_t last_pkt_chunked;
    uint32_t next_seq;
    uint32_t chunk_remainder;
    int data_extracted;
    uint32_t max_seq;
    bool flow_depth_excd;
}HTTP_RESP_STATE;

typedef struct s_HTTP_LOG_STATE
{
    uint32_t uri_bytes;
    uint32_t hostname_bytes;
    MemBucket *log_bucket;
    uint8_t *uri_extracted;
    uint8_t *hostname_extracted;
}HTTP_LOG_STATE;

typedef struct _HttpSessionData
{
    uint64_t event_flags;
    HTTP_RESP_STATE resp_state;
    DECOMPRESS_STATE *decomp_state;
    HTTP_LOG_STATE *log_state;
    sfip_t *true_ip;
    decode_utf_state_t utf_state;
    uint8_t log_flags;
    uint8_t cli_small_chunk_count;
    uint8_t srv_small_chunk_count;
    MimeState *mime_ssn;
    fd_session_p_t fd_state;
} HttpSessionData;

typedef struct _HISearch
{
    char *name;
    int   name_len;

} HISearch;

typedef struct _HiSearchToken               
{   
    char *name;
    int   name_len;
    int   search_id;
} HiSearchToken;

typedef struct _HISearchInfo
{
    int id;
    int index;
    int length;
} HISearchInfo;


#define COPY_URI 1
#define COPY_HOSTNAME 2

#define HTTP_LOG_URI        0x0001
#define HTTP_LOG_HOSTNAME   0x0002
#define HTTP_LOG_GZIP_DATA  0x0004
#define HTTP_LOG_JSNORM_DATA  0x0008

typedef enum _HiSearchIdEnum
{
    HI_JAVASCRIPT = 0,
    HI_LAST
} HiSearchId;

typedef enum _HtmlSearchIdEnum
{
    HTML_JS = 0,
    HTML_EMA,
    HTML_VB,
    HTML_LAST
} HtmlSearchId;

extern void *hi_javascript_search_mpse;
extern void *hi_htmltype_search_mpse;
extern HISearch hi_js_search[HI_LAST];
extern HISearch hi_html_search[HTML_LAST];
extern HISearch *hi_current_search;
extern HISearchInfo hi_search_info;

void ApplyFlowDepth(HTTPINSPECT_CONF *, Packet *, HttpSessionData *, int, int, uint32_t);



int SnortHttpInspect(HTTPINSPECT_GLOBAL_CONF *GlobalConf, Packet *p);
int ProcessGlobalConf(HTTPINSPECT_GLOBAL_CONF *, char *, int);
int PrintGlobalConf(HTTPINSPECT_GLOBAL_CONF *);
int ProcessUniqueServerConf(struct _SnortConfig *, HTTPINSPECT_GLOBAL_CONF *, char *, int);
int HttpInspectInitializeGlobalConfig(HTTPINSPECT_GLOBAL_CONF *, char *, int);
HttpSessionData * SetNewHttpSessionData(Packet *, void *);
void FreeHttpSessionData(void *data);
int GetHttpTrueIP(void *data, uint8_t **buf, uint32_t *len, uint32_t *type);
int GetHttpGzipData(void *data, uint8_t **buf, uint32_t *len, uint32_t *type);
int GetHttpJSNormData(void *data, uint8_t **buf, uint32_t *len, uint32_t *type);
int GetHttpUriData(void *data, uint8_t **buf, uint32_t *len, uint32_t *type);
int GetHttpHostnameData(void *data, uint8_t **buf, uint32_t *len, uint32_t *type);
void HI_SearchInit(void);
void HI_SearchFree(void);
int HI_SearchStrFound(void *, void *, int , void *, void *);

static inline HttpSessionData * GetHttpSessionData(Packet *p)
{
    if (p->ssnptr == NULL)
        return NULL;
    return (HttpSessionData *)session_api->get_application_data(p->ssnptr, PP_HTTPINSPECT);
}

static inline sfip_t *GetTrueIPForSession(void *data)
{
    HttpSessionData *hsd = NULL;

    if (data == NULL)
        return NULL;
    hsd = (HttpSessionData *)session_api->get_application_data(data, PP_HTTPINSPECT);

    if(hsd == NULL)
        return NULL;

    return hsd->true_ip;

}

static inline void ResetGzipState(DECOMPRESS_STATE *ds)
{
    if (ds == NULL)
        return;

    inflateEnd(&(ds->d_stream));

    ds->inflate_init = 0;
    ds->compr_bytes_read = 0;
    ds->decompr_bytes_read = 0;
    ds->compress_fmt = 0;
    ds->decompress_data = 0;
}

static inline void ResetRespState(HTTP_RESP_STATE *ds)
{
    if (ds == NULL)
        return;
    ds->inspect_body = 0;
    ds->last_pkt_contlen = 0;
    ds->last_pkt_chunked = 0;
    ds->inspect_reassembled = 0;
    ds->next_seq = 0;
    ds->chunk_remainder = 0;
    ds->data_extracted = 0;
    ds->max_seq = 0;
}

static inline int SetLogBuffers(HttpSessionData *hsd)
{
    int iRet = 0;
    if (hsd->log_state == NULL)
    {
        MemBucket *bkt = mempool_alloc(http_mempool);

        if (bkt != NULL)
        {
            hsd->log_state = (HTTP_LOG_STATE *)calloc(1, sizeof(HTTP_LOG_STATE));
            if( hsd->log_state != NULL )
            {
                hsd->log_state->log_bucket = bkt;
                hsd->log_state->uri_bytes = 0;
                hsd->log_state->hostname_bytes = 0;
                hsd->log_state->uri_extracted = (uint8_t *)bkt->data;
                hsd->log_state->hostname_extracted = (uint8_t *)bkt->data + MAX_URI_EXTRACTED;
            }
            else
            {
                mempool_free(http_mempool, bkt);
                iRet = -1;
            }
        }
        else
            iRet = -1;
    }

    return iRet;
}

static inline void SetHttpDecode(uint16_t altLen)
{
    HttpDecodeBuf.len = altLen;
}


#endif
