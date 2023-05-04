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
#include <string.h>
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
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
PreprocStats tcpWinPerfStats;
extern PreprocStats ruleOTNEvalPerfStats;
#endif

#include "sfhashfcn.h"
#include "detection_options.h"

typedef struct _TcpWinCheckData
{
    uint16_t tcp_win;
    uint8_t not_flag;

} TcpWinCheckData;

void TcpWinCheckInit(struct _SnortConfig *, char *, OptTreeNode *, int);
void ParseTcpWin(struct _SnortConfig *, char *, OptTreeNode *);
int TcpWinCheckEq(void *option_data, Packet *p);

uint32_t TcpWinCheckHash(void *d)
{
    uint32_t a,b,c;
    TcpWinCheckData *data = (TcpWinCheckData *)d;

    a = data->tcp_win;
    b = data->not_flag;
    c = RULE_OPTION_TYPE_TCP_WIN;

    final(a,b,c);

    return c;
}

int TcpWinCheckCompare(void *l, void *r)
{
    TcpWinCheckData *left = (TcpWinCheckData *)l;
    TcpWinCheckData *right = (TcpWinCheckData *)r;

    if (!left || !right)
        return DETECTION_OPTION_NOT_EQUAL;

    if ((left->tcp_win == right->tcp_win) &&
        (left->not_flag == right->not_flag))
    {
        return DETECTION_OPTION_EQUAL;
    }

    return DETECTION_OPTION_NOT_EQUAL;
}



/****************************************************************************
 *
 * Function: SetupTcpWinCheck()
 *
 * Purpose: Associate the window keyword with TcpWinCheckInit
 *
 * Arguments: None.
 *
 * Returns: void function
 *
 ****************************************************************************/
void SetupTcpWinCheck(void)
{
    /* map the keyword to an initialization/processing function */
    RegisterRuleOption("window", TcpWinCheckInit, NULL, OPT_TYPE_DETECTION, NULL);
#ifdef PERF_PROFILING
    RegisterPreprocessorProfile("window", &tcpWinPerfStats, 3, &ruleOTNEvalPerfStats, NULL);
#endif
}


/****************************************************************************
 *
 * Function: TcpWinCheckInit(struct _SnortConfig *, char *, OptTreeNode *)
 *
 * Purpose: Setup the window data struct and link the function into option
 *          function pointer list
 *
 * Arguments: data => rule arguments/data
 *            otn => pointer to the current rule option list node
 *
 * Returns: void function
 *
 ****************************************************************************/
void TcpWinCheckInit(struct _SnortConfig *sc, char *data, OptTreeNode *otn, int protocol)
{
    OptFpList *fpl;
    if(protocol != IPPROTO_TCP)
    {
        FatalError("%s(%d): TCP Options on non-TCP rule\n",
                   file_name, file_line);
    }

    /* multiple declaration check */
    if(otn->ds_list[PLUGIN_TCP_WIN_CHECK])
    {
        FatalError("%s(%d): Multiple TCP window options in rule\n", file_name,
                file_line);
    }

    /* allocate the data structure and attach it to the
       rule's data struct list */
    otn->ds_list[PLUGIN_TCP_WIN_CHECK] = (TcpWinCheckData *)
            SnortAlloc(sizeof(TcpWinCheckData));

    /* this is where the keyword arguments are processed and placed into the
       rule option's data structure */
    ParseTcpWin(sc, data, otn);

    /* finally, attach the option's detection function to the rule's
       detect function pointer list */
    fpl = AddOptFuncToList(TcpWinCheckEq, otn);
    fpl->type = RULE_OPTION_TYPE_TCP_WIN;
    fpl->context = otn->ds_list[PLUGIN_TCP_WIN_CHECK];
}



/****************************************************************************
 *
 * Function: ParseTcpWin(struct _SnortConfig *, char *, OptTreeNode *)
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
void ParseTcpWin(struct _SnortConfig *sc, char *data, OptTreeNode *otn)
{
    TcpWinCheckData *ds_ptr;  /* data struct pointer */
    void *ds_ptr_dup;
    int win_size = 0;
    char *endTok;
    char *start;

    /* set the ds pointer to make it easier to reference the option's
       particular data struct */
    ds_ptr = otn->ds_list[PLUGIN_TCP_WIN_CHECK];

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
        win_size = SnortStrtolRange(start, &endTok, 10, 0, UINT16_MAX);
        if ((endTok == start) || (*endTok != '\0'))
        {
            FatalError("%s(%d) => Invalid parameter '%s' to 'window' (not a "
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
            win_size = SnortStrtolRange(start+1, &endTok, 16, 0, UINT16_MAX);
        }
        if (!start || (endTok == start+1) || (*endTok != '\0'))
        {
            FatalError("%s(%d) => Invalid parameter '%s' to 'window' (not a "
                    "number?) \n", file_name, file_line, data);
        }
    }

    ds_ptr->tcp_win = htons((uint16_t)win_size);

#ifdef DEBUG_MSGS
    DebugMessage(DEBUG_PLUGIN,"TCP Window set to 0x%X\n", ds_ptr->tcp_win);
#endif

    if (add_detection_option(sc, RULE_OPTION_TYPE_TCP_WIN, (void *)ds_ptr, &ds_ptr_dup) == DETECTION_OPTION_EQUAL)
    {
        otn->ds_list[PLUGIN_TCP_WIN_CHECK] = ds_ptr_dup;
        free(ds_ptr);
    }
}


/****************************************************************************
 *
 * Function: TcpWinCheckEq(char *, OptTreeNode *)
 *
 * Purpose: Test the TCP header's window to see if its value is equal to the
 *          value in the rule.
 *
 * Arguments: data => argument data
 *            otn => pointer to the current rule's OTN
 *
 * Returns: void function
 *
 ****************************************************************************/
int TcpWinCheckEq(void *option_data, Packet *p)
{
    TcpWinCheckData *tcpWinCheckData = (TcpWinCheckData *)option_data;
    int rval = DETECTION_OPTION_NO_MATCH;
    PROFILE_VARS;

    if(!p->tcph)
        return rval; /* if error occured while ip header
                   * was processed, return 0 automagically.  */

    PREPROC_PROFILE_START(tcpWinPerfStats);

    if((tcpWinCheckData->tcp_win == p->tcph->th_win) ^ (tcpWinCheckData->not_flag))
    {
        rval = DETECTION_OPTION_MATCH;
    }
#ifdef DEBUG_MSGS
    else
    {
        /* you can put debug comments here or not */
        DebugMessage(DEBUG_PLUGIN,"No match\n");
    }
#endif

    /* if the test isn't successful, return 0 */
    PREPROC_PROFILE_END(tcpWinPerfStats);
    return rval;
}
