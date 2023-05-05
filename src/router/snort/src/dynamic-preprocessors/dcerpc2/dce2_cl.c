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
 * Module for handling connectionless DCE/RPC processing.  Provides
 * functionality for tracking sub-sessions or activities within a
 * connectionless conversation and for tracking and reassembling fragments
 * within each activity.  Also sets appropriate data for use with
 * preprocessor rule options.
 *
 * 8/17/2008 - Initial implementation ... Todd Wease <twease@sourcefire.com>
 *
 ****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_types.h"
#include "spp_dce2.h"
#include "dce2_cl.h"
#include "snort_dce2.h"
#include "dce2_list.h"
#include "dce2_memory.h"
#include "dce2_utils.h"
#include "dce2_stats.h"
#include "dce2_session.h"
#include "dce2_event.h"
#include "dcerpc.h"
#include "sf_types.h"
#include "snort_debug.h"
#include "profiler.h"
#include "sf_snort_packet.h"
#include "sf_dynamic_preprocessor.h"

/********************************************************************
 * Global variables
 ********************************************************************/
static uint8_t dce2_cl_rbuf[IP_MAXPKT];

/********************************************************************
 * Macros
 ********************************************************************/
#define DCE2_CL__MAX_SEQ_NUM  UINT32_MAX

/********************************************************************
 * Structures
 ********************************************************************/
typedef struct _DCE2_ClFragNode
{
    uint32_t frag_number;
    uint16_t frag_len;
    uint8_t *frag_data;

} DCE2_ClFragNode;

typedef struct _DCE2_ClFragTracker
{
    Uuid iface;          /* only set on first fragment received */
    uint32_t iface_vers; /* only set on first fragment received */
    int opnum;           /* set to that of first fragment, i.e fragment number == 0.
                          * initialize to a sentinel */
    int data_byte_order; /* set to that of first fragment, i.e fragment number == 0.
                          * initialize to sentinel */

    DCE2_List *frags;         /* sorted by fragment number */
    int num_expected_frags;   /* set when we get last frag */

} DCE2_ClFragTracker;

typedef struct _DCE2_ClActTracker
{
    Uuid act;
    uint32_t seq_num;
    uint8_t seq_num_invalid;

    DCE2_ClFragTracker frag_tracker;

#if 0
    /* Not currently used.  These are related to getting a sequence number that
     * is at the end of the sequence number space */
    uint32_t last_pkt_sec;
    uint8_t no_frags;
    uint8_t no_requests;
#endif

} DCE2_ClActTracker;

/********************************************************************
 * Private function prototypes
 ********************************************************************/
static DCE2_Ret DCE2_ClHdrChecks(DCE2_SsnData *, const DceRpcClHdr *);
static DCE2_ClActTracker * DCE2_ClGetActTracker(DCE2_ClTracker *, DceRpcClHdr *);
static DCE2_ClActTracker * DCE2_ClInsertActTracker(DCE2_ClTracker *, DceRpcClHdr *);
static void DCE2_ClRequest(DCE2_SsnData *, DCE2_ClActTracker *, DceRpcClHdr *,
                           const uint8_t *, uint16_t);
static void DCE2_ClHandleFrag(DCE2_SsnData *, DCE2_ClActTracker *,
                              DceRpcClHdr *, const uint8_t *, uint16_t);
static void DCE2_ClFragReassemble(DCE2_SsnData*, DCE2_ClActTracker *, const DceRpcClHdr *);
static void DCE2_ClResetFragTracker(DCE2_ClFragTracker *);

static inline void DCE2_ClSetRdata(DCE2_ClActTracker *, const DceRpcClHdr *, uint8_t *, uint16_t);

/* Callbacks */
static int DCE2_ClFragCompare(const void *, const void *);
static void DCE2_ClActDataFree(void *);
static void DCE2_ClActKeyFree(void *);
static void DCE2_ClFragDataFree(void *);

/********************************************************************
 * Function: DCE2_ClInitRdata()
 *
 * Initializes static values in the global CL data reassembly
 * buffer.  These values should never need to be changed after
 * this initialization.
 *
 * Arguments:
 *  uint8_t *
 *      Pointer to the data reassembly buffer.
 *
 * Returns: None
 *
 ********************************************************************/
void DCE2_ClInitRdata(uint8_t *buf)
{
    DceRpcClHdr *cl_hdr = (DceRpcClHdr *)buf;

    /* Set some relevant fields.  These should never get reset */
    cl_hdr->rpc_vers = DCERPC_PROTO_MAJOR_VERS__4;
    cl_hdr->ptype = DCERPC_PDU_TYPE__REQUEST;
    cl_hdr->drep[0] = 0x10;   /* Little endian */
}

/********************************************************************
 * Function: DCE2_ClSetRdata()
 *
 * Sets relevant data fields in the reassembly packet.
 *
 * Arguments:
 *  DCE2_ClActTracker *
 *      Pointer to the activity tracker associated with the
 *      reassemble packet.
 *  DceRpcClHdr *
 *      Pointer to the connectionless header in the wire packet.
 *  uint8_t *
 *      Pointer to the start of the reassembly buffer.
 *  uint16_t
 *      The length of the stub data.
 *
 * Returns: None
 *
 ********************************************************************/
static inline void DCE2_ClSetRdata(DCE2_ClActTracker *at, const DceRpcClHdr *pkt_cl_hdr,
                                   uint8_t *cl_ptr, uint16_t stub_len)
{
    DCE2_ClFragTracker *ft = &at->frag_tracker;
    DceRpcClHdr *cl_hdr = (DceRpcClHdr *)cl_ptr;
    uint16_t opnum = (ft->opnum != DCE2_SENTINEL) ? (uint16_t)ft->opnum : DceRpcClOpnum(pkt_cl_hdr);

    cl_hdr->len = DceRpcHtons(&stub_len, DCERPC_BO_FLAG__LITTLE_ENDIAN);
    DCE2_CopyUuid(&cl_hdr->object, &pkt_cl_hdr->object, DceRpcClByteOrder(cl_hdr));
    DCE2_CopyUuid(&cl_hdr->if_id, &ft->iface, DCERPC_BO_FLAG__LITTLE_ENDIAN);
    DCE2_CopyUuid(&cl_hdr->act_id, &at->act, DCERPC_BO_FLAG__LITTLE_ENDIAN);
    cl_hdr->if_vers = DceRpcHtonl(&ft->iface_vers, DCERPC_BO_FLAG__LITTLE_ENDIAN);
    cl_hdr->opnum = DceRpcHtons(&opnum, DCERPC_BO_FLAG__LITTLE_ENDIAN);
}

/********************************************************************
 * Function: DCE2_ClProcess()
 *
 * Main entry point for connectionless DCE/RPC processing.  Gets
 * the activity tracker associated with this session and passes
 * along to client or server handling.
 *
 * Arguments:
 *  DCE2_SsnData *
 *      Pointer to the session data structure.
 *  DCE2_ClTracker *
 *      Pointer to the connectionless tracker structure.
 *
 * Returns: None
 *
 ********************************************************************/
void DCE2_ClProcess(DCE2_SsnData *sd, DCE2_ClTracker *clt)
{
    DceRpcClHdr *cl_hdr;
    DCE2_ClActTracker *at;
    const uint8_t *data_ptr = sd->wire_pkt->payload;
    uint16_t data_len = sd->wire_pkt->payload_size;
    PROFILE_VARS;

    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CL, "Cl processing ...\n"));

    dce2_stats.cl_pkts++;

    if (data_len < sizeof(DceRpcClHdr))
    {
        if (!DCE2_SsnAutodetected(sd))
            DCE2_Alert(sd, DCE2_EVENT__CL_DATA_LT_HDR, data_len, sizeof(DceRpcClHdr));

        return;
    }

    cl_hdr = (DceRpcClHdr *)data_ptr;

    DCE2_MOVE(data_ptr, data_len, sizeof(DceRpcClHdr));

    if (DCE2_ClHdrChecks(sd, cl_hdr) != DCE2_RET__SUCCESS)
        return;

    PREPROC_PROFILE_START(dce2_pstat_cl_acts);
    at = DCE2_ClGetActTracker(clt, cl_hdr);
    PREPROC_PROFILE_END(dce2_pstat_cl_acts);
    if (at == NULL)
        return;

    if (DCE2_SsnFromClient(sd->wire_pkt))
    {
        switch (DceRpcClPduType(cl_hdr))
        {
            case DCERPC_PDU_TYPE__REQUEST:
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CL, "Request\n"));
                dce2_stats.cl_request++;
                DCE2_ClRequest(sd, at, cl_hdr, data_ptr, data_len);
                break;

            case DCERPC_PDU_TYPE__ACK:
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CL, "Ack\n"));
                dce2_stats.cl_ack++;
                break;

            case DCERPC_PDU_TYPE__CL_CANCEL:
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CL, "Cancel\n"));
                dce2_stats.cl_cancel++;
                break;

            case DCERPC_PDU_TYPE__FACK:
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CL, "Fack\n"));
                dce2_stats.cl_cli_fack++;
                break;

            case DCERPC_PDU_TYPE__PING:
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CL, "Ping\n"));
                dce2_stats.cl_ping++;
                break;

            case DCERPC_PDU_TYPE__RESPONSE:
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CL, "Response from client.  Changing stream direction."));
                _dpd.streamAPI->update_direction(sd->wire_pkt->stream_session, SSN_DIR_FROM_RESPONDER,
                                                 GET_SRC_IP(((SFSnortPacket *)sd->wire_pkt)),
                                                 sd->wire_pkt->src_port);
                break;

            default:
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CL, "Other pdu type\n"));
                dce2_stats.cl_other_req++;
                break;
        }
    }
    else
    {
        switch (DceRpcClPduType(cl_hdr))
        {
            case DCERPC_PDU_TYPE__RESPONSE:
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CL, "Response\n"));
                dce2_stats.cl_response++;
                break;

            case DCERPC_PDU_TYPE__REJECT:
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CL, "Reject\n"));
                dce2_stats.cl_reject++;

                if (DceRpcClSeqNum(cl_hdr) == at->seq_num)
                {
                    DCE2_ClResetFragTracker(&at->frag_tracker);
                    at->seq_num_invalid = 1;
                }

                break;

            case DCERPC_PDU_TYPE__CANCEL_ACK:
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CL, "Cancel Ack\n"));
                dce2_stats.cl_cancel_ack++;
                break;

            case DCERPC_PDU_TYPE__FACK:
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CL, "Fack\n"));
                dce2_stats.cl_srv_fack++;
                break;

            case DCERPC_PDU_TYPE__FAULT:
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CL, "Fault\n"));
                dce2_stats.cl_fault++;
                break;

            case DCERPC_PDU_TYPE__NOCALL:
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CL, "No call\n"));
                dce2_stats.cl_nocall++;
                break;

            case DCERPC_PDU_TYPE__WORKING:
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CL, "Working\n"));
                dce2_stats.cl_working++;
                break;

            default:
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CL, "Other pdu type\n"));
                dce2_stats.cl_other_resp++;
                break;
        }
    }
}

/********************************************************************
 * Function: DCE2_ClHdrChecks()
 *
 * Checks to make sure header fields are sane.  If they aren't,
 * alert on the header anomaly.  If we've autodetected the session,
 * however, don't alert, but set a header anomaly flag, so we can
 * re-autodetect on the next go around.
 *
 * Arguments:
 *  DCE2_SsnData *
 *      Pointer to the session data structure.
 *  DceRpcClHdr *
 *      Pointer to the connectionless header in the packet.
 *
 * Returns:
 *  DCE2_Ret
 *      DCE2_RET__ERROR
 *          We should not continue to inspect.
 *      DCE2_RET__SUCCESS
 *          Continue inspection.
 *
 ********************************************************************/
static DCE2_Ret DCE2_ClHdrChecks(DCE2_SsnData *sd, const DceRpcClHdr *cl_hdr)
{
    if (DceRpcClRpcVers(cl_hdr) != DCERPC_PROTO_MAJOR_VERS__4)
    {
        /* If we autodetected the session, we probably guessed wrong */
        if (!DCE2_SsnAutodetected(sd))
            DCE2_Alert(sd, DCE2_EVENT__CL_BAD_MAJ_VERSION, DceRpcClRpcVers(cl_hdr));

        return DCE2_RET__ERROR;
    }

    if (DceRpcClPduType(cl_hdr) >= DCERPC_PDU_TYPE__MAX)
    {
        if (!DCE2_SsnAutodetected(sd))
            DCE2_Alert(sd, DCE2_EVENT__CL_BAD_PDU_TYPE, DceRpcClPduType(cl_hdr));

        return DCE2_RET__ERROR;
    }

    return DCE2_RET__SUCCESS;
}

/********************************************************************
 * Function: DCE2_ClGetActTracker()
 *
 * Searches for activity tracker in list using activity UUID in
 * packet.  If the activity tracker list is NULL, a new one is
 * created.  If the activity tracker is not found, it is inserted
 * into the list.
 *
 * Arguments:
 *  DCE2_ClTracker *
 *      Pointer to the connectionless tracker.
 *  DceRpcClHdr *
 *      Pointer to the connectionless header in the packet.
 *
 * Returns:
 *  DCE2_ClActTracker *
 *      A valid pointer to an activity tracker on success.
 *      NULL on error.
 *
 ********************************************************************/
static DCE2_ClActTracker * DCE2_ClGetActTracker(DCE2_ClTracker *clt, DceRpcClHdr *cl_hdr)
{
    DCE2_ClActTracker *at = NULL;

    /* Try to find a currently active activity tracker */
    if (clt->act_trackers != NULL)
    {
        Uuid uuid;

        DCE2_CopyUuid(&uuid, &cl_hdr->act_id, DceRpcClByteOrder(cl_hdr));
        at = DCE2_ListFind(clt->act_trackers, (void *)&uuid);
    }
    else
    {
        /* Create a new activity tracker list */
        clt->act_trackers = DCE2_ListNew(DCE2_LIST_TYPE__SPLAYED, DCE2_UuidCompare,
                                         DCE2_ClActDataFree, DCE2_ClActKeyFree,
                                         DCE2_LIST_FLAG__NO_DUPS, DCE2_MEM_TYPE__CL_ACT);
        if (clt->act_trackers == NULL)
            return NULL;
    }

    /* Didn't find a currently active activity tracker */
    if (at == NULL)
    {
        /* Insert a new activity tracker */
        at = DCE2_ClInsertActTracker(clt, cl_hdr);
        if (at == NULL)
            return NULL;
    }

    return at;
}

/********************************************************************
 * Function: DCE2_ClInsertActTracker()
 *
 * Creates and inserts a new activity tracker into a list.
 *
 * Arguments:
 *  DCE2_ClTracker *
 *      Pointer to connectionless tracker.
 *  DceRpcClHdr *
 *      Pointer to the connectionless header in the packet.
 *
 * Returns:
 *  DCE2_ClActTracker *
 *      A valid pointer to an activity tracker on success.
 *      NULL on error.
 *
 ********************************************************************/
static DCE2_ClActTracker * DCE2_ClInsertActTracker(DCE2_ClTracker *clt, DceRpcClHdr *cl_hdr)
{
    Uuid *uuid = (Uuid *)DCE2_Alloc(sizeof(Uuid), DCE2_MEM_TYPE__CL_ACT);
    DCE2_ClActTracker *at;
    DCE2_Ret status;

    if (uuid == NULL)
        return NULL;

    at = (DCE2_ClActTracker *)DCE2_Alloc(sizeof(DCE2_ClActTracker), DCE2_MEM_TYPE__CL_ACT);
    if (at == NULL)
    {
        DCE2_Free((void *)uuid, sizeof(Uuid), DCE2_MEM_TYPE__CL_ACT);
        return NULL;
    }

    DCE2_CopyUuid(uuid, &cl_hdr->act_id, DceRpcClByteOrder(cl_hdr));
    DCE2_CopyUuid(&at->act, &cl_hdr->act_id, DceRpcClByteOrder(cl_hdr));

    status = DCE2_ListInsert(clt->act_trackers, (void *)uuid, (void *)at);
    if (status != DCE2_RET__SUCCESS)
    {
        DCE2_Free((void *)uuid, sizeof(Uuid), DCE2_MEM_TYPE__CL_ACT);
        DCE2_Free((void *)at, sizeof(DCE2_ClActTracker), DCE2_MEM_TYPE__CL_ACT);
        return NULL;
    }

    return at;
}

/********************************************************************
 * Function: DCE2_ClRequest()
 *
 * Handles a client request.
 *
 * Arguments:
 *  DCE2_SsnData *
 *      Pointer to the session data structure.
 *  DCE2_ClActTracker *
 *      Pointer to the connectionless activity tracker.
 *  DceRpcClHdr *
 *      Pointer to the connectionless header in the packet.
 *  const uint8_t *
 *      Pointer to current position in the packet payload.
 *  uint16_t
 *      Length of packet payload left from current pointer
 *      position.
 *
 * Returns: None
 *
 ********************************************************************/
static void DCE2_ClRequest(DCE2_SsnData *sd, DCE2_ClActTracker *at, DceRpcClHdr *cl_hdr,
                           const uint8_t *data_ptr, uint16_t data_len)
{
    uint32_t seq_num = DceRpcClSeqNum(cl_hdr);

    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__CL, "Processing Request ...\n"));

    if (seq_num > at->seq_num)
    {
        /* This is the normal case where the sequence number is incremented
         * for each request.  Set the new sequence number and mark it valid. */
        at->seq_num = seq_num;
        at->seq_num_invalid = 0;

        /* If there are any fragments, the new sequence number invalidates
         * all of the frags that might be currently stored. */
        DCE2_ClResetFragTracker(&at->frag_tracker);
    }
    else if ((seq_num < at->seq_num) || at->seq_num_invalid)
    {
#if 0
        /* If we get a seqence number less than what we're at, the
         * server won't look at it.  If we get the same sequence number,
         * but we've already processed a previous request, it's bad.
         * Fragments will have the same sequence number, but we won't
         * mark the seq number invalid until we've gotten all of them. */
        /* Comment for now since we're not able to detect retransmits */
        DCE2_Alert(sd, DCE2_EVENT__CL_BAD_SEQ_NUM, dce2_pdu_types[DceRpcClPduType(cl_hdr)]);
#endif
        return;
    }

    DCE2_ResetRopts(&sd->ropts);

    if (DceRpcClFrag(cl_hdr))  /* It's a frag */
    {
        dce2_stats.cl_fragments++;
        if (DCE2_GcDceDefrag())
        {
            DCE2_ClHandleFrag(sd, at, cl_hdr, data_ptr, data_len);
            return;
        }
    }
    else   /* It's a full request */
    {
        if ((at->frag_tracker.frags != NULL) &&
            !DCE2_ListIsEmpty(at->frag_tracker.frags))
        {
            /* If we get a full request, i.e. not a frag, any frags
             * we have collected are invalidated */
            DCE2_ClResetFragTracker(&at->frag_tracker);
        }
        else if (seq_num != DCE2_CL__MAX_SEQ_NUM)
        {
            /* This sequence number is now invalid. 0xffffffff is the end of
             * the sequence number space and can be reused */
            at->seq_num_invalid = 1;
        }
        else
        {
            /* Got the last sequence number in the sequence number space */
            dce2_stats.cl_max_seqnum++;
        }
    }

    /* Cache relevant values for rule option processing */
    sd->ropts.first_frag = DceRpcClFirstFrag(cl_hdr);
    DCE2_CopyUuid(&sd->ropts.iface, DceRpcClIface(cl_hdr), DceRpcClByteOrder(cl_hdr));
    sd->ropts.iface_vers = DceRpcClIfaceVers(cl_hdr);
    sd->ropts.hdr_byte_order = DceRpcClByteOrder(cl_hdr);
    sd->ropts.data_byte_order = DceRpcClByteOrder(cl_hdr);
    sd->ropts.opnum = DceRpcClOpnum(cl_hdr);
    sd->ropts.stub_data = (uint8_t *)cl_hdr + sizeof(DceRpcClHdr);

    DCE2_Detect(sd);
}

/********************************************************************
 * Function: DCE2_ClHandleFrag()
 *
 * Handles connectionless fragments.  Creates a new fragment list
 * if necessary and inserts fragment into list.  Sets rule option
 * values based on the fragment.
 *
 * Arguments:
 *  DCE2_SsnData *
 *      Pointer to the session data structure.
 *  DCE2_ClActTracker *
 *      Pointer to the connectionless activity tracker.
 *  DceRpcClHdr *
 *      Pointer to the connectionless header in the packet.
 *  const uint8_t *
 *      Pointer to current position in the packet payload.
 *  uint16_t
 *      Length of packet payload left from current pointer
 *      position.
 *
 * Returns: None
 *
 ********************************************************************/
static void DCE2_ClHandleFrag(DCE2_SsnData *sd, DCE2_ClActTracker *at, DceRpcClHdr *cl_hdr,
                              const uint8_t *data_ptr, uint16_t data_len)
{
    DCE2_ClFragTracker *ft = &at->frag_tracker;
    DCE2_ClFragNode *fn;
    uint16_t frag_len;
    int status;
    PROFILE_VARS;

    PREPROC_PROFILE_START(dce2_pstat_cl_frag);

    /* If the frag length is less than data length there might be authentication
     * data that we don't want to include, otherwise just set to data len */
    if (DceRpcClLen(cl_hdr) < data_len)
        frag_len = DceRpcClLen(cl_hdr);
    else
        frag_len = data_len;

    if (frag_len == 0)
    {
        PREPROC_PROFILE_END(dce2_pstat_cl_frag);
        return;
    }

    if (frag_len > dce2_stats.cl_max_frag_size)
        dce2_stats.cl_max_frag_size = frag_len;

    if (DCE2_GcMaxFrag() && (frag_len > DCE2_GcMaxFragLen()))
        frag_len = DCE2_GcMaxFragLen();

    if (ft->frags == NULL)
    {
        /* Create new list if we don't have one already */
        ft->frags = DCE2_ListNew(DCE2_LIST_TYPE__SORTED, DCE2_ClFragCompare, DCE2_ClFragDataFree,
                                 NULL, DCE2_LIST_FLAG__NO_DUPS | DCE2_LIST_FLAG__INS_TAIL,
                                 DCE2_MEM_TYPE__CL_FRAG);

        if (ft->frags == NULL)
        {
            PREPROC_PROFILE_END(dce2_pstat_cl_frag);
            return;
        }
    }
    else
    {
        /* If we already have a fragment in the list with the same fragment number,
         * that fragment will take precedence over this fragment and this fragment
         * will not be used by the server */
        fn = (DCE2_ClFragNode *)DCE2_ListFind(ft->frags, (void *)(uintptr_t)DceRpcClFragNum(cl_hdr));
        if (fn != NULL)
        {
            PREPROC_PROFILE_END(dce2_pstat_cl_frag);
            return;
        }
    }

    /* Create a new frag node to insert into the list */
    fn = (DCE2_ClFragNode *)DCE2_Alloc(sizeof(DCE2_ClFragNode), DCE2_MEM_TYPE__CL_FRAG);
    if (fn == NULL)
    {
        PREPROC_PROFILE_END(dce2_pstat_cl_frag);

        DCE2_ClFragReassemble(sd, at, cl_hdr);
        return;
    }

    fn->frag_number = DceRpcClFragNum(cl_hdr);
    fn->frag_len = frag_len;

    /* Allocate space for the fragment data */
    fn->frag_data = (uint8_t *)DCE2_Alloc(frag_len, DCE2_MEM_TYPE__CL_FRAG);
    if (fn->frag_data == NULL)
    {
        DCE2_Free((void *)fn, sizeof(DCE2_ClFragNode), DCE2_MEM_TYPE__CL_FRAG);

        PREPROC_PROFILE_END(dce2_pstat_cl_frag);

        DCE2_ClFragReassemble(sd, at, cl_hdr);
        return;
    }

    /* Copy the fragment data in the packet to the space just allocated */
    status = DCE2_Memcpy(fn->frag_data, data_ptr, frag_len, fn->frag_data, fn->frag_data + frag_len);
    if (status != DCE2_RET__SUCCESS)
    {
        DCE2_Free((void *)fn->frag_data, frag_len, DCE2_MEM_TYPE__CL_FRAG);
        DCE2_Free((void *)fn, sizeof(DCE2_ClFragNode), DCE2_MEM_TYPE__CL_FRAG);

        PREPROC_PROFILE_END(dce2_pstat_cl_frag);

        DCE2_ClFragReassemble(sd, at, cl_hdr);
        return;
    }

    if (DCE2_ListIsEmpty(ft->frags))
    {
        /* If this is the first fragment we've received, set interface uuid */
        DCE2_CopyUuid(&ft->iface, DceRpcClIface(cl_hdr), DceRpcClByteOrder(cl_hdr));
        ft->iface_vers = DceRpcClIfaceVers(cl_hdr);
    }

    if (DceRpcClLastFrag(cl_hdr))
    {
        /* Set number of expected frags on last frag */
        ft->num_expected_frags = DceRpcClFragNum(cl_hdr) + 1;
    }
    else if (DceRpcClFirstFrag(cl_hdr))
    {
        /* Set opum and byte order on first frag */
        ft->opnum = DceRpcClOpnum(cl_hdr);
        ft->data_byte_order = DceRpcClByteOrder(cl_hdr);
    }

    /* Insert frag node into the list */
    status = DCE2_ListInsert(ft->frags, (void *)(uintptr_t)fn->frag_number, (void *)fn);
    if (status != DCE2_RET__SUCCESS)
    {
        DCE2_Free((void *)fn->frag_data, frag_len, DCE2_MEM_TYPE__CL_FRAG);
        DCE2_Free((void *)fn, sizeof(DCE2_ClFragNode), DCE2_MEM_TYPE__CL_FRAG);

        PREPROC_PROFILE_END(dce2_pstat_cl_frag);

        DCE2_ClFragReassemble(sd, at, cl_hdr);
        return;
    }

    /* Fragment number field in header is uint16_t */
    if ((ft->num_expected_frags != DCE2_SENTINEL) &&
        (uint16_t)ft->frags->num_nodes == (uint16_t)ft->num_expected_frags)
    {
        PREPROC_PROFILE_END(dce2_pstat_cl_frag);

        /* We got all of the frags - reassemble */
        DCE2_ClFragReassemble(sd, at, cl_hdr);
        at->seq_num_invalid = 1;

        return;
    }

    PREPROC_PROFILE_END(dce2_pstat_cl_frag);

    /* Cache relevant values for rule option processing */
    sd->ropts.first_frag = DceRpcClFirstFrag(cl_hdr);
    DCE2_CopyUuid(&sd->ropts.iface, &ft->iface, DCERPC_BO_FLAG__NONE);
    sd->ropts.iface_vers = ft->iface_vers;
    sd->ropts.hdr_byte_order = DceRpcClByteOrder(cl_hdr);

    if (ft->data_byte_order != DCE2_SENTINEL)
        sd->ropts.data_byte_order = ft->data_byte_order;
    else
        sd->ropts.data_byte_order = DceRpcClByteOrder(cl_hdr);

    if (ft->opnum != DCE2_SENTINEL)
        sd->ropts.opnum = ft->opnum;
    else
        sd->ropts.opnum = DceRpcClOpnum(cl_hdr);

    sd->ropts.stub_data = (uint8_t *)cl_hdr + sizeof(DceRpcClHdr);

    DCE2_Detect(sd);
}

/********************************************************************
 * Function: DCE2_ClFragCompare()
 *
 * Callback to fragment list for sorting the nodes in the list
 * by fragment number.  Values passed in are the fragment numbers.
 *
 * Arguments:
 *  const void *
 *      First fragment number to compare.
 *  const void *
 *      Second fragment number to compare.
 *
 * Returns:
 *  int
 *       1 if first value is greater than second value
 *      -1 if first value is less than second value
 *       0 if first value equals second value
 *
 ********************************************************************/
static int DCE2_ClFragCompare(const void *a, const void *b)
{
    int x = (int)(uintptr_t)a;
    int y = (int)(uintptr_t)b;

    if (x > y)
        return 1;
    if (x < y)
        return -1;

    return 0;
}

/********************************************************************
 * Function: DCE2_ClFragReassemble()
 *
 * Reassembles fragments into reassembly buffer and copies to
 * reassembly packet.
 *
 * Arguments:
 *  DCE2_SsnData *
 *      Pointer to the session data structure.
 *  DCE2_ClActTracker *
 *      Pointer to the connectionless activity tracker.
 *  DceRpcClHdr *
 *      Pointer to the connectionless header in the packet.
 *
 * Returns: None
 *
 ********************************************************************/
static void DCE2_ClFragReassemble(DCE2_SsnData *sd, DCE2_ClActTracker *at, const DceRpcClHdr *cl_hdr)
{
    DCE2_ClFragTracker *ft = &at->frag_tracker;
    DCE2_ClFragNode *fnode;
    uint8_t *rdata = dce2_cl_rbuf;
    uint16_t rlen = sizeof(dce2_cl_rbuf);
    uint32_t stub_len = 0;
    const uint8_t *stub_data = NULL;
    SFSnortPacket *rpkt = NULL;
    PROFILE_VARS;

    PREPROC_PROFILE_START(dce2_pstat_cl_reass);

    for (fnode = (DCE2_ClFragNode *)DCE2_ListFirst(ft->frags);
         fnode != NULL;
         fnode = (DCE2_ClFragNode *)DCE2_ListNext(ft->frags))
    {
        if (fnode->frag_len > rlen)
        {
            DCE2_Log(DCE2_LOG_TYPE__WARN,
                     "%s(%d) Size of fragments exceeds reassembly buffer size. "
                     "Using as many fragments as will fit.", __FILE__, __LINE__);
            break;
        }

        if (DCE2_Memcpy(rdata, fnode->frag_data, fnode->frag_len, rdata, rdata + rlen) != DCE2_RET__SUCCESS)
        {
            DCE2_Log(DCE2_LOG_TYPE__ERROR,
                     "%s(%d) Failed to copy data into fragment "
                     "reassembly buffer.", __FILE__, __LINE__);
            break;
        }

        DCE2_MOVE(rdata, rlen, fnode->frag_len);
        stub_len += fnode->frag_len;
    }

    switch (sd->trans)
    {
        case DCE2_TRANS_TYPE__UDP:
            rpkt = DCE2_GetRpkt(sd->wire_pkt, DCE2_RPKT_TYPE__UDP_CL_FRAG, dce2_cl_rbuf, stub_len);
            if (rpkt == NULL)
            {
                DCE2_Log(DCE2_LOG_TYPE__ERROR,
                         "%s(%d) Failed to create reassembly packet.",
                         __FILE__, __LINE__);
                PREPROC_PROFILE_END(dce2_pstat_cl_reass);
                return;
            }

            DCE2_ClSetRdata(at, cl_hdr, (uint8_t *)rpkt->payload,
                            (uint16_t)(rpkt->payload_size - DCE2_MOCK_HDR_LEN__CL));

            stub_data = rpkt->payload + DCE2_MOCK_HDR_LEN__CL;

            break;

        default:
            DCE2_Log(DCE2_LOG_TYPE__ERROR,
                     "%s(%d) Invalid transport type: %d",
                     __FILE__, __LINE__, sd->trans);
            return;
    }

    PREPROC_PROFILE_END(dce2_pstat_cl_reass);

    if (DCE2_PushPkt(rpkt) != DCE2_RET__SUCCESS)
    {
        DCE2_Log(DCE2_LOG_TYPE__ERROR,
                 "%s(%d) Failed to push packet onto packet stack.",
                 __FILE__, __LINE__);
        return;
    }

    /* Cache relevant values for rule option processing */
    sd->ropts.first_frag = 1;
    DCE2_CopyUuid(&sd->ropts.iface, &ft->iface, DCERPC_BO_FLAG__NONE);
    sd->ropts.iface_vers = ft->iface_vers;
    sd->ropts.hdr_byte_order = DceRpcClByteOrder(cl_hdr);

    if (ft->data_byte_order != DCE2_SENTINEL)
        sd->ropts.data_byte_order = ft->data_byte_order;
    else
        sd->ropts.data_byte_order = DceRpcClByteOrder(cl_hdr);

    if (ft->opnum != DCE2_SENTINEL)
        sd->ropts.opnum = ft->opnum;
    else
        sd->ropts.opnum = DceRpcClOpnum(cl_hdr);

    sd->ropts.stub_data = stub_data;

    DCE2_Detect(sd);
    DCE2_PopPkt();

    dce2_stats.cl_frag_reassembled++;
}

/********************************************************************
 * Function: DCE2_ClResetFragTracker()
 *
 * Destroys the fragment tracker's fragment list and resets opnum,
 * byte order and number of expected frags to a sentinel.
 *
 * Arguments:
 *  DCE2_ClFragTracker *
 *      Pointer to the fragment tracker to reset.
 *
 * Returns: None
 *
 ********************************************************************/
static void DCE2_ClResetFragTracker(DCE2_ClFragTracker *ft)
{
    if (ft == NULL)
        return;

    if (ft->frags != NULL)
    {
        DCE2_ListDestroy(ft->frags);
        ft->frags = NULL;
    }

    ft->opnum = DCE2_SENTINEL;
    ft->data_byte_order = DCE2_SENTINEL;
    ft->num_expected_frags = DCE2_SENTINEL;
}

/********************************************************************
 * Function: DCE2_ClCleanTracker()
 *
 * Destroys all the activity tracker list, which cleans out and
 * frees all data associated with each activity tracker in the
 * list.
 *
 * Arguments:
 *  DCE2_ClTracker *
 *      Pointer to connectionless tracker.
 *
 * Returns: None
 *
 ********************************************************************/
void DCE2_ClCleanTracker(DCE2_ClTracker *clt)
{
    if (clt == NULL)
        return;

    /* Destroy activity trackers list - this will have the
     * effect of freeing everything inside of it */
    DCE2_ListDestroy(clt->act_trackers);
    clt->act_trackers = NULL;
}

/********************************************************************
 * Function: DCE2_ClActDataFree()
 *
 * Callback to activity tracker list for freeing activity trackers.
 *
 * Arguments:
 *  void *
 *      Activity tracker to free.
 *
 * Returns: None
 *
 ********************************************************************/
static void DCE2_ClActDataFree(void *data)
{
    DCE2_ClActTracker *at = (DCE2_ClActTracker *)data;

    if (at == NULL)
        return;

    DCE2_ListDestroy(at->frag_tracker.frags);
    at->frag_tracker.frags = NULL;
    DCE2_Free((void *)at, sizeof(DCE2_ClActTracker), DCE2_MEM_TYPE__CL_ACT);
}

/********************************************************************
 * Function: DCE2_ClActKeyFree()
 *
 * Callback to activity tracker list for freeing the key (this is
 * the activity UUID).  Since key is dynamically allocated, we need
 * to free it.
 *
 * Arguments:
 *  void *
 *      The activity UUID to free.
 *
 * Returns: None
 *
 ********************************************************************/
static void DCE2_ClActKeyFree(void *key)
{
    if (key == NULL)
        return;

    DCE2_Free(key, sizeof(Uuid), DCE2_MEM_TYPE__CL_ACT);
}

/********************************************************************
 * Function: DCE2_ClFragDataFree()
 *
 * Callback to fragment list for freeing data kept in list.  Need
 * to free the frag node and the data attached to it.
 *
 * Arguments:
 *  void *
 *      Pointer to fragment data (a frag node).
 *
 * Returns: None
 *
 ********************************************************************/
static void DCE2_ClFragDataFree(void *data)
{
    DCE2_ClFragNode *fn = (DCE2_ClFragNode *)data;

    if (fn == NULL)
        return;

    if (fn->frag_data != NULL)
        DCE2_Free((void *)fn->frag_data, fn->frag_len, DCE2_MEM_TYPE__CL_FRAG);

    DCE2_Free((void *)fn, sizeof(DCE2_ClFragNode), DCE2_MEM_TYPE__CL_FRAG);
}

