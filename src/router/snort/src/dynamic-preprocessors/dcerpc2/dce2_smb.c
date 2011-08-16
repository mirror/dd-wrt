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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "dce2_smb.h"
#include "dce2_tcp.h"
#include "dce2_co.h"
#include "snort_dce2.h"
#include "dce2_config.h"
#include "dce2_memory.h"
#include "dce2_utils.h"
#include "dce2_debug.h"
#include "dce2_stats.h"
#include "dce2_event.h"
#include "smb.h"
#include "sf_snort_packet.h"
#include "sf_types.h"
#include "profiler.h"
#include "debug.h"
#include "sf_dynamic_preprocessor.h"

#ifndef WIN32
#include <arpa/inet.h>  /* for ntohl */
#endif  /* WIN32 */

/********************************************************************
 * Global variables
 ********************************************************************/
/* There are some static variables in DCE2_SmbChained() */

/********************************************************************
 * Extern variables
 ********************************************************************/
extern DCE2_Stats dce2_stats;
extern DynamicPreprocessorData _dpd;
extern uint8_t dce2_smb_rbuf[];

#ifdef PERF_PROFILING
extern PreprocStats dce2_pstat_smb_seg;
extern PreprocStats dce2_pstat_smb_trans;
extern PreprocStats dce2_pstat_smb_uid;
extern PreprocStats dce2_pstat_smb_tid;
extern PreprocStats dce2_pstat_smb_fid;
#endif

/********************************************************************
 * Macros
 ********************************************************************/
#define DCE2_TC__QUEUE_SIZE  10
#define DCE2_TC__NOT_IPC     DCE2_SENTINEL
#define DCE2_TC__IPC         1

#define DCE2_READ__QUEUE_SIZE  10
#define DCE2_TRANS__QUEUE_SIZE 10

/********************************************************************
 * Private function prototypes
 ********************************************************************/
static DCE2_Ret DCE2_NbssHdrChecks(DCE2_SmbSsnData *, const NbssHdr *);
static DCE2_Ret DCE2_SmbHdrChecks(DCE2_SmbSsnData *, const SmbNtHdr *);
static int DCE2_SmbInspect(DCE2_SmbSsnData *, const SmbNtHdr *);

static void DCE2_SmbProcessData(DCE2_SmbSsnData *, const uint8_t *, uint32_t);
static void DCE2_SmbHandleCom(DCE2_SmbSsnData *, const SmbNtHdr *, const uint8_t *, uint32_t);
static void DCE2_SmbHandleRawData(DCE2_SmbSsnData *, const uint8_t *, uint32_t);

static void DCE2_SmbSessSetupAndX(DCE2_SmbSsnData *, const SmbNtHdr *, const uint8_t *, uint32_t);
static void DCE2_SmbLogoffAndX(DCE2_SmbSsnData *, const SmbNtHdr *, const uint8_t *, uint32_t, int);
static DCE2_Ret DCE2_SmbTreeConnect(DCE2_SmbSsnData *, const SmbNtHdr *, const uint8_t *, uint32_t);
static void DCE2_SmbTreeConnectEnqueue(DCE2_SmbSsnData *, const SmbNtHdr *, const DCE2_Ret);
static void DCE2_SmbTreeConnectAndX(DCE2_SmbSsnData *, const SmbNtHdr *, const uint8_t *, uint32_t);
static void DCE2_SmbTreeDisconnect(DCE2_SmbSsnData *, const SmbNtHdr *, const uint8_t *, uint32_t);
static void DCE2_SmbOpen(DCE2_SmbSsnData *, const SmbNtHdr *, const uint8_t *, uint32_t);
static void DCE2_SmbOpenAndX(DCE2_SmbSsnData *, const SmbNtHdr *, const uint8_t *, uint32_t);
static void DCE2_SmbNtCreateAndX(DCE2_SmbSsnData *, const SmbNtHdr *, const uint8_t *, uint32_t);
static void DCE2_SmbClose(DCE2_SmbSsnData *, const SmbNtHdr *, const uint8_t *, uint32_t, int);
static void DCE2_SmbWrite(DCE2_SmbSsnData *, const SmbNtHdr *, const uint8_t *, uint32_t);
static void DCE2_SmbWriteBlockRaw(DCE2_SmbSsnData *, const SmbNtHdr *, const uint8_t *, uint32_t);
static void DCE2_SmbWriteAndClose(DCE2_SmbSsnData *, const SmbNtHdr *, const uint8_t *, uint32_t);
static void DCE2_SmbWriteAndX(DCE2_SmbSsnData *, const SmbNtHdr *, const uint8_t *, uint32_t);
static void DCE2_SmbTrans(DCE2_SmbSsnData *, const SmbNtHdr *, const uint8_t *, uint32_t);
static void DCE2_SmbTransSec(DCE2_SmbSsnData *, const SmbNtHdr *, const uint8_t *, uint32_t);
static void DCE2_SmbRead(DCE2_SmbSsnData *, const SmbNtHdr *, const uint8_t *, uint32_t);
static void DCE2_SmbReadBlockRaw(DCE2_SmbSsnData *, const SmbNtHdr *, const uint8_t *, uint32_t);
static void DCE2_SmbReadAndX(DCE2_SmbSsnData *, const SmbNtHdr *, const uint8_t *, uint32_t);
static void DCE2_SmbRename(DCE2_SmbSsnData *, const SmbNtHdr *, const uint8_t *, uint32_t);

static int DCE2_SmbGetComSize(DCE2_SmbSsnData *, const SmbNtHdr *, const SmbCommon *, const int);
static int DCE2_SmbGetBcc(DCE2_SmbSsnData *, const SmbNtHdr *, const SmbCommon *, const uint16_t, const int);

static INLINE DCE2_Ret DCE2_SmbCheckComSize(DCE2_SmbSsnData *, const uint32_t, const uint16_t, const int);
static INLINE DCE2_Ret DCE2_SmbCheckBcc(DCE2_SmbSsnData *, const uint32_t, const uint16_t, const int);
static INLINE DCE2_Ret DCE2_SmbCheckDsize(DCE2_SmbSsnData *, const uint32_t,
                                          const uint16_t, const uint16_t, const int);
static INLINE DCE2_Ret DCE2_SmbCheckTotDcnt(DCE2_SmbSsnData *, const uint16_t, const uint16_t, const int);
static INLINE DCE2_Ret DCE2_SmbCheckOffset(DCE2_SmbSsnData *, const uint8_t *, const uint8_t *,
                                           const uint32_t, const int);

static INLINE void DCE2_SmbInvalidShareCheck(DCE2_SmbSsnData *, const SmbNtHdr *, const uint8_t *, uint32_t);

static INLINE void DCE2_SmbSetReadFidNode(DCE2_SmbSsnData *, uint16_t, uint16_t, uint16_t, int);
static INLINE DCE2_SmbFidTrackerNode * DCE2_SmbGetReadFidNode(DCE2_SmbSsnData *);

static void DCE2_SmbChained(DCE2_SmbSsnData *, const SmbNtHdr *, const SmbAndXCommon *,
                            const int, const uint8_t *, uint32_t);

static void DCE2_SmbIncComStat(const SmbNtHdr *);
static void DCE2_SmbIncChainedStat(const SmbNtHdr *, const int, const SmbAndXCommon *);

static void DCE2_WriteCoProcess(DCE2_SmbSsnData *, const SmbNtHdr *,
                                const uint16_t, const uint8_t *, uint16_t);

static void DCE2_SmbInsertUid(DCE2_SmbSsnData *, const uint16_t);
static DCE2_Ret DCE2_SmbFindUid(DCE2_SmbSsnData *, const uint16_t);
static void DCE2_SmbRemoveUid(DCE2_SmbSsnData *ssd, const uint16_t);

static void DCE2_SmbInsertTid(DCE2_SmbSsnData *, const uint16_t);
static DCE2_Ret DCE2_SmbFindTid(DCE2_SmbSsnData *, const uint16_t);
static void DCE2_SmbRemoveTid(DCE2_SmbSsnData *, const uint16_t);

static DCE2_SmbUTNode * DCE2_SmbFindUTNode(DCE2_SmbSsnData *, const uint16_t, const uint16_t);

static void DCE2_SmbInsertFid(DCE2_SmbSsnData *, const uint16_t, const uint16_t, const uint16_t);
static DCE2_SmbFidTrackerNode * DCE2_SmbFindFid(DCE2_SmbSsnData *, const uint16_t,
                                                const uint16_t, const uint16_t);
static void DCE2_SmbRemoveFid(DCE2_SmbSsnData *, const uint16_t, const uint16_t, const uint16_t);

static INLINE DCE2_SmbPMNode * DCE2_SmbInsertPMNode(DCE2_SmbSsnData *, const SmbNtHdr *,
                                                    DCE2_SmbFidNode *, const uint16_t);
static INLINE void DCE2_SmbRemovePMNode(DCE2_SmbSsnData *, const SmbNtHdr *);
static INLINE DCE2_SmbPMNode * DCE2_SmbFindPMNode(DCE2_SmbSsnData *, const SmbNtHdr *);
static INLINE DCE2_Ret DCE2_SmbAddDataToPMNode(DCE2_SmbSsnData *, DCE2_SmbPMNode *, const uint8_t *,
                                               uint16_t, uint16_t);

static void DCE2_SmbQueueTmpFid(DCE2_SmbSsnData *);
static void DCE2_SmbInsertFidNode(DCE2_SmbSsnData *, const uint16_t, const uint16_t,
                                  const uint16_t, DCE2_SmbFidTrackerNode *);

static INLINE void DCE2_SmbCleanFidNode(DCE2_SmbFidTrackerNode *);
static INLINE void DCE2_SmbCleanUTNode(DCE2_SmbUTNode *);
static INLINE void DCE2_SmbCleanPMNode(DCE2_SmbPMNode *);

static int DCE2_SmbUTFCompare(const void *, const void *);
static int DCE2_SmbUTPtreeCompare(const void *, const void *);
static int DCE2_SmbPMCompare(const void *a, const void *b);

static void DCE2_SmbFidDataFree(void *);
static void DCE2_SmbUTDataFree(void *);
static void DCE2_SmbFidTrackerDataFree(void *);
static void DCE2_SmbPMDataFree(void *);

static INLINE void DCE2_SmbResetForMissedPkts(DCE2_SmbSsnData *);
static void DCE2_SmbSetMissedFids(DCE2_SmbSsnData *);
static INLINE DCE2_Ret DCE2_SmbHandleSegmentation(DCE2_SmbSeg *, const uint8_t *, uint16_t, uint32_t, uint16_t *, int);
static INLINE int DCE2_SmbIsSegBuf(DCE2_SmbSsnData *, const uint8_t *);
static INLINE void DCE2_SmbSegAlert(DCE2_SmbSsnData *, DCE2_Event);
static INLINE int DCE2_SmbIsRawData(DCE2_SmbSsnData *);

static INLINE DCE2_SmbSeg * DCE2_SmbGetSegPtr(DCE2_SmbSsnData *);
static INLINE uint32_t * DCE2_SmbGetIgnorePtr(DCE2_SmbSsnData *);

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
void DCE2_SmbInitRdata(uint8_t *nb_ptr, int dir)
{
    NbssHdr *nb_hdr = (NbssHdr *)nb_ptr;
    SmbNtHdr *smb_hdr = (SmbNtHdr *)((uint8_t *)nb_hdr + sizeof(NbssHdr));

    nb_hdr->type = NBSS_SESSION_TYPE__MESSAGE;
    memcpy((void *)smb_hdr->smb_idf, (void *)"\xffSMB", sizeof(smb_hdr->smb_idf));

    if (dir == FLAG_FROM_CLIENT)
    {
        SmbLm10_WriteAndXReq *writex =
            (SmbLm10_WriteAndXReq *)((uint8_t *)smb_hdr + sizeof(SmbNtHdr));
        uint16_t offset = sizeof(SmbNtHdr) + sizeof(SmbLm10_WriteAndXReq);

        smb_hdr->smb_com = SMB_COM_WRITE_ANDX;
        smb_hdr->smb_flg = 0x00;

        writex->smb_wct = 12;
        writex->smb_com2 = SMB_COM_NO_ANDX_COMMAND;
        writex->smb_doff = SmbHtons(&offset);
    }
    else
    {
        SmbLm10_ReadAndXResp *readx =
            (SmbLm10_ReadAndXResp *)((uint8_t *)smb_hdr + sizeof(SmbNtHdr));
        uint16_t offset = sizeof(SmbNtHdr) + sizeof(SmbLm10_ReadAndXResp);

        smb_hdr->smb_com = SMB_COM_READ_ANDX;
        smb_hdr->smb_flg = 0x80;

        readx->smb_wct = 12;
        readx->smb_com2 = SMB_COM_NO_ANDX_COMMAND;
        readx->smb_doff = SmbHtons(&offset);
    }
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
void DCE2_SmbSetRdata(DCE2_SmbSsnData *ssd, uint8_t *nb_ptr, uint16_t co_len)
{
    NbssHdr *nb_hdr = (NbssHdr *)nb_ptr;
    SmbNtHdr *smb_hdr = (SmbNtHdr *)((uint8_t *)nb_hdr + sizeof(NbssHdr));

    smb_hdr->smb_uid = SmbHtons(&ssd->uid);
    smb_hdr->smb_tid = SmbHtons(&ssd->tid);

    if (DCE2_SsnFromClient(ssd->sd.wire_pkt))
    {
        SmbLm10_WriteAndXReq *writex =
            (SmbLm10_WriteAndXReq *)((uint8_t *)smb_hdr + sizeof(SmbNtHdr));
        uint32_t nb_len = sizeof(SmbNtHdr) + sizeof(SmbLm10_WriteAndXReq) + co_len;

        /* The data will get truncated anyway since we can only fit
         * 64K in the reassembly buffer */
        if (nb_len > UINT16_MAX)
            nb_len = UINT16_MAX;

        nb_hdr->length = htons((uint16_t)nb_len);

        writex->smb_fid = SmbHtons(&ssd->fid);
        writex->smb_countleft = SmbHtons(&co_len);
        writex->smb_dsize = SmbHtons(&co_len);
        writex->smb_bcc = SmbHtons(&co_len);
    }
    else
    {
        SmbLm10_ReadAndXResp *readx =
            (SmbLm10_ReadAndXResp *)((uint8_t *)smb_hdr + sizeof(SmbNtHdr));
        uint32_t nb_len = sizeof(SmbNtHdr) + sizeof(SmbLm10_ReadAndXResp) + co_len;

        /* The data will get truncated anyway since we can only fit
         * 64K in the reassembly buffer */
        if (nb_len > UINT16_MAX)
            nb_len = UINT16_MAX;

        nb_hdr->length = htons((uint16_t)nb_len);

        readx->smb_remaining = SmbHtons(&co_len);
        readx->smb_dsize = SmbHtons(&co_len);
        readx->smb_bcc = SmbHtons(&co_len);
    }
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
DCE2_SmbSsnData * DCE2_SmbSsnInit(void)
{
    DCE2_SmbSsnData *ssd = (DCE2_SmbSsnData *)DCE2_Alloc(sizeof(DCE2_SmbSsnData), DCE2_MEM_TYPE__SMB_SSN);

    if (ssd == NULL)
        return NULL;

    ssd->req_uid = DCE2_SENTINEL;
    ssd->req_tid = DCE2_SENTINEL;

    ssd->read_fid_node.fid = DCE2_SENTINEL;
    ssd->br.smb_com = DCE2_SENTINEL;

    ssd->sst.uid = DCE2_SENTINEL;
    ssd->sst.tid = DCE2_SENTINEL;
    ssd->sst.ft_node.fid_node.uid = DCE2_SENTINEL;
    ssd->sst.ft_node.fid_node.tid = DCE2_SENTINEL;
    ssd->sst.ft_node.fid_node.fid = DCE2_SENTINEL;
    DCE2_CoInitTracker(&ssd->sst.ft_node.co_tracker);

    ssd->sst.ptree.ut_node.uid = DCE2_SENTINEL;
    ssd->sst.ptree.ut_node.tid = DCE2_SENTINEL;
    ssd->sst.ptree.ut_node.ft_node.fid_node.uid = DCE2_SENTINEL;
    ssd->sst.ptree.ut_node.ft_node.fid_node.tid = DCE2_SENTINEL;
    ssd->sst.ptree.ut_node.ft_node.fid_node.fid = DCE2_SENTINEL;
    DCE2_CoInitTracker(&ssd->sst.ptree.ut_node.ft_node.co_tracker);

    ssd->pm_node.pid = DCE2_SENTINEL;
    ssd->pm_node.mid = DCE2_SENTINEL;
    ssd->pm_node.fid_node.uid = DCE2_SENTINEL;
    ssd->pm_node.fid_node.tid = DCE2_SENTINEL;
    ssd->pm_node.fid_node.fid = DCE2_SENTINEL;

    DCE2_ResetRopts(&ssd->sd.ropts);

    dce2_stats.smb_sessions++;

    return ssd;
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
void DCE2_SmbProcess(DCE2_SmbSsnData *ssd)
{
    const SFSnortPacket *p = ssd->sd.wire_pkt;
    const uint8_t *data_ptr = p->payload;
    uint16_t data_len = p->payload_size;
    uint32_t *ignore_bytes = DCE2_SmbGetIgnorePtr(ssd);
    DCE2_SmbSeg *seg = DCE2_SmbGetSegPtr(ssd);
    uint16_t overlap_bytes = DCE2_SsnGetOverlap(&ssd->sd);
    uint16_t nb_hdr_need = sizeof(NbssHdr);
    uint16_t smb_hdr_need = nb_hdr_need + sizeof(SmbNtHdr);
    DCE2_Ret status;

    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Processing SMB packet.\n"));
    dce2_stats.smb_pkts++;

    /* If we missed packets previously and couldn't autodetect, we've
     * already reset for missed packets.  If we can autodetect, move on */
    if (ssd->missed_pkts)
    {
        if (DCE2_SmbAutodetect(p) == DCE2_TRANS_TYPE__NONE)
            return;

        ssd->missed_pkts = 0;
    }
    else if (DCE2_SsnMissedPkts(&ssd->sd))
    {
        uint32_t missed_bytes = DCE2_SsnGetMissedBytes(&ssd->sd);

        if (*ignore_bytes != 0)
        {
            if (*ignore_bytes > missed_bytes)
            {
                *ignore_bytes -= missed_bytes;
                missed_bytes = 0;
            }
            else
            {
                *ignore_bytes = 0;
                missed_bytes -= *ignore_bytes;
            }
        }

        DCE2_SmbResetForMissedPkts(ssd);

        if ((missed_bytes != 0) && (DCE2_SmbAutodetect(p) == DCE2_TRANS_TYPE__NONE))
        {
            ssd->missed_pkts = 1;
            return;
        }
    }

    if (overlap_bytes != 0)
        *ignore_bytes += overlap_bytes;

    /* Have to account for segmentation.  Even though stream will give
     * us larger chunks, we might end up in the middle of something */
    while (data_len > 0)
    {
        if (*ignore_bytes)
        {
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Ignoring %u bytes\n", *ignore_bytes));

            if (data_len <= *ignore_bytes)
            {
                *ignore_bytes -= data_len;
                return;
            }
            else
            {
                /* ignore bytes is less than UINT16_MAX */
                DCE2_MOVE(data_ptr, data_len, (uint16_t)*ignore_bytes);
                *ignore_bytes = 0;
            }
        }

        if (ssd->br.smb_com != DCE2_SENTINEL)
        {
            /* If we never get responses to the raw commands, reset them */
            if (((ssd->br.smb_com == SMB_COM_WRITE_BLOCK_RAW) &&
                 DCE2_SsnFromServer(ssd->sd.wire_pkt)) ||
                ((ssd->br.smb_com == SMB_COM_READ_BLOCK_RAW) &&
                 DCE2_SsnFromClient(ssd->sd.wire_pkt)))
            {
                ssd->br.smb_com = DCE2_SENTINEL;
            }
        }

        if (DCE2_BufferIsEmpty(seg->buf))
        {
            const SmbNtHdr *smb_hdr = NULL;
            uint32_t nb_len;
            uint16_t data_used;
            uint32_t nb_need;

            /* Not enough data for NetBIOS header ... add data to segmentation buffer */
            if (data_len < nb_hdr_need)
            {
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Data len(%u) < NetBIOS SS header(%u). "
                               "Queueing data.\n", data_len, nb_hdr_need));

                DCE2_SmbHandleSegmentation(seg, data_ptr, data_len, nb_hdr_need, &data_used, 1);
                return;
            }

            nb_len = NbssLen((NbssHdr *)data_ptr);
            nb_need = nb_hdr_need + nb_len;
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "NetBIOS SS len: %u\n", nb_len));

            /* Only look at session messages - these contain SMBs */
            if (DCE2_NbssHdrChecks(ssd, (NbssHdr *)data_ptr) != DCE2_RET__SUCCESS)
            {
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Not a NetBIOS Session Message.\n"));

                *ignore_bytes = nb_need;
                dce2_stats.smb_ignored_bytes += *ignore_bytes;
                continue;
            }

            /* If we're not in block raw mode or we're waiting for server block raw response */
            if (!DCE2_SmbIsRawData(ssd))
            {
                /* Not enough data for SMB header ... add data to segmentation buffer */
                if (data_len < smb_hdr_need)
                {
                    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Data len(%u) < NetBIOS SS header + SMB header(%u). "
                                   "Queueing data.\n", data_len, smb_hdr_need));

                    seg->nb_len = nb_len;
                    DCE2_SmbHandleSegmentation(seg, data_ptr, data_len, smb_hdr_need, &data_used, 1);
                    return;
                }

                smb_hdr = (SmbNtHdr *)(data_ptr + sizeof(NbssHdr));

                /* See if this is something we need to inspect */
                if (!DCE2_SmbInspect(ssd, smb_hdr))
                {
                    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Not inspecting SMB message.\n"));

                    *ignore_bytes = nb_need;
                    dce2_stats.smb_ignored_bytes += *ignore_bytes;
                    continue;
                }

                if (DCE2_SmbHdrChecks(ssd, smb_hdr) != DCE2_RET__SUCCESS)
                {
                    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Bad SMB header.\n"));

                    *ignore_bytes = nb_need;
                    dce2_stats.smb_ignored_bytes += *ignore_bytes;
                    continue;
                }
            }
#if 0
            /* XXX Maybe determine some reasonable NBSS length for DCE/RPC requests
             * DCE/RPC fragment can be a max 65535 bytes
             * Maybe get Negotiate protocol response and use max buf size?
             * Alert on nb_len > max negotiated length, but continue processing */
            if (nb_len > SOME_MAX_LIMIT)
            {
                ssd->ignore_bytes = nb_len - SOME_MAX_LIMIT;
                dce2_stats.smb_ignored_bytes += *ignore_bytes;
                nb_len = SOME_MAX_LIMIT;
            }
#endif

            /* It's something we want to inspect so make sure we have the full NBSS packet */
            if (data_len < nb_need)
            {
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Data len(%u) < NetBIOS SS header + NetBIOS len(%u). "
                               "Queueing data.\n", data_len, nb_len));

                seg->nb_len = nb_len;
                DCE2_SmbHandleSegmentation(seg, data_ptr, data_len, nb_need, &data_used, 1);
                return;
            }

            DCE2_SmbProcessData(ssd, data_ptr, nb_need);

            /* data_len >= nb_need so it's < UINT16_MAX */
            DCE2_MOVE(data_ptr, data_len, (uint16_t)nb_need);
        }
        else
        {
            const SmbNtHdr *smb_hdr = NULL;
            const NbssHdr *nb_hdr;
            uint16_t data_used;
            uint32_t nb_need;
            int append = 0;

            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Segmentation handling => current buffer "
                           "length: %u\n", DCE2_BufferLength(seg->buf)));

            /* Not enough data yet for NetBIOS header ... add to segmentation buffer */
            if (DCE2_BufferLength(seg->buf) < nb_hdr_need)
            {
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Seg buf len(%u) < NetBIOS SS header(%u). "
                               "Queueing data.\n", DCE2_BufferLength(seg->buf), nb_hdr_need));

                append = 1;
                status = DCE2_SmbHandleSegmentation(seg, data_ptr, data_len, nb_hdr_need, &data_used, append);
                if (status != DCE2_RET__SUCCESS)
                    return;

                /* We've got the NetBIOS header */
                DCE2_MOVE(data_ptr, data_len, data_used);

                seg->nb_len = NbssLen((NbssHdr *)DCE2_BufferData(seg->buf));
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "SEG: NetBIOS SS len: %u\n", seg->nb_len));

                /* Only look at session messages - these contain SMBs */
                if (DCE2_NbssHdrChecks(ssd, (NbssHdr *)DCE2_BufferData(seg->buf)) != DCE2_RET__SUCCESS)
                {
                    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Not a NetBIOS Session Message.\n"));

                    *ignore_bytes = seg->nb_len;
                    dce2_stats.smb_ignored_bytes += *ignore_bytes;
                    DCE2_BufferEmpty(seg->buf);
                    continue;
                }
            }

            nb_hdr = (NbssHdr *)DCE2_BufferData(seg->buf);
            nb_need = nb_hdr_need + seg->nb_len;

            /* If we're not in block raw mode or we're waiting for server block raw response
             * and we don't yet have enough data in segmentation buffer for SMB header */
            if (!DCE2_SmbIsRawData(ssd) &&
                (DCE2_BufferLength(seg->buf) < smb_hdr_need))
            {
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Seg buf len(%u) < NetBIOS SS header + SMB header(%u). "
                               "Queueing data.\n", DCE2_BufferLength(seg->buf), smb_hdr_need));
                append = 1;
                status = DCE2_SmbHandleSegmentation(seg, data_ptr, data_len, smb_hdr_need, &data_used, append);

                if (status != DCE2_RET__SUCCESS)
                    return;

                /* Reset nb_hdr since the seg buffer probably needed to be realloc'ed */
                nb_hdr = (NbssHdr *)DCE2_BufferData(seg->buf); 

                /* We've got the SMB header */
                DCE2_MOVE(data_ptr, data_len, data_used);

                smb_hdr = (SmbNtHdr *)((uint8_t *)nb_hdr + sizeof(NbssHdr));

                if (!DCE2_SmbInspect(ssd, smb_hdr))
                {
                    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Not inspecting SMB message.\n"));

                    /* Already gobbled the SMB header so subtract it */
                    *ignore_bytes = seg->nb_len - sizeof(SmbNtHdr);
                    dce2_stats.smb_ignored_bytes += *ignore_bytes;
                    DCE2_BufferEmpty(seg->buf);
                    continue;
                }

                if (DCE2_SmbHdrChecks(ssd, smb_hdr) != DCE2_RET__SUCCESS)
                {
                    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "SEG: Bad SMB header.\n"));

                    *ignore_bytes = seg->nb_len - sizeof(SmbNtHdr);
                    dce2_stats.smb_ignored_bytes += *ignore_bytes;
                    DCE2_BufferEmpty(seg->buf);
                    continue;
                }
            }

            /* It's something we want to inspect so make sure we have the full NBSS packet */
            if (DCE2_BufferLength(seg->buf) < nb_need)
            {
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Seg buf len(%u) < NetBIOS SS header + seg->nb_len(%u). "
                               "Queueing data.\n", DCE2_BufferLength(seg->buf), seg->nb_len));

                status = DCE2_SmbHandleSegmentation(seg, data_ptr, data_len, nb_need, &data_used, append);
                if (status != DCE2_RET__SUCCESS)
                    return;

                /* We've got the NetBIOS data */
                DCE2_MOVE(data_ptr, data_len, data_used);
            }

            DCE2_SmbProcessData(ssd, DCE2_BufferData(seg->buf), DCE2_BufferLength(seg->buf));
            DCE2_BufferEmpty(seg->buf);
        }
    }
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
static int DCE2_SmbInspect(DCE2_SmbSsnData *ssd, const SmbNtHdr *smb_hdr)
{
    DCE2_Policy policy = DCE2_ScPolicy(ssd->sd.sconfig);
    int smb_com = SmbCom(smb_hdr);

    /* Don't support SMB2 yet */
    if (SmbId(smb_hdr) == DCE2_SMB2_ID)
        return 0;

    /* See if this is something we need to inspect */
    switch (smb_com)
    {
        case SMB_COM_NEGPROT:
            /* Don't really need to look at this */
            return 0;

        case SMB_COM_SESS_SETUP_ANDX:
        case SMB_COM_LOGOFF_ANDX:
        case SMB_COM_TREE_CON:
        case SMB_COM_TREE_CON_ANDX:
        case SMB_COM_RENAME:
            break;

        case SMB_COM_OPEN:
        case SMB_COM_WRITE_AND_CLOSE:
        case SMB_COM_READ:
        case SMB_COM_READ_BLOCK_RAW:
        case SMB_COM_WRITE_BLOCK_RAW:
            /* Samba doesn't allow these commands under an IPC tree */
            switch (policy)
            {
                case DCE2_POLICY__SAMBA:
                case DCE2_POLICY__SAMBA_3_0_37:
                case DCE2_POLICY__SAMBA_3_0_22:
                case DCE2_POLICY__SAMBA_3_0_20:
                    return 0;

                default:
                    break;
            }

            /* Fall through */

        default:
            if (DCE2_SmbFindTid(ssd, SmbTid(smb_hdr)) != DCE2_RET__SUCCESS)
            {
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Couldn't find IPC tid (%u)\n",
                               SmbTid(smb_hdr)));

                if (DCE2_SsnFromClient(ssd->sd.wire_pkt) ||
                    (DCE2_SsnFromServer(ssd->sd.wire_pkt) && !ssd->chained_tc))
                {
                    dce2_stats.smb_non_ipc_packets++;
                    return 0;
                }

                ssd->chained_tc = 0;
            }

            break;
    }

    return 1;
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
static void DCE2_SmbHandleCom(DCE2_SmbSsnData *ssd, const SmbNtHdr *smb_hdr,
                              const uint8_t *nb_ptr, uint32_t nb_len)
{
    DCE2_Ret status;

    DCE2_SmbIncComStat(smb_hdr);

    /* In case we get a reassembled packet */
    ssd->uid = SmbUid(smb_hdr);
    ssd->tid = SmbTid(smb_hdr);

    if (SmbType(smb_hdr) == SMB_TYPE__REQUEST)
    {
        ssd->req_uid = SmbUid(smb_hdr);
        ssd->req_tid = SmbTid(smb_hdr);
    }

    /* Handle the command */
    switch (SmbCom(smb_hdr))
    {
        case SMB_COM_SESS_SETUP_ANDX:
            DCE2_SmbSessSetupAndX(ssd, smb_hdr, nb_ptr, nb_len);
            break;

        case SMB_COM_LOGOFF_ANDX:
            DCE2_SmbLogoffAndX(ssd, smb_hdr, nb_ptr, nb_len, 0);
            break;

        case SMB_COM_TREE_CON:
            status = DCE2_SmbTreeConnect(ssd, smb_hdr, nb_ptr, nb_len);
            if ((SmbType(smb_hdr) == SMB_TYPE__REQUEST) && (status == DCE2_RET__SUCCESS))
                DCE2_SmbTreeConnectEnqueue(ssd, smb_hdr, status);
            break;

        case SMB_COM_TREE_CON_ANDX:
            DCE2_SmbTreeConnectAndX(ssd, smb_hdr, nb_ptr, nb_len);
            break;

        case SMB_COM_TREE_DIS:
            DCE2_SmbTreeDisconnect(ssd, smb_hdr, nb_ptr, nb_len);
            break;

        case SMB_COM_OPEN:
            DCE2_SmbOpen(ssd, smb_hdr, nb_ptr, nb_len);
            break;

        case SMB_COM_OPEN_ANDX:
            DCE2_SmbOpenAndX(ssd, smb_hdr, nb_ptr, nb_len);
            break;

        case SMB_COM_NT_CREATE_ANDX:
            DCE2_SmbNtCreateAndX(ssd, smb_hdr, nb_ptr, nb_len);
            break;

        case SMB_COM_CLOSE:
            DCE2_SmbClose(ssd, smb_hdr, nb_ptr, nb_len, 0);
            break;

        case SMB_COM_TRANS:
            DCE2_SmbTrans(ssd, smb_hdr, nb_ptr, nb_len);
            break;

        case SMB_COM_TRANS_SEC:
            DCE2_SmbTransSec(ssd, smb_hdr, nb_ptr, nb_len);
            break;

        case SMB_COM_WRITE:
            DCE2_SmbWrite(ssd, smb_hdr, nb_ptr, nb_len);
            break;

        case SMB_COM_WRITE_BLOCK_RAW:
            DCE2_SmbWriteBlockRaw(ssd, smb_hdr, nb_ptr, nb_len);
            break;

        case SMB_COM_WRITE_ANDX:
            DCE2_SmbWriteAndX(ssd, smb_hdr, nb_ptr, nb_len);
            break;

        case SMB_COM_WRITE_AND_CLOSE:
            DCE2_SmbWriteAndClose(ssd, smb_hdr, nb_ptr, nb_len);
            break;

        case SMB_COM_READ:
            DCE2_SmbRead(ssd, smb_hdr, nb_ptr, nb_len);
            break;

        case SMB_COM_READ_BLOCK_RAW:
            DCE2_SmbReadBlockRaw(ssd, smb_hdr, nb_ptr, nb_len);
            break;

        case SMB_COM_READ_ANDX:
            DCE2_SmbReadAndX(ssd, smb_hdr, nb_ptr, nb_len);
            break;

        case SMB_COM_RENAME:
            DCE2_SmbRename(ssd, smb_hdr, nb_ptr, nb_len);
            break;

        default:
            break;
    }
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
static int DCE2_SmbGetComSize(DCE2_SmbSsnData *ssd, const SmbNtHdr *smb_hdr,
                              const SmbCommon *sc, const int com)
{
    const int smb_type = SmbType(smb_hdr);
    uint8_t wct = SmbWct(sc);
    int alert = 0;

    /* XXX Might need to only check to make sure word count is greater than
     * the smallest word count.  Watch out for interim or empty commands */
    if (smb_type == SMB_TYPE__REQUEST)
    {
        switch (com)
        {
            case SMB_COM_NEGPROT:
                switch (wct)
                {
                    case 0:
                        break;
                    default:
                        alert = 1;
                        break;
                }

                break;

            case SMB_COM_SESS_SETUP_ANDX:
                switch (wct)
                {
                    case 10:
                    case 12:
                    case 13:
                        break;
                    default:
                        alert = 1;
                        break;
                }
                
                break;

            case SMB_COM_LOGOFF_ANDX:
                switch (wct)
                {
                    case 2:
                        break;
                    default:
                        alert = 1;
                        break;
                }

                break;

            case SMB_COM_TREE_CON:
                switch (wct)
                {
                    case 0:
                        break;
                    default:
                        alert = 1;
                        break;
                }

                break;

            case SMB_COM_TREE_CON_ANDX:
                switch (wct)
                {
                    case 4:
                        break;
                    default:
                        alert = 1;
                        break;
                }

                break;

            case SMB_COM_TREE_DIS:
                switch (wct)
                {
                    case 0:
                        break;
                    default:
                        alert = 1;
                        break;
                }

                break;

            case SMB_COM_OPEN:
                switch (wct)
                {
                    case 2:
                        break;
                    default:
                        alert = 1;
                        break;
                }

                break;

            case SMB_COM_OPEN_ANDX:
                switch (wct)
                {
                    case 15:
                        break;
                    default:
                        alert = 1;
                        break;
                }

                break;

            case SMB_COM_NT_CREATE_ANDX:
                switch (wct)
                {
                    case 24:
                        break;
                    default:
                        alert = 1;
                        break;
                }

                break;

            case SMB_COM_CLOSE:
                switch (wct)
                {
                    case 3:
                        break;
                    default:
                        alert = 1;
                        break;
                }

                break;

            case SMB_COM_WRITE:
                switch (wct)
                {
                    case 5:
                        break;
                    default:
                        alert = 1;
                        break;
                }

                break;

            case SMB_COM_TRANS:
                /* This is for a Transaction with Named Pipe function */
                switch (wct)
                {
                    case 16:
                        break;
                    default:
                        alert = 1;
                        break;
                }

                break;

            case SMB_COM_TRANS_SEC:
                switch (wct)
                {
                    case 8:
                        break;
                    default:
                        alert = 1;
                        break;
                }

                break;

            case SMB_COM_WRITE_AND_CLOSE:
                switch (wct)
                {
                    case 6:
                    case 12:
                        break;
                    default:
                        alert = 1;
                        break;
                }

                break;

            case SMB_COM_WRITE_BLOCK_RAW:
                switch (wct)
                {
                    case 12:
                    case 14:
                        break;
                    default:
                        alert = 1;
                        break;
                }

                break;

            case SMB_COM_WRITE_ANDX:
                switch (wct)
                {
                    case 12:
                    case 14:
                        break;
                    default:
                        alert = 1;
                        break;
                }

                break;

            case SMB_COM_READ:
                switch (wct)
                {
                    case 5:
                        break;
                    default:
                        alert = 1;
                        break;
                }

                break;

            case SMB_COM_READ_BLOCK_RAW:
                switch (wct)
                {
                    case 8:
                    case 10:
                        break;
                    default:
                        alert = 1;
                        break;
                }

                break;

            case SMB_COM_READ_ANDX:
                switch (wct)
                {
                    case 10:
                    case 12:
                        break;
                    default:
                        alert = 1;
                        break;
                }

                break;

            case SMB_COM_RENAME:
                switch (wct)
                {
                    case 1:
                        break;
                    default:
                        alert = 1;
                        break;
                }

                break;

            default:
                DCE2_Log(DCE2_LOG_TYPE__WARN,
                         "%s(%d) Word count check for unused command: 0x%02x",
                         __FILE__, __LINE__, com);
                break;
        }
    }
    else  /* it's a response */
    {
        switch (com)
        {
            case SMB_COM_NEGPROT:
                switch (wct)
                {
                    case 1:
                    case 13:
                    case 17:
                        break;
                    default:
                        alert = 1;
                        break;
                }

                break;

            case SMB_COM_SESS_SETUP_ANDX:
                switch (wct)
                {
                    case 3:
                    case 4:
                        break;
                    default:
                        alert = 1;
                        break;
                }

                break;

            case SMB_COM_LOGOFF_ANDX:
                switch (wct)
                {
                    case 3:
                        /* Windows responds to a LogoffAndX => SessionSetupAndX with just a
                         * LogoffAndX and with the word count field containing 3, but is only
                         * a word count of 2 */
                        wct = 2;
                    case 2:
                        break;
                    default:
                        alert = 1;
                        break;
                }

                break;

            case SMB_COM_TREE_CON:
                switch (wct)
                {
                    case 2:
                        break;
                    default:
                        alert = 1;
                        break;
                }

                break;

            case SMB_COM_TREE_CON_ANDX:
                switch (wct)
                {
                    case 2:
                    case 3:
                    case 7:
                        break;
                    default:
                        alert = 1;
                        break;
                }

                break;

            case SMB_COM_TREE_DIS:
                switch (wct)
                {
                    case 0:
                        break;
                    default:
                        alert = 1;
                        break;
                }

                break;

            case SMB_COM_OPEN:
                switch (wct)
                {
                    case 7:
                        break;
                    default:
                        alert = 1;
                        break;
                }

                break;

            case SMB_COM_OPEN_ANDX:
                switch (wct)
                {
                    case 15:
                        break;
                    default:
                        alert = 1;
                        break;
                }

                break;

            case SMB_COM_NT_CREATE_ANDX:
                switch (wct)
                {
                    case 42:
                        /* Specification says word count is 34, but servers (Windows and
                         * Samba) respond with word count of 42.  Wireshark decodes as word
                         * count 34, but there is extra data at end of packet. The byte
                         * count however is located as if it was a 34 word count */
                        wct = 34;
                    case 34:
                        break;
                    default:
                        alert = 1;
                        break;
                }

                break;

            case SMB_COM_CLOSE:
                switch (wct)
                {
                    case 0:
                        break;
                    default:
                        alert = 1;
                        break;
                }

                break;

            case SMB_COM_WRITE:
                switch (wct)
                {
                    case 1:
                        break;
                    default:
                        alert = 1;
                        break;
                }

                break;

            case SMB_COM_TRANS:
                switch (wct)
                {
                    case 0:  /* Interim Transact response - no data */
                    case 10:
                        break;
                    default:
                        alert = 1;
                        break;
                }

                break;

            case SMB_COM_WRITE_AND_CLOSE:
                switch (wct)
                {
                    case 1:
                        break;
                    default:
                        alert = 1;
                        break;
                }

                break;

            case SMB_COM_WRITE_BLOCK_RAW:
                switch (wct)
                {
                    case 1:
                        break;
                    default:
                        alert = 1;
                        break;
                }

                break;

            case SMB_COM_WRITE_COMPLETE:
                switch (wct)
                {
                    case 1:
                        break;
                    default:
                        alert = 1;
                        break;
                }

                break;

            case SMB_COM_WRITE_ANDX:
                switch (wct)
                {
                    case 6:
                        break;
                    default:
                        alert = 1;
                        break;
                }

                break;

            case SMB_COM_READ:
                switch (wct)
                {
                    case 5:
                        break;
                    default:
                        alert = 1;
                        break;
                }

                break;

            case SMB_COM_READ_ANDX:
                switch (wct)
                {
                    case 12:
                        break;
                    default:
                        alert = 1;
                        break;
                }

                break;

            case SMB_COM_RENAME:
                switch (wct)
                {
                    case 0:
                        break;
                    default:
                        alert = 1;
                        break;
                }

                break;

            default:
                DCE2_Log(DCE2_LOG_TYPE__WARN,
                         "%s(%d) Word count check for unused command: 0x%02x",
                         __FILE__, __LINE__, com);
                break;
        }
    }

    if (alert)
    {
        DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_BAD_WCT, wct);
        return -1;
    }

    return SMB_COM_SIZE(wct);
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
static int DCE2_SmbGetBcc(DCE2_SmbSsnData *ssd, const SmbNtHdr *smb_hdr,
                          const SmbCommon *com_ptr, const uint16_t com_size, const int com)
{
    const int smb_type = SmbType(smb_hdr);
    const uint8_t wct = SmbWct(com_ptr);
    const uint16_t bcc = SmbBcc((uint8_t *)com_ptr, com_size);
    int alert = 0;

    if (smb_type == SMB_TYPE__REQUEST)
    {
        switch (com)
        {
            case SMB_COM_NEGPROT:
                if (bcc < 2)
                    alert = 1;
                break;

            case SMB_COM_SESS_SETUP_ANDX:
                break;

            case SMB_COM_LOGOFF_ANDX:
                if (bcc != 0)
                    alert = 1;
                break;

            case SMB_COM_TREE_CON:
                if (bcc < 4)
                    alert = 1;
                break;

            case SMB_COM_TREE_CON_ANDX:
                if (bcc < 3)
                    alert = 1;
                break;

            case SMB_COM_TREE_DIS:
                if (bcc != 0)
                    alert = 1;
                break;

            case SMB_COM_OPEN:
                if (bcc < 2)
                    alert = 1;
                break;

            case SMB_COM_OPEN_ANDX:
                if (bcc < 1)
                    alert = 1;
                break;

            case SMB_COM_NT_CREATE_ANDX:
                break;

            case SMB_COM_CLOSE:
                if (bcc != 0)
                    alert = 1;
                break;

            case SMB_COM_WRITE:
                if (bcc < 3)
                    alert = 1;
                break;

            case SMB_COM_TRANS:
                break;

            case SMB_COM_TRANS_SEC:
                break;

            case SMB_COM_WRITE_AND_CLOSE:
                if (bcc < 1)
                    alert = 1;
                break;

            case SMB_COM_WRITE_BLOCK_RAW:
                break;

            case SMB_COM_WRITE_ANDX:
                break;

            case SMB_COM_READ:
                if (bcc != 0)
                    alert = 1;
                break;

            case SMB_COM_READ_BLOCK_RAW:
                if (bcc != 0)
                    alert = 1;
                break;

            case SMB_COM_READ_ANDX:
                if (bcc != 0)
                    alert = 1;
                break;

            case SMB_COM_RENAME:
                if (bcc < 4)
                    alert = 1;
                break;

            default:
                DCE2_Log(DCE2_LOG_TYPE__WARN,
                         "%s(%d) Byte count check for unused command: 0x%02x",
                         __FILE__, __LINE__, com);
                break;
        }
    }
    else  /* it's a response */
    {
        switch (com)
        {
            case SMB_COM_NEGPROT:
                break;

            case SMB_COM_SESS_SETUP_ANDX:
                break;

            case SMB_COM_LOGOFF_ANDX:
                if (bcc != 0)
                    alert = 1;
                break;

            case SMB_COM_TREE_CON:
                break;

            case SMB_COM_TREE_CON_ANDX:
                if (bcc < 3)
                    alert = 1;
                break;

            case SMB_COM_TREE_DIS:
                if (bcc != 0)
                    alert = 1;
                break;

            case SMB_COM_OPEN:
                if (bcc != 0)
                    alert = 1;
                break;

            case SMB_COM_OPEN_ANDX:
                if (bcc != 0)
                    alert = 1;
                break;

            case SMB_COM_NT_CREATE_ANDX:
                if (bcc != 0)
                    alert = 1;
                break;

            case SMB_COM_CLOSE:
                if (bcc != 0)
                    alert = 1;
                break;

            case SMB_COM_WRITE:
                switch (wct)
                {
                    case 1:
                        break;
                    default:
                        alert = 1;
                        break;
                }

                break;

            case SMB_COM_TRANS:
                switch (wct)
                {
                    case 0:  /* Interim Transact response - no data */
                        if (bcc != 0)
                            alert = 1;
                        break;
                    default:
                        break;
                }

                break;

            case SMB_COM_WRITE_AND_CLOSE:
                break;

            case SMB_COM_WRITE_BLOCK_RAW:
                if (bcc != 0)
                    alert = 1;
                break;

            case SMB_COM_WRITE_COMPLETE:
                if (bcc != 0)
                    alert = 1;
                break;

            case SMB_COM_WRITE_ANDX:
                if (bcc != 0)
                    alert = 1;
                break;

            case SMB_COM_READ:
                if (bcc < 3)
                    alert = 1;
                break;

            case SMB_COM_READ_ANDX:
                break;

            case SMB_COM_RENAME:
                if (bcc != 0)
                    alert = 1;
                break;

            default:
                DCE2_Log(DCE2_LOG_TYPE__WARN,
                         "%s(%d) Byte count check for unused command: 0x%02x",
                         __FILE__, __LINE__, com);
                break;
        }
    }

    if (alert)
    {
        DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_BAD_BCC, bcc);
        return -1;
    }

    return bcc;
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
static INLINE DCE2_Ret DCE2_SmbCheckComSize(DCE2_SmbSsnData *ssd, const uint32_t nb_len,
                                            const uint16_t com_len, const int smb_com)
{
    if (nb_len < com_len)
    {
        DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_NB_LT_COM, nb_len, com_len);

        return DCE2_RET__ERROR;
    }

    return DCE2_RET__SUCCESS;
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
static INLINE DCE2_Ret DCE2_SmbCheckBcc(DCE2_SmbSsnData *ssd, const uint32_t nb_len,
                                        const uint16_t bcc, const int smb_com)
{
    if (nb_len < bcc)
    {
        DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_NB_LT_BCC, nb_len, bcc);

        return DCE2_RET__ERROR;
    }

    return DCE2_RET__SUCCESS;
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
static INLINE DCE2_Ret DCE2_SmbCheckDsize(DCE2_SmbSsnData *ssd, const uint32_t nb_len,
                                          const uint16_t dsize, const uint16_t bcc, const int smb_com)
{
    if (nb_len < dsize)
    {
        DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_NB_LT_DSIZE, nb_len, dsize);

        return DCE2_RET__ERROR;
    }
    else if (bcc < dsize)
    {
        DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_BCC_LT_DSIZE, bcc, dsize);

        return DCE2_RET__ERROR;
    }

    return DCE2_RET__SUCCESS;
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
static INLINE DCE2_Ret DCE2_SmbCheckTotDcnt(DCE2_SmbSsnData *ssd, const uint16_t dcnt,
                                            const uint16_t total_dcnt, const int smb_com)
{
    if (total_dcnt < dcnt)
    {
        DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_TDCNT_LT_DSIZE,
                (int)total_dcnt, (int)dcnt);

        return DCE2_RET__ERROR;
    }
    else if (total_dcnt == 0)
    {
        DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_TDCNT_ZERO);
        return DCE2_RET__ERROR;
    }

    return DCE2_RET__SUCCESS;
}

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *  uint8_t * - pointer to where the offset would take us.
 *  uint8_t * - pointer to bound offset
 *  uint8_t * - length of data where offset should be within
 *
 * Returns:
 *  DCE2_RET__SUCCESS - Offset is okay.
 *  DCE2_RET__ERROR   - Offset is bad.
 *
 ********************************************************************/
static INLINE DCE2_Ret DCE2_SmbCheckOffset(DCE2_SmbSsnData *ssd, const uint8_t *off_ptr,
                                           const uint8_t *start_bound, const uint32_t length,
                                           const int smb_com)
{
    /* Offset should not point within data we just looked at or be equal to
     * or beyond the length of the NBSS length left */
    if ((off_ptr < start_bound) ||
        (off_ptr > (start_bound + length)))
    {
        DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_BAD_OFF, off_ptr,
                start_bound, start_bound + length);

        return DCE2_RET__ERROR;
    }

    return DCE2_RET__SUCCESS;
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
static DCE2_Ret DCE2_NbssHdrChecks(DCE2_SmbSsnData *ssd, const NbssHdr *nb_hdr)
{
    const SFSnortPacket *p = ssd->sd.wire_pkt;
    int is_seg_buf = DCE2_SmbIsSegBuf(ssd, (uint8_t *)nb_hdr);

    switch (NbssType(nb_hdr))
    {
        case NBSS_SESSION_TYPE__MESSAGE:
            /* Only want to look at session messages */
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "NetBIOS SS type: Message.\n"));

            if (!DCE2_SmbIsRawData(ssd))
            {
                uint32_t nb_len = NbssLen(nb_hdr);

                if (nb_len == 0)
                    return DCE2_RET__IGNORE;

                if (nb_len < sizeof(SmbNtHdr))
                {
                    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "NetBIOS SS len(%u) < SMB header len(%u).\n",
                                   sizeof(SmbNtHdr), sizeof(NbssHdr) + nb_len));

                    if (is_seg_buf)
                        DCE2_SmbSegAlert(ssd, DCE2_EVENT__SMB_NB_LT_SMBHDR);
                    else
                        DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_NB_LT_SMBHDR, nb_len, sizeof(SmbNtHdr));

                    return DCE2_RET__IGNORE;
                }
            }

            return DCE2_RET__SUCCESS;

        case NBSS_SESSION_TYPE__REQUEST:
            dce2_stats.smb_nbss_not_message++;
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "NetBIOS SS type: Request.\n"));
            if (DCE2_SsnFromServer(p))
            {
                if (is_seg_buf)
                    DCE2_SmbSegAlert(ssd, DCE2_EVENT__SMB_BAD_NBSS_TYPE);
                else
                    DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_BAD_NBSS_TYPE);
            }

            break;

        case NBSS_SESSION_TYPE__POS_RESPONSE:
        case NBSS_SESSION_TYPE__NEG_RESPONSE:
        case NBSS_SESSION_TYPE__RETARGET_RESPONSE:
            dce2_stats.smb_nbss_not_message++;
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB,
                           "NetBIOS SS type: Response, Negative response or Retarget response.\n"));
            if (DCE2_SsnFromClient(p))
            {
                if (is_seg_buf)
                    DCE2_SmbSegAlert(ssd, DCE2_EVENT__SMB_BAD_NBSS_TYPE);
                else
                    DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_BAD_NBSS_TYPE);
            }

            break;

        case NBSS_SESSION_TYPE__KEEP_ALIVE:
            dce2_stats.smb_nbss_not_message++;
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "NetBIOS SS type: Keep alive.\n"));
            break;

        default:
            dce2_stats.smb_nbss_not_message++;
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "NetBIOS SS type: Invalid.\n"));

            if (is_seg_buf)
                DCE2_SmbSegAlert(ssd, DCE2_EVENT__SMB_BAD_NBSS_TYPE);
            else
                DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_BAD_NBSS_TYPE);

            break;
    }

    return DCE2_RET__IGNORE;
}

/********************************************************************
 * Function: DCE2_SmbHdrChecks()
 *
 * Checks some relevant fields in the header to make sure they're
 * sane.
 *
 * Arguments:
 *  DCE2_SmbSsnData *
 *      Pointer to the session data structure.
 *  SmbNtHdr *
 *      Pointer to the header struct layed over the packet data.
 *
 * Returns:
 *  DCE2_Ret
 *      DCE2_RET__IGNORE if we should continue processing, but
 *          ignore data because of the error.
 *      DCE2_RET__SUCCESS if we should continue processing.
 *
 ********************************************************************/
static DCE2_Ret DCE2_SmbHdrChecks(DCE2_SmbSsnData *ssd, const SmbNtHdr *smb_hdr)
{
    const SFSnortPacket *p = ssd->sd.wire_pkt;
    int is_seg_buf = DCE2_SmbIsSegBuf(ssd, (uint8_t *)smb_hdr);

    if ((DCE2_SsnFromServer(p) && (SmbType(smb_hdr) == SMB_TYPE__REQUEST)) ||
        (DCE2_SsnFromClient(p) && (SmbType(smb_hdr) == SMB_TYPE__RESPONSE)))
    {
        if (is_seg_buf)
            DCE2_SmbSegAlert(ssd, DCE2_EVENT__SMB_BAD_TYPE);
        else
            DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_BAD_TYPE);

        return DCE2_RET__IGNORE;
    }

    if (SmbId(smb_hdr) != DCE2_SMB_ID)
    {
        if (is_seg_buf)
            DCE2_SmbSegAlert(ssd, DCE2_EVENT__SMB_BAD_ID);
        else
            DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_BAD_ID);

        return DCE2_RET__IGNORE;
    }

    return DCE2_RET__SUCCESS;
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
static void DCE2_SmbSessSetupAndX(DCE2_SmbSsnData *ssd, const SmbNtHdr *smb_hdr,
                                  const uint8_t *nb_ptr, uint32_t nb_len)
{
    const int smb_com = SMB_COM_SESS_SETUP_ANDX;
    const int smb_type = SmbType(smb_hdr);
    const SmbAndXCommon *andx = (SmbAndXCommon *)nb_ptr;
    int com_size, bcc;

    if (smb_type == SMB_TYPE__RESPONSE)
    {
        const SmbEmptyCom *ec = (SmbEmptyCom *)nb_ptr;

        if (DCE2_SmbCheckComSize(ssd, nb_len, sizeof(SmbEmptyCom), smb_com) != DCE2_RET__SUCCESS)
            return;

        /* Server didn't accept client request */
        if ((SmbEmptyComWct(ec) == 0) && (SmbEmptyComBcc(ec) == 0) && SmbError(smb_hdr))
            return;
    }

    if (DCE2_SmbCheckComSize(ssd, nb_len, sizeof(SmbAndXCommon), smb_com) != DCE2_RET__SUCCESS)
        return;

    com_size = DCE2_SmbGetComSize(ssd, smb_hdr, (SmbCommon *)andx, smb_com);
    if ((com_size < 0) ||
        (DCE2_SmbCheckComSize(ssd, nb_len, (uint16_t)com_size, smb_com) != DCE2_RET__SUCCESS))
        return;

    DCE2_MOVE(nb_ptr, nb_len, com_size);

    bcc = DCE2_SmbGetBcc(ssd, smb_hdr, (SmbCommon *)andx, (uint16_t)com_size, smb_com);
    if ((bcc < 0) ||
        (DCE2_SmbCheckBcc(ssd, nb_len, (uint16_t)bcc, smb_com) != DCE2_RET__SUCCESS))
        return;

    DCE2_MOVE(nb_ptr, nb_len, bcc);

    if (smb_type == SMB_TYPE__RESPONSE)
    {
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Inserting uid: %u\n", SmbUid(smb_hdr)));
        DCE2_SmbInsertUid(ssd, SmbUid(smb_hdr));
    }

    if (SmbAndXCom2(andx) != SMB_COM_NO_ANDX_COMMAND)
        DCE2_SmbChained(ssd, smb_hdr, andx, smb_com, nb_ptr, nb_len);
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
static void DCE2_SmbLogoffAndX(DCE2_SmbSsnData *ssd, const SmbNtHdr *smb_hdr,
                               const uint8_t *nb_ptr, uint32_t nb_len, int ssx_chained)
{
    const int smb_com = SMB_COM_LOGOFF_ANDX;
    const int smb_type = SmbType(smb_hdr);
    const SmbAndXCommon *andx = (SmbAndXCommon *)nb_ptr;
    int com_size, bcc;

    if (smb_type == SMB_TYPE__RESPONSE)
    {
        SmbEmptyCom *ec = (SmbEmptyCom *)nb_ptr;

        if (DCE2_SmbCheckComSize(ssd, nb_len, sizeof(SmbEmptyCom), smb_com) != DCE2_RET__SUCCESS)
            return;

        /* Server didn't accept client request */
        if ((SmbEmptyComWct(ec) == 0) && (SmbEmptyComBcc(ec) == 0) && SmbError(smb_hdr))
            return;
    }

    if (DCE2_SmbCheckComSize(ssd, nb_len, sizeof(SmbAndXCommon), smb_com) != DCE2_RET__SUCCESS)
        return;

    com_size = DCE2_SmbGetComSize(ssd, smb_hdr, (SmbCommon *)andx, smb_com);
    if ((com_size < 0) ||
        (DCE2_SmbCheckComSize(ssd, nb_len, (uint16_t)com_size, smb_com) != DCE2_RET__SUCCESS))
        return;

    DCE2_MOVE(nb_ptr, nb_len, com_size);

    bcc = DCE2_SmbGetBcc(ssd, smb_hdr, (SmbCommon *)andx, (uint16_t)com_size, smb_com);
    if ((bcc < 0) ||
        (DCE2_SmbCheckBcc(ssd, nb_len, (uint16_t)bcc, smb_com) != DCE2_RET__SUCCESS))
        return;

    /* Assume above checks are sufficient such that this will succeed */
    if (smb_type == SMB_TYPE__REQUEST)
    {
        /* If there is a SessionSetupAndX chaining the LogoffAndX, the LogoffAndX will
         * apply to the newly created Uid created by the preceding SessionSetupAndX.
         * Don't remove the client request Uid */
        if (!ssx_chained)
        {
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Removing uid: %u\n", SmbUid(smb_hdr)));
            DCE2_SmbRemoveUid(ssd, SmbUid(smb_hdr));
        }
    }
    else
    {
        /* Should only apply to Samba */
        if (ssx_chained)
        {
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Removing uid: %u\n", SmbUid(smb_hdr)));
            DCE2_SmbRemoveUid(ssd, SmbUid(smb_hdr));
        }

        switch (DCE2_ScPolicy(ssd->sd.sconfig))
        {
            case DCE2_POLICY__WIN2000:
            case DCE2_POLICY__WINXP:
            case DCE2_POLICY__WINVISTA:
            case DCE2_POLICY__WIN2003:
            case DCE2_POLICY__WIN2008:
            case DCE2_POLICY__WIN7:
                /* Windows responds to a chained LogoffAndX => SessionSetupAndX with a
                 * word count 3 LogoffAndX without the chained SessionSetupAndX */
                if (SmbWct((SmbCommon *)andx) == 3)
                    DCE2_SmbInsertUid(ssd, SmbUid(smb_hdr));

                break;

            default:
                break;
        }
    }

    DCE2_MOVE(nb_ptr, nb_len, bcc);

    if (SmbAndXCom2(andx) != SMB_COM_NO_ANDX_COMMAND)
        DCE2_SmbChained(ssd, smb_hdr, andx, smb_com, nb_ptr, nb_len);
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
static DCE2_Ret DCE2_SmbTreeConnect(DCE2_SmbSsnData *ssd, const SmbNtHdr *smb_hdr,
                                    const uint8_t *nb_ptr, uint32_t nb_len)
{
    const int smb_com = SMB_COM_TREE_CON;
    const int smb_type = SmbType(smb_hdr);
    const SmbCommon *sc = (SmbCommon *)nb_ptr;
    int com_size, bcc;

    /* Don't care what we return for the server so just return success */
    if (smb_type == SMB_TYPE__RESPONSE)
    {
        int is_ipc;

        if (ssd->tc_queue == NULL)
            return DCE2_RET__SUCCESS;

        if (DCE2_SsnAlerted(&ssd->sd, DCE2_EVENT__SMB_EXCESSIVE_TREE_CONNECTS))
        {
            /* Assume they're all IPC.  Want to see what's going on */
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB,
                           "Excessive tree connects.  Inserting tid: %u\n", SmbTid(smb_hdr)));
            DCE2_SmbInsertTid(ssd, SmbTid(smb_hdr));
        }

        is_ipc = (int)(uintptr_t)DCE2_CQueueDequeue(ssd->tc_queue);
        if (is_ipc != DCE2_TC__IPC)
            return DCE2_RET__SUCCESS;
        
        /* Didn't get a positive response */
        if (SmbError(smb_hdr))
            return DCE2_RET__SUCCESS;
    }

    if (DCE2_SmbCheckComSize(ssd, nb_len, sizeof(SmbCommon), smb_com) != DCE2_RET__SUCCESS)
        return DCE2_RET__ERROR;

    com_size = DCE2_SmbGetComSize(ssd, smb_hdr, sc, smb_com);
    if ((com_size < 0) ||
        (DCE2_SmbCheckComSize(ssd, nb_len, (uint16_t)com_size, smb_com) != DCE2_RET__SUCCESS))
        return DCE2_RET__ERROR;

    DCE2_MOVE(nb_ptr, nb_len, com_size);

    bcc = DCE2_SmbGetBcc(ssd, smb_hdr, sc, (uint16_t)com_size, smb_com);
    if ((bcc < 0) ||
        (DCE2_SmbCheckBcc(ssd, nb_len, (uint16_t)bcc, smb_com) != DCE2_RET__SUCCESS))
        return DCE2_RET__ERROR;

    if (smb_type == SMB_TYPE__REQUEST)
    {
        const uint8_t *bs = NULL;
        unsigned int bs_count = 0;
        const uint8_t ipc_unicode[] = {'I', '\0', 'P', '\0', 'C', '\0', '$', '\0', '\0', '\0'};
        const uint8_t ipc_ascii[] = {'I', 'P', 'C', '$', '\0'};
        const uint8_t *ipc_chars;
        unsigned int ipc_len;
        unsigned int i;

        /* Have at least 4 bytes */

        /* If unicode flag is set, strings, except possibly the service string
         * are going to be unicode.  The NT spec specifies that unicode strings 
         * must be word aligned with respect to the beginning of the SMB and that for
         * type-prefixed strings (this case), the padding byte is found after the
         * type format byte */

        /* This byte will realign things. */
        if (*nb_ptr != SMB_FMT__ASCII)
        {
            DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_BAD_FORMAT, *nb_ptr);
            return DCE2_RET__ERROR;
        }

        DCE2_MOVE(nb_ptr, nb_len, 1);

        /* IPC$ does not need to be case insensitive.  And the case sensitivity flag in
         * the SMB header doesn't seem to have any effect on this. */
        while ((bs = memchr(nb_ptr, '\\', nb_len)) != NULL)
        {
            DCE2_MOVE(nb_ptr, nb_len, (bs - nb_ptr) + 1);
            bs_count++;
        }

        if (SmbUnicode(smb_hdr) && (nb_len > 0))
        {
            DCE2_MOVE(nb_ptr, nb_len, 1);
        }

#if 0
        /* This code is not completely verified */
        /* more than \\path\IPC$ */
        switch (DCE2_ScPolicy(ssd->sd.sconfig))
        {
            case DCE2_POLICY__WIN2000:
            case DCE2_POLICY__WINXP:
            case DCE2_POLICY__WINVISTA:
            case DCE2_POLICY__WIN2003:
            case DCE2_POLICY__WIN2008:
            case DCE2_POLICY__WIN7:
                if (bs_count > 3)
                {
                    /* Alert */
                }

                break;

            default:
                break;
        }
#endif

        /* Check for invalid shares first */
        if ((DCE2_ScSmbInvalidShares(ssd->sd.sconfig) != NULL) && (nb_len > 0))
        {
            DCE2_SmbInvalidShareCheck(ssd, smb_hdr, nb_ptr, nb_len);
        }

        /* Set appropriate array and length */
        if (SmbUnicode(smb_hdr))
        {
            ipc_chars = ipc_unicode;
            ipc_len = sizeof(ipc_unicode);
        }
        else
        {
            ipc_chars = ipc_ascii;
            ipc_len = sizeof(ipc_ascii);
        }

        /* Make sure we have enough data */
        if (nb_len < ipc_len)
            return DCE2_RET__ERROR;

        /* Test for IPC$ */
        for (i = 0; i < ipc_len; i++)
        {
            if ((nb_ptr[i] != ipc_chars[i]) && (nb_ptr[i] != tolower((int)ipc_chars[i])))
                break;
        }

        /* Not IPC$ */
        if (i != ipc_len)
        {
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Tid (%u) not an IPC tree\n",
                           SmbTid(smb_hdr)));

            /* This is just returned to indicate this is not an IPC tree */
            return DCE2_RET__ERROR;
        }
    }
    else
    {
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Inserting tid: %u\n", SmbTid(smb_hdr)));
        DCE2_SmbInsertTid(ssd, SmbTid(smb_hdr));
    }

    return DCE2_RET__SUCCESS;
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
static void DCE2_SmbTreeConnectEnqueue(DCE2_SmbSsnData *ssd, const SmbNtHdr *smb_hdr, const DCE2_Ret ipc_status)
{
    DCE2_Ret status;

    if (SmbType(smb_hdr) != SMB_TYPE__REQUEST)
        return;

    if (ssd->tc_queue == NULL)
        ssd->tc_queue = DCE2_CQueueNew(DCE2_TC__QUEUE_SIZE, NULL, DCE2_MEM_TYPE__SMB_TID);

    if (ssd->tc_queue != NULL)
    {
        if (ipc_status == DCE2_RET__SUCCESS)
            status = DCE2_CQueueEnqueue(ssd->tc_queue, (void *)(uintptr_t)DCE2_TC__IPC);
        else
            status = DCE2_CQueueEnqueue(ssd->tc_queue, (void *)(uintptr_t)DCE2_TC__NOT_IPC);

        if (status != DCE2_RET__SUCCESS)
        {
            /* No space left in queue - way too many tree connects at once */
            DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_EXCESSIVE_TREE_CONNECTS,
                    DCE2_TC__QUEUE_SIZE);
        }
    }
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
static void DCE2_SmbTreeConnectAndX(DCE2_SmbSsnData *ssd, const SmbNtHdr *smb_hdr,
                                    const uint8_t *nb_ptr, uint32_t nb_len)
{
    const int smb_com = SMB_COM_TREE_CON_ANDX;
    const int smb_type = SmbType(smb_hdr);
    const SmbAndXCommon *andx = (SmbAndXCommon *)nb_ptr;
    int com_size, bcc;

    if (smb_type == SMB_TYPE__RESPONSE)
    {
        SmbEmptyCom *ec = (SmbEmptyCom *)nb_ptr;

        if (DCE2_SmbCheckComSize(ssd, nb_len, sizeof(SmbEmptyCom), smb_com) != DCE2_RET__SUCCESS)
            return;

        /* Server didn't accept client request */
        if ((SmbEmptyComWct(ec) == 0) && (SmbEmptyComBcc(ec) == 0) && SmbError(smb_hdr))
            return;
    }

    if (DCE2_SmbCheckComSize(ssd, nb_len, sizeof(SmbAndXCommon), smb_com) != DCE2_RET__SUCCESS)
        return;

    com_size = DCE2_SmbGetComSize(ssd, smb_hdr, (SmbCommon *)andx, smb_com);
    if ((com_size < 0) ||
        (DCE2_SmbCheckComSize(ssd, nb_len, (uint16_t)com_size, smb_com) != DCE2_RET__SUCCESS))
        return;

    DCE2_MOVE(nb_ptr, nb_len, com_size);

    bcc = DCE2_SmbGetBcc(ssd, smb_hdr, (SmbCommon *)andx, (uint16_t)com_size, smb_com);
    if ((bcc < 0) ||
        (DCE2_SmbCheckBcc(ssd, nb_len, (uint16_t)bcc, smb_com) != DCE2_RET__SUCCESS))
        return;

    if (smb_type == SMB_TYPE__RESPONSE)
    {
        const uint8_t ipc_chars[] = {'I', 'P', 'C', '\0'};
        unsigned int i;

        if (nb_len < sizeof(ipc_chars))
            return;

        /* Look for IPC */
        for (i = 0; i < sizeof(ipc_chars); i++)
        {
            if ((nb_ptr[i] != ipc_chars[i]))
                break;
        }

        if (i != sizeof(ipc_chars))
        {
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Tid (%u) not an IPC tree\n",
                           SmbTid(smb_hdr)));
            return;
        }

        /* Insert tid into list */
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Inserting tid: %u\n", SmbTid(smb_hdr)));
        DCE2_SmbInsertTid(ssd, SmbTid(smb_hdr));

        DCE2_MOVE(nb_ptr, nb_len, bcc);
    }
    else
    {
        if (DCE2_ScSmbInvalidShares(ssd->sd.sconfig) != NULL)
        {
            uint16_t pass_len = SmbLm10_TreeConAndXReqPassLen((SmbLm10_TreeConnectAndXReq *)andx);
            const uint8_t *tmp_ptr = nb_ptr;
            uint32_t tmp_len = bcc;

            /* Move past password length */
            if (pass_len < bcc)
            {
                const uint8_t *bs = NULL;

                DCE2_MOVE(tmp_ptr, tmp_len, pass_len);

                while ((bs = memchr(tmp_ptr, '\\', tmp_len)) != NULL)
                {
                    DCE2_MOVE(tmp_ptr, tmp_len, (bs - tmp_ptr) + 1);
                }

                if (SmbUnicode(smb_hdr) && (tmp_len > 0))
                {
                    DCE2_MOVE(tmp_ptr, tmp_len, 1);
                }

                if (tmp_len > 0)
                    DCE2_SmbInvalidShareCheck(ssd, smb_hdr, tmp_ptr, tmp_len);
            }
        }

        DCE2_MOVE(nb_ptr, nb_len, bcc);
    }

    if (SmbAndXCom2(andx) != SMB_COM_NO_ANDX_COMMAND)
        DCE2_SmbChained(ssd, smb_hdr, andx, smb_com, nb_ptr, nb_len);
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
static INLINE void DCE2_SmbInvalidShareCheck(DCE2_SmbSsnData *ssd, const SmbNtHdr *smb_hdr,
                                             const uint8_t *nb_ptr, uint32_t nb_len)
{
    DCE2_List *share_list = DCE2_ScSmbInvalidShares(ssd->sd.sconfig);
    DCE2_SmbShare *smb_share;

    if (share_list == NULL)
        return;

    for (smb_share = (DCE2_SmbShare *)DCE2_ListFirst(share_list);
         smb_share != NULL;
         smb_share = (DCE2_SmbShare *)DCE2_ListNext(share_list))
    {
        unsigned int i;
        const char *share_str;
        unsigned int share_str_len;

        if (SmbUnicode(smb_hdr))
        {
            share_str = smb_share->unicode_str;
            share_str_len = smb_share->unicode_str_len;
        }
        else
        {
            share_str = smb_share->ascii_str;
            share_str_len = smb_share->ascii_str_len;
        }

        /* Make sure we have enough data */
        if (nb_len < share_str_len)
            continue;

        /* Test for share match */
        for (i = 0; i < share_str_len; i++)
        {
            /* All share strings should have been converted to upper case and 
             * should include null terminating bytes */
            if ((nb_ptr[i] != share_str[i]) && (nb_ptr[i] != tolower((int)share_str[i])))
                break;
        }

        if (i == share_str_len)
        {
            /* Should only match one share since no duplicate shares in list */
            DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_INVALID_SHARE, smb_share->ascii_str);
            break;
        }
    }
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
static void DCE2_SmbTreeDisconnect(DCE2_SmbSsnData *ssd, const SmbNtHdr *smb_hdr,
                                   const uint8_t *nb_ptr, uint32_t nb_len)
{
    const int smb_com = SMB_COM_TREE_DIS;
    const int smb_type = SmbType(smb_hdr);
    const SmbCommon *sc = (SmbCommon *)nb_ptr;
    int com_size, bcc;

    if (smb_type == SMB_TYPE__RESPONSE)
    {
        /* Didn't get a positive response */
        if (SmbError(smb_hdr))
            return;
    }

    if (DCE2_SmbCheckComSize(ssd, nb_len, sizeof(SmbCommon), smb_com) != DCE2_RET__SUCCESS)
        return;

    com_size = DCE2_SmbGetComSize(ssd, smb_hdr, sc, smb_com);
    if ((com_size < 0) ||
        (DCE2_SmbCheckComSize(ssd, nb_len, (uint16_t)com_size, smb_com) != DCE2_RET__SUCCESS))
        return;

    DCE2_MOVE(nb_ptr, nb_len, com_size);

    bcc = DCE2_SmbGetBcc(ssd, smb_hdr, sc, (uint16_t)com_size, smb_com);
    if ((bcc < 0) ||
        (DCE2_SmbCheckBcc(ssd, nb_len, (uint16_t)bcc, smb_com) != DCE2_RET__SUCCESS))
        return;

    if (smb_type == SMB_TYPE__RESPONSE)
    {
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Removing tid: %u\n", SmbTid(smb_hdr)));
        DCE2_SmbRemoveTid(ssd, SmbTid(smb_hdr));
    }

    DCE2_MOVE(nb_ptr, nb_len, bcc);
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
static void DCE2_SmbOpen(DCE2_SmbSsnData *ssd, const SmbNtHdr *smb_hdr,
                         const uint8_t *nb_ptr, uint32_t nb_len)
{
    const int smb_com = SMB_COM_OPEN;
    const int smb_type = SmbType(smb_hdr);
    const SmbCommon *sc = (SmbCommon *)nb_ptr;
    int com_size, bcc;

    if (smb_type == SMB_TYPE__RESPONSE)
    {
        if (SmbError(smb_hdr))
            return;
    }

    if (DCE2_SmbCheckComSize(ssd, nb_len, sizeof(SmbCommon), smb_com) != DCE2_RET__SUCCESS)
        return;

    com_size = DCE2_SmbGetComSize(ssd, smb_hdr, sc, smb_com);
    if ((com_size < 0) ||
        (DCE2_SmbCheckComSize(ssd, nb_len, (uint16_t)com_size, smb_com) != DCE2_RET__SUCCESS))
        return;

    DCE2_MOVE(nb_ptr, nb_len, com_size);

    bcc = DCE2_SmbGetBcc(ssd, smb_hdr, sc, (uint16_t)com_size, smb_com);
    if ((bcc < 0) ||
        (DCE2_SmbCheckBcc(ssd, nb_len, (uint16_t)bcc, smb_com) != DCE2_RET__SUCCESS))
        return;

    if (smb_type == SMB_TYPE__RESPONSE)
    {
        const uint16_t fid = SmbCore_OpenRespFid((SmbCore_OpenResp *)sc);
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Inserting fid: 0x%04x\n", fid));
        DCE2_SmbInsertFid(ssd, SmbUid(smb_hdr), SmbTid(smb_hdr), fid);
    }
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
static void DCE2_SmbOpenAndX(DCE2_SmbSsnData *ssd, const SmbNtHdr *smb_hdr,
                             const uint8_t *nb_ptr, uint32_t nb_len)
{
    const int smb_com = SMB_COM_OPEN_ANDX;
    const int smb_type = SmbType(smb_hdr);
    const SmbAndXCommon *andx = (SmbAndXCommon *)nb_ptr;
    int com_size, bcc;

    if (smb_type == SMB_TYPE__RESPONSE)
    {
        const SmbEmptyCom *ec = (SmbEmptyCom *)nb_ptr;

        if (DCE2_SmbCheckComSize(ssd, nb_len, sizeof(SmbEmptyCom), smb_com) != DCE2_RET__SUCCESS)
            return;

        /* Server didn't accept client request */
        if ((SmbEmptyComWct(ec) == 0) && (SmbEmptyComBcc(ec) == 0) && SmbError(smb_hdr))
            return;
    }
    else
    {
        if (!DCE2_QueueIsEmpty(ssd->ft_queue))
            DCE2_QueueEmpty(ssd->ft_queue);
    }

    if (DCE2_SmbCheckComSize(ssd, nb_len, sizeof(SmbAndXCommon), smb_com) != DCE2_RET__SUCCESS)
        return;

    com_size = DCE2_SmbGetComSize(ssd, smb_hdr, (SmbCommon *)andx, smb_com);
    if ((com_size < 0) ||
        (DCE2_SmbCheckComSize(ssd, nb_len, (uint16_t)com_size, smb_com) != DCE2_RET__SUCCESS))
        return;

    DCE2_MOVE(nb_ptr, nb_len, com_size);

    bcc = DCE2_SmbGetBcc(ssd, smb_hdr, (SmbCommon *)andx, (uint16_t)com_size, smb_com);
    if ((bcc < 0) ||
        (DCE2_SmbCheckBcc(ssd, nb_len, (uint16_t)bcc, smb_com) != DCE2_RET__SUCCESS))
        return;

    if (smb_type == SMB_TYPE__RESPONSE)
    {
        const uint16_t fid = SmbLm10_OpenAndXRespFid((SmbLm10_OpenAndXResp *)andx);
        uint16_t uid = (ssd->req_uid != DCE2_SENTINEL) ? (uint16_t)ssd->req_uid : SmbUid(smb_hdr);
        uint16_t tid = (ssd->req_tid != DCE2_SENTINEL) ? (uint16_t)ssd->req_tid : SmbTid(smb_hdr);

        ssd->last_open_fid = fid;

        switch (SmbAndXCom2(andx))
        {
            case SMB_COM_WRITE:
                if (!DCE2_QueueIsEmpty(ssd->ft_queue))
                {
                    DCE2_SmbFidTrackerNode *ft_node =
                        (DCE2_SmbFidTrackerNode *)DCE2_QueueDequeue(ssd->ft_queue);

                    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Dequeueing open => write queue\n", fid));

                    if (ft_node != NULL)
                    {
                        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Inserting fid node: 0x%04x\n", fid));
                        DCE2_SmbInsertFidNode(ssd, uid, tid, fid, ft_node);
                    }

                    break;
                }

                /* Fall through */

            default:
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Inserting fid: 0x%04x\n", fid));
                DCE2_SmbInsertFid(ssd, uid, tid, fid);
                break;
        }
    }

    DCE2_MOVE(nb_ptr, nb_len, bcc);

    if (SmbAndXCom2(andx) != SMB_COM_NO_ANDX_COMMAND)
        DCE2_SmbChained(ssd, smb_hdr, andx, smb_com, nb_ptr, nb_len);
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
static void DCE2_SmbNtCreateAndX(DCE2_SmbSsnData *ssd, const SmbNtHdr *smb_hdr,
                                 const uint8_t *nb_ptr, uint32_t nb_len)
{
    const int smb_com = SMB_COM_NT_CREATE_ANDX;
    const int smb_type = SmbType(smb_hdr);
    const SmbAndXCommon *andx = (SmbAndXCommon *)nb_ptr;
    int com_size, bcc;

    if (smb_type == SMB_TYPE__RESPONSE)
    {
        const SmbEmptyCom *ec = (SmbEmptyCom *)nb_ptr;

        if (DCE2_SmbCheckComSize(ssd, nb_len, sizeof(SmbEmptyCom), smb_com) != DCE2_RET__SUCCESS)
            return;

        /* Server didn't accept client request */
        if ((SmbEmptyComWct(ec) == 0) && (SmbEmptyComBcc(ec) == 0) && SmbError(smb_hdr))
            return;
    }

    if (DCE2_SmbCheckComSize(ssd, nb_len, sizeof(SmbAndXCommon), smb_com) != DCE2_RET__SUCCESS)
        return;

    com_size = DCE2_SmbGetComSize(ssd, smb_hdr, (SmbCommon *)andx, smb_com);
    if ((com_size < 0) ||
        (DCE2_SmbCheckComSize(ssd, nb_len, (uint16_t)com_size, smb_com) != DCE2_RET__SUCCESS))
        return;

    DCE2_MOVE(nb_ptr, nb_len, com_size);

    bcc = DCE2_SmbGetBcc(ssd, smb_hdr, (SmbCommon *)andx, (uint16_t)com_size, smb_com);
    if ((bcc < 0) ||
        (DCE2_SmbCheckBcc(ssd, nb_len, (uint16_t)bcc, smb_com) != DCE2_RET__SUCCESS))
        return;

    if (smb_type == SMB_TYPE__RESPONSE)
    {
        const uint16_t fid = SmbNt10_NtCreateAndXRespFid((SmbNt10_NtCreateAndXResp *)andx);
        uint16_t uid = (ssd->req_uid != DCE2_SENTINEL) ? (uint16_t)ssd->req_uid : SmbUid(smb_hdr);
        uint16_t tid = (ssd->req_tid != DCE2_SENTINEL) ? (uint16_t)ssd->req_tid : SmbTid(smb_hdr);

        ssd->last_open_fid = fid;

        switch (SmbAndXCom2(andx))
        {
            case SMB_COM_WRITE:
                if (!DCE2_QueueIsEmpty(ssd->ft_queue))
                {
                    DCE2_SmbFidTrackerNode *ft_node =
                        (DCE2_SmbFidTrackerNode *)DCE2_QueueDequeue(ssd->ft_queue);

                    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Dequeueing open => write queue\n", fid));

                    if (ft_node != NULL)
                    {
                        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Inserting fid node: 0x%04x\n", fid));
                        DCE2_SmbInsertFidNode(ssd, uid, tid, fid, ft_node);
                    }

                    break;
                }

                /* Fall through */

            default:
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Inserting fid: 0x%04x\n", fid));
                DCE2_SmbInsertFid(ssd, uid, tid, fid);
                break;
        }
    }

    DCE2_MOVE(nb_ptr, nb_len, bcc);

    if (SmbAndXCom2(andx) != SMB_COM_NO_ANDX_COMMAND)
        DCE2_SmbChained(ssd, smb_hdr, andx, smb_com, nb_ptr, nb_len);
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
static void DCE2_SmbClose(DCE2_SmbSsnData *ssd, const SmbNtHdr *smb_hdr,
                          const uint8_t *nb_ptr, uint32_t nb_len, int open_chain)
{
    const int smb_com = SMB_COM_CLOSE;
    const int smb_type = SmbType(smb_hdr);
    const SmbCommon *sc = (SmbCommon *)nb_ptr;
    int com_size, bcc;

    if (smb_type == SMB_TYPE__RESPONSE)
    {
        if (SmbError(smb_hdr))
            return;
    }

    if (DCE2_SmbCheckComSize(ssd, nb_len, sizeof(SmbCommon), smb_com) != DCE2_RET__SUCCESS)
        return;

    com_size = DCE2_SmbGetComSize(ssd, smb_hdr, sc, smb_com);
    if ((com_size < 0) ||
        (DCE2_SmbCheckComSize(ssd, nb_len, (uint16_t)com_size, smb_com) != DCE2_RET__SUCCESS))
        return;

    DCE2_MOVE(nb_ptr, nb_len, com_size);

    bcc = DCE2_SmbGetBcc(ssd, smb_hdr, sc, (uint16_t)com_size, smb_com);
    if ((bcc < 0) ||
        (DCE2_SmbCheckBcc(ssd, nb_len, (uint16_t)bcc, smb_com) != DCE2_RET__SUCCESS))
        return;

    /* Response does not contain fid */
    if (smb_type == SMB_TYPE__REQUEST)
    {
        /* Remove it if it wasn't chained in a chain with an open before it */
        if (!open_chain)
        {
            DCE2_SmbRemoveFid(ssd, SmbUid(smb_hdr), SmbTid(smb_hdr),
                              SmbCore_CloseReqFid((SmbCore_CloseReq *)sc));
        }
    }
    else
    {
        if (open_chain)
        {
            /* The previous OpenAndX or NtCreateAndX will have set last_open_fid
             * to a 16 bit FID */
            DCE2_SmbRemoveFid(ssd, SmbUid(smb_hdr), SmbTid(smb_hdr), (uint16_t)ssd->last_open_fid);
        }
    }
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
static void DCE2_SmbWrite(DCE2_SmbSsnData *ssd, const SmbNtHdr *smb_hdr,
                          const uint8_t *nb_ptr, uint32_t nb_len)
{
    const int smb_com = SMB_COM_WRITE;
    const int smb_type = SmbType(smb_hdr);
    const SmbCommon *sc = (SmbCommon *)nb_ptr;
    int com_size, bcc;

    if (smb_type == SMB_TYPE__RESPONSE)
    {
        if (SmbError(smb_hdr))
        {
            DCE2_QueueEmpty(ssd->ft_queue);
            return;
        }
    }

    if (DCE2_SmbCheckComSize(ssd, nb_len, sizeof(SmbCommon), smb_com) != DCE2_RET__SUCCESS)
        return;

    com_size = DCE2_SmbGetComSize(ssd, smb_hdr, sc, smb_com);
    if ((com_size < 0) ||
        (DCE2_SmbCheckComSize(ssd, nb_len, (uint16_t)com_size, smb_com) != DCE2_RET__SUCCESS))
        return;

    DCE2_MOVE(nb_ptr, nb_len, com_size);

    bcc = DCE2_SmbGetBcc(ssd, smb_hdr, sc, (uint16_t)com_size, smb_com);
    if (bcc < 0)
        return;
    if (DCE2_SmbCheckBcc(ssd, nb_len, (uint16_t)bcc, smb_com) != DCE2_RET__SUCCESS)
        bcc = (int)nb_len;  /* nb_len less than UINT16_MAX */

    if (smb_type == SMB_TYPE__REQUEST)
    {
        uint16_t dsize;
        uint16_t fid = SmbCore_WriteReqFid((SmbCore_WriteReq *)sc);

        /* In case we get a reassembled packet */
        ssd->fid = fid;

        /* Have at least 3 bytes */

        if (*nb_ptr != SMB_FMT__DATA_BLOCK)
        {
            DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_BAD_FORMAT, *nb_ptr);
            return;
        }

        DCE2_MOVE(nb_ptr, nb_len, 1);

        dsize = SmbNtohs((uint16_t *)nb_ptr);

        DCE2_MOVE(nb_ptr, nb_len, 2);

        if (DCE2_SmbCheckDsize(ssd, nb_len, dsize, (uint16_t)(bcc - 3), smb_com) != DCE2_RET__SUCCESS)
            return;

        if (dsize != (bcc - 3))
        {
            DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_INVALID_DSIZE, dsize, bcc);
        }

        if (!DCE2_QueueIsEmpty(ssd->ft_queue))
        {
            DCE2_SmbFidTrackerNode *ft_node = (DCE2_SmbFidTrackerNode *)DCE2_QueueLast(ssd->ft_queue);
            if (ft_node == NULL)
                return;

            /* Init the tracker */
            DCE2_CoInitTracker(&ft_node->co_tracker);
            DCE2_CoProcess(&ssd->sd, &ft_node->co_tracker, nb_ptr, dsize);
        }
        else
        {
            DCE2_WriteCoProcess(ssd, smb_hdr, fid, nb_ptr, dsize);
        }
    }
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
static void DCE2_SmbWriteBlockRaw(DCE2_SmbSsnData *ssd, const SmbNtHdr *smb_hdr,
                                  const uint8_t *nb_ptr, uint32_t nb_len)
{
    const int smb_com = SMB_COM_WRITE_BLOCK_RAW;
    const int smb_type = SmbType(smb_hdr);
    const SmbCommon *sc = (SmbCommon *)nb_ptr;
    int com_size, bcc;

    if (DCE2_SmbCheckComSize(ssd, nb_len, sizeof(SmbCommon), smb_com) != DCE2_RET__SUCCESS)
        return;

    if ((smb_type == SMB_TYPE__RESPONSE) && (SmbError(smb_hdr)))
    {
        ssd->br.smb_com = DCE2_SENTINEL;
        ssd->br.total_count = 0;
        return;
    }

    com_size = DCE2_SmbGetComSize(ssd, smb_hdr, sc, smb_com);
    if ((com_size < 0) ||
        (DCE2_SmbCheckComSize(ssd, nb_len, (uint16_t)com_size, smb_com) != DCE2_RET__SUCCESS))
        return;

    DCE2_MOVE(nb_ptr, nb_len, com_size);

    bcc = DCE2_SmbGetBcc(ssd, smb_hdr, sc, (uint16_t)com_size, smb_com);
    if (bcc < 0)
        return;
    if (DCE2_SmbCheckBcc(ssd, nb_len, (uint16_t)bcc, smb_com) != DCE2_RET__SUCCESS)
        bcc = (int)nb_len;  /* nb_len less than UINT16_MAX */

    if (smb_type == SMB_TYPE__REQUEST)
    {
        DCE2_SmbFidTrackerNode *ft_node;
        uint16_t fid = SmbLm10_WriteBlockRawReqFid((SmbLm10_WriteBlockRawReq *)sc);
        uint16_t total_count = SmbLm10_WriteBlockRawReqTotCount((SmbLm10_WriteBlockRawReq *)sc);
        const uint8_t *doff_ptr =
            (uint8_t *)smb_hdr + SmbLm10_WriteBlockRawReqDoff((SmbLm10_WriteBlockRawReq *)sc);
        uint16_t pad;
        uint16_t dsize;

        /* In case we get a reassembled packet */
        ssd->fid = fid;

        ft_node = DCE2_SmbFindFid(ssd, SmbUid(smb_hdr), SmbTid(smb_hdr), fid);
        if (ft_node == NULL)
            return;

        ssd->br.smb_com = smb_com;
        ssd->br.total_count = total_count;
        ssd->br.fid_node.fid = ft_node->fid_node.fid;
        ssd->br.fid_node.uid = SmbUid(smb_hdr);
        ssd->br.fid_node.tid = SmbTid(smb_hdr);

        dsize = SmbLm10_WriteBlockRawReqDsize((SmbLm10_WriteBlockRawReq *)sc);
        if (dsize != 0)
        {
            if (DCE2_SmbCheckOffset(ssd, doff_ptr, nb_ptr, nb_len, smb_com) != DCE2_RET__SUCCESS)
                return;

            pad = (uint16_t)(doff_ptr - nb_ptr);
            DCE2_MOVE(nb_ptr, nb_len, pad);

            if (DCE2_SmbCheckDsize(ssd, nb_len, dsize, (uint16_t)(bcc - pad), smb_com) != DCE2_RET__SUCCESS)
                return;

            if (DCE2_SmbCheckTotDcnt(ssd, dsize, total_count, smb_com) != DCE2_RET__SUCCESS)
                return;

            if (ssd->br.total_count - dsize < 0)
            {
                DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_TDCNT_LT_DSIZE,
                           (int)ssd->br.total_count, (int)dsize);

                ssd->br.total_count = 0;
            }
            else
            {
                ssd->br.total_count -= dsize;
            }

            DCE2_WriteCoProcess(ssd, smb_hdr, fid, nb_ptr, dsize);

            if (ssd->br.total_count == 0)
            {
                ssd->br.smb_com = DCE2_SENTINEL;
            }
        }
    }
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
static void DCE2_SmbWriteAndClose(DCE2_SmbSsnData *ssd, const SmbNtHdr *smb_hdr,
                                  const uint8_t *nb_ptr, uint32_t nb_len)
{
    const int smb_com = SMB_COM_WRITE_AND_CLOSE;
    const int smb_type = SmbType(smb_hdr);
    const SmbCommon *sc = (SmbCommon *)nb_ptr;
    int com_size, bcc;

    if (smb_type == SMB_TYPE__RESPONSE)
    {
        if (SmbError(smb_hdr))
            return;
    }

    if (DCE2_SmbCheckComSize(ssd, nb_len, sizeof(SmbCommon), smb_com) != DCE2_RET__SUCCESS)
        return;

    com_size = DCE2_SmbGetComSize(ssd, smb_hdr, sc, smb_com);
    if ((com_size < 0) ||
        (DCE2_SmbCheckComSize(ssd, nb_len, (uint16_t)com_size, smb_com) != DCE2_RET__SUCCESS))
        return;

    DCE2_MOVE(nb_ptr, nb_len, com_size);

    bcc = DCE2_SmbGetBcc(ssd, smb_hdr, sc, (uint16_t)com_size, smb_com);
    if (bcc < 0)
        return;
    if (DCE2_SmbCheckBcc(ssd, nb_len, (uint16_t)bcc, smb_com) != DCE2_RET__SUCCESS)
        bcc = (int)nb_len;  /* nb_len less than UINT16_MAX */

    if (smb_type == SMB_TYPE__REQUEST)
    {
        const uint16_t dsize = SmbLm10_WriteAndCloseReqCount((SmbLm10_WriteAndCloseReq6 *)sc);
        uint16_t fid = SmbLm10_WriteAndCloseReqFid((SmbLm10_WriteAndCloseReq6 *)sc);

        /* In case we get a reassembled packet */
        ssd->fid = fid;

        if (DCE2_SmbCheckDsize(ssd, nb_len, dsize, (uint16_t)bcc, smb_com) != DCE2_RET__SUCCESS)
            return;

        if ((dsize + 1) != bcc)
        {
            DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_INVALID_DSIZE, dsize, bcc);
        }

        /* Move past pad */
        DCE2_MOVE(nb_ptr, nb_len, 1);

        DCE2_WriteCoProcess(ssd, smb_hdr, fid, nb_ptr, dsize);
        DCE2_SmbRemoveFid(ssd, SmbUid(smb_hdr), SmbTid(smb_hdr), fid);
    }
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
// the s_* are introduced to avoid having to make lots of changes
// to pass these values from DCE2_SmbWriteAndX() to the callers of
// DCE2_HandleSegmentation().  Should be refactored on rewrite.
static uint16_t s_remain = 0;
static uint16_t s_offset = 0;

// if we return zero here, it means to append to the
// buffer when DCE2_BufferAddData() is called.
uint16_t DCE2_GetWriteOffset (uint16_t total, int append)
{
    // in header or segment with header
    if ( append )
        return 0;

    // calc offset from remaining bytes and pdu total
    if ( s_remain > 0 && total >= s_remain )
        return total - s_remain;

    // this is what was done originally
    return 0;
}
static void DCE2_SmbWriteAndX(DCE2_SmbSsnData *ssd, const SmbNtHdr *smb_hdr,
                              const uint8_t *nb_ptr, uint32_t nb_len)
{
    const int smb_com = SMB_COM_WRITE_ANDX;
    const int smb_type = SmbType(smb_hdr);
    const SmbAndXCommon *andx = (SmbAndXCommon *)nb_ptr;
    int com_size, bcc;

    if (smb_type == SMB_TYPE__RESPONSE)
    {
        const SmbEmptyCom *ec = (SmbEmptyCom *)nb_ptr;

        if (DCE2_SmbCheckComSize(ssd, nb_len, sizeof(SmbEmptyCom), smb_com) != DCE2_RET__SUCCESS)
            return;

        /* Server didn't accept client request */
        if ((SmbEmptyComWct(ec) == 0) && (SmbEmptyComBcc(ec) == 0) && SmbError(smb_hdr))
            return;
    }

    if (DCE2_SmbCheckComSize(ssd, nb_len, sizeof(SmbAndXCommon), smb_com) != DCE2_RET__SUCCESS)
        return;

    com_size = DCE2_SmbGetComSize(ssd, smb_hdr, (SmbCommon *)andx, smb_com);
    if ((com_size < 0) ||
        (DCE2_SmbCheckComSize(ssd, nb_len, (uint16_t)com_size, smb_com) != DCE2_RET__SUCCESS))
        return;

    DCE2_MOVE(nb_ptr, nb_len, com_size);

    bcc = DCE2_SmbGetBcc(ssd, smb_hdr, (SmbCommon *)andx, (uint16_t)com_size, smb_com);
    if (bcc < 0)
        return;
    if (DCE2_SmbCheckBcc(ssd, nb_len, (uint16_t)bcc, smb_com) != DCE2_RET__SUCCESS)
        bcc = (int)nb_len;  /* nb_len less than UINT16_MAX */

    if (smb_type == SMB_TYPE__REQUEST)
    {
        const uint8_t *doff_ptr = (uint8_t *)smb_hdr + SmbLm10_WriteAndXReqDoff((SmbLm10_WriteAndXReq *)andx);
        uint16_t fid = SmbLm10_WriteAndXReqFid((SmbLm10_WriteAndXReq *)andx);
        uint16_t pad;
        uint16_t dsize;

        /* In case we get a reassembled packet */
        ssd->fid = fid;

        if (DCE2_SmbCheckOffset(ssd, doff_ptr, nb_ptr, nb_len, smb_com) != DCE2_RET__SUCCESS)
            return;

        pad = (uint16_t)(doff_ptr - nb_ptr);
        DCE2_MOVE(nb_ptr, nb_len, pad);

        dsize = SmbLm10_WriteAndXReqDsize((SmbLm10_WriteAndXReq *)andx);
        if (DCE2_SmbCheckDsize(ssd, nb_len, dsize, (uint16_t)(bcc - pad), smb_com) != DCE2_RET__SUCCESS)
        {
            /* Seems sometimes the data offset is completely ignored.  Add pad back to the
             * length and if it equals dsize continue to inspect */
            if ((nb_len + pad) != dsize)
                return;

            nb_ptr -= pad;
            nb_len += pad;

            /* Do a dsize check again, this time without pad */
            if (DCE2_SmbCheckDsize(ssd, nb_len, dsize, (uint16_t)bcc, smb_com) != DCE2_RET__SUCCESS)
                return;
        }

        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Request fid: 0x%04x\n", fid));

        s_remain = SmbLm10_WriteAndXReqRemaining((SmbLm10_WriteAndXReq *)andx);
        s_offset = SmbLm10_WriteAndXReqOffset((SmbLm10_WriteAndXReq *)andx);

        DCE2_WriteCoProcess(ssd, smb_hdr, fid, nb_ptr, dsize);

        s_remain = 0;
        s_offset = 0;

        DCE2_MOVE(nb_ptr, nb_len, dsize);
    }
    else
    {
        DCE2_MOVE(nb_ptr, nb_len, bcc);
    }

    if (SmbAndXCom2(andx) != SMB_COM_NO_ANDX_COMMAND)
        DCE2_SmbChained(ssd, smb_hdr, andx, smb_com, nb_ptr, nb_len);
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
static INLINE DCE2_SmbPMNode * DCE2_SmbInsertPMNode(DCE2_SmbSsnData *ssd, const SmbNtHdr *smb_hdr,
                                                    DCE2_SmbFidNode *fid_node, const uint16_t total_dcnt)
{
    DCE2_SmbPMNode *pm_node = NULL;
    uint16_t pid = SmbPid(smb_hdr);
    uint16_t mid = SmbMid(smb_hdr);
    uint16_t uid = SmbUid(smb_hdr);
    uint16_t tid = SmbTid(smb_hdr);
    PROFILE_VARS;

    PREPROC_PROFILE_START(dce2_pstat_smb_trans);

    if ((ssd == NULL) || (fid_node == NULL))
    {
        PREPROC_PROFILE_END(dce2_pstat_smb_trans);
        return NULL;
    }

    if ((ssd->pm_node.pid == DCE2_SENTINEL) &&
        (ssd->pm_node.mid == DCE2_SENTINEL))
    {
        pm_node = &ssd->pm_node;

        pm_node->policy = DCE2_ScPolicy(ssd->sd.sconfig);
        pm_node->pid = (int)pid;
        pm_node->mid = (int)mid;
    }
    else
    {
        if (ssd->pms == NULL)
        {
            /* Don't use a key free function as the key will be the same pointer
             * as the data */
            ssd->pms = DCE2_ListNew(DCE2_LIST_TYPE__SPLAYED, DCE2_SmbPMCompare,
                                    DCE2_SmbPMDataFree, NULL, DCE2_LIST_FLAG__NO_DUPS,
                                    DCE2_MEM_TYPE__SMB_PM);

            if (ssd->pms == NULL)
            {
                PREPROC_PROFILE_END(dce2_pstat_smb_trans);
                return NULL;
            }
        }

        pm_node = (DCE2_SmbPMNode *)DCE2_Alloc(sizeof(DCE2_SmbPMNode), DCE2_MEM_TYPE__SMB_PM);
        if (pm_node == NULL)
        {
            PREPROC_PROFILE_END(dce2_pstat_smb_trans);
            return NULL;
        }

        /* Need to set these now for key */
        pm_node->policy = DCE2_ScPolicy(ssd->sd.sconfig);
        pm_node->pid = (int)pid;
        pm_node->mid = (int)mid;

        if (DCE2_ListInsert(ssd->pms, (void *)pm_node, (void *)pm_node) != DCE2_RET__SUCCESS)
        {
            DCE2_Free((void *)pm_node, sizeof(DCE2_SmbPMNode), DCE2_MEM_TYPE__SMB_PM);
            PREPROC_PROFILE_END(dce2_pstat_smb_trans);
            return NULL;
        }
    }

    if (pm_node != NULL)
    {
        pm_node->total_dcnt = total_dcnt;
        pm_node->fid_node.uid = (int)uid;
        pm_node->fid_node.tid = (int)tid;
        pm_node->fid_node.fid = fid_node->fid;

        DCE2_DEBUG_CODE(DCE2_DEBUG__SMB,
                        printf("Inserted pm_node - "
                               "pid: %u, mid: %u, uid: %u, tid: %u, fid: 0x%04x\n",
                               pm_node->pid, pm_node->mid, pm_node->fid_node.uid,
                               pm_node->fid_node.tid, pm_node->fid_node.fid););
    }

    DCE2_DEBUG_CODE(DCE2_DEBUG__SMB,
                    if (pm_node == NULL) printf("Failed to insert pm_node\n");); 

    PREPROC_PROFILE_END(dce2_pstat_smb_trans);

    return pm_node;
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
static INLINE DCE2_Ret DCE2_SmbAddDataToPMNode(DCE2_SmbSsnData *ssd, DCE2_SmbPMNode *pm_node,
                                               const uint8_t *data_ptr, uint16_t data_len, uint16_t data_disp)
{
    DCE2_Ret status;
    PROFILE_VARS;

    PREPROC_PROFILE_START(dce2_pstat_smb_trans);

    if ((ssd == NULL) || (pm_node == NULL))
    {
        PREPROC_PROFILE_END(dce2_pstat_smb_trans);
        return DCE2_RET__ERROR;
    }

    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Adding data to pm_node\n"));

    if (pm_node->buf == NULL)
    {
        /* Buf size should be the total data count we need */
        pm_node->buf = DCE2_BufferNew(pm_node->total_dcnt, 0, DCE2_MEM_TYPE__SMB_PM);
        if (pm_node->buf == NULL)
        {
            PREPROC_PROFILE_END(dce2_pstat_smb_trans);
            return DCE2_RET__ERROR;
        }
    }

    /* Got more data than total data count */
    if ((DCE2_BufferLength(pm_node->buf) + data_len) > pm_node->total_dcnt)
    {
        PREPROC_PROFILE_END(dce2_pstat_smb_trans);
        return DCE2_RET__ERROR;
    }

    /* XXX Maybe this is alertable since this is overwriting previously written data 
     * and servers don't seem to ever respond */
    if (data_disp < DCE2_BufferLength(pm_node->buf))
        DCE2_BufferSetLength(pm_node->buf, data_disp);

    status = DCE2_BufferAddData(pm_node->buf, data_ptr, data_len, 0,
                                DCE2_BUFFER_MIN_ADD_FLAG__IGNORE);

    if (status != DCE2_RET__SUCCESS)
    {
        PREPROC_PROFILE_END(dce2_pstat_smb_trans);
        return DCE2_RET__ERROR;
    }

    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Successfully added data to pm_node\n"));

    PREPROC_PROFILE_END(dce2_pstat_smb_trans);

    return DCE2_RET__SUCCESS;
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
static INLINE DCE2_SmbPMNode * DCE2_SmbFindPMNode(DCE2_SmbSsnData *ssd, const SmbNtHdr *smb_hdr)
{
    DCE2_Policy policy = DCE2_ScPolicy(ssd->sd.sconfig);
    DCE2_SmbPMNode *pm_node = NULL;
    uint16_t pid = SmbPid(smb_hdr);
    uint16_t mid = SmbMid(smb_hdr);
    int got_pm_node = 0;
    PROFILE_VARS;

    PREPROC_PROFILE_START(dce2_pstat_smb_trans);

    if (ssd == NULL)
    {
        PREPROC_PROFILE_END(dce2_pstat_smb_trans);
        return NULL;
    }

    DCE2_DEBUG_CODE(DCE2_DEBUG__SMB,
                    printf("Find pm_node - pid: %u, mid: %u ... ", pid, mid););

    switch (policy)
    {
        case DCE2_POLICY__SAMBA_3_0_20:
        case DCE2_POLICY__SAMBA_3_0_22:
            if ((ssd->pm_node.mid != DCE2_SENTINEL) ||
                (ssd->pm_node.pid != DCE2_SENTINEL))
            {
                pm_node = &ssd->pm_node;
                got_pm_node = 1;
            }

            break;

        case DCE2_POLICY__SAMBA:
        case DCE2_POLICY__SAMBA_3_0_37:
            if ((ssd->pm_node.mid != DCE2_SENTINEL) &&
                (ssd->pm_node.mid == (int)mid))
            {
                pm_node = &ssd->pm_node;
                got_pm_node = 1;
            }

            break;

        case DCE2_POLICY__WIN2000:
        case DCE2_POLICY__WIN2003:
        case DCE2_POLICY__WINXP:
        case DCE2_POLICY__WINVISTA:
        case DCE2_POLICY__WIN2008:
        case DCE2_POLICY__WIN7:
            if ((ssd->pm_node.pid != DCE2_SENTINEL) && (ssd->pm_node.pid == (int)pid) &&
                (ssd->pm_node.mid != DCE2_SENTINEL) && (ssd->pm_node.mid == (int)mid))
            {
                pm_node = &ssd->pm_node;
                got_pm_node = 1;
            }

            break;

        default:
            break;
    }

    if (!got_pm_node)
    {
        DCE2_SmbPMNode find_node;

        /* Just need policy, pid and mid */
        find_node.policy = policy;
        find_node.pid = (int)pid;
        find_node.mid = (int)mid;

        pm_node = DCE2_ListFind(ssd->pms, (void *)&find_node);
    }

    DCE2_DEBUG_CODE(DCE2_DEBUG__SMB,
                    if (pm_node != NULL) printf("Found - fid: 0x%04x\n", pm_node->fid_node.fid);
                    else printf("Not found\n"););

    PREPROC_PROFILE_END(dce2_pstat_smb_trans);

    return pm_node;
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
static INLINE void DCE2_SmbRemovePMNode(DCE2_SmbSsnData *ssd, const SmbNtHdr *smb_hdr)
{
    DCE2_Policy policy = DCE2_ScPolicy(ssd->sd.sconfig);
    uint16_t pid = SmbPid(smb_hdr);
    uint16_t mid = SmbMid(smb_hdr);
    int cleaned_pm_node = 0;
    PROFILE_VARS;

    PREPROC_PROFILE_START(dce2_pstat_smb_trans);

    if (ssd == NULL)
    {
        PREPROC_PROFILE_END(dce2_pstat_smb_trans);
        return;
    }

    switch (policy)
    {
        case DCE2_POLICY__SAMBA_3_0_20:
        case DCE2_POLICY__SAMBA_3_0_22:
            if (ssd->pm_node.mid != DCE2_SENTINEL)
            {
                DCE2_SmbCleanPMNode(&ssd->pm_node);
                cleaned_pm_node = 1;
            }

            break;

        case DCE2_POLICY__SAMBA:
        case DCE2_POLICY__SAMBA_3_0_37:
            if ((ssd->pm_node.mid != DCE2_SENTINEL) && (ssd->pm_node.mid == (int)mid))
            {
                DCE2_SmbCleanPMNode(&ssd->pm_node);
                cleaned_pm_node = 1;
            }

            break;

        case DCE2_POLICY__WIN2000:
        case DCE2_POLICY__WIN2003:
        case DCE2_POLICY__WINXP:
        case DCE2_POLICY__WINVISTA:
        case DCE2_POLICY__WIN2008:
        case DCE2_POLICY__WIN7:
            if ((ssd->pm_node.pid != DCE2_SENTINEL) && (ssd->pm_node.pid == (int)pid) &&
                (ssd->pm_node.mid != DCE2_SENTINEL) && (ssd->pm_node.mid == (int)mid))
            {
                DCE2_SmbCleanPMNode(&ssd->pm_node);
                cleaned_pm_node = 1;
            }

            break;

        default:
            break;
    }

    if (!cleaned_pm_node)
    {
        DCE2_SmbPMNode rm_node;

        /* Just need policy, pid and mid */
        rm_node.policy = policy;
        rm_node.pid = (int)pid;
        rm_node.mid = (int)mid;

        DCE2_ListRemove(ssd->pms, (void *)&rm_node);
    }

    PREPROC_PROFILE_END(dce2_pstat_smb_trans);
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
static void DCE2_SmbTrans(DCE2_SmbSsnData *ssd, const SmbNtHdr *smb_hdr,
                          const uint8_t *nb_ptr, uint32_t nb_len)
{
    const int smb_com = SMB_COM_TRANS;
    const int smb_type = SmbType(smb_hdr);
    const SmbCommon *sc = (SmbCommon *)nb_ptr;
    int com_size, bcc;
    DCE2_SmbFidTrackerNode *ft_node = NULL;

    if (DCE2_SmbCheckComSize(ssd, nb_len, sizeof(SmbCommon), smb_com) != DCE2_RET__SUCCESS)
        return;

    if (smb_type == SMB_TYPE__REQUEST)
    {
        /* Avoid decoding MAILSLOT and \PIPE\LANMAN requests */
        if (SmbWct(sc) != 16)
            return;
    }
    else
    {
        if (SmbError(smb_hdr))
            return;
    }

    com_size = DCE2_SmbGetComSize(ssd, smb_hdr, sc, smb_com);
    if ((com_size < 0) ||
        (DCE2_SmbCheckComSize(ssd, nb_len, (uint16_t)com_size, smb_com) != DCE2_RET__SUCCESS))
        return;

    if (smb_type == SMB_TYPE__RESPONSE)
    {
        if (com_size >= (int)sizeof(SmbLm10_TransactionResp))
        {
            /* Avoid decoding MAILSLOT and \PIPE\LANMAN responses */
            if (SmbLm10_TransRespParamCnt((SmbLm10_TransactionResp *)sc) > 0)
                return;
        }
        else if (com_size == sizeof(SmbLm10_TransactionInterimResp))
        {
            /* Interim response */
            return;
        }
    }

    DCE2_MOVE(nb_ptr, nb_len, com_size);

    bcc = DCE2_SmbGetBcc(ssd, smb_hdr, sc, (uint16_t)com_size, smb_com);
    if (bcc < 0)
        return;
    if (DCE2_SmbCheckBcc(ssd, nb_len, (uint16_t)bcc, smb_com) != DCE2_RET__SUCCESS)
        bcc = (int)nb_len;  /* nb_len less than UINT16_MAX */

    if (smb_type == SMB_TYPE__REQUEST)
    {
        const uint8_t *doff_ptr;
        uint16_t pad;
        uint16_t dcnt;
        uint16_t total_dcnt;
        uint16_t fid = SmbLm10_TransactNamedPipeReqFid((SmbLm10_TransactNamedPipeReq *)sc);
        DCE2_SmbPMNode *pm_node;

        switch (SmbLm10_TransactNamedPipeReqFunc((SmbLm10_TransactNamedPipeReq *)sc))
        {
            case SMB_TRANS_FUNC__TRANSACT_NM_PIPE:
                break;

            /* XXX See what these do */
            case SMB_TRANS_FUNC__SET_NM_P_HAND_STATE:
            case SMB_TRANS_FUNC__RAW_READ_NM_PIPE:
            case SMB_TRANS_FUNC__Q_NM_P_HAND_STATE:
            case SMB_TRANS_FUNC__Q_NM_PIPE_INFO:
            case SMB_TRANS_FUNC__PEEK_NM_PIPE:
            case SMB_TRANS_FUNC__RAW_WRITE_NM_PIPE:
            case SMB_TRANS_FUNC__WAIT_NM_PIPE:
            case SMB_TRANS_FUNC__CALL_NM_PIPE:
            default:
                return;
        }

        /* See if we've already got a queued fid node for this fid
         * If we already got a trans request and haven't yet got a response for
         * this fid, this request won't be accepted */
        if (DCE2_SmbFindPMNode(ssd, smb_hdr) != NULL)
            return;

        /* In case we get a reassembled packet */
        ssd->fid = fid;

        doff_ptr = (uint8_t *)smb_hdr + SmbLm10_TransactNamedPipeReqDoff((SmbLm10_TransactNamedPipeReq *)sc);
        if (DCE2_SmbCheckOffset(ssd, doff_ptr, nb_ptr, bcc, smb_com) != DCE2_RET__SUCCESS)
            return;

        pad = (uint16_t)(doff_ptr - nb_ptr);
        DCE2_MOVE(nb_ptr, nb_len, pad);

        dcnt = SmbLm10_TransactNamedPipeReqDcnt((SmbLm10_TransactNamedPipeReq *)sc);
        if (DCE2_SmbCheckDsize(ssd, nb_len, dcnt, (uint16_t)(bcc - pad), smb_com) != DCE2_RET__SUCCESS)
            return;

        total_dcnt = SmbLm10_TransactNamedPipeReqTotalDcnt((SmbLm10_TransactNamedPipeReq *)sc);
        if (DCE2_SmbCheckTotDcnt(ssd, dcnt, total_dcnt, smb_com) != DCE2_RET__SUCCESS)
            return;

        ft_node = DCE2_SmbFindFid(ssd, SmbUid(smb_hdr), SmbTid(smb_hdr), fid);
        if (ft_node == NULL)
            return;

        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Request fid: 0x%04x\n", ft_node->fid_node.fid));

        pm_node = DCE2_SmbInsertPMNode(ssd, smb_hdr, &ft_node->fid_node, total_dcnt);
        if (dcnt < total_dcnt)
        {
            if (DCE2_SmbAddDataToPMNode(ssd, pm_node, nb_ptr, dcnt, 0) != DCE2_RET__SUCCESS)
            {
                if (dcnt != 0)
                    DCE2_CoProcess(&ssd->sd, &ft_node->co_tracker, nb_ptr, dcnt);

                if (!ft_node->used)
                    ft_node->used = 1;
            }
        }
        else
        {
            if (dcnt != 0)
                DCE2_CoProcess(&ssd->sd, &ft_node->co_tracker, nb_ptr, dcnt);

            if (!ft_node->used)
                ft_node->used = 1;
        }
    }
    else
    {
        const uint8_t *doff_ptr;
        uint16_t pad;
        uint16_t dcnt;
        uint16_t total_dcnt;
        uint16_t ddisp;
        DCE2_SmbPMNode *pm_node;

        pm_node = DCE2_SmbFindPMNode(ssd, smb_hdr);
        if (pm_node == NULL)
            return;

        ft_node = DCE2_SmbFindFid(ssd, (uint16_t)pm_node->fid_node.uid,
                                  (uint16_t)pm_node->fid_node.tid, (uint16_t)pm_node->fid_node.fid);
        if (ft_node == NULL)
            return;

        doff_ptr = (uint8_t *)smb_hdr + SmbLm10_TransactNamedPipeRespDoff((SmbLm10_TransactNamedPipeResp *)sc);
        if (DCE2_SmbCheckOffset(ssd, doff_ptr, nb_ptr, nb_len, smb_com) != DCE2_RET__SUCCESS)
            return;

        pad = (uint16_t)(doff_ptr - nb_ptr);
        DCE2_MOVE(nb_ptr, nb_len, pad);

        dcnt = SmbLm10_TransactNamedPipeRespDcnt((SmbLm10_TransactNamedPipeResp *)sc);
        if (DCE2_SmbCheckDsize(ssd, nb_len, dcnt, (uint16_t)(bcc - pad), smb_com) != DCE2_RET__SUCCESS)
            return;

        total_dcnt = SmbLm10_TransactNamedPipeRespTotalDcnt((SmbLm10_TransactNamedPipeResp *)sc);
        if (DCE2_SmbCheckTotDcnt(ssd, dcnt, total_dcnt, smb_com) != DCE2_RET__SUCCESS)
            return;

        ddisp = SmbLm10_TransactNamedPipeRespTotalDdisp((SmbLm10_TransactNamedPipeResp *)sc);
        if ((ddisp + dcnt) > total_dcnt)
        {
            DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_DSENT_GT_TDCNT,
                    ddisp + dcnt, total_dcnt);

            return;
        }

        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Response fid: 0x%04x\n", pm_node->fid_node.fid));

        if (dcnt != 0)
            DCE2_CoProcess(&ssd->sd, &ft_node->co_tracker, nb_ptr, dcnt);

        if ((ddisp + dcnt) == total_dcnt)
            DCE2_SmbRemovePMNode(ssd, smb_hdr);
    }
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
static void DCE2_SmbTransSec(DCE2_SmbSsnData *ssd, const SmbNtHdr *smb_hdr,
                             const uint8_t *nb_ptr, uint32_t nb_len)
{
    const int smb_com = SMB_COM_TRANS_SEC;
    const int smb_type = SmbType(smb_hdr);
    const SmbCommon *sc = (SmbCommon *)nb_ptr;
    int com_size, bcc;
    const uint8_t *doff_ptr;
    int pad;
    uint16_t dcnt;
    uint16_t total_dcnt;
    uint16_t ddisp;
    DCE2_SmbPMNode *pm_node;
    DCE2_SmbFidTrackerNode *ft_node;

    if (smb_type == SMB_TYPE__RESPONSE)
    {
        /* Only get a response of this type if error */
        DCE2_SmbRemovePMNode(ssd, smb_hdr);
        return;
    }

    pm_node = DCE2_SmbFindPMNode(ssd, smb_hdr);
    if (pm_node == NULL)
        return;

    ft_node = DCE2_SmbFindFid(ssd, (uint16_t)pm_node->fid_node.uid,
                              (uint16_t)pm_node->fid_node.tid, (uint16_t)pm_node->fid_node.fid);
    if (ft_node == NULL)
    {
        DCE2_SmbRemovePMNode(ssd, smb_hdr);
        return;
    }

    /* In case we get a reassembled packet */
    ssd->fid = (uint16_t)pm_node->fid_node.fid;

    if (DCE2_SmbCheckComSize(ssd, nb_len, sizeof(SmbCommon), smb_com) != DCE2_RET__SUCCESS)
        return;

    com_size = DCE2_SmbGetComSize(ssd, smb_hdr, sc, smb_com);
    if ((com_size < 0) ||
        (DCE2_SmbCheckComSize(ssd, nb_len, (uint16_t)com_size, smb_com) != DCE2_RET__SUCCESS))
        return;

    DCE2_MOVE(nb_ptr, nb_len, com_size);

    bcc = DCE2_SmbGetBcc(ssd, smb_hdr, sc, (uint16_t)com_size, smb_com);
    if (bcc < 0)
        return;
    if (DCE2_SmbCheckBcc(ssd, nb_len, (uint16_t)bcc, smb_com) != DCE2_RET__SUCCESS)
        bcc = (int)nb_len;  /* nb_len less than UINT16_MAX */

    doff_ptr = (uint8_t *)smb_hdr + SmbLm10_TransactSecReqDoff((SmbLm10_TransactionSecondaryReq *)sc);
    if (DCE2_SmbCheckOffset(ssd, doff_ptr, nb_ptr, nb_len, smb_com) != DCE2_RET__SUCCESS)
        return;

    pad = (doff_ptr - nb_ptr);
    DCE2_MOVE(nb_ptr, nb_len, pad);

    dcnt = SmbLm10_TransactSecReqDcnt((SmbLm10_TransactionSecondaryReq *)sc);
    if (DCE2_SmbCheckDsize(ssd, nb_len, dcnt, (uint16_t)(bcc - pad), smb_com) != DCE2_RET__SUCCESS)
        return;

    total_dcnt = SmbLm10_TransactSecReqTotalDcnt((SmbLm10_TransactionSecondaryReq *)sc);
    if (DCE2_SmbCheckTotDcnt(ssd, dcnt, total_dcnt, smb_com) != DCE2_RET__SUCCESS)
        return;

    /* If total data count is not the same as was specified in initial Transact */
    if (pm_node->total_dcnt != total_dcnt)
    {
        /* XXX This really isn't the proper alert.  Create a new alert for this.
         * Total data count mismatch or something */
        DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_TDCNT_LT_DSIZE,
                (int)pm_node->total_dcnt, (int)total_dcnt);
        return;
    }

    ddisp = SmbLm10_TransactSecReqTotalDdisp((SmbLm10_TransactionSecondaryReq *)sc);
    if ((ddisp + dcnt) > total_dcnt)
    {
        DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_TDCNT_LT_DSIZE,
                (int)total_dcnt, (int)(ddisp + dcnt));
        return;
    }

    if ((DCE2_SmbAddDataToPMNode(ssd, pm_node, nb_ptr, dcnt, ddisp) != DCE2_RET__SUCCESS) ||
        ((ddisp + dcnt) == total_dcnt))
    {
        const uint8_t *data_ptr;
        uint16_t data_len;
        SFSnortPacket *rpkt = DCE2_GetRpkt(ssd->sd.wire_pkt, DCE2_RPKT_TYPE__SMB_TRANS,
                                           DCE2_BufferData(pm_node->buf), DCE2_BufferLength(pm_node->buf));

        if (rpkt == NULL)
        {
            DCE2_Log(DCE2_LOG_TYPE__ERROR,
                     "%s(%d) Failed to create reassembly packet.",
                     __FILE__, __LINE__);

            return;
        }

        if (DCE2_PushPkt(rpkt) != DCE2_RET__SUCCESS)
        {
            DCE2_Log(DCE2_LOG_TYPE__ERROR,
                     "%s(%d) Failed to push packet onto packet stack.",
                     __FILE__, __LINE__);
            return;
        }

        DCE2_SmbSetRdata(ssd, (uint8_t *)rpkt->payload,
                         (uint16_t)(rpkt->payload_size - DCE2_MOCK_HDR_LEN__SMB_CLI));

        data_ptr = rpkt->payload + DCE2_MOCK_HDR_LEN__SMB_CLI;
        data_len = rpkt->payload_size - DCE2_MOCK_HDR_LEN__SMB_CLI;

        DCE2_CoProcess(&ssd->sd, &ft_node->co_tracker, data_ptr, data_len);
        DCE2_PopPkt();
    }
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
static void DCE2_SmbReadBlockRaw(DCE2_SmbSsnData *ssd, const SmbNtHdr *smb_hdr,
                                 const uint8_t *nb_ptr, uint32_t nb_len)
{
    const int smb_com = SMB_COM_READ_BLOCK_RAW;
    const int smb_type = SmbType(smb_hdr);
    const SmbCommon *sc = (SmbCommon *)nb_ptr;
    int com_size, bcc;
    DCE2_SmbFidTrackerNode *ft_node = NULL;
    uint16_t uid = SmbUid(smb_hdr);
    uint16_t tid = SmbTid(smb_hdr);

    if (smb_type == SMB_TYPE__RESPONSE)
    {
        /* The server response is the raw data.  Supposedly if an error occurs, the
         * server will send a 0 byte read.  Guessing just the NetBIOS header with
         * zero byte length.  Client upon getting the zero read is supposed to issue
         * another read using ReadAndX or Read to get the error */
        return;
    }

    if (DCE2_SmbCheckComSize(ssd, nb_len, sizeof(SmbCommon), smb_com) != DCE2_RET__SUCCESS)
        return;

    com_size = DCE2_SmbGetComSize(ssd, smb_hdr, sc, smb_com);
    if ((com_size < 0) ||
        (DCE2_SmbCheckComSize(ssd, nb_len, (uint16_t)com_size, smb_com) != DCE2_RET__SUCCESS))
        return;

    DCE2_MOVE(nb_ptr, nb_len, com_size);

    bcc = DCE2_SmbGetBcc(ssd, smb_hdr, sc, (uint16_t)com_size, smb_com);
    if ((bcc < 0) ||
        (DCE2_SmbCheckBcc(ssd, nb_len, (uint16_t)bcc, smb_com) != DCE2_RET__SUCCESS))
        return;

    ft_node = DCE2_SmbFindFid(ssd, uid, tid,
                              SmbLm10_ReadBlockRawReqFid((SmbLm10_ReadBlockRawReq *)sc));

    if (ft_node == NULL)
        return;

    ssd->br.smb_com = smb_com;
    ssd->br.total_count = 0;
    ssd->br.fid_node.uid = uid;
    ssd->br.fid_node.tid = tid;
    ssd->br.fid_node.fid = ft_node->fid_node.fid;
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
static INLINE void DCE2_SmbSetReadFidNode(DCE2_SmbSsnData *ssd, uint16_t uid,
                                          uint16_t tid, uint16_t fid, int smb_com)
{
    if (ssd == NULL)
        return;

    if ((ssd->read_fid_queue == NULL) && (ssd->read_fid_node.fid == DCE2_SENTINEL))
    {
        ssd->read_fid_node.uid = uid;
        ssd->read_fid_node.tid = tid;
        ssd->read_fid_node.fid = fid;
    }
    else
    {
        DCE2_SmbFidNode *fid_node;

        if (ssd->read_fid_queue == NULL)
        {
            ssd->read_fid_queue = DCE2_CQueueNew(DCE2_READ__QUEUE_SIZE,
                                                 DCE2_SmbFidDataFree, DCE2_MEM_TYPE__SMB_FID);
            if (ssd->read_fid_queue == NULL)
                return;
        }

        fid_node = (DCE2_SmbFidNode *)DCE2_Alloc(sizeof(DCE2_SmbFidNode), DCE2_MEM_TYPE__SMB_FID);
        if (fid_node == NULL)
            return;

        if (DCE2_CQueueEnqueue(ssd->read_fid_queue, (void *)fid_node) != DCE2_RET__SUCCESS)
        {
            /* No space left in queue - way too many reads at once.  Not a memory
             * issue because no memory is alloced on insertion */
            DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_EXCESSIVE_READS,
                    DCE2_READ__QUEUE_SIZE);

            DCE2_Free((void *)fid_node, sizeof(DCE2_SmbFidNode), DCE2_MEM_TYPE__SMB_FID);

            return;
        }

        fid_node->uid = uid;
        fid_node->tid = tid;
        fid_node->fid = fid;
    }
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
static INLINE DCE2_SmbFidTrackerNode * DCE2_SmbGetReadFidNode(DCE2_SmbSsnData *ssd)
{
    DCE2_SmbFidNode *fid_node = NULL;
    uint16_t uid, tid, fid;

    if (ssd == NULL)
        return NULL;

    if (ssd->read_fid_node.fid != DCE2_SENTINEL)
    {
        uid = (uint16_t)ssd->read_fid_node.uid;
        tid = (uint16_t)ssd->read_fid_node.tid;
        fid = (uint16_t)ssd->read_fid_node.fid;

        ssd->read_fid_node.fid = DCE2_SENTINEL;
    }
    else if (!DCE2_CQueueIsEmpty(ssd->read_fid_queue))
    {
        fid_node = (DCE2_SmbFidNode *)DCE2_CQueueDequeue(ssd->read_fid_queue);
        if (fid_node == NULL)
            return NULL;

        uid = (uint16_t)fid_node->uid;
        tid = (uint16_t)fid_node->tid;
        fid = (uint16_t)fid_node->fid;

        DCE2_Free((void *)fid_node, sizeof(DCE2_SmbFidNode), DCE2_MEM_TYPE__SMB_FID);
    }
    else
    {
        return NULL;
    }

    return DCE2_SmbFindFid(ssd, uid, tid, fid);
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
static void DCE2_SmbReadAndX(DCE2_SmbSsnData *ssd, const SmbNtHdr *smb_hdr,
                             const uint8_t *nb_ptr, uint32_t nb_len)
{
    const int smb_com = SMB_COM_READ_ANDX;
    const int smb_type = SmbType(smb_hdr);
    const SmbAndXCommon *andx = (SmbAndXCommon *)nb_ptr;
    int com_size, bcc;
    uint16_t uid = SmbUid(smb_hdr);
    uint16_t tid = SmbTid(smb_hdr);

    if (smb_type == SMB_TYPE__RESPONSE)
    {
        const SmbEmptyCom *ec = (SmbEmptyCom *)nb_ptr;

        if (DCE2_SmbCheckComSize(ssd, nb_len, sizeof(SmbEmptyCom), smb_com) != DCE2_RET__SUCCESS)
            return;

        /* Server didn't accept client request */
        if ((SmbEmptyComWct(ec) == 0) && (SmbEmptyComBcc(ec) == 0) && SmbError(smb_hdr))
            return;
    }

    if (DCE2_SmbCheckComSize(ssd, nb_len, sizeof(SmbAndXCommon), smb_com) != DCE2_RET__SUCCESS)
        return;

    com_size = DCE2_SmbGetComSize(ssd, smb_hdr, (SmbCommon *)andx, smb_com);
    if ((com_size < 0) ||
        (DCE2_SmbCheckComSize(ssd, nb_len, (uint16_t)com_size, smb_com) != DCE2_RET__SUCCESS))
        return;

    DCE2_MOVE(nb_ptr, nb_len, com_size);

    bcc = DCE2_SmbGetBcc(ssd, smb_hdr, (SmbCommon *)andx, (uint16_t)com_size, smb_com);
    if (bcc < 0)
        return;
    if (DCE2_SmbCheckBcc(ssd, nb_len, (uint16_t)bcc, smb_com) != DCE2_RET__SUCCESS)
        bcc = (int)nb_len;  /* nb_len less than UINT16_MAX */

    if (smb_type == SMB_TYPE__REQUEST)
    {
        DCE2_SmbFidTrackerNode *ft_node =
            DCE2_SmbFindFid(ssd, uid, tid, SmbLm10_ReadAndXReqFid((SmbLm10_ReadAndXReq *)andx));

        if (ft_node != NULL)
        {
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Request fid: 0x%04x\n", ft_node->fid_node.fid));
            DCE2_SmbSetReadFidNode(ssd, uid, tid, (uint16_t)ft_node->fid_node.fid, smb_com);
        }

        DCE2_MOVE(nb_ptr, nb_len, bcc);
    }
    else
    {
        const uint8_t *doff_ptr = (uint8_t *)smb_hdr + SmbLm10_ReadAndXRespDoff((SmbLm10_ReadAndXResp *)andx);
        uint16_t pad;
        uint16_t dsize;
        DCE2_SmbFidTrackerNode *ft_node = DCE2_SmbGetReadFidNode(ssd);

        if (DCE2_SmbCheckOffset(ssd, doff_ptr, nb_ptr, bcc, smb_com) != DCE2_RET__SUCCESS)
            return;

        pad = (uint16_t)(doff_ptr - nb_ptr);
        DCE2_MOVE(nb_ptr, nb_len, pad);

        dsize = SmbLm10_ReadAndXRespDsize((SmbLm10_ReadAndXResp *)andx);
        if (DCE2_SmbCheckDsize(ssd, nb_len, dsize, (uint16_t)(bcc - pad), smb_com) != DCE2_RET__SUCCESS)
            return;

        if ((dsize != 0) && (ft_node != NULL))
        {
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Response fid: 0x%04x\n", ft_node->fid_node.fid));
            DCE2_CoProcess(&ssd->sd, &ft_node->co_tracker, nb_ptr, dsize);
        }

        DCE2_MOVE(nb_ptr, nb_len, dsize);
    }

    if (SmbAndXCom2(andx) != SMB_COM_NO_ANDX_COMMAND)
        DCE2_SmbChained(ssd, smb_hdr, andx, smb_com, nb_ptr, nb_len);
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
static void DCE2_SmbRead(DCE2_SmbSsnData *ssd, const SmbNtHdr *smb_hdr,
                         const uint8_t *nb_ptr, uint32_t nb_len)
{
    const int smb_com = SMB_COM_READ;
    const int smb_type = SmbType(smb_hdr);
    const SmbCommon *sc = (SmbCommon *)nb_ptr;
    int com_size, bcc;
    uint16_t uid = SmbUid(smb_hdr);
    uint16_t tid = SmbTid(smb_hdr);

    if (smb_type == SMB_TYPE__RESPONSE)
    {
        if (SmbError(smb_hdr))
            return;
    }

    if (DCE2_SmbCheckComSize(ssd, nb_len, sizeof(SmbCommon), smb_com) != DCE2_RET__SUCCESS)
        return;

    com_size = DCE2_SmbGetComSize(ssd, smb_hdr, sc, smb_com);
    if ((com_size < 0) ||
        (DCE2_SmbCheckComSize(ssd, nb_len, (uint16_t)com_size, smb_com) != DCE2_RET__SUCCESS))
        return;

    DCE2_MOVE(nb_ptr, nb_len, com_size);

    bcc = DCE2_SmbGetBcc(ssd, smb_hdr, sc, (uint16_t)com_size, smb_com);
    if ((bcc < 0) ||
        (DCE2_SmbCheckBcc(ssd, nb_len, (uint16_t)bcc, smb_com) != DCE2_RET__SUCCESS))
        return;

    if (smb_type == SMB_TYPE__REQUEST)
    {
        DCE2_SmbFidTrackerNode *ft_node =
            DCE2_SmbFindFid(ssd, SmbUid(smb_hdr), SmbTid(smb_hdr), SmbCore_ReadReqFid((SmbCore_ReadReq *)sc));

        if (ft_node != NULL)
            DCE2_SmbSetReadFidNode(ssd, uid, tid, (uint16_t)ft_node->fid_node.fid, smb_com);

        //DCE2_MOVE(nb_ptr, nb_len, bcc);
    }
    else
    {
        uint16_t dsize;
        DCE2_SmbFidTrackerNode *ft_node = DCE2_SmbGetReadFidNode(ssd);

        /* Have at least 3 bytes */

        if (*nb_ptr != SMB_FMT__DATA_BLOCK)
        {
            DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_BAD_FORMAT, *nb_ptr);
            return;
        }

        DCE2_MOVE(nb_ptr, nb_len, 1);

        dsize = SmbNtohs((uint16_t *)nb_ptr);

        DCE2_MOVE(nb_ptr, nb_len, 2);

        if (DCE2_SmbCheckDsize(ssd, nb_len, dsize, (uint16_t)(bcc - 3), smb_com) != DCE2_RET__SUCCESS)
            return;

        if (dsize != (bcc - 3))
        {
            DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_INVALID_DSIZE, dsize, bcc);
        }

        if ((dsize != 0) && (ft_node != NULL))
            DCE2_CoProcess(&ssd->sd, &ft_node->co_tracker, nb_ptr, dsize);

        //DCE2_MOVE(nb_ptr, nb_len, dsize);
    }
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
static void DCE2_SmbRename(DCE2_SmbSsnData *ssd, const SmbNtHdr *smb_hdr,
                           const uint8_t *nb_ptr, uint32_t nb_len)
{
    const int smb_com = SMB_COM_RENAME;
    const int smb_type = SmbType(smb_hdr);
    const SmbCommon *sc = (SmbCommon *)nb_ptr;
    int com_size, bcc;

    if (smb_type == SMB_TYPE__RESPONSE)
    {
        /* Didn't get a positive response */
        if (SmbError(smb_hdr))
            return;
    }

    if (DCE2_SmbCheckComSize(ssd, nb_len, sizeof(SmbCommon), smb_com) != DCE2_RET__SUCCESS)
        return;

    com_size = DCE2_SmbGetComSize(ssd, smb_hdr, sc, smb_com);
    if ((com_size < 0) ||
        (DCE2_SmbCheckComSize(ssd, nb_len, (uint16_t)com_size, smb_com) != DCE2_RET__SUCCESS))
        return;

    DCE2_MOVE(nb_ptr, nb_len, com_size);

    bcc = DCE2_SmbGetBcc(ssd, smb_hdr, sc, (uint16_t)com_size, smb_com);
    if ((bcc < 0) ||
        (DCE2_SmbCheckBcc(ssd, nb_len, (uint16_t)bcc, smb_com) != DCE2_RET__SUCCESS))
        return;

    if (smb_type == SMB_TYPE__REQUEST)
    {
        unsigned int i;

        /* Have at least 4 bytes */

        if (*nb_ptr != SMB_FMT__ASCII)
        {
            DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_BAD_FORMAT, *nb_ptr);
            return;
        }

        DCE2_MOVE(nb_ptr, nb_len, 1);

        if (SmbUnicode(smb_hdr))
        {
            for (i = 0; i < (nb_len - 1); i += 2)
            {
                if (*((uint16_t *)(nb_ptr + i)) == 0)
                {
                    /* need to go past null bytes and break */
                    i += 2;
                    break;
                }
            }
        }
        else
        {
            for (i = 0; i < nb_len; i++)
            {
                if (nb_ptr[i] == 0)
                {
                    /* need to go past null byte and break */
                    i++;
                    break;
                }
            }
        }

        /* i <= nb_len */
        DCE2_MOVE(nb_ptr, nb_len, i);

        if ((nb_len > 0) && (*nb_ptr != SMB_FMT__ASCII))
        {
            DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_BAD_FORMAT, *nb_ptr);
            return;
        }
    }
}

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Note that the com argument should not be gotten from the SMB
 * header since the command itself might have been chained.
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
static void DCE2_SmbChained(DCE2_SmbSsnData *ssd, const SmbNtHdr *smb_hdr, const SmbAndXCommon *andx,
                            const int smb_com, const uint8_t *nb_ptr, uint32_t nb_len)
{
    const uint8_t *off2_ptr = (uint8_t *)smb_hdr + SmbAndXOff2(andx);
    const uint8_t smb_com2 = SmbAndXCom2(andx);
    int process = 1;
    int ssx_chained = 0;
    static int first = 1;
    static int num_sess = 0;
    static int num_tree = 0;
    static int open_chain = 0;
    static int num_chained = 0;
#define DCE2_SMB_CHAINED__RESET_STATICS { first = 1; num_chained = 0; num_sess = 0; \
                                          num_tree = 0; open_chain = 0; }

    DCE2_SmbIncChainedStat(smb_hdr, smb_com, andx);

    num_chained++;
    if (DCE2_ScSmbMaxChain(ssd->sd.sconfig) &&
        (num_chained >= DCE2_ScSmbMaxChain(ssd->sd.sconfig)))
    {
        DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_EXCESSIVE_CHAINING, DCE2_ScSmbMaxChain(ssd->sd.sconfig));
    }

    if (first && (smb_com == SMB_COM_SESS_SETUP_ANDX))
        num_sess++;

    if (smb_com2 == SMB_COM_SESS_SETUP_ANDX)
        num_sess++;

    if (num_sess > 1)
    {
        /* There is only one place to return a uid. */
        DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_MULT_CHAIN_SS);
        process = 0;
    }

    if ((smb_com2 == SMB_COM_LOGOFF_ANDX) && (num_sess > 0))
    {
        /* This essentially deletes the uid created by the login
         * and doesn't make any sense */
        DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_CHAIN_SS_LOGOFF);
    }

    if (first && (smb_com == SMB_COM_TREE_CON_ANDX))
        num_tree++;

    if ((smb_com2 == SMB_COM_TREE_CON_ANDX) || (smb_com2 == SMB_COM_TREE_CON))
        num_tree++;

    if (num_tree > 1)
    {
        /* There is only one place to return a tid. */
        DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_MULT_CHAIN_TC);
        process = 0;
    }

    if ((smb_com2 == SMB_COM_TREE_DIS) && (num_tree > 0))
    {
        /* This essentially deletes the tid created by the tree connect
         * and doesn't make any sense */
        DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_CHAIN_TC_TDIS);
    }

    switch (smb_com)
    {
        case SMB_COM_OPEN_ANDX:
        case SMB_COM_NT_CREATE_ANDX:
            open_chain = 1;
            break;

        default:
            break;
    }

    if ((smb_com2 == SMB_COM_CLOSE) && open_chain)
    {
        /* This essentially deletes the fid created by the open command
         * and doesn't make any sense */
        DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_CHAIN_OPEN_CLOSE);
    }

    if (first)
        first = 0;

    if (DCE2_SmbCheckOffset(ssd, off2_ptr, nb_ptr, nb_len, smb_com) != DCE2_RET__SUCCESS)
        process = 0;

    if (num_sess > 0)
        ssx_chained = 1;

    if (process)
    {
        /* Move to the next command */
        DCE2_MOVE(nb_ptr, nb_len, (off2_ptr - nb_ptr));

        switch (smb_com)
        {
            case SMB_COM_SESS_SETUP_ANDX:
                if (SmbType(smb_hdr) == SMB_TYPE__RESPONSE)
                    ssd->req_uid = DCE2_SENTINEL;

                switch (smb_com2)
                {
                    case SMB_COM_LOGOFF_ANDX:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA_3_0_20:
                            case DCE2_POLICY__SAMBA_3_0_22:
                            case DCE2_POLICY__SAMBA_3_0_37:
                            case DCE2_POLICY__SAMBA:
                                DCE2_SmbLogoffAndX(ssd, smb_hdr, nb_ptr, nb_len, ssx_chained);
                                break;

                            default:
                                break;
                        }

                        break;

                    case SMB_COM_TREE_CON:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA_3_0_20:
                            case DCE2_POLICY__SAMBA_3_0_22:
                            case DCE2_POLICY__SAMBA_3_0_37:
                            case DCE2_POLICY__SAMBA:
                                {
                                    DCE2_Ret status = DCE2_SmbTreeConnect(ssd, smb_hdr, nb_ptr, nb_len);
                                    if ((SmbType(smb_hdr) == SMB_TYPE__REQUEST) &&
                                        (status == DCE2_RET__SUCCESS))
                                    {
                                        DCE2_SmbTreeConnectEnqueue(ssd, smb_hdr, status);
                                    }
                                }

                                break;

                            default:
                                break;
                        }

                        break;

                    case SMB_COM_TREE_CON_ANDX:
                        DCE2_SmbTreeConnectAndX(ssd, smb_hdr, nb_ptr, nb_len);
                        break;

                    case SMB_COM_TREE_DIS:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA_3_0_20:
                            case DCE2_POLICY__SAMBA_3_0_22:
                            case DCE2_POLICY__SAMBA_3_0_37:
                            case DCE2_POLICY__SAMBA:
                                DCE2_SmbTreeDisconnect(ssd, smb_hdr, nb_ptr, nb_len);
                                break;

                            default:
                                break;
                        }

                        break;

                    case SMB_COM_OPEN:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {

                            case DCE2_POLICY__WIN2000:
                            case DCE2_POLICY__WIN2003:
                            case DCE2_POLICY__WINXP:
                            case DCE2_POLICY__WINVISTA:
                            case DCE2_POLICY__WIN2008:
                            case DCE2_POLICY__WIN7:
                                DCE2_SmbOpen(ssd, smb_hdr, nb_ptr, nb_len);
                                break;

                            default:
                                break;
                        }

                        break;

                    case SMB_COM_OPEN_ANDX:
                        DCE2_SmbOpenAndX(ssd, smb_hdr, nb_ptr, nb_len);
                        break;

                    case SMB_COM_NT_CREATE_ANDX:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA_3_0_20:
                            case DCE2_POLICY__SAMBA_3_0_22:
                            case DCE2_POLICY__SAMBA_3_0_37:
                            case DCE2_POLICY__SAMBA:
                                DCE2_SmbNtCreateAndX(ssd, smb_hdr, nb_ptr, nb_len);
                                break;

                            default:
                                break;
                        }

                        break;

                    case SMB_COM_CLOSE:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA_3_0_20:
                            case DCE2_POLICY__SAMBA_3_0_22:
                            case DCE2_POLICY__SAMBA_3_0_37:
                            case DCE2_POLICY__SAMBA:
                                if (DCE2_SsnFromClient(ssd->sd.wire_pkt) && open_chain)
                                    DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_CHAIN_OPEN_CLOSE);
                                DCE2_SmbClose(ssd, smb_hdr, nb_ptr, nb_len, open_chain);
                                break;

                            default:
                                break;
                        }

                        break;

                    case SMB_COM_TRANS:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__WIN2000:
                                DCE2_SmbTrans(ssd, smb_hdr, nb_ptr, nb_len);
                                break;

                            default:
                                break;
                        }

                        break;

                    case SMB_COM_WRITE:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA_3_0_20:
                            case DCE2_POLICY__SAMBA_3_0_22:
                                if (DCE2_SsnFromClient(ssd->sd.wire_pkt) && open_chain)
                                    DCE2_SmbQueueTmpFid(ssd);
                                DCE2_SmbWrite(ssd, smb_hdr, nb_ptr, nb_len);
                                break;

                            default:
                                break;
                        }

                        break;

                    case SMB_COM_READ_ANDX:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA_3_0_20:
                            case DCE2_POLICY__SAMBA_3_0_22:
                            case DCE2_POLICY__SAMBA_3_0_37:
                            case DCE2_POLICY__SAMBA:
                                DCE2_SmbReadAndX(ssd, smb_hdr, nb_ptr, nb_len);
                                break;

                            default:
                                break;
                        }

                        break;

                    default:
                        break;
                }

                break;

            case SMB_COM_LOGOFF_ANDX:
                switch (smb_com2)
                {
                    case SMB_COM_SESS_SETUP_ANDX:
                        DCE2_SmbSessSetupAndX(ssd, smb_hdr, nb_ptr, nb_len);
                        break;

                    case SMB_COM_TREE_CON_ANDX:
                        /* Windows seems to respond to a chained LogoffAndX/SessionSetupAndX/TreeConnectAndX
                         * with a LogoffAndX/TreeConnectAndX */
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__WIN2000:
                            case DCE2_POLICY__WIN2003:
                            case DCE2_POLICY__WINVISTA:
                            case DCE2_POLICY__WINXP:
                            case DCE2_POLICY__WIN2008:
                            case DCE2_POLICY__WIN7:
                                if (SmbType(smb_hdr) == SMB_TYPE__RESPONSE)
                                {
                                    uint16_t uid = SmbUid(smb_hdr);

                                    /* Add uid if necessary */
                                    if (DCE2_SmbFindUid(ssd, uid) != DCE2_RET__SUCCESS)
                                        DCE2_SmbInsertUid(ssd, uid);

                                    DCE2_SmbTreeConnectAndX(ssd, smb_hdr, nb_ptr, nb_len);
                                }

                                break;

                            default:
                                break;
                        }

                        break;

                    case SMB_COM_TREE_DIS:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA_3_0_20:
                            case DCE2_POLICY__SAMBA_3_0_22:
                            case DCE2_POLICY__SAMBA_3_0_37:
                            case DCE2_POLICY__SAMBA:
                                DCE2_SmbTreeDisconnect(ssd, smb_hdr, nb_ptr, nb_len);
                                break;

                            default:
                                break;
                        }

                        break;

                    default:
                        break;
                }

                break;

            case SMB_COM_TREE_CON_ANDX:
                if (SmbType(smb_hdr) == SMB_TYPE__RESPONSE)
                    ssd->req_tid = DCE2_SENTINEL;

                switch (smb_com2)
                {
                    case SMB_COM_SESS_SETUP_ANDX:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA_3_0_20:
                            case DCE2_POLICY__SAMBA_3_0_22:
                            case DCE2_POLICY__SAMBA_3_0_37:
                            case DCE2_POLICY__SAMBA:
                                DCE2_SmbSessSetupAndX(ssd, smb_hdr, nb_ptr, nb_len);
                                break;

                            default:
                                break;
                        }

                        break;

                    case SMB_COM_LOGOFF_ANDX:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA_3_0_20:
                            case DCE2_POLICY__SAMBA_3_0_22:
                            case DCE2_POLICY__SAMBA_3_0_37:
                            case DCE2_POLICY__SAMBA:
                                DCE2_SmbLogoffAndX(ssd, smb_hdr, nb_ptr, nb_len, ssx_chained);
                                break;

                            default:
                                break;
                        }
                        
                        break;

                    case SMB_COM_TREE_DIS:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA_3_0_20:
                            case DCE2_POLICY__SAMBA_3_0_22:
                            case DCE2_POLICY__SAMBA_3_0_37:
                            case DCE2_POLICY__SAMBA:
                                DCE2_SmbTreeDisconnect(ssd, smb_hdr, nb_ptr, nb_len);
                                break;

                            default:
                                break;
                        }

                        break;

                    case SMB_COM_OPEN:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__WIN2000:
                            case DCE2_POLICY__WIN2003:
                            case DCE2_POLICY__WINXP:
                            case DCE2_POLICY__WINVISTA:
                            case DCE2_POLICY__WIN2008:
                            case DCE2_POLICY__WIN7:
                                DCE2_SmbOpen(ssd, smb_hdr, nb_ptr, nb_len);
                                break;

                            default:
                                break;
                        }

                        break;

                    case SMB_COM_OPEN_ANDX:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA_3_0_20:
                            case DCE2_POLICY__SAMBA_3_0_22:
                            case DCE2_POLICY__SAMBA_3_0_37:
                            case DCE2_POLICY__SAMBA:
                                DCE2_SmbOpenAndX(ssd, smb_hdr, nb_ptr, nb_len);
                                break;

                            default:
                                break;
                        }

                        break;

                    case SMB_COM_NT_CREATE_ANDX:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA_3_0_20:
                            case DCE2_POLICY__SAMBA_3_0_22:
                            case DCE2_POLICY__SAMBA_3_0_37:
                            case DCE2_POLICY__SAMBA:
                                DCE2_SmbNtCreateAndX(ssd, smb_hdr, nb_ptr, nb_len);
                                break;

                            default:
                                break;
                        }

                        break;

                    case SMB_COM_CLOSE:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA_3_0_20:
                            case DCE2_POLICY__SAMBA_3_0_22:
                            case DCE2_POLICY__SAMBA_3_0_37:
                            case DCE2_POLICY__SAMBA:
                                if (open_chain)
                                    DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_CHAIN_OPEN_CLOSE);
                                DCE2_SmbClose(ssd, smb_hdr, nb_ptr, nb_len, open_chain);
                                break;

                            default:
                                break;
                        }

                        break;

                    case SMB_COM_TRANS:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__WIN2000:
                                DCE2_SmbTrans(ssd, smb_hdr, nb_ptr, nb_len);
                                break;

                            default:
                                break;
                        }

                        break;

                    case SMB_COM_WRITE:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA_3_0_20:
                            case DCE2_POLICY__SAMBA_3_0_22:
                            case DCE2_POLICY__SAMBA_3_0_37:
                            case DCE2_POLICY__SAMBA:
                                if (DCE2_SsnFromClient(ssd->sd.wire_pkt) && open_chain)
                                    DCE2_SmbQueueTmpFid(ssd);
                                DCE2_SmbWrite(ssd, smb_hdr, nb_ptr, nb_len);
                                break;

                            default:
                                break;
                        }

                        break;

                    case SMB_COM_READ_ANDX:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA_3_0_20:
                            case DCE2_POLICY__SAMBA_3_0_22:
                            case DCE2_POLICY__SAMBA_3_0_37:
                            case DCE2_POLICY__SAMBA:
                                DCE2_SmbReadAndX(ssd, smb_hdr, nb_ptr, nb_len);
                                break;

                            default:
                                break;
                        }

                        break;

                    default:
                        break;
                }

                break;

            case SMB_COM_OPEN_ANDX:
                switch (smb_com2)
                {
                    case SMB_COM_SESS_SETUP_ANDX:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA_3_0_20:
                            case DCE2_POLICY__SAMBA_3_0_22:
                            case DCE2_POLICY__SAMBA_3_0_37:
                            case DCE2_POLICY__SAMBA:
                                DCE2_SmbSessSetupAndX(ssd, smb_hdr, nb_ptr, nb_len);
                                break;

                            default:
                                break;
                        }

                        break;

                    case SMB_COM_LOGOFF_ANDX:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA_3_0_20:
                            case DCE2_POLICY__SAMBA_3_0_22:
                            case DCE2_POLICY__SAMBA_3_0_37:
                            case DCE2_POLICY__SAMBA:
                                DCE2_SmbLogoffAndX(ssd, smb_hdr, nb_ptr, nb_len, ssx_chained);
                                break;

                            default:
                                break;
                        }
                        
                        break;

                    case SMB_COM_TREE_CON:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA_3_0_20:
                            case DCE2_POLICY__SAMBA_3_0_22:
                            case DCE2_POLICY__SAMBA_3_0_37:
                            case DCE2_POLICY__SAMBA:
                                {
                                    DCE2_Ret status;
                                    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "OpenAndX => TreeConnect\n"));
                                    status = DCE2_SmbTreeConnect(ssd, smb_hdr, nb_ptr, nb_len);
                                    if ((SmbType(smb_hdr) == SMB_TYPE__REQUEST) &&
                                        (status == DCE2_RET__SUCCESS))
                                    {
                                        ssd->chained_tc = 1;
                                        DCE2_SmbTreeConnectEnqueue(ssd, smb_hdr, status);
                                    }
                                }

                                break;

                            default:
                                break;
                        }

                        break;

                    case SMB_COM_TREE_CON_ANDX:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA_3_0_20:
                            case DCE2_POLICY__SAMBA_3_0_22:
                            case DCE2_POLICY__SAMBA_3_0_37:
                            case DCE2_POLICY__SAMBA:
                                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "OpenAndX => TreeConnectAndX\n"));
                                DCE2_SmbTreeConnectAndX(ssd, smb_hdr, nb_ptr, nb_len);
                                if (SmbType(smb_hdr) == SMB_TYPE__REQUEST)
                                    ssd->chained_tc = 1;
                                break;

                            default:
                                break;
                        }

                        break;

                    case SMB_COM_TREE_DIS:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA_3_0_20:
                            case DCE2_POLICY__SAMBA_3_0_22:
                            case DCE2_POLICY__SAMBA_3_0_37:
                            case DCE2_POLICY__SAMBA:
                                DCE2_SmbTreeDisconnect(ssd, smb_hdr, nb_ptr, nb_len);
                                break;

                            default:
                                break;
                        }

                        break;

                    case SMB_COM_OPEN_ANDX:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA_3_0_20:
                            case DCE2_POLICY__SAMBA_3_0_22:
                            case DCE2_POLICY__SAMBA_3_0_37:
                            case DCE2_POLICY__SAMBA:
                                DCE2_SmbOpenAndX(ssd, smb_hdr, nb_ptr, nb_len);
                                break;

                            default:
                                break;
                        }

                        break;

                    case SMB_COM_NT_CREATE_ANDX:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA_3_0_20:
                            case DCE2_POLICY__SAMBA_3_0_22:
                            case DCE2_POLICY__SAMBA_3_0_37:
                            case DCE2_POLICY__SAMBA:
                                DCE2_SmbNtCreateAndX(ssd, smb_hdr, nb_ptr, nb_len);
                                break;

                            default:
                                break;
                        }

                        break;

                    case SMB_COM_CLOSE:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA_3_0_20:
                            case DCE2_POLICY__SAMBA_3_0_22:
                            case DCE2_POLICY__SAMBA_3_0_37:
                            case DCE2_POLICY__SAMBA:
                                if (open_chain)
                                    DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_CHAIN_OPEN_CLOSE);
                                DCE2_SmbClose(ssd, smb_hdr, nb_ptr, nb_len, open_chain);
                                break;

                            default:
                                break;
                        }

                        break;

                    case SMB_COM_WRITE:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA_3_0_20:
                            case DCE2_POLICY__SAMBA_3_0_22:
                            case DCE2_POLICY__SAMBA_3_0_37:
                            case DCE2_POLICY__SAMBA:
                                DCE2_SmbQueueTmpFid(ssd);
                                DCE2_SmbWrite(ssd, smb_hdr, nb_ptr, nb_len);
                                break;

                            default:
                                break;
                        }

                        break;

                    case SMB_COM_READ_ANDX:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA_3_0_20:
                            case DCE2_POLICY__SAMBA_3_0_22:
                            case DCE2_POLICY__SAMBA_3_0_37:
                            case DCE2_POLICY__SAMBA:
                                DCE2_SmbReadAndX(ssd, smb_hdr, nb_ptr, nb_len);
                                break;

                            default:
                                break;
                        }

                        break;

                    default:
                        break;
                }

                break;

            case SMB_COM_NT_CREATE_ANDX:
                switch (smb_com2)
                {
                    case SMB_COM_SESS_SETUP_ANDX:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA_3_0_20:
                            case DCE2_POLICY__SAMBA_3_0_22:
                            case DCE2_POLICY__SAMBA_3_0_37:
                            case DCE2_POLICY__SAMBA:
                                DCE2_SmbSessSetupAndX(ssd, smb_hdr, nb_ptr, nb_len);
                                break;

                            default:
                                break;
                        }

                        break;

                    case SMB_COM_LOGOFF_ANDX:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA:
                            case DCE2_POLICY__SAMBA_3_0_37:
                                DCE2_SmbLogoffAndX(ssd, smb_hdr, nb_ptr, nb_len, ssx_chained);
                                break;

                            default:
                                break;
                        }
                        
                        break;

                    case SMB_COM_TREE_CON:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA_3_0_20:
                            case DCE2_POLICY__SAMBA_3_0_22:
                            case DCE2_POLICY__SAMBA_3_0_37:
                            case DCE2_POLICY__SAMBA:
                                {
                                    DCE2_Ret status;
                                    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "OpenAndX => TreeConnect\n"));
                                    status = DCE2_SmbTreeConnect(ssd, smb_hdr, nb_ptr, nb_len);
                                    if ((SmbType(smb_hdr) == SMB_TYPE__REQUEST) &&
                                        (status == DCE2_RET__SUCCESS))
                                    {
                                        ssd->chained_tc = 1;
                                        DCE2_SmbTreeConnectEnqueue(ssd, smb_hdr, status);
                                    }
                                }

                                break;

                            default:
                                break;
                        }

                        break;

                    case SMB_COM_TREE_CON_ANDX:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA_3_0_20:
                            case DCE2_POLICY__SAMBA_3_0_22:
                            case DCE2_POLICY__SAMBA_3_0_37:
                            case DCE2_POLICY__SAMBA:
                                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "OpenAndX => TreeConnectAndX\n"));
                                DCE2_SmbTreeConnectAndX(ssd, smb_hdr, nb_ptr, nb_len);
                                if (SmbType(smb_hdr) == SMB_TYPE__REQUEST)
                                    ssd->chained_tc = 1;
                                break;

                            default:
                                break;
                        }

                        break;

                    case SMB_COM_TREE_DIS:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA:
                            case DCE2_POLICY__SAMBA_3_0_37:
                                DCE2_SmbTreeDisconnect(ssd, smb_hdr, nb_ptr, nb_len);
                                break;

                            default:
                                break;
                        }

                        break;

                    case SMB_COM_OPEN_ANDX:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA_3_0_20:
                            case DCE2_POLICY__SAMBA_3_0_22:
                            case DCE2_POLICY__SAMBA_3_0_37:
                            case DCE2_POLICY__SAMBA:
                                DCE2_SmbOpenAndX(ssd, smb_hdr, nb_ptr, nb_len);
                                break;

                            default:
                                break;
                        }

                        break;

                    case SMB_COM_NT_CREATE_ANDX:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA_3_0_20:
                            case DCE2_POLICY__SAMBA_3_0_22:
                            case DCE2_POLICY__SAMBA_3_0_37:
                            case DCE2_POLICY__SAMBA:
                                DCE2_SmbNtCreateAndX(ssd, smb_hdr, nb_ptr, nb_len);
                                break;

                            default:
                                break;
                        }

                        break;

                    case SMB_COM_CLOSE:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA:
                            case DCE2_POLICY__SAMBA_3_0_37:
                                if (open_chain)
                                    DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_CHAIN_OPEN_CLOSE);
                                DCE2_SmbClose(ssd, smb_hdr, nb_ptr, nb_len, open_chain);
                                break;

                            default:
                                break;
                        }

                        break;

                    case SMB_COM_WRITE:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA_3_0_20:
                            case DCE2_POLICY__SAMBA_3_0_22:
                            case DCE2_POLICY__SAMBA_3_0_37:
                            case DCE2_POLICY__SAMBA:
                                DCE2_SmbQueueTmpFid(ssd);
                                DCE2_SmbWrite(ssd, smb_hdr, nb_ptr, nb_len);
                                break;

                            default:
                                break;
                        }

                        break;

                    case SMB_COM_READ_ANDX:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA_3_0_20:
                            case DCE2_POLICY__SAMBA_3_0_22:
                            case DCE2_POLICY__SAMBA_3_0_37:
                            case DCE2_POLICY__SAMBA:
                                DCE2_SmbReadAndX(ssd, smb_hdr, nb_ptr, nb_len);
                                break;

                            default:
                                break;
                        }

                        break;

                    default:
                        break;
                }

                break;

            case SMB_COM_WRITE_ANDX:
                switch (smb_com2)
                {
                    case SMB_COM_SESS_SETUP_ANDX:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA_3_0_20:
                            case DCE2_POLICY__SAMBA_3_0_22:
                            case DCE2_POLICY__SAMBA_3_0_37:
                            case DCE2_POLICY__SAMBA:
                                DCE2_SmbSessSetupAndX(ssd, smb_hdr, nb_ptr, nb_len);
                                break;

                            default:
                                break;
                        }

                        break;

                    case SMB_COM_LOGOFF_ANDX:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA_3_0_20:
                            case DCE2_POLICY__SAMBA_3_0_22:
                            case DCE2_POLICY__SAMBA_3_0_37:
                            case DCE2_POLICY__SAMBA:
                                DCE2_SmbLogoffAndX(ssd, smb_hdr, nb_ptr, nb_len, ssx_chained);
                                break;

                            default:
                                break;
                        }
                        
                        break;

                    case SMB_COM_TREE_CON:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA_3_0_20:
                            case DCE2_POLICY__SAMBA_3_0_22:
                            case DCE2_POLICY__SAMBA_3_0_37:
                            case DCE2_POLICY__SAMBA:
                                {
                                    DCE2_Ret status = DCE2_SmbTreeConnect(ssd, smb_hdr, nb_ptr, nb_len);
                                    if ((SmbType(smb_hdr) == SMB_TYPE__REQUEST) &&
                                        (status == DCE2_RET__SUCCESS))
                                    {
                                        ssd->chained_tc = 1;
                                        DCE2_SmbTreeConnectEnqueue(ssd, smb_hdr, status);
                                    }
                                }

                                break;

                            default:
                                break;
                        }

                        break;

                    case SMB_COM_TREE_CON_ANDX:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA_3_0_20:
                            case DCE2_POLICY__SAMBA_3_0_22:
                            case DCE2_POLICY__SAMBA_3_0_37:
                            case DCE2_POLICY__SAMBA:
                                DCE2_SmbTreeConnectAndX(ssd, smb_hdr, nb_ptr, nb_len);
                                if (SmbType(smb_hdr) == SMB_TYPE__REQUEST)
                                    ssd->chained_tc = 1;
                                break;

                            default:
                                break;
                        }

                        break;

                    case SMB_COM_OPEN_ANDX:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA_3_0_20:
                            case DCE2_POLICY__SAMBA_3_0_22:
                            case DCE2_POLICY__SAMBA_3_0_37:
                            case DCE2_POLICY__SAMBA:
                                DCE2_SmbOpenAndX(ssd, smb_hdr, nb_ptr, nb_len);
                                break;

                            default:
                                break;
                        }

                        break;

                    case SMB_COM_NT_CREATE_ANDX:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA_3_0_20:
                            case DCE2_POLICY__SAMBA_3_0_22:
                            case DCE2_POLICY__SAMBA_3_0_37:
                            case DCE2_POLICY__SAMBA:
                                DCE2_SmbNtCreateAndX(ssd, smb_hdr, nb_ptr, nb_len);
                                break;

                            default:
                                break;
                        }

                        break;

                    case SMB_COM_CLOSE:
                        if (open_chain)
                            DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_CHAIN_OPEN_CLOSE);
                        DCE2_SmbClose(ssd, smb_hdr, nb_ptr, nb_len, open_chain);
                        break;

                    case SMB_COM_WRITE_ANDX:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__WIN2000:
                            case DCE2_POLICY__WIN2003:
                            case DCE2_POLICY__WINXP:
                            case DCE2_POLICY__WINVISTA:
                            case DCE2_POLICY__WIN2008:
                            case DCE2_POLICY__WIN7:
                                DCE2_SmbWriteAndX(ssd, smb_hdr, nb_ptr, nb_len);
                                break;

                            default:
                                break;
                        }

                        break;

                    case SMB_COM_WRITE:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA_3_0_20:
                            case DCE2_POLICY__SAMBA_3_0_22:
                            case DCE2_POLICY__SAMBA_3_0_37:
                            case DCE2_POLICY__SAMBA:
                                DCE2_SmbWrite(ssd, smb_hdr, nb_ptr, nb_len);
                                break;

                            default:
                                break;
                        }

                        break;

                    case SMB_COM_READ:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__WIN2000:
                            case DCE2_POLICY__WIN2003:
                            case DCE2_POLICY__WINXP:
                            case DCE2_POLICY__WINVISTA:
                            case DCE2_POLICY__WIN2008:
                            case DCE2_POLICY__WIN7:
                                DCE2_SmbRead(ssd, smb_hdr, nb_ptr, nb_len);
                                break;

                            default:
                                break;
                        }

                        break;

                    case SMB_COM_READ_ANDX:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA_3_0_20:
                            case DCE2_POLICY__SAMBA_3_0_22:
                            case DCE2_POLICY__SAMBA_3_0_37:
                            case DCE2_POLICY__SAMBA:
                            case DCE2_POLICY__WIN2000:
                            case DCE2_POLICY__WIN2003:
                            case DCE2_POLICY__WINXP:
                            case DCE2_POLICY__WINVISTA:
                            case DCE2_POLICY__WIN2008:
                            case DCE2_POLICY__WIN7:
                                DCE2_SmbReadAndX(ssd, smb_hdr, nb_ptr, nb_len);
                                break;

                            default:
                                break;
                        }

                        break;

                    default:
                        break;
                }

                break;

            case SMB_COM_READ_ANDX:
                switch (smb_com2)
                {
                    case SMB_COM_SESS_SETUP_ANDX:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA_3_0_20:
                            case DCE2_POLICY__SAMBA_3_0_22:
                            case DCE2_POLICY__SAMBA_3_0_37:
                            case DCE2_POLICY__SAMBA:
                                DCE2_SmbSessSetupAndX(ssd, smb_hdr, nb_ptr, nb_len);
                                break;

                            default:
                                break;
                        }

                        break;

                    case SMB_COM_LOGOFF_ANDX:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA:
                            case DCE2_POLICY__SAMBA_3_0_37:
                                DCE2_SmbLogoffAndX(ssd, smb_hdr, nb_ptr, nb_len, ssx_chained);
                                break;

                            default:
                                break;
                        }
                        
                        break;

                    case SMB_COM_TREE_CON:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA:
                            case DCE2_POLICY__SAMBA_3_0_37:
                                {
                                    DCE2_Ret status = DCE2_SmbTreeConnect(ssd, smb_hdr, nb_ptr, nb_len);
                                    if ((SmbType(smb_hdr) == SMB_TYPE__REQUEST) &&
                                        (status == DCE2_RET__SUCCESS))
                                    {
                                        ssd->chained_tc = 1;
                                        DCE2_SmbTreeConnectEnqueue(ssd, smb_hdr, status);
                                    }
                                }

                                break;

                            default:
                                break;
                        }

                        break;

                    case SMB_COM_TREE_CON_ANDX:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA:
                            case DCE2_POLICY__SAMBA_3_0_37:
                                DCE2_SmbTreeConnectAndX(ssd, smb_hdr, nb_ptr, nb_len);
                                if (SmbType(smb_hdr) == SMB_TYPE__REQUEST)
                                    ssd->chained_tc = 1;
                                break;

                            default:
                                break;
                        }

                        break;

                    case SMB_COM_TREE_DIS:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA:
                            case DCE2_POLICY__SAMBA_3_0_37:
                                DCE2_SmbTreeDisconnect(ssd, smb_hdr, nb_ptr, nb_len);
                                break;

                            default:
                                break;
                        }

                        break;

                    case SMB_COM_OPEN_ANDX:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA:
                            case DCE2_POLICY__SAMBA_3_0_37:
                                DCE2_SmbOpenAndX(ssd, smb_hdr, nb_ptr, nb_len);
                                break;

                            default:
                                break;
                        }

                        break;

                    case SMB_COM_NT_CREATE_ANDX:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA:
                            case DCE2_POLICY__SAMBA_3_0_37:
                                DCE2_SmbNtCreateAndX(ssd, smb_hdr, nb_ptr, nb_len);
                                break;

                            default:
                                break;
                        }

                        break;

                    case SMB_COM_CLOSE:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA:
                            case DCE2_POLICY__SAMBA_3_0_37:
                                if (open_chain)
                                    DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_CHAIN_OPEN_CLOSE);
                                DCE2_SmbClose(ssd, smb_hdr, nb_ptr, nb_len, open_chain);
                                break;

                            default:
                                break;
                        }

                        break;

                    case SMB_COM_WRITE:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA_3_0_20:
                            case DCE2_POLICY__SAMBA_3_0_22:
                            case DCE2_POLICY__SAMBA_3_0_37:
                            case DCE2_POLICY__SAMBA:
                                DCE2_SmbWrite(ssd, smb_hdr, nb_ptr, nb_len);
                                break;

                            default:
                                break;
                        }

                        break;

                    case SMB_COM_READ_ANDX:
                        switch (DCE2_ScPolicy(ssd->sd.sconfig))
                        {
                            case DCE2_POLICY__SAMBA:
                            case DCE2_POLICY__SAMBA_3_0_37:
                                DCE2_SmbReadAndX(ssd, smb_hdr, nb_ptr, nb_len);
                                break;

                            default:
                                break;
                        }

                        break;

                    default:
                        break;
                }

                break;

            default:
                break;
        }
    }

    DCE2_SMB_CHAINED__RESET_STATICS;
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
static void DCE2_WriteCoProcess(DCE2_SmbSsnData *ssd, const SmbNtHdr *smb_hdr,
                                const uint16_t fid, const uint8_t *nb_ptr, uint16_t dsize)
{
    DCE2_SmbFidTrackerNode *ft_node = DCE2_SmbFindFid(ssd, SmbUid(smb_hdr), SmbTid(smb_hdr), fid);

    if (ft_node == NULL)
        return;

    if (dsize != 0)
        DCE2_CoProcess(&ssd->sd, &ft_node->co_tracker, nb_ptr, dsize);
        
    if (!ft_node->used)
        ft_node->used = 1;
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
static void DCE2_SmbInsertUid(DCE2_SmbSsnData *ssd, const uint16_t uid)
{
    PROFILE_VARS;

    PREPROC_PROFILE_START(dce2_pstat_smb_uid);

    switch (DCE2_ScPolicy(ssd->sd.sconfig))
    {
        case DCE2_POLICY__WIN2000:
        case DCE2_POLICY__SAMBA_3_0_20:
        case DCE2_POLICY__SAMBA_3_0_22:
            if (ssd->sst.uid == DCE2_SENTINEL)
            {
                ssd->sst.uid = uid;
            }
            else
            {
                if (ssd->sst.uids == NULL)
                {
                    ssd->sst.uids = DCE2_ListNew(DCE2_LIST_TYPE__SPLAYED, DCE2_SmbUTFCompare,
                                                 NULL, NULL, DCE2_LIST_FLAG__NO_DUPS, DCE2_MEM_TYPE__SMB_UID);

                    if (ssd->sst.uids == NULL)
                    {
                        PREPROC_PROFILE_END(dce2_pstat_smb_uid);
                        return;
                    }
                }

                DCE2_ListInsert(ssd->sst.uids, (void *)(uintptr_t)uid, (void *)(uintptr_t)uid);
            }

            break;

        default:
            /* The other policies don't use the UID list */
            break;
    }

    PREPROC_PROFILE_END(dce2_pstat_smb_uid);
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
static DCE2_Ret DCE2_SmbFindUid(DCE2_SmbSsnData *ssd, const uint16_t uid)
{
    PROFILE_VARS;

    PREPROC_PROFILE_START(dce2_pstat_smb_uid);

    switch (DCE2_ScPolicy(ssd->sd.sconfig))
    {
        case DCE2_POLICY__WIN2000:
        case DCE2_POLICY__SAMBA_3_0_20:
        case DCE2_POLICY__SAMBA_3_0_22:
            if ((ssd->sst.uid != DCE2_SENTINEL) && (ssd->sst.uid == uid))
            {
                PREPROC_PROFILE_END(dce2_pstat_smb_uid);
                return DCE2_RET__SUCCESS;
            }

            if (ssd->sst.uids != NULL)
            {
                if (DCE2_ListFindKey(ssd->sst.uids, (void *)(uintptr_t)uid) == DCE2_RET__SUCCESS)
                {
                    PREPROC_PROFILE_END(dce2_pstat_smb_uid);
                    return DCE2_RET__SUCCESS;
                }
            }

            break;

        default:
            /* The other policies don't use the UID list */
            PREPROC_PROFILE_END(dce2_pstat_smb_uid);
            return DCE2_RET__SUCCESS;
    }

    PREPROC_PROFILE_END(dce2_pstat_smb_uid);

    return DCE2_RET__ERROR;
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
static void DCE2_SmbRemoveUid(DCE2_SmbSsnData *ssd, const uint16_t uid)
{
    const DCE2_Policy policy = DCE2_ScPolicy(ssd->sd.sconfig);
    PROFILE_VARS;

    PREPROC_PROFILE_START(dce2_pstat_smb_uid);

    switch (policy)
    {
        case DCE2_POLICY__SAMBA:
        case DCE2_POLICY__SAMBA_3_0_37:
        case DCE2_POLICY__WIN2000:
            /* Removing uid invalidates any fid that was created with it */
            if ((ssd->sst.ft_node.fid_node.fid != DCE2_SENTINEL) &&
                (ssd->sst.ft_node.fid_node.uid == (int)uid))
            {
                DCE2_SmbCleanFidNode(&ssd->sst.ft_node);
            }

            if (ssd->sst.fts != NULL)
            {
                DCE2_SmbFidTrackerNode *ft_node;

                for (ft_node = DCE2_ListFirst(ssd->sst.fts);
                     ft_node != NULL;
                     ft_node = DCE2_ListNext(ssd->sst.fts))
                {
                    if (ft_node->fid_node.uid == (int)uid)
                        DCE2_ListRemoveCurrent(ssd->sst.fts);
                }
            }

            if (policy != DCE2_POLICY__WIN2000)
            {
                PREPROC_PROFILE_END(dce2_pstat_smb_uid);
                return;
            }

            /* Fall through for Windows 2000 since we're keeping track of uids since
             * any uid can be used with any fid */

        case DCE2_POLICY__SAMBA_3_0_20:
        case DCE2_POLICY__SAMBA_3_0_22:
            if ((ssd->sst.uid != DCE2_SENTINEL) && (ssd->sst.uid == (int)uid))
                ssd->sst.uid = DCE2_SENTINEL;
            else if (ssd->sst.uids != NULL)
                DCE2_ListRemove(ssd->sst.uids, (void *)(uintptr_t)uid);

            if (policy != DCE2_POLICY__WIN2000)
            {
                PREPROC_PROFILE_END(dce2_pstat_smb_uid);
                return;
            }
            
            /* Fall through for Windows 2000 since we're keeping a pipe tree for it 
             * for use with a first request/write */

        case DCE2_POLICY__WIN2003:
        case DCE2_POLICY__WINXP:
        case DCE2_POLICY__WINVISTA:
        case DCE2_POLICY__WIN2008:
        case DCE2_POLICY__WIN7:
            {
                DCE2_SmbPipeTree *ptree = &ssd->sst.ptree;

                if ((ptree->ut_node.uid != DCE2_SENTINEL) && (ptree->ut_node.uid == (int)uid))
                {
                    DCE2_SmbCleanUTNode(&ptree->ut_node);
                }

                if (ptree->uts != NULL)
                {
                    DCE2_SmbUTNode *ut_node;

                    for (ut_node = DCE2_ListFirst(ptree->uts);
                         ut_node != NULL;
                         ut_node = DCE2_ListNext(ptree->uts))
                    {
                        if (ut_node->uid == (int)uid)
                            DCE2_ListRemoveCurrent(ptree->uts);
                    }
                }
            }

            break;

        default:
            break;
    }

    PREPROC_PROFILE_END(dce2_pstat_smb_uid);
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
static void DCE2_SmbInsertTid(DCE2_SmbSsnData *ssd, const uint16_t tid)
{
    PROFILE_VARS;

    PREPROC_PROFILE_START(dce2_pstat_smb_tid);

    if (ssd->sst.tid == DCE2_SENTINEL)
    {
        ssd->sst.tid = (int)tid;
    }
    else
    {
        if (ssd->sst.tids == NULL)
        {
            ssd->sst.tids = DCE2_ListNew(DCE2_LIST_TYPE__SPLAYED, DCE2_SmbUTFCompare,
                                         NULL, NULL, DCE2_LIST_FLAG__NO_DUPS, DCE2_MEM_TYPE__SMB_TID);

            if (ssd->sst.tids == NULL)
            {
                PREPROC_PROFILE_END(dce2_pstat_smb_tid);
                return;
            }
        }

        DCE2_ListInsert(ssd->sst.tids, (void *)(uintptr_t)tid, (void *)(uintptr_t)tid);
    }

    PREPROC_PROFILE_END(dce2_pstat_smb_tid);
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
static DCE2_Ret DCE2_SmbFindTid(DCE2_SmbSsnData *ssd, const uint16_t tid)
{
    PROFILE_VARS;

    PREPROC_PROFILE_START(dce2_pstat_smb_tid);

    if ((ssd->sst.tid != DCE2_SENTINEL) && (ssd->sst.tid == (int)tid))
    {
        PREPROC_PROFILE_END(dce2_pstat_smb_tid);
        return DCE2_RET__SUCCESS;
    }

    if (ssd->sst.tids != NULL)
    {
        if (DCE2_ListFindKey(ssd->sst.tids, (void *)(uintptr_t)tid) == DCE2_RET__SUCCESS)
        {
            PREPROC_PROFILE_END(dce2_pstat_smb_tid);
            return DCE2_RET__SUCCESS;
        }
    }

    PREPROC_PROFILE_END(dce2_pstat_smb_tid);

    return DCE2_RET__ERROR;
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
static void DCE2_SmbRemoveTid(DCE2_SmbSsnData *ssd, const uint16_t tid)
{
    const DCE2_Policy policy = DCE2_ScPolicy(ssd->sd.sconfig);
    PROFILE_VARS;

    PREPROC_PROFILE_START(dce2_pstat_smb_tid);

    if ((ssd->sst.tid != DCE2_SENTINEL) && (ssd->sst.tid == (int)tid))
        ssd->sst.tid = DCE2_SENTINEL;
    else if (ssd->sst.tids != NULL)
        DCE2_ListRemove(ssd->sst.tids, (void *)(uintptr_t)tid);

    switch (policy)
    {
        case DCE2_POLICY__WIN2000:
        case DCE2_POLICY__SAMBA:
        case DCE2_POLICY__SAMBA_3_0_37:
        case DCE2_POLICY__SAMBA_3_0_20:
        case DCE2_POLICY__SAMBA_3_0_22:
            if ((ssd->sst.ft_node.fid_node.fid != DCE2_SENTINEL) &&
                (ssd->sst.ft_node.fid_node.tid == (int)tid))
            {
                DCE2_SmbCleanFidNode(&ssd->sst.ft_node);
            }

            if (ssd->sst.fts != NULL)
            {
                DCE2_SmbFidTrackerNode *ft_node;

                for (ft_node = DCE2_ListFirst(ssd->sst.fts);
                     ft_node != NULL;
                     ft_node = DCE2_ListNext(ssd->sst.fts))
                {
                    if (ft_node->fid_node.tid == (int)tid)
                        DCE2_ListRemoveCurrent(ssd->sst.fts);
                }
            }

            if (policy != DCE2_POLICY__WIN2000)
            {
                PREPROC_PROFILE_END(dce2_pstat_smb_tid);
                return;
            }

            /* Fall through for Windows 2000 since we're keeping a pipe tree for it 
             * for use with a first request/write */

        case DCE2_POLICY__WIN2003:
        case DCE2_POLICY__WINXP:
        case DCE2_POLICY__WINVISTA:
        case DCE2_POLICY__WIN2008:
        case DCE2_POLICY__WIN7:
            {
                DCE2_SmbPipeTree *ptree = &ssd->sst.ptree;

                if ((ptree->ut_node.tid != DCE2_SENTINEL) && (ptree->ut_node.tid == (int)tid))
                {
                    DCE2_SmbCleanUTNode(&ptree->ut_node);
                }

                if (ptree->uts != NULL)
                {
                    DCE2_SmbUTNode *ut_node;

                    for (ut_node = DCE2_ListFirst(ptree->uts);
                         ut_node != NULL;
                         ut_node = DCE2_ListNext(ptree->uts))
                    {
                        if (ut_node->tid == (int)tid)
                            DCE2_ListRemoveCurrent(ptree->uts);
                    }
                }
            }

            break;

        default:
            break;
    }

    PREPROC_PROFILE_END(dce2_pstat_smb_tid);
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
static DCE2_SmbUTNode * DCE2_SmbFindUTNode(DCE2_SmbSsnData *ssd,
                                           const uint16_t uid, const uint16_t tid)
{
    DCE2_Policy policy = DCE2_ScPolicy(ssd->sd.sconfig);
    DCE2_SmbPipeTree *ptree = &ssd->sst.ptree;
    DCE2_SmbUTNode *ut_node = NULL;

    switch (policy)
    {
        case DCE2_POLICY__WIN2000:
        case DCE2_POLICY__WIN2003:
        case DCE2_POLICY__WINXP:
        case DCE2_POLICY__WINVISTA:
        case DCE2_POLICY__WIN2008:
        case DCE2_POLICY__WIN7:
            if ((ptree->ut_node.uid != DCE2_SENTINEL) && (ptree->ut_node.tid != DCE2_SENTINEL) &&
                (ptree->ut_node.uid == (int)uid) && (ptree->ut_node.tid == (int)tid))
            {
                ut_node = &ptree->ut_node;
            }
            else if (ptree->uts != NULL)
            {
                ut_node = DCE2_ListFind(ptree->uts, (void *)(uintptr_t)((uid << 16) | tid));
            }

            break;

        default:
            break;
    }

    return ut_node;
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
static void DCE2_SmbInsertFid(DCE2_SmbSsnData *ssd, const uint16_t uid,
                              const uint16_t tid, const uint16_t fid)
{
    const DCE2_Policy policy = DCE2_ScPolicy(ssd->sd.sconfig); 
    DCE2_SmbUTNode *ut_node;
    DCE2_SmbFidTrackerNode *ft_node;
    PROFILE_VARS;

    PREPROC_PROFILE_START(dce2_pstat_smb_fid);

    switch (policy)
    {
        case DCE2_POLICY__WIN2000:
        case DCE2_POLICY__SAMBA_3_0_20:
        case DCE2_POLICY__SAMBA_3_0_22:
        case DCE2_POLICY__SAMBA_3_0_37:
        case DCE2_POLICY__SAMBA:
            /* Assume uid is ok since fid is added on server response */
            /* Tid should have already been validated */

            if (ssd->sst.ft_node.fid_node.fid == DCE2_SENTINEL)
            {
                ft_node = &ssd->sst.ft_node;
            }
            else
            {
                if (ssd->sst.fts == NULL)
                {
                    ssd->sst.fts = DCE2_ListNew(DCE2_LIST_TYPE__SPLAYED, DCE2_SmbUTFCompare,
                                                DCE2_SmbFidTrackerDataFree, NULL,
                                                DCE2_LIST_FLAG__NO_DUPS, DCE2_MEM_TYPE__SMB_FID);

                    if (ssd->sst.fts == NULL)
                    {
                        PREPROC_PROFILE_END(dce2_pstat_smb_fid);
                        return;
                    }
                }

                ft_node = (DCE2_SmbFidTrackerNode *)
                    DCE2_Alloc(sizeof(DCE2_SmbFidTrackerNode), DCE2_MEM_TYPE__SMB_FID);

                if (ft_node == NULL)
                {
                    PREPROC_PROFILE_END(dce2_pstat_smb_fid);
                    return;
                }

                if (DCE2_ListInsert(ssd->sst.fts,
                                    (void *)(uintptr_t)fid, (void *)ft_node) != DCE2_RET__SUCCESS)
                {
                    DCE2_Free((void *)ft_node, sizeof(DCE2_SmbFidTrackerNode), DCE2_MEM_TYPE__SMB_FID);
                    PREPROC_PROFILE_END(dce2_pstat_smb_fid);
                    return;
                }
            }

            ft_node->fid_node.fid = (int)fid;
            ft_node->fid_node.uid = (int)uid;
            ft_node->fid_node.tid = (int)tid;
            DCE2_CoInitTracker(&ft_node->co_tracker);

            if (policy != DCE2_POLICY__WIN2000)
            {
                PREPROC_PROFILE_END(dce2_pstat_smb_fid);
                return;
            }

            /* Fall through for Windows 2000 */

        case DCE2_POLICY__WIN2003:
        case DCE2_POLICY__WINXP:
        case DCE2_POLICY__WINVISTA:
        case DCE2_POLICY__WIN2008:
        case DCE2_POLICY__WIN7:
            ut_node = DCE2_SmbFindUTNode(ssd, uid, tid);
            if (ut_node == NULL)
            {
                DCE2_SmbPipeTree *ptree = &ssd->sst.ptree;

                /* Need to create UT node */
                if ((ptree->ut_node.uid == DCE2_SENTINEL) && (ptree->ut_node.tid == DCE2_SENTINEL))
                {
                    ut_node = &ptree->ut_node;
                }
                else
                {
                    if (ptree->uts == NULL)
                    {
                        ptree->uts = DCE2_ListNew(DCE2_LIST_TYPE__SPLAYED, DCE2_SmbUTPtreeCompare,
                                                  DCE2_SmbUTDataFree, NULL, DCE2_LIST_FLAG__NO_DUPS,
                                                  DCE2_MEM_TYPE__SMB_UT);
                        if (ptree->uts == NULL)
                        {
                            PREPROC_PROFILE_END(dce2_pstat_smb_fid);
                            return;
                        }
                    }

                    ut_node = (DCE2_SmbUTNode *)DCE2_Alloc(sizeof(DCE2_SmbUTNode), DCE2_MEM_TYPE__SMB_UT);
                    if (ut_node == NULL)
                    {
                        PREPROC_PROFILE_END(dce2_pstat_smb_fid);
                        return;
                    }

                    if (DCE2_ListInsert(ptree->uts, (void *)(uintptr_t)((uid << 16) | tid),
                                        (void *)ut_node) != DCE2_RET__SUCCESS)
                    {
                        DCE2_Free((void *)ut_node, sizeof(DCE2_SmbUTNode), DCE2_MEM_TYPE__SMB_UT);
                        PREPROC_PROFILE_END(dce2_pstat_smb_fid);
                        return;
                    }
                }

                ut_node->uid = (int)uid;
                ut_node->tid = (int)tid;

                ft_node = &ut_node->ft_node;
            }
            else
            {
                if (ut_node->ft_node.fid_node.fid == DCE2_SENTINEL)
                {
                    ft_node = &ut_node->ft_node;
                }
                else
                {
                    if (ut_node->fts == NULL)
                    {
                        ut_node->fts = DCE2_ListNew(DCE2_LIST_TYPE__SPLAYED, DCE2_SmbUTFCompare,
                                                    DCE2_SmbFidTrackerDataFree, NULL, DCE2_LIST_FLAG__NO_DUPS,
                                                    DCE2_MEM_TYPE__SMB_FID);
                        if (ut_node->fts == NULL)
                        {
                            PREPROC_PROFILE_END(dce2_pstat_smb_fid);
                            return;
                        }
                    }

                    ft_node = (DCE2_SmbFidTrackerNode *)
                        DCE2_Alloc(sizeof(DCE2_SmbFidTrackerNode), DCE2_MEM_TYPE__SMB_FID);

                    if (ft_node == NULL)
                    {
                        PREPROC_PROFILE_END(dce2_pstat_smb_fid);
                        return;
                    }

                    if (DCE2_ListInsert(ut_node->fts,
                                        (void *)(uintptr_t)fid, (void *)ft_node) != DCE2_RET__SUCCESS)
                    {
                        DCE2_Free((void *)ft_node, sizeof(DCE2_SmbFidTrackerNode), DCE2_MEM_TYPE__SMB_FID);
                        PREPROC_PROFILE_END(dce2_pstat_smb_fid);
                        return;
                    }
                }
            }

            ft_node->fid_node.uid = (int)uid;
            ft_node->fid_node.tid = (int)tid;
            ft_node->fid_node.fid = (int)fid;

            DCE2_CoInitTracker(&ft_node->co_tracker);

            break;

        default:
            break;
    }

    PREPROC_PROFILE_END(dce2_pstat_smb_fid);
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
static void DCE2_SmbQueueTmpFid(DCE2_SmbSsnData *ssd)
{
    DCE2_SmbFidTrackerNode *ft_node = NULL;
    PROFILE_VARS;

    PREPROC_PROFILE_START(dce2_pstat_smb_fid);

    if (ssd->ft_queue == NULL)
    {
        ssd->ft_queue = DCE2_QueueNew(DCE2_SmbFidTrackerDataFree, DCE2_MEM_TYPE__SMB_FID);
        if (ssd->ft_queue == NULL)
        {
            PREPROC_PROFILE_END(dce2_pstat_smb_fid);
            return;
        }
    }

    ft_node = (DCE2_SmbFidTrackerNode *)
        DCE2_Alloc(sizeof(DCE2_SmbFidTrackerNode), DCE2_MEM_TYPE__SMB_FID);

    if (ft_node == NULL)
    {
        PREPROC_PROFILE_END(dce2_pstat_smb_fid);
        return;
    }

    if (DCE2_QueueEnqueue(ssd->ft_queue, (void *)ft_node) != DCE2_RET__SUCCESS)
    {
        DCE2_Free((void *)ft_node, sizeof(DCE2_SmbFidTrackerNode), DCE2_MEM_TYPE__SMB_FID);
        PREPROC_PROFILE_END(dce2_pstat_smb_fid);
        return;
    }

    PREPROC_PROFILE_END(dce2_pstat_smb_fid);
}

/********************************************************************
 * Function:
 *
 *
 * Arguments:
 *
 * Returns: None
 *
 * NOTE: Caller must assume that the FidTrackerNode passed in will
 *       no longer be after a call to this function, i.e. DO NOT
 *       USE the ft_node passed in after calling this.
 *
 * Don't initialize tracker here.  It should have already been
 * initialized.
 *
 ********************************************************************/
static void DCE2_SmbInsertFidNode(DCE2_SmbSsnData *ssd, const uint16_t uid, const uint16_t tid,
                                  const uint16_t fid, DCE2_SmbFidTrackerNode *ft_node)
{
    const DCE2_Policy policy = DCE2_ScPolicy(ssd->sd.sconfig); 
    DCE2_SmbUTNode *ut_node;
    DCE2_SmbFidTrackerNode *tmp_ft_node;
    PROFILE_VARS;

    PREPROC_PROFILE_START(dce2_pstat_smb_fid);

    if (ft_node == NULL)
    {
        PREPROC_PROFILE_END(dce2_pstat_smb_fid);
        return;
    }

    switch (policy)
    {
        case DCE2_POLICY__WIN2000:
        case DCE2_POLICY__SAMBA_3_0_20:
        case DCE2_POLICY__SAMBA_3_0_22:
        case DCE2_POLICY__SAMBA_3_0_37:
        case DCE2_POLICY__SAMBA:
            /* Assume uid is ok since fid is added on server response */
            /* Tid should have already been validated */

            if (ssd->sst.fts == NULL)
            {
                ssd->sst.fts = DCE2_ListNew(DCE2_LIST_TYPE__SPLAYED, DCE2_SmbUTFCompare,
                                            DCE2_SmbFidTrackerDataFree, NULL,
                                            DCE2_LIST_FLAG__NO_DUPS, DCE2_MEM_TYPE__SMB_FID);

                if (ssd->sst.fts == NULL)
                {
                    DCE2_Free((void *)ft_node, sizeof(DCE2_SmbFidTrackerNode), DCE2_MEM_TYPE__SMB_FID);
                    PREPROC_PROFILE_END(dce2_pstat_smb_fid);
                    return;
                }
            }

            if (DCE2_ListInsert(ssd->sst.fts,
                                (void *)(uintptr_t)fid, (void *)ft_node) != DCE2_RET__SUCCESS)
            {
                DCE2_Free((void *)ft_node, sizeof(DCE2_SmbFidTrackerNode), DCE2_MEM_TYPE__SMB_FID);
                PREPROC_PROFILE_END(dce2_pstat_smb_fid);
                return;
            }

            ft_node->fid_node.fid = (int)fid;
            ft_node->fid_node.uid = (int)uid;
            ft_node->fid_node.tid = (int)tid;

            if (policy != DCE2_POLICY__WIN2000)
            {
                PREPROC_PROFILE_END(dce2_pstat_smb_fid);
                return;
            }

            /* Need to copy data from passed in ft node into new ft node for 
             * Windows 2000 */
            tmp_ft_node = (DCE2_SmbFidTrackerNode *)
                DCE2_Alloc(sizeof(DCE2_SmbFidTrackerNode), DCE2_MEM_TYPE__SMB_FID);

            if (tmp_ft_node == NULL)
            {
                DCE2_ListRemove(ssd->sst.fts, (void *)(uintptr_t)fid);
                PREPROC_PROFILE_END(dce2_pstat_smb_fid);
                return;
            }

            memcpy(tmp_ft_node, ft_node, sizeof(DCE2_SmbFidTrackerNode));

            /* Already inserted original ft node into ft list.  Don't need
             * reference to it anymore */
            ft_node = tmp_ft_node;

            /* Fall through for Windows 2000 */

        case DCE2_POLICY__WIN2003:
        case DCE2_POLICY__WINXP:
        case DCE2_POLICY__WINVISTA:
        case DCE2_POLICY__WIN2008:
        case DCE2_POLICY__WIN7:
            ut_node = DCE2_SmbFindUTNode(ssd, uid, tid);
            if (ut_node == NULL)
            {
                DCE2_SmbPipeTree *ptree = &ssd->sst.ptree;

                /* Need to create UT node */
                if ((ptree->ut_node.uid == DCE2_SENTINEL) && (ptree->ut_node.tid == DCE2_SENTINEL))
                {
                    ut_node = &ptree->ut_node;
                }
                else
                {
                    if (ptree->uts == NULL)
                    {
                        ptree->uts = DCE2_ListNew(DCE2_LIST_TYPE__SPLAYED, DCE2_SmbUTPtreeCompare,
                                                  DCE2_SmbUTDataFree, NULL, DCE2_LIST_FLAG__NO_DUPS,
                                                  DCE2_MEM_TYPE__SMB_UT);
                        if (ptree->uts == NULL)
                        {
                            DCE2_Free((void *)ft_node, sizeof(DCE2_SmbFidTrackerNode), DCE2_MEM_TYPE__SMB_FID);
                            PREPROC_PROFILE_END(dce2_pstat_smb_fid);
                            return;
                        }
                    }

                    ut_node = (DCE2_SmbUTNode *)DCE2_Alloc(sizeof(DCE2_SmbUTNode), DCE2_MEM_TYPE__SMB_UT);
                    if (ut_node == NULL)
                    {
                        PREPROC_PROFILE_END(dce2_pstat_smb_fid);
                        return;
                    }

                    if (DCE2_ListInsert(ptree->uts, (void *)(uintptr_t)((uid << 16) | tid),
                                        (void *)ut_node) != DCE2_RET__SUCCESS)
                    {
                        DCE2_Free((void *)ut_node, sizeof(DCE2_SmbUTNode), DCE2_MEM_TYPE__SMB_UT);
                        DCE2_Free((void *)ft_node, sizeof(DCE2_SmbFidTrackerNode), DCE2_MEM_TYPE__SMB_FID);
                        PREPROC_PROFILE_END(dce2_pstat_smb_fid);
                        return;
                    }
                }

                ut_node->uid = (int)uid;
                ut_node->tid = (int)tid;
            }

            ft_node->fid_node.fid = (int)fid;
            ft_node->fid_node.uid = (int)uid;
            ft_node->fid_node.tid = (int)tid;

            if (ut_node->fts == NULL)
            {
                ut_node->fts = DCE2_ListNew(DCE2_LIST_TYPE__SPLAYED, DCE2_SmbUTFCompare,
                                            DCE2_SmbFidTrackerDataFree, NULL,
                                            DCE2_LIST_FLAG__NO_DUPS, DCE2_MEM_TYPE__SMB_FID);

                if (ssd->sst.fts == NULL)
                {
                    DCE2_Free((void *)ft_node, sizeof(DCE2_SmbFidTrackerNode), DCE2_MEM_TYPE__SMB_FID);
                    PREPROC_PROFILE_END(dce2_pstat_smb_fid);
                    return;
                }
            }

            if (DCE2_ListInsert(ssd->sst.fts,
                                (void *)(uintptr_t)fid, (void *)ft_node) != DCE2_RET__SUCCESS)
            {
                DCE2_Free((void *)ft_node, sizeof(DCE2_SmbFidTrackerNode), DCE2_MEM_TYPE__SMB_FID);
                PREPROC_PROFILE_END(dce2_pstat_smb_fid);
                return;
            }

            break;

        default:
            DCE2_Log(DCE2_LOG_TYPE__ERROR,
                     "%s(%d) Invalid policy: %d",
                     __FILE__, __LINE__, policy);
            DCE2_Free((void *)ft_node, sizeof(DCE2_SmbFidTrackerNode), DCE2_MEM_TYPE__SMB_FID);
            break;
    }

    PREPROC_PROFILE_END(dce2_pstat_smb_fid);
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
static DCE2_SmbFidTrackerNode * DCE2_SmbFindFid(DCE2_SmbSsnData *ssd, const uint16_t uid,
                                                const uint16_t tid, const uint16_t fid)
{
    const DCE2_Policy policy = DCE2_ScPolicy(ssd->sd.sconfig);
    DCE2_SmbFidTrackerNode *ft_node = NULL;
    DCE2_SmbFidTrackerNode *save_ft_node = NULL;
    DCE2_SmbUTNode *ut_node;
    PROFILE_VARS;

    PREPROC_PROFILE_START(dce2_pstat_smb_fid);

    switch (policy)
    {
        case DCE2_POLICY__SAMBA:
        case DCE2_POLICY__SAMBA_3_0_37:
            /* Only uid used to create fid can be used to make a request */
            if ((ssd->sst.ft_node.fid_node.fid != DCE2_SENTINEL) &&
                (ssd->sst.ft_node.fid_node.uid != DCE2_SENTINEL) &&
                (ssd->sst.ft_node.fid_node.fid == (int)fid) &&
                (ssd->sst.ft_node.fid_node.uid == (int)uid))
            {
                ft_node = &ssd->sst.ft_node;
            }
            else if (ssd->sst.fts != NULL)
            {
                ft_node = (DCE2_SmbFidTrackerNode *)DCE2_ListFind(ssd->sst.fts, (void *)(uintptr_t)fid);
                if ((ft_node != NULL) && (ft_node->fid_node.uid != (int)uid))
                    ft_node = NULL;
            }

            PREPROC_PROFILE_END(dce2_pstat_smb_fid);

            return ft_node;

        case DCE2_POLICY__WIN2000:
        case DCE2_POLICY__SAMBA_3_0_20:
        case DCE2_POLICY__SAMBA_3_0_22:
            /* Any valid uid/tid can be used with any valid fid to make a request */

            /* Check for a valid uid */
            if (DCE2_SmbFindUid(ssd, uid) != DCE2_RET__SUCCESS)
            {
                PREPROC_PROFILE_END(dce2_pstat_smb_fid);
                return NULL;
            }

            if ((ssd->sst.ft_node.fid_node.fid != DCE2_SENTINEL) &&
                (ssd->sst.ft_node.fid_node.fid == (int)fid))
            {
                ft_node = &ssd->sst.ft_node;
            }
            else if (ssd->sst.fts != NULL)
            {
                ft_node = (DCE2_SmbFidTrackerNode *)DCE2_ListFind(ssd->sst.fts, (void *)(uintptr_t)fid);
            }

            if (ft_node == NULL)
            {
                PREPROC_PROFILE_END(dce2_pstat_smb_fid);
                return NULL;
            }

            /* Just return this fid node if we're not Win2000 or we've already 
             * used this fid once */
            if ((policy != DCE2_POLICY__WIN2000) || ft_node->used)
            {
                PREPROC_PROFILE_END(dce2_pstat_smb_fid);
                return ft_node;
            }

            /* Fall through for Windows 2000 */

            /* uid/tid binding of fid only occurs on first use in Windows 2000
             * We're going to check that the fid is bound, but use this fid */
            save_ft_node = ft_node;

            /* Reset fid node to NULL because of Windows 2000 fall through */
            ft_node = NULL;

        case DCE2_POLICY__WIN2003:
        case DCE2_POLICY__WINXP:
        case DCE2_POLICY__WINVISTA:
        case DCE2_POLICY__WIN2008:
        case DCE2_POLICY__WIN7:
            ut_node = DCE2_SmbFindUTNode(ssd, uid, tid);
            if (ut_node == NULL)
            {
                PREPROC_PROFILE_END(dce2_pstat_smb_fid);
                return NULL;
            }

            if ((ut_node->ft_node.fid_node.fid != DCE2_SENTINEL) &&
                (ut_node->ft_node.fid_node.fid == (int)fid))
            {
                ft_node = &ut_node->ft_node;
            }
            else if (ut_node->fts != NULL)
            {
                ft_node = (DCE2_SmbFidTrackerNode *)DCE2_ListFind(ut_node->fts, (void *)(uintptr_t)fid);
            }

            if (ft_node == NULL)
            {
                PREPROC_PROFILE_END(dce2_pstat_smb_fid);
                return NULL;
            }

            if (policy == DCE2_POLICY__WIN2000)
                ft_node = save_ft_node;

            PREPROC_PROFILE_END(dce2_pstat_smb_fid);

            return ft_node;

        default:
            break;
    }

    PREPROC_PROFILE_END(dce2_pstat_smb_fid);

    return NULL;
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
static void DCE2_SmbRemoveFid(DCE2_SmbSsnData *ssd, const uint16_t uid,
                              const uint16_t tid, const uint16_t fid)
{
    const DCE2_Policy policy = DCE2_ScPolicy(ssd->sd.sconfig);
    DCE2_SmbUTNode *ut_node;
    PROFILE_VARS;

    PREPROC_PROFILE_START(dce2_pstat_smb_fid);

    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Removing fid: 0x%04x\n", fid));

    switch (policy)
    {
        case DCE2_POLICY__WIN2000:
        case DCE2_POLICY__SAMBA_3_0_20:
        case DCE2_POLICY__SAMBA_3_0_22:
        case DCE2_POLICY__SAMBA_3_0_37:
        case DCE2_POLICY__SAMBA:
            if ((ssd->sst.ft_node.fid_node.fid != DCE2_SENTINEL) &&
                (ssd->sst.ft_node.fid_node.fid == (int)fid))
            {
                DCE2_SmbCleanFidNode(&ssd->sst.ft_node);
            }
            else if (ssd->sst.fts != NULL)
            {
                DCE2_ListRemove(ssd->sst.fts, (void *)(uintptr_t)fid);
            }

            if (policy != DCE2_POLICY__WIN2000)
            {
                PREPROC_PROFILE_END(dce2_pstat_smb_fid);
                return;
            }

            /* Fall through for Windows 2000 */

        case DCE2_POLICY__WIN2003:
        case DCE2_POLICY__WINXP:
        case DCE2_POLICY__WINVISTA:
        case DCE2_POLICY__WIN2008:
        case DCE2_POLICY__WIN7:
            ut_node = DCE2_SmbFindUTNode(ssd, uid, tid);
            if (ut_node == NULL)
            {
                PREPROC_PROFILE_END(dce2_pstat_smb_fid);
                return;
            }

            if ((ut_node->ft_node.fid_node.fid != DCE2_SENTINEL) &&
                (ut_node->ft_node.fid_node.fid == (int)fid))
            {
                DCE2_SmbCleanFidNode(&ut_node->ft_node);
            }
            else if (ut_node->fts != NULL)
            {
                DCE2_ListRemove(ut_node->fts, (void *)(uintptr_t)fid);
            }

            break;

        default:
            break;
    }

    PREPROC_PROFILE_END(dce2_pstat_smb_fid);
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
static INLINE void DCE2_SmbCleanFidNode(DCE2_SmbFidTrackerNode *ft_node)
{
    PROFILE_VARS;

    PREPROC_PROFILE_START(dce2_pstat_smb_fid);

    if (ft_node == NULL)
    {
        PREPROC_PROFILE_END(dce2_pstat_smb_fid);
        return;
    }

    ft_node->fid_node.uid = DCE2_SENTINEL;
    ft_node->fid_node.tid = DCE2_SENTINEL;
    ft_node->fid_node.fid = DCE2_SENTINEL;
    ft_node->used = 0;

    DCE2_CoCleanTracker(&ft_node->co_tracker);

    PREPROC_PROFILE_END(dce2_pstat_smb_fid);
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
static INLINE void DCE2_SmbCleanUTNode(DCE2_SmbUTNode *ut_node)
{
    if (ut_node == NULL)
        return;

    ut_node->uid = DCE2_SENTINEL;
    ut_node->tid = DCE2_SENTINEL;
    ut_node->ft_node.fid_node.uid = DCE2_SENTINEL;
    ut_node->ft_node.fid_node.tid = DCE2_SENTINEL;
    ut_node->ft_node.fid_node.fid = DCE2_SENTINEL;
    ut_node->ft_node.used = 0;

    DCE2_CoCleanTracker(&ut_node->ft_node.co_tracker);

    if (ut_node->fts != NULL)
    {
        DCE2_ListDestroy(ut_node->fts);
        ut_node->fts = NULL;
    }
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
static INLINE void DCE2_SmbCleanPMNode(DCE2_SmbPMNode *pm_node)
{
    PROFILE_VARS;

    PREPROC_PROFILE_START(dce2_pstat_smb_trans);

    if (pm_node == NULL)
    {
        PREPROC_PROFILE_END(dce2_pstat_smb_trans);
        return;
    }

    if ((pm_node->pid == DCE2_SENTINEL) && (pm_node->mid == DCE2_SENTINEL))
    {
        PREPROC_PROFILE_END(dce2_pstat_smb_trans);
        return;
    }

    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Cleaning pm_node - "
                   "pid: %u, mid: %u, fid: 0x%04x\n",
                   pm_node->pid, pm_node->mid, pm_node->fid_node.fid));

    pm_node->pid = DCE2_SENTINEL;
    pm_node->mid = DCE2_SENTINEL;
    pm_node->fid_node.uid = DCE2_SENTINEL;
    pm_node->fid_node.tid = DCE2_SENTINEL;
    pm_node->fid_node.fid = DCE2_SENTINEL;

    if (pm_node->buf != NULL)
    {
        DCE2_BufferDestroy(pm_node->buf);
        pm_node->buf = NULL;
    }

    PREPROC_PROFILE_END(dce2_pstat_smb_trans);
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
static int DCE2_SmbUTFCompare(const void *a, const void *b)
{
    int x = (int)(uintptr_t)a;
    int y = (int)(uintptr_t)b;

    if (x == y)
        return 0;

    /* Only care about equality for finding */
    return -1;
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
static int DCE2_SmbPMCompare(const void *a, const void *b)
{
    DCE2_SmbPMNode *pm_a = (DCE2_SmbPMNode *)a;
    DCE2_SmbPMNode *pm_b = (DCE2_SmbPMNode *)b;

    if ((a == NULL) || (b == NULL))
    {
        DCE2_Log(DCE2_LOG_TYPE__ERROR,
                 "%s(%d) Key passed in was NULL.", __FILE__, __LINE__);

        return -1;
    }

    if (pm_a->policy != pm_b->policy)
    {
        DCE2_Log(DCE2_LOG_TYPE__ERROR,
                 "%s(%d) Comparing 2 nodes with different "
                 "policies: %d <=> %d.", __FILE__, __LINE__,
                 pm_a->policy, pm_b->policy);

        return -1;
    }

    switch (pm_a->policy)
    {
        case DCE2_POLICY__SAMBA_3_0_20:
        case DCE2_POLICY__SAMBA_3_0_22:
            /* These versions of Samba don't care */
            return 0;

        case DCE2_POLICY__SAMBA:
        case DCE2_POLICY__SAMBA_3_0_37:
            /* Only uses mid */
            if (pm_a->mid == pm_b->mid)
                return 0;

            break;

        case DCE2_POLICY__WIN2000:
        case DCE2_POLICY__WIN2003:
        case DCE2_POLICY__WINXP:
        case DCE2_POLICY__WINVISTA:
        case DCE2_POLICY__WIN2008:
        case DCE2_POLICY__WIN7:
            /* Uses both pid and mid */
            if ((pm_a->pid == pm_b->pid) && (pm_a->mid == pm_b->mid))
                return 0;

            break;

        default:
            DCE2_Log(DCE2_LOG_TYPE__ERROR,
                     "%s(%d) Invalid policy: %d",
                     __FILE__, __LINE__, pm_a->policy);
            return -1;
    }

    /* Only care about equality for finding */
    return -1;
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
static int DCE2_SmbUTPtreeCompare(const void *a, const void *b)
{
    uintptr_t x = (uintptr_t)a;
    uintptr_t y = (uintptr_t)b;
    uint16_t uidx, tidx;
    uint16_t uidy, tidy;

    uidx = (uint16_t)(x >> 16);
    tidx = (uint16_t)(x & 0x0000ffff);

    uidy = (uint16_t)(y >> 16);
    tidy = (uint16_t)(y & 0x0000ffff);

    if ((uidx == 0) || (uidy == 0))
    {
        if (tidx == tidy)
            return 0;
    }
    else if ((tidx == 0) || (tidy == 0))
    {
        if (uidx == uidy)
            return 0;
    }
    else
    {
        if ((uidx == uidy) && (tidx == tidy))
            return 0;
    }

    /* Only care about equality for finding */
    return -1;
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
static INLINE void DCE2_SmbResetForMissedPkts(DCE2_SmbSsnData *ssd)
{
    if (ssd == NULL)
        return;

    DCE2_BufferEmpty(ssd->cli_seg.buf);
    DCE2_BufferEmpty(ssd->srv_seg.buf);

    ssd->req_uid = DCE2_SENTINEL;
    ssd->req_tid = DCE2_SENTINEL;

    if (ssd->br.fid_node.fid != DCE2_SENTINEL)
    {
        ssd->br.fid_node.uid = DCE2_SENTINEL;
        ssd->br.fid_node.tid = DCE2_SENTINEL;
        ssd->br.fid_node.fid = DCE2_SENTINEL;
    }

    if (!DCE2_CQueueIsEmpty(ssd->tc_queue))
        DCE2_CQueueEmpty(ssd->tc_queue);

    if (ssd->read_fid_node.fid != DCE2_SENTINEL)
    {
        ssd->read_fid_node.uid = DCE2_SENTINEL;
        ssd->read_fid_node.tid = DCE2_SENTINEL;
        ssd->read_fid_node.fid = DCE2_SENTINEL;
    }

    DCE2_CQueueEmpty(ssd->read_fid_queue);
    DCE2_SmbCleanPMNode(&ssd->pm_node);
    DCE2_ListEmpty(ssd->pms);
    DCE2_QueueEmpty(ssd->ft_queue);
    DCE2_SmbSetMissedFids(ssd);
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
void DCE2_SmbDataFree(DCE2_SmbSsnData *ssd)
{
    if (ssd == NULL)
        return;

    DCE2_CoCleanTracker(&ssd->sst.ptree.ut_node.ft_node.co_tracker);
    DCE2_CoCleanTracker(&ssd->sst.ft_node.co_tracker);

    if (ssd->sst.ptree.ut_node.fts != NULL)
    {
        DCE2_ListDestroy(ssd->sst.ptree.ut_node.fts);
        ssd->sst.ptree.ut_node.fts = NULL;
    }

    if (ssd->sst.ptree.uts != NULL)
    {
        DCE2_ListDestroy(ssd->sst.ptree.uts);
        ssd->sst.ptree.uts = NULL;
    }

    if (ssd->sst.uids != NULL)
    {
        DCE2_ListDestroy(ssd->sst.uids);
        ssd->sst.uids = NULL;
    }

    if (ssd->sst.tids != NULL)
    {
        DCE2_ListDestroy(ssd->sst.tids);
        ssd->sst.tids = NULL;
    }

    if (ssd->sst.fts != NULL)
    {
        DCE2_ListDestroy(ssd->sst.fts);
        ssd->sst.fts = NULL;
    }

    DCE2_SmbCleanPMNode(&ssd->pm_node);
    if (ssd->pms != NULL)
    {
        DCE2_ListDestroy(ssd->pms);
        ssd->pms = NULL;
    }

    if (ssd->cli_seg.buf != NULL)
    {
        DCE2_BufferDestroy(ssd->cli_seg.buf);
        ssd->cli_seg.buf = NULL;
    }

    if (ssd->srv_seg.buf != NULL)
    {
        DCE2_BufferDestroy(ssd->srv_seg.buf);
        ssd->srv_seg.buf = NULL;
    }

    if (ssd->tc_queue != NULL)
    {
        DCE2_CQueueDestroy(ssd->tc_queue);
        ssd->tc_queue = NULL;
    }

    if (ssd->read_fid_queue != NULL)
    {
        DCE2_CQueueDestroy(ssd->read_fid_queue);
        ssd->read_fid_queue = NULL;
    }

    if (ssd->ft_queue != NULL)
    {
        DCE2_QueueDestroy(ssd->ft_queue);
        ssd->ft_queue = NULL;
    }
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
void DCE2_SmbSsnFree(void *ssn)
{
    DCE2_SmbSsnData *ssd = (DCE2_SmbSsnData *)ssn;

    if (ssd == NULL)
        return;

    DCE2_SmbDataFree(ssd);
    DCE2_Free((void *)ssn, sizeof(DCE2_SmbSsnData), DCE2_MEM_TYPE__SMB_SSN);
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
static void DCE2_SmbFidDataFree(void *data)
{
    DCE2_SmbFidNode *fid_node = (DCE2_SmbFidNode *)data;

    if (fid_node == NULL)
        return;

    DCE2_Free((void *)fid_node, sizeof(DCE2_SmbFidNode), DCE2_MEM_TYPE__SMB_FID);
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
static void DCE2_SmbFidTrackerDataFree(void *data)
{
    DCE2_SmbFidTrackerNode *ft_node = (DCE2_SmbFidTrackerNode *)data;

    if (ft_node == NULL)
        return;

    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Freeing fid: 0x%04x\n", ft_node->fid_node.fid));

    DCE2_CoCleanTracker(&ft_node->co_tracker);
    DCE2_Free((void *)ft_node, sizeof(DCE2_SmbFidTrackerNode), DCE2_MEM_TYPE__SMB_FID);
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
static void DCE2_SmbUTDataFree(void *data)
{
    DCE2_SmbUTNode *ut_node = (DCE2_SmbUTNode *)data;

    if (ut_node == NULL)
        return;

    DCE2_SmbCleanUTNode(ut_node);
    DCE2_Free((void *)ut_node, sizeof(DCE2_SmbUTNode), DCE2_MEM_TYPE__SMB_UT);
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
static void DCE2_SmbPMDataFree(void *data)
{
    DCE2_SmbPMNode *pm_node = (DCE2_SmbPMNode *)data;

    if (pm_node == NULL)
        return;

    DCE2_SmbCleanPMNode(pm_node);
    DCE2_Free((void *)pm_node, sizeof(DCE2_SmbPMNode), DCE2_MEM_TYPE__SMB_PM);
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
static void DCE2_SmbIncComStat(const SmbNtHdr *smb_hdr)
{
    uint8_t smb_com = SmbCom(smb_hdr);
    int smb_type = SmbType(smb_hdr);

    switch (smb_com)
    {
        case SMB_COM_SESS_SETUP_ANDX:
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Session Setup AndX\n"));
            if (smb_type == SMB_TYPE__REQUEST)
                dce2_stats.smb_ssx_req++;
            else
                dce2_stats.smb_ssx_resp++;
            break;

        case SMB_COM_LOGOFF_ANDX:
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Logoff AndX\n"));
            if (smb_type == SMB_TYPE__REQUEST)
                dce2_stats.smb_loffx_req++;
            else
                dce2_stats.smb_loffx_resp++;
            break;

        case SMB_COM_TREE_CON:
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Tree Connect\n"));
            if (smb_type == SMB_TYPE__REQUEST)
                dce2_stats.smb_tc_req++;
            else
                dce2_stats.smb_tc_resp++;
            break;

        case SMB_COM_TREE_CON_ANDX:
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Tree Connect AndX\n"));
            if (smb_type == SMB_TYPE__REQUEST)
                dce2_stats.smb_tcx_req++;
            else
                dce2_stats.smb_tcx_resp++;
            break;

        case SMB_COM_TREE_DIS:
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Tree Disconnect\n"));
            if (smb_type == SMB_TYPE__REQUEST)
                dce2_stats.smb_tdis_req++;
            else
                dce2_stats.smb_tdis_resp++;
            break;

        case SMB_COM_OPEN:
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Open\n"));
            if (smb_type == SMB_TYPE__REQUEST)
                dce2_stats.smb_open_req++;
            else
                dce2_stats.smb_open_resp++;
            break;

        case SMB_COM_OPEN_ANDX:
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Open AndX\n"));
            if (smb_type == SMB_TYPE__REQUEST)
                dce2_stats.smb_openx_req++;
            else
                dce2_stats.smb_openx_resp++;
            break;

        case SMB_COM_NT_CREATE_ANDX:
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Nt Create AndX\n"));
            if (smb_type == SMB_TYPE__REQUEST)
                dce2_stats.smb_ntcx_req++;
            else
                dce2_stats.smb_ntcx_resp++;
            break;

        case SMB_COM_CLOSE:
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Close\n"));
            if (smb_type == SMB_TYPE__REQUEST)
                dce2_stats.smb_close_req++;
            else
                dce2_stats.smb_close_resp++;
            break;

        case SMB_COM_WRITE:
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Write\n"));
            if (smb_type == SMB_TYPE__REQUEST)
                dce2_stats.smb_write_req++;
            else
                dce2_stats.smb_write_resp++;
            break;

        case SMB_COM_WRITE_BLOCK_RAW:
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Write Block Raw\n"));
            if (smb_type == SMB_TYPE__REQUEST)
                dce2_stats.smb_writebr_req++;
            else
                dce2_stats.smb_writebr_resp++;
            break;

        case SMB_COM_WRITE_ANDX:
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Write AndX\n"));
            if (smb_type == SMB_TYPE__REQUEST)
                dce2_stats.smb_writex_req++;
            else
                dce2_stats.smb_writex_resp++;
            break;

        case SMB_COM_WRITE_AND_CLOSE:
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Write and Close\n"));
            if (smb_type == SMB_TYPE__REQUEST)
                dce2_stats.smb_writeclose_req++;
            else
                dce2_stats.smb_writeclose_resp++;
            break;

        case SMB_COM_WRITE_COMPLETE:
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Write Complete\n"));
            if (smb_type == SMB_TYPE__RESPONSE)
                dce2_stats.smb_writecomplete_resp++;
            break;

        case SMB_COM_TRANS:
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Transaction\n"));
            if (smb_type == SMB_TYPE__REQUEST)
                dce2_stats.smb_trans_req++;
            else
                dce2_stats.smb_trans_resp++;
            break;

        case SMB_COM_TRANS_SEC:
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Transaction Secondary\n"));
            if (smb_type == SMB_TYPE__REQUEST)
                dce2_stats.smb_trans_sec_req++;
            /* Shouldn't get a server response of this type */
            break;

        case SMB_COM_READ:
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Read\n"));
            if (smb_type == SMB_TYPE__REQUEST)
                dce2_stats.smb_read_req++;
            else
                dce2_stats.smb_read_resp++;
            break;

        case SMB_COM_READ_BLOCK_RAW:
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Read Block Raw\n"));
            if (smb_type == SMB_TYPE__REQUEST)
                dce2_stats.smb_readbr_req++;
            /* Shouldn't get a server response of this type */
            break;

        case SMB_COM_READ_ANDX:
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Read AndX\n"));
            if (smb_type == SMB_TYPE__REQUEST)
                dce2_stats.smb_readx_req++;
            else
                dce2_stats.smb_readx_resp++;
            break;

        case SMB_COM_RENAME:
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Rename\n"));
            if (smb_type == SMB_TYPE__REQUEST)
                dce2_stats.smb_rename_req++;
            else
                dce2_stats.smb_rename_resp++;
            break;

        default:
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "SMB command: %02x\n", smb_com));
            if (smb_type == SMB_TYPE__REQUEST)
                dce2_stats.smb_other_req++;
            else
                dce2_stats.smb_other_resp++;
            break;
    }
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
static void DCE2_SmbIncChainedStat(const SmbNtHdr *smb_hdr, const int smb_com, const SmbAndXCommon *andx)
{
    const int smb_type = SmbType(smb_hdr);
    const int smb_com2 = SmbAndXCom2(andx);

    switch (smb_com)
    {
        case SMB_COM_SESS_SETUP_ANDX:
            dce2_stats.smb_ssx_chained++;
            switch (smb_com2)
            {
                case SMB_COM_LOGOFF_ANDX:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_ssx_req_chained_loffx++;
                        dce2_stats.smb_loffx_req++;
                    }
                    else
                    {
                        dce2_stats.smb_ssx_resp_chained_loffx++;
                        dce2_stats.smb_loffx_resp++;
                    }

                    break;

                case SMB_COM_TREE_CON:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_ssx_req_chained_tc++;
                        dce2_stats.smb_tc_req++;
                    }
                    else
                    {
                        dce2_stats.smb_ssx_resp_chained_tc++;
                        dce2_stats.smb_tc_resp++;
                    }

                    break;

                case SMB_COM_TREE_CON_ANDX:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_ssx_req_chained_tcx++;
                        dce2_stats.smb_tcx_req++;
                    }
                    else
                    {
                        dce2_stats.smb_ssx_resp_chained_tcx++;
                        dce2_stats.smb_tcx_resp++;
                    }

                    break;

                case SMB_COM_TREE_DIS:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_ssx_req_chained_tdis++;
                        dce2_stats.smb_tdis_req++;
                    }
                    else
                    {
                        dce2_stats.smb_ssx_resp_chained_tdis++;
                        dce2_stats.smb_tdis_resp++;
                    }

                    break;

                case SMB_COM_OPEN:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_ssx_req_chained_open++;
                        dce2_stats.smb_open_req++;
                    }
                    else
                    {
                        dce2_stats.smb_ssx_resp_chained_open++;
                        dce2_stats.smb_open_resp++;
                    }

                    break;

                case SMB_COM_OPEN_ANDX:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_ssx_req_chained_openx++;
                        dce2_stats.smb_openx_req++;
                    }
                    else
                    {
                        dce2_stats.smb_ssx_resp_chained_openx++;
                        dce2_stats.smb_openx_resp++;
                    }

                    break;

                case SMB_COM_NT_CREATE_ANDX:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_ssx_req_chained_ntcx++;
                        dce2_stats.smb_ntcx_req++;
                    }
                    else
                    {
                        dce2_stats.smb_ssx_resp_chained_ntcx++;
                        dce2_stats.smb_ntcx_resp++;
                    }

                    break;

                case SMB_COM_CLOSE:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_ssx_req_chained_close++;
                        dce2_stats.smb_close_req++;
                    }
                    else
                    {
                        dce2_stats.smb_ssx_resp_chained_close++;
                        dce2_stats.smb_close_resp++;
                    }

                    break;

                case SMB_COM_TRANS:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_ssx_req_chained_trans++;
                        dce2_stats.smb_trans_req++;
                    }
                    else
                    {
                        dce2_stats.smb_ssx_resp_chained_trans++;
                        dce2_stats.smb_trans_resp++;
                    }

                    break;

                case SMB_COM_WRITE:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_ssx_req_chained_write++;
                        dce2_stats.smb_write_req++;
                    }
                    else
                    {
                        dce2_stats.smb_ssx_resp_chained_write++;
                        dce2_stats.smb_write_resp++;
                    }

                    break;

                case SMB_COM_READ_ANDX:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_ssx_req_chained_readx++;
                        dce2_stats.smb_readx_req++;
                    }
                    else
                    {
                        dce2_stats.smb_ssx_resp_chained_readx++;
                        dce2_stats.smb_readx_resp++;
                    }

                    break;

                default:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_ssx_req_chained_other++;
                        dce2_stats.smb_other_req++;
                    }
                    else
                    {
                        dce2_stats.smb_ssx_resp_chained_other++;
                        dce2_stats.smb_other_resp++;
                    }

                    break;
            }

            break;

        case SMB_COM_LOGOFF_ANDX:
            dce2_stats.smb_loffx_chained++;
            switch (smb_com2)
            {
                case SMB_COM_SESS_SETUP_ANDX:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_loffx_req_chained_ssx++;
                        dce2_stats.smb_ssx_req++;
                    }
                    else
                    {
                        dce2_stats.smb_loffx_resp_chained_ssx++;
                        dce2_stats.smb_ssx_resp++;
                    }

                    break;

                case SMB_COM_TREE_CON_ANDX:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_loffx_req_chained_tcx++;
                        dce2_stats.smb_tcx_req++;
                    }
                    else
                    {
                        dce2_stats.smb_loffx_resp_chained_tcx++;
                        dce2_stats.smb_tcx_resp++;
                    }

                    break;

                case SMB_COM_TREE_DIS:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_loffx_req_chained_tdis++;
                        dce2_stats.smb_tdis_req++;
                    }
                    else
                    {
                        dce2_stats.smb_loffx_resp_chained_tdis++;
                        dce2_stats.smb_tdis_resp++;
                    }

                    break;

                default:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_loffx_req_chained_other++;
                        dce2_stats.smb_other_req++;
                    }
                    else
                    {
                        dce2_stats.smb_loffx_resp_chained_other++;
                        dce2_stats.smb_other_resp++;
                    }

                    break;
            }

            break;

        case SMB_COM_TREE_CON_ANDX:
            dce2_stats.smb_tcx_chained++;
            switch (smb_com2)
            {
                case SMB_COM_SESS_SETUP_ANDX:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_tcx_req_chained_ssx++;
                        dce2_stats.smb_ssx_req++;
                    }
                    else
                    {
                        dce2_stats.smb_tcx_resp_chained_ssx++;
                        dce2_stats.smb_ssx_resp++;
                    }

                    break;

                case SMB_COM_LOGOFF_ANDX:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_tcx_req_chained_loffx++;
                        dce2_stats.smb_loffx_req++;
                    }
                    else
                    {
                        dce2_stats.smb_tcx_resp_chained_loffx++;
                        dce2_stats.smb_loffx_resp++;
                    }

                    break;

                case SMB_COM_TREE_DIS:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_tcx_req_chained_tdis++;
                        dce2_stats.smb_tdis_req++;
                    }
                    else
                    {
                        dce2_stats.smb_tcx_resp_chained_tdis++;
                        dce2_stats.smb_tdis_resp++;
                    }

                    break;

                case SMB_COM_OPEN:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_tcx_req_chained_open++;
                        dce2_stats.smb_open_req++;
                    }
                    else
                    {
                        dce2_stats.smb_tcx_resp_chained_open++;
                        dce2_stats.smb_open_resp++;
                    }

                    break;

                case SMB_COM_OPEN_ANDX:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_tcx_req_chained_openx++;
                        dce2_stats.smb_openx_req++;
                    }
                    else
                    {
                        dce2_stats.smb_tcx_resp_chained_openx++;
                        dce2_stats.smb_openx_resp++;
                    }

                    break;

                case SMB_COM_NT_CREATE_ANDX:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_tcx_req_chained_ntcx++;
                        dce2_stats.smb_ntcx_req++;
                    }
                    else
                    {
                        dce2_stats.smb_tcx_resp_chained_ntcx++;
                        dce2_stats.smb_ntcx_resp++;
                    }

                    break;

                case SMB_COM_CLOSE:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_tcx_req_chained_close++;
                        dce2_stats.smb_close_req++;
                    }
                    else
                    {
                        dce2_stats.smb_tcx_resp_chained_close++;
                        dce2_stats.smb_close_resp++;
                    }

                    break;

                case SMB_COM_TRANS:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_tcx_req_chained_trans++;
                        dce2_stats.smb_trans_req++;
                    }
                    else
                    {
                        dce2_stats.smb_tcx_resp_chained_trans++;
                        dce2_stats.smb_trans_resp++;
                    }

                    break;

                case SMB_COM_WRITE:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_tcx_req_chained_write++;
                        dce2_stats.smb_write_req++;
                    }
                    else
                    {
                        dce2_stats.smb_tcx_resp_chained_write++;
                        dce2_stats.smb_write_resp++;
                    }

                    break;

                case SMB_COM_READ_ANDX:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_tcx_req_chained_readx++;
                        dce2_stats.smb_readx_req++;
                    }
                    else
                    {
                        dce2_stats.smb_tcx_resp_chained_readx++;
                        dce2_stats.smb_readx_resp++;
                    }

                    break;

                default:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_tcx_req_chained_other++;
                        dce2_stats.smb_other_req++;
                    }
                    else
                    {
                        dce2_stats.smb_tcx_resp_chained_other++;
                        dce2_stats.smb_other_resp++;
                    }

                    break;
            }

            break;

        case SMB_COM_OPEN_ANDX:
            dce2_stats.smb_openx_chained++;
            switch (smb_com2)
            {
                case SMB_COM_SESS_SETUP_ANDX:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_openx_req_chained_ssx++;
                        dce2_stats.smb_ssx_req++;
                    }
                    else
                    {
                        dce2_stats.smb_openx_resp_chained_ssx++;
                        dce2_stats.smb_ssx_resp++;
                    }

                    break;

                case SMB_COM_LOGOFF_ANDX:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_openx_req_chained_loffx++;
                        dce2_stats.smb_loffx_req++;
                    }
                    else
                    {
                        dce2_stats.smb_openx_resp_chained_loffx++;
                        dce2_stats.smb_loffx_resp++;
                    }

                    break;

                case SMB_COM_TREE_CON:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_openx_req_chained_tc++;
                        dce2_stats.smb_tc_req++;
                    }
                    else
                    {
                        dce2_stats.smb_openx_resp_chained_tc++;
                        dce2_stats.smb_tc_resp++;
                    }

                    break;

                case SMB_COM_TREE_CON_ANDX:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_openx_req_chained_tcx++;
                        dce2_stats.smb_tcx_req++;
                    }
                    else
                    {
                        dce2_stats.smb_openx_resp_chained_tcx++;
                        dce2_stats.smb_tcx_resp++;
                    }

                    break;

                case SMB_COM_TREE_DIS:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_openx_req_chained_tdis++;
                        dce2_stats.smb_tdis_req++;
                    }
                    else
                    {
                        dce2_stats.smb_openx_resp_chained_tdis++;
                        dce2_stats.smb_tdis_resp++;
                    }

                    break;

                case SMB_COM_OPEN_ANDX:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_openx_req_chained_openx++;
                        dce2_stats.smb_openx_req++;
                    }
                    else
                    {
                        dce2_stats.smb_openx_resp_chained_openx++;
                        dce2_stats.smb_openx_resp++;
                    }

                    break;

                case SMB_COM_NT_CREATE_ANDX:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_openx_req_chained_ntcx++;
                        dce2_stats.smb_ntcx_req++;
                    }
                    else
                    {
                        dce2_stats.smb_openx_resp_chained_ntcx++;
                        dce2_stats.smb_ntcx_resp++;
                    }

                    break;

                case SMB_COM_CLOSE:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_openx_req_chained_close++;
                        dce2_stats.smb_close_req++;
                    }
                    else
                    {
                        dce2_stats.smb_openx_resp_chained_close++;
                        dce2_stats.smb_close_resp++;
                    }

                    break;

                case SMB_COM_WRITE:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_openx_req_chained_write++;
                        dce2_stats.smb_write_req++;
                    }
                    else
                    {
                        dce2_stats.smb_openx_resp_chained_write++;
                        dce2_stats.smb_write_resp++;
                    }

                    break;

                case SMB_COM_READ_ANDX:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_openx_req_chained_readx++;
                        dce2_stats.smb_readx_req++;
                    }
                    else
                    {
                        dce2_stats.smb_openx_resp_chained_readx++;
                        dce2_stats.smb_readx_resp++;
                    }

                    break;

                default:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_openx_req_chained_other++;
                        dce2_stats.smb_other_req++;
                    }
                    else
                    {
                        dce2_stats.smb_openx_resp_chained_other++;
                        dce2_stats.smb_other_resp++;
                    }

                    break;
            }

            break;

        case SMB_COM_NT_CREATE_ANDX:
            dce2_stats.smb_ntcx_chained++;
            switch (smb_com2)
            {
                case SMB_COM_SESS_SETUP_ANDX:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_ntcx_req_chained_ssx++;
                        dce2_stats.smb_ssx_req++;
                    }
                    else
                    {
                        dce2_stats.smb_ntcx_resp_chained_ssx++;
                        dce2_stats.smb_ssx_resp++;
                    }

                    break;

                case SMB_COM_LOGOFF_ANDX:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_ntcx_req_chained_loffx++;
                        dce2_stats.smb_loffx_req++;
                    }
                    else
                    {
                        dce2_stats.smb_ntcx_resp_chained_loffx++;
                        dce2_stats.smb_loffx_resp++;
                    }

                    break;

                case SMB_COM_TREE_CON:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_ntcx_req_chained_tc++;
                        dce2_stats.smb_tc_req++;
                    }
                    else
                    {
                        dce2_stats.smb_ntcx_resp_chained_tc++;
                        dce2_stats.smb_tc_resp++;
                    }

                    break;

                case SMB_COM_TREE_CON_ANDX:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_ntcx_req_chained_tcx++;
                        dce2_stats.smb_tcx_req++;
                    }
                    else
                    {
                        dce2_stats.smb_ntcx_resp_chained_tcx++;
                        dce2_stats.smb_tcx_resp++;
                    }

                    break;

                case SMB_COM_TREE_DIS:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_ntcx_req_chained_tdis++;
                        dce2_stats.smb_tdis_req++;
                    }
                    else
                    {
                        dce2_stats.smb_ntcx_resp_chained_tdis++;
                        dce2_stats.smb_tdis_resp++;
                    }

                    break;

                case SMB_COM_OPEN_ANDX:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_ntcx_req_chained_openx++;
                        dce2_stats.smb_openx_req++;
                    }
                    else
                    {
                        dce2_stats.smb_ntcx_resp_chained_openx++;
                        dce2_stats.smb_openx_resp++;
                    }

                    break;

                case SMB_COM_NT_CREATE_ANDX:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_ntcx_req_chained_ntcx++;
                        dce2_stats.smb_ntcx_req++;
                    }
                    else
                    {
                        dce2_stats.smb_ntcx_resp_chained_ntcx++;
                        dce2_stats.smb_ntcx_resp++;
                    }

                    break;

                case SMB_COM_CLOSE:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_ntcx_req_chained_close++;
                        dce2_stats.smb_close_req++;
                    }
                    else
                    {
                        dce2_stats.smb_ntcx_resp_chained_close++;
                        dce2_stats.smb_close_resp++;
                    }

                    break;

                case SMB_COM_WRITE:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_ntcx_req_chained_write++;
                        dce2_stats.smb_write_req++;
                    }
                    else
                    {
                        dce2_stats.smb_ntcx_resp_chained_write++;
                        dce2_stats.smb_write_resp++;
                    }

                    break;

                case SMB_COM_READ_ANDX:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_ntcx_req_chained_readx++;
                        dce2_stats.smb_readx_req++;
                    }
                    else
                    {
                        dce2_stats.smb_ntcx_resp_chained_readx++;
                        dce2_stats.smb_readx_resp++;
                    }

                    break;

                default:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_ntcx_req_chained_other++;
                        dce2_stats.smb_other_req++;
                    }
                    else
                    {
                        dce2_stats.smb_ntcx_resp_chained_other++;
                        dce2_stats.smb_other_resp++;
                    }

                    break;
            }

            break;

        case SMB_COM_WRITE_ANDX:
            dce2_stats.smb_writex_chained++;
            switch (smb_com2)
            {
                case SMB_COM_SESS_SETUP_ANDX:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_writex_req_chained_ssx++;
                        dce2_stats.smb_ssx_req++;
                    }
                    else
                    {
                        dce2_stats.smb_writex_resp_chained_ssx++;
                        dce2_stats.smb_ssx_resp++;
                    }

                    break;

                case SMB_COM_LOGOFF_ANDX:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_writex_req_chained_loffx++;
                        dce2_stats.smb_loffx_req++;
                    }
                    else
                    {
                        dce2_stats.smb_writex_resp_chained_loffx++;
                        dce2_stats.smb_loffx_resp++;
                    }

                    break;

                case SMB_COM_TREE_CON:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_writex_req_chained_tc++;
                        dce2_stats.smb_tc_req++;
                    }
                    else
                    {
                        dce2_stats.smb_writex_resp_chained_tc++;
                        dce2_stats.smb_tc_resp++;
                    }

                    break;

                case SMB_COM_TREE_CON_ANDX:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_writex_req_chained_tcx++;
                        dce2_stats.smb_tcx_req++;
                    }
                    else
                    {
                        dce2_stats.smb_writex_resp_chained_tcx++;
                        dce2_stats.smb_tcx_resp++;
                    }

                    break;

                case SMB_COM_OPEN_ANDX:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_writex_req_chained_openx++;
                        dce2_stats.smb_openx_req++;
                    }
                    else
                    {
                        dce2_stats.smb_writex_resp_chained_openx++;
                        dce2_stats.smb_openx_resp++;
                    }

                    break;

                case SMB_COM_NT_CREATE_ANDX:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_writex_req_chained_ntcx++;
                        dce2_stats.smb_ntcx_req++;
                    }
                    else
                    {
                        dce2_stats.smb_writex_resp_chained_ntcx++;
                        dce2_stats.smb_ntcx_resp++;
                    }

                    break;

                case SMB_COM_CLOSE:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_writex_req_chained_close++;
                        dce2_stats.smb_close_req++;
                    }
                    else
                    {
                        dce2_stats.smb_writex_resp_chained_close++;
                        dce2_stats.smb_close_resp++;
                    }

                    break;

                case SMB_COM_WRITE_ANDX:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_writex_req_chained_writex++;
                        dce2_stats.smb_writex_req++;
                    }
                    else
                    {
                        dce2_stats.smb_writex_resp_chained_writex++;
                        dce2_stats.smb_writex_resp++;
                    }

                    break;

                case SMB_COM_WRITE:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_writex_req_chained_write++;
                        dce2_stats.smb_write_req++;
                    }
                    else
                    {
                        dce2_stats.smb_writex_resp_chained_write++;
                        dce2_stats.smb_write_resp++;
                    }

                    break;

                case SMB_COM_READ:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_writex_req_chained_read++;
                        dce2_stats.smb_read_req++;
                    }
                    else
                    {
                        dce2_stats.smb_writex_resp_chained_read++;
                        dce2_stats.smb_read_resp++;
                    }

                    break;

                case SMB_COM_READ_ANDX:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_writex_req_chained_readx++;
                        dce2_stats.smb_readx_req++;
                    }
                    else
                    {
                        dce2_stats.smb_writex_resp_chained_readx++;
                        dce2_stats.smb_readx_resp++;
                    }

                    break;

                default:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_writex_req_chained_other++;
                        dce2_stats.smb_other_req++;
                    }
                    else
                    {
                        dce2_stats.smb_writex_resp_chained_other++;
                        dce2_stats.smb_other_resp++;
                    }

                    break;
            }

            break;

        case SMB_COM_READ_ANDX:
            dce2_stats.smb_readx_chained++;
            switch (smb_com2)
            {
                case SMB_COM_SESS_SETUP_ANDX:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_readx_req_chained_ssx++;
                        dce2_stats.smb_ssx_req++;
                    }
                    else
                    {
                        dce2_stats.smb_readx_resp_chained_ssx++;
                        dce2_stats.smb_ssx_resp++;
                    }

                    break;

                case SMB_COM_LOGOFF_ANDX:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_readx_req_chained_loffx++;
                        dce2_stats.smb_loffx_req++;
                    }
                    else
                    {
                        dce2_stats.smb_readx_resp_chained_loffx++;
                        dce2_stats.smb_loffx_resp++;
                    }

                    break;

                case SMB_COM_TREE_CON:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_readx_req_chained_tc++;
                        dce2_stats.smb_tc_req++;
                    }
                    else
                    {
                        dce2_stats.smb_readx_resp_chained_tc++;
                        dce2_stats.smb_tc_resp++;
                    }

                    break;

                case SMB_COM_TREE_CON_ANDX:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_readx_req_chained_tcx++;
                        dce2_stats.smb_tcx_req++;
                    }
                    else
                    {
                        dce2_stats.smb_readx_resp_chained_tcx++;
                        dce2_stats.smb_tcx_resp++;
                    }

                    break;

                case SMB_COM_TREE_DIS:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_readx_req_chained_tdis++;
                        dce2_stats.smb_tdis_req++;
                    }
                    else
                    {
                        dce2_stats.smb_readx_resp_chained_tdis++;
                        dce2_stats.smb_tdis_resp++;
                    }

                    break;

                case SMB_COM_OPEN_ANDX:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_readx_req_chained_openx++;
                        dce2_stats.smb_openx_req++;
                    }
                    else
                    {
                        dce2_stats.smb_readx_resp_chained_openx++;
                        dce2_stats.smb_openx_resp++;
                    }

                    break;

                case SMB_COM_NT_CREATE_ANDX:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_readx_req_chained_ntcx++;
                        dce2_stats.smb_ntcx_req++;
                    }
                    else
                    {
                        dce2_stats.smb_readx_resp_chained_ntcx++;
                        dce2_stats.smb_ntcx_resp++;
                    }

                    break;

                case SMB_COM_CLOSE:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_readx_req_chained_close++;
                        dce2_stats.smb_close_req++;
                    }
                    else
                    {
                        dce2_stats.smb_readx_resp_chained_close++;
                        dce2_stats.smb_close_resp++;
                    }

                    break;

                case SMB_COM_WRITE:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_readx_req_chained_write++;
                        dce2_stats.smb_write_req++;
                    }
                    else
                    {
                        dce2_stats.smb_readx_resp_chained_write++;
                        dce2_stats.smb_write_resp++;
                    }

                    break;

                case SMB_COM_READ_ANDX:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_readx_req_chained_readx++;
                        dce2_stats.smb_readx_req++;
                    }
                    else
                    {
                        dce2_stats.smb_readx_resp_chained_readx++;
                        dce2_stats.smb_readx_resp++;
                    }

                    break;

                default:
                    if (smb_type == SMB_TYPE__REQUEST)
                    {
                        dce2_stats.smb_readx_req_chained_other++;
                        dce2_stats.smb_other_req++;
                    }
                    else
                    {
                        dce2_stats.smb_readx_resp_chained_other++;
                        dce2_stats.smb_other_resp++;
                    }

                    break;
            }

            break;

        default:
            if (smb_type == SMB_TYPE__REQUEST)
                dce2_stats.smb_other_req++;
            else
                dce2_stats.smb_other_resp++;
            break;
    }
}

/********************************************************************
 * Function: DCE2_SmbHandleSegmentation()
 *
 * Wrapper around DCE2_HandleSegmentation() to allocate a new
 * buffer object if necessary. 
 *
 * Arguments:
 *  DCE2_SmbSeg *
 *      Pointer to an SMB segmentation structure.
 *  uint8_t *
 *      Pointer to the current data cursor in packet.
 *  uint16_t
 *      Length of data from current data cursor.
 *  uint32_t
 *      Length of data that we need in order to consider
 *      desegmentation complete.
 *  uint16_t *
 *      Pointer to basically a return value for the amount of
 *      data in the packet that was actually used for
 *      desegmentation.
 *  int
 *      bool is true if we must append.
 *
 * Returns:
 *  DCE2_Ret
 *      DCE2_RET__ERROR if an error occured.  Nothing can
 *          be trusted.
 *      DCE2_RET__SEG if there is still more desegmentation
 *          to go, i.e. the need length has not been met by
 *          the data length.
 *      DCE2_RET__SUCCESS if desegmentation is complete,
 *          i.e. the need length was met.
 *
 ********************************************************************/
static INLINE DCE2_Ret DCE2_SmbHandleSegmentation(
    DCE2_SmbSeg *seg, const uint8_t *data_ptr,
    uint16_t data_len, uint32_t need_len,
    uint16_t *data_used, int append)
{
    DCE2_Ret status;
    PROFILE_VARS;
    uint32_t offset;

    PREPROC_PROFILE_START(dce2_pstat_smb_seg);

    if (seg == NULL)
    {
        PREPROC_PROFILE_END(dce2_pstat_smb_seg);
        return DCE2_RET__ERROR;
    }

    if (seg->buf == NULL)
    {
        /* No initial size or min alloc size */
        seg->buf = DCE2_BufferNew(need_len, need_len, DCE2_MEM_TYPE__SMB_SEG);
        if (seg->buf == NULL)
        {
            PREPROC_PROFILE_END(dce2_pstat_smb_seg);
            return DCE2_RET__ERROR;
        }
    }
    else if (DCE2_BufferMinAllocSize(seg->buf) != need_len)
    {
        DCE2_BufferSetMinAllocSize(seg->buf, need_len);
    }

    offset = DCE2_GetWriteOffset(need_len, append);

    status = DCE2_HandleSegmentation(
        seg->buf, data_ptr, data_len, offset, need_len, data_used);

    PREPROC_PROFILE_END(dce2_pstat_smb_seg);

    return status;
}

/********************************************************************
 * Function: DCE2_SmbGetSegPtr()
 *
 * Returns the appropriate segmentation buffer.
 *
 * Arguments:
 *  DCE2_SmbSsnData *
 *      Pointer to SMB session data.
 *
 * Returns:
 *  DCE2_SmbSeg *
 *      Pointer to client or server segmenation buffer.
 *
 ********************************************************************/
static INLINE DCE2_SmbSeg * DCE2_SmbGetSegPtr(DCE2_SmbSsnData *ssd)
{
    if (DCE2_SsnFromServer(ssd->sd.wire_pkt))
        return &ssd->srv_seg;

    return &ssd->cli_seg;
}

/********************************************************************
 * Function: DCE2_SmbGetIgnorePtr()
 *
 * Returns a pointer to the bytes we are ignoring on client or
 * server side.  Bytes are ignored if they are associated with
 * data we are not interested in.
 *
 * Arguments:
 *  DCE2_SmbSsnData *
 *      Pointer to SMB session data.
 *
 * Returns:
 *  uint32_t *
 *      Pointer to the client or server ignore bytes.
 *
 ********************************************************************/
static INLINE uint32_t * DCE2_SmbGetIgnorePtr(DCE2_SmbSsnData *ssd)
{
    if (DCE2_SsnFromServer(ssd->sd.wire_pkt))
        return &ssd->srv_ignore_bytes;

    return &ssd->cli_ignore_bytes;
}

/********************************************************************
 * Function: DCE2_SmbIsSegBuf()
 *
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
static INLINE int DCE2_SmbIsSegBuf(DCE2_SmbSsnData *ssd, const uint8_t *ptr)
{
    DCE2_Buffer *seg_buf;

    if (DCE2_SsnFromServer(ssd->sd.wire_pkt))
        seg_buf = ssd->srv_seg.buf;
    else
        seg_buf = ssd->cli_seg.buf;

    if (DCE2_BufferIsEmpty(seg_buf))
        return 0;

    /* See if we're looking at a segmentation buffer */
    if ((ptr < DCE2_BufferData(seg_buf)) ||
        (ptr > (DCE2_BufferData(seg_buf) + DCE2_BufferLength(seg_buf))))
    {
        return 0;
    }

    return 1;
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
static INLINE void DCE2_SmbSegAlert(DCE2_SmbSsnData *ssd, DCE2_Event event)
{
    SFSnortPacket *rpkt;
    DCE2_Buffer *buf;
    uint32_t nb_len = 0;

    if (DCE2_SsnFromClient(ssd->sd.wire_pkt))
        buf = ssd->cli_seg.buf;
    else
        buf = ssd->srv_seg.buf;

    /* This should be called from the desegmentation code. */
    if (DCE2_BufferIsEmpty(buf))
        return;

    rpkt = DCE2_GetRpkt(ssd->sd.wire_pkt, DCE2_RPKT_TYPE__SMB_SEG,
                        DCE2_BufferData(buf), DCE2_BufferLength(buf));

    if (rpkt == NULL)
    {
        DCE2_Log(DCE2_LOG_TYPE__ERROR,
                 "%s(%d) Failed to create reassembly packet.",
                 __FILE__, __LINE__);

        return;
    }

    if (DCE2_PushPkt(rpkt) != DCE2_RET__SUCCESS)
    {
        DCE2_Log(DCE2_LOG_TYPE__ERROR,
                 "%s(%d) Failed to push packet onto packet stack.",
                 __FILE__, __LINE__);
        return;
    }

    if (DCE2_BufferLength(buf) >= sizeof(NbssHdr))
        nb_len = NbssLen((NbssHdr *)DCE2_BufferData(buf));

    switch (event)
    {
        case DCE2_EVENT__SMB_BAD_NBSS_TYPE:
            DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_BAD_NBSS_TYPE);
            break;

        case DCE2_EVENT__SMB_BAD_TYPE:
            DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_BAD_TYPE);
            break;

        case DCE2_EVENT__SMB_BAD_ID:
            DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_BAD_ID);
            break;

        case DCE2_EVENT__SMB_NB_LT_SMBHDR:
            DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_NB_LT_SMBHDR, nb_len, sizeof(SmbNtHdr));
            break;

        default:
            break;
    }

    DCE2_PopPkt();
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
static INLINE int DCE2_SmbIsRawData(DCE2_SmbSsnData *ssd)
{
    if (ssd->br.smb_com == DCE2_SENTINEL)
        return 0;

    return 1;
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
static void DCE2_SmbHandleRawData(DCE2_SmbSsnData *ssd, const uint8_t *nb_ptr, uint32_t nb_len)
{
    DCE2_SmbFidTrackerNode *ft_node =
        DCE2_SmbFindFid(ssd, (uint16_t)ssd->br.fid_node.uid,
                        (uint16_t)ssd->br.fid_node.tid, (uint16_t)ssd->br.fid_node.fid);

    if (ft_node == NULL)
    {
        ssd->br.smb_com = DCE2_SENTINEL;
        return;
    }

    if (ssd->br.smb_com == SMB_COM_WRITE_BLOCK_RAW)
    {
        if (nb_len > ssd->br.total_count)
        {
            DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_TDCNT_LT_DSIZE,
                       (int)ssd->br.total_count, (int)nb_len);

            ssd->br.total_count = 0;
        }
        else
        {
            /* nb_len <= UINT16_MAX */
            ssd->br.total_count -= (uint16_t)nb_len;
        }

        if (ssd->br.total_count == 0)
            ssd->br.smb_com = DCE2_SENTINEL;
    }
    else
    {
        dce2_stats.smb_readbr_resp++;
        ssd->br.smb_com = DCE2_SENTINEL;
    }

    /* It's a raw DCE/RPC request.  Max DCE frag length is 16 bit */
    DCE2_CoProcess(&ssd->sd, &ft_node->co_tracker, nb_ptr, (uint16_t)nb_len);
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
static void DCE2_SmbProcessData(DCE2_SmbSsnData *ssd, const uint8_t *nb_ptr, uint32_t nb_len)
{
    int is_seg_buf = DCE2_SmbIsSegBuf(ssd, nb_ptr);

    if (is_seg_buf)
    {
        /* Put seg buf into a packet */
        SFSnortPacket *rpkt = DCE2_GetRpkt(ssd->sd.wire_pkt, DCE2_RPKT_TYPE__SMB_SEG, nb_ptr, nb_len);
        if (rpkt == NULL)
        {
            DCE2_Log(DCE2_LOG_TYPE__ERROR,
                     "%s(%d) Failed to create reassembly packet.",
                     __FILE__, __LINE__);
            return;
        }

        if (DCE2_PushPkt(rpkt) != DCE2_RET__SUCCESS)
        {
            DCE2_Log(DCE2_LOG_TYPE__ERROR,
                     "%s(%d) Failed to push packet onto packet stack.",
                     __FILE__, __LINE__);
            return;
        }

        nb_ptr = rpkt->payload;
        nb_len = rpkt->payload_size;

        dce2_stats.smb_seg_reassembled++;
    }

    if (DCE2_SmbIsRawData(ssd))
    {
        DCE2_MOVE(nb_ptr, nb_len, sizeof(NbssHdr));
        DCE2_SmbHandleRawData(ssd, nb_ptr, nb_len);
    }
    else
    {
        SmbNtHdr *smb_hdr = (SmbNtHdr *)(nb_ptr + sizeof(NbssHdr));
        DCE2_MOVE(nb_ptr, nb_len, sizeof(NbssHdr) + sizeof(SmbNtHdr));
        DCE2_SmbHandleCom(ssd, smb_hdr, nb_ptr, nb_len);
    }

    if (is_seg_buf)
        DCE2_PopPkt();
}

/********************************************************************
 * Function: DCE2_SmbSetMissedFids()
 *
 * If SMB packets were missed, we have no idea which, if any fid
 * requests/responses were missed.  It could be any, so set all
 * fids to missed packets.
 *
 * Arguments:
 *  DCE2_SmbSsnData *
 *      Pointer to SMB session data.
 *
 * Returns: None
 *
 ********************************************************************/
static void DCE2_SmbSetMissedFids(DCE2_SmbSsnData *ssd)
{
    const DCE2_Policy policy = DCE2_ScPolicy(ssd->sd.sconfig);
    DCE2_SmbFidTrackerNode *ft_node = NULL;
    DCE2_SmbUTNode *ut_node = NULL;

    switch (policy)
    {
        case DCE2_POLICY__SAMBA:
        case DCE2_POLICY__WIN2000:
        case DCE2_POLICY__SAMBA_3_0_20:
        case DCE2_POLICY__SAMBA_3_0_22:
        case DCE2_POLICY__SAMBA_3_0_37:
            /* Only uid used to create fid can be used to make a request */
            if ((ssd->sst.ft_node.fid_node.fid != DCE2_SENTINEL) &&
                (ssd->sst.ft_node.fid_node.uid != DCE2_SENTINEL))
            {
                ssd->sst.ft_node.co_tracker.missed_pkts = 1;
            }

            if (ssd->sst.fts != NULL)
            {
                for (ft_node = DCE2_ListFirst(ssd->sst.fts);
                     ft_node != NULL;
                     ft_node = DCE2_ListNext(ssd->sst.fts))
                {
                    ft_node->co_tracker.missed_pkts = 1;
                }
            }

            break;

        case DCE2_POLICY__WIN2003:
        case DCE2_POLICY__WINXP:
        case DCE2_POLICY__WINVISTA:
        case DCE2_POLICY__WIN2008:
        case DCE2_POLICY__WIN7:
            if ((ssd->sst.ptree.ut_node.uid != DCE2_SENTINEL) &&
                (ssd->sst.ptree.ut_node.tid != DCE2_SENTINEL))
            {
                ut_node = &ssd->sst.ptree.ut_node;
                if (ut_node->ft_node.fid_node.fid != DCE2_SENTINEL)
                {
                    ut_node->ft_node.co_tracker.missed_pkts = 1;
                }

                if (ut_node->fts != NULL)
                {
                    for (ft_node = DCE2_ListFirst(ut_node->fts);
                         ft_node != NULL;
                         ft_node = DCE2_ListNext(ut_node->fts))
                    {
                        ft_node->co_tracker.missed_pkts = 1;
                    }
                }
            }

            if (ssd->sst.ptree.uts != NULL)
            {
                for (ut_node = DCE2_ListFirst(ssd->sst.ptree.uts);
                     ut_node != NULL;
                     ut_node = DCE2_ListNext(ssd->sst.ptree.uts))
                {
                    if (ut_node->ft_node.fid_node.fid != DCE2_SENTINEL)
                    {
                        ut_node->ft_node.co_tracker.missed_pkts = 1;
                    }

                    if (ut_node->fts != NULL)
                    {
                        for (ft_node = DCE2_ListFirst(ut_node->fts);
                             ft_node != NULL;
                             ft_node = DCE2_ListNext(ut_node->fts))
                        {
                            ft_node->co_tracker.missed_pkts = 1;
                        }
                    }
                }
            }

            break;

        default:
            break;
    }
}

