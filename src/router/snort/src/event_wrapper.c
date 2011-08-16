/* $Id$ */
/*
 ** Copyright (C) 1998-2011 Sourcefire, Inc.
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

#include "rules.h"
#include "treenodes.h"
#include "signature.h"
#include "util.h"
#include "event_wrapper.h"
#include "fpdetect.h"
#include "debug.h"
#include "sfPolicy.h"

RuleTreeNode* GenerateSnortEventRtn (
    OptTreeNode* otn,
    tSfPolicyId policyId
) {
    RuleTreeNode* rtn = getRtnFromOtn(otn, policyId);

    rtn = calloc(1, sizeof(RuleTreeNode));

    if ( !rtn )
        return NULL;

    rtn->type = RULE_TYPE__ALERT;

    if ( addRtnToOtn(otn, policyId, rtn) != 0 )
    {
        //unsuccessful adding rtn
        free(rtn);
        return NULL;
    }
    return rtn;
}

OptTreeNode * GenerateSnortEventOtn(
                            uint32_t gen_id,
                            uint32_t sig_id,
                            uint32_t sig_rev,
                            uint32_t classification,
                            uint32_t priority,
                            char *msg )
{
   OptTreeNode * p;
   RuleTreeNode *rtn = NULL;

   p = calloc( 1, sizeof(OptTreeNode) );
   if(!p )
       return 0;

   p->sigInfo.generator = gen_id;
   p->sigInfo.id = sig_id;
   p->sigInfo.rev = sig_rev;
   p->sigInfo.message = msg;
   p->sigInfo.priority = priority;
   p->sigInfo.class_id = classification;

   p->generated = 1;
            
   p->sigInfo.rule_type=SI_RULE_TYPE_PREPROC; /* TODO: could be detect ... */
   p->sigInfo.rule_flushing=SI_RULE_FLUSHING_OFF; /* only standard rules do this */

   p->event_data.sig_generator = gen_id;
   p->event_data.sig_id = sig_id;
   p->event_data.sig_rev = sig_rev;
   p->event_data.classification = classification;
   p->event_data.priority = priority;

   rtn = GenerateSnortEventRtn(p, getRuntimePolicy());
   
   if( !rtn )
   {
       free(p);
       return NULL;
   }

   DEBUG_WRAP(
           LogMessage("Generating OTN for GID: %u, SID: %u\n",gen_id,sig_id););
   
   return p;
}

/*
 * This function has been updated to find an otn and route the call to fpLogEvent
 * if possible.  This requires a rule be written for each decoder event, 
 * and possibly some preporcessor events.  The bulk of eventing is handled vie the 
 * SnortEventqAdd() and SnortEventLog() functions - whichalready  route the events to 
 * the fpLogEvent()function.
 */
uint32_t GenerateSnortEvent(Packet *p,
                             uint32_t gen_id,
                             uint32_t sig_id,
                             uint32_t sig_rev,
                             uint32_t classification,
                             uint32_t priority,
                             char *msg)
{
    struct _OptTreeNode * potn;    

    if(!msg)
    {
        return 0;
    }
    
    /* 
     * Check if we have a preprocessor or decoder event
     * Preprocessors and decoders may be configured to inspect
     * and alert in their principle configuration (legacy code) 
     * this test then checks if the rule otn says they should 
     * be enabled or not.  The rule itself will decide if it should
     * be an alert or a drop (sdrop) condition.
     */
    
     /* every event should have a rule/otn  */
     potn = OtnLookup(snort_conf->otn_map, gen_id, sig_id);
     /* 
     * if no rule otn exists for this event, than it was 
     * not enabled via rules 
     */
     if (potn == NULL)
     {
#ifdef PREPROCESSOR_AND_DECODER_RULE_EVENTS
        if (ScAutoGenPreprocDecoderOtns())
        {
            /* If configured to generate preprocessor and decoder OTNs,
             * do so... */
            potn = GenerateSnortEventOtn(gen_id,
                                         sig_id,
                                         sig_rev,
                                         classification,
                                         priority,
                                         msg);
        }
#else
        /* 
         * Until we have official 'preprocessor/decoder rules' we
         * will add the rule to the otn_lookup , once enabled, remove
         * this call to gen the otn...  Once a preprocessor/decoder
         * event fires we generate the otn, and next time it's found
         * above in otn_lookup.
         */
        potn = GenerateSnortEventOtn(gen_id,
                                     sig_id,
                                     sig_rev,
                                     classification,
                                     priority,
                                     msg);
#endif
        if (potn != NULL)  
            OtnLookupAdd(snort_conf->otn_map, potn);
     }

     if (potn == NULL)
     {
         /* no otn found - do not add it to the queue */
         return 0;
     }

     fpLogEvent( getRuntimeRtnFromOtn(potn), potn,  p );

     return potn->event_data.event_id;
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
int LogTagData(Packet *p,
               uint32_t gen_id,
               uint32_t sig_id,
               uint32_t sig_rev,
               uint32_t classification,
               uint32_t priority,
               uint32_t event_ref,
               time_t ref_sec,
               char *msg)
   
{
    Event event;
    
    if(!event_ref || !ref_sec)
        return 0;

    SetEvent(&event, gen_id, sig_id, sig_rev, classification, priority, event_ref);

    event.ref_time.tv_sec = (uint32_t)ref_sec;
    
    if(p)
        CallLogFuncs(p, msg, NULL, &event);

    return 1;
}
                     
