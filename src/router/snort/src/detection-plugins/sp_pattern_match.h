/* $Id$ */
/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2002-2013 Sourcefire, Inc.
** Copyright (C) 1998-2002 Martin Roesch <roesch@sourcefire.com>
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

#ifndef __SP_PATTERN_MATCH_H__
#define __SP_PATTERN_MATCH_H__

#include <ctype.h>

#include "snort.h"
#include "snort_debug.h"
#include "rules.h" /* needed for OptTreeNode defintion */
#include "treenodes.h"
#include "detection_util.h"
#include "hashstring.h"

/********************************************************************
 * Macros
 ********************************************************************/
#define CHECK_AND_PATTERN_MATCH 1
#define CHECK_URI_PATTERN_MATCH 2
#define PMD_WITHIN_UNDEFINED ((unsigned) -1)

/********************************************************************
 * Data structures
 ********************************************************************/
typedef struct _PatternMatchData
{
    int offset;             /* pattern search start offset */
    int depth;              /* pattern search depth */

    int distance;           /* offset to start from based on last match */
    u_int within;           /* this pattern must be found
                               within X bytes of last match*/

    int8_t offset_var;      /* byte_extract variable indices for offset, */
    int8_t depth_var;       /* depth, distance, within */
    int8_t distance_var;
    int8_t within_var;

    int rawbytes;           /* Search the raw bytes rather than any decoded app
                               buffer */
    int replace_depth;      /* >=0 is offset to start of replace */

    int nocase;             /* Toggle case insensitity */
    int use_doe;            /* Use the doe_ptr for relative pattern searching */
    HTTP_BUFFER http_buffer;/* Index of the URI buffer */
    int buffer_func;        /* buffer function CheckAND or CheckUri */
    u_int pattern_size;     /* size of app layer pattern */
    u_int replace_size;     /* size of app layer replace pattern */
    char *replace_buf;      /* app layer pattern to replace with */
    char *pattern_buf;      /* app layer pattern to match on */
    int (*search)(const char *, int, struct _PatternMatchData *);  /* search function */
    int *skip_stride; /* B-M skip array */
    int *shift_stride; /* B-M shift array */
    u_int pattern_max_jump_size; /* Maximum distance we can jump to search for
                                  * this pattern again. */
    OptFpList *fpl;         /* Pointer to the OTN FPList for this pattern */
                            /* Needed to be able to set the isRelative flag */

    /* Set if fast pattern matcher found a content in the packet,
       but the rule option specifies a negated content. Only
       applies to negative contents that are not relative */
    struct
    {
        struct timeval ts;
        uint64_t packet_number;
        uint32_t rebuild_flag;

    } last_check;

    /* For fast_pattern arguments */
    uint8_t fp;
    uint8_t fp_only;
    uint16_t fp_offset;
    uint16_t fp_length;

    uint8_t exception_flag; /* search for "not this pattern" */

    /* Used in ds_list - do not try to iterate after parsing a rule
     * since the detection option tree will eliminate duplicates and
     * the list may have missing pmds */
    struct _PatternMatchData *prev; /* ptr to previous match struct */
    struct _PatternMatchData *next; /* ptr to next match struct */

    Secure_Hash_Type pattern_type;
    int protected_length;
    bool protected_pattern;
} PatternMatchData;

/********************************************************************
 * Public function prototypes
 ********************************************************************/
void SetupPatternMatch(void);
PatternMatchData * NewNode(OptTreeNode *, int);
void PatternMatchFree(void *d);
uint32_t PatternMatchHash(void *d);
int PatternMatchCompare(void *l, void *r);
void FinalizeContentUniqueness(struct _SnortConfig *sc, OptTreeNode *otn);
void ValidateFastPattern(OptTreeNode *otn);
void make_precomp(PatternMatchData *);
void ParsePattern(char *, OptTreeNode *, int);
void ParseProtectedPattern(char *, OptTreeNode *, int);
int uniSearchCI(const char *, int, PatternMatchData *);
int CheckANDPatternMatch(void *, Packet *);
int CheckUriPatternMatch(void *, Packet *);
void PatternMatchDuplicatePmd(void *, PatternMatchData *);
int PatternMatchAdjustRelativeOffsets(PatternMatchData *orig_pmd, PatternMatchData *dup_pmd,
        const uint8_t *current_cursor, const uint8_t *orig_cursor);

#if 0
/* Not implemented */
int CheckORPatternMatch(Packet *, OptTreeNode *, OptFpList *);
#endif

static inline bool IsHttpBufFpEligible (HTTP_BUFFER http_buffer)
{
    switch ( http_buffer )
    {
    case HTTP_BUFFER_URI:
    case HTTP_BUFFER_HEADER:
    case HTTP_BUFFER_CLIENT_BODY:
        return true;
    default:
        break;
    }
    return false;
}

static inline PatternMatchData * RemovePmdFromList(PatternMatchData *pmd)
{
    if (pmd == NULL)
        return NULL;

    if (pmd->prev)
        pmd->prev->next = pmd->next;
    if (pmd->next)
        pmd->next->prev = pmd->prev;

    pmd->next = NULL;
    pmd->prev = NULL;

    return pmd;
}

static inline int InsertPmdAtFront(PatternMatchData **head, PatternMatchData *ins)
{
    if (head == NULL)
        return -1;

    if (ins == NULL)
        return 0;

    ins->next = *head;
    if (*head != NULL)
        (*head)->prev = ins;
    *head = ins;

    return 0;
}

static inline int AppendPmdToList(PatternMatchData **head, PatternMatchData *ins)
{
    PatternMatchData *tmp;

    if (head == NULL)
        return -1;

    if (ins == NULL)
        return 0;

    if (*head == NULL)
    {
        *head = ins;
        ins->prev = NULL;
        return 0;
    }

    for (tmp = *head; tmp->next != NULL; tmp = tmp->next);
    tmp->next = ins;
    ins->prev = tmp;

    return 0;
}


static inline void FreePmdList(PatternMatchData *pmd_list)
{
    if (pmd_list == NULL)
        return;

    while (pmd_list != NULL)
    {
        PatternMatchData *tmp = pmd_list->next;
        PatternMatchFree((void *)pmd_list);
        pmd_list = tmp;
    }
}

#endif /* __SP_PATTERN_MATCH_H__ */
