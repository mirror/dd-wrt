/*
** Copyright (C) 2014 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2005-2013 Sourcefire, Inc.
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


#ifndef __RNA_FLOW_H__
#define __RNA_FLOW_H__

#include <time.h>
#include "host_tracker.h"
/*#include "preprocessorHeader.h" */
#include "flow.h"

#define RNA_FLOW_TIMEOUT_TCP            (60*60)
#define RNA_FLOW_TIMEOUT_UDP            (3*60)
#define RNA_FLOW_TIMEOUT_IPFRAG          (30)
#define RNA_FLOW_TIMEOUT                 (60)
#define RNA_FLOW_TIMEOUT_NEW_CONNECTION  (90)

#define RNA_FLOW_DATA_ID_INIT_TCP_FP        1
#define RNA_FLOW_DATA_ID_UNKNOWN_STATE      2

enum {
    RNA_FLOW_RESPONDER_SEEN              = (1<<0),

    /**Log packets of the session */
    RNA_FLOW_LOG_UNKNOWN                 = (1<<1),
    /**Acquired a banner */
    RNA_FLOW_GOT_BANNER                  = (1<<2),

    RNA_FLOW_SERVICE_REPORTED            = (1<<3),
    RNA_FLOW_PORT_SERVICE_REPORTED       = (1<<4),
    RNA_FLOW_CLIENTAPP_REPORTED          = (1<<5),
    RNA_FLOW_PAYLOAD_REPORTED            = (1<<6),
    RNA_FLOW_DELAYED_PAYLOADAPP_REPORTED = (1<<7)

} tRNAFlowFlags;

typedef enum
{
    FLOW_NEW, /**< first packet in flow */
    FLOW_FIRST_BIDIRECTIONAL,  /**< first response packet in flow */
    FLOW_ADDITIONAL, /**< additional data on an existing flow */
    FLOW_POSITION_MAX /* Maximum value of a position (must be last in the list */
} FLOW_POSITION;

typedef void (*RNAFlowDataFreeFCN)(void *);

typedef struct _RNA_FLOW_DATA
{
    struct _RNA_FLOW_DATA *next;
    unsigned id;
    void *data;
    RNAFlowDataFreeFCN fcn;
} RNAFlowData;

typedef struct _RNA_FLOW
{
    FLOW_STRUCT_FLAG fsf_type;  /* This must be first. */

    struct _RNA_FLOW *next;

    uint8_t proto;

    RNAFlowData *data;
    unsigned flow_flags; /* normal, timeout, etc. */
    FLOW_POSITION position;

    snort_ip initiator_ip;
    uint16_t initiator_port;

    tAppIdData *appIdData;
} RNAFlow;

#define RNA_UNKNOWN_PACKETS_TO_LOG  5
typedef struct _RNA_UNKNOWN_STATE
{
    struct _RNA_UNKNOWN_STATE *next;
    unsigned init_packets_logged;    /* Count of client packets with data that have been logged */
    unsigned resp_packets_logged;    /* Count of client packets with data that have been logged */
} RNAUnknownState;

void AnalyzeFlow(SFSnortPacket *, tAppIdData *);
RNAFlowData *rna_flowdata_get(RNAFlow *flowp, unsigned id);
void rna_flowdata_pop_and_free(RNAFlow *flowp, unsigned id);
void *rna_flowdata_pop(RNAFlow *flowp, unsigned id);
RNAFlowData *rna_flowdata_add(RNAFlow *flowp, void *data, unsigned id,
                              RNAFlowDataFreeFCN fcn);
extern unsigned long rna_flow_memory;

/**
 * Mark a flow with a particular flag
 *
 * @param flow
 * @param flags
 */
static inline void rna_flow_mark(RNAFlow *flow, unsigned flags)
{
    flow->flow_flags |= flags;
}

/**
 * Mark a flow with a particular flag
 *
 * @param flow
 * @param flags
 */
static inline void rna_flow_clear(RNAFlow *flow, unsigned flags)
{
    flow->flow_flags &= ~flags;
}

/**
 * Check to see if a particular flag exists
 *
 * @param flow
 * @param flags
 */
static inline unsigned rna_flow_checkflag(RNAFlow *flow, unsigned flags)
{
    return (flow->flow_flags & flags);
}

void cleanupRnaFlow(void);

#endif /* __RNA_FLOW_H__ */

