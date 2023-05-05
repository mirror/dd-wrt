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
**  @file       pop_buffer_dump.h
**
**  @author     Seshaiah Erugu <serugu@cisco.com>
**
**  @brief      This file contains structures and functions for
**              dumping buffers used during POP inspection.
*/

#ifndef __POP_BUFFER_DUMP_H__
#define __POP_BUFFER_DUMP_H__

#include "preprocids.h"

#ifdef DUMP_BUFFER

typedef enum {
 POP_REQUEST_PAYLOAD_DUMP,
 POP_REQUEST_COMMAND_DUMP,
 POP_RESPONSE_PAYLOAD_DUMP,
 POP_RESPONSE_STATUS_DUMP,
 POP_STATE_TLS_CLIENT_PEND_DUMP,
 POP_STATE_TLS_DATA_DUMP,
 POP_STATE_TLS_SERVER_PEND_DUMP
} POP_BUFFER_DUMP;

void dumpBuffer(POP_BUFFER_DUMP type, const uint8_t *content, uint16_t len);
void dumpBufferInit(void);
TraceBuffer *getPOPBuffers();


#endif

#endif
