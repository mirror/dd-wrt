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

/* $Id$ */
/* sp_icmp_seq_check
 *
 * Purpose:
 *
 * Test the Sequence number field of ICMP ECHO and ECHO_REPLY packets for
 * specified values.  This is useful for detecting TFN attacks, amongst others.
 *
 * Arguments:
 *
 * The ICMP Seq plugin takes a number as an option argument.
 *
 * Effect:
 *
 * Tests ICMP ECHO and ECHO_REPLY packet Seq field values and returns a
 * "positive" detection result (i.e. passthrough) upon a value match.
 *
 * Comments:
 *
 * This plugin was developed to detect TFN distributed attacks.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <ctype.h>

#include "sf_types.h"
#include "rules.h"
#include "treenodes.h"
#include "decode.h"
#include "plugbase.h"
#include "parser.h"
#include "snort_debug.h"
#include "util.h"
#include "plugin_enum.h"

#include "snort.h"
#include "profiler.h"
#ifdef PERF_PROFILING
PreprocStats icmpSeqPerfStats;
extern PreprocStats ruleOTNEvalPerfStats;
#endif

#include "sfhashfcn.h"
#include "detection_options.h"

typedef struct _IcmpSeqCheckData
{
    unsigned short icmpseq;

} IcmpSeqCheckData;

void IcmpSeqCheckInit(struct _SnortConfig *, char *, OptTreeNode *, int);
void ParseIcmpSeq(struct _SnortConfig *, char *, OptTreeNode *);
int IcmpSeqCheck(void *option_data, Packet *p);

uint32_t IcmpSeqCheckHash(void *d)
{
    uint32_t a,b,c;
    IcmpSeqCheckData *data = (IcmpSeqCheckData *)d;

    a = data->icmpseq;
    b = RULE_OPTION_TYPE_ICMP_SEQ;
    c = 0;

    final(a,b,c);

    return c;
}

int IcmpSeqCheckCompare(void *l, void *r)
{
    IcmpSeqCheckData *left = (IcmpSeqCheckData *)l;
    IcmpSeqCheckData *right = (IcmpSeqCheckData *)r;

    if (!left || !right)
        return DETECTION_OPTION_NOT_EQUAL;

    if (left->icmpseq == right->icmpseq)
    {
        return DETECTION_OPTION_EQUAL;
    }

    return DETECTION_OPTION_NOT_EQUAL;
}



/****************************************************************************
 *
 * Function: SetupIcmpSeqCheck()
 *
 * Purpose: Registers the configuration function and links it to a rule
 *          keyword.  This is the function that gets called from InitPlugins
 *          in plugbase.c.
 *
 * Arguments: None.
 *
 * Returns: void function
 *
 ****************************************************************************/
void SetupIcmpSeqCheck(void)
{
    /* map the keyword to an initialization/processing function */
    RegisterRuleOption("icmp_seq", IcmpSeqCheckInit, NULL, OPT_TYPE_DETECTION, NULL);

#ifdef PERF_PROFILING
    RegisterPreprocessorProfile("icmp_seq", &icmpSeqPerfStats, 3, &ruleOTNEvalPerfStats, NULL);
#endif
    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "Plugin: IcmpSeqCheck Setup\n"););
}


/****************************************************************************
 *
 * Function: IcmpSeqCheckInit(char *, OptTreeNode *)
 *
 * Purpose: Handles parsing the rule information and attaching the associated
 *          detection function to the OTN.
 *
 * Arguments: data => rule arguments/data
 *            otn => pointer to the current rule option list node
 *
 * Returns: void function
 *
 ****************************************************************************/
void IcmpSeqCheckInit(struct _SnortConfig *sc, char *data, OptTreeNode *otn, int protocol)
{
    OptFpList *fpl;
    if(protocol != IPPROTO_ICMP)
    {
        FatalError("%s(%d): ICMP Options on non-ICMP rule\n", file_name, file_line);
    }

    /* multiple declaration check */
    if(otn->ds_list[PLUGIN_ICMP_SEQ_CHECK])
    {
        FatalError("%s(%d): Multiple ICMP seq options in rule\n", file_name,
                file_line);
    }

    /* allocate the data structure and attach it to the
       rule's data struct list */
    otn->ds_list[PLUGIN_ICMP_SEQ_CHECK] = (IcmpSeqCheckData *)
        SnortAlloc(sizeof(IcmpSeqCheckData));

    /* this is where the keyword arguments are processed and placed into the
       rule option's data structure */
    ParseIcmpSeq(sc, data, otn);

    /* finally, attach the option's detection function to the rule's
       detect function pointer list */
    fpl = AddOptFuncToList(IcmpSeqCheck, otn);
    fpl->type = RULE_OPTION_TYPE_ICMP_SEQ;
    fpl->context = otn->ds_list[PLUGIN_ICMP_SEQ_CHECK];
}



/****************************************************************************
 *
 * Function: ParseIcmpSeq(struct _SnortConfig *, char *, OptTreeNode *)
 *
 * Purpose: Convert the rule option argument to program data.
 *
 * Arguments: data => argument data
 *            otn => pointer to the current rule's OTN
 *
 * Returns: void function
 *
 ****************************************************************************/
void ParseIcmpSeq(struct _SnortConfig *sc, char *data, OptTreeNode *otn)
{
    IcmpSeqCheckData *ds_ptr;  /* data struct pointer */
    void *ds_ptr_dup;
    char *endTok;

    /* set the ds pointer to make it easier to reference the option's
       particular data struct */
    ds_ptr = otn->ds_list[PLUGIN_ICMP_SEQ_CHECK];

    /* advance past whitespace */
    while(isspace((int)*data)) data++;

    ds_ptr->icmpseq = (uint16_t)SnortStrtoulRange(data, &endTok, 10, 0, UINT16_MAX);
    if ((endTok == data) || (*endTok != '\0'))
    {
        FatalError("%s(%d) => Invalid parameter '%s' to icmp_seq.  "
                   "Must be between 0 & 65535, inclusive\n",
                   file_name, file_line, data);
    }
    ds_ptr->icmpseq = htons(ds_ptr->icmpseq);

    if (add_detection_option(sc, RULE_OPTION_TYPE_ICMP_SEQ, (void *)ds_ptr, &ds_ptr_dup) == DETECTION_OPTION_EQUAL)
    {
        free(ds_ptr);
        ds_ptr = otn->ds_list[PLUGIN_ICMP_SEQ_CHECK] = ds_ptr_dup;
    }

    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "Set ICMP Seq test value to %d\n", ds_ptr->icmpseq););
}


/****************************************************************************
 *
 * Function: IcmpSeqCheck(char *, OptTreeNode *)
 *
 * Purpose: Compare the ICMP Sequence field to the rule value.
 *
 * Arguments: data => argument data
 *            otn => pointer to the current rule's OTN
 *
 * Returns: If the detection test fails, this function *must* return a zero!
 *          On success, it calls the next function in the detection list
 *
 ****************************************************************************/
int IcmpSeqCheck(void *option_data, Packet *p)
{
    IcmpSeqCheckData *icmpSeq = (IcmpSeqCheckData *)option_data;
    PROFILE_VARS;

    if(!p->icmph)
        return DETECTION_OPTION_NO_MATCH; /* if error occured while icmp header
                   * was processed, return 0 automagically.  */

    PREPROC_PROFILE_START(icmpSeqPerfStats);

    if( (p->icmph->type == ICMP_ECHO || p->icmph->type == ICMP_ECHOREPLY)
        || (p->icmph->type == ICMP6_ECHO || p->icmph->type == ICMP6_REPLY)
      )
    {
        /* test the rule ID value against the ICMP extension ID field */
        if(icmpSeq->icmpseq == p->icmph->s_icmp_seq)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "ICMP ID check success\n"););
            PREPROC_PROFILE_END(icmpSeqPerfStats);
            return DETECTION_OPTION_MATCH;
        }
        else
        {
            /* you can put debug comments here or not */
            DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "ICMP ID check failed\n"););
        }
    }
    PREPROC_PROFILE_END(icmpSeqPerfStats);
    return DETECTION_OPTION_NO_MATCH;
}
