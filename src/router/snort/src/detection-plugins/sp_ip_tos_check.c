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
#include <string.h>

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#include "sf_types.h"
#include "rules.h"
#include "treenodes.h"
#include "decode.h"
#include "plugbase.h"
#include "parser.h"
#include "snort_debug.h"
#include "plugin_enum.h"
#include "util.h"

#include "snort.h"
#include "profiler.h"
#ifdef PERF_PROFILING
PreprocStats ipTosPerfStats;
extern PreprocStats ruleOTNEvalPerfStats;
#endif

#include "sfhashfcn.h"
#include "detection_options.h"

typedef struct _IpTosCheckData
{
    uint8_t ip_tos;
    uint8_t not_flag;

} IpTosCheckData;

void IpTosCheckInit(struct _SnortConfig *, char *, OptTreeNode *, int);
void ParseIpTos(struct _SnortConfig *, char *, OptTreeNode *);
int IpTosCheckEq(void *option_data, Packet *p);

uint32_t IpTosCheckHash(void *d)
{
    uint32_t a,b,c;
    IpTosCheckData *data = (IpTosCheckData *)d;

    a = data->ip_tos;
    b = data->not_flag;
    c = RULE_OPTION_TYPE_IP_TOS;

    final(a,b,c);

    return c;
}

int IpTosCheckCompare(void *l, void *r)
{
    IpTosCheckData *left = (IpTosCheckData *)l;
    IpTosCheckData *right = (IpTosCheckData *)r;

    if (!left || !right)
        return DETECTION_OPTION_NOT_EQUAL;

    if ((left->ip_tos == right->ip_tos) &&
        (left->not_flag == right->not_flag))
    {
        return DETECTION_OPTION_EQUAL;
    }

    return DETECTION_OPTION_NOT_EQUAL;
}



/****************************************************************************
 *
 * Function: SetupIpTosCheck()
 *
 * Purpose: Associate the tos keyword with IpTosCheckInit
 *
 * Arguments: None.
 *
 * Returns: void function
 *
 ****************************************************************************/
void SetupIpTosCheck(void)
{
    /* map the keyword to an initialization/processing function */
    RegisterRuleOption("tos", IpTosCheckInit, NULL, OPT_TYPE_DETECTION, NULL);
#ifdef PERF_PROFILING
    RegisterPreprocessorProfile("tos", &ipTosPerfStats, 3, &ruleOTNEvalPerfStats, NULL);
#endif
    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "Plugin: IpTosCheck Initialized\n"););
}


/****************************************************************************
 *
 * Function: IpTosCheckInit(struct _SnortConfig *, char *, OptTreeNode *)
 *
 * Purpose: Setup the tos data struct and link the function into option
 *          function pointer list
 *
 * Arguments: data => rule arguments/data
 *            otn => pointer to the current rule option list node
 *
 * Returns: void function
 *
 ****************************************************************************/
void IpTosCheckInit(struct _SnortConfig *sc, char *data, OptTreeNode *otn, int protocol)
{
    OptFpList *fpl;
    /* multiple declaration check */
    if(otn->ds_list[PLUGIN_IP_TOS_CHECK])
    {
        FatalError("%s(%d): Multiple IP tos options in rule\n", file_name,
                file_line);
    }

    /* allocate the data structure and attach it to the
       rule's data struct list */
    otn->ds_list[PLUGIN_IP_TOS_CHECK] = (IpTosCheckData *)
            SnortAlloc(sizeof(IpTosCheckData));

    /* this is where the keyword arguments are processed and placed into the
       rule option's data structure */
    ParseIpTos(sc, data, otn);

    /* finally, attach the option's detection function to the rule's
       detect function pointer list */
    fpl = AddOptFuncToList(IpTosCheckEq, otn);
    fpl->type = RULE_OPTION_TYPE_IP_TOS;
    fpl->context = otn->ds_list[PLUGIN_IP_TOS_CHECK];
}



/****************************************************************************
 *
 * Function: ParseIpTos(struct _SnortConfig *, char *, OptTreeNode *)
 *
 * Purpose: Convert the tos option argument to data and plug it into the
 *          data structure
 *
 * Arguments: data => argument data
 *            otn => pointer to the current rule's OTN
 *
 * Returns: void function
 *
 ****************************************************************************/
void ParseIpTos(struct _SnortConfig *sc, char *data, OptTreeNode *otn)
{
    IpTosCheckData *ds_ptr;  /* data struct pointer */
    void *ds_ptr_dup;
    char *endTok;
    char *start;

    /* set the ds pointer to make it easier to reference the option's
       particular data struct */
    ds_ptr = otn->ds_list[PLUGIN_IP_TOS_CHECK];

    /* get rid of any whitespace */
    while(isspace((int)*data))
    {
        data++;
    }

    if(data[0] == '!')
    {
        ds_ptr->not_flag = 1;
        start = &data[1];
    }
    else
    {
        start = &data[0];
    }

    if(strchr(start, (int) 'x') == NULL && strchr(start, (int)'X') == NULL)
    {
        ds_ptr->ip_tos = (uint8_t)SnortStrtoulRange(start, &endTok, 10, 0, UINT8_MAX);
        if ((endTok == start) || (*endTok != '\0'))
        {
            FatalError("%s(%d) => Invalid parameter '%s' to 'tos' (not a "
                    "number?) \n", file_name, file_line, data);
        }
    }
    else
    {
        /* hex? */
        start = strchr(data,(int)'x');
        if(!start)
        {
            start = strchr(data,(int)'X');
        }
        if (start)
        {
            ds_ptr->ip_tos = (uint8_t)SnortStrtoulRange(start+1, &endTok, 16, 0, UINT8_MAX);
        }
        if (!start || (endTok == start+1) || (*endTok != '\0'))
        {
            FatalError("%s(%d) => Invalid parameter '%s' to 'tos' (not a "
                    "number?) \n", file_name, file_line, data);
        }
    }

    if (add_detection_option(sc, RULE_OPTION_TYPE_IP_TOS, (void *)ds_ptr, &ds_ptr_dup) == DETECTION_OPTION_EQUAL)
    {
        free(ds_ptr);
        ds_ptr = otn->ds_list[PLUGIN_IP_TOS_CHECK] = ds_ptr_dup;
    }

    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,"TOS set to %d\n", ds_ptr->ip_tos););
}


/****************************************************************************
 *
 * Function: IpTosCheckEq(char *, OptTreeNode *)
 *
 * Purpose: Test the ip header's tos field to see if its value is equal to the
 *          value in the rule.  This is useful to detect things like the
 *	    "bubonic" DoS tool.
 *
 * Arguments: data => argument data
 *            otn => pointer to the current rule's OTN
 *
 * Returns: void function
 *
 ****************************************************************************/
int IpTosCheckEq(void *option_data, Packet *p)
{
    IpTosCheckData *ipTosCheckData = (IpTosCheckData *)option_data;
    int rval = DETECTION_OPTION_NO_MATCH;
    PROFILE_VARS;

    if(!IPH_IS_VALID(p))
        return rval; /* if error occured while ip header
                   * was processed, return 0 automagically.  */

    PREPROC_PROFILE_START(ipTosPerfStats);

    if((ipTosCheckData->ip_tos == GET_IPH_TOS(p)) ^ (ipTosCheckData->not_flag))
    {
        rval = DETECTION_OPTION_MATCH;
    }
    else
    {
        /* you can put debug comments here or not */
        DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,"No match\n"););
    }

    /* if the test isn't successful, return 0 */
    PREPROC_PROFILE_END(ipTosPerfStats);
    return rval;
}
