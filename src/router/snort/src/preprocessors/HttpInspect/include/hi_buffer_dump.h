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
**  @file       hi_buffer_dump.h
**
**  @author     Rahul Burman <rahburma@cisco.com>
**
**  @brief      This file contains structures and functions for
**              dumping buffers used during HTTP inspection.
*/

#ifndef __HI_BUFFER_DUMP_H__
#define __HI_BUFFER_DUMP_H__

#include "preprocids.h"

#ifdef DUMP_BUFFER
typedef enum {
    METHOD_DUMP,
    URI_DUMP,
    RAW_URI_DUMP,
    REQ_HEADER_DUMP,
    RAW_REQ_HEADER_DUMP,
    COOKIE_DUMP,
    RAW_COOKIE_DUMP,
    CLIENT_BODY_DUMP,
    STAT_CODE_DUMP,
    STAT_MSG_DUMP,
    RESP_HEADER_DUMP,
    RAW_RESP_HEADER_DUMP,
    RESP_COOKIE_DUMP,
    RAW_RESP_COOKIE_DUMP,
    RESP_BODY_DUMP,
    FILE_DATA_DUMP
} HTTP_BUFFER_DUMP;

void dumpBuffer(HTTP_BUFFER_DUMP type, const u_char *content, uint16_t len);
void dumpBufferInit(void);
void clearReqBuffers(void);
void clearRespBuffers(void);
TraceBuffer *getHTTPDumpBuffers(void);
#endif

#endif
