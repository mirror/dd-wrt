/*
**  $Id$
**
**  profiler.c
**
**  Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
**  Copyright (C) 2005-2013 Sourcefire, Inc.
**  Steven Sturges <ssturges@sourcefire.com>
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
**  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "snort.h"
#include "rules.h"
#include "treenodes.h"
#include "treenodes.h"
#include "parser.h"
#include "plugin_enum.h"
#include "util.h"
#include "rules.h"
#include "treenodes.h"
#include "treenodes.h"
#include "profiler.h"
#include "sf_types.h"
#include "sf_textlog.h"
#include "detection_options.h"

#ifdef PERF_PROFILING

/* Data types *****************************************************************/
typedef struct _OTN_WorstPerformer
{
    OptTreeNode *otn;
    struct _OTN_WorstPerformer *next;
    struct _OTN_WorstPerformer *prev;
    double ticks_per_check;
    double ticks_per_match;
    double ticks_per_nomatch;

} OTN_WorstPerformer;

typedef struct _Preproc_WorstPerformer
{
    PreprocStatsNode *node;
    struct _Preproc_WorstPerformer *next;
    struct _Preproc_WorstPerformer *prev;
    struct _Preproc_WorstPerformer *children;
    double ticks_per_check;
    double pct_of_parent;
    double pct_of_total;
} Preproc_WorstPerformer;


/* Globals ********************************************************************/
double ticks_per_microsec = 0.0;
OTN_WorstPerformer *worstPerformers = NULL;

Preproc_WorstPerformer *worstPreprocPerformers = NULL;
PreprocStats totalPerfStats;
PreprocStats metaPerfStats;
static PreprocStatsNode * PreprocStatsNodeList = NULL;
int max_layers = 0;


/* Externs ********************************************************************/
extern PreprocStats mpsePerfStats, rulePerfStats, ncrulePerfStats;


void getTicksPerMicrosec(void)
{
    if (ticks_per_microsec == 0.0)
    {
        ticks_per_microsec = get_ticks_per_usec();
    }
}

void ResetRuleProfiling(void)
{
    /* Cycle through all Rules, print ticks & check count for each */
    RuleTreeNode *rtn;
    SFGHASH_NODE *hashNode;
    OptTreeNode *otn  = NULL;
    tSfPolicyId policyId = 0;
    SnortConfig *sc = snort_conf;

    if ((sc == NULL) || (sc->profile_rules.num == 0))
        return;

    for (hashNode = sfghash_findfirst(sc->otn_map);
            hashNode;
            hashNode = sfghash_findnext(sc->otn_map))
    {
        otn = (OptTreeNode *)hashNode->data;
        for ( policyId = 0;
              policyId < otn->proto_node_num;
              policyId++ )
        {
            rtn = getRtnFromOtn(otn, policyId);
            if (rtn == NULL)
                continue;

            if ((rtn->proto == IPPROTO_TCP) || (rtn->proto == IPPROTO_UDP)
                    || (rtn->proto == IPPROTO_ICMP) || (rtn->proto == ETHERNET_TYPE_IP))
            {
                //do operation
                otn->ticks = 0;
                otn->ticks_match = 0;
                otn->ticks_no_match = 0;
                otn->checks = 0;
                otn->matches = 0;
                otn->alerts = 0;
                otn->noalerts = 0;
#ifdef PPM_MGR
                otn->ppm_disable_cnt = 0;
#endif
            }
        }
    }
}

void PrintWorstRules(int numToPrint)
{
    OptTreeNode *otn;
    OTN_WorstPerformer *node, *tmp;
    int num = 0;
    TextLog *log = NULL;
    time_t cur_time;
    char fullname[STD_BUF];
    int ret;
    SnortConfig *sc = snort_conf;

    if (sc == NULL)
        return;

    getTicksPerMicrosec();

    cur_time = time(NULL);

    if (sc->profile_rules.filename != NULL)
    {
        if (sc->profile_rules.append)
        {
            log = TextLog_Init(sc->profile_rules.filename, 512*1024, 512*1024);

            if (log != NULL)
                TextLog_Print(log, "\ntimestamp: %u\n", cur_time);
        }
        else
        {
            ret = SnortSnprintf(fullname, STD_BUF, "%s.%u", sc->profile_rules.filename, (uint32_t)cur_time);
            if(ret != SNORT_SNPRINTF_SUCCESS)
                FatalError("profiler: file path+name too long\n");
            log = TextLog_Init(fullname, 512*1024, 512*1024);
        }
    }

    if (numToPrint != -1)
    {
        if(log)
        {
            TextLog_Print(log, "Rule Profile Statistics (worst %d rules)\n", numToPrint);
        } else {
            LogMessage("Rule Profile Statistics (worst %d rules)\n", numToPrint);
        }
    }
    else
    {
        if(log)
        {
            TextLog_Print(log, "Rule Profile Statistics (all rules)\n");
        } else {
            LogMessage("Rule Profile Statistics (all rules)\n");
        }
    }

    if(log)
    {
        TextLog_Print(log, "==========================================================\n");
    } else {
        LogMessage("==========================================================\n");
    }

    if (!worstPerformers)
    {
        if(log)
        {
            TextLog_Print(log, "No rules were profiled\n");
            TextLog_Term(log);
        } else {
            LogMessage("No rules were profiled\n");
        }
        return;
    }

    if(log)
    {
        TextLog_Print(log,
#ifdef PPM_MGR
            "%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s\n",
#else
            "%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s\n",
#endif
             6, "Num",
             9, "SID", 4, "GID", 4, "Rev",
            11, "Checks",
            10, "Matches",
            10, "Alerts",
            20, "Microsecs",
            11, "Avg/Check",
            11, "Avg/Match",
            13, "Avg/Nonmatch"
#ifdef PPM_MGR
            , 11, "Disabled"
#endif
            );
    }
    else
    {
        LogMessage(
#ifdef PPM_MGR
            "%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s\n",
#else
            "%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s\n",
#endif
             6, "Num",
             9, "SID", 4, "GID", 4, "Rev",
            11, "Checks",
            10, "Matches",
            10, "Alerts",
            20, "Microsecs",
            11, "Avg/Check",
            11, "Avg/Match",
            13, "Avg/Nonmatch"
#ifdef PPM_MGR
            , 11, "Disabled"
#endif
            );
    }

    if(log)
    {
        TextLog_Print(log,
#ifdef PPM_MGR
            "%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s\n",
#else
            "%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s\n",
#endif
            6, "===",
            9, "===", 4, "===", 4, "===",
            11, "======",
            10, "=======",
            10, "======",
            20, "=========",
            11, "=========",
            11, "=========",
            13, "============"
#ifdef PPM_MGR
            , 11, "========"
#endif
            );
    }
    else
    {
        LogMessage(
#ifdef PPM_MGR
            "%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s\n",
#else
            "%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s\n",
#endif
            6, "===",
            9, "===", 4, "===", 4, "===",
            11, "======",
            10, "=======",
            10, "======",
            20, "=========",
            11, "=========",
            11, "=========",
            13, "============"
#ifdef PPM_MGR
            , 11, "========"
#endif
            );
    }

    for (node = worstPerformers, num=1;
         node && ((numToPrint < 0) ? 1 : (num <= numToPrint));
         node= node->next, num++)
    {
        //if (!node)
        //    break;
        otn = node->otn;

        if(log)
        {
            TextLog_Print(log,
#ifdef PPM_MGR
                "%*d%*d%*d%*d" FMTu64("*") FMTu64("*") FMTu64("*") FMTu64("*") "%*.1f%*.1f%*.1f" FMTu64("*") "\n",
#else
                "%*d%*d%*d%*d" FMTu64("*") FMTu64("*") FMTu64("*") FMTu64("*") "%*.1f%*.1f%*.1f" "\n",
#endif
                6, num, 9, otn->sigInfo.id, 4, otn->sigInfo.generator, 4, otn->sigInfo.rev,
                11, otn->checks,
                10, otn->matches,
                10, otn->alerts,
                20, (uint64_t)(otn->ticks/ticks_per_microsec),
                11, node->ticks_per_check/ticks_per_microsec,
                11, node->ticks_per_match/ticks_per_microsec,
                13, node->ticks_per_nomatch/ticks_per_microsec
#ifdef PPM_MGR
                , 11, otn->ppm_disable_cnt
#endif
                );
        }
        else
        {
            LogMessage(
#ifdef PPM_MGR
                "%*d%*d%*d%*d" FMTu64("*") FMTu64("*") FMTu64("*") FMTu64("*") "%*.1f%*.1f%*.1f" FMTu64("*") "\n",
#else
                "%*d%*d%*d%*d" FMTu64("*") FMTu64("*") FMTu64("*") FMTu64("*") "%*.1f%*.1f%*.1f" "\n",
#endif
                6, num, 9, otn->sigInfo.id, 4, otn->sigInfo.generator, 4, otn->sigInfo.rev,
                11, otn->checks,
                10, otn->matches,
                10, otn->alerts,
                20, (uint64_t)(otn->ticks/ticks_per_microsec),
                11, node->ticks_per_check/ticks_per_microsec,
                11, node->ticks_per_match/ticks_per_microsec,
                13, node->ticks_per_nomatch/ticks_per_microsec
#ifdef PPM_MGR
                , 11, otn->ppm_disable_cnt
#endif
                );
        }
    }

    /* Do some cleanup */
    for (node = worstPerformers; node; )
    {
        tmp = node->next;
        free(node);
        node = tmp;
    }

    if(log)
        TextLog_Term(log);
    worstPerformers = NULL;
}

void CollectRTNProfile(void)
{
    OptTreeNode *otn;
    OTN_WorstPerformer *new, *node, *last = NULL;
    char got_position;
    SFGHASH_NODE *hashNode;
    SnortConfig *sc = snort_conf;

    if (sc == NULL)
        return;

    for (hashNode = sfghash_findfirst(sc->otn_map);
            hashNode;
            hashNode = sfghash_findnext(sc->otn_map))
    {
        otn = (OptTreeNode *)hashNode->data;

        /* Only log info if OTN has actually been eval'd */
        if (otn->checks > 0 && otn->ticks > 0)
        {
            double ticks_per_check = (double)otn->ticks/(double)otn->checks;
            double ticks_per_nomatch;
            double ticks_per_match;

            if (otn->matches > otn->checks)
                otn->checks = otn->matches;

            if (otn->matches)
                ticks_per_match = (double)otn->ticks_match/(double)otn->matches;
            else
                ticks_per_match = 0.0;

            if (otn->checks == otn->matches)
                ticks_per_nomatch = 0.0;
            else
                ticks_per_nomatch = (double)otn->ticks_no_match/(double)(otn->checks - otn->matches);

            /* Find where he goes in the list
             * Cycle through the list and add
             * this where it goes
             */
            new = (OTN_WorstPerformer *)SnortAlloc(sizeof(OTN_WorstPerformer));
            new->otn = otn;
            new->ticks_per_check = ticks_per_check;
            new->ticks_per_match = ticks_per_match;
            new->ticks_per_nomatch = ticks_per_nomatch;

            got_position = 0;

            for (node = worstPerformers; node && !got_position; node = node->next)
            {
                last = node;
                switch (sc->profile_rules.sort)
                {
                    case PROFILE_SORT_CHECKS:
                        if (otn->checks >= node->otn->checks)
                        {
                            got_position = 1;
                        }
                        break;
                    case PROFILE_SORT_MATCHES:
                        if (otn->matches >= node->otn->matches)
                        {
                            got_position = 1;
                        }
                        break;
                    case PROFILE_SORT_NOMATCHES:
                        if (otn->checks - otn->matches >
                                node->otn->checks - node->otn->matches)
                        {
                            got_position = 1;
                        }
                        break;
                    case PROFILE_SORT_AVG_TICKS_PER_MATCH:
                        if (ticks_per_match >= node->ticks_per_match)
                        {
                            got_position = 1;
                        }
                        break;
                    case PROFILE_SORT_AVG_TICKS_PER_NOMATCH:
                        if (ticks_per_nomatch >= node->ticks_per_nomatch)
                        {
                            got_position = 1;
                        }
                        break;
                    case PROFILE_SORT_TOTAL_TICKS:
                        if (otn->ticks >= node->otn->ticks)
                        {
                            got_position = 1;
                        }
                        break;
                    default:
                    case PROFILE_SORT_AVG_TICKS:
                        if (ticks_per_check >= node->ticks_per_check)
                        {
                            got_position = 1;
                        }
                        break;
                }
                if (got_position)
                    break;
            }

            if (node)
            {
                new->next = node;
                new->prev = node->prev;
                node->prev = new;
                if (new->prev)
                    new->prev->next = new;
                /* Reset the head of list */
                if (node == worstPerformers)
                    worstPerformers = new;
            }
            else
            {
                if (!last)
                {
                    worstPerformers = new;
                }
                else
                {
                    new->prev = last;
                    last->next = new;
                }
            }
        }
    }
}


void ShowRuleProfiles(void)
{
    /* Cycle through all Rules, print ticks & check count for each */
    SnortConfig *sc = snort_conf;

    if ((sc == NULL) || (sc->profile_rules.num == 0))
        return;

    detection_option_tree_update_otn_stats(sc->detection_option_tree_hash_table);

    CollectRTNProfile();

    /* Specifically call out a top xxx or something? */
    PrintWorstRules(sc->profile_rules.num);
    return;
}

/* The preprocessor profile list is only accessed for printing stats when
 * Snort shuts down, so adding new nodes during a reload shouldn't be a
 * problem. */
void RegisterPreprocessorProfile(const char *keyword, PreprocStats *stats, int layer, PreprocStats *parent, StatsNodeFreeFunc freefn)
{
    PreprocStatsNode *node;

    if (stats == NULL)
        return;

    node = (PreprocStatsNode *)SnortAlloc(sizeof(PreprocStatsNode));

    if (PreprocStatsNodeList == NULL)
    {
        PreprocStatsNodeList = node;
    }
    else
    {
        PreprocStatsNode *tmp = PreprocStatsNodeList;
        PreprocStatsNode *last;

        do
        {
            if (strcasecmp(tmp->name, keyword) == 0)
            {
                //FatalError("Duplicate Preprocessor Stats Name (%s)\n", keyword);
                /* Don't fatal error here since during a reload there are
                 * probably going to be dups - just return */
                //multiple policy support
                free(node);
                return;
            }

            last = tmp;
            tmp = tmp->next;

        } while (tmp != NULL);

        last->next = node;
    }

    node->name = SnortStrdup(keyword);
    node->stats = stats;  /* Set the stats reference */
    node->parent = parent;
    node->layer = layer;
    node->freefn = freefn;

    if (layer > max_layers)
        max_layers = layer;
}

void FreePreprocPerformance(Preproc_WorstPerformer *idx)
{
    Preproc_WorstPerformer *child, *tmp;
    child = idx->children;
    while (child)
    {
        FreePreprocPerformance(child);
        tmp = child;
        child = child->next;
        free(tmp);
    }
}

void PrintPreprocPerformance(TextLog *log, int num, Preproc_WorstPerformer *idx)
{
    Preproc_WorstPerformer *child;
    int i;
    /* indent 'Num' based on the layer */
    unsigned int indent = 6 - (5 - idx->node->layer);

    if (num != 0)
    {
        indent += 2;
        if(log)
        {
            TextLog_Print(log, "%*d%*s%*d" FMTu64("*") FMTu64("*") FMTu64("*") "%*.2f%*.2f%*.2f\n",
                   indent, num,
                   28 - indent, idx->node->name, 6, idx->node->layer,
                   11, idx->node->stats->checks,
                   11, idx->node->stats->exits,
                   20, (uint64_t)(idx->node->stats->ticks/ticks_per_microsec),
                   11, idx->ticks_per_check/ticks_per_microsec,
                   14, idx->pct_of_parent,
                   13, idx->pct_of_total);
        }
        else
        {
            LogMessage("%*d%*s%*d" FMTu64("*") FMTu64("*") FMTu64("*") "%*.2f%*.2f%*.2f\n",
        	                   indent, num,
        	                   28 - indent, idx->node->name, 6, idx->node->layer,
        	                   11, idx->node->stats->checks,
        	                   11, idx->node->stats->exits,
        	                   20, (uint64_t)(idx->node->stats->ticks/ticks_per_microsec),
        	                   11, idx->ticks_per_check/ticks_per_microsec,
        	                   14, idx->pct_of_parent,
        	                   13, idx->pct_of_total);
        }
    }
    else
    {
        /* The totals */
        indent += strlen(idx->node->name);

        if(log)
        {
            TextLog_Print(log, "%*s%*s%*d" FMTu64("*") FMTu64("*") FMTu64("*") "%*.2f%*.2f%*.2f\n",
                   indent, idx->node->name,
                   28 - indent, idx->node->name, 6, idx->node->layer,
                   11, idx->node->stats->checks,
                   11, idx->node->stats->exits,
                   20, (uint64_t)(idx->node->stats->ticks/ticks_per_microsec),
                   11, idx->ticks_per_check/ticks_per_microsec,
                   14, idx->pct_of_parent,
                   13, idx->pct_of_parent);
        }
        else
        {
            LogMessage("%*s%*s%*d" FMTu64("*") FMTu64("*") FMTu64("*") "%*.2f%*.2f%*.2f\n",
        	                   indent, idx->node->name,
        	                   28 - indent, idx->node->name, 6, idx->node->layer,
        	                   11, idx->node->stats->checks,
        	                   11, idx->node->stats->exits,
        	                   20, (uint64_t)(idx->node->stats->ticks/ticks_per_microsec),
        	                   11, idx->ticks_per_check/ticks_per_microsec,
        	                   14, idx->pct_of_parent,
        	                   13, idx->pct_of_parent);
        }
    }

    child = idx->children;

    i = 1;
    while (child)
    {
        PrintPreprocPerformance(log, i++, child);
        child = child->next;
    }
}

void CleanupPreprocStatsNodeList(void)
{
    PreprocStatsNode *node, *nxt;

    node = PreprocStatsNodeList;
    while (node)
    {
        nxt = node->next;
        if (node->freefn)
            node->freefn(node->stats);
        free(node->name);
        free(node);
        node = nxt;
    }
    PreprocStatsNodeList = NULL;
}

void CleanupPreprocPerformance(Preproc_WorstPerformer *worst)
{
    Preproc_WorstPerformer *idx, *child, *tmp;

    idx = worst;
    while (idx)
    {
        tmp = idx->next;
        child = idx->children;
        CleanupPreprocPerformance(child);

        free(idx);
        idx = tmp;
    }
}

void PrintWorstPreprocs(int numToPrint)
{
    Preproc_WorstPerformer *idx;
    Preproc_WorstPerformer *total = NULL;
    int num = 0;
    TextLog *log = NULL;
    time_t cur_time;
    char fullname[STD_BUF];
    int ret;
    SnortConfig *sc = snort_conf;

    getTicksPerMicrosec();

    cur_time = time(NULL);
    if (sc->profile_preprocs.filename != NULL)
    {
        if (sc->profile_preprocs.append)
        {
            log = TextLog_Init(sc->profile_preprocs.filename, 512*1024, 512*1024);

            if (log != NULL)
                TextLog_Print(log, "\ntimestamp: %u\n", cur_time);
        }
        else
        {
            ret = SnortSnprintf(fullname, STD_BUF, "%s.%u", sc->profile_preprocs.filename, (uint32_t)cur_time);
            if(ret != SNORT_SNPRINTF_SUCCESS)
                FatalError("profiler: file path+name too long\n");
            log = TextLog_Init(fullname, 512*1024, 512*1024);
        }
    }

    if (numToPrint != -1)
    {
        if(log)
        {
            TextLog_Print(log, "Preprocessor Profile Statistics (worst %d)\n", numToPrint);
        }
        else
        {
            LogMessage("Preprocessor Profile Statistics (worst %d)\n", numToPrint);
        }
    }
    else
    {
        if(log)
        {
            TextLog_Print(log, "Preprocessor Profile Statistics (all)\n");
        }
        else
        {
            LogMessage("Preprocessor Profile Statistics (all)\n");
        }
    }

    if(log)
    {
        TextLog_Print(log, "==========================================================\n");
    }
    else
    {
        LogMessage("==========================================================\n");
    }

    if (!worstPreprocPerformers)
    {
        if(log)
        {
            TextLog_Print(log, "No Preprocessors were profiled\n");
            TextLog_Term(log);
        }
        else
        {
            LogMessage("No Preprocessors were profiled\n");
        }
        CleanupPreprocPerformance(worstPreprocPerformers);
        worstPreprocPerformers = NULL;
        return;
    }

    if(log)
    {
        TextLog_Print(log, "%*s%*s%*s%*s%*s%*s%*s%*s%*s\n",
            4, "Num",
            24, "Preprocessor",
            6, "Layer",
            11, "Checks",
            11, "Exits",
            20, "Microsecs",
            11, "Avg/Check",
            14, "Pct of Caller",
            13, "Pct of Total");
    }
    else
    {
        LogMessage("%*s%*s%*s%*s%*s%*s%*s%*s%*s\n",
    	            4, "Num",
    	            24, "Preprocessor",
    	            6, "Layer",
    	            11, "Checks",
    	            11, "Exits",
    	            20, "Microsecs",
    	            11, "Avg/Check",
    	            14, "Pct of Caller",
    	            13, "Pct of Total");
    }

    if(log)
    {
        TextLog_Print(log, "%*s%*s%*s%*s%*s%*s%*s%*s%*s\n",
            4, "===",
            24, "============",
            6, "=====",
            11, "======",
            11, "=====",
            20, "=========",
            11, "=========",
            14, "=============",
            13, "============");
    }
    else
    {
        LogMessage("%*s%*s%*s%*s%*s%*s%*s%*s%*s\n",
    	            4, "===",
    	            24, "============",
    	            6, "=====",
    	            11, "======",
    	            11, "=====",
    	            20, "=========",
    	            11, "=========",
    	            14, "=============",
    	            13, "============");
    }

    for (idx = worstPreprocPerformers, num=1;
         idx && ((numToPrint < 0) ? 1 : (num <= numToPrint));
         idx= idx->next, num++)
    {
        /* Skip the total counter */
        if (idx->node->stats == &totalPerfStats)
        {
            num--;
            total = idx;
            continue;
        }
        //if (!idx)
        //    break;
        PrintPreprocPerformance(log, num, idx);
        //LogMessage("%*d%*s%*d%*d" FMTu64("*") "%*.1f%*.1f\n",
        //    6, num, 20, idx->node->name, 6, idx->node->layer,
        //    11, idx->node->stats->checks,
        //    11, idx->node->stats->exits,
        //    20, idx->node->stats->ticks,
        //    11, idx->ticks_per_check,
        //    14, idx->pct_of_parent,
        //    14, idx->pct_of_total);
    }
    if (total)
        PrintPreprocPerformance(log, 0, total);

    if(log)
        TextLog_Term(log);
    CleanupPreprocPerformance(worstPreprocPerformers);
    worstPreprocPerformers = NULL;
}

Preproc_WorstPerformer *findPerfParent(PreprocStatsNode *node,
                                       Preproc_WorstPerformer *top)
{
    Preproc_WorstPerformer *list = top;
    Preproc_WorstPerformer *parent;

    if (!list)
        return NULL;

    if (list->node->layer > node->layer)
        return NULL;

    while (list)
    {
        if (list->node->stats == node->parent)
        {
            parent = list;
            return parent;
        }

        parent = findPerfParent(node, list->children);

        if (parent)
            return parent;

        list = list->next;
    }

    return NULL;
}

void ResetPreprocProfiling(void)
{
    PreprocStatsNode *idx = NULL;
    SnortConfig *sc = snort_conf;

    if ((sc == NULL) || (sc->profile_preprocs.num == 0))
        return;

    for (idx = PreprocStatsNodeList; idx != NULL; idx = idx->next)
    {
        idx->stats->ticks = 0;
        idx->stats->ticks_start = 0;
        idx->stats->checks = 0;
        idx->stats->exits = 0;
    }
}

void ShowPreprocProfiles(void)
{
    /* Cycle through all Rules, print ticks & check count for each */
    PreprocStatsNode *idx;
    int layer;
    Preproc_WorstPerformer *parent, *new, *this = NULL, *last = NULL;
    char got_position;
    Preproc_WorstPerformer *listhead;
    double ticks_per_check;
    SnortConfig *sc = snort_conf;

    if ((sc == NULL) || (sc->profile_preprocs.num == 0))
        return;

    /* Adjust mpse stats to not include rule evaluation */
    mpsePerfStats.ticks -= rulePerfStats.ticks;
    /* And adjust the rules to include the NC rules */
    rulePerfStats.ticks += ncrulePerfStats.ticks;

    for (layer=0;layer<=max_layers;layer++)
    {

        for (idx = PreprocStatsNodeList; idx; idx = idx->next)
        {
            if (idx->stats->checks == 0 || idx->stats->ticks == 0)
                continue;

            if (idx->layer != layer)
                continue;

            last = NULL;

            ticks_per_check = (double)idx->stats->ticks/(double)idx->stats->checks;

            new = (Preproc_WorstPerformer *)SnortAlloc(sizeof(Preproc_WorstPerformer));
            new->node = idx;
            new->ticks_per_check = ticks_per_check;

            if (idx->parent)
            {
                /* Find this idx's parent in the list */
                parent = findPerfParent(idx, worstPreprocPerformers);
                if (parent && (parent->node->stats != &totalPerfStats))
                {
                    listhead = parent->children;
                }
                else
                {
                    listhead = worstPreprocPerformers;
                    parent = NULL;
                }
                new->pct_of_parent = (double)idx->stats->ticks/idx->parent->ticks*100.0;
                new->pct_of_total = (double)idx->stats->ticks/totalPerfStats.ticks*100.0;
            }
            else
            {
                parent = NULL;
                new->pct_of_parent = 0.0;
                new->pct_of_total = 100.0;
                listhead = worstPreprocPerformers;
            }

            got_position = 0;

            for (this = listhead; this && !got_position; this = this->next)
            {
                last = this;
                switch (sc->profile_preprocs.sort)
                {
                    case PROFILE_SORT_CHECKS:
                        if (new->node->stats->checks >= this->node->stats->checks)
                        {
                            got_position = 1;
                        }
                        break;
                    case PROFILE_SORT_TOTAL_TICKS:
                        if (new->node->stats->ticks >= this->node->stats->ticks)
                        {
                            got_position = 1;
                        }
                        break;
                    default:
                    case PROFILE_SORT_AVG_TICKS:
                        if (new->ticks_per_check >= this->ticks_per_check)
                        {
                            got_position = 1;
                        }
                        break;
                }
                if (got_position)
                    break;
            }
            if (this)
            {
                new->next = this;
                new->prev = this->prev;
                this->prev = new;
                if (new->prev)
                    new->prev->next = new;
                /* Reset the head of the list */
                if (this == listhead)
                {
                    if (parent)
                    {
                        parent->children = new;
                    }
                    else
                    {
                        worstPreprocPerformers = new;
                    }
                }
            }
            else
            {
                if (!last)
                {
                    if (parent)
                    {
                        parent->children = new;
                    }
                    else
                    {
                        worstPreprocPerformers = new;
                    }
                }
                else
                {
                    new->prev = last;
                    last->next = new;
                }
            }
        }
    }
    PrintWorstPreprocs(sc->profile_preprocs.num);
    CleanupPreprocStatsNodeList();
}

#endif
