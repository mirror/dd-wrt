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
**  @file       dns_buffer_dump.h
**
**  @author     Seshaiah Erugu <serugu@cisco.com>
**
**  @brief      This file contains structures and functions for
**              dumping buffers used during DNS inspection.
*/

#ifndef __DNS_BUFFER_DUMP_H__
#define __DNS_BUFFER_DUMP_H__

#include "preprocids.h"

#ifdef DUMP_BUFFER

typedef enum {
 DNS_PAYLOAD_DUMP,
 DNS_QUESTION_DUMP,
 DNS_ANSWER_DUMP,
 DNS_RESP_STATE_ANS_RR_DUMP,
 DNS_RESP_STATE_AUTH_RR_DUMP,
 DNS_RESP_STATE_ADD_RR_DUMP,
 DNS_RR_TYPE_TXT_VULN_DUMP,
 DNS_OBSOLETE_TYPES_DUMP,
 DNS_EXPERIMENTAL_TYPES_DUMP,
 DNS_SKIP_RDATA_DUMP
} DNS_BUFFER_DUMP;

void dumpBuffer(DNS_BUFFER_DUMP type, const uint8_t *content, uint16_t len);
void dumpBufferInit(void);
TraceBuffer *getDNSBuffers();


#endif

#endif

