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
**  @file       dnp3_buffer_dump.h
**
**  @author     Krishnakanth <vkambala@cisco.com>
**
**  @brief      This file contains structures and functions for
**              dumping buffers for DNP3 protocol
*/

#ifndef __DNP3_BUFFER_DUMP_H__
#define __DNP3_BUFFER_DUMP_H__

#include "preprocids.h"

#ifdef DUMP_BUFFER

typedef enum {
    DNP3_SERVER_RESPONSE_DUMP,
    DNP3_CLINET_REQUEST_DUMP,
    DNP3_RESERVED_BAD_ADDR_DUMP,
    DNP3_BAD_CRC_DUMP
} DNP3_BUFFER_DUMP;

void dumpBuffer(DNP3_BUFFER_DUMP type, const uint8_t *content, uint16_t len);
void dumpBufferInit(void);
TraceBuffer *getDNP3Buffers();

#endif

#endif

