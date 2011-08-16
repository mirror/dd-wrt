/* $Id$ */
/*
 ** Copyright (C) 2004-2011 Sourcefire, Inc.
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
 */
/**
**  @file       event_queue.c
**
**  @author     Daniel Roelker <droelker@sourcefire.com>
**  @author     Marc Norton <mnorton@sourcefire.com>
**
**  @brief      Snort wrapper to sfeventq library.
**
**  These functions wrap the sfeventq API and provide the priority
**  functions for ordering incoming events.
**
** Notes:
**  11/1/05  Updates to add support for rules for all events in 
**           decoders and preprocessors and the detection engine.
**           Added support for rule by rule flushing control via
**           metadata. Also added code to check fo an otn for every
**           event (gid,sid pair).  This is now required to get events
**           to be logged. The decoders and preprocessors are still 
**           configured independently, which allows them to inspect and 
**           call the alerting functions SnortEventqAdd, GenerateSnortEvent()
**           and GenerateEvent2() for sfportscan.c.  The GenerateSnortEvent()
**           function now finds and otn and calls fpLogEvent.
**
**           Any event that has no otn associated with it's gid,sid pair,
**           will/should not alert, even if the preprocessor or decoiderr is
**           configured to detect an alertable event.
**
**           In the future, preporcessor may have an api that gets called
**           after rules are loaded that checks for the gid/sid -> otn 
**           mapping, and then adjusts it's inspection or detection
**           accordingly.  
**
**           SnortEventqAdd() - only adds events that have an otn 
**          
*/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "fpcreate.h"
#include "fpdetect.h"
#include "util.h"
#include "sfeventq.h"
#include "event_wrapper.h"
#include "event_queue.h"
#include "sfthreshold.h"
#include "sfPolicy.h"

//-------------------------------------------------
// the push/pop methods ensure that qIndex stays in
// bounds and that it is only popped after it was
// successfully pushed.
static unsigned qIndex = 0;
static unsigned qOverflow = 0;

void SnortEventqPush(void)
{
    if ( qIndex < NUM_EVENT_QUEUES-1 ) qIndex++;
    else qOverflow++;
}

void SnortEventqPop(void)
{
    if ( qOverflow > 0 ) qOverflow--;
    else if ( qIndex > 0 ) qIndex--;
}
//-------------------------------------------------

/*
**  Set default values
*/
EventQueueConfig * EventQueueConfigNew(void)
{
    EventQueueConfig *eq =
        (EventQueueConfig *)SnortAlloc(sizeof(EventQueueConfig));

    eq->max_events = 8;
    eq->log_events = 3;

    eq->order = SNORT_EVENTQ_CONTENT_LEN;
    eq->process_all_events = 0;

    return eq;
}

void EventQueueConfigFree(EventQueueConfig *eq)
{
    if (eq == NULL)
        return;

    free(eq);
}

/*
 *  Changed so events are inserted in action config order 'drop alert ...',
 *  and sub sorted in each action group by priority or content length.
 *  The sub sorting is done in fpFinalSelect inf fpdetect.c.  Once the
 *  events are inserted they can all be logged, as we only insert
 *  g_event_queue.log_events into the queue.
 *  ... Jan '06
 */
int SnortEventqAdd(unsigned int gid, 
                   unsigned int sid, 
                   unsigned int rev, 
                   unsigned int classification, 
                   unsigned int pri,
                   char        *msg,
                   void        *rule_info)
{
    EventNode *en;
    en = (EventNode *)sfeventq_event_alloc(snort_conf->event_queue[qIndex]);

    if(!en)
        return -1;

    en->gid = gid;
    en->sid = sid;
    en->rev = rev;
    en->classification = classification;
    en->priority = pri;
    en->msg = msg;
    en->rule_info = rule_info;

    /* 
     * Check if we have a preprocessor or decoder event
     * Preprocessors and decoders may be configured to inspect
     * and alert in their principal configuration (legacy code) 
     * this test than checks if the rule otn says they should 
     * be enabled or not.  The rule itself will decide if it should
     * be an alert or a drop (sdrop) condition.
     */
   
#ifdef PREPROCESSOR_AND_DECODER_RULE_EVENTS
    {
        struct _OptTreeNode * potn;

        /* every event should have a rule/otn  */
        potn = OtnLookup(snort_conf->otn_map, gid, sid);
        /* 
         * if no rule otn exists for this event, than it was 
         * not enabled via rules 
         */

        if (potn == NULL)
        {
            if (ScAutoGenPreprocDecoderOtns())
            {
                /* Generate an OTN if configured to do so.... */
                potn = GenerateSnortEventOtn(en->gid,
                                             en->sid,
                                             en->rev,
                                             en->classification,
                                             en->priority,
                                             en->msg);

                if (potn != NULL)  
                    OtnLookupAdd(snort_conf->otn_map, potn);
            }
            if (potn == NULL) 
            {
                /* no otn found/created - do not add it to the queue */
                return 0;
            }
        }
        else
        {
            tSfPolicyId policyId = getRuntimePolicy();
            RuleTreeNode* rtn = getRtnFromOtn(potn, policyId);

            if ( !rtn )
            {
                if ( ScAutoGenPreprocDecoderOtns() )
                    rtn = GenerateSnortEventRtn(potn, getRuntimePolicy());

                if ( !rtn )
                    return 0;
            }
        }    
    }
#endif
     
    if (sfeventq_add(snort_conf->event_queue[qIndex], (void *)en))
    {
        return -1;
    } 
    return 0;
}
#ifdef OLD_RULE_ORDER
static int OrderPriority(void *event1, void *event2)
{
    EventNode *e1;
    EventNode *e2;

    if(!event1 || !event2)
        return 0;

    e1 = (EventNode *)event1;
    e2 = (EventNode *)event2;

    if(e1->priority < e2->priority)
        return 1;

    return 0;
}

static int OrderContentLength(void *event1, void *event2)
{
    EventNode *e1;
    EventNode *e2;
    OTNX      *o1;
    OTNX      *o2;

    if(!event1 || !event2)
        return 0;

    e1 = (EventNode *)event1;
    e2 = (EventNode *)event2;

    if(!e1->rule_info && e2->rule_info)
    {
        /*
        **  incoming event is not a rule, keep
        **  looking.
        */
        return 0;
    }
    else if(e1->rule_info && !e2->rule_info)
    {
        /*
        **  incoming event is a rule, event in queue
        **  is not.  Put incoming here.
        */
        return 1;
    }
    else if(!e1->rule_info && !e2->rule_info)
    {
        /*
        **  Neither event is a rule.  Use incoming as
        **  priority.  Last one in goes at the end to 
        **  preserve rule order.
        */
        return 0;
    }

    /*
    **  We already know that these pointers aren't NULL by
    **  the previous logic.
    */
    o1 = (OTNX *)e1->rule_info;
    o2 = (OTNX *)e2->rule_info;

    if(o1->content_length > o2->content_length)
        return 1;

    return 0;
}
#endif


void SnortEventqNew(
    EventQueueConfig *eq_config, SF_EVENTQ *eq[]
) {
    int (*sort)(void *, void*) = NULL;
    int i;

#ifdef OLD_RULE_ORDER
    if (eq_config->order == SNORT_EVENTQ_PRIORITY)
    {
        sort = OrderPriority;
    }
    else if (eq_config->order == SNORT_EVENTQ_CONTENT_LEN)
    {
        sort = OrderContentLength;
    }
    else
    {
        FatalError("Order function for event queue is invalid.\n");
    }
#endif

    for ( i = 0; i < NUM_EVENT_QUEUES; i++ )
    {
        eq[i] = sfeventq_new(eq_config->max_events, eq_config->log_events,
                      sizeof(EventNode), sort);

        if (eq[i] == NULL)
            FatalError("Failed to initialize Snort event queue.\n");
    }
}

void SnortEventqFree(SF_EVENTQ *eq[])
{
    int i;
    for ( i = 0; i < NUM_EVENT_QUEUES; i++ )
        sfeventq_free(eq[i]);
}

static int LogSnortEvents(void *event, void *user)
{
    Packet    *p;
    EventNode *en;
    OTNX      *otnx;
    struct    _OptTreeNode * potn;
    SNORT_EVENTQ_USER *snort_user;

    if(!event || !user)
        return 0;

    en         = (EventNode *)event;
    snort_user = (SNORT_EVENTQ_USER *)user;
    p  = (Packet *)snort_user->pkt;

    /*
    **  Log rule events differently because we have to.
    */
    if(en->rule_info)
    {
        otnx = (OTNX *)en->rule_info;
        if(!otnx->otn || !getRuntimeRtnFromOtn(otnx->otn))
            return 0;

        snort_user->rule_alert = otnx->otn->sigInfo.rule_flushing;
        fpLogEvent(getRuntimeRtnFromOtn(otnx->otn), otnx->otn, p);
    }
    else
    {
        /* Look up possible decoder and preprocessor event otn */
        potn = OtnLookup(snort_conf->otn_map, en->gid, en->sid);

        if (potn == NULL)
        {
#ifdef PREPROCESSOR_AND_DECODER_RULE_EVENTS
            if (ScAutoGenPreprocDecoderOtns())
            {
                /* Generate an OTN if configured to do so.... */
                potn = GenerateSnortEventOtn(en->gid,
                                             en->sid,
                                             en->rev,
                                             en->classification,
                                             en->priority,
                                             en->msg);
            }
#else
            /* Always generate an OTN.... */
            potn = GenerateSnortEventOtn(en->gid,
                                         en->sid,
                                         en->rev,
                                         en->classification,
                                         en->priority,
                                         en->msg);
#endif        
            if (potn != NULL)
            {
                OtnLookupAdd(snort_conf->otn_map, potn);
            }
        }

        if( potn )
        {
            char *tmp = potn->sigInfo.message;
            snort_user->rule_alert = potn->sigInfo.rule_flushing;
            potn->sigInfo.message = en->msg;

            fpLogEvent( getRuntimeRtnFromOtn(potn), potn, p );
            potn->sigInfo.message = tmp;
        }
    }

    sfthreshold_reset();

    return 0;
}

/*
**  NAME
**    SnortEventqLog::
*/
/**
**  We return whether we logged events or not.  We've add a eventq user
**  structure so we can track whether the events logged were rule events
**  or preprocessor/decoder events.  The reason being that we don't want
**  to flush a TCP stream for preprocessor/decoder events, and cause
**  early flushing of the stream.
**
**  @return 1 logged events
**  @return 0 did not log events or logged only decoder/preprocessor events
*/
int SnortEventqLog(SF_EVENTQ *eq[], Packet *p)
{
    static SNORT_EVENTQ_USER user;

    user.rule_alert = 0x00;
    user.pkt = (void *)p;

    if (sfeventq_action(eq[qIndex], LogSnortEvents, (void *)&user) > 0)
    {
        if (user.rule_alert)
            return 1;
    }

    return 0;
}

void SnortEventqReset(void)
{
    sfeventq_reset(snort_conf->event_queue[qIndex]);
}

