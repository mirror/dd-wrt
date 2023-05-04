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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <stdlib.h>
#include <ctype.h>

#include "sf_types.h"
#include "rules.h"
#include "treenodes.h"
#include "decode.h"
#include "plugbase.h"
#include "parser.h"
#include "util.h"
#include "snort_debug.h"
#include "plugin_enum.h"

#include "snort.h"
#include "profiler.h"
#ifdef PERF_PROFILING
PreprocStats tcpSeqPerfStats;
extern PreprocStats ruleOTNEvalPerfStats;
#endif

#include "sfhashfcn.h"
#include "detection_options.h"

typedef struct _TcpSeqCheckData
{
    u_long tcp_seq;

} TcpSeqCheckData;

void TcpSeqCheckInit(struct _SnortConfig *, char *, OptTreeNode *, int);
void ParseTcpSeq(struct _SnortConfig *, char *, OptTreeNode *);
int CheckTcpSeqEq(void *option_data, Packet *p);

uint32_t TcpSeqCheckHash(void *d)
{
    uint32_t a,b,c;
    TcpSeqCheckData *data = (TcpSeqCheckData *)d;

    a = data->tcp_seq;
    b = RULE_OPTION_TYPE_TCP_SEQ;
    c = 0;

    final(a,b,c);

    return c;
}

int TcpSeqCheckCompare(void *l, void *r)
{
    TcpSeqCheckData *left = (TcpSeqCheckData *)l;
    TcpSeqCheckData *right = (TcpSeqCheckData *)r;

    if (!left || !right)
        return DETECTION_OPTION_NOT_EQUAL;

    if (left->tcp_seq == right->tcp_seq)
    {
        return DETECTION_OPTION_EQUAL;
    }

    return DETECTION_OPTION_NOT_EQUAL;
}

/****************************************************************************
 *
 * Function: SetupTcpSeqCheck()
 *
 * Purpose: Link the seq keyword to the initialization function
 *
 * Arguments: None.
 *
 * Returns: void function
 *
 ****************************************************************************/
void SetupTcpSeqCheck(void)
{
    /* map the keyword to an initialization/processing function */
    RegisterRuleOption("seq", TcpSeqCheckInit, NULL, OPT_TYPE_DETECTION, NULL);
#ifdef PERF_PROFILING
    RegisterPreprocessorProfile("seq", &tcpSeqPerfStats, 3, &ruleOTNEvalPerfStats, NULL);
#endif

    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,"Plugin: TcpSeqCheck Initialized\n"););
}


/****************************************************************************
 *
 * Function: TcpSeqCheckInit(struct _SnortConfig *, char *, OptTreeNode *)
 *
 * Purpose: Attach the option data to the rule data struct and link in the
 *          detection function to the function pointer list.
 *
 * Arguments: data => rule arguments/data
 *            otn => pointer to the current rule option list node
 *
 * Returns: void function
 *
 ****************************************************************************/
void TcpSeqCheckInit(struct _SnortConfig *sc, char *data, OptTreeNode *otn, int protocol)
{
    OptFpList *fpl;
    if(protocol != IPPROTO_TCP)
    {
        FatalError("Line %s (%d): TCP Options on non-TCP rule\n", file_name, file_line);
    }

    /* multiple declaration check */
    if(otn->ds_list[PLUGIN_TCP_SEQ_CHECK])
    {
        FatalError("%s(%d): Multiple TCP seq options in rule\n", file_name,
                file_line);
    }

    /* allocate the data structure and attach it to the
       rule's data struct list */
    otn->ds_list[PLUGIN_TCP_SEQ_CHECK] = (TcpSeqCheckData *)
            SnortAlloc(sizeof(TcpSeqCheckData));

    /* this is where the keyword arguments are processed and placed into the
       rule option's data structure */
    ParseTcpSeq(sc, data, otn);

    /* finally, attach the option's detection function to the rule's
       detect function pointer list */
    fpl = AddOptFuncToList(CheckTcpSeqEq, otn);
    fpl->type = RULE_OPTION_TYPE_TCP_SEQ;
    fpl->context = otn->ds_list[PLUGIN_TCP_SEQ_CHECK];
}



/****************************************************************************
 *
 * Function: ParseTcpSeq(struct _SnortConfig *, char *, OptTreeNode *)
 *
 * Purpose: Attach the option rule's argument to the data struct.
 *
 * Arguments: data => argument data
 *            otn => pointer to the current rule's OTN
 *
 * Returns: void function
 *
 ****************************************************************************/
void ParseTcpSeq(struct _SnortConfig *sc, char *data, OptTreeNode *otn)
{
    char **ep = NULL;
    void *ds_ptr_dup;
    TcpSeqCheckData *ds_ptr;  /* data struct pointer */

    /* set the ds pointer to make it easier to reference the option's
       particular data struct */
    ds_ptr = otn->ds_list[PLUGIN_TCP_SEQ_CHECK];

    ds_ptr->tcp_seq = strtoul(data, ep, 0);
    ds_ptr->tcp_seq = htonl(ds_ptr->tcp_seq);

    if (add_detection_option(sc, RULE_OPTION_TYPE_TCP_SEQ, (void *)ds_ptr, &ds_ptr_dup) == DETECTION_OPTION_EQUAL)
    {
        otn->ds_list[PLUGIN_TCP_SEQ_CHECK] = ds_ptr_dup;
        free(ds_ptr);
    }

    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "Seq set to %lX\n", ds_ptr->tcp_seq););

}


/****************************************************************************
 *
 * Function: CheckTcpSeqEq(char *, OptTreeNode *)
 *
 * Purpose: Check to see if the packet's TCP ack field is equal to the rule
 *          ack value.
 *
 * Arguments: data => argument data
 *            otn => pointer to the current rule's OTN
 *
 * Returns: void function
 *
 ****************************************************************************/
int CheckTcpSeqEq(void *option_data, Packet *p)
{
    TcpSeqCheckData *tcpSeqCheckData = (TcpSeqCheckData *)option_data;
    int rval = DETECTION_OPTION_NO_MATCH;
    PROFILE_VARS;

    if(!p->tcph)
        return rval; /* if error appeared when tcp header was processed,
               * test fails automagically */

    PREPROC_PROFILE_START(tcpSeqPerfStats);

    if(tcpSeqCheckData->tcp_seq == p->tcph->th_seq)
    {
        rval = DETECTION_OPTION_MATCH;
    }
#ifdef DEBUG_MSGS
    else
    {
        /* you can put debug comments here or not */
        DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,"No match\n"););
    }
#endif

    /* if the test isn't successful, return 0 */
    PREPROC_PROFILE_END(tcpSeqPerfStats);
    return rval;
}
