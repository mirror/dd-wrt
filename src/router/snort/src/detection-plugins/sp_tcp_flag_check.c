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

#define M_NORMAL  0
#define M_ALL     1
#define M_ANY     2
#define M_NOT     3

#include "snort.h"
#include "profiler.h"
#ifdef PERF_PROFILING
PreprocStats tcpFlagsPerfStats;
extern PreprocStats ruleOTNEvalPerfStats;
#endif

#include "sfhashfcn.h"
#include "detection_options.h"

typedef struct _TCPFlagCheckData
{
    u_char mode;
    u_char tcp_flags;
    u_char tcp_mask; /* Mask to take away from the flags check */

} TCPFlagCheckData;

void TCPFlagCheckInit(struct _SnortConfig *, char *, OptTreeNode *, int);
void ParseTCPFlags(struct _SnortConfig *, char *, OptTreeNode *);
int CheckTcpFlags(void *option_data, Packet *p);

uint32_t TcpFlagCheckHash(void *d)
{
    uint32_t a,b,c;
    TCPFlagCheckData *data = (TCPFlagCheckData *)d;

    a = data->mode;
    b = data->tcp_flags || (data->tcp_mask << 8);
    c = RULE_OPTION_TYPE_TCP_FLAG;

    final(a,b,c);

    return c;
}

int TcpFlagCheckCompare(void *l, void *r)
{
    TCPFlagCheckData *left = (TCPFlagCheckData *)l;
    TCPFlagCheckData *right = (TCPFlagCheckData *)r;

    if (!left || !right)
        return DETECTION_OPTION_NOT_EQUAL;

    if ((left->mode == right->mode) &&
        (left->tcp_flags == right->tcp_flags) &&
        (left->tcp_mask == right->tcp_mask))
    {
        return DETECTION_OPTION_EQUAL;
    }

    return DETECTION_OPTION_NOT_EQUAL;
}


void SetupTCPFlagCheck(void)
{
    RegisterRuleOption("flags", TCPFlagCheckInit, NULL, OPT_TYPE_DETECTION, NULL);
#ifdef PERF_PROFILING
    RegisterPreprocessorProfile("flags", &tcpFlagsPerfStats, 3, &ruleOTNEvalPerfStats, NULL);
#endif
    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "Plugin: TCPFlagCheck Initialized!\n"););
}



void TCPFlagCheckInit(struct _SnortConfig *sc, char *data, OptTreeNode *otn, int protocol)
{
    OptFpList *fpl;

    if(protocol != IPPROTO_TCP)
    {
        FatalError("Line %s (%d): TCP Options on non-TCP rule\n", file_name, file_line);
    }

    /* multiple declaration check */
    if(otn->ds_list[PLUGIN_TCP_FLAG_CHECK])
    {
        FatalError("%s(%d): Multiple TCP flags options in rule\n", file_name,
                file_line);
    }

    otn->ds_list[PLUGIN_TCP_FLAG_CHECK] = (TCPFlagCheckData *)
            SnortAlloc(sizeof(TCPFlagCheckData));

    /* set up the pattern buffer */
    ParseTCPFlags(sc, data, otn);

    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "Adding TCP flag check function (%p) to list\n",
			    CheckTcpFlags););

    /* link the plugin function in to the current OTN */
    fpl = AddOptFuncToList(CheckTcpFlags, otn);
    fpl->type = RULE_OPTION_TYPE_TCP_FLAG;
    fpl->context = otn->ds_list[PLUGIN_TCP_FLAG_CHECK];

    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "OTN function CheckTcpFlags added to rule!\n"););
}



/****************************************************************************
 *
 * Function: ParseTCPflags(struct _SnortConfig *, char *, OptTreeNode *)
 *
 * Purpose: Figure out which TCP flags the current rule is interested in
 *
 * Arguments: rule => the rule string
 *
 * Returns: void function
 *
 ***************************************************************************/
void ParseTCPFlags(struct _SnortConfig *sc, char *rule, OptTreeNode *otn)
{
    char *fptr;
    char *fend;
    int comma_set = 0;
    TCPFlagCheckData *idx;
    void *ds_ptr_dup;

    idx = otn->ds_list[PLUGIN_TCP_FLAG_CHECK];

    fptr = rule;

    /* make sure there is atleast a split pointer */
    if(fptr == NULL)
    {
        FatalError("[!] Line %s (%d): Flags missing in TCP flag rule\n", file_name, file_line);
    }

    while(isspace((u_char) *fptr))
        fptr++;

    if(strlen(fptr) == 0)
    {
        FatalError("[!] Line %s (%d): Flags missing in TCP flag rule\n", file_name, file_line);
    }

    /* find the end of the alert string */
    fend = fptr + strlen(fptr);

    idx->mode = M_NORMAL; /* this is the default, unless overridden */

    while(fptr < fend && comma_set == 0)
    {
        switch(*fptr)
        {
            case 'f':
            case 'F':
                idx->tcp_flags |= R_FIN;
                break;

            case 's':
            case 'S':
                idx->tcp_flags |= R_SYN;
                break;

            case 'r':
            case 'R':
                idx->tcp_flags |= R_RST;
                break;

            case 'p':
            case 'P':
                idx->tcp_flags |= R_PSH;
                break;

            case 'a':
            case 'A':
                idx->tcp_flags |= R_ACK;
                break;

            case 'u':
            case 'U':
                idx->tcp_flags |= R_URG;
                break;

            case '0':
                idx->tcp_flags = 0;
                break;

            case '1': /* reserved bit flags */
            case 'c':
            case 'C':
                idx->tcp_flags |= R_CWR; /* Congestion Window Reduced, RFC 3168 */
                break;

            case '2': /* reserved bit flags */
            case 'e':
            case 'E':
                idx->tcp_flags |= R_ECE; /* ECN echo, RFC 3168 */
                break;

            case '!': /* not, fire if all flags specified are not present,
                         other are don't care */
                idx->mode = M_NOT;
                break;
            case '*': /* star or any, fire if any flags specified are
                         present, other are don't care */
                idx->mode = M_ANY;
                break;
            case '+': /* plus or all, fire if all flags specified are
                         present, other are don't care */
                idx->mode = M_ALL;
                break;
            case ',':
                comma_set = 1;
                break;
            default:
                FatalError("%s(%d): bad TCP flag = \"%c\"\n"
                           "Valid otions: UAPRSFCE or 0 for NO flags (e.g. NULL scan),"
                           " and !, + or * for modifiers\n",
                           file_name, file_line, *fptr);
        }

        fptr++;
    }

    while(isspace((u_char) *fptr))
        fptr++;


    /* create the mask portion now */
    while(fptr < fend && comma_set == 1)
    {
        switch(*fptr)
        {
            case 'f':
            case 'F':
                idx->tcp_mask |= R_FIN;
                break;

            case 's':
            case 'S':
                idx->tcp_mask |= R_SYN;
                break;

            case 'r':
            case 'R':
                idx->tcp_mask |= R_RST;
                break;

            case 'p':
            case 'P':
                idx->tcp_mask |= R_PSH;
                break;

            case 'a':
            case 'A':
                idx->tcp_mask |= R_ACK;
                break;

            case 'u':
            case 'U':
                idx->tcp_mask |= R_URG;
                break;

            case '1': /* reserved bit flags */
            case 'c':
            case 'C':
                idx->tcp_mask |= R_CWR; /* Congestion Window Reduced, RFC 3168 */
                break;

            case '2': /* reserved bit flags */
            case 'e':
            case 'E':
                idx->tcp_mask |= R_ECE; /* ECN echo, RFC 3168 */
                break;
            default:
                FatalError(" Line %s (%d): bad TCP flag = \"%c\"\n  Valid otions: UAPRSFCE \n",
                           file_name, file_line, *fptr);
        }

        fptr++;
    }

    if (add_detection_option(sc, RULE_OPTION_TYPE_TCP_FLAG, (void *)idx, &ds_ptr_dup) == DETECTION_OPTION_EQUAL)
    {
        otn->ds_list[PLUGIN_TCP_FLAG_CHECK] = ds_ptr_dup;
        free(idx);
    }
}

int CheckTcpFlags(void *option_data, Packet *p)
{
    TCPFlagCheckData *flagptr = (TCPFlagCheckData *)option_data;
    int rval = DETECTION_OPTION_NO_MATCH;
    u_char tcp_flags;
    PROFILE_VARS;

    PREPROC_PROFILE_START(tcpFlagsPerfStats);

    if(!p->tcph)
    {
        /* if error appeared when tcp header was processed,
         * test fails automagically */
        PREPROC_PROFILE_END(tcpFlagsPerfStats);
        return rval;
    }

    /* the flags we really want to check are all the ones
     */

    tcp_flags = p->tcph->th_flags & (0xFF ^ flagptr->tcp_mask);

    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "           <!!> CheckTcpFlags: "););

    switch((flagptr->mode))
    {
        case M_NORMAL:
            if(flagptr->tcp_flags == tcp_flags) /* only these set */
            {
                DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,"Got TCP [default] flag match!\n"););
                rval = DETECTION_OPTION_MATCH;
            }
            else
            {
                DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,"No match\n"););
            }
            break;

        case M_ALL:
            /* all set */
            if((flagptr->tcp_flags & tcp_flags) == flagptr->tcp_flags)
            {
                DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "Got TCP [ALL] flag match!\n"););
                rval = DETECTION_OPTION_MATCH;
            }
            else
            {
                DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,"No match\n"););
            }
            break;

        case M_NOT:
            if((flagptr->tcp_flags & tcp_flags) == 0)  /* none set */
            {
                DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,"Got TCP [NOT] flag match!\n"););
                rval = DETECTION_OPTION_MATCH;
            }
            else
            {
                DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "No match\n"););
            }
            break;

        case M_ANY:
            if((flagptr->tcp_flags & tcp_flags) != 0)  /* something set */
            {
                DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,"Got TCP [ANY] flag match!\n"););
                rval = DETECTION_OPTION_MATCH;
            }
            else
            {
                DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,"No match\n"););
            }
            break;

        default:  /* Should never see this */
            DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "TCP flag check went to default case"
				    " for some silly reason\n"););
            break;
    }

    PREPROC_PROFILE_END(tcpFlagsPerfStats);
    return rval;
}
