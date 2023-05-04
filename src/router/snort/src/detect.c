/* $Id$ */
/*
** Copyright (C) 1998-2002 Martin Roesch <roesch@sourcefire.com>
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2002-2013 Sourcefire, Inc.
**    Dan Roelker <droelker@sourcefire.com>
**    Marc Norton <mnorton@sourcefire.com>
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
**
** NOTES
**   5.7.02: Added interface for new detection engine. (Norton/Roelker)
**
*/

#define FASTPKT

/*  I N C L U D E S  **********************************************/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <stdlib.h>

#include "snort.h"
#include "detect.h"
#include "plugbase.h"
#include "snort_debug.h"
#include "util.h"
#include "mstring.h"
#include "tag.h"
#include "pcrm.h"
#include "fpcreate.h"
#include "fpdetect.h"
#include "sfthreshold.h"
#include "event_wrapper.h"
#include "event_queue.h"
#include "obfuscation.h"
#include "profiler.h"
#include "session_api.h"
#include "session_common.h"
#include "stream_api.h"
#include "snort_stream_udp.h"
#include "active.h"
#include "signature.h"
#include "ipv6_port.h"
#include "ppm.h"
#include "sf_types.h"
#include "active.h"
#include "detection_util.h"
#include "preprocids.h"
#if defined(FEAT_OPEN_APPID)
#include "sp_appid.h"
#include "appIdApi.h"
#endif /* defined(FEAT_OPEN_APPID) */

#ifdef PORTLISTS
#include "sfutil/sfportobject.h"
#endif
#ifdef PERF_PROFILING
PreprocStats detectPerfStats;
#endif

#ifdef TARGET_BASED
#include "target-based/sftarget_protocol_reference.h"
#endif
#include "sfPolicy.h"

/* #define ITERATIVE_ENGINE */


OptTreeNode *otn_tmp = NULL;       /* OptTreeNode temp ptr */

int do_detect;
int do_detect_content;
uint16_t event_id;
static char check_tags_flag;

static int CheckTagging(Packet *);

#ifdef PERF_PROFILING
PreprocStats eventqPerfStats;
#endif

static inline int preprocHandlesProto( Packet *p, PreprocEvalFuncNode *ppn )
{
    return ( ( p->proto_bits & ppn->proto_mask ) || ( ppn->proto_mask == PROTO_BIT__ALL ) );
}

static inline bool processDecoderAlertsActionQ( Packet *p )
{
    // with policy selected, process any decoder alerts and queued actions
    DecodePolicySpecific(p);
    // actions are queued only for IDS case
    sfActionQueueExecAll(decoderActionQ);
    return true;
}

static void DispatchPreprocessors( Packet *p, tSfPolicyId policy_id, SnortPolicy *policy )
{
    SessionControlBlock *scb = NULL;
    PreprocEvalFuncNode *ppn;
    PreprocEnableMask pps_enabled_foo;
    bool alerts_processed = false;

#if defined(DAQ_VERSION) && DAQ_VERSION > 9
    uint64_t start=0, end=0;
#endif
 
    // No expected sessions yet.
    p->expectedSession = NULL;

    // until we are in a Session context dispatch preprocs from the policy list if there is one
    p->cur_pp = policy->preproc_eval_funcs;
    if( p->cur_pp == NULL )
    {
        alerts_processed = processDecoderAlertsActionQ( p );
        LogMessage("WARNING: No preprocessors configured for policy %d.\n", policy_id);
        return;
    }

    pps_enabled_foo = policy->pp_enabled[ p->dp ] | policy->pp_enabled[ p->sp ];
    EnablePreprocessors( p, pps_enabled_foo );
    do {
        ppn = p->cur_pp;
        p->cur_pp = ppn->next;

        // if packet has no data and we are up to APP preprocs then get out
        if( p->dsize == 0 && ppn->priority >= PRIORITY_APPLICATION )
            break;

        if ( preprocHandlesProto( p, ppn ) && IsPreprocessorEnabled( p, ppn->preproc_bit ) )
        {
#if defined(DAQ_VERSION) && DAQ_VERSION > 9
            if (p->pkth && (p->pkth->flags & DAQ_PKT_FLAG_DEBUG_ON))
            {
                get_clockticks(start);
                ppn->func( p, ppn->context );
                get_clockticks(end);
                print_flow(p,NULL,ppn->preproc_id,start,end);
            }
            else
                ppn->func( p, ppn->context );
#else
            ppn->func( p, ppn->context );
#endif 
        }


        if( !alerts_processed && ( p->ips_os_selected || ppn->preproc_id == PP_FW_RULE_ENGINE ) )
            alerts_processed = processDecoderAlertsActionQ( p );

        if( scb == NULL && p->ssnptr != NULL )
            scb = ( SessionControlBlock * ) p->ssnptr;
        // if we now have session, update enabled pps if changed by previous preproc
        if( scb != NULL && pps_enabled_foo != scb->enabled_pps )
        {
            EnablePreprocessors( p, scb->enabled_pps );
            pps_enabled_foo = scb->enabled_pps;
        }
 
        if( ( ppn->preproc_id == PP_FW_RULE_ENGINE ) && 
            ( ( IPH_IS_VALID( p ) ) && ( GET_IPH_PROTO( p ) == IPPROTO_UDP ) ) && 
            ( session_api->protocol_tracking_enabled( SESSION_PROTO_UDP ) ) ) 
        {
            InspectPortFilterUdp( p );
        }

    } while ( ( p->cur_pp != NULL ) && !( p->packet_flags & PKT_PASS_RULE ) );

    // queued decoder alerts are processed after the selection of the
    // IPS rule config for the flow, if not yet done then process them now
    if( !alerts_processed )
        alerts_processed = processDecoderAlertsActionQ( p );

    if( p->dsize == 0 )
        DisableDetect( p );
}


int Preprocess(Packet * p)
{
    int retval = 0;
    int detect_retval = 0;
    tSfPolicyId policy_id;

   /* NAP runtime policy may have been updated during decode, 
    * but preprocess needs default nap policy for session
    * preprocessor selection, hence use default policy when
    * called without session pointer set.
    */
    if( !p->ssnptr )
        policy_id = getDefaultPolicy();
    else
        policy_id = getNapRuntimePolicy();

    SnortPolicy *policy = snort_conf->targeted_policies[policy_id];
#ifdef PPM_MGR
    uint64_t pktcnt=0;
#endif
    PROFILE_VARS;

    if (policy == NULL)
        return -1;

#ifdef PPM_MGR
    /* Begin Packet Performance Monitoring  */
    if( PPM_PKTS_ENABLED() )
    {
        pktcnt = PPM_INC_PKT_CNT();
        PPM_GET_TIME();
        PPM_INIT_PKT_TIMER();
#ifdef DEBUG
        if( PPM_DEBUG_PKTS() )
        {
           /* for debugging, info gathering, so don't worry about
           *  (unsigned) casting of pktcnt, were not likely to debug
           *  4G packets
           */
           LogMessage("PPM: Process-BeginPkt[%u] caplen=%u\n",
             (unsigned)pktcnt,p->pkth->caplen);
        }
#endif
    }
#endif

    // If the packet has errors or syn over rate, we won't analyze it.
    if ( p->error_flags )
    {
        // process any decoder alerts now that policy has been selected...
        DecodePolicySpecific(p);

        //actions are queued only for IDS case
        sfActionQueueExecAll(decoderActionQ);
        DEBUG_WRAP(DebugMessage(DEBUG_DETECT,
            "Packet errors = 0x%x, ignoring traffic!\n", p->error_flags););

        if ( p->error_flags & PKT_ERR_BAD_TTL )
            pc.bad_ttl++;
        else if( p->error_flags & PKT_ERR_SYN_RL_DROP )
            pc.syn_rate_limit_drops++;
        else
            pc.invalid_checksums++;
    }
    else
    {
        /* Not a completely ideal place for this since any entries added on the
         * PacketCallback -> ProcessPacket -> Preprocess trail will get
         * obliterated - right now there isn't anything adding entries there.
         * Really need it here for stream5 clean exit, since all of the
         * flushed, reassembled packets are going to be injected directly into
         * this function and there may be enough that the obfuscation entry
         * table will overflow if we don't reset it.  Putting it here does
         * have the advantage of fewer entries per logging cycle */
        obApi->resetObfuscationEntries();

        do_detect = do_detect_content = !snort_conf->disable_all_policies;

        /*
        **  Reset the appropriate application-layer protocol fields
        */
        ClearHttpBuffers();
        p->alt_dsize = 0;
        DetectReset(p->data, p->dsize);

        // ok, dispatch all preprocs enabled for this packet/session
        DispatchPreprocessors( p, policy_id, policy );

        if ( do_detect ) 
        {
            detect_retval = Detect(p);
        }
    }

    check_tags_flag = 1;

#ifdef DUMP_BUFFER
    dumped_state = false;
#endif

    PREPROC_PROFILE_START(eventqPerfStats);
    retval = SnortEventqLog(snort_conf->event_queue, p);

#ifdef DUMP_BUFFER

    /* dump_alert_only makes sure that bufferdump happens only when a rule is
    triggered.

    dumped_state avoids repetition of buffer dump for a packet that has an
    alert, when --buffer-dump is given as command line option.

    When --buffer-dump is given as command line option, BufferDump output
    plugin is called for each packet. bdfptr will be NULL for all other output
    plugins.
    */

    if (!dump_alert_only && !dumped_state)
    {
         OutputFuncNode *idx = LogList;

         while (idx != NULL)
         {
             if (idx->bdfptr != NULL)
                 idx->bdfptr(p, NULL , idx->arg, NULL);

             idx = idx->next;
         }
     }
#endif

    SnortEventqReset();
    PREPROC_PROFILE_END(eventqPerfStats);

    /* Check for normally closed session */
    if( session_api )
        session_api->check_session_closed(p);

    if( session_api && p->ssnptr &&
      ( session_api->get_session_flags(p->ssnptr) & SSNFLAG_FREE_APP_DATA) )
    {
        SessionControlBlock *scb = ( SessionControlBlock * ) p->ssnptr;
        session_api->free_application_data(scb);
        scb->ha_state.session_flags &= ~SSNFLAG_FREE_APP_DATA;
    }

    /*
    ** By checking tagging here, we make sure that we log the
    ** tagged packet whether it generates an alert or not.
    */
    if (IPH_IS_VALID(p))
        CheckTagging(p);

    otn_tmp = NULL;

    /*
    **  If we found events in this packet, let's flush
    **  the stream to make sure that we didn't miss any
    **  attacks before this packet.
    */
    if(retval && IsTCP(p) && stream_api)
        stream_api->alert_flush_stream(p);

#ifdef PPM_MGR
    if( PPM_PKTS_ENABLED() )
    {
        PPM_GET_TIME();
        PPM_TOTAL_PKT_TIME();
        PPM_ACCUM_PKT_TIME();
#ifdef DEBUG
        if( PPM_DEBUG_PKTS() )
        {
            LogMessage("PPM: Pkt[%u] Used= ",(unsigned)pktcnt);
            PPM_PRINT_PKT_TIME("%g usecs\n");
            LogMessage("PPM: Process-EndPkt[%u]\n\n",(unsigned)pktcnt);
        }
#endif
       // When detection is required to happen and it is skipped , then only we will print the trace. 
        if (do_detect && (!detect_retval)) 
            PPM_LATENCY_TRACE();

        PPM_PKT_LOG(p);
    }
    if( PPM_RULES_ENABLED() )
    {
        PPM_RULE_LOG(pktcnt, p);
    }
    if( PPM_PKTS_ENABLED() )
    {
        PPM_END_PKT_TIMER();
    }
#endif

    return retval;
}

/*
**  NAME
**    CheckTagging::
*/
/**
**  This is where we check to see if we tag the packet.  We only do
**  this if we've alerted on a non-pass rule and the packet is not
**  rebuilt.
**
**  We don't log rebuilt packets because the output plugins log the
**  individual packets of a rebuilt stream, so we don't want to dup
**  tagged packets for rebuilt streams.
**
**  @return integer
*/
static int CheckTagging(Packet *p)
{
    Event event;

    if(check_tags_flag == 1 && !(p->packet_flags & PKT_REBUILT_STREAM))
    {
        void* listhead = NULL;
        DEBUG_WRAP(DebugMessage(DEBUG_FLOW, "calling CheckTagList\n"););

        if(CheckTagList(p, &event, &listhead))
        {
            DEBUG_WRAP(DebugMessage(DEBUG_FLOW, "Matching tag node found, "
                        "calling log functions\n"););

            /* if we find a match, we want to send the packet to the
             * logging mechanism
             */
            CallLogFuncs(p, "Tagged Packet", listhead, &event);
        }
    }

    return 0;
}

#if defined(FEAT_OPEN_APPID)
static void updateEventAppName (Packet *p, OptTreeNode *otn, Event *event)
{
    const char *appName;
    size_t appNameLen;
    AppIdOptionData *app_data = (AppIdOptionData*)otn->ds_list[PLUGIN_APPID];

    if (app_data && (app_data->matched_appid) && (appName = appIdApi.getApplicationName(app_data->matched_appid)))
    {
        appNameLen = strlen(appName);
        if (appNameLen >= sizeof(event->app_name))
            appNameLen = sizeof(event->app_name) - 1;
        memcpy(event->app_name, appName, appNameLen);
        event->app_name[appNameLen] = '\0';
    }
    else if (p->ssnptr)
    {
        //log most specific appid when rule didn't have any appId
        int16_t serviceProtoId, clientProtoId, payloadProtoId, miscProtoId, pickedProtoId;

        stream_api->get_application_id(p->ssnptr, &serviceProtoId, &clientProtoId, &payloadProtoId, &miscProtoId);
        if ((p->packet_flags & PKT_FROM_CLIENT))
        {
            if (!(pickedProtoId = payloadProtoId) &&  !(pickedProtoId = miscProtoId) && !(pickedProtoId = clientProtoId))
                pickedProtoId = serviceProtoId;
        }
        else
        {
            if (!(pickedProtoId = payloadProtoId) &&  !(pickedProtoId = miscProtoId) && !(pickedProtoId = serviceProtoId))
                pickedProtoId = clientProtoId;
        }

        if ((pickedProtoId) && (appName = appIdApi.getApplicationName(pickedProtoId)))
        {
            appNameLen = strlen(appName);
            if (appNameLen >= sizeof(event->app_name))
                appNameLen = sizeof(event->app_name) - 1;
            memcpy(event->app_name, appName, appNameLen);
            event->app_name[appNameLen] = '\0';
        }
        else
        {
            event->app_name[0] = 0;
        }
    }
    else
    {
        event->app_name[0] = 0;
    }
}
#endif /* defined(FEAT_OPEN_APPID) */
void CallLogFuncs(Packet *p, const char *message, ListHead *head, Event *event)
{
    OutputFuncNode *idx = NULL;

    if (event->sig_generator != GENERATOR_TAG)
    {
        event->ref_time.tv_sec = p->pkth->ts.tv_sec;
        event->ref_time.tv_usec = p->pkth->ts.tv_usec;
    }
    /* set the event number */
    event->event_id = event_id | ScEventLogId();

    check_tags_flag = 0;

    pc.log_pkts++;

    if ( head == NULL || head->LogList == NULL )
    {
        CallLogPlugins(p, message, event);
        return;
    }

    idx = head->LogList;
    while ( idx != NULL )
    {
        idx->func(p, message, idx->arg, event);
        idx = idx->next;
    }
}

void CallLogPlugins(Packet * p, const char *message, Event *event)
{
    OutputFuncNode *idx = LogList;

    while ( idx != NULL )
    {
        idx->func(p, message, idx->arg, event);
        idx = idx->next;
    }
}

/* Call the output functions that are directly attached to the signature */
void CallSigOutputFuncs(Packet *p, OptTreeNode *otn, Event *event)
{
    OutputFuncNode *idx = NULL;

    idx = otn->outputFuncs;

    while(idx)
    {
        idx->func(p, otn->sigInfo.message, idx->arg, event);
        idx = idx->next;
    }
}

void CallAlertFuncs(Packet * p, const char *message, ListHead * head, Event *event)
{
    OutputFuncNode *idx = NULL;

    event->ref_time.tv_sec = p->pkth->ts.tv_sec;
    event->ref_time.tv_usec = p->pkth->ts.tv_usec;

    /* set the event number */
    event->event_id = event_id | ScEventLogId();
    /* set the event reference info */
    event->event_reference = event->event_id;

    pc.total_alert_pkts++;

    if ( event->sig_generator != GENERATOR_SPP_REPUTATION )
    {
        /* Don't include IP Reputation events in count */
        pc.alert_pkts++;
    }

    if ( head == NULL || head->AlertList == NULL )
    {
        CallAlertPlugins(p, message, event);
        return;
    }

    idx = head->AlertList;
    while ( idx != NULL )
    {
        idx->func(p, message, idx->arg, event);
        idx = idx->next;
    }
}


void CallAlertPlugins(Packet * p, const char *message, Event *event)
{
    OutputFuncNode *idx = AlertList;

    while ( idx != NULL )
    {
        idx->func(p, message, idx->arg, event);
        idx = idx->next;
    }
}

/****************************************************************************
 *
 * Function: Detect(Packet *)
 *
 * Purpose: Apply the rules lists to the current packet
 *
 * Arguments: p => ptr to the decoded packet struct
 *
 * Returns: 1 == detection event
 *          0 == no detection
 *
 ***************************************************************************/
int Detect(Packet * p)
{
    int detected = 0;
    PROFILE_VARS;

#if defined(DAQ_VERSION) && DAQ_VERSION > 9
    uint64_t start = 0, end = 0;
#endif

    if ((p == NULL) || !IPH_IS_VALID(p))
    {
        return 0;
    }
    
    if (stream_api && stream_api->is_session_http2(p->ssnptr) 
        && !(p->packet_flags & PKT_REBUILT_STREAM)
        && !(p->packet_flags & PKT_PDU_TAIL)) 
    {
        return 0;
    }

    if (!snort_conf->ip_proto_array[GET_IPH_PROTO(p)])
    {
#ifdef GRE
        switch (p->outer_family)
        {
            case AF_INET:
                if (!snort_conf->ip_proto_array[p->outer_ip4h.ip_proto])
                    return 0;
                break;

            case AF_INET6:
                if (!snort_conf->ip_proto_array[p->outer_ip6h.next])
                    return 0;
                break;

            default:
                return 0;
        }
#else
        return 0;
#endif  /* GRE */
    }

    if (p->packet_flags & PKT_PASS_RULE)
    {
        /* If we've already seen a pass rule on this,
         * no need to continue do inspection.
         */
        return 0;
    }

#ifdef PPM_MGR
    /*
     * Packet Performance Monitoring
     * (see if preprocessing took too long)
     */
    if( PPM_PKTS_ENABLED() )
    {
        PPM_GET_TIME();
        PPM_PACKET_TEST();

        if( PPM_PACKET_ABORT_FLAG() )
            return 0;
    }
#endif

    /*
    **  This is where we short circuit so
    **  that we can do IP checks.
    */
    PREPROC_PROFILE_START(detectPerfStats);
 #if defined(DAQ_VERSION) && DAQ_VERSION > 9
    if (p->pkth && (p->pkth->flags & DAQ_PKT_FLAG_DEBUG_ON))
    {
        get_clockticks(start);
        detected = fpEvalPacket(p);
        get_clockticks(end);
        print_flow(p,"DETECTION",0,start,end);
    }
    else
        detected = fpEvalPacket(p);
 #else
    detected = fpEvalPacket(p);
#endif

    PREPROC_PROFILE_END(detectPerfStats);

    return detected;
}

void TriggerResponses(Packet * p, OptTreeNode * otn)
{

    RspFpList *idx;

    idx = otn->rsp_func;

    DEBUG_WRAP(DebugMessage(DEBUG_DETECT,"Triggering responses %p\n", idx););

    while(idx != NULL)
    {
        idx->func(p, idx->params);
        idx = idx->next;
    }

}

int CheckAddrPort(
                sfip_var_t *rule_addr,
                PortObject * po,
                Packet *p,
                uint32_t flags, int mode)
{
    sfaddr_t* pkt_addr;              /* packet IP address */
    u_short pkt_port;                /* packet port */
    int global_except_addr_flag = 0; /* global exception flag is set */
    int any_port_flag = 0;           /* any port flag set */
    int except_port_flag = 0;        /* port exception flag set */
    int ip_match = 0;                /* flag to indicate addr match made */

    DEBUG_WRAP(DebugMessage(DEBUG_DETECT, "CheckAddrPort: "););
    /* set up the packet particulars */
    if(mode & CHECK_SRC_IP)
    {
        pkt_addr = GET_SRC_IP(p);
        pkt_port = p->sp;

        DEBUG_WRAP(DebugMessage(DEBUG_DETECT,"SRC "););

        if(mode & INVERSE)
        {
            global_except_addr_flag = flags & EXCEPT_DST_IP;
            any_port_flag = flags & ANY_DST_PORT;
            except_port_flag = flags & EXCEPT_DST_PORT;
        }
        else
        {
            global_except_addr_flag = flags & EXCEPT_SRC_IP;
            any_port_flag = flags & ANY_SRC_PORT;
            except_port_flag = flags & EXCEPT_SRC_PORT;
        }
    }
    else
    {
        pkt_addr = GET_DST_IP(p);
        pkt_port = p->dp;

        DEBUG_WRAP(DebugMessage(DEBUG_DETECT, "DST "););

        if(mode & INVERSE)
        {
            global_except_addr_flag = flags & EXCEPT_SRC_IP;
            any_port_flag = flags & ANY_SRC_PORT;
            except_port_flag = flags & EXCEPT_SRC_PORT;
        }
        else
        {
            global_except_addr_flag = flags & EXCEPT_DST_IP;
            any_port_flag = flags & ANY_DST_PORT;
            except_port_flag = flags & EXCEPT_DST_PORT;
        }
    }

    DEBUG_WRAP(DebugMessage(DEBUG_DETECT, "addr %lx, port %d ", pkt_addr,
                pkt_port););

    if(!rule_addr)
        goto bail;

    if(!(global_except_addr_flag)) /*modeled after Check{Src,Dst}IP function*/
    {
        if(sfvar_ip_in(rule_addr, pkt_addr))
            ip_match = 1;
    }
    else
    {
        DEBUG_WRAP(DebugMessage(DEBUG_DETECT, ", global exception flag set"););
        /* global exception flag is up, we can't match on *any*
         * of the source addresses
         */

        if(sfvar_ip_in(rule_addr, pkt_addr))
            return 0;

        ip_match=1;
    }

bail:
    if(!ip_match)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_DETECT, ", no address match,  "
                    "packet rejected\n"););
        return 0;
    }

    DEBUG_WRAP(DebugMessage(DEBUG_DETECT, ", addresses accepted"););

    /* if the any port flag is up, we're all done (success) */
    if(any_port_flag)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_DETECT, ", any port match, "
                    "packet accepted\n"););
        return 1;
    }

#ifdef TARGET_BASED
    if (!(mode & (CHECK_SRC_PORT | CHECK_DST_PORT)))
    {
        DEBUG_WRAP(
            DebugMessage(DEBUG_ATTRIBUTE, "detect.c: CheckAddrPort..."
                "target-based-protocol=%d,ignoring ports\n",
                GetProtocolReference(p)););
        return 1;
    }
    else
    {
        DEBUG_WRAP(
            DebugMessage(DEBUG_ATTRIBUTE, "detect.c: CheckAddrPort..."
                "target-based-protocol=%d,not ignoring ports\n",
                GetProtocolReference(p)););
    }
#endif /* TARGET_BASED */

    /* check the packet port against the rule port */
    if( !PortObjectHasPort(po,pkt_port) )
    {
        /* if the exception flag isn't up, fail */
        if(!except_port_flag)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_DETECT, ", port mismatch,  "
                        "packet rejected\n"););
            return 0;
        }
        DEBUG_WRAP(DebugMessage(DEBUG_DETECT, ", port mismatch exception"););
    }
    else
    {
        /* if the exception flag is up, fail */
        if(except_port_flag)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_DETECT,
                                    ", port match exception,  packet rejected\n"););
            return 0;
        }
        DEBUG_WRAP(DebugMessage(DEBUG_DETECT, ", ports match"););
    }

    /* ports and address match */
    DEBUG_WRAP(DebugMessage(DEBUG_DETECT, ", packet accepted!\n"););
    return 1;

}

/****************************************************************************
 *
 * Function: DumpList(IpAddrNode*)
 *
 * Purpose: print out the chain lists by header block node group
 *
 * Arguments: node => the head node
 *
 * Returns: void function
 *
 ***************************************************************************/
void DumpList(IpAddrNode *idx, int negated)
{
    DEBUG_WRAP(int i=0;);
    if(!idx)
        return;

    while(idx != NULL)
    {
       DEBUG_WRAP(DebugMessage(DEBUG_RULES,
                        "[%d]    %s",
                        i++, sfip_ntoa(&idx->ip->addr)););

       if(negated)
       {
           DEBUG_WRAP(DebugMessage(DEBUG_RULES,
                       "    (EXCEPTION_FLAG Active)\n"););
       }
       else
       {
           DEBUG_WRAP(DebugMessage(DEBUG_RULES, "\n"););
       }

       idx = idx->next;
    }
}


/****************************************************************************
 *
 * Function: DumpChain(RuleTreeNode *, char *, char *)
 *
 * Purpose: Iterate over RTNs calling DumpList on each
 *
 * Arguments: rtn_idx => the RTN index pointer
 *                       rulename => the name of the rule the list belongs to
 *            listname => the name of the list being printed out
 *
 * Returns: void function
 *
 ***************************************************************************/
void DumpChain(RuleTreeNode * rtn_head, char *rulename, char *listname)
{
    // XXX Not yet implemented - Rule chain dumping
}

#define CHECK_ADDR_SRC_ARGS(x) (x)->src_portobject
#define CHECK_ADDR_DST_ARGS(x) (x)->dst_portobject

int CheckBidirectional(Packet *p, struct _RuleTreeNode *rtn_idx,
        RuleFpList *fp_list, int check_ports)
{
    DEBUG_WRAP(DebugMessage(DEBUG_DETECT, "Checking bidirectional rule...\n"););

    if(CheckAddrPort(rtn_idx->sip, CHECK_ADDR_SRC_ARGS(rtn_idx), p,
                     rtn_idx->flags, CHECK_SRC_IP | (check_ports ? CHECK_SRC_PORT : 0)))
    {
        DEBUG_WRAP(DebugMessage(DEBUG_DETECT, "   Src->Src check passed\n"););
        if(! CheckAddrPort(rtn_idx->dip, CHECK_ADDR_DST_ARGS(rtn_idx), p,
                           rtn_idx->flags, CHECK_DST_IP | (check_ports ? CHECK_DST_PORT : 0)))
        {
            DEBUG_WRAP(DebugMessage(DEBUG_DETECT,
                                    "   Dst->Dst check failed,"
                                    " checking inverse combination\n"););
            if(CheckAddrPort(rtn_idx->dip, CHECK_ADDR_DST_ARGS(rtn_idx), p,
                             rtn_idx->flags, (CHECK_SRC_IP | INVERSE | (check_ports ? CHECK_SRC_PORT : 0))))
            {
                DEBUG_WRAP(DebugMessage(DEBUG_DETECT,
                                    "   Inverse Dst->Src check passed\n"););
                if(!CheckAddrPort(rtn_idx->sip, CHECK_ADDR_SRC_ARGS(rtn_idx), p,
                                  rtn_idx->flags, (CHECK_DST_IP | INVERSE | (check_ports ? CHECK_DST_PORT : 0))))
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_DETECT,
                                    "   Inverse Src->Dst check failed\n"););
                    return 0;
                }
                else
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_DETECT, "Inverse addr/port match\n"););
                }
            }
            else
            {
                DEBUG_WRAP(DebugMessage(DEBUG_DETECT, "   Inverse Dst->Src check failed,"
                                        " trying next rule\n"););
                return 0;
            }
        }
        else
        {
            DEBUG_WRAP(DebugMessage(DEBUG_DETECT, "dest IP/port match\n"););
        }
    }
    else
    {
        DEBUG_WRAP(DebugMessage(DEBUG_DETECT,
                                "   Src->Src check failed, trying inverse test\n"););
        if(CheckAddrPort(rtn_idx->dip, CHECK_ADDR_DST_ARGS(rtn_idx), p,
                         rtn_idx->flags, CHECK_SRC_IP | INVERSE | (check_ports ? CHECK_SRC_PORT : 0)))
        {
            DEBUG_WRAP(DebugMessage(DEBUG_DETECT,
                        "   Dst->Src check passed\n"););

            if(!CheckAddrPort(rtn_idx->sip, CHECK_ADDR_SRC_ARGS(rtn_idx), p,
                        rtn_idx->flags, CHECK_DST_IP | INVERSE | (check_ports ? CHECK_DST_PORT : 0)))
            {
                DEBUG_WRAP(DebugMessage(DEBUG_DETECT,
                            "   Src->Dst check failed\n"););
                return 0;
            }
            else
            {
                DEBUG_WRAP(DebugMessage(DEBUG_DETECT,
                            "Inverse addr/port match\n"););
            }
        }
        else
        {
            DEBUG_WRAP(DebugMessage(DEBUG_DETECT,"   Inverse test failed, "
                        "testing next rule...\n"););
            return 0;
        }
    }

    DEBUG_WRAP(DebugMessage(DEBUG_DETECT,"   Bidirectional success!\n"););
    return 1;
}


/****************************************************************************
 *
 * Function: CheckSrcIp(Packet *, struct _RuleTreeNode *, RuleFpList *)
 *
 * Purpose: Test the source IP and see if it equals the SIP of the packet
 *
 * Arguments: p => ptr to the decoded packet data structure
 *            rtn_idx => ptr to the current rule data struct
 *            fp_list => ptr to the current function pointer node
 *
 * Returns: 0 on failure (no match), 1 on success (match)
 *
 ***************************************************************************/
int CheckSrcIP(Packet * p, struct _RuleTreeNode * rtn_idx, RuleFpList * fp_list, int check_ports)
{

    DEBUG_WRAP(DebugMessage(DEBUG_DETECT,"CheckSrcIPEqual: "););

    if(!(rtn_idx->flags & EXCEPT_SRC_IP))
    {
        if( sfvar_ip_in(rtn_idx->sip, GET_SRC_IP(p)) )
        {
// XXX NOT YET IMPLEMENTED - debugging in Snort6
#if 0
#ifdef DEBUG_MSGS
            sfaddr_t ip;
            if(idx->addr_flags & EXCEPT_IP) {
                DebugMessage(DEBUG_DETECT, "  SIP exception match\n");
            }
            else
            {
                DebugMessage(DEBUG_DETECT, "  SIP match\n");
            }

            ip = *iph_ret_src(p);    /* necessary due to referencing/dereferencing */
            DebugMessage(DEBUG_DETECT, "Rule: %s     Packet: %s\n",
                   inet_ntoa(idx->ip_addr), inet_ntoa(ip));
#endif /* DEBUG */
#endif

            /* the packet matches this test, proceed to the next test */
            return fp_list->next->RuleHeadFunc(p, rtn_idx, fp_list->next, check_ports);
        }
    }
    else
    {
        /* global exception flag is up, we can't match on *any*
         * of the source addresses
         */
        DEBUG_WRAP(DebugMessage(DEBUG_DETECT,"  global exception flag, \n"););

        if( sfvar_ip_in(rtn_idx->sip, GET_SRC_IP(p)) ) return 0;

        return fp_list->next->RuleHeadFunc(p, rtn_idx, fp_list->next, check_ports);
    }

    DEBUG_WRAP(DebugMessage(DEBUG_DETECT,"  Mismatch on SIP\n"););

    return 0;

    /* return 0 on a failed test */
    return 0;
}


/****************************************************************************
 *
 * Function: CheckDstIp(Packet *, struct _RuleTreeNode *, RuleFpList *)
 *
 * Purpose: Test the dest IP and see if it equals the DIP of the packet
 *
 * Arguments: p => ptr to the decoded packet data structure
 *            rtn_idx => ptr to the current rule data struct
 *            fp_list => ptr to the current function pointer node
 *
 * Returns: 0 on failure (no match), 1 on success (match)
 *
 ***************************************************************************/
int CheckDstIP(Packet *p, struct _RuleTreeNode *rtn_idx, RuleFpList *fp_list, int check_ports)
{

    DEBUG_WRAP(DebugMessage(DEBUG_DETECT, "CheckDstIPEqual: ");)

    if(!(rtn_idx->flags & EXCEPT_DST_IP))
    {
        if( sfvar_ip_in(rtn_idx->dip, GET_DST_IP(p)) )
        {
// #ifdef DEBUG_MSGS
// XXX idx's equivalent is lost inside of sfvar_ip_in
//            DebugMessage(DEBUG_DETECT, "Rule: %s     Packet: ",
//                   inet_ntoa(idx->ip_addr));
//            DebugMessage(DEBUG_DETECT, "%s\n", sfip_ntoa(iph_ret_dst(p)));
// #endif

            /* the packet matches this test, proceed to the next test */
            return fp_list->next->RuleHeadFunc(p, rtn_idx, fp_list->next, check_ports);
        }
    }
    else
    {
        /* global exception flag is up, we can't match on *any*
         * of the source addresses */
        DEBUG_WRAP(DebugMessage(DEBUG_DETECT,"  global exception flag, \n"););

        if( sfvar_ip_in(rtn_idx->dip, GET_DST_IP(p)) ) return 0;

        return fp_list->next->RuleHeadFunc(p, rtn_idx, fp_list->next, check_ports);
    }

    return 0;
}


int CheckSrcPortEqual(Packet *p, struct _RuleTreeNode *rtn_idx,
        RuleFpList *fp_list, int check_ports)
{
    DEBUG_WRAP(DebugMessage(DEBUG_DETECT,"CheckSrcPortEqual: "););

#ifdef TARGET_BASED
    /* Check if attributes provided match earlier */
    if (check_ports == 0)
    {
        DEBUG_WRAP(
            DebugMessage(DEBUG_ATTRIBUTE, "detect.c: CheckSrcPortEq..."
                "target-based-protocol=%d,ignoring ports\n",
                GetProtocolReference(p)););
        return fp_list->next->RuleHeadFunc(p, rtn_idx, fp_list->next, check_ports);
    }
    else
    {
        DEBUG_WRAP(
            DebugMessage(DEBUG_ATTRIBUTE, "detect.c: CheckSrcPortEq..."
                "target-based-protocol=%d,not ignoring ports\n",
                GetProtocolReference(p)););
    }
#endif /* TARGET_BASED */
    if( PortObjectHasPort(rtn_idx->src_portobject,p->sp) )
    {
        DEBUG_WRAP(DebugMessage(DEBUG_DETECT, "  SP match!\n"););
        return fp_list->next->RuleHeadFunc(p, rtn_idx, fp_list->next, check_ports);
    }
    else
    {
        DEBUG_WRAP(DebugMessage(DEBUG_DETECT, "   SP mismatch!\n"););
    }

    return 0;
}

int CheckSrcPortNotEq(Packet *p, struct _RuleTreeNode *rtn_idx,
        RuleFpList *fp_list, int check_ports)
{
    DEBUG_WRAP(DebugMessage(DEBUG_DETECT,"CheckSrcPortNotEq: "););

#ifdef TARGET_BASED
    /* Check if attributes provided match earlier */
    if (check_ports == 0)
    {
        DEBUG_WRAP(
            DebugMessage(DEBUG_ATTRIBUTE, "detect.c: CheckSrcPortNotEq..."
                "target-based-protocol=%d,ignoring ports\n",
                GetProtocolReference(p)););
        return fp_list->next->RuleHeadFunc(p, rtn_idx, fp_list->next, check_ports);
    }
    else
    {
        DEBUG_WRAP(
            DebugMessage(DEBUG_ATTRIBUTE, "detect.c: CheckSrcPortNotEq..."
                "target-based-protocol=%d,not ignoring ports\n",
                GetProtocolReference(p)););
    }
#endif /* TARGET_BASED */
    if( !PortObjectHasPort(rtn_idx->src_portobject,p->sp) )
    {
        DEBUG_WRAP(DebugMessage(DEBUG_DETECT, "  !SP match!\n"););
        return fp_list->next->RuleHeadFunc(p, rtn_idx, fp_list->next, check_ports);
    }
    else
    {
        DEBUG_WRAP(DebugMessage(DEBUG_DETECT, "  !SP mismatch!\n"););
    }

    return 0;
}

int CheckDstPortEqual(Packet *p, struct _RuleTreeNode *rtn_idx,
        RuleFpList *fp_list, int check_ports)
{
    DEBUG_WRAP(DebugMessage(DEBUG_DETECT,"CheckDstPortEqual: "););

#ifdef TARGET_BASED
    /* Check if attributes provided match earlier */
    if (check_ports == 0)
    {
        DEBUG_WRAP(
            DebugMessage(DEBUG_ATTRIBUTE, "detect.c: CheckDstPortEq..."
            "target-based-protocol=%d,ignoring ports\n",
            GetProtocolReference(p)););
        return fp_list->next->RuleHeadFunc(p, rtn_idx, fp_list->next, check_ports);
    }
    else
    {
        DEBUG_WRAP(
            DebugMessage(DEBUG_ATTRIBUTE, "detect.c: CheckDstPortEq..."
            "target-based-protocol=%d,not ignoring ports\n",
            GetProtocolReference(p)););
    }
#endif /* TARGET_BASED */
    if( PortObjectHasPort(rtn_idx->dst_portobject,p->dp) )
    {
        DEBUG_WRAP(DebugMessage(DEBUG_DETECT, " DP match!\n"););
        return fp_list->next->RuleHeadFunc(p, rtn_idx, fp_list->next, check_ports);
    }
    else
    {
        DEBUG_WRAP(DebugMessage(DEBUG_DETECT," DP mismatch!\n"););
    }
    return 0;
}


int CheckDstPortNotEq(Packet *p, struct _RuleTreeNode *rtn_idx,
        RuleFpList *fp_list, int check_ports)
{
    DEBUG_WRAP(DebugMessage(DEBUG_DETECT,"CheckDstPortNotEq: "););

#ifdef TARGET_BASED
    /* Check if attributes provided match earlier */
    if (check_ports == 0)
    {
        DEBUG_WRAP(
            DebugMessage(DEBUG_ATTRIBUTE, "detect.c: CheckDstPortNotEq..."
            "target-based-protocol=%d,ignoring ports\n",
            GetProtocolReference(p)););
        return fp_list->next->RuleHeadFunc(p, rtn_idx, fp_list->next, check_ports);
    }
    else
    {
        DEBUG_WRAP(
            DebugMessage(DEBUG_ATTRIBUTE, "detect.c: CheckDstPortNotEq..."
            "target-based-protocol=%d,not ignoring ports\n",
            GetProtocolReference(p)););
    }
#endif /* TARGET_BASED */
    if( !PortObjectHasPort(rtn_idx->dst_portobject,p->dp) )
    {
        DEBUG_WRAP(DebugMessage(DEBUG_DETECT, " !DP match!\n"););
        return fp_list->next->RuleHeadFunc(p, rtn_idx, fp_list->next, check_ports);
    }
    else
    {
        DEBUG_WRAP(DebugMessage(DEBUG_DETECT," !DP mismatch!\n"););
    }

    return 0;
}

int RuleListEnd(Packet *p, struct _RuleTreeNode *rtn_idx,
        RuleFpList *fp_list, int check_ports)
{
    return 1;
}


int OptListEnd(void *option_data, Packet *p)
{
    return DETECTION_OPTION_MATCH;
}

/* Rule Match Action Functions */
int PassAction(void)
{
    pc.pass_pkts++;

    DEBUG_WRAP(DebugMessage(DEBUG_DETECT,"   => Pass rule, returning...\n"););
    return 1;
}

int AlertAction(Packet * p, OptTreeNode * otn, RuleTreeNode * rtn, Event * event)
{
    if (!rtn)
    {
        // This function may be called from ppm, which doesn't do an RTN lookup
        rtn = getRuntimeRtnFromOtn(otn);
        if (!rtn)
            return 0;
    }

    DEBUG_WRAP(DebugMessage(DEBUG_DETECT,
                "        <!!> Generating alert! \"%s\", policyId %d\n", otn->sigInfo.message, getIpsRuntimePolicy()););
#if defined(FEAT_OPEN_APPID)
    updateEventAppName (p, otn, event);
#endif /* defined(FEAT_OPEN_APPID) */

    /* Call OptTreeNode specific output functions */
    if(otn->outputFuncs)
        CallSigOutputFuncs(p, otn, event);

    if (ScAlertPacketCount())
        print_packet_count();

    CallAlertFuncs(p, otn->sigInfo.message, rtn->listhead, event);

    DEBUG_WRAP(DebugMessage(DEBUG_DETECT, "   => Finishing alert packet!\n"););

    CallLogFuncs(p, otn->sigInfo.message, rtn->listhead, event);

    /*
    if(p->ssnptr != NULL && stream_api)
    {
        if(stream_api->alert_flush_stream(p) == 0)
        {
            CallLogFuncs(p, otn->sigInfo.message, otn->rtn->listhead, event);
        }
    }
    else
    {
        CallLogFuncs(p, otn->sigInfo.message, otn->rtn->listhead, event);
    }
    */

    DEBUG_WRAP(DebugMessage(DEBUG_DETECT,"   => Alert packet finished, returning!\n"););

    return 1;
}

int DropAction(Packet * p, OptTreeNode * otn, RuleTreeNode * rtn, Event * event)
{
    DEBUG_WRAP(DebugMessage(DEBUG_DETECT,
               "        <!!> Generating Alert and dropping! \"%s\"\n",
               otn->sigInfo.message););

    if(stream_api && !stream_api->alert_inline_midstream_drops())
    {
        if(session_api->get_session_flags(p->ssnptr) & SSNFLAG_MIDSTREAM)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_DETECT,
                " <!!> Alert Came From Midstream Session Silently Drop! "
                "\"%s\"\n", otn->sigInfo.message););

            Active_DropSession(p);
            if (pkt_trace_enabled)
                addPktTraceData(VERDICT_REASON_SNORT, snprintf(trace_line, MAX_TRACE_LINE,
                    "Snort: gid %u, sid %u, midstream %s\n", otn->sigInfo.generator, otn->sigInfo.id, getPktTraceActMsg()));
            else addPktTraceData(VERDICT_REASON_SNORT, 0);
            return 1;
        }
    }

    /*
    **  Set packet flag so output plugins will know we dropped the
    **  packet we just logged.
    */
    Active_DropSession(p);
#if defined(FEAT_OPEN_APPID)
    updateEventAppName (p, otn, event);
#endif /* defined(FEAT_OPEN_APPID) */

    CallAlertFuncs(p, otn->sigInfo.message, rtn->listhead, event);

    CallLogFuncs(p, otn->sigInfo.message, rtn->listhead, event);

    if (pkt_trace_enabled)
        addPktTraceData(VERDICT_REASON_SNORT, snprintf(trace_line, MAX_TRACE_LINE,
            "Snort detect_drop: gid %u, sid %u, %s\n", otn->sigInfo.generator, otn->sigInfo.id, getPktTraceActMsg()));
    else addPktTraceData(VERDICT_REASON_SNORT, 0);
    return 1;
}

int SDropAction(Packet * p, OptTreeNode * otn, Event * event)
{
    DEBUG_WRAP(DebugMessage(DEBUG_DETECT,
               "        <!!> Dropping without Alerting! \"%s\"\n",
               otn->sigInfo.message););

    // Let's silently drop the packet
    Active_DropSession(p);
    if (pkt_trace_enabled)
        addPktTraceData(VERDICT_REASON_SNORT, snprintf(trace_line, MAX_TRACE_LINE,
            "Snort detect_sdrop: gid %u, sid %u, %s\n", otn->sigInfo.generator, otn->sigInfo.id, getPktTraceActMsg()));
    else addPktTraceData(VERDICT_REASON_SNORT, 0);
    return 1;
}

int LogAction(Packet * p, OptTreeNode * otn, RuleTreeNode * rtn, Event * event)
{
    DEBUG_WRAP(DebugMessage(DEBUG_DETECT,"   => Logging packet data and returning...\n"););

    CallLogFuncs(p, otn->sigInfo.message, rtn->listhead, event);

#ifdef BENCHMARK
    printf("        <!!> Check count = %d\n", check_count);
    check_count = 0;
    printf(" **** cmpcount: %d **** \n", cmpcount);
#endif

    return 1;
}

/* end of rule action functions */

