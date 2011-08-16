/****************************************************************************
 * Copyright (C) 2008-2011 Sourcefire, Inc.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 ****************************************************************************
 * 
 ****************************************************************************/

#include "dce2_tcp.h"
#include "snort_dce2.h"
#include "dce2_co.h"
#include "dce2_memory.h"
#include "dce2_stats.h"
#include "sf_snort_packet.h"
#include "sf_dynamic_preprocessor.h"

/********************************************************************
 * Extern variables
 ********************************************************************/
extern DynamicPreprocessorData _dpd;
extern DCE2_Stats dce2_stats;

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
DCE2_TcpSsnData * DCE2_TcpSsnInit(void)
{
    DCE2_TcpSsnData *tsd = DCE2_Alloc(sizeof(DCE2_TcpSsnData), DCE2_MEM_TYPE__TCP_SSN);

    if (tsd == NULL)
        return NULL;

    DCE2_CoInitTracker(&tsd->co_tracker);
    DCE2_ResetRopts(&tsd->sd.ropts);

    dce2_stats.tcp_sessions++;

    return tsd;
}

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
void DCE2_TcpProcess(DCE2_TcpSsnData *tsd)
{
    const SFSnortPacket *p = tsd->sd.wire_pkt;
    const uint8_t *data_ptr = p->payload;
    uint16_t data_len = p->payload_size;
    uint16_t overlap_bytes = DCE2_SsnGetOverlap(&tsd->sd);

    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__MAIN, "Processing TCP packet.\n"));
    dce2_stats.tcp_pkts++;

    if (overlap_bytes != 0)
    {
        if (overlap_bytes >= data_len)
            return;

        DCE2_MOVE(data_ptr, data_len, overlap_bytes);
    }

    DCE2_CoProcess(&tsd->sd, &tsd->co_tracker, data_ptr, data_len);
}

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
void DCE2_TcpDataFree(DCE2_TcpSsnData *tsd)
{
    if (tsd == NULL)
        return;

    DCE2_CoCleanTracker(&tsd->co_tracker);
}

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
void DCE2_TcpSsnFree(void *ssn)
{
    DCE2_TcpSsnData *tsd = (DCE2_TcpSsnData *)ssn;

    if (tsd == NULL)
        return;

    DCE2_TcpDataFree(tsd);
    DCE2_Free((void *)tsd, sizeof(DCE2_TcpSsnData), DCE2_MEM_TYPE__TCP_SSN);
}

