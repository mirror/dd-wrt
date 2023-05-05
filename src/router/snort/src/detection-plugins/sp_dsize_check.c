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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "sf_types.h"
#include "rules.h"
#include "treenodes.h"
#include "decode.h"
#include "plugbase.h"
#include "snort_debug.h"
#include "parser.h"
#include "plugin_enum.h"
#include "util.h"
#include "sfhashfcn.h"

#include "snort.h"
#include "profiler.h"
#ifdef PERF_PROFILING
PreprocStats dsizePerfStats;
extern PreprocStats ruleOTNEvalPerfStats;
#endif

#include "sfhashfcn.h"
#include "detection_options.h"

#define DSIZE_EQ                   1
#define DSIZE_GT                   2
#define DSIZE_LT                   3
#define DSIZE_RANGE                4

typedef struct _DsizeCheckData
{
    int dsize;
    int dsize2;
    char operator;
} DsizeCheckData;

void DsizeCheckInit(struct _SnortConfig *, char *, OptTreeNode *, int);
void ParseDsize(struct _SnortConfig *, char *, OptTreeNode *);

int CheckDsize(void *option_data, Packet *p);

uint32_t DSizeCheckHash(void *d)
{
    uint32_t a,b,c;
    DsizeCheckData *data = (DsizeCheckData *)d;

    a = data->dsize;
    b = data->dsize2;
    c = data->operator;

    mix(a,b,c);

    a += RULE_OPTION_TYPE_DSIZE;

    final(a,b,c);

    return c;
}

int DSizeCheckCompare(void *l, void *r)
{
    DsizeCheckData *left = (DsizeCheckData *)l;
    DsizeCheckData *right = (DsizeCheckData *)r;

    if (!left || !right)
        return DETECTION_OPTION_NOT_EQUAL;

    if (( left->dsize == right->dsize) &&
        ( left->dsize2 == right->dsize2) &&
        ( left->operator == right->operator))
    {
        return DETECTION_OPTION_EQUAL;
    }

    return DETECTION_OPTION_NOT_EQUAL;
}

/****************************************************************************
 *
 * Function: SetupDsizeCheck()
 *
 * Purpose: Attach the dsize keyword to the rule parse function
 *
 * Arguments: None.
 *
 * Returns: void function
 *
 ****************************************************************************/
void SetupDsizeCheck(void)
{
    /* map the keyword to an initialization/processing function */
    RegisterRuleOption("dsize", DsizeCheckInit, NULL, OPT_TYPE_DETECTION, NULL);
#ifdef PERF_PROFILING
    RegisterPreprocessorProfile("dsize_eq", &dsizePerfStats, 3, &ruleOTNEvalPerfStats, NULL);
#endif
    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "Plugin: DsizeCheck Initialized\n"););
}


/****************************************************************************
 *
 * Function: DsizeCheckInit(struct _SnortConfig *, char *, OptTreeNode *)
 *
 * Purpose: Parse the rule argument and attach it to the rule data struct,
 *          then attach the detection function to the function list
 *
 * Arguments: data => rule arguments/data
 *            otn => pointer to the current rule option list node
 *
 * Returns: void function
 *
 ****************************************************************************/
void DsizeCheckInit(struct _SnortConfig *sc, char *data, OptTreeNode *otn, int protocol)
{
    /* multiple declaration check */
    if(otn->ds_list[PLUGIN_DSIZE_CHECK])
    {
        FatalError("%s(%d): Multiple dsize options in rule\n", file_name,
                file_line);
    }

    /* allocate the data structure and attach it to the
       rule's data struct list */

    otn->ds_list[PLUGIN_DSIZE_CHECK] = (DsizeCheckData *)
        SnortAlloc(sizeof(DsizeCheckData));

    /* this is where the keyword arguments are processed and placed into the
       rule option's data structure */
    ParseDsize(sc, data, otn);

    /* NOTE: I moved the AddOptFuncToList call to the parsing function since
       the linking is best determined within that function */
}



/****************************************************************************
 *
 * Function: ParseDsize(struct _SnortConfig *, char *, OptTreeNode *)
 *
 * Purpose: Parse the dsize function argument and attach the detection
 *          function to the rule list as well.
 *
 * Arguments: data => argument data
 *            otn => pointer to the current rule's OTN
 *
 * Returns: void function
 *
 ****************************************************************************/
void ParseDsize(struct _SnortConfig *sc, char *data, OptTreeNode *otn)
{
    DsizeCheckData *ds_ptr;  /* data struct pointer */
    char *pcEnd;
    char *pcTok;
    int  iDsize = 0;
    void *ds_ptr_dup;
    OptFpList *fpl;

    /* set the ds pointer to make it easier to reference the option's
       particular data struct */
    ds_ptr = (DsizeCheckData *)otn->ds_list[PLUGIN_DSIZE_CHECK];

    while(isspace((int)*data)) data++;

    /* If a range is specified, put min in ds_ptr->dsize and max in
       ds_ptr->dsize2 */

    if(isdigit((int)*data) && strchr(data, '<') && strchr(data, '>'))
    {
        pcTok = strtok(data, " <>");
        if(!pcTok)
        {
            /*
            **  Fatal
            */
            FatalError("%s(%d): Invalid 'dsize' argument.\n",
                       file_name, file_line);
        }

        iDsize = strtol(pcTok, &pcEnd, 10);
        if(iDsize < 0 || *pcEnd)
        {
            FatalError("%s(%d): Invalid 'dsize' argument.\n",
                       file_name, file_line);
        }

        ds_ptr->dsize = (unsigned short)iDsize;

        pcTok = strtok(NULL, " <>");
        if(!pcTok)
        {
            FatalError("%s(%d): Invalid 'dsize' argument.\n",
                       file_name, file_line);
        }

        iDsize = strtol(pcTok, &pcEnd, 10);
        if(iDsize < 0 || *pcEnd)
        {
            FatalError("%s(%d): Invalid 'dsize' argument.\n",
                       file_name, file_line);
        }

        ds_ptr->dsize2 = (unsigned short)iDsize;

        ds_ptr->operator = DSIZE_RANGE;

#ifdef DEBUG_MSGS
        DebugMessage(DEBUG_PLUGIN, "min dsize: %d\n", ds_ptr->dsize);
        DebugMessage(DEBUG_PLUGIN, "max dsize: %d\n", ds_ptr->dsize2);
#endif
        fpl = AddOptFuncToList(CheckDsize, otn);
        fpl->type = RULE_OPTION_TYPE_DSIZE;

        if (add_detection_option(sc, RULE_OPTION_TYPE_DSIZE, (void *)ds_ptr, &ds_ptr_dup) == DETECTION_OPTION_EQUAL)
        {
            free(ds_ptr);
            ds_ptr = otn->ds_list[PLUGIN_DSIZE_CHECK] = ds_ptr_dup;
        }
        fpl->context = ds_ptr;

        return;
    }
    else if(*data == '>')
    {
        data++;
        fpl = AddOptFuncToList(CheckDsize, otn);
        ds_ptr->operator = DSIZE_GT;
    }
    else if(*data == '<')
    {
        data++;
        fpl = AddOptFuncToList(CheckDsize, otn);
        ds_ptr->operator = DSIZE_LT;
    }
    else
    {
        fpl = AddOptFuncToList(CheckDsize, otn);
        ds_ptr->operator = DSIZE_EQ;
    }

    fpl->type = RULE_OPTION_TYPE_DSIZE;

    while(isspace((int)*data)) data++;

    iDsize = strtol(data, &pcEnd, 10);
    if(iDsize < 0 || *pcEnd)
    {
        FatalError("%s(%d): Invalid 'dsize' argument.\n",
                   file_name, file_line);
    }

    ds_ptr->dsize = (unsigned short)iDsize;

    if (add_detection_option(sc, RULE_OPTION_TYPE_DSIZE, (void *)ds_ptr, &ds_ptr_dup) == DETECTION_OPTION_EQUAL)
    {
        free(ds_ptr);
        ds_ptr = otn->ds_list[PLUGIN_DSIZE_CHECK] = ds_ptr_dup;
     }
     fpl->context = ds_ptr;

    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "Payload length = %d\n", ds_ptr->dsize););

}

/****************************************************************************
 *
 * Function: CheckDsizeEq(char *, OptTreeNode *)
 *
 * Purpose: Test the packet's payload size against the rule payload size value
 *
 * Arguments: data => argument data
 *            otn => pointer to the current rule's OTN
 *
 * Returns:  0 on failure, return value of next list function on success
 ****************************************************************************/
int CheckDsize(void *option_data, Packet *p)
{
    DsizeCheckData *ds_ptr = (DsizeCheckData *)option_data;
    int rval = DETECTION_OPTION_NO_MATCH;
    PROFILE_VARS;

    if (!ds_ptr)
        return rval;

    PREPROC_PROFILE_START(dsizePerfStats);

    /* fake packet dsizes are always wrong */
    /* (unless they are PDUs) */
    if (
        (p->packet_flags & PKT_REBUILT_STREAM) &&
        !(p->packet_flags & PKT_PDU_HEAD) )
    {
        PREPROC_PROFILE_END(dsizePerfStats);
        return rval;
    }

    switch (ds_ptr->operator)
    {
        case DSIZE_EQ:
            if (ds_ptr->dsize == p->dsize)
                rval = DETECTION_OPTION_MATCH;
            break;
        case DSIZE_GT:
            if (ds_ptr->dsize < p->dsize)
                rval = DETECTION_OPTION_MATCH;
            break;
        case DSIZE_LT:
            if (ds_ptr->dsize > p->dsize)
                rval = DETECTION_OPTION_MATCH;
            break;
        case DSIZE_RANGE:
            if ((ds_ptr->dsize <= p->dsize) &&
                (ds_ptr->dsize2 >= p->dsize))
                rval = DETECTION_OPTION_MATCH;
            break;
        default:
            break;
    }

    PREPROC_PROFILE_END(dsizePerfStats);
    return rval;
}
