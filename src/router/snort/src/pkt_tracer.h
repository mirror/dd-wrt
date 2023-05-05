/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
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

#ifndef _PKT_TRACER_H
#define _PKT_TRACER_H

#include "sf_ip.h"
#include "sfdaq.h"
#include "dynamic-plugins/sf_dynamic_common.h"

#define CS_TYPE_PKT_TRACER 120
#define MAX_TRACE_LINE 256

extern volatile int pkt_trace_cli_flag;
extern bool pkt_trace_enabled;
extern char trace_line[MAX_TRACE_LINE];

int DebugPktTracer(uint16_t type, const uint8_t *data, uint32_t length, void **new_context,
        char* statusBuf, int statusBuf_len);
bool pktTracerDebugCheck(Packet* p);
bool pktTracerDebugCheckSsn(void* ssn);
void addPktTraceData(int module, int traceLen);
void addPktTraceInfo(void *packet);
void writePktTraceData(DAQ_Verdict verdict, unsigned int napId, unsigned int ipsId, const Packet* p);
const char* getPktTraceActMsg();
void SavePktTrace();
void RestorePktTrace();

extern Verdict_Reason verdict_reason;
#if defined(HAVE_DAQ_EXT_MODFLOW) && defined(HAVE_DAQ_VERDICT_REASON)
void sendReason(const Packet* p);
#endif

#endif
