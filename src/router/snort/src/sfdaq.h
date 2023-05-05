/* $Id$ */
/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2005-2013 Sourcefire, Inc.
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

// @file    sfdaq.h
// @author  Russ Combs <rcombs@sourcefire.com>

#ifndef __DAQ_H__
#define __DAQ_H__

#include <stdio.h>
#include <daq.h>

#include "ipv6_port.h"
#define PKT_TIMEOUT  1000  // ms, worst daq resolution is 1 sec

struct _SnortConfig;
#include "decode.h"

void DAQ_Load(const struct _SnortConfig*);
void DAQ_Unload(void);

void DAQ_Init(const struct _SnortConfig*);
void DAQ_Term(void);
void DAQ_Abort(void);

int DAQ_PrintTypes(FILE*);
const char* DAQ_GetType(void);

int DAQ_Unprivileged(void);
int DAQ_UnprivilegedStart(void);
int DAQ_CanReplace(void);
int DAQ_CanInject(void);
int DAQ_CanWhitelist(void);
int DAQ_CanRetry (void);
int DAQ_RawInjection(void);
uint32_t DAQ_GetCapabilities(void);
#if defined(DAQ_CAPA_CST_TIMEOUT)
int DAQ_CanGetTimeout(void);
#endif
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
int DAQ_CanGetVrf(void);
#endif

const char* DAQ_GetInterfaceSpec(void);
uint32_t DAQ_GetSnapLen(void);
int DAQ_GetBaseProtocol(void);
int DAQ_SetFilter(const char*);

// total stats are accumulated when daq is deleted
int DAQ_New(const struct _SnortConfig*, const char* intf);
void DAQ_UpdateTunnelBypass(struct _SnortConfig*);
int DAQ_Delete(void);

int DAQ_Start(void);
int DAQ_WasStarted(void);
int DAQ_Stop(void);

// TBD some stuff may be inlined once encapsulations are straight
// (but only where performance justifies exposing implementation!)
int DAQ_Acquire(int max, DAQ_Analysis_Func_t, uint8_t* user);
int DAQ_Inject(const DAQ_PktHdr_t*, int rev, const uint8_t* buf, uint32_t len);
int DAQ_BreakLoop(int error);
#ifdef HAVE_DAQ_ACQUIRE_WITH_META
void DAQ_Set_MetaCallback(DAQ_Meta_Func_t meta_callback);
#endif
DAQ_Mode DAQ_GetInterfaceMode(const DAQ_PktHdr_t *h);

int DAQ_ModifyFlowOpaque(const DAQ_PktHdr_t *hdr, uint32_t opaque);
#ifdef HAVE_DAQ_EXT_MODFLOW
int DAQ_ModifyFlowHAState(const DAQ_PktHdr_t *hdr, const void *data, uint32_t length);
int DAQ_ModifyFlow(const DAQ_PktHdr_t *hdr, const DAQ_ModFlow_t* mod);
#endif
#ifdef HAVE_DAQ_QUERYFLOW
#ifdef REG_TEST
int DAQ_QueryFlow( DAQ_PktHdr_t *hdr, DAQ_QueryFlow_t* query);
#else
int DAQ_QueryFlow(const DAQ_PktHdr_t *hdr, DAQ_QueryFlow_t* query);
#endif
#endif

#if defined(DAQ_VERSION) && DAQ_VERSION > 8
void DAQ_DebugPkt(uint8_t moduleId, uint8_t logLevel, const DAQ_Debug_Packet_Params_t *params, const char *msg, ...);
#endif

#ifdef HAVE_DAQ_DP_ADD_DC

typedef struct _DAQ_DC_Params
{
    unsigned flags;
    unsigned timeout_ms;
} DAQ_DC_Params;
#define DAQ_DC_FLOAT            0x01
#define DAQ_DC_ALLOW_MULTIPLE   0x02
#define DAQ_DC_PERSIST          0x04
void DAQ_Add_Dynamic_Protocol_Channel(const Packet *ctrlPkt, sfaddr_t* cliIP, uint16_t cliPort,
                                      sfaddr_t* srvIP, uint16_t srvPort, uint8_t protocol,
                                      DAQ_DC_Params* params);
#endif

#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
static inline uint16_t DAQ_GetSourceAddressSpaceID(const DAQ_PktHdr_t *h)
{
    return h->address_space_id_src;
}

static inline uint16_t DAQ_GetDestinationAddressSpaceID(const DAQ_PktHdr_t *h)
{
    return h->address_space_id_dst;
}
#endif

#ifdef HAVE_DAQ_ADDRESS_SPACE_ID
static inline uint16_t DAQ_GetAddressSpaceID(const DAQ_PktHdr_t *h)
{
    return h->address_space_id;
}
#endif

// returns total stats if no daq else current stats
// returns statically allocated stats - don't free
const DAQ_Stats_t* DAQ_GetStats(void);

#if defined (DAQ_VERSION) && DAQ_VERSION >9
#define SNORT_DEBUG_PKT_LOG(pkth,moduleId,logLevel,msg,args...)\
{\
    DAQ_Debug_Packet_Params_t daq_pkt_params;\
    if((pkth) && ((pkth)->flags & DAQ_PKT_FLAG_DEBUG_ON))\
    {\
        daq_pkt_params.pkt_hdr = (DAQ_PktHdr_t *)(pkth);\
        DAQ_DebugPkt(moduleId, logLevel, &daq_pkt_params, msg, ##args);\
    }\
}
#endif

#if defined(DAQ_VERSION) && DAQ_VERSION > 9
#define DEBUG_SNORT_ENGINE(p, logLevel, format, args...) \
{ \
    SNORT_DEBUG_PKT_LOG(p->pkth, DAQ_DEBUG_PKT_MODULE_SNORT_ENGINE, logLevel, format, ##args) \
}
#else
#define DEBUG_SNORT_ENGINE(p, logLevel, format, args...) \
{ \
}
#endif

#if defined(DAQ_VERSION) && DAQ_VERSION > 9
int DAQ_Ioctl(unsigned int type, char *buf, size_t *size);
#endif
#endif // __DAQ_H__

