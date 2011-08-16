/****************************************************************************
 * Copyright (C) 2009-2011 Sourcefire, Inc.
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
 ***************************************************************************/

#ifndef _INTEL_H_
#define _INTEL_H_

#include "cpa.h"
#include "pm/cpa_pm.h"
#include "cpa_types.h"
#include "debug.h"


/* DATA TYPES *****************************************************************/
typedef struct _IntelPmPattern
{
    void *user_data;
    void *rule_option_tree;
    void *neg_list;

    unsigned char *pattern;
    unsigned int pattern_len;
    unsigned int no_case;
    unsigned int negative;
    int id;   /* pattern id passed in from mpse */
    Cpa32U patternId;  /* actual pattern id */

} IntelPmPattern;

typedef struct _IntelPm
{
    Cpa16U patternGroupId;
    Cpa32U patternIds;
    CpaPmSessionCtx sessionCtx;

    /* Temporary data for building trees */
    int (*build_tree)(void *id, void **existing_tree);
    int (*neg_list_func)(void *id, void **list);

    void *match_queue;

    /* Temporary data for match callback */
    void *data;
    int (*match)(void *id, void *tree, int index, void *data, void *neg_list);

    void (*user_free)(void *);
    void (*option_tree_free)(void **);
    void (*neg_list_free)(void **);

    IntelPmPattern *pattern_array;
    Cpa32U pattern_array_len;

    /* Every IntelPm has a reference to this */
    void *handles;

} IntelPm;


/* PROTOTYPES *****************************************************************/
void IntelPmStartInstance(void);
void IntelPmStopInstance(void);
void * IntelPmNew(void (*user_free)(void *p),
        void (*option_tree_free)(void **p),
        void (*neg_list_free)(void **p));
void IntelPmDelete(IntelPm *ipm);
int IntelPmAddPattern(IntelPm *ipm, unsigned char *pat, int pat_len,
        unsigned no_case, unsigned negative, void *pat_data, int pat_id);
int IntelPmFinishGroup(IntelPm *ipm,
       int (*build_tree)(void *id, void **existing_tree),
       int (*neg_list_func)(void *id, void **list));
void IntelPmCompile(void);
void IntelPmActivate(void);
void IntelPmDeactivate(void);
int IntelPmSearch(IntelPm *ipm, unsigned char *buffer, int buffer_len,
        int (*match)(void * id, void *tree, int index, void *data, void *neg_list),
        void *data);
int IntelGetPatternCount(IntelPm *ipm);
int IntelPmPrintInfo(IntelPm *ipm);
void IntelPmPrintSummary(void);
void IntelPmPrintBufferStats(void);

#endif  /* _INTEL_H_ */
