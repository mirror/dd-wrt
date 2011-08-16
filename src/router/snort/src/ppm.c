/****************************************************************************
 *
 * Copyright (C) 2006-2011 Sourcefire, Inc.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 ****************************************************************************/

/*
**  $Id$
**
**  ppm.c
**
**  Packet Performance Monitor
**
**  Author: Marc Norton <mnorton@sourcefire.com>
**  Date: 10/2006
**
**  Usage:
*
*   config ppm: max-pkt-time usecs
*   config ppm: max-rule-time usecs
*   config ppm: max-suspend-time secs
*   config ppm: threshold count
*   config ppm: suspend-expensive-rules
*   config ppm: fastpath-expensive-packets
*   config ppm: pkt-events  syslog|console
*   config ppm: rule-events alert|syslog|console
*   config ppm: debug-rules
*   config ppm: debug-pkts
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <time.h>

#include "snort.h"
#include "rules.h"
#include "treenodes.h"
#include "treenodes.h"
#include "decode.h"
#include "parser.h"
#include "plugin_enum.h"
#include "util.h"
#include "rules.h"
#include "treenodes.h"
#include "treenodes.h"
#include "fpcreate.h"
#include "event_queue.h"
#include "event_wrapper.h"
#include "log.h"
#include "ppm.h"
#include "sf_types.h"
#include "generators.h"

#ifdef PPM_MGR

#define PPM_BASE_SUSPEND_RULE_GID 1000
#define PPM_BASE_CLEAR_RULE_GID   2000

#define PPM_DEFAULT_MAX_PKT_TICKS    0
#define PPM_DEFAULT_MAX_RULE_TICKS   0
#define PPM_DEFAULT_MAX_SUSP_SECS   60
#define PPM_DEFAULT_RULE_THRESHOLD   5

PPM_TICKS ppm_tpu = 0; /* ticks per usec */

ppm_pkt_timer_t   ppm_pkt_times[PPM_MAX_TIMERS];
ppm_pkt_timer_t  *ppm_pt = NULL;
unsigned int      ppm_pkt_index = 0;
ppm_rule_timer_t  ppm_rule_times[PPM_MAX_TIMERS];
ppm_rule_timer_t *ppm_rt = NULL;
unsigned int      ppm_rule_times_index = 0;
uint64_t            ppm_cur_time = 0;

/* temporary flags */
int ppm_abort_this_pkt = 0;
int ppm_suspend_this_rule = 0;

/* debug-pkts  data */
#define MAX_DP_NRULES 1000
typedef struct
{
    uint64_t   pkt;
    detection_option_tree_root_t * tree;
    PPM_TICKS ticks;

} ppm_rules_t;

/* suspended rules */
static ppm_rules_t ppm_rules[MAX_DP_NRULES];
static int ppm_n_rules;

/* cleared rules - re-enabled */
static detection_option_tree_root_t * ppm_crules[MAX_DP_NRULES];
static int ppm_n_crules;

void ppm_init_rules(void)
{
    ppm_n_rules = 0;
    ppm_n_crules = 0;
}

void ppm_set_rule(detection_option_tree_root_t * root,PPM_TICKS ticks)
{
    if( ppm_n_rules < MAX_DP_NRULES )
    {
        ppm_rules[ppm_n_rules].tree=root;
        ppm_rules[ppm_n_rules].ticks=ticks;
        ppm_n_rules++;
    }
}

void ppm_clear_rule(detection_option_tree_root_t *root)
{
    if( ppm_n_crules < MAX_DP_NRULES )
    {
        ppm_crules[ppm_n_crules++]=root;
    }
}

/*
 * calc ticks per micro-secs in integer units
 */
static int ppm_calc_ticks(void)
{
    ppm_tpu = (PPM_TICKS)get_ticks_per_usec();

    if( ppm_tpu == 0 )
    {
        return -1;
    }

    return 0;
}
void ppm_print_cfg(ppm_cfg_t *ppm_cfg)
{
    if (ppm_cfg == NULL)
        return;

    if (!ppm_cfg->enabled)
        return ;

    if( ppm_cfg->max_pkt_ticks )
    {
        LogMessage("\n");
        LogMessage("Packet Performance Monitor Config:\n");
        LogMessage("  ticks per usec  : %lu ticks\n",(unsigned long)ppm_tpu);

        LogMessage("  max packet time : %lu usecs\n",(unsigned long)(ppm_cfg->max_pkt_ticks/ppm_tpu));
        LogMessage("  packet action   : ");
        if( ppm_cfg->pkt_action )
            LogMessage("fastpath-expensive-packets\n");
        else
            LogMessage("none\n");
        LogMessage("  packet logging  : %s\n", ppm_cfg->pkt_log ? "log" : "none");
        LogMessage("  debug-pkts      : %s\n",(ppm_cfg->debug_pkts)? "enabled":"disabled");
    }

    if( ppm_cfg->max_rule_ticks)
    {
        LogMessage("\n");
        LogMessage("Rule Performance Monitor Config:\n");
        LogMessage("  ticks per usec  : %lu ticks\n",(unsigned long)ppm_tpu);

        LogMessage("  max rule time   : %lu usecs\n",(unsigned long)(ppm_cfg->max_rule_ticks/ppm_tpu));
        LogMessage("  rule action     : ");
        if( ppm_cfg->rule_action )
        {
            LogMessage("suspend-expensive-rules\n");
            LogMessage("  rule threshold  : %u \n",(unsigned int)ppm_cfg->rule_threshold);
        }
        else
            LogMessage("none\n");

#ifdef PPM_TEST
        /* use usecs instead of ticks for rule suspension during pcap playback */
        LogMessage("  suspend timeout : %lu secs\n", (unsigned long)(ppm_cfg->max_suspend_ticks/((uint64_t)1000000)) );
#else
        LogMessage("  suspend timeout : %lu secs\n", (unsigned long)(ppm_cfg->max_suspend_ticks/((uint64_t)ppm_tpu*1000000)) );
#endif
        LogMessage("  rule logging    : ");
        if(ppm_cfg->rule_log&PPM_LOG_ALERT) LogMessage("alert " );
        if(ppm_cfg->rule_log&PPM_LOG_MESSAGE) LogMessage("log ");
        if(!ppm_cfg->rule_log) LogMessage("none ");
        LogMessage("\n");
        /*LogMessage("  debug-rules     : %s\n",(ppm_cfg->debug_rules)?"enabled":"disabled"); unsupported */
    }
}

static
int print_rule( int proto, RuleTreeNode * r, OptTreeNode * o )
{
    if( o->rule_state != RULE_STATE_ENABLED )
    {
        //if( o->sigInfo.generator==1 || o->sigInfo.generator==3 )
        LogMessage("   disabled gid=%u, sid=%u\n",o->sigInfo.generator,o->sigInfo.id);
    }

    return 0;
}

void ppm_print_summary(ppm_cfg_t *ppm_cfg)
{
    if (ppm_cfg == NULL)
        return;

    if (!ppm_cfg->enabled)
        return;

    LogMessage("===============================================================================\n");
    if(ppm_cfg->max_pkt_ticks)
    {
        LogMessage("Packet Performance Summary:\n");

        LogMessage("   max packet time       : %g usecs\n",
            ppm_ticks_to_usecs(ppm_cfg->max_pkt_ticks));

        LogMessage("   packet events         : %u\n",
            (unsigned int)ppm_cfg->pkt_event_cnt);

        if( ppm_cfg->tot_pkts )
            LogMessage("   avg pkt time          : %g usecs\n",
                ppm_ticks_to_usecs((PPM_TICKS)(ppm_cfg->tot_pkt_time/
                    ppm_cfg->tot_pkts)));
    }

    if(ppm_cfg->max_rule_ticks)
    {
        LogMessage("Rule Performance Summary:\n");

        LogMessage("   max rule time         : %lu usecs\n",
            (unsigned long)(ppm_cfg->max_rule_ticks/ppm_tpu));

        LogMessage("   rule events           : %u\n",
            (unsigned int)ppm_cfg->rule_event_cnt);

        if( ppm_cfg->tot_rules )
            LogMessage("   avg rule time         : %g usecs\n",
                ppm_ticks_to_usecs((PPM_TICKS)(ppm_cfg->tot_rule_time/
                    ppm_cfg->tot_rules)));

        if( ppm_cfg->tot_nc_rules )
            LogMessage("   avg nc-rule time      : %g usecs\n",
                ppm_ticks_to_usecs((PPM_TICKS)(ppm_cfg->tot_nc_rule_time/
                    ppm_cfg->tot_nc_rules)));

        if( ppm_cfg->tot_pcre_rules )
            LogMessage("   avg nc-pcre-rule time : %g usecs\n",
                ppm_ticks_to_usecs((PPM_TICKS)(ppm_cfg->tot_pcre_rule_time/
                    ppm_cfg->tot_pcre_rules)));

        fpWalkOtns( 0, print_rule );
    }
}

double ppm_ticks_to_usecs(PPM_TICKS ticks)
{
    if (ppm_tpu > 0)
        return (double)ticks / ppm_tpu;

    return 0.0;
}

/*
 *  Initialization
 */
void ppm_init(ppm_cfg_t *ppm_cfg)
{
    /* calc ticks per usec */
    if (ppm_calc_ticks() == -1)
        return;

    ppm_cfg->enabled = 1;

    ppm_cfg->max_pkt_ticks = PPM_DEFAULT_MAX_PKT_TICKS;
    ppm_cfg->max_rule_ticks = PPM_DEFAULT_MAX_RULE_TICKS;

    /* use usecs instead of ticks for rule suspension during pcap playback */
    ppm_cfg->max_suspend_ticks = (uint64_t)PPM_DEFAULT_MAX_SUSP_SECS * 1000000;
#ifndef PPM_TEST
    ppm_cfg->max_suspend_ticks *= ppm_tpu;
#endif
    ppm_cfg->rule_threshold = PPM_DEFAULT_RULE_THRESHOLD;
}

/*
 *  Logging functions - syslog and/or events
 */
#define PPM_FMT_FASTPATH "PPM: Pkt-Event Pkt[" STDi64 "] used=%g usecs, %u rules, %u nc-rules tested, packet fastpathed.\n"
#define PPM_FMT_PACKET   "PPM: Pkt-Event Pkt[" STDi64 "] used=%g usecs, %u rules, %u nc-rules tested.\n"

void ppm_pkt_log(ppm_cfg_t *ppm_cfg)
{
    if (!ppm_cfg->max_pkt_ticks)
        return;

    ppm_cfg->pkt_event_cnt++;

    if (ppm_cfg->pkt_log)
    {
        if (ppm_abort_this_pkt)
        {
            LogMessage(PPM_FMT_FASTPATH,
                       ppm_pt->pktcnt,
                       ppm_ticks_to_usecs((PPM_TICKS)ppm_pt->tot),
                       ppm_pt->rule_tests,
                       ppm_pt->nc_rule_tests );
        }
        else
        {
            LogMessage(PPM_FMT_PACKET,
                       ppm_pt->pktcnt,
                       ppm_ticks_to_usecs((PPM_TICKS)ppm_pt->tot),
                       ppm_pt->rule_tests,
                       ppm_pt->nc_rule_tests );
        }
    }
}

#define PPM_FMT_SUSPENDED "PPM: Rule-Event address=0x%p Pkt[" STDi64 "] used=%g usecs suspended %s"
#define PPM_FMT_REENABLED "PPM: Rule-Event address=0x%p Pkt[" STDi64 "] re-enabled %s"

void ppm_rule_log(ppm_cfg_t *ppm_cfg, uint64_t pktcnt, Packet *p)
{
    //char buf[STD_BUF];
    detection_option_tree_root_t *proot;
    OptTreeNode * potn;
    char timestamp[TIMEBUF_SIZE];
    *timestamp = '\0';

    if (!ppm_cfg->max_rule_ticks)
        return ;

    if (ppm_n_crules)
    {
        if (ppm_cfg->rule_log & PPM_LOG_ALERT)
        {
            Event ev;

            /* make sure we have an otn already in our table for this event */
            potn = OtnLookup(snort_conf->otn_map, GENERATOR_PPM, PPM_EVENT_RULE_TREE_ENABLED);
            if (potn == NULL)
            {
                /* have to make one */
                potn = GenerateSnortEventOtn(GENERATOR_PPM, /* GID */
                                             PPM_EVENT_RULE_TREE_ENABLED, /* SID */
                                             1, /* Rev */
                                             0, /* classification */
                                             3, /* priority (low) */
                                             PPM_EVENT_RULE_TREE_ENABLED_STR /* msg string */);
                if (potn == NULL)
                    return;

                /* add it */
                OtnLookupAdd(snort_conf->otn_map, potn);
            }

            SetEvent(&ev,
                     potn->sigInfo.generator, /* GID */
                     potn->sigInfo.id, /* SID */
                     potn->sigInfo.rev, /* Rev */
                     potn->sigInfo.class_id, /* classification */
                     potn->sigInfo.priority, /* priority (low) */
                     0);

            AlertAction(p, potn, &ev);
        }

        if (ppm_cfg->rule_log & PPM_LOG_MESSAGE) 
        {
            int i;

            if(!*timestamp)
                ts_print((struct timeval*)&p->pkth->ts, timestamp);

            for (i=0; i< ppm_n_crules; i++)
            {
                proot = ppm_crules[i];

                LogMessage(PPM_FMT_REENABLED,
                           (void*)proot,
                           pktcnt,
                           timestamp);
            }
        }

        ppm_n_crules = 0;
    }

    if (ppm_n_rules)
    {
        if (ppm_cfg->rule_log & PPM_LOG_ALERT)
        {
            Event ev;

            /* make sure we have an otn already in our table for this event */
            potn = OtnLookup(snort_conf->otn_map, GENERATOR_PPM, PPM_EVENT_RULE_TREE_DISABLED);
            if (potn == NULL)
            {
                /* have to make one */
                potn = GenerateSnortEventOtn(GENERATOR_PPM, /* GID */
                                             PPM_EVENT_RULE_TREE_DISABLED, /* SID */
                                             1, /* Rev */
                                             0, /* classification */
                                             3, /* priority (low) */
                                             PPM_EVENT_RULE_TREE_DISABLED_STR /* msg string */);

                if (potn == NULL)
                    return;

                OtnLookupAdd(snort_conf->otn_map, potn);
            }

            SetEvent(&ev,
                     potn->sigInfo.generator, /* GID */
                     potn->sigInfo.id, /* SID */
                     potn->sigInfo.rev, /* Rev */
                     potn->sigInfo.class_id, /* classification */
                     potn->sigInfo.priority, /* priority (low) */
                     0);

            AlertAction(p, potn, &ev);
        }

        if (ppm_cfg->rule_log & PPM_LOG_MESSAGE)
        {
            int i;

            if(!*timestamp)
                ts_print((struct timeval*)&p->pkth->ts, timestamp);

            for (i=0; i< ppm_n_rules; i++)
            {
                proot = ppm_rules[i].tree;

                LogMessage(PPM_FMT_SUSPENDED,
                           (void*)proot,
                           pktcnt,
                           ppm_ticks_to_usecs((PPM_TICKS)ppm_rules[i].ticks),
                           timestamp);
            }
        }

        ppm_n_rules = 0;
    }
}

void ppm_set_rule_event(ppm_cfg_t *ppm_cfg, detection_option_tree_root_t *root)
{
    if (!ppm_cfg->max_rule_ticks)
        return;

    ppm_cfg->rule_event_cnt++;

    if (ppm_cfg->rule_log && ppm_rt)
        ppm_set_rule(root, ppm_rt->tot);
}

void ppm_clear_rule_event(ppm_cfg_t *ppm_cfg, detection_option_tree_root_t *root)
{
    if (!ppm_cfg->max_rule_ticks)
        return;

    ppm_cfg->rule_event_cnt++;

    if (ppm_cfg->rule_log)
        ppm_clear_rule(root);
}


/*
 * Config functions
 */

void ppm_set_pkt_action(ppm_cfg_t *ppm_cfg, int flag)
{
    ppm_cfg->pkt_action = flag;
}

void ppm_set_pkt_log(ppm_cfg_t *ppm_cfg, int flag)
{
    ppm_cfg->pkt_log |= flag;
}

void ppm_set_rule_action(ppm_cfg_t *ppm_cfg, int flag)
{
    ppm_cfg->rule_action = flag;
}

void ppm_set_rule_log(ppm_cfg_t *ppm_cfg, int flag)
{
    ppm_cfg->rule_log |= flag;
}

void ppm_set_max_pkt_time(ppm_cfg_t *ppm_cfg, PPM_USECS usecs)
{
    ppm_cfg->max_pkt_ticks = usecs * ppm_tpu;
}

void ppm_set_max_rule_time(ppm_cfg_t *ppm_cfg, PPM_USECS usecs)
{
    ppm_cfg->max_rule_ticks = usecs * ppm_tpu;
}

void ppm_set_max_suspend_time(ppm_cfg_t *ppm_cfg, PPM_SECS secs)
{
    /* use usecs instead of ticks for rule suspension during pcap playback */
    ppm_cfg->max_suspend_ticks = (uint64_t)secs * 1000000;
#ifndef PPM_TEST
    ppm_cfg->max_suspend_ticks *= ppm_tpu;
#endif
}

void ppm_set_rule_threshold(ppm_cfg_t *ppm_cfg, unsigned int cnt)
{
    ppm_cfg->rule_threshold = cnt;
}

void ppm_set_debug_rules(ppm_cfg_t *ppm_cfg, int flag)
{
    ppm_cfg->debug_rules = flag;
}

void ppm_set_debug_pkts(ppm_cfg_t *ppm_cfg, int flag)
{
    ppm_cfg->debug_pkts = flag;
}

#endif

