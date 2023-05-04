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
#include "parser.h"
#include "plugbase.h"
#include "snort_debug.h"
#include "plugin_enum.h"
#include "util.h"

#include "snort.h"
#include "profiler.h"
#ifdef PERF_PROFILING
PreprocStats ipIdPerfStats;
extern PreprocStats ruleOTNEvalPerfStats;
#endif

#include "sfhashfcn.h"
#include "detection_options.h"

typedef struct _IpIdCheckData
{
    u_long ip_id;

} IpIdCheckData;

void IpIdCheckInit(struct _SnortConfig *, char *, OptTreeNode *, int);
void ParseIpId(struct _SnortConfig *, char *, OptTreeNode *);
int IpIdCheckEq(void *option_data, Packet *p);

uint32_t IpIdCheckHash(void *d)
{
    uint32_t a,b,c;
    IpIdCheckData *data = (IpIdCheckData *)d;

    a = data->ip_id;
    b = RULE_OPTION_TYPE_IP_ID;
    c = 0;

    final(a,b,c);

    return c;
}

int IpIdCheckCompare(void *l, void *r)
{
    IpIdCheckData *left = (IpIdCheckData *)l;
    IpIdCheckData *right = (IpIdCheckData *)r;

    if (!left || !right)
        return DETECTION_OPTION_NOT_EQUAL;

    if (left->ip_id == right->ip_id)
    {
        return DETECTION_OPTION_EQUAL;
    }

    return DETECTION_OPTION_NOT_EQUAL;
}

/****************************************************************************
 *
 * Function: SetupIpIdCheck()
 *
 * Purpose: Associate the id keyword with IpIdCheckInit
 *
 * Arguments: None.
 *
 * Returns: void function
 *
 ****************************************************************************/
void SetupIpIdCheck(void)
{
    /* map the keyword to an initialization/processing function */
    RegisterRuleOption("id", IpIdCheckInit, NULL, OPT_TYPE_DETECTION, NULL);
#ifdef PERF_PROFILING
    RegisterPreprocessorProfile("id", &ipIdPerfStats, 3, &ruleOTNEvalPerfStats, NULL);
#endif

    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,"Plugin: IpIdCheck Initialized\n"););
}


/****************************************************************************
 *
 * Function: IpIdCheckInit(struct _SnortConfig *, char *, OptTreeNode *)
 *
 * Purpose: Setup the id data struct and link the function into option
 *          function pointer list
 *
 * Arguments: data => rule arguments/data
 *            otn => pointer to the current rule option list node
 *
 * Returns: void function
 *
 ****************************************************************************/
void IpIdCheckInit(struct _SnortConfig *sc, char *data, OptTreeNode *otn, int protocol)
{
    OptFpList *fpl;
    /* multiple declaration check */
    if(otn->ds_list[PLUGIN_IP_ID_CHECK])
    {
        FatalError("%s(%d): Multiple IP id options in rule\n", file_name,
                file_line);
    }

    /* allocate the data structure and attach it to the
       rule's data struct list */
    otn->ds_list[PLUGIN_IP_ID_CHECK] = (IpIdCheckData *)
            SnortAlloc(sizeof(IpIdCheckData));

    /* this is where the keyword arguments are processed and placed into the
       rule option's data structure */
    ParseIpId(sc, data, otn);

    /* finally, attach the option's detection function to the rule's
       detect function pointer list */
    fpl = AddOptFuncToList(IpIdCheckEq, otn);
    fpl->type = RULE_OPTION_TYPE_IP_ID;
    fpl->context = otn->ds_list[PLUGIN_IP_ID_CHECK];
}



/****************************************************************************
 *
 * Function: ParseIpId(struct _SnortConfig *, char *, OptTreeNode *)
 *
 * Purpose: Convert the id option argument to data and plug it into the
 *          data structure
 *
 * Arguments: data => argument data
 *            otn => pointer to the current rule's OTN
 *
 * Returns: void function
 *
 ****************************************************************************/
void ParseIpId(struct _SnortConfig *sc, char *data, OptTreeNode *otn)
{
    IpIdCheckData *ds_ptr;  /* data struct pointer */
    void *ds_ptr_dup;
    int ip_id;
    char *endTok;

    /* set the ds pointer to make it easier to reference the option's
       particular data struct */
    ds_ptr = otn->ds_list[PLUGIN_IP_ID_CHECK];

    /* get rid of any whitespace */
    while(isspace((int)*data))
    {
        data++;
    }

    ip_id = SnortStrtolRange(data, &endTok, 10, 0, UINT16_MAX);
    if ((endTok == data) || (*endTok != '\0'))
    {
        FatalError("%s(%d) => Invalid parameter '%s' to id (not a "
                   "number?) \n", file_name, file_line, data);
    }
    ds_ptr->ip_id = htons( (u_short) ip_id);

    if (add_detection_option(sc, RULE_OPTION_TYPE_IP_ID, (void *)ds_ptr, &ds_ptr_dup) == DETECTION_OPTION_EQUAL)
    {
        free(ds_ptr);
        ds_ptr = otn->ds_list[PLUGIN_IP_ID_CHECK] = ds_ptr_dup;
    }

    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,"ID set to %ld\n", ds_ptr->ip_id););
}


/****************************************************************************
 *
 * Function: IpIdCheckEq(char *, OptTreeNode *)
 *
 * Purpose: Test the ip header's id field to see if its value is equal to the
 *          value in the rule.  This is useful to detect things like "elite"
 *          numbers, oddly repeating numbers, etc.
 *
 * Arguments: data => argument data
 *            otn => pointer to the current rule's OTN
 *
 * Returns: void function
 *
 ****************************************************************************/
int IpIdCheckEq(void *option_data, Packet *p)
{
    IpIdCheckData *ipIdCheckData = (IpIdCheckData *)option_data;
    int rval = DETECTION_OPTION_NO_MATCH;
    PROFILE_VARS;

    if(!IPH_IS_VALID(p))
        return rval; /* if error occured while ip header
                   * was processed, return 0 automagically.  */

    PREPROC_PROFILE_START(ipIdPerfStats);

    if(ipIdCheckData->ip_id == GET_IPH_ID(p))
    {
        /* call the next function in the function list recursively */
        rval = DETECTION_OPTION_MATCH;
    }
    else
    {
        /* you can put debug comments here or not */
        DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "No match for sp_ip_id_check\n"););
    }

    /* if the test isn't successful, return 0 */
    PREPROC_PROFILE_END(ipIdPerfStats);
    return rval;
}
