/*
**  $Id$
**
**  fpdetect.c
**
**  Copyright (C) 2002-2011 Sourcefire, Inc.
**  Author(s):  Dan Roelker <droelker@sourcefire.com>
**              Marc Norton <mnorton@sourcefire.com>
**              Andrew R. Baker <andrewb@snort.org>
**              Andrew J. Mullican <amullican@sourcefire.com>
**              Steven Sturges <ssturges@sourcefire.com>
**  NOTES
**  5.15.02 - Initial Source Code. Norton/Roelker
**  2002-12-06 - Modify event selection logic to fix broken custom rule types
**               arbitrary rule type ordering (ARB)
**  2005-02-08 - Track alerts per session so that they aren't double reported
**               for rebuilt packets.  AJM.
**  2005-02-17 - Track alerts per IP frag tracker so that they aren't double
**               reported for rebuilt frags.  SAS (code similar to AJM's for
**               per session tracking).
**
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License Version 2 as
**  published by the Free Software Foundation.  You may not use, modify or
**  distribute this program under any other version of the GNU General
**  Public License.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
**
*/
#include "snort.h"
#include "detect.h"
#include "debug.h"
#include "util.h"
#include "tag.h"
#include "rules.h"
#include "treenodes.h"
#include "pcrm.h"
#include "fpcreate.h"
#include "fpdetect.h"
#include "mpse.h"
#include "bitop.h"
#include "perf-event.h"
#include "sfthreshold.h"
#include "rate_filter.h"
#include "event_queue.h"
#include "active.h"

#include "sp_pattern_match.h"
#include "spp_frag3.h"
#include "stream_api.h"
#ifdef TARGET_BASED
#include "target-based/sftarget_protocol_reference.h"
#include "target-based/sftarget_reader.h"
#endif

#include "ppm.h"
#include "sfPolicy.h"
#include "generators.h"
#include "detection_util.h"

/*
**  This define enables set-wise signature detection for
**  IP and ICMP packets.  During early testing, the old
**  method of detection seemed faster for ICMP and IP 
**  signatures, but with modifications to the set-wise engine
**  performance became much better.  This define could be
**  taken out, but is still in for regression testing.
*/

/*
**  GLOBALS
**  These variables are local to this file and deal with
**  configuration issues that are set in snort.conf through
**  variables.
*/

/*
**  Assorted global variables from the old detection engine
**  for backwards compatibility.
*/
extern uint16_t event_id;
extern OptTreeNode *otn_tmp;
extern SFEVENT sfEvent;

/*
**  Static function prototypes
*/
int fpEvalRTN(RuleTreeNode *rtn, Packet *p, int check_ports);
static INLINE int fpEvalHeaderIp(Packet *p, int ip_proto, OTNX_MATCH_DATA *);
static INLINE int fpEvalHeaderIcmp(Packet *p, OTNX_MATCH_DATA *);
static INLINE int fpEvalHeaderTcp(Packet *p, OTNX_MATCH_DATA *);
static INLINE int fpEvalHeaderUdp(Packet *p, OTNX_MATCH_DATA *);
static INLINE int fpEvalHeaderSW(PORT_GROUP *port_group, Packet *p, 
                                 int check_ports, char ip_rule, OTNX_MATCH_DATA *);
static int rule_tree_match (void* id, void * tree, int index, void * data, void *neg_list );
int fpAddMatch( OTNX_MATCH_DATA *omd_local, OTNX *otnx, int pLen, OptTreeNode *otn);
static INLINE int fpAddSessionAlert(Packet *p, OptTreeNode *otn);
static INLINE int fpSessionAlerted(Packet *p, OptTreeNode *otn);
        
//static INLINE int fpLogEvent(RuleTreeNode *rtn, OptTreeNode *otn, Packet *p);

#ifdef PERF_PROFILING
PreprocStats rulePerfStats;
PreprocStats ncrulePerfStats;
PreprocStats ruleRTNEvalPerfStats;
PreprocStats ruleOTNEvalPerfStats;
#endif

/* initialize the global OTNX_MATCH_DATA variable */
OTNX_MATCH_DATA * OtnXMatchDataNew(int num_rule_types)
{
    OTNX_MATCH_DATA *omd = (OTNX_MATCH_DATA *)SnortAlloc(sizeof(OTNX_MATCH_DATA));

    omd->iMatchInfoArraySize = num_rule_types;
    omd->matchInfo = (MATCH_INFO *)SnortAlloc(num_rule_types * sizeof(MATCH_INFO));

    return omd;
}

/*
**
**  NAME
**    InitMatchInfo::
**
**  DESCRIPTION
**    Initialize the OTNX_MATCH_DATA structure.  We do this for
**    every packet so calloc is not used as this would zero the
**    whole space and this only sets the necessary counters to
**    zero, and saves us time.
**
**  FORMAL INPUTS
**    OTNX_MATCH_DATA * - pointer to structure to init.
**
**  FORMAL OUTPUT
**    None
**
*/
static INLINE void InitMatchInfo(OTNX_MATCH_DATA *o)
{
    int i = 0;
    int total = 0;

    EventQueueConfig *eq = snort_conf->event_queue_config;

    for(i = 0; i < o->iMatchInfoArraySize; i++)
    {
        total += o->matchInfo[i].iMatchCount;
        o->matchInfo[i].iMatchCount  = 0;
        o->matchInfo[i].iMatchIndex  = 0;
        o->matchInfo[i].iMatchMaxLen = 0;
    }
    total -= eq->log_events;

    if ( total > 0 )
        pc.log_limit += total;
}

void OtnxMatchDataFree(OTNX_MATCH_DATA *omd)
{
    if (omd == NULL)
        return;

    if ( snort_conf && snort_conf->event_queue_config )
        InitMatchInfo(omd);

    if (omd->matchInfo != NULL)
        free(omd->matchInfo);

    free(omd);
}

// called by fpLogEvent(), which does the filtering etc.
// this handles the non-rule-actions (responses and tagging).
static INLINE void fpLogOther (Packet* p, OptTreeNode* otn, int action)
{
    TriggerResponses(p, otn);
    SetTags(p, otn, event_id);
}

/*
**
**  NAME
**    fpLogEvent::
**
**  DESCRIPTION
**    This function takes the corresponding RTN and OTN for a snort rule
**    and logs the event and packet that was alerted upon.  This 
**    function was pulled out of fpEvalSomething, so now we can log an
**    event no matter where we are.
**
**  FORMAL INPUTS
**    RuleTreeNode * - rtn for snort rule
**    OptTreeNode  * - otn for snort rule
**    Packet       * - packet that elicited event.
*/
int fpLogEvent(RuleTreeNode *rtn, OptTreeNode *otn, Packet *p)
{
    int action = -1, rateAction = -1;
    int override, filterEvent = 0;

    if (!rtn || !otn)
    {
        return 1;
    }
    
    DEBUG_WRAP(DebugMessage(DEBUG_DETECT,
                "   => Got rule match, rtn type = %d\n",
                rtn->type););

    if ( otn->stateless )
    {
        /* Stateless rule, set the stateless bit */
        p->packet_flags |= PKT_STATELESS;
    }
    else
    {
        /* Not stateless, clear the stateless bit if it was set
         * from a previous rule.
         */
        p->packet_flags &= ~PKT_STATELESS;
    }

    if ((p->packet_flags & PKT_STREAM_UNEST_UNI) &&
        ScAssureEstablished() &&
        (!(p->packet_flags & PKT_REBUILT_STREAM)) &&
        (otn->stateless == 0))
    {
        // We still want to drop packets that are drop rules.
        // We just don't want to see the alert.
        if ( (rtn->type == RULE_TYPE__DROP) ||
             (rtn->type == RULE_TYPE__SDROP) ||
             (rtn->type == RULE_TYPE__REJECT) )
        {
            Active_DropSession();
        }
        fpLogOther(p, otn, rtn->type);
        return 1;
    }

    // perform rate filtering tests - impacts action taken
    rateAction = RateFilter_Test(otn, p);
    override = ( rateAction >= RULE_TYPE__MAX );
    if ( override ) rateAction -= RULE_TYPE__MAX;

    // internal events are no-ops
    if ( (rateAction < 0) && EventIsInternal(otn->sigInfo.generator) )
    {
        return 1;
    }
    action = (rateAction < 0) ? (int)rtn->type : rateAction;

    // When rate filters kick in, event filters are still processed.
    // perform event filtering tests - impacts logging
    if ( IPH_IS_VALID(p) )
    {
        filterEvent = sfthreshold_test(
            otn->event_data.sig_generator,
            otn->event_data.sig_id,
            GET_SRC_IP(p), GET_DST_IP(p),
            p->pkth->ts.tv_sec);
    }
    else
    {
        snort_ip cleared;
        IP_CLEAR(cleared);

        filterEvent = sfthreshold_test(
            otn->event_data.sig_generator,
            otn->event_data.sig_id,
            IP_ARG(cleared), IP_ARG(cleared),
            p->pkth->ts.tv_sec);
    }

    if ( (filterEvent < 0) || (filterEvent > 0 && !override) )
    {
        /*
        **  If InlineMode is on, then we still want to drop packets
        **  that are drop rules.  We just don't want to see the alert.
        */
        if ( (action == RULE_TYPE__DROP) ||
             (action == RULE_TYPE__SDROP) ||
             (action == RULE_TYPE__REJECT) )
        {
            Active_DropSession();
        }
        pc.event_limit++;
        fpLogOther(p, otn, action);
        return 1;
    }

    //  Set the ref_time to 0 so we make the logging work right.
    otn->event_data.ref_time.tv_sec = 0;
    
    /*  Set otn_tmp because log.c uses it to log details
    **  of the event.  Maybe we should look into making this
    **  part of the log routines and not a global variable.
    **  This way we could support multiple events per packet.
    */
    otn_tmp = otn;
    OTN_PROFILE_ALERT(otn);

    event_id++;

    switch (action)
    {
        case RULE_TYPE__PASS:
            PassAction();
            break;

        case RULE_TYPE__ACTIVATE:
            ActivateAction(p, otn, &otn->event_data);
            break;

        case RULE_TYPE__ALERT:
            AlertAction(p, otn, &otn->event_data);
            break;

        case RULE_TYPE__DYNAMIC:
            DynamicAction(p, otn, &otn->event_data);
            break;

        case RULE_TYPE__LOG:
            LogAction(p, otn, &otn->event_data);
            break;

        case RULE_TYPE__DROP:
            DropAction(p, otn, &otn->event_data);
            break;

        case RULE_TYPE__SDROP:
            SDropAction(p, otn, &otn->event_data);
            break;

        case RULE_TYPE__REJECT:
            DropAction(p, otn, &otn->event_data);
            break;

        default:
            break;
    }

    otn_tmp = NULL;
    fpLogOther(p, otn, action);
    return 0;
}

/*
**
**  NAME
**    fpAddMatch::
**
**  DESCRIPTION
**    Add and Event to the appropriate Match Queue: Alert, Pass, or Log.
**    This allows us to find multiple events per packet and pick the 'best'
**    one.  This function also allows us to change the order of alert,
**    pass, and log signatures by cacheing them for decision later.
**
**    IMPORTANT NOTE:
**    fpAddMatch must be called even when the queue has been maxed
**    out.  This is because there are three different queues (alert,
**    pass, log) and unless all three are filled (or at least the 
**    queue that is in the highest priority), events must be looked
**    at to see if they are members of a queue that is not maxed out.
**
**  FORMAL INPUTS
**    OTNX_MATCH_DATA    * - the omd to add the event to.
**    OTNX               * - the otnx to add.
**    int pLen             - length of pattern that matched, 0 for no content
**    OptTreeNode        * - the otn to add.
**
**  FORMAL OUTPUTS
**    int - 1 max_events variable hit, 0 successful.
**
*/
int fpAddMatch(OTNX_MATCH_DATA *omd_local, OTNX *otnx, int pLen,
               OptTreeNode *otn)
{
    MATCH_INFO * pmi;
    int evalIndex;
    int i;
    RuleTreeNode *rtn = getRuntimeRtnFromOtn(otn);

    evalIndex = rtn->listhead->ruleListNode->evalIndex;
    
    /* bounds check index */
    if( evalIndex >= omd_local->iMatchInfoArraySize )
    {
        pc.match_limit++;
        return 1;
    }
    pmi = &omd_local->matchInfo[evalIndex];

    /*
    **  If we hit the max number of unique events for any rule type alert,
    **  log or pass, then we don't add it to the list.
    */
    if( pmi->iMatchCount >= (int)snort_conf->fast_pattern_config->max_queue_events || 
        pmi->iMatchCount >= MAX_EVENT_MATCH)
    {
        pc.match_limit++;
        return 1;
    }

    /* Check that we are not storing the same otn again */
    for( i=0; i< pmi->iMatchCount;i++ )
    {
        if( pmi->MatchArray[ i  ] == otn )
        {
            //LogMessage("fpAddMatch: storing the same otn...\n");
            return 0;
        }
    }

    /*
    **  Add the event to the appropriate list
    */
    pmi->MatchArray[ pmi->iMatchCount ] = otn;

    /*
    **  This means that we are adding a NC rule
    **  and we only set the index to this rule
    **  if there is no content rules in the
    **  same array.
    */
    if(pLen > 0)
    {
        /*
        **  Event Comparison Function
        **  Here the largest content match is the
        **  priority
        */
        if( pmi->iMatchMaxLen < pLen )
        {
            pmi->iMatchMaxLen = pLen;
            pmi->iMatchIndex  = pmi->iMatchCount;
        }
    }

    pmi->iMatchCount++;
  
    return 0;
}

/*
**
**  NAME
**    fpEvalRTN::
**
**  DESCRIPTION
**    Evaluates an RTN against a packet.  We can probably get rid of
**    the check_ports variable, but it's in there for good luck.  :)
**
**  FORMAL INPUTS
**    RuleTreeNode * - RTN to check packet against.
**    Packet       * - Packet to evaluate
**    int            - whether to do a quick enhancement against ports.
**
**  FORMAL OUTPUT
**    int - 1 if match, 0 if match failed.
**
*/
int fpEvalRTN(RuleTreeNode *rtn, Packet *p, int check_ports)
{
    PROFILE_VARS;

    PREPROC_PROFILE_START(ruleRTNEvalPerfStats);

    if(rtn == NULL)
    {
        PREPROC_PROFILE_END(ruleRTNEvalPerfStats);
        return 0;
    }

    /* TODO: maybe add a port test here ... */

    if (rtn->type == RULE_TYPE__DYNAMIC)
    {
        if (!snort_conf->active_dynamic_nodes)
        {
            PREPROC_PROFILE_END(ruleRTNEvalPerfStats);
            return 0;
        }

        if(rtn->active_flag == 0)
        {
            PREPROC_PROFILE_END(ruleRTNEvalPerfStats);
            return 0;
        }
    }

    DEBUG_WRAP(DebugMessage(DEBUG_DETECT, "[*] Rule Head %d\n", 
                rtn->head_node_number);)

    if(!rtn->rule_func->RuleHeadFunc(p, rtn, rtn->rule_func, check_ports))
    {
        DEBUG_WRAP(DebugMessage(DEBUG_DETECT,
                    "   => Header check failed, checking next node\n"););
        DEBUG_WRAP(DebugMessage(DEBUG_DETECT, 
                    "   => returned from next node check\n"););
        PREPROC_PROFILE_END(ruleRTNEvalPerfStats);
        return 0;
    }

    DEBUG_WRAP(DebugMessage(DEBUG_DETECT, 
             "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n"););
    DEBUG_WRAP(DebugMessage(DEBUG_DETECT, "   => RTN %d Matched!\n",
                rtn->head_node_number););
    DEBUG_WRAP(DebugMessage(DEBUG_DETECT, 
            "vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n"););
    /*
    **  Return that there is a rule match and log the event outside
    **  of this routine.
    */
    PREPROC_PROFILE_END(ruleRTNEvalPerfStats);
    return 1;
}

static int detection_option_tree_evaluate(detection_option_tree_root_t *root, detection_option_eval_data_t *eval_data)
{
    int i, rval = 0;
    PROFILE_VARS;

    if (!root)
        return 0;

    PREPROC_PROFILE_START(ruleOTNEvalPerfStats); /* Not really OTN, but close */

#ifdef PPM_MGR
    /* Start Rule Timer */
    if( PPM_RULES_ENABLED() )
    {
        PPM_GET_TIME();
        PPM_INIT_RULE_TIMER();

        if (root->tree_state == RULE_STATE_DISABLED)
        {
            PPM_REENABLE_TREE(root, eval_data->p);

            if (root->tree_state == RULE_STATE_DISABLED)
            {
                PPM_END_RULE_TIMER();
                return 0;
            }
        }
    }
#endif

    for ( i = 0; i< root->num_children; i++)
    {
        /* New tree, reset doe_ptr for safety */
        UpdateDoePtr(NULL, 0);

        /* Increment number of events generated from that child */
        rval += detection_option_node_evaluate(root->children[i], eval_data);
    }

#ifdef PPM_MGR
    if( PPM_ENABLED() )
    {
        PPM_GET_TIME();

        /* Rule test */
        if( PPM_RULES_ENABLED() )
        {
            if( PPM_PKTS_ENABLED() )
                PPM_INC_PKT_RULE_TESTS();

            PPM_RULE_TEST(root, eval_data->p);
            PPM_ACCUM_RULE_TIME();
            PPM_END_RULE_TIMER();
        }
    }
#endif

    PREPROC_PROFILE_END(ruleOTNEvalPerfStats);
    return rval;
}

static int rule_tree_match( void * id, void *tree, int index, void * data, void * neg_list)
{
    OTNX_MATCH_DATA  *pomd   = (OTNX_MATCH_DATA *)data;
    PMX              *pmx    = (PMX*)id;
    RULE_NODE        *rnNode = (RULE_NODE*)(pmx->RuleNode);
    OTNX             *otnx   = (OTNX*)(rnNode->rnRuleData);
    PatternMatchData *pmd    = (PatternMatchData*)pmx->PatternMatchData;
    detection_option_tree_root_t *root = (detection_option_tree_root_t *)tree;
    detection_option_eval_data_t eval_data;
    NCListNode *ncl;
    int               rval=0;
    PROFILE_VARS;

    eval_data.pomd = pomd;
    eval_data.otnx = otnx;
    eval_data.p = pomd->p;
    eval_data.pmd = pmd;
    eval_data.flowbit_failed = 0;
    eval_data.flowbit_noalert = 0;

    PREPROC_PROFILE_START(rulePerfStats);

    /* NOTE: The otn will be the first one in the match state. If there are
     * multiple rules associated with a match state, mucking with the otn
     * may muck with an unintended rule */

    /* Set flag for not contents so they aren't evaluated */
    for (ncl = (NCListNode *)neg_list; ncl != NULL; ncl = ncl->next)
    {
        PMX *neg_pmx = (PMX *)ncl->pmx;
        PatternMatchData *neg_pmd = (PatternMatchData *)neg_pmx->PatternMatchData;

        neg_pmd->last_check.ts.tv_sec = eval_data.p->pkth->ts.tv_sec;
        neg_pmd->last_check.ts.tv_usec = eval_data.p->pkth->ts.tv_usec;
        neg_pmd->last_check.packet_number = rule_eval_pkt_count;
        neg_pmd->last_check.rebuild_flag = (eval_data.p->packet_flags & REBUILD_FLAGS);
    }

    rval = detection_option_tree_evaluate(root, &eval_data);
    if (rval)
    {
        /*
        **  We have a qualified event from this tree 
        */
        pomd->pg->pgQEvents++;
        UpdateQEvents(&sfEvent);
    }
    else
    {
        /*
        ** This means that the event is non-qualified.
        */
        pomd->pg->pgNQEvents++;
        UpdateNQEvents(&sfEvent);
    }

    PREPROC_PROFILE_END(rulePerfStats);
    if (eval_data.flowbit_failed)
    {
        return -1;
    }
    
#ifdef GRE
    /* If this is for an IP rule set, evalute the rules from
     * the inner IP offset as well */
    if (eval_data.p->packet_flags & PKT_IP_RULE)
    {
        if (eval_data.p->outer_ip_data)
        {
            const uint8_t *tmp_data = eval_data.p->data;
            uint16_t tmp_dsize = eval_data.p->dsize;
            void *tmp_iph = (void *)eval_data.p->iph;
#ifdef SUP_IP6
            void *tmp_ip4h = (void *)eval_data.p->ip4h;
            void *tmp_ip6h = (void *)eval_data.p->ip6h;
#endif
            eval_data.p->iph = eval_data.p->inner_iph;
#ifdef SUP_IP6
            eval_data.p->ip4h = &eval_data.p->inner_ip4h;
            eval_data.p->ip6h = &eval_data.p->inner_ip6h;
#endif
            eval_data.p->data = eval_data.p->ip_data;
            eval_data.p->dsize = eval_data.p->ip_dsize;

            /* clear so we dont keep recursing */
            eval_data.p->packet_flags &= ~PKT_IP_RULE; 
            eval_data.p->packet_flags |= PKT_IP_RULE_2ND;
            
            /* Recurse, and evaluate with the inner IP */
            rval = rule_tree_match(id, tree, index, data, NULL);

            eval_data.p->packet_flags &= ~PKT_IP_RULE_2ND;
            eval_data.p->packet_flags |= PKT_IP_RULE;

            /* restore original data & dsize */
            eval_data.p->iph = tmp_iph;
#ifdef SUP_IP6
            eval_data.p->ip4h = (IP4Hdr*)tmp_ip4h;
            eval_data.p->ip6h = (IP6Hdr*)tmp_ip6h;
#endif
            eval_data.p->data = tmp_data;
            eval_data.p->dsize = tmp_dsize;
        }
    }
#endif
    return 0;
}

static int sortOrderByPriority(const void *e1, const void *e2)
{
    OptTreeNode *otn1;
    OptTreeNode *otn2;

    if (!e1 || !e2)
        return 0;

    otn1 = *(OptTreeNode **)e1;
    otn2 = *(OptTreeNode **)e2;

    if( otn1->sigInfo.priority < otn2->sigInfo.priority )
        return +1;

    if( otn1->sigInfo.priority > otn2->sigInfo.priority )
        return -1;

    /* This improves stability of repeated tests */
    if( otn1->sigInfo.id < otn2->sigInfo.id )
        return +1;

    if( otn1->sigInfo.id > otn2->sigInfo.id )
        return -1;

    return 0;
}

static int sortOrderByContentLength(const void *e1, const void *e2)
{
    OptTreeNode *otn1;
    OptTreeNode *otn2;

    if (!e1 || !e2)
        return 0;

    otn1 = *(OptTreeNode **)e1;
    otn2 = *(OptTreeNode **)e2;

    /**** XXX: TODO for RULE_OPTION_TREE */
    return 0;
}


/*
**
**  NAME
**    fpFinalSelectEvent::
**
**  DESCRIPTION
**    fpFinalSelectEvent is called at the end of packet processing
**    to decide, if there hasn't already been a selection, to decide
**    what event to select.  This function is different from 
**    fpSelectEvent by the fact that fpSelectEvent only selects an
**    event if it is the first priority setting (drop/pass/alert...).
**
**    We also loop through the events we log, so that we don't log the
**    same event twice.  This can happen with unique conflicts some
**    of the time.
**
**    IMPORTANT NOTE:
**    We call fpFinalSelectEvent() after all processing of the packet
**    has been completed.  The reason this must be called afterwards is
**    because of unique rule group conflicts for a packet.  If there is
**    a unique conflict, then we inspect both rule groups and do the final
**    event select after both rule groups have been inspected.  The
**    problem came up with bi-directional rules with pass rule ordering
**    as the first type of rule.  Before we would detect a alert rule in
**    the first rule group, and since there was no pass rules we would
**    log that alert rule.  However, if we had inspected the second rule
**    group, we would have found a pass rule and that should have taken
**    precedence.  We now inspect both rule groups before doing a final
**    event select.
**
**    NOTES
**    Jan 2006 : marc norton
**    Previously it was possible to not log all desired events, if for
**    instance the rule order was alert->drop in inline mode we would
**    alert but no drop.  The default ordering of 'drop alert pass log ...'
**    normally handles this, however, it could happen.  Also, in the
**    default ordering alerts on the same packet a drop was applied to
**    did not get logged. To be more flexible and handle all manners of
**    subjective rule ordering and logging desired by the whole farm we've
**    changed things a bit.
**
**    Now, each actions event list is processed in order, based on the rule
**    order.  We process all events up to the log limit specified via the
**    'config event_queue: ...' as you might expect.  Pass rules are
**    handled a bit differently. As soon as a pass rule based event is
**    processed in the event queue, we stop processing any further events
**    on the packet if the pass event is the 1st ordering that sees an
**    event.  Otherwise if the ordering has it that pass rule events are
**    processed after a drop or alert you will see the drops and alerts,
**    and the pass event just causes us to stop processing any more events
**    on the packet, but the packet does not pass.  Also, the --alert-on-drop
**    flag causes any drop/sdrop/reject rules to be loaded as alert rules.
**    The default has been to ignore them on parsing.
**
**    If this is less than clear, herese the $.02 version:
**    default order -> pass drop alert log ( --alert-before-pass reverts
**    to -> drop alert pass log ) the 1st action-type of events in the rule
**    ordering to be seen gets logged by default the --flush-all-events
**    flag will cause secondary and tertiary action-events to be logged.
**    the -o flag is useless, but accepted, for now.
**    the max_events and log fields are reduced to only needing the log
**    events field. max_fields is harmless.
**    ( drop rules may be honored as alerts in IDS mode (no -Q) by using
**    the --alert-on-drop flag )
**
**  FORMAL INPUTS
**    OTNX_MATCH_DATA * - omd to select event from.
**    Packet *          - pointer to packet to log.
**
**  FORMAL OUTPUT
**    int - return 0 if no match, 1 if match.
**   
*/
static INLINE int fpFinalSelectEvent(OTNX_MATCH_DATA *o, Packet *p)
{
    int i;
    int j;
    int k;
    OptTreeNode *otn;
    int tcnt = 0;
    EventQueueConfig *eq = snort_conf->event_queue_config;
    RuleTreeNode *rtn;

    for( i = 0; i < o->iMatchInfoArraySize; i++ )
    {
        /* bail if were not dumping events in all the action groups,
         * and we've alresady got some events */
        if (!ScProcessAllEvents() && (tcnt > 0))
            return 1;

        if(o->matchInfo[i].iMatchCount)
        {
            /*
             * We must always sort so if we que 8 and log 3 and they are
             * all from the same action group we want them sorted so we get
             * the highest 3 in priority, priority and lenght sort do NOT
             * take precedence over 'alert drop pass ...' ordering.  If
             * order is 'drop alert', and we log 3 for drop alertsdo not
             * get logged.  IF order is 'alert drop', and we log 3 for
             * alert, than no drops are logged.  So, there should be a
             * built in drop/sdrop/reject comes before alert/pass/log as
             * part of the natural ordering....Jan '06..
             */
            /* Sort the rules in this action group */
            if (eq->order == SNORT_EVENTQ_PRIORITY)
            {
                qsort(o->matchInfo[i].MatchArray, o->matchInfo[i].iMatchCount,
                      sizeof(void *), sortOrderByPriority);
            }
            else if (eq->order == SNORT_EVENTQ_CONTENT_LEN)
            {
                qsort(o->matchInfo[i].MatchArray, o->matchInfo[i].iMatchCount,
                      sizeof(void*), sortOrderByContentLength);
            }
            else
            {
                FatalError("fpdetect: Order function for event queue is invalid.\n");
            }

            /* Process each event in the action (alert,drop,log,...) groups */
            for(j=0; j < o->matchInfo[i].iMatchCount; j++)
            {
                otn = o->matchInfo[i].MatchArray[j];
                rtn = getRtnFromOtn(otn, getRuntimePolicy());

                if ((otn != NULL) && (rtn != NULL) && (rtn->type == RULE_TYPE__PASS))
                {
                    /* Already acted on rules, so just don't act on anymore */
                    if( tcnt > 0 )
                        return 1;
                }

                /*
                **  Loop here so we don't log the same event
                **  multiple times.
                */
                for(k = 0; k < j; k++)
                {
                    if(o->matchInfo[i].MatchArray[k] == otn)
                    {
                        otn = NULL; 
                        break;
                    }
                }

                if( otn && !fpSessionAlerted(p, otn) &&
                   !fpFragAlerted(p, otn))
                {
                    /*
                    **  QueueEvent
                    */
                    int err = SnortEventqAdd(
                                   otn->sigInfo.generator, 
                                   otn->sigInfo.id,
                                   otn->sigInfo.rev,
                                   otn->sigInfo.class_id,
                                   otn->sigInfo.priority,
                                   otn->sigInfo.message,
                                   (void *)NULL);
                    if ( err )
                        pc.queue_limit++;

                    tcnt++;
                }

                /* Only count it if we're going to log it */
                if (tcnt <= eq->log_events)
                {
                    if ( p->ssnptr )
                        fpAddSessionAlert(p, otn);

                    if ( p->fragtracker )
                        fpAddFragAlert(p, otn);
                }

                if (tcnt >= eq->max_events)
                {
                    pc.queue_limit++;
                    return 1;
                }
                /* only log/count one pass */
                if ((otn != NULL) && (rtn != NULL) && (rtn->type == RULE_TYPE__PASS))
                {
                    p->packet_flags |= PKT_PASS_RULE;
                    return 1;
                }
            }
        }
    }

    return 0;
}

/*
**  
**  NAME
**    fpAddSessionAlert::
**
**  DESCRIPTION
**    This function flags an alert per session.
**
**  FORMAL INPUTS
**    Packet *     - the packet to inspect
**    OTNX *       - the rule that generated the alert
**
**  FORMAL OUTPUTS
**    int - 0 if not flagged
**          1 if flagged
**
*/
static INLINE int fpAddSessionAlert(Packet *p, OptTreeNode *otn)
{
    if ( !p->ssnptr )
        return 0;

    if ( !otn )
        return 0;

    /* Only track a certain number of alerts per session */
    if (stream_api)
        return !stream_api->add_session_alert(p->ssnptr, p,
                                  otn->sigInfo.generator,
                                  otn->sigInfo.id);
    return 0;
}

/*
**  
**  NAME
**    fpSessionAlerted::
**
**  DESCRIPTION
**    This function indicates whether or not an alert has been generated previously
**    in this session, but only if this is a rebuilt packet.
**
**  FORMAL INPUTS
**    Packet *     - the packet to inspect
**    OTNX *       - the rule that generated the alert
**
**  FORMAL OUTPUTS
**    int - 0 if alert NOT previously generated
**          1 if alert previously generated
**
*/
static INLINE int fpSessionAlerted(Packet *p, OptTreeNode *otn)
{
    SigInfo *si = &otn->sigInfo;

    if ( !stream_api )
        return 0;

    if (!stream_api->check_session_alerted(p->ssnptr, p, si->generator, si->id))
        return 0;
    else
        return 1;

}

#if 0
Not currently used
/*
 * Prints an OTN in a simple format with:
 *
 * rule proto: # gid: # sid: # sp: # dp # \n
 */
void printRuleFmt1( OptTreeNode * otn )
{
    RuleTreeNode *rtn = getParserRtnFromOtn(otn);

    LogMessage("rule proto: ");        

    if(      rtn->proto== IPPROTO_TCP     )LogMessage("tcp  ");
    else if( rtn->proto== IPPROTO_UDP     )LogMessage("udp  ");
    else if( rtn->proto== IPPROTO_ICMP    )LogMessage("icmp ");
    else if( rtn->proto== ETHERNET_TYPE_IP)LogMessage("ip   ");
   
    LogMessage("gid:%u sid:%5u ", otn->sigInfo.generator,otn->sigInfo.id);
    
    LogMessage(" sp:");
    
    fflush(stdout);fflush(stderr);
    PortObjectPrintPortsRaw(rtn->src_portobject);
    fflush(stdout);fflush(stderr);
   
    LogMessage(" dp:");
            
    PortObjectPrintPortsRaw(rtn->dst_portobject);
    printf("\n");
    fflush(stdout);fflush(stderr);
}
#endif

/*
**  
**  NAME
**    fpEvalHeaderSW::
**
**  DESCRIPTION
**    This function does a set-wise match on content, and walks an otn list
**    for non-content.  The otn list search will eventually be redone for 
**    for performance purposes.
**
**  FORMAL INPUTS
**    PORT_GROUP * - the port group to inspect
**    Packet *     - the packet to inspect
**    int          - whether src/dst ports should be checked (udp/tcp or icmp)
**    char         - whether the rule is an IP rule (change the packet payload pointer)
**
**  FORMAL OUTPUTS
**    int - 0 for failed pattern match
**          1 for sucessful pattern match
**
*/
static INLINE int fpEvalHeaderSW(PORT_GROUP *port_group, Packet *p,
        int check_ports, char ip_rule, OTNX_MATCH_DATA *omd)
{
    RULE_NODE *rnWalk;
    void * so;
    int start_state;
    const uint8_t *tmp_payload = p->data;
    uint16_t tmp_dsize = p->dsize;
    void *tmp_iph = (void *)p->iph;
#ifdef SUP_IP6
    void *tmp_ip6h = (void *)p->ip6h;
    void *tmp_ip4h = (void *)p->ip4h;
#endif
    char repeat = 0;
    FastPatternConfig *fp = snort_conf->fast_pattern_config;
    PROFILE_VARS;

    if (ip_rule)
    {
        /* Set the packet payload pointers to that of IP,
         ** since this is an IP rule. */
#ifdef GRE
        if (p->outer_ip_data)
        {
            p->iph = p->outer_iph;
# ifdef SUP_IP6
            p->ip6h = &p->outer_ip6h;
            p->ip4h = &p->outer_ip4h;
# endif
            p->data = p->outer_ip_data;
            p->dsize = p->outer_ip_dsize;
            p->packet_flags |= PKT_IP_RULE;
            repeat = 2;
        }
        else
#endif  /* GRE */
        {
            if (p->ip_data)
            {
                p->data = p->ip_data;
                p->dsize = p->ip_dsize;
                p->packet_flags |= PKT_IP_RULE;
            }
        }
    }
    else
    {
        p->packet_flags &= ~PKT_IP_RULE;
    }

    /*
     **  Init the info for rule ordering selection
     */
    //InitMatchInfo(omd);

    if (do_detect_content)
    {
        /*
         **  PKT_STREAM_INSERT packets are being rebuilt and re-injected
         **  through this detection engine.  So in order to avoid pattern
         **  matching bytes twice, we wait until the PKT_STREAM_INSERT 
         **  packets are rebuilt and injected through the detection engine.
         **
         **  PROBLEM:
         **  If a stream gets stomped on before it gets re-injected, an attack
         **  would be missed.  So before a connection gets stomped, we 
         **  re-inject the stream we have.
         */

        /*
         **  First evaluate the detection functions.  Namely those things
         **  that are between a preprocessor and rules.
         */
        {
            tSfPolicyId policy_id = getRuntimePolicy();
            SnortPolicy *policy = snort_conf->targeted_policies[policy_id];
            /* safe to assume policy is non NULL here because of check in
             * Preprocess() */
            DetectionEvalFuncNode *idx = policy->detect_eval_funcs;

            for (; (idx != NULL) && !(p->packet_flags & PKT_PASS_RULE); idx = idx->next)
            {
                if ((p->proto_bits & idx->proto_mask) || (idx->proto_mask == PROTO_BIT__ALL))
                    //IsDetectBitSet(p, idx->preproc_bit))
                {
                    idx->func(p, idx->context);
                }
            }
        }

        if (fp->inspect_stream_insert || !(p->packet_flags & PKT_STREAM_INSERT))
        {
            omd->pg = port_group;
            omd->p = p;
            omd->check_ports = check_ports;

            /*
             **   Uri-Content Match
             **   This check indicates that http_decode found
             **   at least one uri
             */
            if (p->uri_count > 0)
            {
                int i;

                for (i = HTTP_BUFFER_URI; (i < p->uri_count) && (i <= HTTP_BUFFER_METHOD); i++)
                {
                    if ((UriBufs[i].uri == NULL) || (UriBufs[i].length == 0))
                        continue;

                    switch (i)
                    {
                        case HTTP_BUFFER_URI:
                            so = (void *)port_group->pgPms[PM_TYPE__HTTP_URI_CONTENT];
                            break;
                        case HTTP_BUFFER_HEADER:
                            so = (void *)port_group->pgPms[PM_TYPE__HTTP_HEADER_CONTENT];
                            break;
                        case HTTP_BUFFER_CLIENT_BODY:
                            so = (void *)port_group->pgPms[PM_TYPE__HTTP_CLIENT_BODY_CONTENT];
                            break;
                        case HTTP_BUFFER_METHOD:
                            so = (void *)port_group->pgPms[PM_TYPE__HTTP_METHOD_CONTENT];
                            break;
                        default:
                            so = NULL;
                            break;
                    }

                    if ((so != NULL) && (mpseGetPatternCount(so) > 0))
                    {
                        start_state = 0;
                        mpseSearch(so, UriBufs[i].uri, UriBufs[i].length, 
                                rule_tree_match, omd, &start_state);
#ifdef PPM_MGR
                        /* Bail if we spent too much time already */
                        if (PPM_PACKET_ABORT_FLAG())
                            goto fp_eval_header_sw_reset_ip;
#endif
                    }
                }
            }

            /*
             **  Decode Content Match
             **  We check to see if the packet has been normalized into
             **  the global (decode.c) DecodeBuffer.  Currently, only
             **  telnet normalization writes to this buffer.  So, if
             **  it is set, we do this the match against the normalized
             **  buffer and we do the check against the original 
             **  payload, in case any of the rules have the 
             **  'rawbytes' option.
             */
            so = (void *)port_group->pgPms[PM_TYPE__CONTENT];
            if ((so != NULL) && (mpseGetPatternCount(so) > 0))
            {
                if ((p->packet_flags & PKT_ALT_DECODE) && DecodeBuffer.len)
                {
                    start_state = 0;
                    mpseSearch(so, DecodeBuffer.data, DecodeBuffer.len, 
                            rule_tree_match, omd, &start_state);
#ifdef PPM_MGR
                    /* Bail if we spent too much time already */
                    if (PPM_PACKET_ABORT_FLAG())
                        goto fp_eval_header_sw_reset_ip;
#endif
                }

                 /*
                 **  Content-Match - If no Uri-Content matches, than do a Content search
                 **
                 **  NOTE:
                 **    We may want to bail after the Content search if there
                 **    has been a successful match.
                 */
                if (p->data && p->dsize)
                {
                    uint16_t pattern_match_size = p->dsize;

                    if ( IsLimitedDetect(p) && (p->alt_dsize < p->dsize) )
                        pattern_match_size = p->alt_dsize;

                    start_state = 0;
                    mpseSearch(so, p->data, pattern_match_size,
                            rule_tree_match, omd, &start_state);
#ifdef PPM_MGR
                    /* Bail if we spent too much time already */
                    if (PPM_PACKET_ABORT_FLAG())
                        goto fp_eval_header_sw_reset_ip;
#endif
                }
            }
        }
    }

    /*
     **  PKT_REBUILT_STREAM packets are re-injected streams.  This means
     **  that the "packet headers" are completely bogus and only the 
     **  content matches are important.  So for PKT_REBUILT_STREAMs, we
     **  don't inspect against no-content OTNs since these deal with 
     **  packet headers, packet sizes, etc.
     **
     **  NOTE:
     **  This has been changed when evaluating no-content rules because
     **  it was interfering with the pass->alert ordering.  We still
     **  need to check no-contents against rebuilt packets, because of
     **  this problem.  Immediate solution is to have the detection plugins
     **  bail if the rule should only be inspected against packets, a.k.a
     **  dsize checks.
     */

    /*
     **  Walk and test the non-content OTNs
     */
    if (fpDetectGetDebugPrintNcRules(fp))
        LogMessage("NC-testing %u rules\n", port_group->pgNoContentCount);

#ifdef PPM_MGR
    if( PPM_ENABLED() )
        PPM_GET_TIME();
#endif

    do
    {
        if (port_group->pgHeadNC)
        {
            detection_option_eval_data_t eval_data;
            int rval;

            rnWalk = port_group->pgHeadNC;
            eval_data.pomd = omd;
            eval_data.otnx = rnWalk->rnRuleData;
            eval_data.p = p;
            eval_data.pmd = NULL;
            eval_data.flowbit_failed = 0;
            eval_data.flowbit_noalert = 0;

            PREPROC_PROFILE_START(ncrulePerfStats);
            rval = detection_option_tree_evaluate(port_group->pgNonContentTree, &eval_data);
            PREPROC_PROFILE_END(ncrulePerfStats);

            if (rval)
            {
                /* We have a qualified event from this tree */
                port_group->pgQEvents++;
                UpdateQEvents(&sfEvent);
            }
            else
            {
                /* This means that the event is non-qualified. */
                port_group->pgNQEvents++;
                UpdateNQEvents(&sfEvent);
            }
        }

#ifdef GRE
        if (ip_rule && p->outer_ip_data)
        {
            /* Evaluate again with the inner IPs */
            p->iph = p->inner_iph;
# ifdef SUP_IP6
            p->ip6h = &p->inner_ip6h;
            p->ip4h = &p->inner_ip4h;
# endif
            p->data = p->ip_data;
            p->dsize = p->ip_dsize;
            p->packet_flags |= PKT_IP_RULE_2ND | PKT_IP_RULE;
            repeat--;
        }
#else
        repeat = 0;
#endif  /* GRE */
    }
    while(repeat != 0);

#ifdef PPM_MGR  /* Tag only used with PPM right now */
fp_eval_header_sw_reset_ip:
#endif
    if (ip_rule)
    {
        /* Set the data & dsize back to original values. */
        p->iph = tmp_iph;
#ifdef SUP_IP6
        p->ip6h = (IP6Hdr *)tmp_ip6h;
        p->ip4h = (IP4Hdr *)tmp_ip4h;
#endif
        p->data = tmp_payload;
        p->dsize = tmp_dsize;
        p->packet_flags &= ~(PKT_IP_RULE| PKT_IP_RULE_2ND);
    }

    return 0;
}

/*
** fpEvalHeaderUdp::
*/
static INLINE int fpEvalHeaderUdp(Packet *p, OTNX_MATCH_DATA *omd)
{
    PORT_GROUP *src = NULL, *dst = NULL, *gen = NULL;

#ifdef TARGET_BASED
    if (IsAdaptiveConfigured(getRuntimePolicy(), 0))
    {
        /* Check for a service/protocol ordinal for this packet */
        int16_t proto_ordinal = GetProtocolReference(p);

        DEBUG_WRAP( DebugMessage(DEBUG_ATTRIBUTE,"proto_ordinal=%d\n",proto_ordinal););

        if (proto_ordinal > 0)
        {
            /* TODO:  To From Server ?, else we apply  */
            dst = fpGetServicePortGroupByOrdinal(snort_conf->sopgTable, IPPROTO_UDP,
                                                 TO_SERVER, proto_ordinal);
            src = fpGetServicePortGroupByOrdinal(snort_conf->sopgTable, IPPROTO_UDP,
                                                 TO_CLIENT, proto_ordinal);

            DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE,
                        "fpEvalHeaderUdpp:targetbased-ordinal-lookup: "
                        "sport=%d, dport=%d, proto_ordinal=%d, src:%x, "
                        "dst:%x, gen:%x\n",p->sp,p->dp,proto_ordinal,src,dst,gen););
        }
    }

    if ((src == NULL) && (dst == NULL))
    {
        /* we did not have a target based port group, use ports */
        if (!prmFindRuleGroupUdp(snort_conf->prmUdpRTNX, p->dp, p->sp, &src, &dst, &gen))
            return 0;

        DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE,
                    "fpEvalHeaderUdp: sport=%d, dport=%d, "
                    "src:%x, dst:%x, gen:%x\n",p->sp,p->dp,src,dst,gen););
    }
#else
    if (!prmFindRuleGroupUdp(snort_conf->prmUdpRTNX, p->dp, p->sp, &src, &dst, &gen))
        return 0;
#endif

    if (fpDetectGetDebugPrintNcRules(snort_conf->fast_pattern_config))
    {
        LogMessage(
            "fpEvalHeaderUdp: sport=%d, dport=%d, src:%p, dst:%p, gen:%p\n",
             p->sp, p->dp, (void*)src, (void*)dst, (void*)gen);
    }

    InitMatchInfo(omd);

    if (dst != NULL)
    {
        if (fpEvalHeaderSW(dst, p, 1, 0, omd))
            return 1;
    }

    if (src != NULL)
    {
        if (fpEvalHeaderSW(src, p, 1, 0, omd))
            return 1;
    }

    if (gen != NULL)
    {
        if (fpEvalHeaderSW(gen, p, 1, 0, omd))
            return 1;
    }

    return fpFinalSelectEvent(omd, p);
}

/*
**  fpEvalHeaderTcp::
*/
static INLINE int fpEvalHeaderTcp(Packet *p, OTNX_MATCH_DATA *omd)
{
    PORT_GROUP *src = NULL, *dst = NULL, *gen = NULL;

#ifdef TARGET_BASED
    if (IsAdaptiveConfigured(getRuntimePolicy(), 0))
    {
        int16_t proto_ordinal = GetProtocolReference(p);

        DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "proto_ordinal=%d\n", proto_ordinal););

        if (proto_ordinal > 0)
        {
            if (p->packet_flags & PKT_FROM_SERVER) /* to cli */
            {
                DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "pkt_from_server\n"););

                src = fpGetServicePortGroupByOrdinal(snort_conf->sopgTable, IPPROTO_TCP,
                                                     0 /*to_cli */,  proto_ordinal);
            }

            if (p->packet_flags & PKT_FROM_CLIENT) /* to srv */
            {
                DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "pkt_from_client\n"););

                dst = fpGetServicePortGroupByOrdinal(snort_conf->sopgTable, IPPROTO_TCP,
                                                     1 /*to_srv */,  proto_ordinal);
            }

            DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE,
                        "fpEvalHeaderTcp:targetbased-ordinal-lookup: "
                        "sport=%d, dport=%d, proto_ordinal=%d, src:%x, "
                        "dst:%x, gen:%x\n",p->sp,p->dp,proto_ordinal,src,dst,gen););
        }
    }

    if ((src == NULL) && (dst == NULL))
    {
        /* we did not have a target based group, use ports */
        if (!prmFindRuleGroupTcp(snort_conf->prmTcpRTNX, p->dp, p->sp, &src, &dst, &gen))
            return 0;

        DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE,
                    "fpEvalHeaderTcp: sport=%d, "
                    "dport=%d, src:%x, dst:%x, gen:%x\n",p->sp,p->dp,src,dst,gen););
    }
#else
    if (!prmFindRuleGroupTcp(snort_conf->prmTcpRTNX, p->dp, p->sp, &src, &dst, &gen))
        return 0;
#endif

    if (fpDetectGetDebugPrintNcRules(snort_conf->fast_pattern_config))
    {
        LogMessage(
            "fpEvalHeaderTcp: sport=%d, dport=%d, src:%p, dst:%p, gen:%p\n",
             p->sp, p->dp, (void*)src, (void*)dst, (void*)gen);
    }

    InitMatchInfo(omd);

    if (dst != NULL)
    {
        if (fpEvalHeaderSW(dst, p, 1, 0, omd))
            return 1;
    }

    if (src != NULL)
    {
        if (fpEvalHeaderSW(src, p, 1, 0, omd))
            return 1;
    }

    if (gen != NULL)
    {
        if(fpEvalHeaderSW(gen, p, 1, 0, omd))
            return 1;
    }

    return fpFinalSelectEvent(omd, p);
}

/*
**  fpEvalHeaderICMP::
*/
static INLINE int fpEvalHeaderIcmp(Packet *p, OTNX_MATCH_DATA *omd)
{
    PORT_GROUP *gen = NULL, *type = NULL;

    if (!prmFindRuleGroupIcmp(snort_conf->prmIcmpRTNX, p->icmph->type, &type, &gen))
        return 0;
  
    if (fpDetectGetDebugPrintNcRules(snort_conf->fast_pattern_config))
    {
        LogMessage(
            "fpEvalHeaderIcmp: icmp->type=%d type=%p gen=%p\n",
            p->icmph->type, (void*)type, (void*)gen);
    }

    InitMatchInfo(omd);

    if (type != NULL)
    {
        if (fpEvalHeaderSW(type, p, 0, 0, omd))
            return 1;
    }

    if (gen != NULL)
    {
        if (fpEvalHeaderSW(gen, p, 0, 0, omd))
            return 1;
    }

    return fpFinalSelectEvent(omd, p);
}

/*
**  fpEvalHeaderIP::
*/
static INLINE int fpEvalHeaderIp(Packet *p, int ip_proto, OTNX_MATCH_DATA *omd)
{
    PORT_GROUP *gen = NULL, *ip_group = NULL;

    if (!prmFindRuleGroupIp(snort_conf->prmIpRTNX, ip_proto, &ip_group, &gen))
        return 0;
    
    if(fpDetectGetDebugPrintNcRules(snort_conf->fast_pattern_config))
        LogMessage("fpEvalHeaderIp: ip_group=%p, gen=%p\n", (void*)ip_group, (void*)gen);
 
    InitMatchInfo(omd);

    if (ip_group != NULL)
    {
        if (fpEvalHeaderSW(ip_group, p, 0, 1, omd))
            return 1;
    }

    if (gen != NULL)
    {
        if (fpEvalHeaderSW(gen, p, 0, 1, omd))
            return 1;
    }

    return fpFinalSelectEvent(omd, p);
}

/*
**
**  NAME
**    fpEvalPacket::
**
**  DESCRIPTION
**    This function is the interface to the Detect() routine.  Here 
**    the IP protocol is processed.  If it is TCP, UDP, or ICMP, we
**    process the both that particular ruleset and the IP ruleset
**    with in the fpEvalHeader for that protocol.  If the protocol
**    is not TCP, UDP, or ICMP, we just process the packet against
**    the IP rules at the end of the fpEvalPacket routine.  Since
**    we are using a setwise methodology for snort rules, both the
**    network layer rules and the transport layer rules are done
**    at the same time.  While this is not the best for modularity,
**    it is the best for performance, which is what we are working
**    on currently.
**
**  FORMAL INPUTS
**    Packet * - the packet to inspect
**
**  FORMAL OUTPUT
**    int - 0 means that packet has been processed.
**
*/
int fpEvalPacket(Packet *p)
{
    int ip_proto = GET_IPH_PROTO(p);
    OTNX_MATCH_DATA *omd = snort_conf->omd;

    /* Run UDP rules against the UDP header of Teredo packets */
    if ( p->udph && (p->proto_bits & PROTO_BIT__TEREDO) )
    {
        uint16_t tmp_sp = p->sp;
        uint16_t tmp_dp = p->dp;
        const UDPHdr *tmp_udph = p->udph;
        const uint8_t *tmp_data = p->data;
        uint16_t tmp_dsize = p->dsize;

        if (p->outer_udph)
        {
            p->udph = p->outer_udph;
        }
        p->sp = ntohs(p->udph->uh_sport);
        p->dp = ntohs(p->udph->uh_dport);
        p->data = (const uint8_t *)p->udph + UDP_HEADER_LEN;
        p->dsize = p->outer_ip_dsize - UDP_HEADER_LEN;

        fpEvalHeaderUdp(p, omd);

        p->sp = tmp_sp;
        p->dp = tmp_dp;
        p->udph = tmp_udph;
        p->data = tmp_data;
        p->dsize = tmp_dsize;
    }

    switch(ip_proto)
    {
        case IPPROTO_TCP:
            DEBUG_WRAP(DebugMessage(DEBUG_DETECT, 
                        "Detecting on TcpList\n"););

            if(p->tcph == NULL)
            {
                ip_proto = -1;
                break;
            }

            return fpEvalHeaderTcp(p, omd);

        case IPPROTO_UDP:
            DEBUG_WRAP(DebugMessage(DEBUG_DETECT, 
                        "Detecting on UdpList\n"););

            if(p->udph == NULL)
            {
                ip_proto = -1;
                break;
            }
            
            return fpEvalHeaderUdp(p, omd);

#ifdef SUP_IP6
        case IPPROTO_ICMPV6:
#endif
        case IPPROTO_ICMP:
            DEBUG_WRAP(DebugMessage(DEBUG_DETECT, 
                        "Detecting on IcmpList\n"););

            if(p->icmph == NULL)
            {
                ip_proto = -1;
                break; 
            }

            return fpEvalHeaderIcmp(p, omd);

        default:
            break;
    }

    /*
    **  No Match on TCP/UDP, Do IP
    */
    return fpEvalHeaderIp(p, ip_proto, omd);
}

void fpEvalIpProtoOnlyRules(SF_LIST **ip_proto_only_lists, Packet *p)
{
    if ((p != NULL) && IPH_IS_VALID(p))
    {
        SF_LIST *l = ip_proto_only_lists[GET_IPH_PROTO(p)];
        OptTreeNode *otn;

        /* If list is NULL, sflist_first returns NULL */
        for (otn = (OptTreeNode *)sflist_first(l);
             otn != NULL;
             otn = (OptTreeNode *)sflist_next(l))
        {
            if (fpEvalRTN(getRuntimeRtnFromOtn(otn), p, 0))
            {
                SnortEventqAdd(otn->sigInfo.generator, 
                               otn->sigInfo.id,
                               otn->sigInfo.rev,
                               otn->sigInfo.class_id,
                               otn->sigInfo.priority,
                               otn->sigInfo.message,
                               (void *)NULL);
            }
        }
    }
}

