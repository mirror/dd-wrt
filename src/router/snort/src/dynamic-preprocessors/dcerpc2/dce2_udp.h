/****************************************************************************
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2008-2013 Sourcefire, Inc.
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
 ****************************************************************************
 *
 ****************************************************************************/

#ifndef _DCE2_UDP_H_
#define _DCE2_UDP_H_

#include "dce2_cl.h"
#include "dce2_session.h"
#include "dce2_list.h"
#include "dce2_utils.h"
#include "dcerpc.h"
#include "sf_snort_packet.h"
#include "sf_types.h"
#include "snort_debug.h"

/********************************************************************
 * Structures
 ********************************************************************/
typedef struct _DCE2_UdpSsnData
{
    DCE2_SsnData sd;
    DCE2_ClTracker cl_tracker;

} DCE2_UdpSsnData;

/********************************************************************
 * Inline function prototypes
 ********************************************************************/
static inline DCE2_TransType DCE2_UdpAutodetect(const SFSnortPacket *);

/********************************************************************
 * Public function prototypes
 ********************************************************************/
DCE2_UdpSsnData * DCE2_UdpSsnInit(void);
void DCE2_UdpProcess(DCE2_UdpSsnData *);
void DCE2_UdpDataFree(DCE2_UdpSsnData *);
void DCE2_UdpSsnFree(void *);

/*********************************************************************
 * Function: DCE2_UdpAutodetect()
 *
 * Purpose: Tries to determine if a packet is likely to be DCE/RPC
 *          over UDP.
 *
 * Arguments:
 *  const uint8_t * - pointer to packet data.
 *  uint16_t - packet data length.
 *
 * Returns:
 *  DCE2_TranType
 *
 *********************************************************************/
static inline DCE2_TransType DCE2_UdpAutodetect(const SFSnortPacket *p)
{
    if (p->payload_size >= sizeof(DceRpcClHdr))
    {
        DceRpcClHdr *cl_hdr = (DceRpcClHdr *)p->payload;

        if ((DceRpcClRpcVers(cl_hdr) == DCERPC_PROTO_MAJOR_VERS__4) &&
            ((DceRpcClPduType(cl_hdr) == DCERPC_PDU_TYPE__REQUEST) ||
             (DceRpcClPduType(cl_hdr) == DCERPC_PDU_TYPE__RESPONSE) ||
             (DceRpcClPduType(cl_hdr) == DCERPC_PDU_TYPE__FAULT) ||
             (DceRpcClPduType(cl_hdr) == DCERPC_PDU_TYPE__REJECT) ||
             (DceRpcClPduType(cl_hdr) == DCERPC_PDU_TYPE__FACK)) &&
            ((DceRpcClLen(cl_hdr) != 0) &&
             (DceRpcClLen(cl_hdr) + sizeof(DceRpcClHdr)) <= p->payload_size))
        {
            return DCE2_TRANS_TYPE__UDP;
        }
    }

    return DCE2_TRANS_TYPE__NONE;
}

#endif   /* _DCE2_UDP_H_ */

