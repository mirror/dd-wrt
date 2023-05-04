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
#include <ctype.h>

#include "sf_types.h"
#include "rules.h"
#include "treenodes.h"
#include "decode.h"
#include "snort_debug.h"
#include "plugbase.h"
#include "parser.h"
#include "plugin_enum.h"
#include "util.h"
#include "sfhashfcn.h"

#include "snort.h"
#include "profiler.h"
#ifdef PERF_PROFILING
PreprocStats ttlCheckPerfStats;
extern PreprocStats ruleOTNEvalPerfStats;
#endif

#include "sfhashfcn.h"
#include "detection_options.h"

#define TTL_CHECK_EQ 1
#define TTL_CHECK_GT 2
#define TTL_CHECK_LT 3
#define TTL_CHECK_RG 4
#define TTL_CHECK_GT_EQ 5
#define TTL_CHECK_LT_EQ 6

typedef struct _TtlCheckData
{
    int ttl;
    int h_ttl;
    char oper;
} TtlCheckData;

void TtlCheckInit(struct _SnortConfig *, char *, OptTreeNode *, int);
void ParseTtl(struct _SnortConfig *, char *, OptTreeNode *);
int CheckTtl(void *option_data, Packet *p);

/****************************************************************************
 *
 * Function: SetupTtlCheck()
 *
 * Purpose: Register the ttl option keyword with its setup function
 *
 * Arguments: None.
 *
 * Returns: void function
 *
 ****************************************************************************/
void SetupTtlCheck(void)
{
    /* map the keyword to an initialization/processing function */
    RegisterRuleOption("ttl", TtlCheckInit, NULL, OPT_TYPE_DETECTION, NULL);
#ifdef PERF_PROFILING
    RegisterPreprocessorProfile("ttl_check", &ttlCheckPerfStats, 3, &ruleOTNEvalPerfStats, NULL);
#endif
    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "Plugin: TTLCheck Initialized\n"););
}

uint32_t TtlCheckHash(void *d)
{
    uint32_t a,b,c;
    TtlCheckData *data = (TtlCheckData *)d;

    a = data->ttl;
    b = data->h_ttl;
    c = data->oper;

    mix(a,b,c);

    a += RULE_OPTION_TYPE_TTL;

    final(a,b,c);

    return c;
}

int TtlCheckCompare(void *l, void *r)
{
    TtlCheckData *left = (TtlCheckData *)l;
    TtlCheckData *right = (TtlCheckData *)r;

    if (!left || !right)
        return DETECTION_OPTION_NOT_EQUAL;

    if ((left->ttl == right->ttl) &&
        (left->h_ttl == right->h_ttl) &&
        (left->oper == right->oper))
    {
        return DETECTION_OPTION_EQUAL;
    }

    return DETECTION_OPTION_NOT_EQUAL;
}

/****************************************************************************
 *
 * Function: TtlCheckInit(struct _SnortConfig *, char *, OptTreeNode *)
 *
 * Purpose: Parse the ttl keyword arguments and link the detection module
 *          into the function list
 *
 * Arguments: data => rule arguments/data
 *            otn => pointer to the current rule option list node
 *
 * Returns: void function
 *
 ****************************************************************************/
void TtlCheckInit(struct _SnortConfig *sc, char *data, OptTreeNode *otn, int protocol)
{
    /* multiple declaration check */
    if(otn->ds_list[PLUGIN_TTL_CHECK])
    {
        FatalError("%s(%d): Multiple IP ttl options in rule\n", file_name,
                file_line);
    }

    /* allocate the data structure and attach it to the
       rule's data struct list */
    otn->ds_list[PLUGIN_TTL_CHECK] = (TtlCheckData *)
            SnortAlloc(sizeof(TtlCheckData));

    /* this is where the keyword arguments are processed and placed into the
       rule option's data structure */
    ParseTtl(sc, data, otn);

    /* NOTE: the AddOptFuncToList call is moved to the parsing function since
       the linking is best determined within that function */
}



/****************************************************************************
 *
 * Function: ParseTtl(struct _SnortConfig *, char *, OptTreeNode *)
 *
 * Purpose: Parse the TTL keyword's arguments
 *
 * Arguments: data => argument data
 *            otn => pointer to the current rule's OTN
 *
 * Returns: void function
 *
 ****************************************************************************/
void ParseTtl(struct _SnortConfig *sc, char *data, OptTreeNode *otn)
{
    OptFpList *fpl = NULL;
    TtlCheckData *ds_ptr;  /* data struct pointer */
    void *ds_ptr_dup;
    char ttlrel;
    char *endTok;
    int ttl;
    char *origData = data;
    char *curPtr  = data;
    int equals_present = 0, rel_present =0;

    /* set the ds pointer to make it easier to reference the option's
       particular data struct */
    ds_ptr = (TtlCheckData *)otn->ds_list[PLUGIN_TTL_CHECK];

    if(data == NULL)
    {
        FatalError("%s(%d) => No arguments to 'ttl' \n"
                                , file_name, file_line);
    }

    while(isspace((int)*data)) data++;

    ttlrel = *data;
    curPtr = data;

    switch (ttlrel) {
        case '-':
            ds_ptr->h_ttl = -1; /* leading dash flag */
            data++;
            rel_present = 1;
            break;
        case '>':
        case '<':
            curPtr++;
            while(isspace((int)*curPtr)) curPtr++;
            if((*curPtr) == '=')
            {
                equals_present = 1;
                data = curPtr;
            }
        case '=':
            data++;
            rel_present = 1;
            break;
       default:
            ttlrel = '=';
    }
    while(isspace((int)*data)) data++;

    ttl = SnortStrtol(data, &endTok, 10);
    /* next char after first number must either be - or NULL */
    if ((endTok == data) || ((*endTok != '-') && (*endTok != '\0')))
    {
        FatalError("%s(%d) => Invalid parameter '%s' to 'ttl' (not a "
                "number?) \n", file_name, file_line, origData);
    }

    if (ttl< 0 || ttl > 255)
    {
        FatalError("%s(%d) => Invalid number '%s' to 'ttl' (should be between 0 to  "
                                "255) \n", file_name, file_line, origData);
    }
    ds_ptr->ttl = ttl;

    data = endTok;
    if (*data == '-')
    {
        if(rel_present || (ds_ptr->h_ttl == -1 ))
        {
            FatalError("%s(%d) => Invalid parameter '%s' to 'ttl' (not a "
                "number?) \n", file_name, file_line, origData);
        }
        data++;
        ttlrel = '-';
    }
    switch (ttlrel)
    {
        case '>':
            fpl = AddOptFuncToList(CheckTtl, otn);
            if(equals_present)
                ds_ptr->oper = TTL_CHECK_GT_EQ;
            else
                ds_ptr->oper = TTL_CHECK_GT;
            break;
        case '<':
            fpl = AddOptFuncToList(CheckTtl, otn);
            if(equals_present)
                ds_ptr->oper = TTL_CHECK_LT_EQ;
            else
                ds_ptr->oper = TTL_CHECK_LT;
            break;
        case '=':
            fpl = AddOptFuncToList(CheckTtl, otn);
            ds_ptr->oper = TTL_CHECK_EQ;
            break;
        case '-':
            while(isspace((int)*data)) data++;
            if (ds_ptr->h_ttl != -1)
            {
                if(*data=='\0')
                {
                    ds_ptr->h_ttl = 255;
                }
                else
                {
                    ttl = SnortStrtol(data, &endTok, 10);
                    if ((endTok == data) || (*endTok != '\0') || (ds_ptr->ttl > ttl))
                    {
                        FatalError("%s(%d) => Invalid parameter '%s' to 'ttl' "
                                "(not a number or invalid range?) \n", file_name, file_line, origData);
                    }
                    if (ttl< 0 || ttl > 255)
                    {
                        FatalError("%s(%d) => Invalid number '%s' to 'ttl' (should be between 0 to  "
                                "255) \n", file_name, file_line, origData);
                    }
                    if (ttl == 0)
                        ds_ptr->h_ttl = 255;
                    else
                        ds_ptr->h_ttl = ttl;
                }
            }
            else /* leading dash*/
            {
                ds_ptr->h_ttl = ds_ptr->ttl;
                ds_ptr->ttl   = 0;
            }
            fpl = AddOptFuncToList(CheckTtl, otn);
            ds_ptr->oper = TTL_CHECK_RG;
            break;
        default:
            /* wtf? */
            /* we need at least one statement after "default" or else Visual C++ issues a warning */
            break;
    }

    if (add_detection_option(sc, RULE_OPTION_TYPE_TTL, (void *)ds_ptr, &ds_ptr_dup) == DETECTION_OPTION_EQUAL)
    {
        free(ds_ptr);
        ds_ptr = otn->ds_list[PLUGIN_TTL_CHECK] = ds_ptr_dup;
    }

    if (fpl)
    {
        fpl->type = RULE_OPTION_TYPE_TTL;
        fpl->context = ds_ptr;
    }

    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "Set TTL check value to %c%d (%d)\n", ttlrel, ds_ptr->ttl, ds_ptr->h_ttl););

}


int CheckTtl(void *option_data, Packet *p)
{
    TtlCheckData *ttlCheckData = (TtlCheckData *)option_data;
    int rval = DETECTION_OPTION_NO_MATCH;
    PROFILE_VARS;

    if(!IPH_IS_VALID(p))
        return rval;

    PREPROC_PROFILE_START(ttlCheckPerfStats);

    switch (ttlCheckData->oper)
    {
        case TTL_CHECK_EQ:
            if (ttlCheckData->ttl == GET_IPH_TTL(p))
                rval = DETECTION_OPTION_MATCH;
#ifdef DEBUG_MSGS
            else
            {
                DebugMessage(DEBUG_PLUGIN, "CheckTtlEq: Not equal to %d\n",
                    ttlCheckData->ttl);
            }
#endif
            break;
        case TTL_CHECK_GT:
            if (ttlCheckData->ttl < GET_IPH_TTL(p))
                rval = DETECTION_OPTION_MATCH;
#ifdef DEBUG_MSGS
            else
            {
                DebugMessage(DEBUG_PLUGIN, "CheckTtlEq: Not greater than %d\n",
                    ttlCheckData->ttl);
            }
#endif
            break;
        case TTL_CHECK_LT:
            if (ttlCheckData->ttl > GET_IPH_TTL(p))
                rval = DETECTION_OPTION_MATCH;
#ifdef DEBUG_MSGS
            else
            {
                DebugMessage(DEBUG_PLUGIN, "CheckTtlEq: Not less than %d\n",
                    ttlCheckData->ttl);
            }
#endif
            break;
        case TTL_CHECK_GT_EQ:
            if (ttlCheckData->ttl <= GET_IPH_TTL(p))
                rval = DETECTION_OPTION_MATCH;
#ifdef DEBUG_MSGS
            else
            {
                DebugMessage(DEBUG_PLUGIN, "CheckTtlEq: Not greater than or equal to %d\n",
                    ttlCheckData->ttl);
            }
#endif
            break;
        case TTL_CHECK_LT_EQ:
            if (ttlCheckData->ttl >= GET_IPH_TTL(p))
                rval = DETECTION_OPTION_MATCH;
#ifdef DEBUG_MSGS
            else
            {
                DebugMessage(DEBUG_PLUGIN, "CheckTtlEq: Not less than or equal to %d\n",
                    ttlCheckData->ttl);
            }
#endif
            break;

         case TTL_CHECK_RG:
            if ((ttlCheckData->ttl <= GET_IPH_TTL(p)) &&
                (ttlCheckData->h_ttl >= GET_IPH_TTL(p)))
                rval = DETECTION_OPTION_MATCH;
#ifdef DEBUG_MSGS
            else
            {
                DebugMessage(DEBUG_PLUGIN, "CheckTtlLT: Not Within the range %d - %d (%d)\n",
                     ttlCheckData->ttl,
                     ttlCheckData->h_ttl,
                     GET_IPH_TTL(p));
            }
#endif
            break;
        default:
            break;
    }

    /* if the test isn't successful, return 0 */
    PREPROC_PROFILE_END(ttlCheckPerfStats);
    return rval;
}
