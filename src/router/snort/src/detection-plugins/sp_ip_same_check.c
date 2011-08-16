/*
** Copyright (C) 2002-2011 Sourcefire, Inc.
** Copyright (C) 1998-2002 Martin Roesch <roesch@sourcefire.com>
** Copyright (C) 2001 Phil Wood <cpw@lanl.gov>
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
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <stdlib.h>
#include <ctype.h>

#include "rules.h"
#include "treenodes.h"
#include "decode.h"
#include "plugbase.h"
#include "parser.h"
#include "debug.h"
#include "util.h"
#include "plugin_enum.h"

#include "snort.h"
#include "profiler.h"
#ifdef PERF_PROFILING
PreprocStats ipSamePerfStats;
extern PreprocStats ruleOTNEvalPerfStats;
#endif

#include "detection_options.h"

typedef struct _IpSameData
{
    u_char ip_same;

} IpSameData;

void IpSameCheckInit(char *, OptTreeNode *, int);
void ParseIpSame(char *, OptTreeNode *);
int IpSameCheck(void *option_data, Packet *p);

uint32_t IpSameCheckHash(void *d)
{
    uint32_t a,b,c;

    /* NO data stored for the option */

    a = RULE_OPTION_TYPE_IP_SAME;
    b = 0;
    c = 0;

    final(a,b,c);

    return c;
}

int IpSameCheckCompare(void *l, void *r)
{
    /* NO data stored for the option */
    return DETECTION_OPTION_EQUAL;
}


/****************************************************************************
 * 
 * Function: SetupIpSameCheck()
 *
 * Purpose: Associate the same keyword with IpSameCheckInit
 *
 * Arguments: None.
 *
 * Returns: void function
 *
 ****************************************************************************/
void SetupIpSameCheck(void)
{
    /* map the keyword to an initialization/processing function */
    RegisterRuleOption("sameip", IpSameCheckInit, NULL, OPT_TYPE_DETECTION, NULL);
#ifdef PERF_PROFILING
    RegisterPreprocessorProfile("sameip", &ipSamePerfStats, 3, &ruleOTNEvalPerfStats);
#endif
    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "Plugin: IpSameCheck Initialized\n"););
}


/****************************************************************************
 * 
 * Function: IpSameCheckInit(char *, OptTreeNode *)
 *
 * Purpose: Setup the same data struct and link the function into option
 *          function pointer list
 *
 * Arguments: data => rule arguments/data
 *            otn => pointer to the current rule option list node
 *
 * Returns: void function
 *
 ****************************************************************************/
void IpSameCheckInit(char *data, OptTreeNode *otn, int protocol)
{
    OptFpList *fpl;
    void *ds_ptr_dup;

    /* multiple declaration check */ 
    if(otn->ds_list[PLUGIN_IP_SAME_CHECK])
    {
        FatalError("%s(%d): Multiple sameip options in rule\n", file_name,
                file_line);
    }

    /* allocate the data structure and attach it to the
       rule's data struct list */
    otn->ds_list[PLUGIN_IP_SAME_CHECK] = (void *)1; /* Just store something there */
    //otn->ds_list[PLUGIN_IP_SAME_CHECK] = (IpSameData *)
    //        SnortAlloc(sizeof(IpSameData));

    /* this is where the keyword arguments are processed and placed into the 
       rule option's data structure */
    ParseIpSame(data, otn);

    if (add_detection_option(RULE_OPTION_TYPE_IP_SAME, (void *)NULL, &ds_ptr_dup) == DETECTION_OPTION_EQUAL)
    {
        //otn->ds_list[PLUGIN_IP_SAME_CHECK] = ds_ptr_dup;
    }

    /* finally, attach the option's detection function to the rule's 
       detect function pointer list */
    fpl = AddOptFuncToList(IpSameCheck, otn);
    fpl->type = RULE_OPTION_TYPE_IP_SAME;
}



/****************************************************************************
 * 
 * Function: ParseIpSame(char *, OptTreeNode *)
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
void ParseIpSame(char *data, OptTreeNode *otn)
{
    return; /* the check below bombs. */

#if 0
    IpSameData *ds_ptr;  /* data struct pointer */

    /* set the ds pointer to make it easier to reference the option's
       particular data struct */
    ds_ptr = otn->ds_list[PLUGIN_IP_SAME_CHECK];

    /* get rid of any whitespace */
    while(isspace((int)*data))
    {
        data++;
    }
    if (*data) {
        FatalError("%s(%d): arg '%s' not required\n", file_name, file_line, data);
    }
#endif
}


/****************************************************************************
 * 
 * Function: IpSameCheck(char *, OptTreeNode *)
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
int IpSameCheck(void *option_data, Packet *p)
{
    int rval = DETECTION_OPTION_NO_MATCH;
    PROFILE_VARS;

    if(!IPH_IS_VALID(p))
        return rval; /* if error occured while ip header
                   * was processed, return 0 automagically.  */

    PREPROC_PROFILE_START(ipSamePerfStats);

    if (IP_EQUALITY( GET_SRC_IP(p), GET_DST_IP(p)))
#ifdef SUP_IP6
    {
	    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,"Match!  %x ->",
                    sfip_ntoa(GET_SRC_IP(p)));
               DebugMessage(DEBUG_PLUGIN, " %x\n",
                    sfip_ntoa(GET_DST_IP(p))));
        rval = DETECTION_OPTION_MATCH;
    }
    else
    {
    	DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,"No match!  %x ->",
                    sfip_ntoa(GET_SRC_IP(p)));
               DebugMessage(DEBUG_PLUGIN, " %x\n",
                    sfip_ntoa(GET_DST_IP(p))));
    }
#else
    {
	    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,"Match!  %x -> %x\n",
				p->iph->ip_src.s_addr,  p->iph->ip_dst.s_addr););
        rval = DETECTION_OPTION_MATCH;
    }
    else
    {
        /* you can put debug comments here or not */
        DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,"No match %x -> %x\n",
				p->iph->ip_src.s_addr,  p->iph->ip_dst.s_addr););
    }
#endif

    /* if the test isn't successful, return 0 */
    PREPROC_PROFILE_END(ipSamePerfStats);
    return rval;
}
