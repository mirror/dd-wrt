/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2002-2013 Sourcefire, Inc.
** Copyright (C) 1998-2002 Martin Roesch <roesch@sourcefire.com>
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

/* $Id$ */
/* spp_rpc_decode
 *
 * Purpose:
 *
 * This preprocessor normalizes the RPC requests from remote machines by
 * converting all fragments into one continous stream.
 * This is very useful for doing things like defeating hostile attackers
 * trying to stealth themselves from IDSs by fragmenting the request so the
 * string 0186A0 is broken up.
 *
 * Arguments:
 *
 * This plugin takes a list of integers representing the TCP ports that the
 * user is interested in having normalized
 *
 * Effect:
 *
 * Changes the data in the packet payload and changes
 * p->dsize to reflect the new (smaller) payload size.
 *
 * Comments:
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <assert.h>
#include <sys/types.h>
#include <stdlib.h>
#include <ctype.h>

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#include "decode.h"
#include "plugbase.h"
#include "parser.h"
#include "log.h"
#include "snort_debug.h"
#include "util.h"

#include "mstring.h"
#include "snort.h"
#include "detect.h"
#include "log.h"
#include "generators.h"
#include "event_queue.h"

#include "profiler.h"
#include "snort_bounds.h"
#include "strlcatu.h"
#include "detection_util.h"

#include "session_api.h"
#include "stream_api.h"

#ifdef TARGET_BASED
#include "sftarget_protocol_reference.h"
#endif
#include "sfPolicy.h"

#define OPT_ALERT_FRAGMENTS "alert_fragments"
#define OPT_ALERT_MULTIPLE_REQUESTS "no_alert_multiple_requests"
#define OPT_ALERT_LARGE_FRAGMENTS "no_alert_large_fragments"
#define OPT_ALERT_INCOMPLETE "no_alert_incomplete"

#define TEXT_ALERT_MULTIPLE_REQUESTS "alert_multiple_requests"
#define TEXT_ALERT_LARGE_FRAGMENTS "alert_large_fragments"
#define TEXT_ALERT_INCOMPLETE "alert_incomplete"
#define RPC_DECODE_DEFAULT_PORTS "111 32271"

#define RPC_CLASS DECODE_CLASS /* use the same classification as the other decoder alerts */

#define RPC_MAX_BUF_SIZE   256
#define RPC_FRAG_HDR_SIZE  sizeof(uint32_t)
#define RPC_FRAG_LEN(ptr)  (ntohl(*((uint32_t *)ptr)) & 0x7FFFFFFF)

typedef struct _RpcDecodeConfig
{
    char alert_fragments;    /* Alert when we see ANY fragmented RPC requests */
    char alert_incomplete;   /* Alert when we don't see all of a request in one packet */
    char alert_multi;        /* Alert when we see multiple requests in one packet */
    char alert_large;        /* Alert when we see multiple requests in one packet */
    uint8_t RpcDecodePorts[MAXPORTS/8];

} RpcDecodeConfig;

typedef struct _RpcBuffer
{
    uint8_t *data;
    uint32_t len;
    uint32_t size;

} RpcBuffer;

typedef struct _RpcSsnData
{
    int active;
    int events;
    uint32_t frag_len;
    uint32_t ignore;
    uint32_t nseq;
    uint32_t ofp;
    RpcBuffer seg;
    RpcBuffer frag;

} RpcSsnData;

typedef enum _RpcStatus
{
    RPC_STATUS__SUCCESS,
    RPC_STATUS__ERROR,
    RPC_STATUS__DEFRAG

} RpcStatus;

static tSfPolicyUserContextId rpc_decode_config = NULL;
static const uint32_t flush_size = 28;
static const uint32_t rpc_memcap = 1048510;
static uint32_t rpc_memory = 0;

#ifdef TARGET_BASED
static int16_t rpc_decode_app_protocol_id;
#endif

#ifdef PERF_PROFILING
PreprocStats rpcdecodePerfStats;
#endif

static void RpcDecodeInit(struct _SnortConfig *, char *);
static void PreprocRpcDecode(Packet *, void *);
static void ParseRpcConfig(RpcDecodeConfig *, char *);
static int ConvertRPC(RpcDecodeConfig *, RpcSsnData *, Packet *);
static void RpcDecodeFreeConfig(tSfPolicyUserContextId rpc);
static void RpcDecodeCleanExit(int, void *);
static void enablePortStreamServices(struct _SnortConfig *, RpcDecodeConfig *, tSfPolicyId);
#ifdef TARGET_BASED
static void _addServicesToStreamFilter(struct _SnortConfig *, tSfPolicyId);
#endif
static void RpcDecodePortsAssign(uint8_t *, char *);
static int RpcDecodeIsEligible(RpcDecodeConfig *, Packet *);

#ifdef SNORT_RELOAD
static void RpcDecodeReload(struct _SnortConfig *, char *, void **);
static void * RpcDecodeReloadSwap(struct _SnortConfig *, void *);
static void RpcDecodeReloadSwapFree(void *);
#endif

static RpcSsnData * RpcSsnDataNew(Packet *);
static void RpcSsnDataFree(void *);
static inline void RpcSsnClean(RpcSsnData *);
static inline void RpcSsnSetInactive(RpcSsnData *, Packet *);
static inline int RpcSsnIsActive(RpcSsnData *);

static RpcStatus RpcStatefulInspection(RpcDecodeConfig *, RpcSsnData *, Packet *);
static inline void RpcPreprocEvent(RpcDecodeConfig *, RpcSsnData *, int);
static RpcStatus RpcHandleFrag(RpcDecodeConfig *, RpcSsnData *, const uint8_t *);
static RpcStatus RpcPrepRaw(const uint8_t *, uint32_t, Packet *);
static RpcStatus RpcPrepFrag(RpcSsnData *, Packet *);
static RpcStatus RpcPrepSeg(RpcSsnData *, Packet *);
static inline uint32_t RpcBufLen(RpcBuffer *);
static inline uint8_t * RpcBufData(RpcBuffer *);
static RpcStatus RpcBufAdd(RpcBuffer *, const uint8_t *, uint32_t);
static inline void RpcBufClean(RpcBuffer *);

static inline void * RpcAlloc(uint32_t);
static inline void RpcFree(void *, uint32_t);


//function to assign the RpcDecodePorts array
static void RpcDecodePortsAssign(uint8_t *RpcDecodePorts, char *portlist)
{
    int num;
    int num_toks;
    char **toks;
    if( portlist == NULL || *portlist == '\0')
    {
        portlist = RPC_DECODE_DEFAULT_PORTS;
    }
    toks = mSplit(portlist, " \t", 0, &num_toks, 0);

    for(num = 0; num < num_toks; num++)
    {
        if(isdigit((int)toks[num][0]))
        {
            char *num_p = NULL; /* used to determine last position in string */
            long t_num;

            t_num = strtol(toks[num], &num_p, 10);

            if(*num_p != '\0')
            {
                ParseError("Port Number invalid format: %s.", toks[num]);
            }
            else if(t_num < 0 || t_num > MAXPORTS-1 )
            {
                ParseError("Port Number out of range: %d.", t_num);
            }

            RpcDecodePorts[(t_num/8)] |= 1<<(t_num%8);
        }
    }
    mSplitFree(&toks, num_toks);

}

/*
 * Function: SetupRpcDecode()
 *
 * Purpose: Registers the preprocessor keyword and initialization
 *          function into the preprocessor list.
 *
 * Arguments: None.
 *
 * Returns: void function
 *
 */
void SetupRpcDecode(void)
{
    /* link the preprocessor keyword to the init function in
       the preproc list */
#ifndef SNORT_RELOAD
    RegisterPreprocessor("rpc_decode", RpcDecodeInit);
#else
    RegisterPreprocessor("rpc_decode", RpcDecodeInit, RpcDecodeReload, NULL,
                         RpcDecodeReloadSwap, RpcDecodeReloadSwapFree);
#endif

    DEBUG_WRAP(DebugMessage(DEBUG_RPC,"Preprocessor: RpcDecode in setup...\n"););
}


/*
 * Function: RpcDecodeInit(char *)
 *
 * Purpose: Processes the args sent to the preprocessor, sets up the
 *          port list, links the processing function into the preproc
 *          function list
 *
 * Arguments: args => ptr to argument string
 *
 * Returns: void function
 *
 */
void RpcDecodeInit(struct _SnortConfig *sc, char *args)
{
    tSfPolicyId policy_id = getParserPolicy(sc);
    RpcDecodeConfig *pPolicyConfig = NULL;

    if (rpc_decode_config == NULL)
    {
        rpc_decode_config = sfPolicyConfigCreate();

        AddFuncToPreprocCleanExitList(RpcDecodeCleanExit, NULL, PRIORITY_LAST, PP_RPCDECODE);

#ifdef PERF_PROFILING
        RegisterPreprocessorProfile("rpcdecode", &rpcdecodePerfStats, 0, &totalPerfStats, NULL);
#endif

#ifdef TARGET_BASED
        /* Find and cache protocol ID for packet comparison */
        rpc_decode_app_protocol_id = FindProtocolReference("sunrpc");
        if (rpc_decode_app_protocol_id == SFTARGET_UNKNOWN_PROTOCOL)
            rpc_decode_app_protocol_id = AddProtocolReference("sunrpc");

        // register with session to handle applications
        session_api->register_service_handler( PP_RPCDECODE, rpc_decode_app_protocol_id );
#endif
    }

    sfPolicyUserPolicySet (rpc_decode_config, policy_id);
    pPolicyConfig = (RpcDecodeConfig *) sfPolicyUserDataGetCurrent(rpc_decode_config);
    if (pPolicyConfig)
    {
        ParseError("RPC decode can only be configured once.\n");
    }

    pPolicyConfig = (RpcDecodeConfig *)SnortAlloc(sizeof(RpcDecodeConfig));
    if (!pPolicyConfig)
    {
        ParseError("RPC_DECODE preprocessor: memory allocate failed.\n");
    }

    sfPolicyUserDataSetCurrent(rpc_decode_config, pPolicyConfig);

    /* parse the argument list into a list of ports to normalize */
    ParseRpcConfig(pPolicyConfig, args);

    /* Set the preprocessor function into the function list */
    AddFuncToPreprocList(sc, PreprocRpcDecode, PRIORITY_APPLICATION, PP_RPCDECODE, PROTO_BIT__TCP);
    enablePortStreamServices(sc, pPolicyConfig, policy_id);

#ifdef TARGET_BASED
    _addServicesToStreamFilter(sc, policy_id);
#endif

    DEBUG_WRAP(DebugMessage(DEBUG_RPC,"Preprocessor: RpcDecode Initialized\n"););
}

/*
 * Function: ParseRpcConfig(char *)
 *
 * Purpose: Reads the list of port numbers from the argument string and
 *          parses them into the port list data struct
 *
 * Arguments: portlist => argument list
 *
 * Returns: void function
 *
 */
void ParseRpcConfig(RpcDecodeConfig *rpc, char *portlist)
{
    char portstr[STD_BUF];
    char **toks;
    int is_reset = 0;
    int num_toks;
    int num;

    if (rpc == NULL)
        return;

    if(portlist == NULL || *portlist == '\0')
    {
        portlist = RPC_DECODE_DEFAULT_PORTS;
    }

    rpc->alert_multi = 1;
    rpc->alert_incomplete = 1;
    rpc->alert_large = 1;

    /* tokenize the argument list */
    toks = mSplit(portlist, " \t", 0, &num_toks, 0);

    LogMessage("rpc_decode arguments:\n");

    /* convert the tokens and place them into the port list */
    for(num = 0; num < num_toks; num++)
    {
        if(isdigit((int)toks[num][0]))
        {
            char *num_p = NULL; /* used to determine last position in string */
            long t_num;

            t_num = strtol(toks[num], &num_p, 10);

            if(*num_p != '\0')
            {
                ParseError("Port Number invalid format: %s.", toks[num]);
            }
            else if(t_num < 0 || t_num > MAXPORTS-1)
            {
                ParseError("Port Number out of range: %d\n", t_num);
            }

            /* user specified a legal port number and it should override the default
               port list, so reset it unless already done */
            if(!is_reset)
            {
                memset(rpc->RpcDecodePorts, 0, sizeof(rpc->RpcDecodePorts));
                portstr[0] = '\0';
                is_reset = 1;
            }

            /* mark this port as being interesting using some portscan2-type voodoo,
               and also add it to the port list string while we're at it so we can
               later print out all the ports with a single LogMessage() */
            rpc->RpcDecodePorts[(t_num/8)] |= 1<<(t_num%8);
            strlcat(portstr, toks[num], STD_BUF - 1);
            strlcat(portstr, " ", STD_BUF - 1);
        }
        else if(!strcasecmp(OPT_ALERT_MULTIPLE_REQUESTS,toks[num]))
        {
            rpc->alert_multi = 0;
        }
        else if(!strcasecmp(OPT_ALERT_INCOMPLETE,toks[num]))
        {
            rpc->alert_incomplete = 0;
        }
        else if(!strcasecmp(OPT_ALERT_LARGE_FRAGMENTS,toks[num]))
        {
            rpc->alert_large = 0;
        }
        else if(!strcasecmp(OPT_ALERT_FRAGMENTS,toks[num]))
        {
            rpc->alert_fragments = 1;
        }
        else
        {
            ParseError("Unknown argument to rpc_decode "
                       "preprocessor: \"%s\".", toks[num]);
        }
    }

    mSplitFree(&toks, num_toks);

    if( !is_reset )
    {
        RpcDecodePortsAssign(rpc->RpcDecodePorts, RPC_DECODE_DEFAULT_PORTS);
    }

    /* print out final port list */
    LogMessage("    Ports to decode RPC on: %s\n", is_reset ? portstr : RPC_DECODE_DEFAULT_PORTS);
    LogMessage("    %s: %s\n", OPT_ALERT_FRAGMENTS, rpc->alert_fragments ? "ACTIVE": "INACTIVE");
    LogMessage("    %s: %s\n", TEXT_ALERT_LARGE_FRAGMENTS, rpc->alert_large ? "ACTIVE": "INACTIVE");
    LogMessage("    %s: %s\n", TEXT_ALERT_INCOMPLETE, rpc->alert_incomplete ? "ACTIVE": "INACTIVE");
    LogMessage("    %s: %s\n", TEXT_ALERT_MULTIPLE_REQUESTS, rpc->alert_multi ? "ACTIVE": "INACTIVE");
}


/*
 * Function: PreprocRpcDecode(Packet *)
 *
 * Purpose: Inspects the packet's payload for fragment records and
 *          converts them into one infragmented record.
 *
 * Arguments: p => pointer to the current packet data struct
 *
 * Returns: void function
 *
 */
static void PreprocRpcDecode(Packet *p, void *context)
{
    RpcDecodeConfig *rconfig = NULL;
    RpcSsnData *rsdata = NULL;
    PROFILE_VARS;

    sfPolicyUserPolicySet(rpc_decode_config, getNapRuntimePolicy());
    rconfig = (RpcDecodeConfig *)sfPolicyUserDataGetCurrent(rpc_decode_config);

    /* Not configured in this policy */
    if (rconfig == NULL)
        return;

    // preconditions - what we registered for
    assert(IsTCP(p) && p->dsize);

    /* If we're stateful that means stream5 has been configured.
     * In this case we don't look at server packets.
     * There is the case were stream5 configuration requires a 3 way handshake.
     * If no 3 way, then the packet flags won't be set, so don't look at it
     * since we won't be able to determeine who's the client and who's the server. */
    if (ScStateful()
        && ((p->packet_flags & PKT_FROM_SERVER)
            || (!(p->packet_flags & PKT_FROM_CLIENT))))
    {
        return;
    }

    if ( ( session_api != NULL ) && ( p->ssnptr != NULL ) )
        rsdata = session_api->get_application_data(p->ssnptr, PP_RPCDECODE);

    if (rsdata == NULL)
    {
        if (!RpcDecodeIsEligible(rconfig, p))
            return;
    }

    PREPROC_PROFILE_START(rpcdecodePerfStats);

    if ((rsdata == NULL) && (session_api != NULL) && (p->ssnptr != NULL))
    {
        if (!(session_api->get_session_flags(p->ssnptr) & SSNFLAG_MIDSTREAM))
            rsdata = RpcSsnDataNew(p);
    }

    if (RpcSsnIsActive(rsdata)
            && ((p->packet_flags & PKT_REBUILT_STREAM)
                || (rsdata->nseq == 0)))
    {
        RpcStatus ret = RpcStatefulInspection(rconfig, rsdata, p);

        if (ret == RPC_STATUS__SUCCESS)
        {
            PREPROC_PROFILE_END(rpcdecodePerfStats);
            return;
        }

        /* Something went wrong - deactivate session tracking
         * and decode normally */
        if (ret == RPC_STATUS__ERROR)
            RpcSsnSetInactive(rsdata, p);
    }

    DEBUG_WRAP(DebugMessage(DEBUG_RPC,"Stateless inspection\n"););

    RpcPreprocEvent(rconfig, rsdata, ConvertRPC(rconfig, rsdata, p));

    PREPROC_PROFILE_END(rpcdecodePerfStats);
    return;
}

static inline void RpcPreprocEvent(RpcDecodeConfig *rconfig, RpcSsnData *rsdata, int event)
{
    if (rconfig == NULL)
        return;

    if (rsdata != NULL)
    {
        /* Only log one event of the same type per session */
        if (rsdata->events & (1 << event))
            return;

        rsdata->events |= (1 << event);
    }

    switch (event)
    {
        case RPC_FRAG_TRAFFIC:
            if (rconfig->alert_fragments)
            {
                SnortEventqAdd(GENERATOR_SPP_RPC_DECODE, RPC_FRAG_TRAFFIC,
                        1, RPC_CLASS, 3, RPC_FRAG_TRAFFIC_STR, 0);
            }
            break;
        case RPC_MULTIPLE_RECORD:
            if (rconfig->alert_multi)
            {
                SnortEventqAdd(GENERATOR_SPP_RPC_DECODE, RPC_MULTIPLE_RECORD,
                        1, RPC_CLASS, 3, RPC_MULTIPLE_RECORD_STR, 0);
            }
            break;
        case RPC_LARGE_FRAGSIZE:
            if (rconfig->alert_large)
            {
                SnortEventqAdd(GENERATOR_SPP_RPC_DECODE, RPC_LARGE_FRAGSIZE,
                        1, RPC_CLASS, 3, RPC_LARGE_FRAGSIZE_STR, 0);
            }
            break;
        case RPC_INCOMPLETE_SEGMENT:
            if (rconfig->alert_incomplete)
            {
                SnortEventqAdd(GENERATOR_SPP_RPC_DECODE, RPC_INCOMPLETE_SEGMENT,
                        1, RPC_CLASS, 3, RPC_INCOMPLETE_SEGMENT_STR, 0);
            }
            break;
        case RPC_ZERO_LENGTH_FRAGMENT:
            if (rconfig->alert_multi)
            {
                SnortEventqAdd(GENERATOR_SPP_RPC_DECODE, RPC_ZERO_LENGTH_FRAGMENT,
                        1, RPC_CLASS, 3, RPC_ZERO_LENGTH_FRAGMENT_STR, 0);
            }
            break;
        default:
            break;
    }
}

static int RpcDecodeIsEligible(RpcDecodeConfig *rconfig, Packet *p)
{
    int valid_app_id = 0;
#ifdef TARGET_BASED
    int16_t app_id = SFTARGET_UNKNOWN_PROTOCOL;

    /* check stream info, fall back to checking ports */
    if (session_api != NULL)
    {
        app_id = session_api->get_application_protocol_id(p->ssnptr);
        if (app_id > 0)
        {
            valid_app_id = 1;
        }
    }

    if (valid_app_id && app_id != rpc_decode_app_protocol_id)
        return 0;
#endif

    if (!valid_app_id)
    {
        uint16_t check_port;

        if (p->packet_flags & PKT_FROM_CLIENT)
            check_port = p->dp;
        else if (p->packet_flags & PKT_FROM_SERVER)
            check_port = p->sp;
        /* The below are for the case where stream5 is not configured */
        else if (p->sp < p->dp)
            check_port = p->sp;
        else
            check_port = p->dp;

        if (!(rconfig->RpcDecodePorts[(check_port/8)] & (1<<(check_port%8))))
            return 0;
    }

    return 1;
}

static RpcStatus RpcStatefulInspection(RpcDecodeConfig *rconfig,
        RpcSsnData *rsdata, Packet *p)
{
    const uint8_t *data = p->data;
    uint16_t dsize = p->dsize;
    uint32_t seq = ntohl(p->tcph->th_seq);
    int need;
    RpcStatus status;

    DEBUG_WRAP(DebugMessage(DEBUG_RPC,
                "STATEFUL: Start *******************************\n"););
    DEBUG_WRAP(DebugMessage(DEBUG_RPC,
                "STATEFUL: Ssn: %p\n", rsdata););

    if ((rsdata->nseq != seq) && (rsdata->nseq != 0))
    {
        uint32_t overlap;

        if (rsdata->nseq < seq)
        {
            /* Missed packets - stop tracking */
            DEBUG_WRAP(DebugMessage(DEBUG_RPC,
                        "STATEFUL: Missed data\n"););
            return RPC_STATUS__ERROR;
        }

        overlap = rsdata->nseq - seq;
        if (dsize <= overlap)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_RPC,
                        "STATEFUL: All data overlapped\n"););
            return RPC_STATUS__SUCCESS;
        }

        data += overlap;
        dsize -= (uint16_t)overlap;

        seq += overlap;
    }

    rsdata->nseq = seq + dsize;

    if (rsdata->ignore)
    {
        if (dsize < rsdata->ignore)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_RPC,
                        "STATEFUL: Ignoring %u bytes\n", dsize););

            rsdata->ignore -= dsize;

            DEBUG_WRAP(DebugMessage(DEBUG_RPC,
                        "STATEFUL: Bytes left to ignore: %u \n", rsdata->ignore););

            return RPC_STATUS__SUCCESS;
        }

        DEBUG_WRAP(DebugMessage(DEBUG_RPC,
                    "STATEFUL: Ignoring %u bytes\n", rsdata->ignore););

        dsize -= (uint16_t)rsdata->ignore;
        data += rsdata->ignore;
        rsdata->ignore = 0;
    }

    /* Might need to evaluate same packet, different decode buffer
     * more than once and detection option tree won't let us do that
     * by default */
    p->packet_flags |= PKT_ALLOW_MULTIPLE_DETECT;

    while (dsize > 0)
    {
        if (RpcBufLen(&rsdata->seg) == 0)
        {
            if (dsize < RPC_FRAG_HDR_SIZE)
            {
                DEBUG_WRAP(DebugMessage(DEBUG_RPC,
                            "STATEFUL: Not enough data for frag header: %u\n",
                            dsize););

                RpcPreprocEvent(rconfig, rsdata, RPC_INCOMPLETE_SEGMENT);

                if (RpcBufAdd(&rsdata->seg, data, dsize) != RPC_STATUS__SUCCESS)
                    return RPC_STATUS__ERROR;

                break;
            }

            rsdata->frag_len = RPC_FRAG_LEN(data);

            DEBUG_WRAP(DebugMessage(DEBUG_RPC,
                        "STATEFUL: Fragment length: %u\n", rsdata->frag_len););

            if (dsize < (RPC_FRAG_HDR_SIZE + rsdata->frag_len))
            {
                DEBUG_WRAP(DebugMessage(DEBUG_RPC,
                            "STATEFUL: Not enough data for fragment: %u\n",
                            dsize););

                RpcPreprocEvent(rconfig, rsdata, RPC_INCOMPLETE_SEGMENT);

                if (RpcBufAdd(&rsdata->seg, data, dsize) != RPC_STATUS__SUCCESS)
                    return RPC_STATUS__ERROR;

                break;
            }

            dsize -= (RPC_FRAG_HDR_SIZE + rsdata->frag_len);

            status = RpcHandleFrag(rconfig, rsdata, data);

            if (status == RPC_STATUS__ERROR)
                return RPC_STATUS__ERROR;

            if (status == RPC_STATUS__DEFRAG)
            {
                DEBUG_WRAP(DebugMessage(DEBUG_RPC,
                            "STATEFUL: Last frag - calling detect\n"););

                if ((dsize != 0) || (data != p->data))
                {
                    /* Only do this if there is more than one fragment in
                     * the data we got */
                    if (RpcPrepRaw(data, rsdata->frag_len, p) != RPC_STATUS__SUCCESS)
                        return RPC_STATUS__ERROR;

                    Detect(p);
                }

                if ((dsize > 0) && rconfig->alert_multi)
                    RpcPreprocEvent(rconfig, rsdata, RPC_MULTIPLE_RECORD);
            }

            data += (RPC_FRAG_HDR_SIZE + rsdata->frag_len);
        }
        else
        {
            if (RpcBufLen(&rsdata->seg) < RPC_FRAG_HDR_SIZE)
            {
                need = RPC_FRAG_HDR_SIZE - RpcBufLen(&rsdata->seg);
                if (dsize < need)
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_RPC,
                                "STATEFUL: Not enough data for frag header "
                                "(%u): %u\n", need, dsize););

                    RpcPreprocEvent(rconfig, rsdata, RPC_INCOMPLETE_SEGMENT);

                    if (RpcBufAdd(&rsdata->seg, data, dsize) != RPC_STATUS__SUCCESS)
                        return RPC_STATUS__ERROR;

                    break;
                }

                if (RpcBufAdd(&rsdata->seg, data, need) != RPC_STATUS__SUCCESS)
                    return RPC_STATUS__ERROR;

                data += need;
                dsize -= need;

                rsdata->frag_len = RPC_FRAG_LEN(RpcBufData(&rsdata->seg));

                DEBUG_WRAP(DebugMessage(DEBUG_RPC,
                            "STATEFUL: Fragment length: %u\n", rsdata->frag_len););
            }

            need = rsdata->frag_len - (RpcBufLen(&rsdata->seg) - RPC_FRAG_HDR_SIZE);
            if (dsize < need)
            {
                DEBUG_WRAP(DebugMessage(DEBUG_RPC,
                            "STATEFUL: Not enough data for fragment (%u): %u\n",
                            need, dsize););

                RpcPreprocEvent(rconfig, rsdata, RPC_INCOMPLETE_SEGMENT);

                if (RpcBufAdd(&rsdata->seg, data, dsize) != RPC_STATUS__SUCCESS)
                    return RPC_STATUS__ERROR;

                break;
            }

            if (RpcBufAdd(&rsdata->seg, data, need) != RPC_STATUS__SUCCESS)
                return RPC_STATUS__ERROR;

            data += need;
            dsize -= need;

            status = RpcHandleFrag(rconfig, rsdata, RpcBufData(&rsdata->seg));

            if (status == RPC_STATUS__ERROR)
                return RPC_STATUS__ERROR;

            if (status == RPC_STATUS__DEFRAG)
            {
                if (RpcBufLen(&rsdata->frag) != 0)
                {
                    if (RpcPrepFrag(rsdata, p) != RPC_STATUS__SUCCESS)
                        return RPC_STATUS__ERROR;
                }
                else
                {
                    if (RpcPrepSeg(rsdata, p) != RPC_STATUS__SUCCESS)
                        return RPC_STATUS__ERROR;
                }

                DEBUG_WRAP(DebugMessage(DEBUG_RPC,
                            "STATEFUL: Last frag - calling detect\n"););

                if ((dsize > 0) && rconfig->alert_multi)
                    RpcPreprocEvent(rconfig, rsdata, RPC_MULTIPLE_RECORD);

                Detect(p);
                RpcBufClean(&rsdata->frag);
            }

            RpcBufClean(&rsdata->seg);
        }
    }

    if (RpcBufLen(&rsdata->frag) != 0)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_RPC,
                    "STATEFUL: Prepping Frag data: %u\n",
                    RpcBufLen(&rsdata->frag)););

        if (RpcPrepFrag(rsdata, p) != RPC_STATUS__SUCCESS)
            return RPC_STATUS__ERROR;
    }
    else if (RpcBufLen(&rsdata->seg) != 0)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_RPC,
                    "STATEFUL: Prepping Seg data: %u\n",
                    RpcBufLen(&rsdata->seg)););

        if (RpcPrepSeg(rsdata, p) != RPC_STATUS__SUCCESS)
            return RPC_STATUS__ERROR;
    }

    DEBUG_WRAP(DebugMessage(DEBUG_RPC,
                "STATEFUL: Success *****************************\n"););

    return RPC_STATUS__SUCCESS;
}

static RpcStatus RpcPrepRaw(const uint8_t *data, uint32_t fraglen, Packet *p)
{
    int status;

    status = SafeMemcpy(DecodeBuffer.data, data, RPC_FRAG_HDR_SIZE + fraglen,
            DecodeBuffer.data, DecodeBuffer.data + sizeof(DecodeBuffer.data));

    if (status != SAFEMEM_SUCCESS)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_RPC,
                    "STATEFUL: Failed to copy raw data to alt buffer\n"););
        return RPC_STATUS__ERROR;
    }

    SetAltDecode((uint16_t)(RPC_FRAG_HDR_SIZE + fraglen));

    return RPC_STATUS__SUCCESS;
}

static RpcStatus RpcPrepFrag(RpcSsnData *rsdata, Packet *p)
{
    int status;
    uint32_t fraghdr = htonl(RpcBufLen(&rsdata->frag));

    DecodeBuffer.data[0] = *((uint8_t *) &fraghdr);
    DecodeBuffer.data[1] = *(((uint8_t *) &fraghdr) + 1);
    DecodeBuffer.data[2] = *(((uint8_t *) &fraghdr) + 2);
    DecodeBuffer.data[3] = *(((uint8_t *) &fraghdr) + 3);

    DecodeBuffer.data[0] |= 0x80;

    status = SafeMemcpy(DecodeBuffer.data+4, RpcBufData(&rsdata->frag),
            RpcBufLen(&rsdata->frag), DecodeBuffer.data+4,
            DecodeBuffer.data + sizeof(DecodeBuffer.data));

    if (status != SAFEMEM_SUCCESS)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_RPC,
                    "STATEFUL: Failed to copy frag data to alt buffer\n"););
        RpcBufClean(&rsdata->frag);
        return RPC_STATUS__ERROR;
    }

    SetAltDecode((uint16_t)RpcBufLen(&rsdata->frag));

    if (RpcBufLen(&rsdata->frag) > RPC_MAX_BUF_SIZE)
        RpcBufClean(&rsdata->frag);

    return RPC_STATUS__SUCCESS;
}

static RpcStatus RpcPrepSeg(RpcSsnData *rsdata, Packet *p)
{
    int status;

    status = SafeMemcpy(DecodeBuffer.data, RpcBufData(&rsdata->seg),
            RpcBufLen(&rsdata->seg), DecodeBuffer.data,
            DecodeBuffer.data + sizeof(DecodeBuffer.data));

    if (status != SAFEMEM_SUCCESS)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_RPC,
                    "STATEFUL: Failed to copy seg data to alt buffer\n"););
        RpcBufClean(&rsdata->seg);
        return RPC_STATUS__ERROR;
    }

    SetAltDecode((uint16_t)RpcBufLen(&rsdata->seg));

    if (RpcBufLen(&rsdata->seg) > RPC_MAX_BUF_SIZE)
    {
        rsdata->ignore = (sizeof(uint32_t) + rsdata->frag_len) - RpcBufLen(&rsdata->seg);
        DEBUG_WRAP(DebugMessage(DEBUG_RPC,
                    "STATEFUL: Ignoring %u bytes\n", rsdata->ignore););
        RpcBufClean(&rsdata->seg);
    }

    return RPC_STATUS__SUCCESS;
}

static RpcStatus RpcHandleFrag(RpcDecodeConfig *rconfig,
        RpcSsnData *rsdata, const uint8_t *fragment)
{
    int last_frag = fragment[0] & 0x80;
    uint32_t frag_len = RPC_FRAG_LEN(fragment);

    if (frag_len == 0)
        RpcPreprocEvent(rconfig, rsdata, RPC_ZERO_LENGTH_FRAGMENT);

    if (!last_frag)
        RpcPreprocEvent(rconfig, rsdata, RPC_FRAG_TRAFFIC);

    if ((RpcBufLen(&rsdata->frag) == 0) && last_frag)
        return RPC_STATUS__DEFRAG;

    DEBUG_WRAP(DebugMessage(DEBUG_RPC,
                "STATEFUL: Adding %u bytes to frag buffer\n", frag_len););

    if (RpcBufAdd(&rsdata->frag,
                fragment + sizeof(uint32_t), frag_len) != RPC_STATUS__SUCCESS)
    {
        return RPC_STATUS__ERROR;
    }

    if (last_frag)
        return RPC_STATUS__DEFRAG;

    return RPC_STATUS__SUCCESS;
}

static inline uint32_t RpcBufLen(RpcBuffer *buf)
{
    return buf == NULL ? 0 : buf->len;
}

static inline uint8_t * RpcBufData(RpcBuffer *buf)
{
    return buf == NULL ? NULL : buf->data;
}

static RpcStatus RpcBufAdd(RpcBuffer *buf, const uint8_t *data, uint32_t dsize)
{
    const uint32_t min_alloc = flush_size;
    uint32_t alloc_size = dsize;
    int status;

    if (buf == NULL)
        return RPC_STATUS__ERROR;

    if (dsize == 0)
        return RPC_STATUS__SUCCESS;

    if (alloc_size < min_alloc)
        alloc_size = min_alloc;

    if (buf->data == NULL)
    {
        buf->data = (uint8_t *)RpcAlloc(alloc_size);
        if (buf->data == NULL)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_RPC,
                        "STATEFUL: Failed to allocate buffer data\n"););
            return RPC_STATUS__ERROR;
        }

        buf->size = alloc_size;
    }
    else if ((buf->len + dsize) > buf->size)
    {
        uint32_t new_size = buf->len + alloc_size;
        uint8_t *tmp = (uint8_t *)RpcAlloc(new_size);

        if (tmp == NULL)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_RPC,
                        "STATEFUL: Failed to reallocate buffer data\n"););
            RpcBufClean(buf);
            return RPC_STATUS__ERROR;
        }

        status = SafeMemcpy(tmp, buf->data, buf->len, tmp, tmp + new_size);
        RpcFree(buf->data, buf->size);
        buf->data = tmp;
        buf->size = new_size;

        if (status != SAFEMEM_SUCCESS)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_RPC,
                        "STATEFUL: Failed to move buffer data\n"););
            RpcBufClean(buf);
            return RPC_STATUS__ERROR;
        }
    }

    status = SafeMemcpy(buf->data + buf->len, data, dsize,
            buf->data + buf->len, buf->data + buf->size);
    if (status != SAFEMEM_SUCCESS)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_RPC,
                    "STATEFUL: Failed to copy data to buffer\n"););
        RpcBufClean(buf);
        return RPC_STATUS__ERROR;
    }

    buf->len += dsize;

    return RPC_STATUS__SUCCESS;
}

static inline void RpcBufClean(RpcBuffer *buf)
{
    if (buf->data != NULL)
    {
        RpcFree(buf->data, buf->size);
        buf->data = NULL;
    }

    buf->len = 0;
    buf->size = 0;
}

static inline void * RpcAlloc(uint32_t size)
{
    if ((rpc_memory + size) > rpc_memcap)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_RPC, "STATEFUL: Memcap exceeded\n"););
        return NULL;
    }

    rpc_memory += size;
    return SnortAlloc(size);
}

static inline void RpcFree(void *data, uint32_t size)
{
    if (data == NULL)
        return;

    if (rpc_memory < size)
        rpc_memory = 0;
    else
        rpc_memory -= size;

    free(data);
}

static inline void RpcSsnSetInactive(RpcSsnData *rsdata, Packet *p)
{
    if (rsdata == NULL)
        return;

    DEBUG_WRAP(DebugMessage(DEBUG_RPC, "STATEFUL: Deactivating session: %p\n",
                rsdata););

    stream_api->set_flush_point(p->ssnptr, SSN_DIR_FROM_SERVER, rsdata->ofp);
    RpcSsnClean(rsdata);
}

static inline int RpcSsnIsActive(RpcSsnData *rsdata)
{
    if (rsdata == NULL)
        return 0;
    return rsdata->active;
}

static inline void RpcSsnClean(RpcSsnData *rsdata)
{
    if (rsdata == NULL)
        return;

    rsdata->active = 0;
    rsdata->frag_len = 0;
    rsdata->ignore = 0;
    RpcBufClean(&rsdata->seg);
    RpcBufClean(&rsdata->frag);
}

static RpcSsnData * RpcSsnDataNew(Packet *p)
{
    RpcSsnData *rsdata = (RpcSsnData *)RpcAlloc(sizeof(RpcSsnData));
    if (rsdata != NULL)
    {
        char rdir = stream_api->get_reassembly_direction(p->ssnptr);

        if (!(rdir & SSN_DIR_TO_SERVER))
        {
            rdir |= SSN_DIR_TO_SERVER;
            stream_api->set_reassembly(p->ssnptr, STREAM_FLPOLICY_FOOTPRINT,
                    rdir, STREAM_FLPOLICY_SET_ABSOLUTE);
        }

        rsdata->active = 1;
        rsdata->ofp = stream_api->get_flush_point(p->ssnptr, SSN_DIR_TO_SERVER);

        stream_api->set_flush_point(p->ssnptr, SSN_DIR_TO_SERVER, flush_size);
        session_api->set_application_data(p->ssnptr,
                PP_RPCDECODE, (void *)rsdata, RpcSsnDataFree);

        DEBUG_WRAP(DebugMessage(DEBUG_RPC, "STATEFUL: Created new session: "
                    "%p\n", rsdata););
    }

    return rsdata;
}

static void RpcSsnDataFree(void *data)
{
    RpcSsnData *rsdata = (RpcSsnData *)data;

    if (data == NULL)
        return;

    DEBUG_WRAP(DebugMessage(DEBUG_RPC, "STATEFUL: Deleting session: %p\n",
                rsdata););

    RpcSsnClean(rsdata);
    RpcFree(rsdata, sizeof(RpcSsnData));
}

/* most significant bit */
#define MSB 0x80000000

/*
 * For proto ref, see rfc1831 section 10 and page 445 UNP vol2
 *
 * check to make sure we've got enough data to process a record
 *
 * Where did the original 16 come from?  It seems that it could be
 * a last frag of 0 length according to spec.
 *
 * The minimum "valid" packet for us is 8 fields * 4 bytes
 *
 * This decoder is ignorant of TCP state so we'll have to assume
 * that reassembled TCP stuff is reinjected to the preprocessor
 * chain
 *
 * This decoder is also ignorant of multiple RPC requests in a
 * single stream.  To compensate, we can configure alerts
 *
 * Additionally, we don't do anything to verify that this is
 * really an RPC service port so we don't decode anything that
 * happens as a result
 *
 * From rfc1831:
 *
 *  Fragment Header ( 1 flag bit, 31 bit uint )
 *     RPC Body
 *
 *        unsigned int xid
 *        struct call_body {
 *             unsigned int rpcvers;  // must be equal to two (2)
 *             unsigned int prog;
 *             unsigned int vers;
 *             unsigned int proc;
 *             opaque_auth  cred;
 *             opaque_auth  verf;
 *        }
 */

static int ConvertRPC(RpcDecodeConfig *rconfig, RpcSsnData *rsdata, Packet *p)
{
    const uint8_t *data = p->data;
    uint32_t psize = p->dsize;
    uint8_t *norm_index;
    uint8_t *data_index;     /* this is the index pointer to walk thru the data */
    uint8_t *data_end;       /* points to the end of the payload for loop control */
    uint32_t length;          /* length of current fragment */
    int last_fragment = 0; /* have we seen the last fragment sign? */
    uint32_t decoded_len; /* our decoded length is always atleast a 0 byte header */
    uint32_t fraghdr;   /* Used to store the RPC fragment header data */
    int fragcount = 0;   /* How many fragment counters have we seen? */
    int ret;
    uint8_t *decode_buf_start = DecodeBuffer.data;
    uint8_t *decode_buf_end = decode_buf_start + sizeof(DecodeBuffer.data);

    if (psize < 32)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_RPC, "Not enough data to decode: %u\n",
                    psize););
        return 0;
    }

    /* on match, normalize the data */
    DEBUG_WRAP(DebugMessage(DEBUG_RPC, "Got RPC traffic (%u bytes)!\n", psize););

    /* cheesy alignment safe fraghdr = *(uint32_t *) data*/
    *((uint8_t *)  &fraghdr)      = data[0];
    *(((uint8_t *) &fraghdr) + 1) = data[1];
    *(((uint8_t *) &fraghdr) + 2) = data[2];
    *(((uint8_t *) &fraghdr) + 3) = data[3];


    /* The fragment header is 4 bytes in network byte order */
    fraghdr = ntohl(fraghdr);
    length = fraghdr & 0x7FFFFFFF;

    /* Check to see if we are on the last fragment */
    if(fraghdr & MSB)
    {
        /* on match, normalize the data */
        DEBUG_WRAP(DebugMessage(DEBUG_RPC, "Found Last Fragment: %u!\n", length););

        if((length + 4 != psize) && !(p->packet_flags & PKT_REBUILT_STREAM))
        {
            DEBUG_WRAP(DebugMessage(DEBUG_RPC, "It's not the only thing in this buffer!"
                                    " length: %d psize: %d!\n", length, psize););
            return RPC_MULTIPLE_RECORD;
        }
        else if ( length == 0 )
        {
            DEBUG_WRAP(DebugMessage(DEBUG_RPC, "Zero-length RPC fragment detected."
                                    " length: %d psize: %d.\n", length, psize););
            return RPC_ZERO_LENGTH_FRAGMENT;
        }
        return 0;
    }
    else if (rconfig->alert_fragments)
    {
        RpcPreprocEvent(rconfig, rsdata, RPC_FRAG_TRAFFIC);
    }

    norm_index = DecodeBuffer.data;
    data_index = (uint8_t *)data;
    data_end = (uint8_t *)data + psize;

    /* now we know it's in fragmented records, 4 bytes of
     * header(of which the most sig bit fragment (0=yes 1=no).
     * The header is followed by the value move pointer up 4
     * bytes, we need to stuff header in first 4 bytes.
     * But the header has the total length...we don't know
     * until the end
     */

    /* This is where decoded data will be written */
    norm_index += 4;
    decoded_len = 4;

    /* always make sure that we have enough data to process atleast
     * the header and that we only process at most, one fragment
     */

    while(((data_end - data_index) >= 4) && (last_fragment == 0))
    {
        /* get the fragment length (31 bits) and move the pointer to
           the start of the actual data */

        *((uint8_t *) &fraghdr)       = data_index[0];
        *(((uint8_t *) &fraghdr) + 1) = data_index[1];
        *(((uint8_t *) &fraghdr) + 2) = data_index[2];
        *(((uint8_t *) &fraghdr) + 3) = data_index[3];

        fraghdr = ntohl(fraghdr);
        length = fraghdr & 0x7FFFFFFF;

        if (length == 0)
            break;

        /* move the current index into the packet past the
           fragment header */
        data_index += 4;

        if(fraghdr & MSB)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_RPC, "Last Fragment detected\n"););
            last_fragment = 1;
        }

        if((length + decoded_len) < decoded_len)
        {
            /* don't allow integer overflow to confuse us.  Should be
             * caught by length > psize but who knows when weird
             * psize's might be allowed */

            DEBUG_WRAP(DebugMessage(DEBUG_RPC, "Integer Overflow"
                                    " field(%d) exceeds packet size(%d)\n",
                                    length, psize););
            return RPC_LARGE_FRAGSIZE;
        }

        decoded_len += length;

        if(length > psize)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_RPC, "Length of"
                                    " field(%d) exceeds packet size(%d)\n",
                                    length, psize););
            return RPC_INCOMPLETE_SEGMENT;
        }
        else if(decoded_len > psize)
        {
            /* The entire request is larger than our current packet
             *  size
             */
            DEBUG_WRAP(DebugMessage(DEBUG_RPC, " Decoded Length (%d)"
                                    "exceeds packet size(%d)\n",
                                    decoded_len, psize););
            return RPC_LARGE_FRAGSIZE;
        }
        else if((data_index + length) > data_end)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_RPC,
                                    "returning LARGE_FRAGSIZE"
                                    "since we'd read past our end\n"););
            return RPC_LARGE_FRAGSIZE;
        }
        else
        {
            fragcount++;

            DEBUG_WRAP(DebugMessage(DEBUG_RPC,
                                    "length: %d size: %d decoded_len: %d\n",
                                    length, psize, decoded_len););

            ret = SafeMemcpy(norm_index, data_index, length, decode_buf_start, decode_buf_end);
            if (ret != SAFEMEM_SUCCESS)
            {
                return 0;
            }

            norm_index += length;
            data_index += length;
        }
    }

    /* rewrite the header on the request packet */
    /* move the fragment header back onto the data */

    fraghdr = ntohl(decoded_len); /* size */

    DecodeBuffer.data[0] = *((uint8_t *) &fraghdr);
    DecodeBuffer.data[1] = *(((uint8_t *) &fraghdr) + 1);
    DecodeBuffer.data[2] = *(((uint8_t *) &fraghdr) + 2);
    DecodeBuffer.data[3] = *(((uint8_t *) &fraghdr) + 3);

    DecodeBuffer.data[0] |=  0x80;             /* Mark as unfragmented */

    /* is there another request encoded that is trying to evade us by doing
     *
     * frag last frag [ more data ]?
     */
    if(decoded_len + ((fragcount - 1) * 4) != psize)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_RPC, "decoded len does not compute: %d\n",
                                decoded_len););
        return RPC_MULTIPLE_RECORD;
    }

    DEBUG_WRAP(DebugMessage(DEBUG_RPC, "New size: %d\n", decoded_len);
               DebugMessage(DEBUG_RPC, "converted data:\n");
               //PrintNetData(stdout, data, decoded_len);
               );

    SetAltDecode((uint16_t)decoded_len);

    return 0;
}

static void enablePortStreamServices(struct _SnortConfig *sc, RpcDecodeConfig *rpc, tSfPolicyId policy_id)
{
    uint32_t portNum;

    if (rpc == NULL)
        return;

    if (stream_api)
    {
        for (portNum = 0; portNum < MAXPORTS; portNum++)
        {
            if(rpc->RpcDecodePorts[(portNum/8)] & (1<<(portNum%8)))
            {
                //Add port the port
                stream_api->set_port_filter_status ( sc, IPPROTO_TCP, (uint16_t) portNum,
                                                     PORT_MONITOR_SESSION, policy_id, 1 );
                stream_api->register_reassembly_port( NULL, 
                                                      (uint16_t) portNum, 
                                                      SSN_DIR_FROM_SERVER | SSN_DIR_FROM_CLIENT );
                session_api->enable_preproc_for_port( sc, PP_RPCDECODE, PROTO_BIT__TCP, (uint16_t) portNum ); 
            }
        }
    }
}

#ifdef TARGET_BASED
static void _addServicesToStreamFilter(struct _SnortConfig *sc, tSfPolicyId policy_id)
{
    if (stream_api)
        stream_api->set_service_filter_status
            (sc, rpc_decode_app_protocol_id, PORT_MONITOR_SESSION, policy_id, 1);
}
#endif

static int RpcDecodeFreeConfigPolicy(tSfPolicyUserContextId rpc,tSfPolicyId policyId, void* pData )
{
    RpcDecodeConfig *pPolicyConfig = (RpcDecodeConfig *)pData;
    sfPolicyUserDataClear (rpc, policyId);
    free(pPolicyConfig);
    return 0;
}

static void RpcDecodeFreeConfig(tSfPolicyUserContextId rpc)
{

    if (rpc == NULL)
        return;
    sfPolicyUserDataFreeIterate (rpc, RpcDecodeFreeConfigPolicy);
    sfPolicyConfigDelete(rpc);
}

static void RpcDecodeCleanExit(int signal, void *unused)
{
    RpcDecodeFreeConfig(rpc_decode_config);
    rpc_decode_config = NULL;
}

#ifdef SNORT_RELOAD
static void RpcDecodeReload(struct _SnortConfig *sc, char *args, void **new_config)
{
    tSfPolicyUserContextId rpc_decode_swap_config = (tSfPolicyUserContextId)*new_config;
    int policy_id = (int)getParserPolicy(sc);
    RpcDecodeConfig *pPolicyConfig = NULL;

    if (!rpc_decode_swap_config)
    {
        rpc_decode_swap_config = sfPolicyConfigCreate();
        *new_config = rpc_decode_swap_config;
    }
    sfPolicyUserPolicySet (rpc_decode_swap_config, policy_id);
    pPolicyConfig = (RpcDecodeConfig *)sfPolicyUserDataGetCurrent(rpc_decode_swap_config);
    if (pPolicyConfig)
    {
       ParseError("RPC decode can only be configured once.\n");
    }

    pPolicyConfig = (RpcDecodeConfig *)SnortAlloc(sizeof(RpcDecodeConfig));
    if (!pPolicyConfig)
    {
        ParseError("RPC Decode preprocessor: memory allocate failed.\n");
    }
    sfPolicyUserDataSetCurrent(rpc_decode_swap_config, pPolicyConfig);

    /* parse the argument list into a list of ports to normalize */
    ParseRpcConfig(pPolicyConfig, args);

    /* Set the preprocessor function into the function list */
    AddFuncToPreprocList(sc, PreprocRpcDecode, PRIORITY_APPLICATION, PP_RPCDECODE, PROTO_BIT__TCP);

    enablePortStreamServices(sc, pPolicyConfig, policy_id);

#ifdef TARGET_BASED
    _addServicesToStreamFilter(sc, policy_id);
#endif
}

static void * RpcDecodeReloadSwap(struct _SnortConfig *sc, void *swap_config)
{
    tSfPolicyUserContextId rpc_decode_swap_config = (tSfPolicyUserContextId)swap_config;
    tSfPolicyUserContextId old_config = rpc_decode_config;

    if (rpc_decode_swap_config == NULL)
        return NULL;

    rpc_decode_config = rpc_decode_swap_config;
    return (void *)old_config;
}

static void RpcDecodeReloadSwapFree(void *data)
{
    if (data == NULL)
        return;

    RpcDecodeFreeConfig((tSfPolicyUserContextId)data);
}
#endif

