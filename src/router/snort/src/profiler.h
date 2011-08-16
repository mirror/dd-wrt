/*
** Copyright (C) 2005-2011 Sourcefire, Inc.
** Author: Steven Sturges <ssturges@sourcefire.com>
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

/* $Id$ */

#ifndef __PROFILER_H__
#define __PROFILER_H__

#ifdef PERF_PROFILING

#include "sf_types.h"
#include "cpuclock.h"

/* Sort preferences for rule profiling */
#define PROFILE_SORT_CHECKS 1
#define PROFILE_SORT_MATCHES 2
#define PROFILE_SORT_NOMATCHES 3
#define PROFILE_SORT_AVG_TICKS 4
#define PROFILE_SORT_AVG_TICKS_PER_MATCH 5
#define PROFILE_SORT_AVG_TICKS_PER_NOMATCH 6
#define PROFILE_SORT_TOTAL_TICKS 7

/* MACROS that handle profiling of rules and preprocessors */
#define PROFILE_VARS uint64_t ticks_start = 0, ticks_end = 0, ticks_delta = 0

#define PROFILE_START \
    get_clockticks(ticks_start);

#define PROFILE_END \
    get_clockticks(ticks_end); \
    ticks_delta = ticks_end - ticks_start;

#ifndef PROFILING_RULES
#define PROFILING_RULES ScProfileRules()
#endif

#define NODE_PROFILE_VARS uint64_t ticks_start = 0, ticks_end = 0, ticks_delta = 0, node_deltas = 0

#define NODE_PROFILE_START(node) \
    if (PROFILING_RULES) { \
        node->checks++; \
        PROFILE_START; \
    }

#define NODE_PROFILE_END_MATCH(node) \
    if (PROFILING_RULES) { \
        PROFILE_END; \
        node->ticks += ticks_delta + node_deltas; \
        node->ticks_match += ticks_delta + node_deltas; \
    }

#define NODE_PROFILE_END_NOMATCH(node) \
    if (PROFILING_RULES) { \
        PROFILE_END; \
        node->ticks += ticks_delta + node_deltas; \
        node->ticks_no_match += ticks_delta + node_deltas; \
    }

#define NODE_PROFILE_TMPSTART(node) \
    if (PROFILING_RULES) { \
        PROFILE_START; \
    }

#define NODE_PROFILE_TMPEND(node) \
    if (PROFILING_RULES) { \
        PROFILE_END; \
        node_deltas += ticks_delta; \
    }

#define OTN_PROFILE_ALERT(otn) otn->alerts++;

#ifndef PROFILING_PREPROCS
#define PROFILING_PREPROCS ScProfilePreprocs()
#endif

#define PREPROC_PROFILE_START(ppstat) \
    if (PROFILING_PREPROCS) { \
        ppstat.checks++; \
        PROFILE_START; \
        ppstat.ticks_start = ticks_start; \
    } 

#define PREPROC_PROFILE_REENTER_START(ppstat) \
    if (PROFILING_PREPROCS) { \
        PROFILE_START; \
        ppstat.ticks_start = ticks_start; \
    } 

#define PREPROC_PROFILE_TMPSTART(ppstat) \
    if (PROFILING_PREPROCS) { \
        PROFILE_START; \
        ppstat.ticks_start = ticks_start; \
    } 

#define PREPROC_PROFILE_END(ppstat) \
    if (PROFILING_PREPROCS) { \
        PROFILE_END; \
        ppstat.exits++; \
        ppstat.ticks += ticks_end - ppstat.ticks_start; \
    } 

#define PREPROC_PROFILE_REENTER_END(ppstat) \
    if (PROFILING_PREPROCS) { \
        PROFILE_END; \
        ppstat.ticks += ticks_end - ppstat.ticks_start; \
    } 

#define PREPROC_PROFILE_TMPEND(ppstat) \
    if (PROFILING_PREPROCS) { \
        PROFILE_END; \
        ppstat.ticks += ticks_end - ppstat.ticks_start; \
    } 

/************** Profiling API ******************/
void ShowRuleProfiles(void);

/* Preprocessor stats info */
typedef struct _PreprocStats
{
    uint64_t ticks, ticks_start;
    uint64_t checks;
    uint64_t exits;
} PreprocStats;

typedef struct _PreprocStatsNode
{
    PreprocStats *stats;
    char *name;
    int layer;
    PreprocStats *parent;
    struct _PreprocStatsNode *next;
} PreprocStatsNode;

typedef struct _ProfileConfig
{
    int num;
    int sort;
    int append;
    char *filename;

} ProfileConfig;

void RegisterPreprocessorProfile(char *keyword, PreprocStats *stats, int layer, PreprocStats *parent);
void ShowPreprocProfiles(void);
void ResetRuleProfiling(void);
void ResetPreprocProfiling(void);
void CleanupPreprocStatsNodeList(void);
extern PreprocStats totalPerfStats;
#else
#define PROFILE_VARS
#define NODE_PROFILE_VARS
#define NODE_PROFILE_START(node)
#define NODE_PROFILE_END_MATCH(node)
#define NODE_PROFILE_END_NOMATCH(node)
#define NODE_PROFILE_TMPSTART(node)
#define NODE_PROFILE_TMPEND(node)
#define OTN_PROFILE_ALERT(otn)
#define PREPROC_PROFILE_START(ppstat)
#define PREPROC_PROFILE_REENTER_START(ppstat)
#define PREPROC_PROFILE_TMPSTART(ppstat)
#define PREPROC_PROFILE_END(ppstat)
#define PREPROC_PROFILE_REENTER_END(ppstat)
#define PREPROC_PROFILE_TMPEND(ppstat)
#endif

#endif  /* __PROFILER_H__ */
