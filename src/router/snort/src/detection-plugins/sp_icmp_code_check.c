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
#include "util.h"
#include "snort_debug.h"
#include "plugin_enum.h"
#include "sfhashfcn.h"

#include "snort.h"
#include "profiler.h"
#ifdef PERF_PROFILING
PreprocStats icmpCodePerfStats;
extern PreprocStats ruleOTNEvalPerfStats;
#endif

#include "sfhashfcn.h"
#include "detection_options.h"

typedef struct _IcmpCodeCheckData
{
    /* the icmp code number */
    int icmp_code;
    int icmp_code2;
    uint8_t operator;
} IcmpCodeCheckData;

#define ICMP_CODE_TEST_EQ 1
#define ICMP_CODE_TEST_GT 2
#define ICMP_CODE_TEST_LT 3
#define ICMP_CODE_TEST_RG 4


void IcmpCodeCheckInit(struct _SnortConfig *, char *, OptTreeNode *, int);
void ParseIcmpCode(struct _SnortConfig *, char *, OptTreeNode *);
int IcmpCodeCheck(void *option_data, Packet *);

uint32_t IcmpCodeCheckHash(void *d)
{
    uint32_t a,b,c;
    IcmpCodeCheckData *data = (IcmpCodeCheckData *)d;

    a = data->icmp_code;
    b = data->icmp_code2;
    c = data->operator;

    mix(a,b,c);

    a += RULE_OPTION_TYPE_ICMP_CODE;

    final(a,b,c);

    return c;
}

int IcmpCodeCheckCompare(void *l, void *r)
{
    IcmpCodeCheckData *left = (IcmpCodeCheckData *)l;
    IcmpCodeCheckData *right = (IcmpCodeCheckData *)r;

    if (!left || !right)
        return DETECTION_OPTION_NOT_EQUAL;

    if ((left->icmp_code == right->icmp_code) &&
        (left->icmp_code2 == right->icmp_code2) &&
        (left->operator == right->operator))
    {
        return DETECTION_OPTION_EQUAL;
    }

    return DETECTION_OPTION_NOT_EQUAL;
}

/****************************************************************************
 *
 * Function: SetupIcmpCodeCheck()
 *
 * Purpose: Register the icode keyword and configuration function
 *
 * Arguments: None.
 *
 * Returns: void function
 *
 ****************************************************************************/
void SetupIcmpCodeCheck(void)
{
    /* map the keyword to an initialization/processing function */
    RegisterRuleOption("icode", IcmpCodeCheckInit, NULL, OPT_TYPE_DETECTION, NULL);
#ifdef PERF_PROFILING
    RegisterPreprocessorProfile("icode", &icmpCodePerfStats, 3, &ruleOTNEvalPerfStats, NULL);
#endif

    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "Plugin: IcmpCodeCheck Initialized\n"););
}


/****************************************************************************
 *
 * Function: IcmpCodeCheckInit(char *, OptTreeNode *)
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
void IcmpCodeCheckInit(struct _SnortConfig *sc, char *data, OptTreeNode *otn, int protocol)
{
    OptFpList *fpl;
    if(protocol != IPPROTO_ICMP)
    {
        FatalError( "%s(%d): ICMP Options on non-ICMP rule\n", file_name, file_line);
    }

    /* multiple declaration check */
    if(otn->ds_list[PLUGIN_ICMP_CODE])
    {
        FatalError("%s(%d): Multiple icmp code options in rule\n", file_name,
                file_line);
    }

    /* allocate the data structure and attach it to the
       rule's data struct list */

    otn->ds_list[PLUGIN_ICMP_CODE] = (IcmpCodeCheckData *)
            SnortAlloc(sizeof(IcmpCodeCheckData));

    /* this is where the keyword arguments are processed and placed into the
       rule option's data structure */
    ParseIcmpCode(sc, data, otn);

    /* finally, attach the option's detection function to the rule's
       detect function pointer list */

    fpl = AddOptFuncToList(IcmpCodeCheck, otn);
    fpl->type = RULE_OPTION_TYPE_ICMP_CODE;
    fpl->context = otn->ds_list[PLUGIN_ICMP_CODE];
}



/****************************************************************************
 *
 * Function: ParseIcmpCode(struct _SnortConfig *, char *, OptTreeNode *)
 *
 * Purpose: Process the icode argument and stick it in the data struct
 *
 * Arguments: data => argument data
 *            otn => pointer to the current rule's OTN
 *
 * Returns: void function
 *
 ****************************************************************************/
void ParseIcmpCode(struct _SnortConfig *sc, char *data, OptTreeNode *otn)
{
    char *code;
    IcmpCodeCheckData *ds_ptr;  /* data struct pointer */
    void *ds_ptr_dup;
    char *endptr = NULL;

    /* set the ds pointer to make it easier to reference the option's
       particular data struct */
    ds_ptr = otn->ds_list[PLUGIN_ICMP_CODE];

    /* set a pointer to the data so to leave the original unchanged */
    code = data;

    if(!data)
    {
        FatalError("%s (%d): No ICMP Code Specified\n",
                   file_name, file_line);
    }


    /* get rid of whitespace before the data */
    while(isspace((int)*data))
        data++;

    if (*data == '\0')
    {
        FatalError("%s (%d): No ICMP Code Specified\n",
                   file_name, file_line);
    }

    /*
     * If a range is specified, put the min in icmp_code, and the max in
     * icmp_code2
     */

    if ((isdigit((int)*data) || (*data == '-') || (*data == '+')) && strstr(data, "<>"))
    {
        ds_ptr->icmp_code = strtol(data, &endptr, 10);
        while (isspace((int)*endptr))
            endptr++;

        if (*endptr != '<')
        {
            FatalError("%s (%d): Invalid ICMP icode in rule: %s\n",
                       file_name, file_line, code);
        }

        data = endptr;

        data += 2;   /* move past <> */

        while (isspace((int)*data))
            data++;

        ds_ptr->icmp_code2 = strtol(data, &endptr, 10);
        if (*data == '\0' || *endptr != '\0')
        {
            FatalError("%s (%d): Invalid ICMP icode in rule: %s\n",
                       file_name, file_line, code);
        }

        if( (ds_ptr->icmp_code < -1) || (ds_ptr->icmp_code > 254) )
             FatalError("%s (%d): Invalid ICMP icode lower limit in rule: %d\n",
                       file_name, file_line, ds_ptr->icmp_code);

        if( (ds_ptr->icmp_code2 <= 0) || (ds_ptr->icmp_code2 > 256) )
             FatalError("%s (%d): Invalid ICMP icode upper limit in rule: %d\n",
                       file_name, file_line, ds_ptr->icmp_code2);

        if( (ds_ptr->icmp_code2 - ds_ptr->icmp_code) <= 1 )
             FatalError("%s (%d): Invalid ICMP icode (upper-lower) <= 1 in rule: %s\n",
                       file_name, file_line, code);

        ds_ptr->operator = ICMP_CODE_TEST_RG;
    }
    /* otherwise if its greater than... */
    else if (*data == '>')
    {
        data++;
        while (isspace((int)*data))
            data++;

        ds_ptr->icmp_code = strtol(data, &endptr, 10);
        if (*data == '\0' || *endptr != '\0')
        {
            FatalError("%s (%d): Invalid ICMP icode in rule: %s\n",
                       file_name, file_line, code);
        }

        if( (ds_ptr->icmp_code < 0) || (ds_ptr->icmp_code > 254) )
             FatalError("%s (%d): Invalid ICMP icode lower limit in rule: %d\n",
                       file_name, file_line, ds_ptr->icmp_code);

        ds_ptr->operator = ICMP_CODE_TEST_GT;
    }
    /* otherwise if its less than ... */
    else if (*data == '<')
    {
        data++;
        while (isspace((int)*data))
            data++;

        ds_ptr->icmp_code = strtol(data, &endptr, 10);
        if (*data == '\0' || *endptr != '\0')
        {
            FatalError("%s (%d): Invalid ICMP icode in rule: %s\n",
                       file_name, file_line, code);
        }

        if( (ds_ptr->icmp_code <= 0) || (ds_ptr->icmp_code > 256) )
             FatalError("%s (%d): Invalid ICMP icode upper limit in rule: %d\n",
                       file_name, file_line, ds_ptr->icmp_code);

        ds_ptr->operator = ICMP_CODE_TEST_LT;
    }
    /* otherwise check if its a digit */
    else
    {
        ds_ptr->icmp_code = strtol(data, &endptr, 10);
        if (*endptr != '\0')
        {
            FatalError("%s (%d): Invalid ICMP icode in rule: %s\n",
                       file_name, file_line, code);
        }

        if( (ds_ptr->icmp_code < 0) || (ds_ptr->icmp_code > 255) )
             FatalError("%s (%d): Invalid ICMP icode upper limit in rule: %d\n",
                       file_name, file_line, ds_ptr->icmp_code);

        ds_ptr->operator = ICMP_CODE_TEST_EQ;
    }
    if (add_detection_option(sc, RULE_OPTION_TYPE_ICMP_CODE, (void *)ds_ptr, &ds_ptr_dup) == DETECTION_OPTION_EQUAL)
    {
        otn->ds_list[PLUGIN_ICMP_CODE] = ds_ptr_dup;
        free(ds_ptr);
    }

    return;
}


/****************************************************************************
 *
 * Function: IcmpCodeCheck(Packet *p, OptTreeNode *, OptFpList *fp_list)
 *
 * Purpose: Test the packet's ICMP code field value against the option's
 *          ICMP code
 *
 * Arguments: data => argument data
 *            otn => pointer to the current rule's OTN
 *
 * Returns: void function
 *
 ****************************************************************************/
int IcmpCodeCheck(void *option_data, Packet *p)
{
    IcmpCodeCheckData *ds_ptr;
    int rval = DETECTION_OPTION_NO_MATCH;
    PROFILE_VARS;

    ds_ptr = (IcmpCodeCheckData *)option_data;

    /* return 0  if we don't have an icmp header */
    if(!p->icmph)
        return rval;

    PREPROC_PROFILE_START(icmpCodePerfStats);

    switch(ds_ptr->operator)
    {
        case ICMP_CODE_TEST_EQ:
            if (ds_ptr->icmp_code == p->icmph->code)
                rval = DETECTION_OPTION_MATCH;
            break;
        case ICMP_CODE_TEST_GT:
            if (p->icmph->code > ds_ptr->icmp_code)
                rval = DETECTION_OPTION_MATCH;
            break;
        case ICMP_CODE_TEST_LT:
            if (p->icmph->code < ds_ptr->icmp_code)
                rval = DETECTION_OPTION_MATCH;
            break;
        case ICMP_CODE_TEST_RG:
            if (p->icmph->code > ds_ptr->icmp_code &&
                    p->icmph->code < ds_ptr->icmp_code2)
                rval = DETECTION_OPTION_MATCH;
            break;
        default:
            break;
    }

    DEBUG_WRAP(
        if (rval == DETECTION_OPTION_MATCH)
        {
            DebugMessage(DEBUG_PLUGIN, "Got icmp code match!\n");
        }
        else
        {
            DebugMessage(DEBUG_PLUGIN, "Failed icmp code match!\n");
        }
        );

    PREPROC_PROFILE_END(icmpCodePerfStats);

    return rval;
}
