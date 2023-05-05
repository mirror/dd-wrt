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
 * Module for handling connection-oriented DCE/RPC processing.  Provides
 * context id, interface UUID correlation and tracking for use with the
 * preprocessor rule options.  Provides desegmentation and defragmentation.
 * Sets appropriate data for use with the preprocessor rule options.
 *
 * 8/17/2008 - Initial implementation ... Todd Wease <twease@sourcefire.com>
 *
 ****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_types.h"
#include "spp_dce2.h"
#include "dce2_co.h"
#include "dce2_tcp.h"
#include "dce2_smb.h"
#include "snort_dce2.h"
#include "dce2_memory.h"
#include "dce2_utils.h"
#include "dce2_stats.h"
#include "dce2_event.h"
#include "dce2_debug.h"
#include "dcerpc.h"
#include "profiler.h"
#include "sf_snort_packet.h"
#include "sf_dynamic_preprocessor.h"

/********************************************************************
 * Macros
 ********************************************************************/
#define DCE2_CO__MIN_ALLOC_SIZE     50
#define DCE2_MAX_XMIT_SIZE_FUZZ    500

/********************************************************************
 * Global variables
 ********************************************************************/
static int co_reassembled = 0;

/********************************************************************
 * Enumerations
 ********************************************************************/
typedef enum _DCE2_CoRpktType
{
    DCE2_CO_RPKT_TYPE__SEG,
    DCE2_CO_RPKT_TYPE__FRAG,
    DCE2_CO_RPKT_TYPE__ALL

} DCE2_CoRpktType;

typedef enum _DCE2_CoCtxState
{
    DCE2_CO_CTX_STATE__ACCEPTED,
    DCE2_CO_CTX_STATE__REJECTED,
    DCE2_CO_CTX_STATE__PENDING

} DCE2_CoCtxState;

/********************************************************************
 * Structures
 ********************************************************************/
typedef struct _DCE2_CoCtxIdNode
{
    uint16_t ctx_id;           /* The context id */
    Uuid iface;                /* The presentation syntax uuid for the interface */
    uint16_t iface_vers_maj;   /* The major version of the interface */
    uint16_t iface_vers_min;   /* The minor version of the interface */

    /* Whether or not the server accepted or rejected the client bind/alter context
     * request.  Initially set to pending until server response */
    DCE2_CoCtxState state;

} DCE2_CoCtxIdNode;

/********************************************************************
 * Private function prototypes
 ********************************************************************/
static DCE2_Ret DCE2_CoHdrChecks(DCE2_SsnData *, DCE2_CoTracker *, const DceRpcCoHdr *);
static void DCE2_CoDecode(DCE2_SsnData *, DCE2_CoTracker *,
                          const uint8_t *, uint16_t);
static void DCE2_CoSegDecode(DCE2_SsnData *, DCE2_CoTracker *, DCE2_CoSeg *);
static void DCE2_CoBind(DCE2_SsnData *, DCE2_CoTracker *,
                        const DceRpcCoHdr *, const uint8_t *, uint16_t);
static void DCE2_CoAlterCtx(DCE2_SsnData *, DCE2_CoTracker *,
                            const DceRpcCoHdr *, const uint8_t *, uint16_t);
static void DCE2_CoCtxReq(DCE2_SsnData *, DCE2_CoTracker *, const DceRpcCoHdr *,
                          const uint8_t, const uint8_t *, uint16_t);
static void DCE2_CoBindAck(DCE2_SsnData *, DCE2_CoTracker *,
                           const DceRpcCoHdr *, const uint8_t *, uint16_t);
static void DCE2_CoRequest(DCE2_SsnData *, DCE2_CoTracker *,
                           const DceRpcCoHdr *, const uint8_t *, uint16_t);
static void DCE2_CoResponse(DCE2_SsnData *, DCE2_CoTracker *,
                            const DceRpcCoHdr *, const uint8_t *, uint16_t);
static void DCE2_CoHandleFrag(DCE2_SsnData *, DCE2_CoTracker *,
                              const DceRpcCoHdr *, const uint8_t *, uint16_t);
static inline DCE2_Ret DCE2_CoHandleSegmentation(DCE2_CoSeg *, const uint8_t *,
        uint16_t, uint16_t, uint16_t *);
static void DCE2_CoReassemble(DCE2_SsnData *, DCE2_CoTracker *, DCE2_CoRpktType);
static inline void DCE2_CoFragReassemble(DCE2_SsnData *, DCE2_CoTracker *);
static DCE2_Ret DCE2_CoSetIface(DCE2_SsnData *, DCE2_CoTracker *, uint16_t);
static int DCE2_CoCtxCompare(const void *, const void *);
static void DCE2_CoCtxFree(void *);

static inline void DCE2_CoSetRopts(DCE2_SsnData *, DCE2_CoTracker *, const DceRpcCoHdr *);
static inline void DCE2_CoSetRdata(DCE2_SsnData *, DCE2_CoTracker *, uint8_t *, uint16_t);
static inline void DCE2_CoResetFragTracker(DCE2_CoFragTracker *);
static inline void DCE2_CoResetTracker(DCE2_CoTracker *);
static inline DCE2_Ret DCE2_CoInitCtxStorage(DCE2_CoTracker *);
static inline void DCE2_CoEraseCtxIds(DCE2_CoTracker *);
static inline void DCE2_CoSegAlert(DCE2_SsnData *, DCE2_CoTracker *, DCE2_Event);
static inline SFSnortPacket * DCE2_CoGetSegRpkt(DCE2_SsnData *, const uint8_t *, uint32_t);
static inline DCE2_RpktType DCE2_CoGetRpktType(DCE2_SsnData *, DCE2_BufType);
static SFSnortPacket * DCE2_CoGetRpkt(DCE2_SsnData *, DCE2_CoTracker *, DCE2_CoRpktType, DCE2_RpktType *);

static inline DCE2_CoSeg * DCE2_CoGetSegPtr(DCE2_SsnData *, DCE2_CoTracker *);
static inline DCE2_Buffer * DCE2_CoGetFragBuf(DCE2_SsnData *, DCE2_CoFragTracker *);
static inline int DCE2_CoIsSegBuf(DCE2_SsnData *, DCE2_CoTracker *, const uint8_t *);
static void DCE2_CoEarlyReassemble(DCE2_SsnData *, DCE2_CoTracker *);
static DCE2_Ret DCE2_CoSegEarlyRequest(DCE2_CoTracker *, const uint8_t *, uint32_t);
static int DCE2_CoGetAuthLen(DCE2_SsnData *, const DceRpcCoHdr *,
        const uint8_t *, uint16_t);

/********************************************************************
 * Function: DCE2_CoInitRdata()
 *
 * Initializes header of defragmentation reassembly packet.
 * Sets relevant fields in header that will not have to change
 * from reassembly to reassembly.  The reassembly buffer used is
 * big enough for the header.
 *
 * Arguments:
 *  uint8_t *
 *      Pointer to the place in the reassembly packet to set
 *      the header data.
 *
 * Returns: None
 *
 ********************************************************************/
void DCE2_CoInitRdata(uint8_t *co_ptr, int dir)
{
    DceRpcCoHdr *co_hdr = (DceRpcCoHdr *)co_ptr;

    /* Set some relevant fields.  These should never get reset */
    co_hdr->pversion.major = DCERPC_PROTO_MAJOR_VERS__5;
    co_hdr->pfc_flags = (DCERPC_CO_PFC_FLAGS__FIRST_FRAG | DCERPC_CO_PFC_FLAGS__LAST_FRAG);
    co_hdr->packed_drep[0] = 0x10;   /* Little endian */

    if (dir == FLAG_FROM_CLIENT)
        co_hdr->ptype = DCERPC_PDU_TYPE__REQUEST;
    else
        co_hdr->ptype = DCERPC_PDU_TYPE__RESPONSE;
}

/********************************************************************
 * Function: DCE2_CoSetRdata()
 *
 * Sets relevant fields in the defragmentation reassembly packet
 * based on data gathered from the session and reassembly phase.
 * The reassembly buffer used is big enough for the headers.
 *
 * Arguments:
 *  DCE2_CoTracker *
 *      Pointer to the relevant connection-oriented tracker.
 *  uint8_t *
 *      Pointer to the place in the reassembly packet where the
 *      header starts.
 *  uint16_t
 *      The length of the stub data.
 *
 * Returns: None
 *
 ********************************************************************/
static inline void DCE2_CoSetRdata(DCE2_SsnData *sd, DCE2_CoTracker *cot,
                                   uint8_t *co_ptr, uint16_t stub_len)
{
    DceRpcCoHdr *co_hdr = (DceRpcCoHdr *)co_ptr;
    /* If we've set the fragment tracker context id or opnum, use them. */
    uint16_t ctx_id =
        (cot->frag_tracker.ctx_id != DCE2_SENTINEL) ?
            (uint16_t)cot->frag_tracker.ctx_id : (uint16_t)cot->ctx_id;
    uint16_t opnum =
        (cot->frag_tracker.opnum != DCE2_SENTINEL) ?
            (uint16_t)cot->frag_tracker.opnum : (uint16_t)cot->opnum;

    if (DCE2_SsnFromClient(sd->wire_pkt))
    {
        DceRpcCoRequest *co_req = (DceRpcCoRequest *)((uint8_t *)co_hdr + sizeof(DceRpcCoHdr));
        /* Doesn't really matter if this wraps ... it is basically just for presentation */
        uint16_t flen = sizeof(DceRpcCoHdr) + sizeof(DceRpcCoRequest) + stub_len;

        co_hdr->frag_length = DceRpcHtons(&flen, DCERPC_BO_FLAG__LITTLE_ENDIAN);
        co_req->context_id = DceRpcHtons(&ctx_id, DCERPC_BO_FLAG__LITTLE_ENDIAN);
        co_req->opnum = DceRpcHtons(&opnum, DCERPC_BO_FLAG__LITTLE_ENDIAN);
    }
    else
    {
        DceRpcCoResponse *co_resp = (DceRpcCoResponse *)((uint8_t *)co_hdr + sizeof(DceRpcCoHdr));
        uint16_t flen = sizeof(DceRpcCoHdr) + sizeof(DceRpcCoResponse) + stub_len;

        co_hdr->frag_length = DceRpcHtons(&flen, DCERPC_BO_FLAG__LITTLE_ENDIAN);
        co_resp->context_id = DceRpcHtons(&ctx_id, DCERPC_BO_FLAG__LITTLE_ENDIAN);
    }
}

/********************************************************************
 * Function: DCE2_CoProcess()
 *
 * Main entry point for connection-oriented DCE/RPC processing.
 * Since there can be more than one DCE/RPC pdu in the packet, it
 * loops through the packet data until none is left.  It handles
 * transport layer segmentation and buffers data until it gets the
 * full pdu, then hands off to pdu processing.
 *
 * Arguments:
 *  DCE2_SsnData *
 *      Pointer to the session data structure.
 *  DCE2_CoTracker *
 *      Pointer to the relevant connection-oriented tracker.
 *  const uint8_t *
 *      Pointer to packet data
 *  uint16_t
 *      Packet data length
 *
 * Returns: None
 *
 ********************************************************************/
void DCE2_CoProcess(DCE2_SsnData *sd, DCE2_CoTracker *cot,
                    const uint8_t *data_ptr, uint16_t data_len)
{
    DCE2_CoSeg *seg = DCE2_CoGetSegPtr(sd, cot);
    DCE2_Ret status;
    uint32_t num_frags = 0;

    dce2_stats.co_pdus++;
    co_reassembled = 0;

    while (data_len > 0)
    {
        num_frags++;

        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CO, "DCE/RPC message number: %u\n", num_frags));

        /* Fast track full fragments */
        if (DCE2_BufferIsEmpty(seg->buf))
        {
            const uint8_t *frag_ptr = data_ptr;
            uint16_t frag_len;
            uint16_t data_used;

            /* Not enough data left for a header.  Buffer it and return */
            if (data_len < sizeof(DceRpcCoHdr))
            {
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CO,
                            "Not enough data in packet for DCE/RPC Connection-oriented header.\n"));

                DCE2_CoHandleSegmentation(seg, data_ptr, data_len, sizeof(DceRpcCoHdr), &data_used);

                /* Just break out of loop in case early detect is enabled */
                break;
            }

            if (DCE2_CoHdrChecks(sd, cot, (DceRpcCoHdr *)data_ptr) != DCE2_RET__SUCCESS)
                return;

            frag_len = DceRpcCoFragLen((DceRpcCoHdr *)data_ptr);

            /* Not enough data left for the pdu. */
            if (data_len < frag_len)
            {
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CO,
                            "Not enough data in packet for fragment length: %u\n", frag_len));

                /* Set frag length so we don't have to check it again in seg code */
                seg->frag_len = frag_len;

                DCE2_CoHandleSegmentation(seg, data_ptr, data_len, frag_len, &data_used);
                break;
            }

            DCE2_MOVE(data_ptr, data_len, frag_len);

            /* Got a full DCE/RPC pdu */
            DCE2_CoDecode(sd, cot, frag_ptr, frag_len);

            /* If we're configured to do defragmentation only detect on first frag
             * since we'll detect on reassembled */
            if (!DCE2_GcDceDefrag() || ((num_frags == 1) && !co_reassembled))
                DCE2_Detect(sd);

            /* Reset if this is a last frag */
            if (DceRpcCoLastFrag((DceRpcCoHdr *)frag_ptr))
                num_frags = 0;
        }
        else  /* We've already buffered data */
        {
            uint16_t data_used = 0;

            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CO, "Segmentation buffer has %u bytes\n",
                           DCE2_BufferLength(seg->buf)));

            /* Need more data to get header */
            if (DCE2_BufferLength(seg->buf) < sizeof(DceRpcCoHdr))
            {
                status = DCE2_CoHandleSegmentation(seg, data_ptr, data_len, sizeof(DceRpcCoHdr), &data_used);

                /* Still not enough for header */
                if (status != DCE2_RET__SUCCESS)
                    break;

                /* Move the length of the amount of data we used to get header */
                DCE2_MOVE(data_ptr, data_len, data_used);

                if (DCE2_CoHdrChecks(sd, cot, (DceRpcCoHdr *)DCE2_BufferData(seg->buf)) != DCE2_RET__SUCCESS)
                {
                    int data_back;
                    DCE2_BufferEmpty(seg->buf);
                    /* Move back to original packet header */
                    data_back = -data_used;
                    DCE2_MOVE(data_ptr, data_len, data_back);
                    /*Check the original packet*/
                    if (DCE2_CoHdrChecks(sd, cot, (DceRpcCoHdr *)data_ptr) != DCE2_RET__SUCCESS)
                        return;
                    else
                    {
                        /*Only use the original packet, ignore the data in seg_buffer*/
                        num_frags = 0;
                        continue;
                    }
                }

                seg->frag_len = DceRpcCoFragLen((DceRpcCoHdr *)DCE2_BufferData(seg->buf));
            }

            /* Need more data for full pdu */
            if (DCE2_BufferLength(seg->buf) < seg->frag_len)
            {
                status = DCE2_CoHandleSegmentation(seg, data_ptr, data_len, seg->frag_len, &data_used);

                /* Still not enough */
                if (status != DCE2_RET__SUCCESS)
                    break;

                DCE2_MOVE(data_ptr, data_len, data_used);
            }

            /* Do this before calling DCE2_CoSegDecode since it will empty
             * seg buffer */
            if (DceRpcCoLastFrag((DceRpcCoHdr *)seg->buf->data))
                num_frags = 0;

            /* Got the full DCE/RPC pdu. Need to create new packet before decoding */
            DCE2_CoSegDecode(sd, cot, seg);

            if ( !data_used )
                break;
        }
    }

    if (DCE2_GcReassembleEarly() && !co_reassembled)
        DCE2_CoEarlyReassemble(sd, cot);
}

/********************************************************************
 * Function: DCE2_CoHandleSegmentation()
 *
 * Wrapper around DCE2_HandleSegmentation() to allocate a new
 * buffer object if necessary.
 *
 * Arguments:
 *  DCE2_CoSeg *
 *      Pointer to a connection-oriented segmentation structure.
 *  uint8_t *
 *      Pointer to the current data cursor in packet.
 *  uint16_t
 *      Length of data from current data cursor.
 *  uint16_t
 *      Length of data that we need in order to consider
 *      desegmentation complete.
 *  uint16_t *
 *      Pointer to basically a return value for the amount of
 *      data in the packet that was actually used for
 *      desegmentation.
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
static inline DCE2_Ret DCE2_CoHandleSegmentation(DCE2_CoSeg *seg,
        const uint8_t *data_ptr, uint16_t data_len, uint16_t need_len,
        uint16_t *data_used)
{
    DCE2_Ret status;
    PROFILE_VARS;

    PREPROC_PROFILE_START(dce2_pstat_co_seg);

    if (seg == NULL)
    {
        PREPROC_PROFILE_END(dce2_pstat_co_seg);
        return DCE2_RET__ERROR;
    }

    if (seg->buf == NULL)
    {
        seg->buf = DCE2_BufferNew(need_len, DCE2_CO__MIN_ALLOC_SIZE, DCE2_MEM_TYPE__CO_SEG);
        if (seg->buf == NULL)
        {
            PREPROC_PROFILE_END(dce2_pstat_co_seg);
            return DCE2_RET__ERROR;
        }
    }

    status = DCE2_HandleSegmentation(seg->buf,
            data_ptr, data_len, need_len, data_used);

    PREPROC_PROFILE_END(dce2_pstat_co_seg);

    return status;
}

/********************************************************************
 * Function: DCE2_CoHdrChecks()
 *
 * Checks some relevant fields in the header to make sure they're
 * sane.
 *
 * Arguments:
 *  DCE2_SsnData *
 *      Pointer to the session data structure.
 *  DCE2_CoTracker *
 *      Pointer to the relevant connection-oriented tracker.
 *  DceRpcCoHdr *
 *      Pointer to the header struct layed over the packet data.
 *
 * Returns:
 *  DCE2_Ret
 *      DCE2_RET__ERROR if we should not continue processing.
 *      DCE2_RET__SUCCESS if we should continue processing.
 *
 ********************************************************************/
static DCE2_Ret DCE2_CoHdrChecks(DCE2_SsnData *sd, DCE2_CoTracker *cot, const DceRpcCoHdr *co_hdr)
{
    uint16_t frag_len = DceRpcCoFragLen(co_hdr);
    DceRpcPduType pdu_type = DceRpcCoPduType(co_hdr);
    int is_seg_buf = DCE2_CoIsSegBuf(sd, cot, (uint8_t *)co_hdr);

    if (frag_len < sizeof(DceRpcCoHdr))
    {
        /* Assume we autodetected incorrectly or that DCE/RPC is not running
         * over the SMB named pipe */
        if (!DCE2_SsnAutodetected(sd) && (sd->trans != DCE2_TRANS_TYPE__SMB))
        {
            if (is_seg_buf)
                DCE2_CoSegAlert(sd, cot, DCE2_EVENT__CO_FLEN_LT_HDR);
            else
                DCE2_Alert(sd, DCE2_EVENT__CO_FLEN_LT_HDR, frag_len, sizeof(DceRpcCoHdr));
        }

        return DCE2_RET__ERROR;
    }

    if (DceRpcCoVersMaj(co_hdr) != DCERPC_PROTO_MAJOR_VERS__5)
    {
        if (!DCE2_SsnAutodetected(sd) && (sd->trans != DCE2_TRANS_TYPE__SMB))
        {
            if (is_seg_buf)
                DCE2_CoSegAlert(sd, cot, DCE2_EVENT__CO_BAD_MAJ_VERSION);
            else
                DCE2_Alert(sd, DCE2_EVENT__CO_BAD_MAJ_VERSION, DceRpcCoVersMaj(co_hdr));
        }

        return DCE2_RET__ERROR;
    }

    if (DceRpcCoVersMin(co_hdr) != DCERPC_PROTO_MINOR_VERS__0)
    {
        if (!DCE2_SsnAutodetected(sd) && (sd->trans != DCE2_TRANS_TYPE__SMB))
        {
            if (is_seg_buf)
                DCE2_CoSegAlert(sd, cot, DCE2_EVENT__CO_BAD_MIN_VERSION);
            else
                DCE2_Alert(sd, DCE2_EVENT__CO_BAD_MIN_VERSION, DceRpcCoVersMin(co_hdr));
        }

        return DCE2_RET__ERROR;
    }

    if (pdu_type >= DCERPC_PDU_TYPE__MAX)
    {
        if (!DCE2_SsnAutodetected(sd) && (sd->trans != DCE2_TRANS_TYPE__SMB))
        {
            if (is_seg_buf)
                DCE2_CoSegAlert(sd, cot, DCE2_EVENT__CO_BAD_PDU_TYPE);
            else
                DCE2_Alert(sd, DCE2_EVENT__CO_BAD_PDU_TYPE, DceRpcCoPduType(co_hdr));
        }

        return DCE2_RET__ERROR;
    }

    if (DCE2_SsnFromClient(sd->wire_pkt) && (cot->max_xmit_frag != DCE2_SENTINEL))
    {
        if (frag_len > cot->max_xmit_frag)
        {
            if (is_seg_buf)
                DCE2_CoSegAlert(sd, cot, DCE2_EVENT__CO_FRAG_GT_MAX_XMIT_FRAG);
            else
                DCE2_Alert(sd, DCE2_EVENT__CO_FRAG_GT_MAX_XMIT_FRAG, dce2_pdu_types[pdu_type],
                           frag_len, cot->max_xmit_frag);
        }
        else if (!DceRpcCoLastFrag(co_hdr) && (pdu_type == DCERPC_PDU_TYPE__REQUEST)
                && ((((int)cot->max_xmit_frag - DCE2_MAX_XMIT_SIZE_FUZZ) < 0)
                    || ((int)frag_len < ((int)cot->max_xmit_frag - DCE2_MAX_XMIT_SIZE_FUZZ))))
        {
            /* If client needs to fragment the DCE/RPC request, it shouldn't be less than the
             * maximum xmit size negotiated. Only if it's not a last fragment. Make this alert
             * only if it is considerably less - have seen legitimate fragments that are just
             * slightly less the negotiated fragment size. */
            if (is_seg_buf)
                DCE2_CoSegAlert(sd, cot, DCE2_EVENT__CO_FRAG_LT_MAX_XMIT_FRAG);
            else
                DCE2_Alert(sd, DCE2_EVENT__CO_FRAG_LT_MAX_XMIT_FRAG, dce2_pdu_types[pdu_type],
                           frag_len, cot->max_xmit_frag);
        }

        /* Continue processing */
    }

    return DCE2_RET__SUCCESS;
}

/********************************************************************
 * Function: DCE2_CoDecode()
 *
 * Main processing for the DCE/RPC pdu types.  Most are not
 * implemented as, currently, they are not necessary and only
 * stats are kept for them.  Important are the bind, alter context
 * and request.
 *
 * Arguments:
 *  DCE2_SsnData *
 *      Pointer to the session data structure
 *  DCE2_CoTracker *
 *      Pointer to the relevant connection-oriented tracker.
 *  const uint8_t *
 *      Pointer to the start of the DCE/RPC pdu in the packet data.
 *  uint16_t
 *      Fragment length of the pdu.
 *
 * Returns: None
 *
 ********************************************************************/
static void DCE2_CoDecode(DCE2_SsnData *sd, DCE2_CoTracker *cot,
                          const uint8_t *frag_ptr, uint16_t frag_len)
{
    /* Already checked that we have enough data for header */
    const DceRpcCoHdr *co_hdr = (DceRpcCoHdr *)frag_ptr;
    int pdu_type = DceRpcCoPduType(co_hdr);

    /* We've got the main header.  Move past it to the
     * start of the pdu */
    DCE2_MOVE(frag_ptr, frag_len, sizeof(DceRpcCoHdr));

    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CO, "PDU type: "));

    /* Client specific pdu types - some overlap with server */
    if (DCE2_SsnFromClient(sd->wire_pkt))
    {
        switch (pdu_type)
        {
            case DCERPC_PDU_TYPE__BIND:
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CO, "Bind\n"));
                dce2_stats.co_bind++;

                /* Make sure context id list and queue are initialized */
                if (DCE2_CoInitCtxStorage(cot) != DCE2_RET__SUCCESS)
                    return;

                DCE2_CoBind(sd, cot, co_hdr, frag_ptr, frag_len);

                break;

            case DCERPC_PDU_TYPE__ALTER_CONTEXT:
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CO, "Alter Context\n"));
                dce2_stats.co_alter_ctx++;

                if (DCE2_CoInitCtxStorage(cot) != DCE2_RET__SUCCESS)
                    return;

                DCE2_CoAlterCtx(sd, cot, co_hdr, frag_ptr, frag_len);

                break;

            case DCERPC_PDU_TYPE__REQUEST:
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CO, "Request\n"));
                dce2_stats.co_request++;

                if (DCE2_ListIsEmpty(cot->ctx_ids) &&
                    DCE2_QueueIsEmpty(cot->pending_ctx_ids))
                {
                    return;
                }

                DCE2_CoRequest(sd, cot, co_hdr, frag_ptr, frag_len);

                break;

            case DCERPC_PDU_TYPE__AUTH3:
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CO, "Auth3\n"));
                dce2_stats.co_auth3++;
                break;

            case DCERPC_PDU_TYPE__CO_CANCEL:
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CO, "Cancel\n"));
                dce2_stats.co_cancel++;
                break;

            case DCERPC_PDU_TYPE__ORPHANED:
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CO, "Orphaned\n"));
                dce2_stats.co_orphaned++;
                break;

            case DCERPC_PDU_TYPE__MICROSOFT_PROPRIETARY_OUTLOOK2003_RPC_OVER_HTTP:
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CO, "Microsoft Request To Send RPC over HTTP\n"));
                dce2_stats.co_ms_pdu++;
                break;

            default:
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CO, "Unknown (0x%02x)\n", pdu_type));
                dce2_stats.co_other_req++;
                break;
        }
    }
    else
    {
        switch (pdu_type)
        {
            case DCERPC_PDU_TYPE__BIND_ACK:
            case DCERPC_PDU_TYPE__ALTER_CONTEXT_RESP:
                if (pdu_type == DCERPC_PDU_TYPE__BIND_ACK)
                {
                    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CO, "Bind Ack\n"));
                    dce2_stats.co_bind_ack++;
                }
                else
                {
                    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CO, "Alter Context Response\n"));
                    dce2_stats.co_alter_ctx_resp++;
                }

                if (DCE2_QueueIsEmpty(cot->pending_ctx_ids))
                    return;

                /* Bind ack and alter context response have the same
                 * header structure, just different pdu type */
                DCE2_CoBindAck(sd, cot, co_hdr, frag_ptr, frag_len);

                /* Got the bind/alter response - clear out the pending queue */
                DCE2_QueueEmpty(cot->pending_ctx_ids);

                break;

            case DCERPC_PDU_TYPE__BIND_NACK:
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CO, "Bind Nack\n"));
                dce2_stats.co_bind_nack++;

                /* Bind nack in Windows seems to blow any previous context away */
                switch (DCE2_SsnGetServerPolicy(sd))
                {
                    case DCE2_POLICY__WIN2000:
                    case DCE2_POLICY__WIN2003:
                    case DCE2_POLICY__WINXP:
                    case DCE2_POLICY__WINVISTA:
                    case DCE2_POLICY__WIN2008:
                    case DCE2_POLICY__WIN7:
                        DCE2_CoEraseCtxIds(cot);
                        break;

                    default:
                        break;
                }

                cot->got_bind = 0;

                break;

            case DCERPC_PDU_TYPE__RESPONSE:
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CO, "Response\n"));
                dce2_stats.co_response++;
                DCE2_CoResponse(sd, cot, co_hdr, frag_ptr, frag_len);
                break;

            case DCERPC_PDU_TYPE__FAULT:
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CO, "Fault\n"));
                dce2_stats.co_fault++;

                /* Clear out the client side */
                DCE2_QueueEmpty(cot->pending_ctx_ids);
                DCE2_BufferEmpty(cot->cli_seg.buf);
                DCE2_BufferEmpty(cot->frag_tracker.cli_stub_buf);

                DCE2_CoResetTracker(cot);

                break;

            case DCERPC_PDU_TYPE__SHUTDOWN:
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CO, "Shutdown\n"));
                dce2_stats.co_shutdown++;
                break;

            case DCERPC_PDU_TYPE__REJECT:
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CO, "Reject\n"));
                dce2_stats.co_reject++;

                DCE2_QueueEmpty(cot->pending_ctx_ids);

                break;

            case DCERPC_PDU_TYPE__MICROSOFT_PROPRIETARY_OUTLOOK2003_RPC_OVER_HTTP:
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CO, "Microsoft Request To Send RPC over HTTP\n"));
                dce2_stats.co_ms_pdu++;
                break;

            default:
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CO, "Unknown (0x%02x)\n", pdu_type));
                dce2_stats.co_other_resp++;
                break;
        }
    }
}

/********************************************************************
 * Function: DCE2_CoBind()
 *
 * Handles the processing of a client bind request.  There are
 * differences between Windows and Samba and even early Samba in
 * how multiple binds on the session are handled.  Processing of
 * the context id bindings is handed off.
 *
 * Arguments:
 *  DCE2_SsnData *
 *      Pointer to the session data structure.
 *  DCE2_CoTracker *
 *      Pointer to the relevant connection-oriented tracker.
 *  DceRpcCoHdr *
 *      Pointer to the main header in the packet data.
 *  const uint8_t *
 *      Pointer to the current processing point of the DCE/RPC
 *      pdu in the packet data.
 *  uint16_t
 *      Fragment length left in the pdu.
 *
 * Returns: None
 *
 ********************************************************************/
static void DCE2_CoBind(DCE2_SsnData *sd, DCE2_CoTracker *cot,
                        const DceRpcCoHdr *co_hdr, const uint8_t *frag_ptr, uint16_t frag_len)
{
    DCE2_Policy policy = DCE2_SsnGetServerPolicy(sd);
    DceRpcCoBind *bind = (DceRpcCoBind *)frag_ptr;

    if (frag_len < sizeof(DceRpcCoBind))
    {
        DCE2_Alert(sd, DCE2_EVENT__CO_FLEN_LT_SIZE, dce2_pdu_types[DceRpcCoPduType(co_hdr)],
                   frag_len, sizeof(DceRpcCoBind));
        return;
    }

    DCE2_MOVE(frag_ptr, frag_len, sizeof(DceRpcCoBind));

    switch (policy)
    {
        case DCE2_POLICY__WIN2000:
        case DCE2_POLICY__WIN2003:
        case DCE2_POLICY__WINXP:
        case DCE2_POLICY__WINVISTA:
        case DCE2_POLICY__WIN2008:
        case DCE2_POLICY__WIN7:
            /* Windows will not accept more than one bind */
            if (!DCE2_ListIsEmpty(cot->ctx_ids))
            {
                /* Delete context id list if anything there */
                DCE2_CoEraseCtxIds(cot);
                return;
            }

            /* Byte order of stub data will be that of the bind */
            cot->data_byte_order = DceRpcCoByteOrder(co_hdr);

            break;

        case DCE2_POLICY__SAMBA:
        case DCE2_POLICY__SAMBA_3_0_37:
        case DCE2_POLICY__SAMBA_3_0_22:
            if (cot->got_bind)
                return;

            break;

        case DCE2_POLICY__SAMBA_3_0_20:
            /* Accepts multiple binds */
            break;

        default:
            DCE2_Log(DCE2_LOG_TYPE__ERROR,
                     "%s(%d) Invalid policy: %d",
                     __FILE__, __LINE__, policy);
            return;
    }

    cot->max_xmit_frag = (int)DceRpcCoBindMaxXmitFrag(co_hdr, bind);
    DCE2_CoCtxReq(sd, cot, co_hdr, DceRpcCoNumCtxItems(bind), frag_ptr, frag_len);
}

/********************************************************************
 * Function: DCE2_CoAlterCtx()
 *
 * Handles the processing of a client alter context request.
 * Again, differences in how this is handled - whether we've seen
 * a bind yet or not, altering the data byte order.  Processing
 * of the context id bindings is handed off.
 *
 * Arguments:
 *  DCE2_SsnData *
 *      Pointer to the session data structure.
 *  DCE2_CoTracker *
 *      Pointer to the relevant connection-oriented tracker.
 *  DceRpcCoHdr *
 *      Pointer to the main header in the packet data.
 *  const uint8_t *
 *      Pointer to the current processing point of the DCE/RPC
 *      pdu in the packet data.
 *  uint16_t
 *      Fragment length left in the pdu.
 *
 * Returns: None
 *
 ********************************************************************/
static void DCE2_CoAlterCtx(DCE2_SsnData *sd, DCE2_CoTracker *cot,
                            const DceRpcCoHdr *co_hdr, const uint8_t *frag_ptr, uint16_t frag_len)
{
    DCE2_Policy policy = DCE2_SsnGetServerPolicy(sd);
    DceRpcCoAltCtx *alt_ctx = (DceRpcCoAltCtx *)frag_ptr;

    if (frag_len < sizeof(DceRpcCoAltCtx))
    {
        DCE2_Alert(sd, DCE2_EVENT__CO_FLEN_LT_SIZE, dce2_pdu_types[DceRpcCoPduType(co_hdr)],
                   frag_len, sizeof(DceRpcCoAltCtx));
        return;
    }

    DCE2_MOVE(frag_ptr, frag_len, sizeof(DceRpcCoAltCtx));

    switch (policy)
    {
        case DCE2_POLICY__WIN2000:
        case DCE2_POLICY__WIN2003:
        case DCE2_POLICY__WINXP:
        case DCE2_POLICY__WINVISTA:
        case DCE2_POLICY__WIN2008:
        case DCE2_POLICY__WIN7:
            /* Windows will not accept an alter context before
             * bind and will bind_nak it */
            if (DCE2_ListIsEmpty(cot->ctx_ids))
                return;

            if (cot->data_byte_order != (int)DceRpcCoByteOrder(co_hdr))
            {
                /* This is anomalous behavior.  Alert, but continue processing */
                if (cot->data_byte_order != DCE2_SENTINEL)
                    DCE2_Alert(sd, DCE2_EVENT__CO_ALTER_CHANGE_BYTE_ORDER);
            }

            break;

        case DCE2_POLICY__SAMBA:
        case DCE2_POLICY__SAMBA_3_0_37:
        case DCE2_POLICY__SAMBA_3_0_22:
        case DCE2_POLICY__SAMBA_3_0_20:
            /* Nothing for Samba */
            break;

        default:
            DCE2_Log(DCE2_LOG_TYPE__ERROR,
                     "%s(%d) Invalid policy: %d",
                     __FILE__, __LINE__, policy);
            break;
    }

    /* Alter context is typedef'ed as a bind */
    DCE2_CoCtxReq(sd, cot, co_hdr, DceRpcCoNumCtxItems((DceRpcCoBind *)alt_ctx), frag_ptr, frag_len);
}

/********************************************************************
 * Function: DCE2_CoCtxReq()
 *
 * Handles parsing the context id list out of the packet.
 * Context ids and associated uuids are stored in a queue and
 * dequeued upon server response.  Server response doesn't
 * indicate by context id which bindings were accepted or
 * rejected, but the index or order they were in in the client
 * bind or alter context, hence the queue.
 *
 * Arguments:
 *  DCE2_SsnData *
 *      Pointer to the session data structure.
 *  DCE2_CoTracker *
 *      Pointer to the relevant connection-oriented tracker.
 *  DceRpcCoHdr *
 *      Pointer to the main header in the packet data.
 *  const uint8_t
 *      The number of context items in the bind or alter context.
 *  const uint8_t *
 *      Pointer to the current processing point of the DCE/RPC
 *      pdu in the packet data.
 *  uint16_t
 *      Fragment length left in the pdu.
 *
 * Returns: None
 *
 ********************************************************************/
static void DCE2_CoCtxReq(DCE2_SsnData *sd, DCE2_CoTracker *cot, const DceRpcCoHdr *co_hdr,
                          const uint8_t num_ctx_items, const uint8_t *frag_ptr, uint16_t frag_len)
{
    DCE2_Policy policy = DCE2_SsnGetServerPolicy(sd);
    unsigned int i;
    DCE2_Ret status;

    if (num_ctx_items == 0)
    {
        DCE2_Alert(sd, DCE2_EVENT__CO_ZERO_CTX_ITEMS, dce2_pdu_types[DceRpcCoPduType(co_hdr)]);
        return;
    }

    for (i = 0; i < num_ctx_items; i++)
    {
        DceRpcCoContElem *ctx_elem = (DceRpcCoContElem *)frag_ptr;
        uint16_t ctx_id;
        uint8_t num_tsyns;
        const Uuid *iface;
        uint16_t if_vers_maj;
        uint16_t if_vers_min;
        DCE2_CoCtxIdNode *ctx_node;
        int j;
        PROFILE_VARS;

        if (frag_len < sizeof(DceRpcCoContElem))
        {
            DCE2_Alert(sd, DCE2_EVENT__CO_FLEN_LT_SIZE, dce2_pdu_types[DceRpcCoPduType(co_hdr)],
                       frag_len, sizeof(DceRpcCoContElem));
            return;
        }

        ctx_id = DceRpcCoContElemCtxId(co_hdr, ctx_elem);
        num_tsyns = DceRpcCoContElemNumTransSyntaxes(ctx_elem);
        iface = DceRpcCoContElemIface(ctx_elem);
        if_vers_maj = DceRpcCoContElemIfaceVersMaj(co_hdr, ctx_elem);
        if_vers_min = DceRpcCoContElemIfaceVersMin(co_hdr, ctx_elem);

        /* No transfer syntaxes */
        if (num_tsyns == 0)
        {
            DCE2_Alert(sd, DCE2_EVENT__CO_ZERO_TSYNS, dce2_pdu_types[DceRpcCoPduType(co_hdr)]);
            return;
        }

        DCE2_MOVE(frag_ptr, frag_len, sizeof(DceRpcCoContElem));

        /* Don't really care about the transfer syntaxes */
        for (j = 0; j < num_tsyns; j++)
        {
            if (frag_len < sizeof(DceRpcCoSynId))
            {
                DCE2_Alert(sd, DCE2_EVENT__CO_FLEN_LT_SIZE, dce2_pdu_types[DceRpcCoPduType(co_hdr)],
                           frag_len, sizeof(DceRpcCoSynId));
                return;
            }

            DCE2_MOVE(frag_ptr, frag_len, sizeof(DceRpcCoSynId));
        }

        PREPROC_PROFILE_START(dce2_pstat_co_ctx);

        /* If there is already an accepted node with in the list
         * with this ctx, just return */
        if (policy == DCE2_POLICY__SAMBA_3_0_20)
        {
            ctx_node = DCE2_ListFind(cot->ctx_ids, (void *)(uintptr_t)ctx_id);
            if ((ctx_node != NULL) && (ctx_node->state != DCE2_CO_CTX_STATE__REJECTED))
            {
                PREPROC_PROFILE_END(dce2_pstat_co_ctx);
                return;
            }
        }

        ctx_node = (DCE2_CoCtxIdNode *)DCE2_Alloc(sizeof(DCE2_CoCtxIdNode), DCE2_MEM_TYPE__CO_CTX);
        if (ctx_node == NULL)
        {
            PREPROC_PROFILE_END(dce2_pstat_co_ctx);
            return;
        }

        /* Add context id to pending queue */
        status = DCE2_QueueEnqueue(cot->pending_ctx_ids, ctx_node);
        if (status != DCE2_RET__SUCCESS)
        {
            DCE2_Free((void *)ctx_node, sizeof(DCE2_CoCtxIdNode), DCE2_MEM_TYPE__CO_CTX);
            PREPROC_PROFILE_END(dce2_pstat_co_ctx);
            return;
        }

        /* This node will get moved to the context id list upon server response */
        ctx_node->ctx_id = ctx_id;
        DCE2_CopyUuid(&ctx_node->iface, iface, DceRpcCoByteOrder(co_hdr));
        ctx_node->iface_vers_maj = if_vers_maj;
        ctx_node->iface_vers_min = if_vers_min;
        ctx_node->state = DCE2_CO_CTX_STATE__PENDING;

        PREPROC_PROFILE_END(dce2_pstat_co_ctx);

        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CO, "Added Context item to queue.\n"
                       " Context id: %u\n"
                       " Interface: %s\n"
                       " Interface major version: %u\n"
                       " Interface minor version: %u\n",
                       ctx_node->ctx_id,
                       DCE2_UuidToStr(&ctx_node->iface, DCERPC_BO_FLAG__NONE),
                       ctx_node->iface_vers_maj, ctx_node->iface_vers_min));

        switch (policy)
        {
            case DCE2_POLICY__SAMBA:
            case DCE2_POLICY__SAMBA_3_0_37:
            case DCE2_POLICY__SAMBA_3_0_22:
            case DCE2_POLICY__SAMBA_3_0_20:
                /* Samba only ever looks at one context item.  Not sure
                 * if this is an alertable offense */
                return;

            default:
                break;
        }
    }
}

/********************************************************************
 * Function: DCE2_CoBindAck()
 *
 * Handles the processing of a server bind ack or a server alter
 * context response since they share the same header.
 * Moves context id items from the pending queue into a list
 * ultimately used by the rule options and sets each context item
 * as accepted or rejected based on the server response.
 *
 * Arguments:
 *  DCE2_SsnData *
 *      Pointer to the session data structure.
 *  DCE2_CoTracker *
 *      Pointer to the relevant connection-oriented tracker.
 *  DceRpcCoHdr *
 *      Pointer to the main header in the packet data.
 *  const uint8_t *
 *      Pointer to the current processing point of the DCE/RPC
 *      pdu in the packet data.
 *  uint16_t
 *      Fragment length left in the pdu.
 *
 * Returns: None
 *
 ********************************************************************/
static void DCE2_CoBindAck(DCE2_SsnData *sd, DCE2_CoTracker *cot,
                           const DceRpcCoHdr *co_hdr, const uint8_t *frag_ptr, uint16_t frag_len)
{
    DCE2_Policy policy = DCE2_SsnGetServerPolicy(sd);
    DceRpcCoBindAck *bind_ack = (DceRpcCoBindAck *)frag_ptr;
    uint16_t sec_addr_len;
    const uint8_t *ctx_data;
    uint16_t ctx_len;
    uint16_t pad = 0;
    DceRpcCoContResultList *ctx_list;
    uint8_t num_ctx_results;
    unsigned int i;
    uint16_t max_recv_frag;
    DCE2_Ret status;

    if (frag_len < sizeof(DceRpcCoBindAck))
    {
        DCE2_Alert(sd, DCE2_EVENT__CO_FLEN_LT_SIZE, dce2_pdu_types[DceRpcCoPduType(co_hdr)],
                   frag_len, sizeof(DceRpcCoBindAck));
        return;
    }

    DCE2_MOVE(frag_ptr, frag_len, sizeof(DceRpcCoBindAck));

    /* Set what should be the maximum amount of data a client can send in a fragment */
    max_recv_frag = DceRpcCoBindAckMaxRecvFrag(co_hdr, bind_ack);
    if ((cot->max_xmit_frag == DCE2_SENTINEL) || (max_recv_frag < cot->max_xmit_frag))
        cot->max_xmit_frag = (int)max_recv_frag;

    sec_addr_len = DceRpcCoSecAddrLen(co_hdr, bind_ack);

    ctx_data = frag_ptr;
    ctx_len = frag_len;

    /* First move past secondary address */
    if (ctx_len < sec_addr_len)
    {
        DCE2_Alert(sd, DCE2_EVENT__CO_FLEN_LT_SIZE, dce2_pdu_types[DceRpcCoPduType(co_hdr)],
                   ctx_len, sec_addr_len);
        return;
    }

    DCE2_MOVE(ctx_data, ctx_len, sec_addr_len);

    /* padded to 4 octet */
    if ((sizeof(DceRpcCoBindAck) + sec_addr_len) & 3)
        pad = (4 - ((sizeof(DceRpcCoBindAck) + sec_addr_len) & 3));

    if (ctx_len < pad)
    {
        DCE2_Alert(sd, DCE2_EVENT__CO_FLEN_LT_SIZE, dce2_pdu_types[DceRpcCoPduType(co_hdr)],
                   ctx_len, pad);
        return;
    }

    DCE2_MOVE(ctx_data, ctx_len, pad);

    /* Now we're at the start of the context item results */
    if (ctx_len < sizeof(DceRpcCoContResultList))
    {
        DCE2_Alert(sd, DCE2_EVENT__CO_FLEN_LT_SIZE, dce2_pdu_types[DceRpcCoPduType(co_hdr)],
                   ctx_len, sizeof(DceRpcCoContResultList));
        return;
    }

    ctx_list = (DceRpcCoContResultList *)ctx_data;
    num_ctx_results = DceRpcCoContNumResults(ctx_list);

    DCE2_MOVE(ctx_data, ctx_len, sizeof(DceRpcCoContResultList));

    for (i = 0; i < num_ctx_results; i++)
    {
        DceRpcCoContResult *ctx_result;
        uint16_t result;
        DCE2_CoCtxIdNode *ctx_node, *existing_ctx_node;
        PROFILE_VARS;

        if (ctx_len < sizeof(DceRpcCoContResult))
        {
            DCE2_Alert(sd, DCE2_EVENT__CO_FLEN_LT_SIZE, dce2_pdu_types[DceRpcCoPduType(co_hdr)],
                       ctx_len, sizeof(DceRpcCoContResult));
            return;
        }

        ctx_result = (DceRpcCoContResult *)ctx_data;
        result = DceRpcCoContRes(co_hdr, ctx_result);

        DCE2_MOVE(ctx_data, ctx_len, sizeof(DceRpcCoContResult));

        if (DCE2_QueueIsEmpty(cot->pending_ctx_ids))
            return;

        PREPROC_PROFILE_START(dce2_pstat_co_ctx);

        /* Dequeue context item in pending queue - this will get put in the permanent
         * context id list or free'd */
        ctx_node = (DCE2_CoCtxIdNode *)DCE2_QueueDequeue(cot->pending_ctx_ids);
        if (ctx_node == NULL)
        {
            DCE2_Log(DCE2_LOG_TYPE__ERROR,
                     "%s(%d) Failed to dequeue a context id node.",
                     __FILE__, __LINE__);
            PREPROC_PROFILE_END(dce2_pstat_co_ctx);
            return;
        }

        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CO, "Adding Context item to context item list.\n"
                       " Context id: %u\n"
                       " Interface: %s\n"
                       " Interface major version: %u\n"
                       " Interface minor version: %u\n",
                       ctx_node->ctx_id,
                       DCE2_UuidToStr(&ctx_node->iface, DCERPC_BO_FLAG__NONE),
                       ctx_node->iface_vers_maj, ctx_node->iface_vers_min));

        if (result == DCERPC_CO_CONT_DEF_RESULT__ACCEPTANCE)
        {
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CO, "Server accepted context item.\n"));
            ctx_node->state = DCE2_CO_CTX_STATE__ACCEPTED;
            if (DceRpcCoPduType(co_hdr) == DCERPC_PDU_TYPE__BIND_ACK)
                cot->got_bind = 1;
        }
        else
        {
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CO, "Server rejected context item.\n"));
            ctx_node->state = DCE2_CO_CTX_STATE__REJECTED;
            cot->got_bind = 0;
        }

        existing_ctx_node =
            (DCE2_CoCtxIdNode *)DCE2_ListFind(cot->ctx_ids, (void *)(uintptr_t)ctx_node->ctx_id);

        if (existing_ctx_node != NULL)
        {
            switch (policy)
            {
                case DCE2_POLICY__WIN2000:
                case DCE2_POLICY__WIN2003:
                case DCE2_POLICY__WINXP:
                case DCE2_POLICY__WINVISTA:
                case DCE2_POLICY__WIN2008:
                case DCE2_POLICY__WIN7:
                    if (ctx_node->state == DCE2_CO_CTX_STATE__REJECTED)
                        break;

                    if (existing_ctx_node->state == DCE2_CO_CTX_STATE__REJECTED)
                    {
                        existing_ctx_node->ctx_id = ctx_node->ctx_id;
                        DCE2_CopyUuid(&existing_ctx_node->iface, &ctx_node->iface, DCERPC_BO_FLAG__NONE);
                        existing_ctx_node->iface_vers_maj = ctx_node->iface_vers_maj;
                        existing_ctx_node->iface_vers_min = ctx_node->iface_vers_min;
                        existing_ctx_node->state = ctx_node->state;
                    }

                    break;

                case DCE2_POLICY__SAMBA:
                case DCE2_POLICY__SAMBA_3_0_37:
                case DCE2_POLICY__SAMBA_3_0_22:
                case DCE2_POLICY__SAMBA_3_0_20:
                    /* Samba actually alters the context.  Windows keeps the old */
                    if (ctx_node->state != DCE2_CO_CTX_STATE__REJECTED)
                    {
                        existing_ctx_node->ctx_id = ctx_node->ctx_id;
                        DCE2_CopyUuid(&existing_ctx_node->iface, &ctx_node->iface, DCERPC_BO_FLAG__NONE);
                        existing_ctx_node->iface_vers_maj = ctx_node->iface_vers_maj;
                        existing_ctx_node->iface_vers_min = ctx_node->iface_vers_min;
                        existing_ctx_node->state = ctx_node->state;
                    }

                    break;

                default:
                    break;
            }

            DCE2_Free((void *)ctx_node, sizeof(DCE2_CoCtxIdNode), DCE2_MEM_TYPE__CO_CTX);
        }
        else
        {
            status = DCE2_ListInsert(cot->ctx_ids, (void *)(uintptr_t)ctx_node->ctx_id, (void *)ctx_node);
            if (status != DCE2_RET__SUCCESS)
            {
                /* Hit memcap */
                DCE2_Free((void *)ctx_node, sizeof(DCE2_CoCtxIdNode), DCE2_MEM_TYPE__CO_CTX);
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CO,
                            "Failed to add context id node to list.\n"));
                PREPROC_PROFILE_END(dce2_pstat_co_ctx);
                return;
            }
        }

        PREPROC_PROFILE_END(dce2_pstat_co_ctx);
    }
}

/********************************************************************
 * Function: DCE2_CoRequest()
 *
 * Handles a DCE/RPC request from the client.  This is were the
 * client actually asks the server to do stuff on it's behalf.
 * If it's a first/last fragment, set relevant rule option
 * data and return. If it's a true fragment, do some target
 * based futzing to set the right opnum and context id for
 * the to be reassembled packet.
 *
 * Arguments:
 *  DCE2_SsnData *
 *      Pointer to the session data structure.
 *  DCE2_CoTracker *
 *      Pointer to the relevant connection-oriented tracker.
 *  DceRpcCoHdr *
 *      Pointer to the main header in the packet data.
 *  const uint8_t *
 *      Pointer to the current processing point of the DCE/RPC
 *      pdu in the packet data.
 *  uint16_t
 *      Fragment length left in the pdu.
 *
 * Returns: None
 *
 ********************************************************************/
static void DCE2_CoRequest(DCE2_SsnData *sd, DCE2_CoTracker *cot,
                           const DceRpcCoHdr *co_hdr, const uint8_t *frag_ptr, uint16_t frag_len)
{
    DceRpcCoRequest *rhdr = (DceRpcCoRequest *)frag_ptr;
    uint16_t req_size = sizeof(DceRpcCoRequest);
    DCE2_Policy policy = DCE2_SsnGetServerPolicy(sd);

    /* Account for possible object uuid */
    if (DceRpcCoObjectFlag(co_hdr))
        req_size += sizeof(Uuid);

    if (frag_len < req_size)
    {
        DCE2_Alert(sd, DCE2_EVENT__CO_FLEN_LT_SIZE, dce2_pdu_types[DceRpcCoPduType(co_hdr)],
                   frag_len, req_size);
        return;
    }

    switch (policy)
    {
        /* After 3.0.37 up to 3.5.2 byte order of stub data is always
         * interpreted as little endian */
        case DCE2_POLICY__SAMBA:
            cot->data_byte_order = DCERPC_BO_FLAG__LITTLE_ENDIAN;
            break;

        case DCE2_POLICY__SAMBA_3_0_37:
        case DCE2_POLICY__SAMBA_3_0_22:
        case DCE2_POLICY__SAMBA_3_0_20:
            cot->data_byte_order = DceRpcCoByteOrder(co_hdr);
            break;

        default:
            break;
    }

    /* Move past header */
    DCE2_MOVE(frag_ptr, frag_len, req_size);

    /* If for some reason we had some fragments queued */
    if (DceRpcCoFirstFrag(co_hdr) && !DceRpcCoLastFrag(co_hdr)
            && !DCE2_BufferIsEmpty(cot->frag_tracker.cli_stub_buf))
    {
        DCE2_CoFragReassemble(sd, cot);
        DCE2_BufferEmpty(cot->frag_tracker.cli_stub_buf);
        DCE2_CoResetFragTracker(&cot->frag_tracker);
    }

    cot->stub_data = frag_ptr;
    cot->opnum = DceRpcCoOpnum(co_hdr, rhdr);
    cot->ctx_id = DceRpcCoCtxId(co_hdr, rhdr);
    cot->call_id = DceRpcCoCallId(co_hdr);

    if (DceRpcCoFirstFrag(co_hdr) && DceRpcCoLastFrag(co_hdr))
    {
        int auth_len = DCE2_CoGetAuthLen(sd, co_hdr, frag_ptr, frag_len);
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CO, "First and last fragment.\n"));
        if (auth_len == -1)
            return;
        DCE2_CoSetRopts(sd, cot, co_hdr);
    }
    else
    {
        DCE2_CoFragTracker *ft = &cot->frag_tracker;
        int auth_len = DCE2_CoGetAuthLen(sd, co_hdr, frag_ptr, frag_len);

        dce2_stats.co_req_fragments++;

#ifdef DEBUG_MSGS
        DCE2_DEBUG_CODE(DCE2_DEBUG__CO,
            if (DceRpcCoFirstFrag(co_hdr)) DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CO, "First fragment.\n"));
            else if (DceRpcCoLastFrag(co_hdr)) DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CO, "Last fragment.\n"));
            else DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CO, "Middle fragment.\n"));
            DCE2_PrintPktData(frag_ptr, frag_len););
#endif

        if (auth_len == -1)
            return;

        if (DCE2_BufferIsEmpty(ft->cli_stub_buf))
        {
            ft->expected_opnum = cot->opnum;
            ft->expected_ctx_id = cot->ctx_id;
            ft->expected_call_id = cot->call_id;
        }
        else
        {
            /* Don't return for these, because we can still process and servers
             * will still accept and deal with the anomalies in their own way */
            if ((ft->expected_opnum != DCE2_SENTINEL) &&
                (ft->expected_opnum != cot->opnum))
            {
                DCE2_Alert(sd, DCE2_EVENT__CO_FRAG_DIFF_OPNUM,
                           cot->opnum, ft->expected_opnum);
            }

            if ((ft->expected_ctx_id != DCE2_SENTINEL) &&
                (ft->expected_ctx_id != cot->ctx_id))
            {
                DCE2_Alert(sd, DCE2_EVENT__CO_FRAG_DIFF_CTX_ID,
                           cot->ctx_id, ft->expected_ctx_id);
            }

            if ((ft->expected_call_id != DCE2_SENTINEL) &&
                (ft->expected_call_id != cot->call_id))
            {
                DCE2_Alert(sd, DCE2_EVENT__CO_FRAG_DIFF_CALL_ID,
                           cot->call_id, ft->expected_call_id);
            }
        }

        /* Possibly set opnum in frag tracker */
        switch (policy)
        {
            case DCE2_POLICY__WIN2000:
            case DCE2_POLICY__WIN2003:
            case DCE2_POLICY__WINXP:
            case DCE2_POLICY__SAMBA:
            case DCE2_POLICY__SAMBA_3_0_37:
            case DCE2_POLICY__SAMBA_3_0_22:
            case DCE2_POLICY__SAMBA_3_0_20:
                if (DceRpcCoLastFrag(co_hdr))
                    ft->opnum = cot->opnum;
                break;

            case DCE2_POLICY__WINVISTA:
            case DCE2_POLICY__WIN2008:
            case DCE2_POLICY__WIN7:
                if (DceRpcCoFirstFrag(co_hdr))
                    ft->opnum = cot->opnum;
                break;

            default:
                DCE2_Log(DCE2_LOG_TYPE__ERROR,
                         "%s(%d) Invalid policy: %d",
                         __FILE__, __LINE__, policy);
                break;
        }

        /* Possibly set context id in frag tracker */
        switch (policy)
        {
            case DCE2_POLICY__WIN2000:
            case DCE2_POLICY__WIN2003:
            case DCE2_POLICY__WINXP:
            case DCE2_POLICY__WINVISTA:
            case DCE2_POLICY__WIN2008:
            case DCE2_POLICY__WIN7:
                if (DceRpcCoFirstFrag(co_hdr))
                {
                    ft->ctx_id = cot->ctx_id;
                }
                else if ((ft->expected_call_id != DCE2_SENTINEL) &&
                         (ft->expected_call_id != cot->call_id))
                {
                    /* Server won't accept frag */
                    return;
                }

                break;

            case DCE2_POLICY__SAMBA:
            case DCE2_POLICY__SAMBA_3_0_37:
            case DCE2_POLICY__SAMBA_3_0_22:
            case DCE2_POLICY__SAMBA_3_0_20:
                if (DceRpcCoLastFrag(co_hdr))
                {
                    ft->ctx_id = cot->ctx_id;
                }

                break;

            default:
                DCE2_Log(DCE2_LOG_TYPE__ERROR,
                         "%s(%d) Invalid policy: %d",
                         __FILE__, __LINE__, policy);
                break;
        }

        DCE2_CoSetRopts(sd, cot, co_hdr);

        /* If we're configured to do defragmentation */
        if (DCE2_GcDceDefrag())
        {
            /* Don't want to include authentication data in fragment */
            DCE2_CoHandleFrag(sd, cot, co_hdr, frag_ptr,
                    (uint16_t)(frag_len - (uint16_t)auth_len));
        }
    }
}

/********************************************************************
 * Function: DCE2_CoResponse()
 *
 * Handles a DCE/RPC response from the server.
 * Samba responds to SMB bind write, request write before read with
 * a response to the request and doesn't send a bind ack.  Get the
 * context id from the pending context id list and put in stable
 * list.
 *
 * Arguments:
 *  DCE2_SsnData *
 *      Pointer to the session data structure.
 *  DCE2_CoTracker *
 *      Pointer to the relevant connection-oriented tracker.
 *  DceRpcCoHdr *
 *      Pointer to the main header in the packet data.
 *  const uint8_t *
 *      Pointer to the current processing point of the DCE/RPC
 *      pdu in the packet data.
 *  uint16_t
 *      Fragment length left in the pdu.
 *
 * Returns: None
 *
 ********************************************************************/
static void DCE2_CoResponse(DCE2_SsnData *sd, DCE2_CoTracker *cot,
                            const DceRpcCoHdr *co_hdr, const uint8_t *frag_ptr, uint16_t frag_len)
{
    DceRpcCoResponse *rhdr = (DceRpcCoResponse *)frag_ptr;
    uint16_t ctx_id;
    DCE2_Policy policy = DCE2_SsnGetServerPolicy(sd);

    if (frag_len < sizeof(DceRpcCoResponse))
    {
        DCE2_Alert(sd, DCE2_EVENT__CO_FLEN_LT_SIZE, dce2_pdu_types[DceRpcCoPduType(co_hdr)],
                   frag_len, sizeof(DceRpcCoResponse));
        return;
    }

    switch (policy)
    {
        case DCE2_POLICY__SAMBA:
            cot->data_byte_order = DCERPC_BO_FLAG__LITTLE_ENDIAN;
            break;

        case DCE2_POLICY__SAMBA_3_0_37:
        case DCE2_POLICY__SAMBA_3_0_22:
        case DCE2_POLICY__SAMBA_3_0_20:
            cot->data_byte_order = DceRpcCoByteOrder(co_hdr);
            break;

        default:
            break;
    }

    ctx_id = DceRpcCoCtxIdResp(co_hdr, rhdr);

    /* If pending queue is not empty, add this context id as accepted and all
     * others as pending */
    while (!DCE2_QueueIsEmpty(cot->pending_ctx_ids))
    {
        DCE2_Ret status;
        DCE2_CoCtxIdNode *ctx_node = (DCE2_CoCtxIdNode *)DCE2_QueueDequeue(cot->pending_ctx_ids);

        if (ctx_node == NULL)
        {
            DCE2_Log(DCE2_LOG_TYPE__ERROR,
                     "%s(%d) Failed to dequeue a context id node.",
                     __FILE__, __LINE__);
            return;
        }

        if (ctx_node->ctx_id == ctx_id)
            ctx_node->state = DCE2_CO_CTX_STATE__ACCEPTED;

        status = DCE2_ListInsert(cot->ctx_ids, (void *)(uintptr_t)ctx_node->ctx_id, (void *)ctx_node);
        if (status != DCE2_RET__SUCCESS)
        {
            /* Might be a duplicate in there already.  If there is we would have used it
             * anyway before looking at the pending queue.  Just get rid of it */
            DCE2_Free((void *)ctx_node, sizeof(DCE2_CoCtxIdNode), DCE2_MEM_TYPE__CO_CTX);
            return;
        }
    }

    /* Move past header */
    DCE2_MOVE(frag_ptr, frag_len, sizeof(DceRpcCoResponse));

    /* If for some reason we had some fragments queued */
    if (DceRpcCoFirstFrag(co_hdr) && !DCE2_BufferIsEmpty(cot->frag_tracker.srv_stub_buf))
    {
        DCE2_CoFragReassemble(sd, cot);
        DCE2_BufferEmpty(cot->frag_tracker.srv_stub_buf);
        DCE2_CoResetFragTracker(&cot->frag_tracker);
    }

    cot->stub_data = frag_ptr;
    /* Opnum not in response header - have to use previous client's */
    cot->ctx_id = ctx_id;
    cot->call_id = DceRpcCoCallId(co_hdr);

    if (DceRpcCoFirstFrag(co_hdr) && DceRpcCoLastFrag(co_hdr))
    {
        int auth_len = DCE2_CoGetAuthLen(sd, co_hdr, frag_ptr, frag_len);
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CO, "First and last fragment.\n"));
        if (auth_len == -1)
            return;
        DCE2_CoSetRopts(sd, cot, co_hdr);
    }
    else
    {
        //DCE2_CoFragTracker *ft = &cot->frag_tracker;
        int auth_len = DCE2_CoGetAuthLen(sd, co_hdr, frag_ptr, frag_len);

        dce2_stats.co_resp_fragments++;
        if (auth_len == -1)
            return;

#if 0
        /* TBD - Target based foo */
        /* Don't return for these, because we can still process */
        if ((ft->expected_opnum != DCE2_SENTINEL) &&
            (ft->expected_opnum != cot->opnum))
        {
            DCE2_Alert(sd, DCE2_EVENT__CO_FRAG_DIFF_OPNUM,
                       cot->opnum, ft->expected_opnum);
        }

        if ((ft->expected_ctx_id != DCE2_SENTINEL) &&
            (ft->expected_ctx_id != cot->ctx_id))
        {
            DCE2_Alert(sd, DCE2_EVENT__CO_FRAG_DIFF_CTX_ID,
                       cot->ctx_id, ft->expected_ctx_id);
        }

        if ((ft->expected_call_id != DCE2_SENTINEL) &&
            (ft->expected_call_id != cot->call_id))
        {
            DCE2_Alert(sd, DCE2_EVENT__CO_FRAG_DIFF_CALL_ID,
                       cot->call_id, ft->expected_call_id);
        }
#endif

        DCE2_CoSetRopts(sd, cot, co_hdr);

        /* If we're configured to do defragmentation */
        if (DCE2_GcDceDefrag())
        {
            DCE2_CoHandleFrag(sd, cot, co_hdr, frag_ptr,
                    (uint16_t)(frag_len - (uint16_t)auth_len));
        }
    }
}

/********************************************************************
 * Function: DCE2_CoHandleFrag()
 *
 * Handles adding a fragment to the defragmentation buffer.
 * Does overflow checking.  Maximum length of fragmentation buffer
 * is based on the maximum packet length Snort can handle.
 *
 * Arguments:
 *  DCE2_SsnData *
 *      Pointer to the session data structure.
 *  DCE2_CoTracker *
 *      Pointer to the relevant connection-oriented tracker.
 *  DceRpcCoHdr *
 *      Pointer to the main header in the packet data.
 *  const uint8_t *
 *      Pointer to the current processing point of the DCE/RPC
 *      pdu in the packet data.
 *  uint16_t
 *      Fragment length left in the pdu.
 *
 * Returns: None
 *
 ********************************************************************/
static void DCE2_CoHandleFrag(DCE2_SsnData *sd, DCE2_CoTracker *cot,
                              const DceRpcCoHdr *co_hdr, const uint8_t *frag_ptr, uint16_t frag_len)
{
    DCE2_Buffer *frag_buf = DCE2_CoGetFragBuf(sd, &cot->frag_tracker);
    uint32_t size = (frag_len < DCE2_CO__MIN_ALLOC_SIZE) ? DCE2_CO__MIN_ALLOC_SIZE : frag_len;
    uint16_t max_frag_data;
    DCE2_BufferMinAddFlag mflag = DCE2_BUFFER_MIN_ADD_FLAG__USE;
    DCE2_Ret status;
    PROFILE_VARS;

    PREPROC_PROFILE_START(dce2_pstat_co_frag);

    if (DCE2_SsnFromClient(sd->wire_pkt))
    {
        if (frag_len > dce2_stats.co_cli_max_frag_size)
            dce2_stats.co_cli_max_frag_size = frag_len;

        if (dce2_stats.co_cli_min_frag_size == 0 || frag_len < dce2_stats.co_cli_min_frag_size)
            dce2_stats.co_cli_min_frag_size = frag_len;
    }
    else
    {
        if (frag_len > dce2_stats.co_srv_max_frag_size)
            dce2_stats.co_srv_max_frag_size = frag_len;

        if (dce2_stats.co_srv_min_frag_size == 0 || frag_len < dce2_stats.co_srv_min_frag_size)
            dce2_stats.co_srv_min_frag_size = frag_len;
    }

    if (frag_buf == NULL)
    {
        if (DCE2_SsnFromServer(sd->wire_pkt))
        {
            cot->frag_tracker.srv_stub_buf =
                DCE2_BufferNew(size, DCE2_CO__MIN_ALLOC_SIZE, DCE2_MEM_TYPE__CO_FRAG);
            frag_buf = cot->frag_tracker.srv_stub_buf;
        }
        else
        {
            cot->frag_tracker.cli_stub_buf =
                DCE2_BufferNew(size, DCE2_CO__MIN_ALLOC_SIZE, DCE2_MEM_TYPE__CO_FRAG);
            frag_buf = cot->frag_tracker.cli_stub_buf;
        }

        if (frag_buf == NULL)
        {
            PREPROC_PROFILE_START(dce2_pstat_co_frag);
            return;
        }
    }

    /* If there's already data in the buffer and this is a first frag
     * we probably missed packets */
    if (DceRpcCoFirstFrag(co_hdr) && !DCE2_BufferIsEmpty(frag_buf))
    {
        DCE2_CoResetFragTracker(&cot->frag_tracker);
        DCE2_BufferEmpty(frag_buf);
    }

    /* Check for potential overflow */
    if (sd->trans == DCE2_TRANS_TYPE__SMB)
        max_frag_data = DCE2_GetRpktMaxData(sd, DCE2_RPKT_TYPE__SMB_CO_FRAG);
    else
        max_frag_data = DCE2_GetRpktMaxData(sd, DCE2_RPKT_TYPE__TCP_CO_FRAG);

    if (DCE2_GcMaxFrag() && (frag_len > DCE2_GcMaxFragLen()))
        frag_len = DCE2_GcMaxFragLen();

    if ((DCE2_BufferLength(frag_buf) + frag_len) > max_frag_data)
        frag_len = max_frag_data - (uint16_t)DCE2_BufferLength(frag_buf);

    if (frag_len != 0)
    {
        /* If it's the last fragment we're going to flush so just alloc
         * exactly what we need ... or if there is more data than can fit
         * in the reassembly buffer */
        if (DceRpcCoLastFrag(co_hdr) || (DCE2_BufferLength(frag_buf) == max_frag_data))
            mflag = DCE2_BUFFER_MIN_ADD_FLAG__IGNORE;

        status = DCE2_BufferAddData(frag_buf, frag_ptr,
                frag_len, DCE2_BufferLength(frag_buf), mflag);

        if (status != DCE2_RET__SUCCESS)
        {
            PREPROC_PROFILE_END(dce2_pstat_co_frag);

            /* Either hit memcap or a memcpy failed - reassemble */
            DCE2_CoFragReassemble(sd, cot);
            DCE2_BufferEmpty(frag_buf);
            return;
        }
    }

    PREPROC_PROFILE_END(dce2_pstat_co_frag);

    /* Reassemble if we got a last frag ... */
    if (DceRpcCoLastFrag(co_hdr))
    {
        DCE2_CoFragReassemble(sd, cot);
        DCE2_BufferEmpty(frag_buf);

        /* Set this for the server response since response doesn't
         * contain client opnum used */
        cot->opnum = cot->frag_tracker.opnum;
        DCE2_CoResetFragTracker(&cot->frag_tracker);

        /* Return early - rule opts will be set in reassembly handler */
        return;
    }
    else if (DCE2_BufferLength(frag_buf) == max_frag_data)
    {
        /* ... or can't fit any more data in the buffer
         * Don't reset frag tracker */
        DCE2_CoFragReassemble(sd, cot);
        DCE2_BufferEmpty(frag_buf);
        return;
    }
}

/********************************************************************
 * Function: DCE2_CoFragReassemble()
 *
 * Wrapper for the generic reassembly function.  Calls generic
 * reassembly function specifying that we want to do fragmentation
 * reassembly.
 *
 * Arguments:
 *  DCE2_SsnData *
 *      Pointer to the session data structure.
 *  DCE2_CoTracker *
 *      Pointer to the relevant connection-oriented tracker.
 *
 * Returns: None
 *
 ********************************************************************/
static inline void DCE2_CoFragReassemble(DCE2_SsnData *sd, DCE2_CoTracker *cot)
{
    DCE2_CoReassemble(sd, cot, DCE2_CO_RPKT_TYPE__FRAG);
}

/********************************************************************
 * Function: DCE2_CoReassemble()
 *
 * Gets a reassemly packet based on the transport and the type of
 * reassembly we want to do.  Sets rule options and calls detect
 * on the reassembled packet.
 *
 * Arguments:
 *  DCE2_SsnData *
 *      Pointer to the session data structure.
 *  DCE2_CoTracker *
 *      Pointer to the relevant connection-oriented tracker.
 *  DCE2_CoRpktType
 *      Specifies whether we want to do segmenation, fragmentation
 *      or fragmentation and segmentation reassembly.
 *
 * Returns: None
 *
 ********************************************************************/
static void DCE2_CoReassemble(DCE2_SsnData *sd, DCE2_CoTracker *cot, DCE2_CoRpktType co_rtype)
{
    DCE2_RpktType rpkt_type;
    DceRpcCoHdr *co_hdr;
    SFSnortPacket *rpkt;
    int smb_hdr_len = DCE2_SsnFromClient(sd->wire_pkt) ? DCE2_MOCK_HDR_LEN__SMB_CLI : DCE2_MOCK_HDR_LEN__SMB_SRV;
    int co_hdr_len = DCE2_SsnFromClient(sd->wire_pkt) ? DCE2_MOCK_HDR_LEN__CO_CLI : DCE2_MOCK_HDR_LEN__CO_SRV;
    PROFILE_VARS;

    PREPROC_PROFILE_START(dce2_pstat_co_reass);

    rpkt = DCE2_CoGetRpkt(sd, cot, co_rtype, &rpkt_type);
    if (rpkt == NULL)
    {
        DCE2_Log(DCE2_LOG_TYPE__ERROR,
                 "%s(%d) Could not create DCE/RPC frag reassembled packet.\n",
                 __FILE__, __LINE__);
        PREPROC_PROFILE_END(dce2_pstat_co_reass);
        return;
    }

    switch (rpkt_type)
    {
        case DCE2_RPKT_TYPE__SMB_CO_FRAG:
        case DCE2_RPKT_TYPE__SMB_CO_SEG:
            DCE2_SmbSetRdata((DCE2_SmbSsnData *)sd, (uint8_t *)rpkt->payload,
                             (uint16_t)(rpkt->payload_size - smb_hdr_len));

            if (rpkt_type == DCE2_RPKT_TYPE__SMB_CO_FRAG)
            {
                DCE2_CoSetRdata(sd, cot, (uint8_t *)rpkt->payload + smb_hdr_len,
                                (uint16_t)(rpkt->payload_size - (smb_hdr_len + co_hdr_len)));

                if (DCE2_SsnFromClient(sd->wire_pkt))
                    dce2_stats.co_cli_frag_reassembled++;
                else
                    dce2_stats.co_srv_frag_reassembled++;
            }
            else
            {
                if (DCE2_SsnFromClient(sd->wire_pkt))
                    dce2_stats.co_cli_seg_reassembled++;
                else
                    dce2_stats.co_srv_seg_reassembled++;
            }

            co_hdr = (DceRpcCoHdr *)(rpkt->payload + smb_hdr_len);
            cot->stub_data = rpkt->payload + smb_hdr_len + co_hdr_len;

            break;

        case DCE2_RPKT_TYPE__TCP_CO_FRAG:
        case DCE2_RPKT_TYPE__TCP_CO_SEG:
            if (rpkt_type == DCE2_RPKT_TYPE__TCP_CO_FRAG)
            {
                DCE2_CoSetRdata(sd, cot, (uint8_t *)rpkt->payload, (uint16_t)(rpkt->payload_size - co_hdr_len));

                if (DCE2_SsnFromClient(sd->wire_pkt))
                    dce2_stats.co_cli_frag_reassembled++;
                else
                    dce2_stats.co_srv_frag_reassembled++;
            }
            else
            {
                if (DCE2_SsnFromClient(sd->wire_pkt))
                    dce2_stats.co_cli_seg_reassembled++;
                else
                    dce2_stats.co_srv_seg_reassembled++;
            }

            co_hdr = (DceRpcCoHdr *)rpkt->payload;
            cot->stub_data = rpkt->payload + co_hdr_len;

            break;

        default:
            DCE2_Log(DCE2_LOG_TYPE__ERROR,
                     "%s(%d) Invalid rpkt type: %d",
                     __FILE__, __LINE__, rpkt_type);
            PREPROC_PROFILE_END(dce2_pstat_co_reass);
            return;
    }

    PREPROC_PROFILE_END(dce2_pstat_co_reass);

    /* Push packet onto stack */
    if (DCE2_PushPkt(rpkt) != DCE2_RET__SUCCESS)
    {
        DCE2_Log(DCE2_LOG_TYPE__ERROR,
                 "%s(%d) Failed to push packet onto packet stack.",
                 __FILE__, __LINE__);
        return;
    }

    DCE2_CoSetRopts(sd, cot, co_hdr);

    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CO, "Reassembled CO fragmented packet:\n"));
    DCE2_DEBUG_CODE(DCE2_DEBUG__CO, DCE2_PrintPktData(rpkt->payload, rpkt->payload_size););

    DCE2_Detect(sd);
    DCE2_PopPkt();

    co_reassembled = 1;
}

/********************************************************************
 * Function: DCE2_CoSetIface()
 *
 * Sets the interface UUID for the rules options.  Looks in the
 * context id list.  If nothing found there, it looks in the pending
 * list (in case we never saw the server response because of
 * missed packets) to see if something is there.
 *
 * Arguments:
 *  DCE2_SsnData *
 *      Pointer to the session data structure.
 *  DCE2_CoTracker *
 *      Pointer to the relevant connection-oriented tracker.
 *  uint16_t
 *      The context id to use for the lookup.
 *
 * Returns:
 *  DCE2_Ret
 *      DCE2_RET__ERROR if the interface UUID could not be found
 *          based on the context id passed in.
 *      DCE2_RET__SUCESS if the interface UUID could be found and
 *          the appropriate rule options could be set.
 *
 ********************************************************************/
static DCE2_Ret DCE2_CoSetIface(DCE2_SsnData *sd, DCE2_CoTracker *cot, uint16_t ctx_id)
{
    DCE2_CoCtxIdNode *ctx_id_node;
    PROFILE_VARS;

    /* This should be set if we've gotten a Bind */
    if (cot->ctx_ids == NULL)
        return DCE2_RET__ERROR;

    PREPROC_PROFILE_START(dce2_pstat_co_ctx);

    ctx_id_node = (DCE2_CoCtxIdNode *)DCE2_ListFind(cot->ctx_ids, (void *)(uintptr_t)ctx_id);
    if (ctx_id_node == NULL)  /* context id not found in list */
    {
        /* See if it's in the queue.  An easy evasion would be to stagger the writes
         * and reads such that we see a request before seeing the server bind ack */
        if (cot->pending_ctx_ids != NULL)
        {
            for (ctx_id_node = (DCE2_CoCtxIdNode *)DCE2_QueueFirst(cot->pending_ctx_ids);
                 ctx_id_node != NULL;
                 ctx_id_node = (DCE2_CoCtxIdNode *)DCE2_QueueNext(cot->pending_ctx_ids))
            {
                if (ctx_id_node->ctx_id == ctx_id)
                    break;
            }
        }

        if (ctx_id_node == NULL)
        {
            PREPROC_PROFILE_END(dce2_pstat_co_ctx);
            return DCE2_RET__ERROR;
        }
    }

    if (ctx_id_node->state == DCE2_CO_CTX_STATE__REJECTED)
    {
        PREPROC_PROFILE_END(dce2_pstat_co_ctx);
        return DCE2_RET__ERROR;
    }

    DCE2_CopyUuid(&sd->ropts.iface, &ctx_id_node->iface, DCERPC_BO_FLAG__NONE);
    sd->ropts.iface_vers_maj = ctx_id_node->iface_vers_maj;
    sd->ropts.iface_vers_min = ctx_id_node->iface_vers_min;

    PREPROC_PROFILE_END(dce2_pstat_co_ctx);

    return DCE2_RET__SUCCESS;
}

/********************************************************************
 * Function: DCE2_CoCtxCompare()
 *
 * Callback to context id list for finding the right interface
 * UUID node.  Values passed in are context ids which are used as
 * the keys for the list.
 *
 * Arguments:
 *  const void *
 *      First context id to compare.
 *  const void *
 *      Second context id to compare.
 *
 * Returns:
 *  int
 *       0 if first value equals second value
 *      -1 if first value does not equal the second value
 *
 ********************************************************************/
static int DCE2_CoCtxCompare(const void *a, const void *b)
{
    int x = (int)(uintptr_t)a;
    int y = (int)(uintptr_t)b;

    if (x == y)
        return 0;

    /* Only care about equality for finding */
    return -1;
}

/********************************************************************
 * Function: DCE2_CoCtxFree()
 *
 * Callback to context id list for freeing context id nodes in
 * the list.
 *
 * Arguments:
 *  void *
 *      Context id node to free.
 *
 * Returns: None
 *
 ********************************************************************/
static void DCE2_CoCtxFree(void *data)
{
    if (data == NULL)
        return;

    DCE2_Free(data, sizeof(DCE2_CoCtxIdNode), DCE2_MEM_TYPE__CO_CTX);
}

/********************************************************************
 * Function: DCE2_CoInitTracker()
 *
 * Initializes fields in the connection-oriented tracker to
 * sentinels.  Many decisions are made based on whether or not
 * these fields have been set.
 *
 * Arguments:
 *  DCE2_CoTracker *
 *      Pointer to the relevant connection-oriented tracker.
 *
 * Returns: None
 *
 ********************************************************************/
void DCE2_CoInitTracker(DCE2_CoTracker *cot)
{
    if (cot == NULL)
        return;

    cot->max_xmit_frag = DCE2_SENTINEL;
    cot->data_byte_order = DCE2_SENTINEL;
    cot->ctx_id = DCE2_SENTINEL;
    cot->opnum = DCE2_SENTINEL;
    cot->call_id = DCE2_SENTINEL;
    cot->stub_data = NULL;
    cot->got_bind = 0;

    cot->frag_tracker.opnum = DCE2_SENTINEL;
    cot->frag_tracker.ctx_id = DCE2_SENTINEL;
    cot->frag_tracker.expected_call_id = DCE2_SENTINEL;
    cot->frag_tracker.expected_opnum = DCE2_SENTINEL;
    cot->frag_tracker.expected_ctx_id = DCE2_SENTINEL;
}

/********************************************************************
 * Function: DCE2_CoResetTracker()
 *
 * Resets frag tracker fields after having reassembled.
 *
 * Arguments:
 *  DCE2_CoFragTracker *
 *      Pointer to the relevant connection-oriented frag tracker.
 *
 * Returns: None
 *
 ********************************************************************/
static inline void DCE2_CoResetFragTracker(DCE2_CoFragTracker *ft)
{
    if (ft == NULL)
        return;

    ft->opnum = DCE2_SENTINEL;
    ft->ctx_id = DCE2_SENTINEL;
    ft->expected_call_id = DCE2_SENTINEL;
    ft->expected_ctx_id = DCE2_SENTINEL;
    ft->expected_opnum = DCE2_SENTINEL;
}

/********************************************************************
 * Function: DCE2_CoResetTracker()
 *
 * Resets fields that are transient for requests after the bind or
 * alter context.  The context id and opnum are dependent on the
 * request and in the case of fragmented requests are set until all
 * fragments are received.  If we got a full request or all of the
 * fragments, these should be reset.
 *
 * Arguments:
 *  DCE2_CoTracker *
 *      Pointer to the relevant connection-oriented tracker.
 *
 * Returns: None
 *
 ********************************************************************/
static inline void DCE2_CoResetTracker(DCE2_CoTracker *cot)
{
    if (cot == NULL)
        return;

    cot->ctx_id = DCE2_SENTINEL;
    cot->opnum = DCE2_SENTINEL;
    cot->call_id = DCE2_SENTINEL;
    cot->stub_data = NULL;

    DCE2_CoResetFragTracker(&cot->frag_tracker);
}

/********************************************************************
 * Function: DCE2_CoCleanTracker()
 *
 * Destroys all dynamically allocated data associated with
 * connection-oriented tracker.
 *
 * Arguments:
 *  DCE2_CoTracker *
 *      Pointer to the relevant connection-oriented tracker.
 *
 * Returns: None
 *
 ********************************************************************/
void DCE2_CoCleanTracker(DCE2_CoTracker *cot)
{
    if (cot == NULL)
        return;

    DCE2_BufferDestroy(cot->frag_tracker.cli_stub_buf);
    cot->frag_tracker.cli_stub_buf = NULL;

    DCE2_BufferDestroy(cot->frag_tracker.srv_stub_buf);
    cot->frag_tracker.srv_stub_buf = NULL;

    DCE2_BufferDestroy(cot->cli_seg.buf);
    cot->cli_seg.buf = NULL;

    DCE2_BufferDestroy(cot->srv_seg.buf);
    cot->srv_seg.buf = NULL;

    DCE2_ListDestroy(cot->ctx_ids);
    cot->ctx_ids = NULL;

    DCE2_QueueDestroy(cot->pending_ctx_ids);
    cot->pending_ctx_ids = NULL;

    DCE2_CoInitTracker(cot);
}

/********************************************************************
 * Function: DCE2_CoEraseCtxIds()
 *
 * Empties out the context id list and the pending context id
 * queue.  Does not free the list and queue - might need to still
 * use them.
 *
 * Arguments:
 *  DCE2_CoTracker *
 *      Pointer to the relevant connection-oriented tracker.
 *
 * Returns: None
 *
 ********************************************************************/
static inline void DCE2_CoEraseCtxIds(DCE2_CoTracker *cot)
{
    if (cot == NULL)
        return;

    DCE2_QueueEmpty(cot->pending_ctx_ids);
    DCE2_ListEmpty(cot->ctx_ids);
}

/********************************************************************
 * Function: DCE2_CoSegAlert()
 *
 * We have to alert with the appropriate data so the alert actually
 * makes sense.  In this case, the alert was generated based on
 * data in a segmentation buffer.  We have to create a packet
 * using the segmentation buffer before actually alerting.
 *
 * Arguments:
 *  DCE2_SsnData *
 *      Pointer to the session data structure.
 *  DCE2_CoTracker *
 *      Pointer to the relevant connection-oriented tracker.
 *  DCE2_Event
 *      The event that was generated.
 *
 * Returns: None
 *
 ********************************************************************/
static inline void DCE2_CoSegAlert(DCE2_SsnData *sd, DCE2_CoTracker *cot, DCE2_Event event)
{
    SFSnortPacket *rpkt;
    DCE2_Buffer *buf;
    DceRpcCoHdr *co_hdr;
    uint16_t frag_len;
    DceRpcPduType pdu_type;

    if (DCE2_SsnFromClient(sd->wire_pkt))
        buf = cot->cli_seg.buf;
    else
        buf = cot->srv_seg.buf;

    /* This should be called from the desegmentation code after there is
     * enough data for a connection oriented header.  All of the alerts
     * here require a header. */
    if (DCE2_BufferIsEmpty(buf) ||
        (DCE2_BufferLength(buf) < sizeof(DceRpcCoHdr)))
    {
        return;
    }

    rpkt = DCE2_CoGetSegRpkt(sd, DCE2_BufferData(buf), DCE2_BufferLength(buf));
    if (rpkt == NULL)
        return;

    co_hdr = (DceRpcCoHdr *)DCE2_BufferData(buf);
    frag_len = DceRpcCoFragLen(co_hdr);
    pdu_type = DceRpcCoPduType(co_hdr);

    if (DCE2_PushPkt(rpkt) != DCE2_RET__SUCCESS)
    {
        DCE2_Log(DCE2_LOG_TYPE__ERROR,
                 "%s(%d) Failed to push packet onto packet stack.",
                 __FILE__, __LINE__);
        return;
    }

    switch (event)
    {
        case DCE2_EVENT__CO_FLEN_LT_HDR:
            DCE2_Alert(sd, event, frag_len, sizeof(DceRpcCoHdr));
            break;

        case DCE2_EVENT__CO_BAD_MAJ_VERSION:
            DCE2_Alert(sd, event, DceRpcCoVersMaj(co_hdr));
            break;

        case DCE2_EVENT__CO_BAD_MIN_VERSION:
            DCE2_Alert(sd, event, DceRpcCoVersMin(co_hdr));
            break;

        case DCE2_EVENT__CO_BAD_PDU_TYPE:
            DCE2_Alert(sd, event, DceRpcCoPduType(co_hdr));
            break;

        case DCE2_EVENT__CO_FRAG_GT_MAX_XMIT_FRAG:
            DCE2_Alert(sd, event, dce2_pdu_types[pdu_type],
                       frag_len, cot->max_xmit_frag);
            break;

        case DCE2_EVENT__CO_FRAG_LT_MAX_XMIT_FRAG:
            DCE2_Alert(sd, event, dce2_pdu_types[pdu_type],
                       frag_len, cot->max_xmit_frag);
            break;

        default:
            break;
    }

    DCE2_PopPkt();
}

/********************************************************************
 * Function: DCE2_CoGetSegRpkt()
 *
 * Gets and returns a reassembly packet based on a segmentation
 * buffer.
 *
 * Arguments:
 *  DCE2_SsnData *
 *      Pointer to the session data structure.
 *  const uint8_t *
 *      Pointer to the start of data in the segmentation buffer.
 *  uint32_t
 *      The length of the data in the segmentation buffer.
 *
 * Returns:
 *  SFSnortPacket *
 *      A valid pointer to a reassembled packet on success.
 *      NULL on error.
 *
 ********************************************************************/
static inline SFSnortPacket * DCE2_CoGetSegRpkt(DCE2_SsnData *sd,
                                                const uint8_t *data_ptr, uint32_t data_len)
{
    SFSnortPacket *rpkt = NULL;
    int smb_hdr_len = DCE2_SsnFromClient(sd->wire_pkt) ? DCE2_MOCK_HDR_LEN__SMB_CLI : DCE2_MOCK_HDR_LEN__SMB_SRV;

    switch (sd->trans)
    {
        case DCE2_TRANS_TYPE__SMB:
            rpkt = DCE2_GetRpkt(sd->wire_pkt, DCE2_RPKT_TYPE__SMB_CO_SEG,
                                data_ptr, data_len);
            if (rpkt == NULL)
            {
                DCE2_Log(DCE2_LOG_TYPE__ERROR,
                         "%s(%d) Failed to create reassembly packet.",
                         __FILE__, __LINE__);
                return NULL;
            }

            DCE2_SmbSetRdata((DCE2_SmbSsnData *)sd, (uint8_t *)rpkt->payload,
                             (uint16_t)(rpkt->payload_size - smb_hdr_len));

            break;

        case DCE2_TRANS_TYPE__TCP:
        case DCE2_TRANS_TYPE__HTTP_PROXY:
        case DCE2_TRANS_TYPE__HTTP_SERVER:
            rpkt = DCE2_GetRpkt(sd->wire_pkt, DCE2_RPKT_TYPE__TCP_CO_SEG,
                                data_ptr, data_len);
            if (rpkt == NULL)
            {
                DCE2_Log(DCE2_LOG_TYPE__ERROR,
                         "%s(%d) Failed to create reassembly packet.",
                         __FILE__, __LINE__);
                return NULL;
            }

            break;

        default:
            DCE2_Log(DCE2_LOG_TYPE__ERROR,
                     "%s(%d) Invalid transport type: %d",
                     __FILE__, __LINE__, sd->trans);
            break;
    }

    return rpkt;
}

/********************************************************************
 * Function: DCE2_CoInitCtxStorage()
 *
 * Allocates, if necessary, and initializes the context id list
 * and the context id pending queue.
 *
 * Arguments:
 *  DCE2_CoTracker *
 *      Pointer to the relevant connection-oriented tracker.
 *
 * Returns:
 *  DCE2_Ret
 *      DCE2_RET__ERROR
 *          We were not able to allocate data for new lists.
 *      DCE2_RET__SUCCESS
 *          We were able to allocate and initialize new lists.
 *
 ********************************************************************/
static inline DCE2_Ret DCE2_CoInitCtxStorage(DCE2_CoTracker *cot)
{
    if (cot == NULL)
        return DCE2_RET__ERROR;

    if (cot->ctx_ids == NULL)
    {
        cot->ctx_ids = DCE2_ListNew(DCE2_LIST_TYPE__SPLAYED, DCE2_CoCtxCompare, DCE2_CoCtxFree,
                                    NULL, DCE2_LIST_FLAG__NO_DUPS, DCE2_MEM_TYPE__CO_CTX);
        if (cot->ctx_ids == NULL)
            return DCE2_RET__ERROR;
    }

    if (cot->pending_ctx_ids == NULL)
    {
        cot->pending_ctx_ids = DCE2_QueueNew(DCE2_CoCtxFree, DCE2_MEM_TYPE__CO_CTX);
        if (cot->pending_ctx_ids == NULL)
        {
            DCE2_ListDestroy(cot->ctx_ids);
            cot->ctx_ids = NULL;
            return DCE2_RET__ERROR;
        }
    }
    else if (!DCE2_QueueIsEmpty(cot->pending_ctx_ids))
    {
        DCE2_QueueEmpty(cot->pending_ctx_ids);
    }

    return DCE2_RET__SUCCESS;
}

/********************************************************************
 * Function: DCE2_CoEarlyReassemble()
 *
 * Checks to see if we should send a reassembly packet based on
 * the current data in fragmentation and segmentation buffers
 * to the detection engine.  Whether we do or not is based on
 * whether or not we are configured to do so.  The number of bytes
 * in the fragmentation and segmentation buffers are calulated
 * and if they exceed the amount we are configured for, we
 * reassemble.
 *
 * Arguments:
 *  DCE2_SsnData *
 *      Pointer to the session data structure.
 *  DCE2_CoTracker *
 *      Pointer to the relevant connection-oriented tracker.
 *
 * Returns: None
 *
 ********************************************************************/
static void DCE2_CoEarlyReassemble(DCE2_SsnData *sd, DCE2_CoTracker *cot)
{
    DCE2_Buffer *frag_buf = DCE2_CoGetFragBuf(sd, &cot->frag_tracker);

    if (DCE2_SsnFromServer(sd->wire_pkt))
        return;

    if (!DCE2_BufferIsEmpty(frag_buf))
    {
        uint32_t bytes = DCE2_BufferLength(frag_buf);
        uint32_t seg_bytes = 0;

        if (!DCE2_BufferIsEmpty(cot->cli_seg.buf))
        {
            uint16_t hdr_size = sizeof(DceRpcCoHdr) + sizeof(DceRpcCoRequest);

            if (DCE2_BufferLength(cot->cli_seg.buf) > hdr_size)
            {
                DceRpcCoHdr *co_hdr = (DceRpcCoHdr *)DCE2_BufferData(cot->cli_seg.buf);

                if (DceRpcCoPduType(co_hdr) == DCERPC_PDU_TYPE__REQUEST)
                {
                    seg_bytes = DCE2_BufferLength(cot->cli_seg.buf) - hdr_size;

                    if ((UINT32_MAX - bytes) < seg_bytes)
                        seg_bytes = UINT32_MAX - bytes;

                    bytes += seg_bytes;
                }
            }
        }

        if (bytes >= DCE2_GcReassembleThreshold())
        {
            if (seg_bytes == 0)
            {
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CO, "Early reassemble - DCE/RPC fragments\n"));
                DCE2_CoReassemble(sd, cot, DCE2_CO_RPKT_TYPE__FRAG);
            }
            else
            {
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CO, "Early reassemble - DCE/RPC fragments and segments\n"));
                DCE2_CoReassemble(sd, cot, DCE2_CO_RPKT_TYPE__ALL);
            }
        }
    }
    else if (!DCE2_BufferIsEmpty(cot->cli_seg.buf))
    {
        uint16_t hdr_size = sizeof(DceRpcCoHdr) + sizeof(DceRpcCoRequest);
        uint32_t bytes = DCE2_BufferLength(cot->cli_seg.buf);

        if (bytes < hdr_size)
            return;

        if (bytes >= DCE2_GcReassembleThreshold())
        {
            DCE2_Ret status;

            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CO, "Early reassemble - DCE/RPC segments\n"));

            status = DCE2_CoSegEarlyRequest(cot, DCE2_BufferData(cot->cli_seg.buf), bytes);
            if (status != DCE2_RET__SUCCESS)
            {
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CO,
                               "Not enough data in seg buffer to set rule option data.\n"));
                return;
            }

            DCE2_CoReassemble(sd, cot, DCE2_CO_RPKT_TYPE__SEG);
        }
    }
}

/********************************************************************
 * Function: DCE2_CoGetRpkt()
 *
 * Creates a reassembled packet based on the kind of data
 * (fragment, segment or both) we want to put in the reassembled
 * packet.
 *
 * Arguments:
 *  DCE2_SsnData *
 *      Pointer to the session data structure.
 *  DCE2_CoTracker *
 *      Pointer to the relevant connection-oriented tracker.
 *  DCE2_CoRpktType
 *      Whether we want a defrag, deseg or defrag + deseg
 *      reassembled packet.
 *  DCE2_RpktType *
 *      This is set based on the transport for the session, and
 *      potentially used by the caller to set fields in the
 *      reassembled packet.
 *
 * Returns:
 *  SFSnortPacket *
 *      Pointer to the reassembled packet.
 *
 ********************************************************************/
static SFSnortPacket * DCE2_CoGetRpkt(DCE2_SsnData *sd, DCE2_CoTracker *cot,
                                      DCE2_CoRpktType co_rtype, DCE2_RpktType *rtype)
{
    DCE2_CoSeg *seg_buf = DCE2_CoGetSegPtr(sd, cot);
    DCE2_Buffer *frag_buf = DCE2_CoGetFragBuf(sd, &cot->frag_tracker);
    const uint8_t *frag_data = NULL, *seg_data = NULL;
    uint32_t frag_len = 0, seg_len = 0;
    SFSnortPacket *rpkt = NULL;

    *rtype = DCE2_RPKT_TYPE__NULL;

    switch (co_rtype)
    {
        case DCE2_CO_RPKT_TYPE__ALL:
            if (!DCE2_BufferIsEmpty(frag_buf))
            {
                frag_data = DCE2_BufferData(frag_buf);
                frag_len = DCE2_BufferLength(frag_buf);
            }

            if (!DCE2_BufferIsEmpty(seg_buf->buf))
            {
                seg_data = DCE2_BufferData(seg_buf->buf);
                seg_len = DCE2_BufferLength(seg_buf->buf);
            }

            break;

        case DCE2_CO_RPKT_TYPE__FRAG:
            if (!DCE2_BufferIsEmpty(frag_buf))
            {
                frag_data = DCE2_BufferData(frag_buf);
                frag_len = DCE2_BufferLength(frag_buf);
            }

            break;

        case DCE2_CO_RPKT_TYPE__SEG:
            if (!DCE2_BufferIsEmpty(seg_buf->buf))
            {
                seg_data = DCE2_BufferData(seg_buf->buf);
                seg_len = DCE2_BufferLength(seg_buf->buf);
            }

            break;

        default:
            DCE2_Log(DCE2_LOG_TYPE__ERROR,
                     "%s(%d) Invalid CO rpkt type: %d",
                     __FILE__, __LINE__, co_rtype);
            return NULL;
    }

    /* Seg stub data will be added to end of frag data */
    if ((frag_data != NULL) && (seg_data != NULL))
    {
        uint16_t hdr_size = sizeof(DceRpcCoHdr) + sizeof(DceRpcCoRequest);

        /* Need to just extract the stub data from the seg buffer
         * if there is enough data there */
        if (seg_len > hdr_size)
        {
            DceRpcCoHdr *co_hdr = (DceRpcCoHdr *)seg_data;

            /* Don't use it if it's not a request and therefore doesn't
             * belong with the frag data.  This is an insanity check -
             * shouldn't have seg data that's not a request if there are
             * frags queued up */
            if (DceRpcCoPduType(co_hdr) != DCERPC_PDU_TYPE__REQUEST)
            {
                seg_data = NULL;
                seg_len = 0;
            }
            else
            {
                DCE2_MOVE(seg_data, seg_len, hdr_size);
            }
        }
        else  /* Not enough stub data in seg buffer */
        {
            seg_data = NULL;
            seg_len = 0;
        }
    }

    if (frag_data != NULL)
        *rtype = DCE2_CoGetRpktType(sd, DCE2_BUF_TYPE__FRAG);
    else if (seg_data != NULL)
        *rtype = DCE2_CoGetRpktType(sd, DCE2_BUF_TYPE__SEG);

    if (*rtype == DCE2_RPKT_TYPE__NULL)
        return NULL;

    if (frag_data != NULL)
    {
        rpkt = DCE2_GetRpkt(sd->wire_pkt, *rtype, frag_data, frag_len);
        if (rpkt == NULL)
        {
            DCE2_Log(DCE2_LOG_TYPE__ERROR,
                     "%s(%d) Failed to create reassembly packet.",
                     __FILE__, __LINE__);
            return NULL;
        }

        if (seg_data != NULL)
        {
            /* If this fails, we'll still have the frag data */
            DCE2_AddDataToRpkt(rpkt, *rtype, seg_data, seg_len);
        }
    }
    else if (seg_data != NULL)
    {
        rpkt = DCE2_GetRpkt(sd->wire_pkt, *rtype, seg_data, seg_len);
        if (rpkt == NULL)
        {
            DCE2_Log(DCE2_LOG_TYPE__ERROR,
                     "%s(%d) Failed to create reassembly packet.",
                     __FILE__, __LINE__);
            return NULL;
        }
    }

    return rpkt;
}

/********************************************************************
 * Function: DCE2_CoSegDecode()
 *
 * Creates a reassembled packet from the segmentation buffer and
 * sends off to be decoded.  It's also detected on since the
 * detection engine has yet to see this data.
 *
 * Arguments:
 *  DCE2_SsnData *
 *      Pointer to the session data structure.
 *  DCE2_CoTracker *
 *      Pointer to the relevant connection-oriented tracker.
 *  DCE2_CoSeg *
 *      Pointer to the client or server segmentation buffer struct.
 *
 * Returns: None
 *
 ********************************************************************/
static void DCE2_CoSegDecode(DCE2_SsnData *sd, DCE2_CoTracker *cot, DCE2_CoSeg *seg)
{
    const uint8_t *frag_ptr;
    uint16_t frag_len;
    SFSnortPacket *rpkt;
    int smb_hdr_len = DCE2_SsnFromClient(sd->wire_pkt) ? DCE2_MOCK_HDR_LEN__SMB_CLI : DCE2_MOCK_HDR_LEN__SMB_SRV;
    PROFILE_VARS;

    if (DCE2_SsnFromClient(sd->wire_pkt))
        dce2_stats.co_cli_seg_reassembled++;
    else
        dce2_stats.co_srv_seg_reassembled++;

    PREPROC_PROFILE_START(dce2_pstat_co_reass);
    rpkt = DCE2_CoGetSegRpkt(sd, DCE2_BufferData(seg->buf), DCE2_BufferLength(seg->buf));
    PREPROC_PROFILE_END(dce2_pstat_co_reass);

    // FIXTHIS - don't toss data until success response to
    // allow for retransmission of last segment of pdu. if
    // we don't do it here 2 things break:
    // (a) we can't alert on this packet; and
    // (b) subsequent pdus aren't desegmented correctly.
    DCE2_BufferEmpty(seg->buf);

    if (rpkt == NULL)
        return;

    /* Set the start of the connection oriented pdu to where it
     * is in the reassembled packet */
    switch (sd->trans)
    {
        case DCE2_TRANS_TYPE__SMB:
            frag_ptr = rpkt->payload + smb_hdr_len;
            frag_len = rpkt->payload_size - smb_hdr_len;
            break;

        case DCE2_TRANS_TYPE__TCP:
        case DCE2_TRANS_TYPE__HTTP_PROXY:
        case DCE2_TRANS_TYPE__HTTP_SERVER:
            frag_ptr = rpkt->payload;
            frag_len = rpkt->payload_size;
            break;

        default:
            DCE2_Log(DCE2_LOG_TYPE__ERROR,
                     "%s(%d) Invalid transport type: %d",
                     __FILE__, __LINE__, sd->trans);
            return;
    }

    if (DCE2_PushPkt(rpkt) != DCE2_RET__SUCCESS)
    {
        DCE2_Log(DCE2_LOG_TYPE__ERROR,
                 "%s(%d) Failed to push packet onto packet stack.",
                 __FILE__, __LINE__);
        return;
    }

    /* All is good.  Decode the pdu */
    DCE2_CoDecode(sd, cot, frag_ptr, frag_len);

    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CO, "Reassembled CO segmented packet\n"));
    DCE2_DEBUG_CODE(DCE2_DEBUG__CO, DCE2_PrintPktData(rpkt->payload, rpkt->payload_size););

    /* Call detect since this is a reassembled packet that the
     * detection engine hasn't seen yet */
    if (!co_reassembled)
        DCE2_Detect(sd);

    DCE2_PopPkt();
}

/********************************************************************
 * Function: DCE2_CoSetRopts()
 *
 * Sets values necessary for the rule options.
 *
 * Arguments:
 *  DCE2_SsnData *
 *      Pointer to the session data structure.
 *  DCE2_CoTracker *
 *      Pointer to the relevant connection-oriented tracker.
 *  DceRpcCoHdr *
 *      Pointer to connection-oriented header in packet.
 *
 * Returns: None
 *
 ********************************************************************/
static inline void DCE2_CoSetRopts(DCE2_SsnData *sd, DCE2_CoTracker *cot, const DceRpcCoHdr *co_hdr)
{
    DCE2_CoFragTracker *ft = &cot->frag_tracker;
    int opnum = (ft->opnum != DCE2_SENTINEL) ? ft->opnum : cot->opnum;
    int ctx_id = (ft->ctx_id != DCE2_SENTINEL) ? ft->ctx_id : cot->ctx_id;

    int data_byte_order =
        (cot->data_byte_order != DCE2_SENTINEL) ?
            cot->data_byte_order : (int)DceRpcCoByteOrder(co_hdr);

    if (DCE2_CoSetIface(sd, cot, (uint16_t)ctx_id) != DCE2_RET__SUCCESS)
        sd->ropts.first_frag = DCE2_SENTINEL;
    else
        sd->ropts.first_frag = DceRpcCoFirstFrag(co_hdr);

    sd->ropts.hdr_byte_order = DceRpcCoByteOrder(co_hdr);
    sd->ropts.data_byte_order = data_byte_order;
    sd->ropts.opnum = opnum;
    sd->ropts.stub_data = cot->stub_data;
}

/********************************************************************
 * Function: DCE2_CoGetRpktType()
 *
 * Determines the type of reassembly packet we need to use
 * based on the transport and buffer type.
 *
 * Arguments:
 *  DCE2_SsnData *
 *      Pointer to the session data structure.
 *  DCE2_BufType
 *      The type of buffer we're using - fragmentation or
 *      segmentation.
 *
 * Returns:
 *  DCE2_RpktType
 *      The type of reassembly packet type we should be using
 *      given the transport and buffer type.
 *
 ********************************************************************/
static inline DCE2_RpktType DCE2_CoGetRpktType(DCE2_SsnData *sd, DCE2_BufType btype)
{
    DCE2_RpktType rtype = DCE2_RPKT_TYPE__NULL;

    switch (sd->trans)
    {
        case DCE2_TRANS_TYPE__SMB:
            switch (btype)
            {
                case DCE2_BUF_TYPE__SEG:
                    rtype = DCE2_RPKT_TYPE__SMB_CO_SEG;
                    break;

                case DCE2_BUF_TYPE__FRAG:
                    rtype = DCE2_RPKT_TYPE__SMB_CO_FRAG;
                    break;

                default:
                    DCE2_Log(DCE2_LOG_TYPE__ERROR,
                             "%s(%d) Invalid buffer type: %d",
                             __FILE__, __LINE__, btype);
                    break;
            }
            break;

        case DCE2_TRANS_TYPE__TCP:
        case DCE2_TRANS_TYPE__HTTP_PROXY:
        case DCE2_TRANS_TYPE__HTTP_SERVER:
            switch (btype)
            {
                case DCE2_BUF_TYPE__SEG:
                    rtype = DCE2_RPKT_TYPE__TCP_CO_SEG;
                    break;

                case DCE2_BUF_TYPE__FRAG:
                    rtype = DCE2_RPKT_TYPE__TCP_CO_FRAG;
                    break;

                default:
                    DCE2_Log(DCE2_LOG_TYPE__ERROR,
                             "%s(%d) Invalid buffer type: %d",
                             __FILE__, __LINE__, btype);
                    break;
            }
            break;

        default:
            DCE2_Log(DCE2_LOG_TYPE__ERROR,
                     "%s(%d) Invalid transport type: %d",
                     __FILE__, __LINE__, sd->trans);
            break;
    }
    return rtype;
}

/********************************************************************
 * Function: DCE2_CoIsSegBuf()
 *
 * Determines if the pointer passed in is within a segmentation
 * buffer.
 *
 * Arguments:
 *  DCE2_SsnData *
 *      Pointer to the session data structure.
 *  DCE2_CoTracker *
 *      Pointer to the relevant connection-oriented tracker.
 *  const uint8_t *
 *      The pointer to test.
 *
 * Returns:
 *  int
 *      1 if the pointer is in a segmentation buffer.
 *      0 if the pointer is not within a segmentation buffer.
 *
 ********************************************************************/
static inline int DCE2_CoIsSegBuf(DCE2_SsnData *sd, DCE2_CoTracker *cot, const uint8_t *ptr)
{
    DCE2_Buffer *seg_buf;

    if (DCE2_SsnFromServer(sd->wire_pkt))
        seg_buf = cot->srv_seg.buf;
    else
        seg_buf = cot->cli_seg.buf;

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
 * Function: DCE2_CoSegEarlyRequest()
 *
 * Used to set rule option data if we are doing an early
 * reassembly on data in the segmentation buffer.  If we are
 * taking directly from the segmentation buffer, none of the
 * rule option data will be set since processing doesn't get to
 * that point.  Only do if this is a Request PDU.
 *
 * Arguments:
 *  DCE2_CoTracker *
 *      Pointer to the relevant connection-oriented tracker.
 *  const uint8_t *
 *      Pointer to the segmentation buffer data.
 *  uint16_t
 *      Length of the segmentation buffer data.
 *
 * Returns:
 *  DCE2_Ret
 *      DCE2_RET__SUCCESS if there is enough data in buffer to
 *          set rule option data and we should continue processing.
 *      DCE2_RET__ERROR if there is not enough data in segmentation
 *          buffer to set rule option data and we should not
 *          continue processing.
 *
 ********************************************************************/
static DCE2_Ret DCE2_CoSegEarlyRequest(DCE2_CoTracker *cot,
                                       const uint8_t *seg_ptr, uint32_t seg_len)
{
    const DceRpcCoHdr *co_hdr;
    const DceRpcCoRequest *rhdr;
    uint16_t req_size = sizeof(DceRpcCoRequest);

    if (seg_len < sizeof(DceRpcCoHdr))
        return DCE2_RET__ERROR;

    co_hdr = (DceRpcCoHdr *)seg_ptr;
    DCE2_MOVE(seg_ptr, seg_len, sizeof(DceRpcCoHdr));

    if (DceRpcCoPduType(co_hdr) != DCERPC_PDU_TYPE__REQUEST)
        return DCE2_RET__ERROR;

    rhdr = (DceRpcCoRequest *)seg_ptr;

    /* Account for possible object uuid */
    if (DceRpcCoObjectFlag(co_hdr))
        req_size += sizeof(Uuid);

    if (seg_len < req_size)
        return DCE2_RET__ERROR;

    cot->opnum = DceRpcCoOpnum(co_hdr, rhdr);
    cot->ctx_id = DceRpcCoCtxId(co_hdr, rhdr);
    cot->call_id = DceRpcCoCallId(co_hdr);

    return DCE2_RET__SUCCESS;
}

/********************************************************************
 * Function: DCE2_CoGetSegPtr()
 *
 * Returns the appropriate segmentation buffer.
 *
 * Arguments:
 *  DCE2_SsnData *
 *      Pointer to session data.
 *  DCE2_CoTracker *
 *      Pointer to connection-oriented tracker.
 *
 * Returns:
 *  DCE2_CoSeg *
 *      Pointer to client or server segmenation buffer.
 *
 ********************************************************************/
static inline DCE2_CoSeg * DCE2_CoGetSegPtr(DCE2_SsnData *sd, DCE2_CoTracker *cot)
{
    if (DCE2_SsnFromServer(sd->wire_pkt))
        return &cot->srv_seg;

    return &cot->cli_seg;
}

/********************************************************************
 * Function: DCE2_CoGetFragBuf()
 *
 * Returns the appropriate fragmentation buffer.
 *
 * Arguments:
 *  DCE2_SsnData *
 *      Pointer to session data.
 *  DCE2_CoFragTracker *
 *      Pointer to connection-oriented fragmentation tracker.
 *
 * Returns:
 *  DCE2_Buffer *
 *      Pointer to client or server fragmentation buffer.
 *
 ********************************************************************/
static inline DCE2_Buffer * DCE2_CoGetFragBuf(DCE2_SsnData *sd, DCE2_CoFragTracker *ft)
{
    if (DCE2_SsnFromServer(sd->wire_pkt))
        return ft->srv_stub_buf;

    return ft->cli_stub_buf;
}

static int DCE2_CoGetAuthLen(DCE2_SsnData *sd, const DceRpcCoHdr *co_hdr,
        const uint8_t *frag_ptr, uint16_t frag_len)
{
    DceRpcCoAuthVerifier *auth_hdr;
    uint16_t auth_len = DceRpcCoAuthLen(co_hdr);

    if (auth_len == 0)
        return 0;

    auth_len += sizeof(DceRpcCoAuthVerifier);

    /* This means the auth len was bogus */
    if (auth_len > frag_len)
    {
        DCE2_Alert(sd, DCE2_EVENT__CO_FLEN_LT_SIZE, dce2_pdu_types[DceRpcCoPduType(co_hdr)],
                frag_len, auth_len);
        return -1;
    }

    auth_hdr = (DceRpcCoAuthVerifier *)(frag_ptr + (frag_len - auth_len));
    if (DceRpcCoAuthLevel(auth_hdr) == DCERPC_CO_AUTH_LEVEL__PKT_PRIVACY)
    {
        /* Data is encrypted - don't inspect */
        return -1;
    }

    auth_len += DceRpcCoAuthPad(auth_hdr);

    /* This means the auth pad len was bogus */
    if (auth_len > frag_len)
    {
        DCE2_Alert(sd, DCE2_EVENT__CO_FLEN_LT_SIZE, dce2_pdu_types[DceRpcCoPduType(co_hdr)],
                frag_len, auth_len);
        return -1;
    }

    return (int)auth_len;
}
