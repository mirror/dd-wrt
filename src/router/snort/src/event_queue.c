/* $Id$ */
/*
 ** Copyright (C) 2014 Cisco and/or its affiliates. All rights reserved.
 ** Copyright (C) 2004-2013 Sourcefire, Inc.
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

#include "sf_types.h"
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

static unsigned s_events = 0;

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

int SnortEventqAdd(uint32_t gid, uint32_t sid, uint32_t rev,
        uint32_t classification, uint32_t priority, char *msg, void *rule_info)
{
    OptTreeNode *otn = (OptTreeNode *)rule_info;
    EventNode *en;

    // Preprocessors and decoder will call this function with a NULL rule_info
    // since they don't have access to the OTN.  Get the OTN and set the event
    // node's rule_info so when LogSnortEvents is called another lookup of the
    // OTN isn't necessary.
    // Return 0 if no OTN since -1 return indicates queue limit reached. See
    // fpFinalSelectEvent()
    if (otn == NULL)
    {
        otn = GetOTN(gid, sid, rev, classification, priority, msg);
        if (otn == NULL)
            return 0;
        rule_info = (void *)otn;
    }
    else if (getRtnFromOtn(otn, getIpsRuntimePolicy()) == NULL)
    {
        // If the rule isn't in the current policy, don't add it to
        // the event queue.
        return 0;
    }

    en = (EventNode *)sfeventq_event_alloc(snort_conf->event_queue[qIndex]);
    if (en == NULL)
        return -1;

    en->gid = gid;
    en->sid = sid;
    en->rev = rev;
    en->classification = classification;
    en->priority = priority;
    en->msg = msg;
    en->rule_info = rule_info;

    if (sfeventq_add(snort_conf->event_queue[qIndex], (void *)en) != 0)
        return -1;

    s_events++;

    return 0;
}

void SnortEventqNew(
    EventQueueConfig *eq_config, SF_EVENTQ *eq[]
) {
    int i;

    for ( i = 0; i < NUM_EVENT_QUEUES; i++ )
    {
        eq[i] = sfeventq_new(eq_config->max_events,
                eq_config->log_events, sizeof(EventNode));

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
    EventNode *en = (EventNode *)event;
    SNORT_EVENTQ_USER *snort_user = (SNORT_EVENTQ_USER *)user;
    OptTreeNode *otn;
    RuleTreeNode *rtn = NULL;

    if ((event == NULL) || (user == NULL))
        return 0;

    if ( s_events > 0 )
        s_events--;

    if (en->rule_info != NULL)
    {
        otn = (OptTreeNode *)en->rule_info;
    }
    else
    {
        // The above en->rule_info shouldn't be NULL, but leave this here
        // in case for some reason this function should be called outside
        // of the normal SnortEventqAdd() -> SnortEventqLog() chain.
        otn = GetOTN(en->gid, en->sid, en->rev,
                en->classification, en->priority, en->msg);
    }

    if (otn != NULL)
        rtn = getRuntimeRtnFromOtn(otn);

    if ((otn != NULL) && (rtn != NULL))
    {
        snort_user->rule_alert = otn->sigInfo.rule_flushing;

        if (IsPreprocDecoderRule(otn->sigInfo.rule_type))
        {
            // If it's a preprocessor or decoder rule, the message we want to
            // use is the one that was passed in, not what is in the message of
            // the OTN, which will be generic if the rule was not autogenerated
            // and potentially wrong if it was.
            char *tmp = otn->sigInfo.message;
            otn->sigInfo.message = en->msg;
            fpLogEvent(rtn, otn, (Packet *)snort_user->pkt);
            otn->sigInfo.message = tmp;
        }
        else
        {
            fpLogEvent(rtn, otn, (Packet *)snort_user->pkt);
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

static inline void reset_counts (void)
{
    pc.log_limit += s_events;
    s_events = 0;
}

void SnortEventqResetCounts (void)
{
    reset_counts();
}

void SnortEventqReset(void)
{
    sfeventq_reset(snort_conf->event_queue[qIndex]);
    reset_counts();
}

