/* $Id$ */
/*
 ** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 ** Copyright (C) 1998-2013 Sourcefire, Inc.
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
 * @file   event_wrapper.c
 * @author Chris Green <cmg@sourcefire.com>
 *
 * @date   Wed Jun 18 10:49:59 2003
 *
 * @brief  generate a snort event
 *
 * This is a wrapper around SetEvent,CallLogFuncs,CallEventFuncs
 *
 * Notes:
 *
 *   10/31/05 - Marc Norton
 *   Changes to support every event being controlled via a rule.
 *   Modified GenerateSnortEvent() to re-route events to 'fpLogEvent'
 *   if a suitable otn was found.  If no otn was found, than we do
 *   not log the event at all, as no rule was provided.
 *   Preprocessors are configured independently, and may detect
 *   an event, but the rule controls the alert/drop functionality.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_types.h"
#include "rules.h"
#include "treenodes.h"
#include "signature.h"
#include "util.h"
#include "event_wrapper.h"
#include "fpdetect.h"
#include "snort_debug.h"
#include "sfPolicy.h"

RuleTreeNode* GenerateSnortEventRtn (OptTreeNode* otn, tSfPolicyId policyId)
{
    RuleTreeNode *rtn = getRtnFromOtn(otn, policyId);
    if (!rtn)
    {
        rtn = calloc(1, sizeof(RuleTreeNode));
        if (rtn)
        {
            rtn->type = RULE_TYPE__ALERT;
            if (addRtnToOtn(NULL, otn, policyId, rtn) != 0)
                rtn = NULL;
        }
    }

    return rtn;
}

OptTreeNode * GenerateSnortEventOtn(
                            uint32_t gen_id,
                            uint32_t sig_id,
                            uint32_t sig_rev,
                            uint32_t classification,
                            uint32_t priority,
                            const char *msg )
{
    OptTreeNode *otn;
    RuleTreeNode *rtn;

    otn = otnCreate(gen_id, sig_id, sig_rev, classification, priority, msg);
    if (otn)
    {
        rtn = GenerateSnortEventRtn(otn, getIpsRuntimePolicy());
        if (!rtn)
        {
            free(otn);
            return NULL;
        }
    }

    DEBUG_WRAP(
        LogMessage("Generating OTN for GID: %u, SID: %u\n",gen_id,sig_id););

    return otn;
}

/*
 * This function returns the rule action given the
 * details about the rule
 */
int GetSnortEventAction(uint32_t gid, uint32_t sid,
                        uint32_t rev, uint32_t classification,
                        uint32_t priority, const char *msg)
{
    OptTreeNode *otn;
    RuleTreeNode *rtn;

    if (msg == NULL)
        return 0;

    otn = GetApplicableOtn(gid, sid, rev, classification, priority, msg);

    if (otn == NULL)
        return 0;

    rtn = getRuntimeRtnFromOtn(otn);
    if (rtn == NULL)
        return 0;

    return (rtn->type);
}


/*
 * This function has been updated to find an otn and route the call to fpLogEvent
 * if possible.  This requires a rule be written for each decoder event,
 * and possibly some preporcessor events.  The bulk of eventing is handled vie the
 * SnortEventqAdd() and SnortEventLog() functions - whichalready  route the events to
 * the fpLogEvent()function.
 */

uint32_t GenerateSnortEvent(
    Packet *p,
    uint32_t gid,
    uint32_t sid,
    uint32_t rev,
    uint32_t classification,
    uint32_t priority,
    const char * msg
    )
{
    OptTreeNode *otn;
    RuleTreeNode *rtn;

    if (msg == NULL)
        return 0;

    otn = GetApplicableOtn(gid, sid, rev, classification, priority, msg);

    if (otn == NULL)
        return 0;

    rtn = getRuntimeRtnFromOtn(otn);
    if (rtn == NULL)
        return 0;

    LogSnortEvent(p, otn, rtn, msg);

    return otn->event_data.event_id;
}

/**
 * Log additional packet data using the same kinda mechanism tagging does.
 *
 * @param p Packet to log
 * @param gen_id generator id
 * @param sig_id signature id
 * @param sig_rev revision is
 * @param classification classification id
 * @param priority priority level
 * @param event_ref reference of a previous event
 * @param ref_sec the tv_sec of that previous event
 * @param msg The message data txt
 *
 * @return 1 on success, 0 on FAILURE ( note this is to stay the same as GenerateSnortEvent() )
 */
int LogTagData(
    Packet * p,
    uint32_t gen_id,
    uint32_t sig_id,
    uint32_t sig_rev,
    uint32_t classification,
    uint32_t priority,
    uint32_t event_ref,
    time_t ref_sec,
    char * msg
    )
{
    Event event;

    if(!event_ref || !ref_sec)
        return 0;

#if !defined(FEAT_OPEN_APPID)
    SetEvent(&event, gen_id, sig_id, sig_rev, classification, priority, event_ref);
#else /* defined(FEAT_OPEN_APPID) */
    SetEvent(&event, gen_id, sig_id, sig_rev, classification, priority, event_ref, NULL);
#endif /* defined(FEAT_OPEN_APPID) */

    event.ref_time.tv_sec = (uint32_t)ref_sec;

    if(p)
        CallLogFuncs(p, msg, NULL, &event);

    return 1;
}

void LogSnortEvent(
    Packet * p,
    OptTreeNode * otn,
    RuleTreeNode * rtn,
    const char * event_msg
    )
{
    if (IsPreprocDecoderRule(otn->sigInfo.rule_type))
    {
        // If it's a preprocessor or decoder rule, the message we want to
        // use is the one that was passed in, not what is in the message of
        // the OTN, which will be generic if the rule was not autogenerated
        // and potentially wrong if it was.
        const char *tmp = otn->sigInfo.message;
        otn->sigInfo.message = event_msg;
        fpLogEvent(rtn, otn, p);
        otn->sigInfo.message = tmp;
    }
    else
    {
        fpLogEvent(rtn, otn, p);
    }
}

OptTreeNode *otnCreate(uint32_t gid, uint32_t sid, uint32_t rev,
                       uint32_t classification, uint32_t priority, const char *msg)
{
    OptTreeNode *otn = calloc(1, sizeof(OptTreeNode));
    if (otn)
    {
        otn->sigInfo.generator = gid;
        otn->sigInfo.id = sid;
        otn->sigInfo.rev = rev;
        otn->sigInfo.message = msg;
        otn->sigInfo.priority = priority;
        otn->sigInfo.class_id = classification;

        otn->generated = 1;

        otn->sigInfo.rule_type=SI_RULE_TYPE_PREPROC; /* TODO: could be detect ... */
        otn->sigInfo.rule_flushing=SI_RULE_FLUSHING_OFF; /* only standard rules do this */

        otn->event_data.sig_generator = gid;
        otn->event_data.sig_id = sid;
        otn->event_data.sig_rev = rev;
        otn->event_data.classification = classification;
        otn->event_data.priority = priority;
    }

    return otn;
}

