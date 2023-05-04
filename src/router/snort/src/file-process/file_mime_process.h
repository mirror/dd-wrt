/*
 ** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 ** Copyright (C) 2012-2013 Sourcefire, Inc.
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
 **
 **  Author(s):  Hui Cao <hcao@sourcefire.com>
 **
 **  NOTES
 **  9.25.2012 - Initial Source Code. Hcao
 */

#ifndef _FILE_MIME_PROCESS_H_
#define _FILE_MIME_PROCESS_H_

#include "file_api.h"
#include "sf_email_attach_decode.h"
#include "mempool.h"
#include "sfPolicy.h"
#include "file_mail_common.h"
#include "memory_stats.h"

int set_log_buffers(MAIL_LogState **log_state, MAIL_LogConfig *conf, void *mempool, void* scbPtr, uint32_t preproc_id);
void* init_mime_mempool(int max_mime_mem, int max_depth, void *mempool, const char *preproc_name);
void* init_log_mempool(uint32_t email_hdrs_log_depth, uint32_t memcap,  void *mempool, const char *preproc_name);
void init_mime(void);
void free_mime(void);
const uint8_t* process_mime_data(void *packet, const uint8_t *start, const uint8_t *end,
        MimeState *mime_ssn, bool upload, bool paf_enabled, char *preproc_name, uint32_t preproc_id);
void free_mime_session(MimeState *mime_ssn);
void finalize_mime_position(void *ssnptr, void *decode_state, FilePosition *position);

void force_flush_stream (void *ssn);
void reset_mime_paf_state(MimeDataPafInfo *data_info);
/*  Process data boundary and flush each file based on boundary*/
bool process_mime_paf_data(MimeDataPafInfo *data_info,  uint8_t val);
bool check_data_end(void *end_state,  uint8_t val);

#ifdef SNORT_RELOAD
void update_mime_mempool(void*, int, int);
void update_log_mempool(void*, int, int);
#ifdef REG_TEST
void displayMimeMempool(void *memory_pool, DecodeConfig *decode_conf_old, DecodeConfig *decode_conf_new);
void displayLogMempool(void *memory_pool, unsigned memcap_old, unsigned memcap_new);
void displayDecodeDepth(DecodeConfig *decode_conf_old, DecodeConfig *decode_conf_new);
#endif
#endif

#endif
