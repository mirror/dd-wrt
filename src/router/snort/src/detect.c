/* $Id$ */
/*
** Copyright (C) 1998-2002 Martin Roesch <roesch@sourcefire.com>
** Copyright (C) 2002-2011 Sourcefire, Inc.
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
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
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
#include <assert.h>

#include "snort.h"
#include "detect.h"
#include "plugbase.h"
#include "debug.h"
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
#include "stream_api.h"
#include "active.h"
#include "signature.h"
#include "ipv6_port.h"
#include "ppm.h"
#include "sf_types.h"
#include "active.h"
#include "detection_util.h"

#ifdef PORTLISTS
#include "sfutil/sfportobject.h"
#endif
#ifdef PERF_PROFILING
PreprocStats detectPerfStats;
#endif

extern int preproc_proto_mask;
extern OutputFuncNode *AlertList;
extern OutputFuncNode *LogList;

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

int Preprocess(Packet * p)
{
    int retval = 0;
    tSfPolicyId policy_id = getRuntimePolicy();
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
        if( PPM_DEBUG_PKTS() )
        {
           /* for debugging, info gathering, so don't worry about
           *  (unsigned) casting of pktcnt, were not likely to debug
           *  4G packets
           */
           LogMessage("PPM: Process-BeginPkt[%u] caplen=%u\n",
             (unsigned)pktcnt,p->pkth->caplen);
        }
    }
#endif
    
    // If the packet has errors, we won't analyze it.
    if ( p->error_flags )
    {
        DEBUG_WRAP(DebugMessage(DEBUG_DETECT,
            "Packet errors = 0x%x, ignoring traffic!\n", p->error_flags););

        if ( p->error_flags & PKT_ERR_BAD_TTL )
            pc.bad_ttl++;
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

        do_detect = do_detect_content = 1;

        /*
        **  Reset the appropriate application-layer protocol fields
        */
        p->uri_count = 0;
        DetectReset();
        p->alt_dsize = 0;

        /* Most preprocessor protocols are over TCP and 90+ percent of traffic in most
         * environments is TCP so this check almost always passes.  Initial performance
         * tests indicate this check hinders performance slightly, but keep it here
         * commented in case initial performance tests are wrong.  Its main purpose is
         * to filter out traffic that no preprocessors are going to look at thus
         * avoiding iterating through each preprocessor */
        //if (p->proto_bits & preproc_proto_mask)
        {
            PreprocEvalFuncNode *idx = policy->preproc_eval_funcs;

            /* Turn on all preprocessors */
            EnablePreprocessors(p);

            for (; (idx != NULL) && !(p->packet_flags & PKT_PASS_RULE); idx = idx->next)
            {
                if (((p->proto_bits & idx->proto_mask) || (idx->proto_mask == PROTO_BIT__ALL)) &&
                    IsPreprocBitSet(p, idx->preproc_bit))
                {
                    idx->func(p, idx->context);
                }
            }
        }

        if ((do_detect) && (p->bytes_to_inspect != -1))
        {
            /* Check if we are only inspecting a portion of this packet... */
            if (p->bytes_to_inspect > 0)
            {
                DEBUG_WRAP(DebugMessage(DEBUG_DETECT, "Ignoring part of server "
                    "traffic -- only looking at %d of %d bytes!!!\n",
                    p->bytes_to_inspect, p->dsize););
                p->dsize = (uint16_t)p->bytes_to_inspect;
            }

            Detect(p);
        }
        else if (p->bytes_to_inspect == -1)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_DETECT, "Ignoring server traffic!!!\n"););
        }
    }

    check_tags_flag = 1;
    
    PREPROC_PROFILE_START(eventqPerfStats);
    retval = SnortEventqLog(snort_conf->event_queue, p);
    SnortEventqReset();
    PREPROC_PROFILE_END(eventqPerfStats);

    /*
    ** By checking tagging here, we make sure that we log the
    ** tagged packet whether it generates an alert or not.
    */
    if (IPH_IS_VALID(p))
        CheckTagging(p);

    /* Simulate above behavior for preprocessor reassembled packets */
    if ((p->packet_flags & PKT_PREPROC_RPKT) && do_detect && (p->bytes_to_inspect != -1))
    {
        PreprocReassemblyPktFuncNode *rpkt_idx = policy->preproc_reassembly_pkt_funcs;

        /* Loop through the preprocessors that have registered a 
         * function to get a reassembled packet */
        while (rpkt_idx != NULL)
        {
            Packet *pp = NULL;

            assert(rpkt_idx->func != NULL);

            /* If the preprocessor bit is set, get the reassembled packet */
            if (IsPreprocReassemblyPktBitSet(p, rpkt_idx->preproc_id))
            {
                pp = (Packet *)rpkt_idx->func();
            }

            if (pp != NULL)
            {
                /* If the original packet's bytes to inspect is set,
                 * set it for the reassembled packet */
                if (p->bytes_to_inspect > 0)
                    pp->dsize = (uint16_t)p->bytes_to_inspect;

                if (Detect(pp))
                {
                    PREPROC_PROFILE_START(eventqPerfStats);
                    retval |= SnortEventqLog(snort_conf->event_queue, pp);
                    SnortEventqReset();
                    PREPROC_PROFILE_END(eventqPerfStats);
                }
            }

            rpkt_idx = rpkt_idx->next;
        }
    }

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
        if( PPM_DEBUG_PKTS() )
        {
            LogMessage("PPM: Pkt[%u] Used= ",(unsigned)pktcnt);
            PPM_PRINT_PKT_TIME("%g usecs\n");
            LogMessage("PPM: Process-EndPkt[%u]\n\n",(unsigned)pktcnt);
        }

        PPM_PKT_LOG();
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
        DEBUG_WRAP(DebugMessage(DEBUG_FLOW, "calling CheckTagList\n"););

        if(CheckTagList(p, &event))
        {
            DEBUG_WRAP(DebugMessage(DEBUG_FLOW, "Matching tag node found, "
                        "calling log functions\n"););

            /* if we find a match, we want to send the packet to the
             * logging mechanism
             */
            CallLogFuncs(p, "Tagged Packet", NULL, &event);
        } 
    }

    return 0;
}

/*
 *  11/2/05 marc norton
 *  removed thresholding from this function. This function should only
 *  be called by fpLogEvent, which already does the thresholding test.
 */
void CallLogFuncs(Packet *p, char *message, ListHead *head, Event *event)
{
    OutputFuncNode *idx = NULL;

    event->ref_time.tv_sec = p->pkth->ts.tv_sec;
    event->ref_time.tv_usec = p->pkth->ts.tv_usec;

    /* set the event number */
    event->event_id = event_id | ScEventLogId();

#ifndef SUP_IP6
    if(BsdPseudoPacket) 
    {
        p = BsdPseudoPacket;
    }
#endif
    check_tags_flag = 0;

    if(head == NULL)
    {
        CallLogPlugins(p, message, NULL, event);
        return;
    }

    pc.log_pkts++;
     
    idx = head->LogList;
    if(idx == NULL)
        idx = LogList;

    while(idx != NULL)
    {
        idx->func(p, message, idx->arg, event);
        idx = idx->next;
    }
}

void CallLogPlugins(Packet * p, char *message, void *args, Event *event)
{
    OutputFuncNode *idx;

    idx = LogList;

    pc.log_pkts++;

    while(idx != NULL)
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

/*
 *  11/2/05 marc norton
 *  removed thresholding from this function. This function should only
 *  be called by fpLogEvent, which already does the thresholding test.
 */
void CallAlertFuncs(Packet * p, char *message, ListHead * head, Event *event)
{
    OutputFuncNode *idx = NULL;

    event->ref_time.tv_sec = p->pkth->ts.tv_sec;
    event->ref_time.tv_usec = p->pkth->ts.tv_usec;

    /* set the event number */
    event->event_id = event_id | ScEventLogId();
    /* set the event reference info */
    event->event_reference = event->event_id;

#ifndef SUP_IP6
    if(BsdPseudoPacket) 
    {
        p = BsdPseudoPacket;
    }
#endif

    if(head == NULL)
    {
        CallAlertPlugins(p, message, NULL, event);
        return;
    }

    pc.alert_pkts++;
    idx = head->AlertList;
    if(idx == NULL)
        idx = AlertList;

    while(idx != NULL)
    {
        idx->func(p, message, idx->arg, event);
        idx = idx->next;
    }
}


void CallAlertPlugins(Packet * p, char *message, void *args, Event *event)
{
    OutputFuncNode *idx;

    DEBUG_WRAP(DebugMessage(DEBUG_DETECT, "Call Alert Plugins\n"););
    idx = AlertList;

    pc.alert_pkts++;
    while(idx != NULL)
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

    if ((p == NULL) || !IPH_IS_VALID(p))
    {
        return 0;
    }


    if (!snort_conf->ip_proto_array[GET_IPH_PROTO(p)])
    {
#ifdef GRE
# ifdef SUP_IP6
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
# else
        if ((p->outer_iph == NULL) || !snort_conf->ip_proto_array[p->outer_iph->ip_proto])
            return 0;
# endif  /* SUP_IP6 */
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
    detected = fpEvalPacket(p);
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
#ifdef SUP_IP6
                sfip_var_t *rule_addr,
#else
                IpAddrSet *rule_addr,
#endif
                PortObject * po, 
                Packet *p, 
                uint32_t flags, int mode)
{
    snort_ip_p pkt_addr;              /* packet IP address */
    u_short pkt_port;           /* packet port */
    int global_except_addr_flag = 0; /* global exception flag is set */
    int any_port_flag = 0;           /* any port flag set */
    int except_port_flag = 0;        /* port exception flag set */
    int ip_match = 0;                /* flag to indicate addr match made */
#ifndef SUP_IP6
    IpAddrNode *idx;            /* ip addr struct indexer */
#endif

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
#ifdef SUP_IP6
        if(sfvar_ip_in(rule_addr, pkt_addr)) 
            ip_match = 1;
#else
        ip_match = 0;

        if(rule_addr->iplist)
        {
            for(idx=rule_addr->iplist; idx; idx=idx->next)
            {
                if(idx->ip_addr == (pkt_addr & idx->netmask)) 
                {
                    ip_match = 1; 
                    break;
                }
            }
        }
        else 
            ip_match = 1;
        
        if(ip_match)
        {
            for(idx=rule_addr->neg_iplist; idx; idx=idx->next)
            {
                if(idx->ip_addr == (pkt_addr & idx->netmask)) 
                {
                    ip_match = 0; break;
                }
            }
        }
        
        if(ip_match) 
            goto bail;
#endif
    }
    else
    {
        DEBUG_WRAP(DebugMessage(DEBUG_DETECT, ", global exception flag set"););
        /* global exception flag is up, we can't match on *any* 
         * of the source addresses 
         */

#ifdef SUP_IP6
        if(sfvar_ip_in(rule_addr, pkt_addr)) 
            return 0;

        ip_match=1;
#else
        if(rule_addr->iplist)
        {
            ip_match = 0;
            for(idx=rule_addr->iplist; idx; idx=idx->next)
            {
                if(idx->ip_addr == (pkt_addr & idx->netmask)) 
                {
                    ip_match = 1; 
                    break;
                }
            }
        }
        else 
            ip_match = 1;
        
        if(ip_match)
        {
            for(idx=rule_addr->neg_iplist; idx; idx=idx->next)
            {
                if(idx->ip_addr == (pkt_addr & idx->netmask)) 
                {
                    ip_match = 0; break;
                }
            }
        }
       
        if(!ip_match) 
            return 0;
#endif
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
#ifdef SUP_IP6
       DEBUG_WRAP(DebugMessage(DEBUG_RULES,
                        "[%d]    %s",
                        i++, sfip_ntoa(idx->ip)););
#else
       DEBUG_WRAP(DebugMessage(DEBUG_RULES,
                        "[%d]    0x%.8lX / 0x%.8lX",
                        i++, (u_long) idx->ip_addr,
                        (u_long) idx->netmask););
#endif

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
#ifdef SUP_IP6
    // XXX Not yet implemented - Rule chain dumping
#else

    RuleTreeNode *rtn_idx;

    DEBUG_WRAP(DebugMessage(DEBUG_RULES, "%s %s\n", rulename, listname););

    rtn_idx = rtn_head;

    if(rtn_idx == NULL)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_RULES, "    Empty!\n\n"););
    }

#endif
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
#ifndef SUP_IP6
    int match = 0;
    IpAddrNode *pos_idx, *neg_idx; /* ip address indexer */
#endif

    DEBUG_WRAP(DebugMessage(DEBUG_DETECT,"CheckSrcIPEqual: "););

#ifdef SUP_IP6
    if(!(rtn_idx->flags & EXCEPT_SRC_IP)) 
    {
        if( sfvar_ip_in(rtn_idx->sip, GET_SRC_IP(p)) )
        {
// XXX NOT YET IMPLEMENTED - debugging in Snort6
#if 0
#ifdef DEBUG
            sfip_t ip;
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

#else

    if(rtn_idx->sip)
    {
        match = 0;

        pos_idx = rtn_idx->sip->iplist;                  
        neg_idx = rtn_idx->sip->neg_iplist;                  

        if(!pos_idx) 
        {
            for( ; neg_idx; neg_idx = neg_idx->next) 
            {
                if(neg_idx->ip_addr == 
                           (p->iph->ip_src.s_addr & neg_idx->netmask)) 
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_DETECT,"  Mismatch on SIP\n"););
                    return 0;
                }
            } 

            DEBUG_WRAP(DebugMessage(DEBUG_DETECT,"  SIP match\n"););
            return fp_list->next->RuleHeadFunc(p, rtn_idx, fp_list->next, check_ports);
        }
    
        while(pos_idx)              
        {
            if(neg_idx)
            {
                if(neg_idx->ip_addr == 
                           (p->iph->ip_src.s_addr & neg_idx->netmask)) 
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_DETECT,"  Mismatch on SIP\n"););
                    return 0;
                }
            
                neg_idx = neg_idx->next;
            } 
            /* No more potential negations.  Check if we've already matched. */
            else if(match)
            {
                DEBUG_WRAP(DebugMessage(DEBUG_DETECT,"  SIP match\n"););
                return fp_list->next->RuleHeadFunc(p, rtn_idx, fp_list->next, check_ports);
            }

            if(!match) 
            {
                if(pos_idx->ip_addr == 
                   (p->iph->ip_src.s_addr & pos_idx->netmask)) 
                {
                     match = 1;
                }
                else
                {
                    pos_idx = pos_idx->next;
                }
            }
        } 
    }

    DEBUG_WRAP(DebugMessage(DEBUG_DETECT,"  Mismatch on SIP\n"););

#endif
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
#ifndef SUP_IP6
    IpAddrNode *pos_idx, *neg_idx;  /* ip address indexer */
    int match;
#endif

    DEBUG_WRAP(DebugMessage(DEBUG_DETECT, "CheckDstIPEqual: ");)

#ifdef SUP_IP6
    if(!(rtn_idx->flags & EXCEPT_DST_IP)) 
    {
        if( sfvar_ip_in(rtn_idx->dip, GET_DST_IP(p)) )
        {
// #ifdef DEBUG
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
#else

    if(rtn_idx->dip)
    {
        match = 0;

        pos_idx = rtn_idx->dip->iplist;                  
        neg_idx = rtn_idx->dip->neg_iplist;                  

        if(!pos_idx) 
        {
            for( ; neg_idx; neg_idx = neg_idx->next) 
            {
                if(neg_idx->ip_addr == 
                           (p->iph->ip_dst.s_addr & neg_idx->netmask)) 
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_DETECT,"  Mismatch on DIP\n"););
                    return 0;
                }
            } 

            DEBUG_WRAP(DebugMessage(DEBUG_DETECT,"  DIP match\n"););
            return fp_list->next->RuleHeadFunc(p, rtn_idx, fp_list->next, check_ports);
        }

        while(pos_idx)              
        {
            if(neg_idx)
            {
                if(neg_idx->ip_addr == 
                           (p->iph->ip_dst.s_addr & neg_idx->netmask)) 
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_DETECT, "  DIP exception match\n"););
                    return 0;
                }
            
                neg_idx = neg_idx->next;
            } 
            /* No more potential negations.  Check if we've already matched. */
            else if(match)
            {
                DEBUG_WRAP(DebugMessage(DEBUG_DETECT, "  DIP match\n"););
                return fp_list->next->RuleHeadFunc(p, rtn_idx, fp_list->next, check_ports);
            }

            if(!match) 
            {
                if(pos_idx->ip_addr == 
                   (p->iph->ip_dst.s_addr & pos_idx->netmask)) 
                {
                     match = 1;
                }
                else 
                {
                    pos_idx = pos_idx->next;
                }
            }
        } 
    }

    DEBUG_WRAP(DebugMessage(DEBUG_DETECT, "  DIP exception match\n"););
    return 0;
#endif
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

int ActivateAction(Packet * p, OptTreeNode * otn, Event *event)
{
    RuleTreeNode *rtn = getRuntimeRtnFromOtn(otn);

    DEBUG_WRAP(DebugMessage(DEBUG_DETECT,
                   "        <!!> Activating and generating alert! \"%s\"\n",
                   otn->sigInfo.message););
    CallAlertFuncs(p, otn->sigInfo.message, rtn->listhead, event);

    if (otn->OTN_activation_ptr == NULL)
    {
        LogMessage("WARNING: an activation rule with no "
                "dynamic rules matched!\n");
        return 0;
    }

    otn->OTN_activation_ptr->active_flag = 1;
    otn->OTN_activation_ptr->countdown = 
        otn->OTN_activation_ptr->activation_counter;

    otn->RTN_activation_ptr->active_flag = 1;
    otn->RTN_activation_ptr->countdown += 
        otn->OTN_activation_ptr->activation_counter;

    snort_conf->active_dynamic_nodes++;
    DEBUG_WRAP(DebugMessage(DEBUG_DETECT,"   => Finishing activation packet!\n"););
    
    CallLogFuncs(p, otn->sigInfo.message, rtn->listhead, event);
    DEBUG_WRAP(DebugMessage(DEBUG_DETECT, 
                "   => Activation packet finished, returning!\n"););

    return 1;
}

int AlertAction(Packet * p, OptTreeNode * otn, Event *event)
{
    RuleTreeNode *rtn = getRuntimeRtnFromOtn(otn);

    DEBUG_WRAP(DebugMessage(DEBUG_DETECT,
                "        <!!> Generating alert! \"%s\", policyId %d\n", otn->sigInfo.message, getRuntimePolicy()););

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

int DropAction(Packet * p, OptTreeNode * otn, Event *event)
{
    RuleTreeNode *rtn = getRuntimeRtnFromOtn(otn);

    DEBUG_WRAP(DebugMessage(DEBUG_DETECT,
               "        <!!> Generating Alert and dropping! \"%s\"\n",
               otn->sigInfo.message););
    
    if(stream_api && !stream_api->alert_inline_midstream_drops())
    {
        if(stream_api->get_session_flags(p->ssnptr) & SSNFLAG_MIDSTREAM) 
        {
            DEBUG_WRAP(DebugMessage(DEBUG_DETECT,
                " <!!> Alert Came From Midstream Session Silently Drop! "
                "\"%s\"\n", otn->sigInfo.message);); 

            Active_DropSession();
            return 1;
        }
    }

    /*
    **  Set packet flag so output plugins will know we dropped the
    **  packet we just logged.
    */
    Active_DropSession();

    CallAlertFuncs(p, otn->sigInfo.message, rtn->listhead, event);

    CallLogFuncs(p, otn->sigInfo.message, rtn->listhead, event);

    return 1;
}

int SDropAction(Packet * p, OptTreeNode * otn, Event *event)
{
    DEBUG_WRAP(DebugMessage(DEBUG_DETECT,
               "        <!!> Dropping without Alerting! \"%s\"\n",
               otn->sigInfo.message););

    // Let's silently drop the packet
    Active_DropSession();

    return 1;
}

int DynamicAction(Packet * p, OptTreeNode * otn, Event *event)
{
    RuleTreeNode *rtn = getRuntimeRtnFromOtn(otn);

    DEBUG_WRAP(DebugMessage(DEBUG_DETECT, "   => Logging packet data and"
                            " adjusting dynamic counts (%d/%d)...\n",
                            rtn->countdown, otn->countdown););

    CallLogFuncs(p, otn->sigInfo.message, rtn->listhead, event);

    otn->countdown--;

    if( otn->countdown <= 0 )
    {
        otn->active_flag = 0;
        snort_conf->active_dynamic_nodes--;
        DEBUG_WRAP(DebugMessage(DEBUG_DETECT, "   <!!> Shutting down dynamic OTN node\n"););
    }
    
    rtn->countdown--;

    if( rtn->countdown <= 0 )
    {
        rtn->active_flag = 0;
        DEBUG_WRAP(DebugMessage(DEBUG_DETECT, "   <!!> Shutting down dynamic RTN node\n"););
    }

    return 1;
}

int LogAction(Packet * p, OptTreeNode * otn, Event *event)
{
    RuleTreeNode *rtn = getRuntimeRtnFromOtn(otn);

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

