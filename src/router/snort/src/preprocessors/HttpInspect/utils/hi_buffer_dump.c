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
**  @file       hi_buffer_dump.c
**
**  @author     Rahul Burman <rahburma@cisco.com>
**
**  @brief      This file contains structures and functions for
**              dumping buffers used during HTTP inspection.
*/

#include "detection_util.h"
#include "hi_buffer_dump.h"
#include "memory_stats.h"


#ifdef DUMP_BUFFER
TraceBuffer buf[MAX_HTTP_BUFFER_DUMP] = {{"METHOD_DUMP", NULL, 0},
                                         {"URI_DUMP", NULL, 0},
                                         {"RAW_URI_DUMP", NULL, 0},
                                         {"REQ_HEADER_DUMP", NULL, 0},
                                         {"RAW_REQ_HEADER_DUMP", NULL, 0},
                                         {"COOKIE_DUMP", NULL, 0},
                                         {"RAW_COOKIE_DUMP", NULL, 0},
                                         {"CLIENT_BODY_DUMP", NULL, 0},
                                         {"STAT_CODE_DUMP", NULL, 0},
                                         {"STAT_MSG_DUMP", NULL, 0},
                                         {"RESP_HEADER_DUMP", NULL, 0},
                                         {"RAW_RESP_HEADER_DUMP", NULL, 0},
                                         {"RESP_COOKIE_DUMP", NULL, 0},
                                         {"RAW_RESP_COOKIE_DUMP", NULL, 0},
                                         {"RESP_BODY_DUMP", NULL, 0},
                                         {"FILE_DATA_DUMP", NULL, 0}};

void dumpBuffer(HTTP_BUFFER_DUMP type, const u_char *content, uint16_t len) {
    if (dump_enabled) {
        char *buf_cont = (char *)SnortPreprocAlloc(1, len + 1, PP_HTTPINSPECT, 
                                      PP_MEM_CATEGORY_SESSION);
        memcpy(buf_cont, content, len);
        buf[type].buf_content = buf_cont;
        buf[type].length = len;
    }
}

void clearReqBuffers() {
    unsigned int i;
    for (i = METHOD_DUMP; i <= CLIENT_BODY_DUMP; i++) {
       if (buf[i].buf_content) {
          SnortPreprocFree(buf[i].buf_content, buf[i].length, PP_HTTPINSPECT, 
               PP_MEM_CATEGORY_SESSION);
          buf[i].buf_content = NULL;
          buf[i].length = 0;
       }
    }
}

void clearRespBuffers() {
    unsigned int i;
    for (i = STAT_CODE_DUMP; i <= FILE_DATA_DUMP; i++) {
       if (buf[i].buf_content) {
          SnortPreprocFree(buf[i].buf_content, buf[i].length, PP_HTTPINSPECT, 
               PP_MEM_CATEGORY_SESSION);
          buf[i].buf_content = NULL;
          buf[i].length = 0;
       }
    }
}

TraceBuffer *getHTTPDumpBuffers()
{
    return buf;
}
#endif

