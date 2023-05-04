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

#include <stdlib.h>
#include <string.h>
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
#include "sp_icmp_type_check.h"
#include "sfhashfcn.h"

#include "snort.h"
#include "profiler.h"
#ifdef PERF_PROFILING
PreprocStats icmpTypePerfStats;
extern PreprocStats ruleOTNEvalPerfStats;
#endif

#include "sfhashfcn.h"
#include "detection_options.h"

void IcmpTypeCheckInit(struct _SnortConfig *, char *, OptTreeNode *, int);
void ParseIcmpType(struct _SnortConfig *, char *, OptTreeNode *);
int IcmpTypeCheck(void *option_data, Packet *p);

uint32_t IcmpTypeCheckHash(void *d)
{
    uint32_t a,b,c;
    IcmpTypeCheckData *data = (IcmpTypeCheckData *)d;

    a = data->icmp_type;
    b = data->icmp_type2;
    c = data->operator;

    mix(a,b,c);

    a += RULE_OPTION_TYPE_ICMP_TYPE;

    final(a,b,c);

    return c;
}

int IcmpTypeCheckCompare(void *l, void *r)
{
    IcmpTypeCheckData *left = (IcmpTypeCheckData *)l;
    IcmpTypeCheckData *right = (IcmpTypeCheckData *)r;

    if (!left || !right)
        return DETECTION_OPTION_NOT_EQUAL;

    if ((left->icmp_type == right->icmp_type) &&
        (left->icmp_type2 == right->icmp_type2) &&
        (left->operator == right->operator))
    {
        return DETECTION_OPTION_EQUAL;
    }

    return DETECTION_OPTION_NOT_EQUAL;
}

/****************************************************************************
 *
 * Function: SetupIcmpTypeCheck()
 *
 * Purpose: Register the itype keyword and configuration function
 *
 * Arguments: None.
 *
 * Returns: void function
 *
 ****************************************************************************/
void SetupIcmpTypeCheck(void)
{
    /* map the keyword to an initialization/processing function */
    RegisterRuleOption("itype", IcmpTypeCheckInit, NULL, OPT_TYPE_DETECTION, NULL);
#ifdef PERF_PROFILING
    RegisterPreprocessorProfile("itype", &icmpTypePerfStats, 3, &ruleOTNEvalPerfStats, NULL);
#endif
    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,"Plugin: IcmpTypeCheck Initialized\n"););
}


/****************************************************************************
 *
 * Function: IcmpTypeCheckInit(struct _SnortConfig *, char *, OptTreeNode *)
 *
 * Purpose: Initialize the rule data structs and parse the rule argument
 *          data, then link in the detection function
 *
 * Arguments: data => rule arguments/data
 *            otn => pointer to the current rule option list node
 *
 * Returns: void function
 *
 ****************************************************************************/
void IcmpTypeCheckInit(struct _SnortConfig *sc, char *data, OptTreeNode *otn, int protocol)
{
    OptFpList *fpl;
    if(protocol != IPPROTO_ICMP)
    {
        FatalError("%s(%d): ICMP Options on non-ICMP rule\n", file_name, file_line);
    }

    /* multiple declaration check */
    if(otn->ds_list[PLUGIN_ICMP_TYPE])
    {
        FatalError("%s(%d): Multiple ICMP type options in rule\n", file_name,
                file_line);
    }

    /* allocate the data structure and attach it to the
       rule's data struct list */
    otn->ds_list[PLUGIN_ICMP_TYPE] = (IcmpTypeCheckData *)
            SnortAlloc(sizeof(IcmpTypeCheckData));

    /* this is where the keyword arguments are processed and placed into the
       rule option's data structure */
    ParseIcmpType(sc, data, otn);

    /* finally, attach the option's detection function to the rule's
       detect function pointer list */
    fpl = AddOptFuncToList(IcmpTypeCheck, otn);
    fpl->type = RULE_OPTION_TYPE_ICMP_TYPE;
    fpl->context = otn->ds_list[PLUGIN_ICMP_TYPE];
}



/****************************************************************************
 *
 * Function: ParseIcmpType(struct _SnortConfig *, char *, OptTreeNode *)
 *
 * Purpose: Process the itype argument and stick it in the data struct
 *
 * Arguments: data => argument data
 *            otn => pointer to the current rule's OTN
 *
 * Returns: void function
 *
 ****************************************************************************/
void ParseIcmpType(struct _SnortConfig *sc, char *data, OptTreeNode *otn)
{
    char *type;
    IcmpTypeCheckData *ds_ptr;  /* data struct pointer */
    void *ds_ptr_dup;
    char *endptr = NULL;

    /* set the ds pointer to make it easier to reference the option's
       particular data struct */
    ds_ptr = otn->ds_list[PLUGIN_ICMP_TYPE];

    /* set a pointer to the data so to leave the original unchanged */
    type = data;

    if(!data)
    {
        FatalError("%s (%d): No ICMP Type Specified\n",
                   file_name, file_line);
    }

    /* get rid of spaces before the data */
    while(isspace((int)*data))
        data++;

    if (*data == '\0')
    {
        FatalError("%s (%d): No ICMP Type Specified : %s\n",
                   file_name, file_line, type);
    }

    /*
     * if a range is specified, put the min in icmp_type, and the max in
     * icmp_type2
     */

    if (isdigit((int)*data) && strstr(data, "<>"))
    {
        ds_ptr->icmp_type = strtol(data, &endptr, 10);
        while (isspace((int)*endptr))
            endptr++;

        if (*endptr != '<')
        {
            FatalError("%s (%d): Invalid ICMP itype in rule: %s\n",
                       file_name, file_line, type);
        }

        data = endptr;

        data += 2;   /* move past <> */

        while (isspace((int)*data))
            data++;

        ds_ptr->icmp_type2 = strtol(data, &endptr, 10);
        if (*data == '\0' || *endptr != '\0')
        {
            FatalError("%s (%d): Invalid ICMP itype in rule: %s\n",
                       file_name, file_line, type);
        }

        ds_ptr->operator = ICMP_TYPE_TEST_RG;
    }
    /* otherwise if its greater than... */
    else if (*data == '>')
    {
        data++;
        while (isspace((int)*data))
            data++;

        ds_ptr->icmp_type = strtol(data, &endptr, 10);
        if (*data == '\0' || *endptr != '\0')
        {
            FatalError("%s (%d): Invalid ICMP itype in rule: %s\n",
                       file_name, file_line, type);
        }

        ds_ptr->operator = ICMP_TYPE_TEST_GT;
    }
    /* otherwise if its less than ... */
    else if (*data == '<')
    {
        data++;
        while (isspace((int)*data))
            data++;

        ds_ptr->icmp_type = strtol(data, &endptr, 10);
        if (*data == '\0' || *endptr != '\0')
        {
            FatalError("%s (%d): Invalid ICMP itype in rule: %s\n",
                       file_name, file_line, type);
        }

        ds_ptr->operator  = ICMP_TYPE_TEST_LT;
    }
    /* otherwise check if its a digit */
    else
    {
        ds_ptr->icmp_type = strtol(data, &endptr, 10);
        if (*endptr != '\0')
        {
            FatalError("%s (%d): Invalid ICMP itype in rule: %s\n",
                       file_name, file_line, type);
        }

        ds_ptr->operator = ICMP_TYPE_TEST_EQ;
    }

    if (add_detection_option(sc, RULE_OPTION_TYPE_ICMP_TYPE, (void *)ds_ptr, &ds_ptr_dup) == DETECTION_OPTION_EQUAL)
    {
        free(ds_ptr);
        ds_ptr = otn->ds_list[PLUGIN_ICMP_TYPE] = ds_ptr_dup;
    }

    return;
}

/****************************************************************************
 *
 * Function: IcmpTypeCheck(char *, OptTreeNode *)
 *
 * Purpose: Test the packet's ICMP type field value against the option's
 *          ICMP type
 *
 * Arguments: data => argument data
 *            otn => pointer to the current rule's OTN
 *
 * Returns: void function
 *
 ****************************************************************************/
int IcmpTypeCheck(void *option_data, Packet *p)
{
    IcmpTypeCheckData *ds_ptr;
    int rval = DETECTION_OPTION_NO_MATCH;
    PROFILE_VARS;

    ds_ptr = (IcmpTypeCheckData *)option_data;

    /* return 0  if we don't have an icmp header */
    if(!p->icmph)
        return rval;

    PREPROC_PROFILE_START(icmpTypePerfStats);

    switch(ds_ptr->operator)
    {
        case ICMP_TYPE_TEST_EQ:
            if (p->icmph->type == ds_ptr->icmp_type)
                rval = DETECTION_OPTION_MATCH;
            break;
        case ICMP_TYPE_TEST_GT:
            if (p->icmph->type > ds_ptr->icmp_type)
                rval = DETECTION_OPTION_MATCH;
            break;
        case ICMP_TYPE_TEST_LT:
            if (p->icmph->type < ds_ptr->icmp_type)
                rval = DETECTION_OPTION_MATCH;
            break;
        case ICMP_TYPE_TEST_RG:
            if (p->icmph->type > ds_ptr->icmp_type &&
                    p->icmph->type < ds_ptr->icmp_type2)
                rval = DETECTION_OPTION_MATCH;
            break;
    }

    DEBUG_WRAP(
        if (rval == DETECTION_OPTION_MATCH)
        {
            DebugMessage(DEBUG_PLUGIN, "Got icmp type match!\n");
        }
        else
        {
            DebugMessage(DEBUG_PLUGIN, "Failed icmp type match!\n");
        }
        );

    PREPROC_PROFILE_END(icmpTypePerfStats);
    return rval;
}
