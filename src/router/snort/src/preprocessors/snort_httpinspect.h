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
 
#ifndef __SNORT_HTTPINSPECT_H__
#define __SNORT_HTTPINSPECT_H__

#include "decode.h"
#include "stream_api.h"
#include "hi_ui_config.h"
#include "util_utf.h"

#ifdef ZLIB
#include "mempool.h"
#include <zlib.h>
#endif

/**
**  The definition of the configuration separators in the snort.conf
**  configure line.
*/
#define CONF_SEPARATORS " \t\n\r"
#define MAX_METHOD_LEN  7

/*
**  These are the definitions of the parser section delimiting 
**  keywords to configure HttpInspect.  When one of these keywords
**  are seen, we begin a new section.
*/
#define GLOBAL        "global"
#define GLOBAL_SERVER "global_server"
#define SERVER        "server"
#define CLIENT        "client"


#ifdef ZLIB

#define DEFAULT_MAX_GZIP_MEM 838860 
#define GZIP_MEM_MAX    104857600
#define GZIP_MEM_MIN    3276
#define MAX_GZIP_DEPTH    65535
#define DEFAULT_COMP_DEPTH 1460
#define DEFAULT_DECOMP_DEPTH 2920

#define DEFLATE_WBITS   -15
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
    MemBucket *gzip_bucket;
    unsigned char *compr_buffer;
    unsigned char *decompr_buffer;

} DECOMPRESS_STATE;
#endif

typedef struct s_HTTP_RESP_STATE
{
    uint8_t inspect_body;
    uint8_t inspect_reassembled;
    uint8_t last_pkt_contlen;
    uint8_t last_pkt_chunked;
    uint32_t next_seq;
    int last_chunk_size;
    int flow_depth_read;
    uint32_t max_seq;
    int is_max_seq;
}HTTP_RESP_STATE;

typedef struct _HttpSessionData
{
    uint32_t event_flags;
    HTTP_RESP_STATE resp_state;
#ifdef ZLIB
    DECOMPRESS_STATE *decomp_state;
#endif
    sfip_t *true_ip;
    decode_utf_state_t utf_state;
} HttpSessionData;


int SnortHttpInspect(HTTPINSPECT_GLOBAL_CONF *GlobalConf, Packet *p);
int ProcessGlobalConf(HTTPINSPECT_GLOBAL_CONF *, char *, int);
int PrintGlobalConf(HTTPINSPECT_GLOBAL_CONF *);
int ProcessUniqueServerConf(HTTPINSPECT_GLOBAL_CONF *, char *, int);
int HttpInspectInitializeGlobalConfig(HTTPINSPECT_GLOBAL_CONF *, char *, int);
HttpSessionData * SetNewHttpSessionData(Packet *p, void *session);
void FreeHttpSessionData(void *data);

static INLINE HttpSessionData * GetHttpSessionData(Packet *p)
{
    if (p->ssnptr == NULL)
        return NULL;
    return (HttpSessionData *)stream_api->get_application_data(p->ssnptr, PP_HTTPINSPECT);
}

static INLINE sfip_t *GetTrueIPForSession(Packet *p)
{
    HttpSessionData *hsd = NULL;

    hsd = GetHttpSessionData(p);

    if(hsd == NULL)
        return NULL;

    return hsd->true_ip;

}

#ifdef ZLIB
static INLINE void ResetGzipState(DECOMPRESS_STATE *ds)
{
    if (ds == NULL)
        return;

    inflateEnd(&(ds->d_stream));

    memset(ds->gzip_bucket->data, 0, ds->compr_depth + ds->decompr_depth);

    ds->inflate_init = 0;
    ds->compr_bytes_read = 0;
    ds->decompr_bytes_read = 0;
    ds->compress_fmt = 0;
    ds->decompress_data = 0;
}
#endif  /* ZLIB */

static INLINE void ResetRespState(HTTP_RESP_STATE *ds)
{
    if (ds == NULL)
        return;
    ds->inspect_body = 0;
    ds->last_pkt_contlen = 0;
    ds->last_pkt_chunked = 0;
    ds->inspect_reassembled = 0;
    ds->next_seq = 0;
    ds->last_chunk_size = 0;
    ds->flow_depth_read = 0;
    ds->max_seq = 0;
    ds->is_max_seq = 0;
}


#endif
