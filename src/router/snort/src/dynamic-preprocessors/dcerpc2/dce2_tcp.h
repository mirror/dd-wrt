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

#ifndef _DCE2_TCP_H_
#define _DCE2_TCP_H_

#include "dce2_session.h"
#include "dce2_co.h"
#include "dce2_utils.h"
#include "dcerpc.h"
#include "sf_snort_packet.h"
#include "sf_types.h"
#include "snort_debug.h"

/********************************************************************
 * Structures
 ********************************************************************/
typedef struct _DCE2_TcpSsnData
{
    DCE2_SsnData sd;
    DCE2_CoTracker co_tracker;

} DCE2_TcpSsnData;

/********************************************************************
 * Inline function prototypes
 ********************************************************************/
static inline DCE2_TransType DCE2_TcpAutodetect(const SFSnortPacket *);

/********************************************************************
 * Public function prototypes
 ********************************************************************/
DCE2_TcpSsnData * DCE2_TcpSsnInit(void);
void DCE2_TcpProcess(DCE2_TcpSsnData *);
void DCE2_TcpDataFree(DCE2_TcpSsnData *);
void DCE2_TcpSsnFree(void *);

/*********************************************************************
 * Function: DCE2_TcpAutodetect()
 *
 * Purpose: Tries to determine if a packet is likely to be DCE/RPC
 *          over TCP.
 *
 * Arguments:
 *  const uint8_t * - pointer to packet data.
 *  uint16_t - packet data length.
 *
 * Returns:
 *  DCE2_TranType
 *
 *********************************************************************/
static inline DCE2_TransType DCE2_TcpAutodetect(const SFSnortPacket *p)
{
    if (p->payload_size >= sizeof(DceRpcCoHdr))
    {
        DceRpcCoHdr *co_hdr = (DceRpcCoHdr *)p->payload;

        if ((DceRpcCoVersMaj(co_hdr) == DCERPC_PROTO_MAJOR_VERS__5)
                && (DceRpcCoVersMin(co_hdr) == DCERPC_PROTO_MINOR_VERS__0)
                && ((DCE2_SsnFromClient(p)
                        && DceRpcCoPduType(co_hdr) == DCERPC_PDU_TYPE__BIND)
                    || (DCE2_SsnFromServer(p)
                        && DceRpcCoPduType(co_hdr) == DCERPC_PDU_TYPE__BIND_ACK))
                && (DceRpcCoFragLen(co_hdr) >= sizeof(DceRpcCoHdr)))
        {
            return DCE2_TRANS_TYPE__TCP;
        }
    }
    else if ((*p->payload == DCERPC_PROTO_MAJOR_VERS__5) && DCE2_SsnFromClient(p))
    {
        return DCE2_TRANS_TYPE__TCP;
    }

    return DCE2_TRANS_TYPE__NONE;
}

#endif  /* _DCE2_TCP_H_ */

