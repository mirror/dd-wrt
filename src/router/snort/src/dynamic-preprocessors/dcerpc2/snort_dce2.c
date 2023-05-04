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

#include <daq.h>
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_types.h"
#include "snort_dce2.h"
#include "spp_dce2.h"
#include "dce2_config.h"
#include "dce2_utils.h"
#include "dce2_list.h"
#include "dce2_stats.h"
#include "dce2_session.h"
#include "dce2_event.h"
#include "dce2_smb.h"
#include "dce2_udp.h"
#include "dce2_tcp.h"
#include "dce2_http.h"
#include "dce2_co.h"
#include "dce2_cl.h"
#include "dce2_memory.h"
#include "sf_dynamic_preprocessor.h"
#include "stream_api.h"
#include "sfrt.h"
#include "profiler.h"
#include "sfPolicy.h"
#include "sf_seqnums.h"

/********************************************************************
 * Global variables
 ********************************************************************/
DCE2_CStack *dce2_pkt_stack = NULL;
DCE2_ProtoIds dce2_proto_ids;

static SFSnortPacket* dce2_rpkt[DCE2_RPKT_TYPE__MAX] = {
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

#ifdef SNORT_RELOAD
APPDATA_ADJUSTER *ada;
#endif

static int dce2_detected = 0;

// Used to indicate that a session is no longer being looked at
// It will be the session data that is returned so a pointer check
// is sufficient to tell if the preprocessor shouldn't look at the session.
uint8_t dce2_no_inspect;

/********************************************************************
 * Macros
 ********************************************************************/
#define DCE2_PKT_STACK__SIZE  10

/********************************************************************
 * Private function prototypes
 ********************************************************************/
static DCE2_SsnData * DCE2_NewSession(SFSnortPacket *, tSfPolicyId);
static DCE2_TransType DCE2_GetTransport(SFSnortPacket *, const DCE2_ServerConfig *, int *);
static DCE2_TransType DCE2_GetDetectTransport(SFSnortPacket *, const DCE2_ServerConfig *);
static DCE2_TransType DCE2_GetAutodetectTransport(SFSnortPacket *, const DCE2_ServerConfig *);
static DCE2_Ret DCE2_SetSsnState(DCE2_SsnData *, SFSnortPacket *);
static void DCE2_SsnFree(void *);

/*********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 *********************************************************************/
static DCE2_SsnData * DCE2_NewSession(SFSnortPacket *p, tSfPolicyId policy_id)
{
    DCE2_SsnData *sd = NULL;
    DCE2_TransType trans;
    const DCE2_ServerConfig *sc = DCE2_ScGetConfig(p);
    int autodetected = 0;
    PROFILE_VARS;

    PREPROC_PROFILE_START(dce2_pstat_new_session);

    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__MAIN, "Creating new session: "));
    trans = DCE2_GetTransport(p, sc, &autodetected);
    switch (trans)
    {
        case DCE2_TRANS_TYPE__SMB:
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__MAIN, "SMB transport ... "));
            sd = (DCE2_SsnData *)DCE2_SmbSsnInit(p);
            break;

        case DCE2_TRANS_TYPE__TCP:
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__MAIN, "TCP transport ... "));
            sd = (DCE2_SsnData *)DCE2_TcpSsnInit();
            break;

        case DCE2_TRANS_TYPE__UDP:
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__MAIN, "UDP transport ... "));
            sd = (DCE2_SsnData *)DCE2_UdpSsnInit();
            break;

        case DCE2_TRANS_TYPE__HTTP_PROXY:
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__MAIN, "RPC over HTTP proxy transport ... "));
            sd = (DCE2_SsnData *)DCE2_HttpProxySsnInit();
            break;

        case DCE2_TRANS_TYPE__HTTP_SERVER:
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__MAIN, "RPC over HTTP server transport ... "));
            sd = (DCE2_SsnData *)DCE2_HttpServerSsnInit();
            break;

        case DCE2_TRANS_TYPE__NONE:
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__MAIN, "Not configured to "
                        "look at this traffic or unable to autodetect - not inspecting.\n"));
            PREPROC_PROFILE_END(dce2_pstat_new_session);
            return NULL;

        default:
            DCE2_Log(DCE2_LOG_TYPE__ERROR,
                     "%s(%d) Invalid transport type: %d",
                     __FILE__, __LINE__, trans);
            PREPROC_PROFILE_END(dce2_pstat_new_session);
            return NULL;
    }

    if (sd == NULL)
    {
        PREPROC_PROFILE_END(dce2_pstat_new_session);
        return NULL;
    }

    DCE2_SsnSetAppData(p, (void *)sd, DCE2_SsnFree);
#ifdef SNORT_RELOAD
    ada_add( ada, (void *) sd, p->stream_session );
#endif

    dce2_stats.sessions++;
    dce2_stats.sessions_active++;
    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__MAIN, "Created (%p)\n", (void *)sd));

    sd->trans = trans;
    sd->server_policy = DCE2_ScPolicy(sc);
    sd->client_policy = DCE2_POLICY__WINXP;  // Default to Windows XP
    sd->sconfig = sc;
    sd->wire_pkt = p;

    sd->policy_id = policy_id;
    sd->config = dce2_config;
    ((DCE2_Config *)sfPolicyUserDataGet(sd->config, policy_id))->ref_count++;

    if (autodetected)
    {
        dce2_stats.sessions_autodetected++;

#ifdef DEBUG
        if (DCE2_SsnFromServer(p))
            dce2_stats.autoports[p->src_port][trans]++;
        else
            dce2_stats.autoports[p->dst_port][trans]++;
#endif

        DCE2_SsnSetAutodetected(sd, p);
    }

    /* If we've determined a transport, make sure we're doing
     * reassembly on the session */
    if (IsTCP(p))
    {
        int rs_dir = DCE2_SsnGetReassembly(p);

        if (!_dpd.isPafEnabled() && (rs_dir != SSN_DIR_BOTH))
        {
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__MAIN,
                        "Setting client/server reassembly to FOOTPRINT for this session.\n"));
            DCE2_SsnSetReassembly(p);
        }

        if (!DCE2_SsnIsRebuilt(p))
        {
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__MAIN, "Got non-rebuilt packet\n"));

            if (DCE2_SsnIsStreamInsert(p))
            {
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__MAIN, "Stream inserted - not inspecting.\n"));
                PREPROC_PROFILE_END(dce2_pstat_new_session);
                return NULL;
            }
            else if ((DCE2_SsnFromClient(p) && (rs_dir == SSN_DIR_FROM_SERVER))
                     || (DCE2_SsnFromServer(p) && (rs_dir == SSN_DIR_FROM_CLIENT))
                     || (rs_dir == SSN_DIR_BOTH))
            {
                /* Reassembly was already set for this session, but stream
                 * decided not to use the packet so it's probably not good */
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__MAIN, "Got non-stream inserted packet - not inspecting\n"));
                PREPROC_PROFILE_END(dce2_pstat_new_session);
                return NULL;
            }
        }
    }

    PREPROC_PROFILE_END(dce2_pstat_new_session);
    return sd;
}

/*********************************************************************
 * Function: DCE2_Process()
 *
 * Purpose: Main entry point for DCE/RPC processing.
 *
 * Arguments:
 *  SFSnortPacket * - pointer to packet structure
 *
 * Returns:
 *  DCE2_Ret - status
 *
 *********************************************************************/
DCE2_Ret DCE2_Process(SFSnortPacket *p)
{
    tSfPolicyId policy_id = _dpd.getNapRuntimePolicy();
    DCE2_SsnData *sd = (DCE2_SsnData *)DCE2_SsnGetAppData(p);
    PROFILE_VARS;

    PREPROC_PROFILE_START(dce2_pstat_session);

    if ((sd != NULL) && DCE2_SsnNoInspect(sd))
    {
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__MAIN, "Session set to "
                    "not inspect.  Returning\n"));
        PREPROC_PROFILE_END(dce2_pstat_session);
        return DCE2_RET__NOT_INSPECTED;
    }

    dce2_eval_config = (DCE2_Config *)sfPolicyUserDataGet(dce2_config, policy_id);
    if (sd != NULL)
        dce2_eval_config = (DCE2_Config *)sfPolicyUserDataGet(sd->config, sd->policy_id);

    if (dce2_eval_config == NULL)
    {
        PREPROC_PROFILE_END(dce2_pstat_session);
        return DCE2_RET__NOT_INSPECTED;
    }

    if (sd == NULL)
    {
        sd = DCE2_NewSession(p, policy_id);
        if (sd == NULL)
        {
            PREPROC_PROFILE_END(dce2_pstat_session);
            return DCE2_RET__NOT_INSPECTED;
        }
    }
    else
    {
        sd->wire_pkt = p;

        if (_dpd.isPafEnabled() && !DCE2_SsnIsPafActive(p))
        {
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__MAIN, "PAF was aborted on "
                        "one or both sides - aborting session inspection\n"));
            DCE2_SetNoInspect(sd);
            PREPROC_PROFILE_END(dce2_pstat_session);
            return DCE2_RET__NOT_INSPECTED;
        }

        if (IsTCP(p) && !DCE2_SsnIsRebuilt(p))
        {
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__MAIN, "Got non-rebuilt packet "
                        "on session (%p)\n", (void *)sd));

            if (DCE2_SsnIsStreamInsert(p))
            {
                if (!_dpd.isPafEnabled())
                {
                    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__MAIN, "Flushing opposite direction.\n"));
                    DCE2_SsnFlush(p);
                }

                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__MAIN, "Stream inserted - not inspecting.\n"));
            }
            else
            {
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__MAIN, "Got non-stream inserted packet "
                            "- not inspecting\n"));
            }

            PREPROC_PROFILE_END(dce2_pstat_session);
            return DCE2_RET__NOT_INSPECTED;
        }
        else if (DCE2_SsnAutodetected(sd) && !(p->flags & sd->autodetect_dir))
        {
            /* Try to autodetect in opposite direction */
            if ((sd->trans != DCE2_TRANS_TYPE__HTTP_PROXY) &&
                    (sd->trans != DCE2_TRANS_TYPE__HTTP_SERVER) &&
                    (DCE2_GetAutodetectTransport(p, sd->sconfig) != sd->trans))
            {
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__MAIN, "Bad autodetect.\n"));

                DCE2_SetNoInspect(sd);
                dce2_stats.bad_autodetects++;

                PREPROC_PROFILE_END(dce2_pstat_session);
                return DCE2_RET__NOT_INSPECTED;
            }

            DCE2_SsnClearAutodetected(sd);
        }
    }

    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__MAIN, "Session pointer: %p\n", (void *)sd));

    if (IsTCP(p) && (DCE2_SetSsnState(sd, p) != DCE2_RET__SUCCESS))
    {
        PREPROC_PROFILE_END(dce2_pstat_session);
        return DCE2_RET__NOT_INSPECTED;
    }

    if (DCE2_PushPkt((void *)p) != DCE2_RET__SUCCESS)
    {
        DCE2_Log(DCE2_LOG_TYPE__ERROR,
                 "%s(%d) Failed to push packet onto packet stack.",
                 __FILE__, __LINE__);
        PREPROC_PROFILE_END(dce2_pstat_session);
        return DCE2_RET__NOT_INSPECTED;
    }

    p->flags |= FLAG_ALLOW_MULTIPLE_DETECT;
    dce2_detected = 0;

    PREPROC_PROFILE_END(dce2_pstat_session);

    switch (sd->trans)
    {
        case DCE2_TRANS_TYPE__SMB:
            DCE2_SmbProcess((DCE2_SmbSsnData *)sd);
            break;
        case DCE2_TRANS_TYPE__TCP:
            DCE2_TcpProcess((DCE2_TcpSsnData *)sd);
            break;
        case DCE2_TRANS_TYPE__UDP:
            DCE2_UdpProcess((DCE2_UdpSsnData *)sd);
            break;
        case DCE2_TRANS_TYPE__HTTP_PROXY:
            DCE2_HttpProcessProxy((DCE2_HttpSsnData *)sd);
            break;
        case DCE2_TRANS_TYPE__HTTP_SERVER:
            DCE2_HttpProcessServer((DCE2_HttpSsnData *)sd);
            break;
        default:
            DCE2_Log(DCE2_LOG_TYPE__ERROR,
                     "%s(%d) Invalid transport type: %d",
                     __FILE__, __LINE__, sd->trans);
            return DCE2_RET__NOT_INSPECTED;
    }

    if (sd->flags & DCE2_SSN_FLAG__NO_INSPECT)
    {
        DCE2_SetNoInspect(sd);
        DCE2_PopPkt();
        PREPROC_PROFILE_END(dce2_pstat_session);
        return DCE2_RET__NOT_INSPECTED;
    }

    if (!dce2_detected)
        DCE2_Detect(sd);

    DCE2_ResetRopts(&sd->ropts);
    DCE2_PopPkt();

    if (dce2_mem_state == DCE2_MEM_STATE__MEMCAP)
    {
        DCE2_SetNoInspect(sd);
        dce2_mem_state = DCE2_MEM_STATE__OKAY;
        return DCE2_RET__NOT_INSPECTED;
    }

    if (DCE2_SsnAutodetected(sd))
        return DCE2_RET__NOT_INSPECTED;

    return DCE2_RET__INSPECTED;
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
void DCE2_SetNoInspect(DCE2_SsnData *sd)
{
    if (sd == NULL)
        return;

    dce2_stats.sessions_aborted++;
    DCE2_SsnSetNoInspect(sd->wire_pkt);
}

/********************************************************************
 * Function: DCE2_SetSsnState()
 *
 * Purpose:
 *  Checks for missing packets and overlapping data on session
 *
 * Arguments:
 *  DCE2_SsnData *  - session data pointer
 *  SFSnortPacket * - packet structure
 *
 * Returns:
 *  DCE2_RET__SUCCESS
 *  DCE2_RET__ERROR
 *
 ********************************************************************/
static DCE2_Ret DCE2_SetSsnState(DCE2_SsnData *sd, SFSnortPacket *p)
{
    uint32_t pkt_seq = ntohl(p->tcp_header->sequence);
    PROFILE_VARS;

    PREPROC_PROFILE_START(dce2_pstat_session_state);

    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__MAIN, "Payload size: %u\n", p->payload_size));

    if (DCE2_SsnFromClient(p) && !DCE2_SsnSeenClient(sd))
    {
        uint32_t pkt_ack = ntohl(p->tcp_header->acknowledgement);

        if (DCE2_SsnSeenServer(sd) && SEQ_LT(sd->cli_seq, pkt_seq)
                && (sd->trans != DCE2_TRANS_TYPE__HTTP_SERVER))
        {
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__MAIN,
                        "Missing packets on session - aborting session inspection\n"));
            DCE2_SetNoInspect(sd);
            PREPROC_PROFILE_END(dce2_pstat_session_state);
            return DCE2_RET__ERROR;
        }

        DCE2_SsnSetSeenClient(sd);
        sd->cli_seq = pkt_seq;
        sd->cli_nseq = pkt_seq + p->payload_size;

        if (!DCE2_SsnSeenServer(sd))
            sd->srv_seq = sd->srv_nseq = pkt_ack;

        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__MAIN, "Initial client => seq: %u, "
                    "next seq: %u\n", sd->cli_seq, sd->cli_nseq));
    }
    else if (DCE2_SsnFromServer(p) && !DCE2_SsnSeenServer(sd))
    {
        uint32_t pkt_ack = ntohl(p->tcp_header->acknowledgement);

        if ((DCE2_SsnSeenClient(sd) && SEQ_LT(sd->srv_seq, pkt_seq))
                || (!DCE2_SsnSeenClient(sd) && (sd->trans != DCE2_TRANS_TYPE__HTTP_SERVER)))
        {
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__MAIN,
                        "Missing packets on session - aborting session inspection\n"));
            PREPROC_PROFILE_END(dce2_pstat_session_state);
            DCE2_SetNoInspect(sd);
            return DCE2_RET__ERROR;
        }

        DCE2_SsnSetSeenServer(sd);
        sd->srv_seq = pkt_seq;
        sd->srv_nseq = pkt_seq + p->payload_size;

        if (!DCE2_SsnSeenClient(sd))
            sd->cli_seq = sd->cli_nseq = pkt_ack;

        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__MAIN, "Initial server => seq: %u, "
                    "next seq: %u\n", sd->srv_seq, sd->srv_nseq));
    }
    else
    {
        uint32_t *ssn_seq;
        uint32_t *ssn_nseq;

        if (DCE2_SsnFromClient(p))
        {
            ssn_seq = &sd->cli_seq;
            ssn_nseq = &sd->cli_nseq;

            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__MAIN, "Client last => seq: %u, "
                        "next seq: %u\n", sd->cli_seq, sd->cli_nseq));
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__MAIN, "This packet => seq: %u, "
                        "next seq: %u\n", pkt_seq, pkt_seq + p->payload_size));
        }
        else
        {
            ssn_seq = &sd->srv_seq;
            ssn_nseq = &sd->srv_nseq;

            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__MAIN, "Server last => seq: %u, "
                        "next seq: %u\n", sd->srv_seq, sd->srv_nseq));
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__MAIN, "This packet => seq: %u, "
                        "next seq: %u\n", pkt_seq, pkt_seq + p->payload_size));
        }

        if (*ssn_nseq != pkt_seq)
        {
            if (SEQ_LT(*ssn_nseq, pkt_seq))
            {
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__MAIN, "Next expected sequence number (%u) is less than "
                               "this sequence number (%u).\n", *ssn_nseq, pkt_seq));
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__MAIN,
                            "Missing packets on session - aborting session inspection\n"));

                DCE2_SetNoInspect(sd);
                PREPROC_PROFILE_END(dce2_pstat_session_state);
                return DCE2_RET__ERROR;
            }
            else
            {
                /* Got some kind of overlap.  This shouldn't happen since we're doing
                 * reassembly on both sides and not looking at non-reassembled packets
                 * Actually this can happen if the stream seg list is empty */
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__MAIN, "Overlap => seq: %u, "
                            "next seq: %u - aborting session inspection\n",
                            pkt_seq, pkt_seq + p->payload_size));

                DCE2_SetNoInspect(sd);
                PREPROC_PROFILE_END(dce2_pstat_session_state);
                return DCE2_RET__ERROR;
            }
        }

        *ssn_seq = pkt_seq;
        *ssn_nseq = pkt_seq + p->payload_size;
    }

    PREPROC_PROFILE_END(dce2_pstat_session_state);
    return DCE2_RET__SUCCESS;
}

/*********************************************************************
 * Function: DCE2_GetTransport()
 *
 * Determines whether or not we should look at this traffic and if
 * so, what transport it should be classified as.
 *
 * Arguments:
 *  SFSnortPacket *
 *      Pointer to packet structure.
 *  const DCE2_ServerConfig *
 *      The server configuration associated with the packet's IP.
 *  int *
 *      Pointer to a value that will be filled in with whether
 *      or not the packet was autodetected.
 *      Non-zero if autodetected
 *      Zero if not autodetected
 *
 * Returns:
 *  DCE2_TransType
 *      DCE2_TRANS_TYPE__NONE if a transport could not be
 *          determined or target based labeled the session as
 *          traffic we are not interested in.
 *      DCE2_TRANS_TYPE__SMB if the traffic is determined to be
 *          DCE/RPC over SMB.
 *      DCE2_TRANS_TYPE__TCP if the traffic is determined to be
 *          DCE/RPC over TCP.
 *      DCE2_TRANS_TYPE__UDP if the traffic is determined to be
 *          DCE/RPC over UDP.
 *      DCE2_TRANS_TYPE__HTTP_PROXY if the traffic is determined
 *          to be DCE/RPC over HTTP proxy.
 *      DCE2_TRANS_TYPE__HTTP_SERVER if the traffic is determined
 *          to be DCE/RPC over HTTP server.
 *
 *********************************************************************/
static DCE2_TransType DCE2_GetTransport(SFSnortPacket *p, const DCE2_ServerConfig *sc, int *autodetected)
{
    DCE2_TransType trans = DCE2_TRANS_TYPE__NONE;
#ifdef TARGET_BASED
    int16_t proto_id = 0;
#endif

    *autodetected = 0;

#ifdef TARGET_BASED
    if (_dpd.isAdaptiveConfigured())
    {
        proto_id = _dpd.sessionAPI->get_application_protocol_id(p->stream_session);

        if (proto_id == SFTARGET_UNKNOWN_PROTOCOL)
            return DCE2_TRANS_TYPE__NONE;
    }

    if (proto_id != 0)
    {
        if (proto_id == dce2_proto_ids.dcerpc)
        {
            if (IsTCP(p))
            {
                return DCE2_TRANS_TYPE__TCP;
            }
            else
            {
                return DCE2_TRANS_TYPE__UDP;
            }
        }
        else if (proto_id == dce2_proto_ids.nbss)
        {
            return DCE2_TRANS_TYPE__SMB;
        }
    }
    else
#endif
    {
        trans = DCE2_GetDetectTransport(p, sc);
        if (trans == DCE2_TRANS_TYPE__NONE)
        {
            trans = DCE2_GetAutodetectTransport(p, sc);
            *autodetected = 1;
        }
        else if ((trans == DCE2_TRANS_TYPE__HTTP_PROXY) &&
                 (DCE2_ScAutodetectHttpProxyPorts(sc) == DCE2_CS__ENABLED))
        {
            trans = DCE2_HttpAutodetectProxy(p);
            *autodetected = 1;
        }
    }

    return trans;
}

/*********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 *********************************************************************/
static DCE2_TransType DCE2_GetDetectTransport(SFSnortPacket *p, const DCE2_ServerConfig *sc)
{
    DCE2_TransType trans = DCE2_TRANS_TYPE__NONE;
    uint16_t port;

    if (DCE2_SsnFromServer(p))
        port = p->src_port;
    else
        port = p->dst_port;

    /* Check our configured ports to see if we should continue processing */
    if (IsTCP(p))
    {
        if (DCE2_ScIsDetectPortSet(sc, port, DCE2_TRANS_TYPE__SMB))
            trans = DCE2_TRANS_TYPE__SMB;
        else if (DCE2_ScIsDetectPortSet(sc, port, DCE2_TRANS_TYPE__TCP))
            trans = DCE2_TRANS_TYPE__TCP;
        else if (DCE2_ScIsDetectPortSet(sc, port, DCE2_TRANS_TYPE__HTTP_PROXY))
            trans = DCE2_TRANS_TYPE__HTTP_PROXY;
        else if (DCE2_ScIsDetectPortSet(sc, port, DCE2_TRANS_TYPE__HTTP_SERVER))
            trans = DCE2_TRANS_TYPE__HTTP_SERVER;
    }
    else  /* it's UDP */
    {
        if (DCE2_ScIsDetectPortSet(sc, port, DCE2_TRANS_TYPE__UDP))
            trans = DCE2_TRANS_TYPE__UDP;
    }

    return trans;
}

/*********************************************************************
 * Function: DCE2_GetAutodetectTransport()
 *
 *
 * Arguments:
 *
 * Returns:
 *
 *********************************************************************/
static DCE2_TransType DCE2_GetAutodetectTransport(SFSnortPacket *p, const DCE2_ServerConfig *sc)
{
    DCE2_TransType trans = DCE2_TRANS_TYPE__NONE;
    uint16_t port;

    if (DCE2_SsnFromServer(p))
        port = p->src_port;
    else
        port = p->dst_port;

    if (IsTCP(p))
    {
        /* Look for raw DCE/RCP over TCP first, since it's
         * more likely not to have configured a port for this. */
        if (DCE2_ScIsAutodetectPortSet(sc, port, DCE2_TRANS_TYPE__TCP))
        {
            trans = DCE2_TcpAutodetect(p);
            if (trans != DCE2_TRANS_TYPE__NONE)
                return trans;
        }

        if (DCE2_ScIsAutodetectPortSet(sc, port, DCE2_TRANS_TYPE__HTTP_SERVER))
        {
            trans = DCE2_HttpAutodetectServer(p);
            if (trans != DCE2_TRANS_TYPE__NONE)
                return trans;
        }

        if (DCE2_ScIsAutodetectPortSet(sc, port, DCE2_TRANS_TYPE__HTTP_PROXY))
        {
            trans = DCE2_HttpAutodetectProxy(p);
            if (trans != DCE2_TRANS_TYPE__NONE)
                return trans;
        }

        if (DCE2_ScIsAutodetectPortSet(sc, port, DCE2_TRANS_TYPE__SMB))
        {
            trans = DCE2_SmbAutodetect(p);
            if (trans != DCE2_TRANS_TYPE__NONE)
                return trans;
        }
    }
    else  /* it's UDP */
    {
        if (DCE2_ScIsAutodetectPortSet(sc, port, DCE2_TRANS_TYPE__UDP))
        {
            trans = DCE2_UdpAutodetect(p);
            if (trans != DCE2_TRANS_TYPE__NONE)
                return trans;
        }
    }

    return DCE2_TRANS_TYPE__NONE;
}

/*********************************************************************
 * Function: DCE2_InitRpkts()
 *
 * Purpose: Allocate and initialize reassembly packets.
 *
 * Arguments: None
 *
 * Returns: None
 *
 *********************************************************************/
void DCE2_InitRpkts(void)
{
    int i;
    dce2_pkt_stack = DCE2_CStackNew(DCE2_PKT_STACK__SIZE, NULL, DCE2_MEM_TYPE__INIT);

    if (dce2_pkt_stack == NULL)
    {
        DCE2_Die("%s(%d) Failed to allocate memory for packet stack.",
                 __FILE__, __LINE__);
    }
    for ( i = 0; i < DCE2_RPKT_TYPE__MAX; i++ )
        dce2_rpkt[i] = _dpd.encodeNew();
}

/*********************************************************************
 * Function: DCE2_GetRpkt()
 *
 * Purpose:
 *
 * Arguments:
 *  SFSnortPacket *  - pointer to packet off wire
 *  const uint8_t *  - pointer to data to attach to reassembly packet
 *  uint16_t - length of data
 *
 * Returns:
 *  SFSnortPacket * - pointer to reassembly packet
 *
 *********************************************************************/
SFSnortPacket * DCE2_GetRpkt(const SFSnortPacket *wire_pkt, DCE2_RpktType rpkt_type,
                             const uint8_t *data, uint32_t data_len)
{
    DCE2_Ret status;
    SFSnortPacket *rpkt;
    uint16_t payload_len = 0;
    uint16_t data_overhead = 0;

    rpkt = dce2_rpkt[rpkt_type];

    switch (rpkt_type)
    {
        case DCE2_RPKT_TYPE__SMB_SEG:
            _dpd.encodeFormat(ENC_DYN_FWD, wire_pkt, rpkt, PSEUDO_PKT_SMB_SEG);
            break;

        case DCE2_RPKT_TYPE__SMB_TRANS:
            // TBD these memset()s could be encapsulated by the various
            // init functions which should also return the data_overhead.
            // Better still pass in rpkt and let the init function update
            // payload, etc.  Also, some memsets could probably be avoided
            // by explicitly setting the unitialized header fields.
            _dpd.encodeFormat(ENC_DYN_FWD, wire_pkt, rpkt, PSEUDO_PKT_SMB_TRANS);

            if (DCE2_SsnFromClient(wire_pkt))
            {
                data_overhead = DCE2_MOCK_HDR_LEN__SMB_CLI;
                memset((void*)rpkt->payload, 0, data_overhead);
                DCE2_SmbInitRdata((uint8_t *)rpkt->payload, FLAG_FROM_CLIENT);
            }
            else
            {
                data_overhead = DCE2_MOCK_HDR_LEN__SMB_SRV;
                memset((void*)rpkt->payload, 0, data_overhead);
                DCE2_SmbInitRdata((uint8_t *)rpkt->payload, FLAG_FROM_SERVER);
            }
            break;

        case DCE2_RPKT_TYPE__SMB_CO_SEG:
            _dpd.encodeFormat(ENC_DYN_FWD, wire_pkt, rpkt, PSEUDO_PKT_DCE_SEG);

            if (DCE2_SsnFromClient(wire_pkt))
            {
                data_overhead = DCE2_MOCK_HDR_LEN__SMB_CLI;
                memset((void*)rpkt->payload, 0, data_overhead);
                DCE2_SmbInitRdata((uint8_t *)rpkt->payload, FLAG_FROM_CLIENT);
            }
            else
            {
                data_overhead = DCE2_MOCK_HDR_LEN__SMB_SRV;
                memset((void*)rpkt->payload, 0, data_overhead);
                DCE2_SmbInitRdata((uint8_t *)rpkt->payload, FLAG_FROM_SERVER);
            }
            break;

        case DCE2_RPKT_TYPE__SMB_CO_FRAG:
            _dpd.encodeFormat(ENC_DYN_FWD, wire_pkt, rpkt, PSEUDO_PKT_DCE_FRAG);

            if (DCE2_SsnFromClient(wire_pkt))
            {
                data_overhead = DCE2_MOCK_HDR_LEN__SMB_CLI + DCE2_MOCK_HDR_LEN__CO_CLI;
                memset((void*)rpkt->payload, 0, data_overhead);
                DCE2_SmbInitRdata((uint8_t *)rpkt->payload, FLAG_FROM_CLIENT);
                DCE2_CoInitRdata((uint8_t *)rpkt->payload +
                    DCE2_MOCK_HDR_LEN__SMB_CLI, FLAG_FROM_CLIENT);
            }
            else
            {
                data_overhead = DCE2_MOCK_HDR_LEN__SMB_SRV + DCE2_MOCK_HDR_LEN__CO_SRV;
                memset((void*)rpkt->payload, 0, data_overhead);
                DCE2_SmbInitRdata((uint8_t *)rpkt->payload, FLAG_FROM_SERVER);
                DCE2_CoInitRdata((uint8_t *)rpkt->payload +
                    DCE2_MOCK_HDR_LEN__SMB_SRV, FLAG_FROM_SERVER);
            }
            break;

        case DCE2_RPKT_TYPE__TCP_CO_SEG:
            _dpd.encodeFormat(ENC_DYN_FWD, wire_pkt, rpkt, PSEUDO_PKT_DCE_SEG);
            break;

        case DCE2_RPKT_TYPE__TCP_CO_FRAG:
            _dpd.encodeFormat(ENC_DYN_FWD, wire_pkt, rpkt, PSEUDO_PKT_DCE_FRAG);

            if (DCE2_SsnFromClient(wire_pkt))
            {
                data_overhead = DCE2_MOCK_HDR_LEN__CO_CLI;
                memset((void*)rpkt->payload, 0, data_overhead);
                DCE2_CoInitRdata((uint8_t *)rpkt->payload, FLAG_FROM_CLIENT);
            }
            else
            {
                data_overhead = DCE2_MOCK_HDR_LEN__CO_SRV;
                memset((void*)rpkt->payload, 0, data_overhead);
                DCE2_CoInitRdata((uint8_t *)rpkt->payload, FLAG_FROM_SERVER);
            }
            break;

        case DCE2_RPKT_TYPE__UDP_CL_FRAG:
            _dpd.encodeFormat(ENC_DYN_FWD, wire_pkt, rpkt, PSEUDO_PKT_DCE_FRAG);
            data_overhead = DCE2_MOCK_HDR_LEN__CL;
            memset((void*)rpkt->payload, 0, data_overhead);
            DCE2_ClInitRdata((uint8_t *)rpkt->payload);
            break;

        default:
            DCE2_Log(DCE2_LOG_TYPE__ERROR,
                     "%s(%d) Invalid reassembly packet type: %d",
                     __FILE__, __LINE__, rpkt_type);
            return NULL;
    }

    payload_len = rpkt->max_payload;

    if ((data_overhead + data_len) > payload_len)
        data_len -= (data_overhead + data_len) - payload_len;

    status = DCE2_Memcpy(
        (void *)(rpkt->payload + data_overhead),
        (void *)data, (size_t)data_len, (void *)rpkt->payload,
        (void *)((uint8_t *)rpkt->payload + payload_len));

    if (status != DCE2_RET__SUCCESS)
    {
        DCE2_Log(DCE2_LOG_TYPE__ERROR,
                 "%s(%d) Failed to copy data into reassembly packet.",
                 __FILE__, __LINE__);
        return NULL;
    }

    rpkt->payload_size = (uint16_t)(data_overhead + data_len);
    _dpd.encodeUpdate(rpkt);

    if (wire_pkt->family == AF_INET)
    {
        rpkt->ip4h->ip_len = rpkt->ip4_header->data_length;
    }
    else
    {
        IP6RawHdr* ip6h = (IP6RawHdr*)rpkt->raw_ip6_header;
        if ( ip6h ) rpkt->ip6h->len = ip6h->ip6_payload_len;
    }

    rpkt->flags |= FLAG_STREAM_EST;
    if (DCE2_SsnFromClient(wire_pkt))
        rpkt->flags |= FLAG_FROM_CLIENT;
    else
        rpkt->flags |= FLAG_FROM_SERVER;
    rpkt->stream_session = wire_pkt->stream_session;

    return rpkt;
}

/*********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 *********************************************************************/
DCE2_Ret DCE2_AddDataToRpkt(SFSnortPacket *rpkt, DCE2_RpktType rtype,
                            const uint8_t *data, uint32_t data_len)
{
    int hdr_overhead = 0;
    const uint8_t *pkt_data_end;
    const uint8_t *payload_end;
    DCE2_Ret status;

    if ((rpkt == NULL) || (data == NULL) || (data_len == 0))
        return DCE2_RET__ERROR;

    if (rpkt->payload == NULL)
        return DCE2_RET__ERROR;

    /* This is a check to make sure we don't overwrite header data */
    switch (rtype)
    {
        case DCE2_RPKT_TYPE__SMB_CO_SEG:
            if (DCE2_SsnFromClient(rpkt))
                hdr_overhead = DCE2_MOCK_HDR_LEN__SMB_CLI;
            else
                hdr_overhead = DCE2_MOCK_HDR_LEN__SMB_SRV;
            break;

        case DCE2_RPKT_TYPE__SMB_CO_FRAG:
            if (DCE2_SsnFromClient(rpkt))
                hdr_overhead = DCE2_MOCK_HDR_LEN__SMB_CLI + DCE2_MOCK_HDR_LEN__CO_CLI;
            else
                hdr_overhead = DCE2_MOCK_HDR_LEN__SMB_SRV + DCE2_MOCK_HDR_LEN__CO_SRV;
            break;

        case DCE2_RPKT_TYPE__TCP_CO_FRAG:
            if (DCE2_SsnFromClient(rpkt))
                hdr_overhead = DCE2_MOCK_HDR_LEN__CO_CLI;
            else
                hdr_overhead = DCE2_MOCK_HDR_LEN__CO_SRV;
            break;

        case DCE2_RPKT_TYPE__UDP_CL_FRAG:
            hdr_overhead = DCE2_MOCK_HDR_LEN__CL;
            break;

        default:
            break;
    }

    if (rpkt->payload_size < hdr_overhead)
        return DCE2_RET__ERROR;

    pkt_data_end = rpkt->pkt_data + rpkt->max_payload;
    payload_end = rpkt->payload + rpkt->payload_size;

    if ((payload_end + data_len) > pkt_data_end)
        data_len = pkt_data_end - payload_end;

    status = DCE2_Memcpy((void *)payload_end, (void *)data, (size_t)data_len,
                         (void *)payload_end, (void *)pkt_data_end);

    if (status != DCE2_RET__SUCCESS)
    {
        DCE2_Log(DCE2_LOG_TYPE__ERROR,
                 "%s(%d) Failed to copy data into reassembly packet.",
                 __FILE__, __LINE__);
        return DCE2_RET__ERROR;
    }

    rpkt->payload_size += (uint16_t)data_len;
    // there is room for optimization here since the update was done
    // earlier - that my be eliminated, but only in this case one
    // approach is to move the updates to push pkt - but don't want
    // to update non-dce2 pseudo pkts; perhaps a flag check there
    // will suffice.
    _dpd.encodeUpdate(rpkt);

    if (rpkt->family == AF_INET)
    {
        rpkt->ip4h->ip_len = rpkt->ip4_header->data_length;
    }
    else
    {
        IP6RawHdr* ip6h = (IP6RawHdr*)rpkt->raw_ip6_header;
        if ( ip6h ) rpkt->ip6h->len = ip6h->ip6_payload_len;
    }
    return DCE2_RET__SUCCESS;
}

/*********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 *********************************************************************/
DCE2_Ret DCE2_PushPkt(SFSnortPacket *p)
{
    SFSnortPacket *top_pkt = (SFSnortPacket *)DCE2_CStackTop(dce2_pkt_stack);

    if (top_pkt != NULL)
    {
        PROFILE_VARS;

        PREPROC_PROFILE_START(dce2_pstat_log);

        _dpd.pushAlerts();
        _dpd.logAlerts((void *)top_pkt);
        _dpd.resetAlerts();
        _dpd.popAlerts();

        PREPROC_PROFILE_END(dce2_pstat_log);
    }

    if (DCE2_CStackPush(dce2_pkt_stack, (void *)p) != DCE2_RET__SUCCESS)
        return DCE2_RET__ERROR;

    return DCE2_RET__SUCCESS;
}

/*********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 *********************************************************************/
void DCE2_PopPkt(void)
{
    SFSnortPacket *pop_pkt = (SFSnortPacket *)DCE2_CStackPop(dce2_pkt_stack);
    PROFILE_VARS;

    PREPROC_PROFILE_START(dce2_pstat_log);

    if (pop_pkt == NULL)
    {
        DCE2_Log(DCE2_LOG_TYPE__ERROR,
                 "%s(%d) No packet to pop off stack.",
                 __FILE__, __LINE__);
        PREPROC_PROFILE_END(dce2_pstat_log);
        return;
    }

    _dpd.pushAlerts();
    _dpd.logAlerts((void *)pop_pkt);
    _dpd.resetAlerts();
    _dpd.popAlerts();

    PREPROC_PROFILE_END(dce2_pstat_log);
}

/*********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 *********************************************************************/
void DCE2_Detect(DCE2_SsnData *sd)
{
    SFSnortPacket *top_pkt = (SFSnortPacket *)DCE2_CStackTop(dce2_pkt_stack);
    PROFILE_VARS;

    if (top_pkt == NULL)
    {
        DCE2_Log(DCE2_LOG_TYPE__ERROR,
                 "%s(%d) No packet on top of stack.",
                 __FILE__, __LINE__);
        return;
    }

    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__MAIN, "Detecting ------------------------------------------------\n"));
    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS, " Rule options:\n"));
    DCE2_DEBUG_CODE(DCE2_DEBUG__ROPTIONS, DCE2_PrintRoptions(&sd->ropts););
    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__MAIN, "Payload:\n"));
    DCE2_DEBUG_CODE(DCE2_DEBUG__MAIN, DCE2_PrintPktData(top_pkt->payload, top_pkt->payload_size););
    DCE2_DEBUG_CODE(DCE2_DEBUG__ROPTIONS,
            if (sd->ropts.stub_data != NULL) {
            printf("\nStub data:\n");
            DCE2_PrintPktData(sd->ropts.stub_data,
                top_pkt->payload_size - (sd->ropts.stub_data - top_pkt->payload)); });

    PREPROC_PROFILE_START(dce2_pstat_detect);

    _dpd.pushAlerts();
    _dpd.detect(top_pkt);
    _dpd.popAlerts();

    PREPROC_PROFILE_END(dce2_pstat_detect);

    /* Always reset rule option data after detecting */
    DCE2_ResetRopts(&sd->ropts);
    dce2_detected = 1;
    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__MAIN, "----------------------------------------------------------\n"));
}

void DCE2_FileDetect(DCE2_SsnData *sd)
{
    SFSnortPacket *top_pkt = (SFSnortPacket *)DCE2_CStackTop(dce2_pkt_stack);
    PROFILE_VARS;

    if (top_pkt == NULL)
    {
        DCE2_Log(DCE2_LOG_TYPE__ERROR,
                 "%s(%d) No packet on top of stack.",
                 __FILE__, __LINE__);
        return;
    }

    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__MAIN, "Detecting ------------------------------------------------\n"));
    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__MAIN, "Payload:\n"));
    DCE2_DEBUG_CODE(DCE2_DEBUG__MAIN, DCE2_PrintPktData(top_pkt->payload, top_pkt->payload_size););

    PREPROC_PROFILE_START(dce2_pstat_smb_file_detect);

    _dpd.pushAlerts();
    _dpd.detect(top_pkt);
    _dpd.popAlerts();

    PREPROC_PROFILE_END(dce2_pstat_smb_file_detect);

    // Reset file data pointer after detecting
    _dpd.setFileDataPtr(NULL, 0);
    dce2_detected = 1;
    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__MAIN, "----------------------------------------------------------\n"));
}

/*********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 *********************************************************************/
// TBD this function could be called on the actual rpkt
// to very easily get the exact available payload space and
// then truncate the data as needed.  That avoids the calculations
// here which inevitably include tacit assumptions about the
// rpkt which may not be true (nor future proof).
uint16_t DCE2_GetRpktMaxData(DCE2_SsnData *sd, DCE2_RpktType rtype)
{
    const SFSnortPacket *p = sd->wire_pkt;
    uint16_t overhead;

    uint8_t* base, *last;
    int n = p->next_layer_index - 1;
    if ( n < 2 ) return 0;

    base = p->proto_layers[1].proto_start;
    last = p->proto_layers[n].proto_start + p->proto_layers[n].proto_length;

    overhead = last - base;

    switch (rtype)
    {
        case DCE2_RPKT_TYPE__SMB_SEG:
        case DCE2_RPKT_TYPE__SMB_TRANS:
            break;

        case DCE2_RPKT_TYPE__SMB_CO_SEG:
            if (DCE2_SsnFromClient(p))
                overhead += DCE2_MOCK_HDR_LEN__SMB_CLI;
            else
                overhead += DCE2_MOCK_HDR_LEN__SMB_SRV;
            break;

        case DCE2_RPKT_TYPE__SMB_CO_FRAG:
            if (DCE2_SsnFromClient(p))
                overhead += DCE2_MOCK_HDR_LEN__SMB_CLI + DCE2_MOCK_HDR_LEN__CO_CLI;
            else
                overhead += DCE2_MOCK_HDR_LEN__SMB_SRV + DCE2_MOCK_HDR_LEN__CO_SRV;
            break;

        case DCE2_RPKT_TYPE__TCP_CO_SEG:
            break;

        case DCE2_RPKT_TYPE__TCP_CO_FRAG:
            if (DCE2_SsnFromClient(p))
                overhead += DCE2_MOCK_HDR_LEN__CO_CLI;
            else
                overhead += DCE2_MOCK_HDR_LEN__CO_SRV;
            break;

        case DCE2_RPKT_TYPE__UDP_CL_FRAG:
            overhead += DCE2_MOCK_HDR_LEN__CL;
            break;

        default:
            DCE2_Log(DCE2_LOG_TYPE__ERROR,
                     "%s(%d) Invalid reassembly packet type: %d",
                     __FILE__, __LINE__, rtype);
            return 0;
    }
    return (IP_MAXPKT - overhead);
}

/******************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ******************************************************************/
void DCE2_FreeGlobals(void)
{
    int i;

    if (dce2_pkt_stack != NULL)
    {
        DCE2_CStackDestroy(dce2_pkt_stack);
        dce2_pkt_stack = NULL;
    }

    for ( i = 0; i < DCE2_RPKT_TYPE__MAX; i++ )
    {
        if ( dce2_rpkt[i] != NULL )
        {
            _dpd.encodeDelete(dce2_rpkt[i]);
            dce2_rpkt[i] = NULL;
        }
    }

    DCE2_EventsFree();
}

static void DCE2_SsnFree(void *data)
{
    DCE2_SsnData *sd = (DCE2_SsnData *)data;
#ifdef SNORT_RELOAD
    DCE2_Config *pPolicyConfig;
    tSfPolicyUserContextId config;
    tSfPolicyId policy_id;
#endif

    if (sd == NULL)
        return;

#ifdef SNORT_RELOAD
    ada_appdata_freed( ada, data );
    config = sd->config;
    policy_id = sd->policy_id;
#endif

    switch (sd->trans)
    {
        case DCE2_TRANS_TYPE__SMB:
            DCE2_SmbSsnFree((DCE2_SmbSsnData *)sd);
            break;

        case DCE2_TRANS_TYPE__TCP:
            DCE2_TcpSsnFree((DCE2_TcpSsnData *)sd);
            break;

        case DCE2_TRANS_TYPE__UDP:
            DCE2_UdpSsnFree((DCE2_UdpSsnData *)sd);
            break;

        case DCE2_TRANS_TYPE__HTTP_SERVER:
        case DCE2_TRANS_TYPE__HTTP_PROXY:
            DCE2_HttpSsnFree((DCE2_HttpSsnData *)sd);
            break;

        default:
            DCE2_Log(DCE2_LOG_TYPE__ERROR,
                     "%s(%d) Invalid transport type: %d",
                     __FILE__, __LINE__, sd->trans);
            return;
    }

#ifdef SNORT_RELOAD
    pPolicyConfig = (DCE2_Config *)sfPolicyUserDataGet(config, policy_id);

    if (pPolicyConfig != NULL)
    {
        pPolicyConfig->ref_count--;
        if ((pPolicyConfig->ref_count == 0) &&
            (config != dce2_config))
        {
            sfPolicyUserDataClear (config, policy_id);
            DCE2_FreeConfig(pPolicyConfig);

            /* No more outstanding policies for this config */
            if (sfPolicyUserPolicyGetActive(config) == 0)
                DCE2_FreeConfigs(config);
        }
    }
#endif
    dce2_stats.sessions_active--;
}

