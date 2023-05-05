/*
 ** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 ** Copyright (C) 2002-2013 Sourcefire, Inc.
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
 ** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <time.h>

#include "detection_util.h"
#include "sfutil/sf_textlog.h"
#include "rules.h"
#include "snort.h"


uint8_t base64_decode_buf[DECODE_BLEN];
uint32_t base64_decode_size;

uint8_t mime_present;

const uint8_t *doe_ptr;
uint8_t doe_buf_flags;
uint16_t detect_flags;

uint32_t http_mask;
HttpBuffer http_buffer[HTTP_BUFFER_MAX];

DataPointer DetectBuffer;
DataPointer file_data_ptr;
DataBuffer DecodeBuffer;
void *global_ssl_callback = NULL;

const char* http_buffer_name[HTTP_BUFFER_MAX] =
{
    "error/unset",
    "http_uri",
    "http_header",
    "http_client_body",
    "http_method",
    "http_cookie",
    "http_stat_code",
    "http_stat_msg",
    "http_raw_uri",
    "http_raw_header",
    "http_raw_cookie",
};

static const char* rule_type[RULE_TYPE__MAX] = {
    "none", "alert", "drop", "log", "pass", "reject", "sdrop"
};

#define LOG_CHARS 16

static TextLog* tlog = NULL;
static unsigned nEvents = 0;

static void LogBuffer (const char* s, const uint8_t* p, unsigned n)
{
    char hex[(3*LOG_CHARS)+1];
    char txt[LOG_CHARS+1];
    unsigned odx = 0, idx = 0, at = 0;

    if ( !p )
        return;

    if ( n > snort_conf->event_trace_max )
        n = snort_conf->event_trace_max;

    for ( idx = 0; idx < n; idx++)
    {
        uint8_t byte = p[idx];
        sprintf(hex + 3*odx, "%2.02X ", byte);
        txt[odx++] = isprint(byte) ? byte : '.';

        if ( odx == LOG_CHARS )
        {
            txt[odx] = hex[3*odx] = '\0';
            TextLog_Print(tlog, "%s[%2u] %s %s\n", s, at, hex, txt);
            at = idx + 1;
            odx = 0;
        }
    }
    if ( odx )
    {
        txt[odx] = hex[3*odx] = '\0';
        TextLog_Print(tlog, "%s[%2u] %-48.48s %s\n", s, at, hex, txt);
    }
}

void EventTrace_Log (const Packet* p, OptTreeNode* otn, int action)
{
    int i;
    const char* acts = (action < RULE_TYPE__MAX) ? rule_type[action] : "ERROR";

    if ( !tlog )
        return;

    TextLog_Print(tlog,
        "\nEvt=%u, Gid=%u, Sid=%u, Rev=%u, Act=%s\n",
        event_id, otn->sigInfo.generator,
        otn->sigInfo.id, otn->sigInfo.rev, acts
    );
    TextLog_Print(tlog,
        "Pkt=%lu, Sec=%u.%6u, Len=%u, Cap=%u\n",
        pc.total_from_daq, p->pkth->ts.tv_sec, p->pkth->ts.tv_usec,
        p->pkth->pktlen, p->pkth->caplen
    );
    TextLog_Print(tlog,
        "Pkt Bits: Flags=0x%X, PP=0x%llx, Proto=0x%X"
        ", Err=0x%X\n",
        p->packet_flags, p->preprocessor_bits,
        (unsigned)p->proto_bits, (unsigned)p->error_flags
    );
    TextLog_Print(tlog,
        "Pkt Cnts: Dsz=%u, Alt=%u, Uri=0x%X\n",
        (unsigned)p->dsize, (unsigned)p->alt_dsize, http_mask
    );
    TextLog_Print(tlog, "Detect: DoeFlags=0x%X, DetectFlags=0x%X, DetBuf=%u, B64=%u\n",
        doe_buf_flags, detect_flags, DetectBuffer.len, base64_decode_size
    );
    LogBuffer("Decode", DecodeBuffer.data, DecodeBuffer.len);
    LogBuffer("Detect", DetectBuffer.data, DetectBuffer.len);
    LogBuffer("FileData", file_data_ptr.data, file_data_ptr.len);
    LogBuffer("Base64", base64_decode_buf, base64_decode_size);
    if(mime_present)
        LogBuffer("Mime", file_data_ptr.data, file_data_ptr.len);

    for ( i = 0; i < HTTP_BUFFER_MAX; i++ )
    {
        const HttpBuffer* hb = GetHttpBuffer(i);

        if ( !hb )
            continue;

        TextLog_Print(tlog, "%s[%u] = 0x%X\n",
            http_buffer_name[i], hb->length, hb->encode_type);

        LogBuffer(http_buffer_name[i], hb->buf, hb->length);
    }
    nEvents++;
}

void EventTrace_Init (void)
{
    if ( snort_conf->event_trace_max > 0 )
    {
        time_t now = time(NULL);
        const char* ts = ctime(&now);

        char buf[STD_BUF];
        const char* dir = snort_conf->log_dir ? snort_conf->log_dir : ".";
        snprintf(buf, sizeof(buf), "%s/%s", dir, snort_conf->event_trace_file);

        tlog = TextLog_Init (buf, 128, 8*1024*1024);
        TextLog_Print(tlog, "\nTrace started at %s", ts);
        TextLog_Print(tlog, "Trace max_data is %u bytes\n", snort_conf->event_trace_max);
    }
}

void EventTrace_Term (void)
{
    if ( tlog )
    {
        time_t now = time(NULL);
        const char* ts = ctime(&now);
        TextLog_Print(tlog, "\nTraced %u events\n", nEvents);
        TextLog_Print(tlog, "Trace stopped at %s", ts);
        TextLog_Term(tlog);
    }
}

