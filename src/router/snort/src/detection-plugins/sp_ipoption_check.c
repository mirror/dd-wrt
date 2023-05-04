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
PreprocStats ipOptionPerfStats;
extern PreprocStats ruleOTNEvalPerfStats;
#endif

#include "sfhashfcn.h"
#include "detection_options.h"

typedef struct _IpOptionData
{
    u_char ip_option;
    u_char any_flag;

} IpOptionData;

void IpOptionInit(struct _SnortConfig *, char *, OptTreeNode *, int);
void ParseIpOptionData(struct _SnortConfig *, char *, OptTreeNode *);
int CheckIpOptions(void *option_data, Packet *p);

uint32_t IpOptionCheckHash(void *d)
{
    uint32_t a,b,c;
    IpOptionData *data = (IpOptionData *)d;

    a = data->ip_option;
    b = data->any_flag;
    c = RULE_OPTION_TYPE_IP_OPTION;

    final(a,b,c);

    return c;
}

int IpOptionCheckCompare(void *l, void *r)
{
    IpOptionData *left = (IpOptionData *)l;
    IpOptionData *right = (IpOptionData *)r;

    if (!left || !right)
        return DETECTION_OPTION_NOT_EQUAL;

    if ((left->ip_option == right->ip_option) &&
        (left->any_flag == right->any_flag))
    {
        return DETECTION_OPTION_EQUAL;
    }

    return DETECTION_OPTION_NOT_EQUAL;
}

/****************************************************************************
 *
 * Function: SetupTemplate()
 *
 * Purpose: Generic detection engine plugin template.  Registers the
 *          configuration function and links it to a rule keyword.
 *
 * Arguments: None.
 *
 * Returns: void function
 *
 ****************************************************************************/
void SetupIpOptionCheck(void)
{
    /* map the keyword to an initialization/processing function */
    RegisterRuleOption("ipopts", IpOptionInit, NULL, OPT_TYPE_DETECTION, NULL);
#ifdef PERF_PROFILING
    RegisterPreprocessorProfile("ipopts", &ipOptionPerfStats, 3, &ruleOTNEvalPerfStats, NULL);
#endif
    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,"Plugin: IpOptionCheck Initialized\n"););
}


/****************************************************************************
 *
 * Function: TemplateInit(struct _SnortConfig *, char *, OptTreeNode *)
 *
 * Purpose: Generic rule configuration function.  Handles parsing the rule
 *          information and attaching the associated detection function to
 *          the OTN.
 *
 * Arguments: data => rule arguments/data
 *            otn => pointer to the current rule option list node
 *
 * Returns: void function
 *
 ****************************************************************************/
void IpOptionInit(struct _SnortConfig *sc, char *data, OptTreeNode *otn, int protocol)
{
    OptFpList *fpl;
    /* multiple declaration check */
    if(otn->ds_list[PLUGIN_IPOPTION_CHECK])
    {
        FatalError("%s(%d): Multiple ipopts options in rule\n", file_name,
                file_line);
    }

    /* allocate the data structure and attach it to the
       rule's data struct list */
    otn->ds_list[PLUGIN_IPOPTION_CHECK] = (IpOptionData *)
            SnortAlloc(sizeof(IpOptionData));

    /* this is where the keyword arguments are processed and placed into the
       rule option's data structure */
    ParseIpOptionData(sc, data, otn);

    /* finally, attach the option's detection function to the rule's
       detect function pointer list */
    fpl = AddOptFuncToList(CheckIpOptions, otn);
    fpl->type = RULE_OPTION_TYPE_IP_OPTION;
    fpl->context = otn->ds_list[PLUGIN_IPOPTION_CHECK];
}



/****************************************************************************
 *
 * Function: ParseIpOptionData(struct _SnortConfig *, char *, OptTreeNode *)
 *
 * Purpose: This is the function that is used to process the option keyword's
 *          arguments and attach them to the rule's data structures.
 *
 * Arguments: data => argument data
 *            otn => pointer to the current rule's OTN
 *
 * Returns: void function
 *
 ****************************************************************************/
void ParseIpOptionData(struct _SnortConfig *sc, char *data, OptTreeNode *otn)
{
    IpOptionData *ds_ptr;  /* data struct pointer */
    void *ds_ptr_dup;

    /* set the ds pointer to make it easier to reference the option's
       particular data struct */
    ds_ptr = otn->ds_list[PLUGIN_IPOPTION_CHECK];

    if(data == NULL)
    {
        FatalError("%s(%d): IP Option keyword missing argument!\n", file_name, file_line);
    }

    while(isspace((u_char)*data))
        data++;


    if(strcasecmp(data, "rr") == 0)
    {
        ds_ptr->ip_option = IPOPT_RR;
    }
    else if(strcasecmp(data, "eol") == 0)
    {
        ds_ptr->ip_option = IPOPT_EOL;
    }
    else if(strcasecmp(data, "nop") == 0)
    {
        ds_ptr->ip_option = IPOPT_NOP;
    }
    else if(strcasecmp(data, "ts") == 0)
    {
        ds_ptr->ip_option = IPOPT_TS;
    }
    else if(strcasecmp(data, "esec") == 0)
    {
        ds_ptr->ip_option = IPOPT_ESEC;
    }
    else if(strcasecmp(data, "sec") == 0)
    {
        ds_ptr->ip_option = IPOPT_SECURITY;
    }
    else if(strcasecmp(data, "lsrr") == 0)
    {
        ds_ptr->ip_option = IPOPT_LSRR;
    }
    else if(strcasecmp(data, "lsrre") == 0)
    {
        ds_ptr->ip_option = IPOPT_LSRR_E;
    }
    else if(strcasecmp(data, "satid") == 0)
    {
        ds_ptr->ip_option = IPOPT_SATID;
    }
    else if(strcasecmp(data, "ssrr") == 0)
    {
        ds_ptr->ip_option = IPOPT_SSRR;
    }
    else if(strcasecmp(data, "any") == 0)
    {
        ds_ptr->ip_option = 0;
        ds_ptr->any_flag = 1;
    }
    else
    {
        FatalError("%s(%d) => Unknown IP option argument: %s!\n",
                   file_name, file_line, data);
    }

    if (add_detection_option(sc, RULE_OPTION_TYPE_IP_OPTION, (void *)ds_ptr, &ds_ptr_dup) == DETECTION_OPTION_EQUAL)
    {
        free(ds_ptr);
        ds_ptr = otn->ds_list[PLUGIN_IPOPTION_CHECK] = ds_ptr_dup;
    }

}


/****************************************************************************
 *
 * Function: TemplateDetectorFunction(char *, OptTreeNode *)
 *
 * Purpose: Use this function to perform the particular detection routine
 *          that this rule keyword is supposed to encompass.
 *
 * Arguments: data => argument data
 *            otn => pointer to the current rule's OTN
 *
 * Returns: void function
 *
 ****************************************************************************/
int CheckIpOptions(void *option_data, Packet *p)
{
    IpOptionData *ipOptionData = (IpOptionData *)option_data;
    int rval = DETECTION_OPTION_NO_MATCH;
    int i;
    PROFILE_VARS;

    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "CheckIpOptions:"););
    if(!IPH_IS_VALID(p))
        return rval; /* if error occured while ip header
                   * was processed, return 0 automagically.  */

    PREPROC_PROFILE_START(ipOptionPerfStats);

    if((ipOptionData->any_flag == 1) && (p->ip_option_count > 0))
    {
        DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "Matched any ip options!\n"););
        rval = DETECTION_OPTION_MATCH;
        PREPROC_PROFILE_END(ipOptionPerfStats);
        return rval;
    }

    for(i=0; i< (int) p->ip_option_count; i++)
    {
    	DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "testing pkt(%d):rule(%d)\n",
				ipOptionData->ip_option,
				p->ip_options[i].code); );

        if(ipOptionData->ip_option == p->ip_options[i].code)
        {
            rval = DETECTION_OPTION_MATCH;
            PREPROC_PROFILE_END(ipOptionPerfStats);
            return rval;
        }
    }

    /* if the test isn't successful, return 0 */
    PREPROC_PROFILE_END(ipOptionPerfStats);
    return rval;
}
