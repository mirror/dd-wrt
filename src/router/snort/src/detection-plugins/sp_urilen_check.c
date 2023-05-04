/* $Id */
/*
 ** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 ** Copyright (C) 2005-2013 Sourcefire, Inc.
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
 ** along with this program; if nto, write to the Free Software
 ** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 ** USA
 */

/*
 * sp_urilen_check.c: Detection plugin to expose URI length to user rules.
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
#include "mstring.h"

#include "sp_urilen_check.h"

#include "snort.h"
#include "profiler.h"
#ifdef PERF_PROFILING
PreprocStats urilenCheckPerfStats;
extern PreprocStats ruleOTNEvalPerfStats;
#endif

#include "sfhashfcn.h"
#include "detection_options.h"
#include "detection_util.h"

void UriLenCheckInit( struct _SnortConfig *, char*, OptTreeNode*, int );
void ParseUriLen( struct _SnortConfig *, char*, OptTreeNode* );
int CheckUriLen(void *option_data, Packet*p);

uint32_t UriLenCheckHash(void *d)
{
    uint32_t a,b,c;
    UriLenCheckData *data = (UriLenCheckData *)d;

    a = data->urilen;
    b = data->urilen2;
    c = data->oper;

    mix(a,b,c);

    a += data->uri_buf;
    b += RULE_OPTION_TYPE_URILEN;

    final(a,b,c);

    return c;
}

int UriLenCheckCompare(void *l, void *r)
{
    UriLenCheckData *left = (UriLenCheckData *)l;
    UriLenCheckData *right = (UriLenCheckData *)r;

    if (!left || !right)
        return DETECTION_OPTION_NOT_EQUAL;

    if ((left->urilen == right->urilen)
            && (left->urilen2 == right->urilen2)
            && (left->oper == right->oper)
            && (left->uri_buf == right->uri_buf))
    {
        return DETECTION_OPTION_EQUAL;
    }

    return DETECTION_OPTION_NOT_EQUAL;
}


/* Called from plugbase to register any detection plugin keywords.
 *
 * PARAMETERS:	None.
 *
 * RETURNS:	Nothing.
 */
void SetupUriLenCheck(void)
{
    RegisterRuleOption("urilen", UriLenCheckInit, NULL, OPT_TYPE_DETECTION, NULL);
#ifdef PERF_PROFILING
    RegisterPreprocessorProfile("urilen_check", &urilenCheckPerfStats, 3, &ruleOTNEvalPerfStats, NULL);
#endif
}

/* Parses the urilen rule arguments and attaches info to
 * the rule data structure for later use. Attaches detection
 * function to OTN function list.
 *
 * PARAMETERS:
 *
 * argp:	Rule arguments
 * otnp:  	Pointer to the current rule option list node
 * protocol:    Pointer specified for the rule currently being parsed
 *
 * RETURNS:	Nothing.
 */
void UriLenCheckInit( struct _SnortConfig *sc, char* argp, OptTreeNode* otnp, int protocol )
{
    /* Sanity check(s) */
    if ( !otnp )
        return;

    /* Check if there have been multiple urilen options specified
     * in the same rule.
     */
    if ( otnp->ds_list[PLUGIN_URILEN_CHECK] )
    {
        FatalError("%s(%d): Multiple urilen options in rule\n",
                file_name, file_line );
    }

    otnp->ds_list[PLUGIN_URILEN_CHECK] = SnortAlloc(sizeof(UriLenCheckData));

    ParseUriLen( sc, argp, otnp );
}

/* Parses the urilen rule arguments and attaches the resulting
 * parameters to the rule data structure. Based on arguments,
 * attaches the appropriate callback/processing function
 * to be used when the OTN is evaluated.
 *
 * PARAMETERS:
 *
 * argp:	Pointer to string containing the arguments to be
 *		parsed.
 * otnp:	Pointer to the current rule option list node.
 *
 * RETURNS:	Nothing.
 */
void ParseUriLen( struct _SnortConfig *sc, char* argp, OptTreeNode* otnp )
{
    OptFpList *fpl;
    UriLenCheckData* datap = (UriLenCheckData*)otnp->ds_list[PLUGIN_URILEN_CHECK];
    void *datap_dup;
    char* curp = NULL;
    char **toks;
    int num_toks;

    toks = mSplit(argp, ",", 2, &num_toks, '\\');
    if (!num_toks)
    {
        FatalError("%s(%d): 'urilen' requires arguments.\n",
                file_name, file_line);
    }

    curp = toks[0];

    /* Parse the string */
    if (isdigit((int)*curp) && strstr(curp, "<>"))
    {
        char **mtoks;
        int num_mtoks;
        char* endp = NULL;
        long int val;

        mtoks = mSplit(curp, "<>", 2, &num_mtoks, '\\');
        if (num_mtoks != 2)
        {
            FatalError("%s(%d): Invalid 'urilen' argument.\n",
                    file_name, file_line);
        }

        val = strtol(mtoks[0], &endp, 0);
        if ((val < 0) || *endp || (val > UINT16_MAX))
        {
            FatalError("%s(%d): Invalid 'urilen' argument.\n",
                    file_name, file_line);
        }

        datap->urilen = (uint16_t)val;

        val = strtol(mtoks[1], &endp, 0);
        if ((val < 0) || *endp || (val > UINT16_MAX))
        {
            FatalError("%s(%d): Invalid 'urilen' argument.\n",
                    file_name, file_line);
        }

        datap->urilen2 = (uint16_t)val;

        if (datap->urilen2 < datap->urilen)
        {
            uint16_t tmp = datap->urilen;
            datap->urilen = datap->urilen2;
            datap->urilen2 = tmp;
        }

        datap->oper = URILEN_CHECK_RG;

        mSplitFree(&mtoks, num_mtoks);
    }
    else
    {
        char* endp = NULL;
        long int val;

        if(*curp == '>')
        {
            curp++;
            datap->oper = URILEN_CHECK_GT;
        }
        else if(*curp == '<')
        {
            curp++;
            datap->oper = URILEN_CHECK_LT;
        }
        else
        {
            datap->oper = URILEN_CHECK_EQ;
        }

        while(isspace((int)*curp)) curp++;

        if (!*curp)
        {
            FatalError("%s(%d): Invalid 'urilen' argument.\n",
                    file_name, file_line);
        }

        val = strtol(curp, &endp, 0);
        if ((val < 0) || *endp || (val > UINT16_MAX))
        {
            FatalError("%s(%d): Invalid 'urilen' argument.\n",
                    file_name, file_line);
        }

        if ((datap->oper == URILEN_CHECK_LT) && (val == 0))
        {
            FatalError("%s(%d): Invalid 'urilen' argument.\n",
                    file_name, file_line);
        }

        datap->urilen = (uint16_t)val;
    }

    if (num_toks > 1)
    {
        if (!strcmp(toks[1], URI_LEN_BUF_NORM))
            datap->uri_buf = HTTP_BUFFER_URI;
        else if (!strcmp(toks[1], URI_LEN_BUF_RAW))
            datap->uri_buf = HTTP_BUFFER_RAW_URI;
        else
            FatalError("%s(%d): Invalid 'urilen' argument.\n",
                    file_name, file_line);
    }
    else
    {
        if (strchr(argp, ','))
        {
            FatalError("%s(%d): Invalid 'urilen' argument.\n",
                    file_name, file_line);
        }

        datap->uri_buf = HTTP_BUFFER_RAW_URI;
    }

    mSplitFree(&toks, num_toks);

    fpl = AddOptFuncToList(CheckUriLen, otnp);
    fpl->type = RULE_OPTION_TYPE_URILEN;

    if (add_detection_option(sc, RULE_OPTION_TYPE_URILEN, (void *)datap, &datap_dup) == DETECTION_OPTION_EQUAL)
    {
        otnp->ds_list[PLUGIN_URILEN_CHECK] = datap_dup;
        free(datap);
    }

    fpl->context = otnp->ds_list[PLUGIN_URILEN_CHECK];
}

int CheckUriLen(void *option_data, Packet *p)
{
    UriLenCheckData *udata = (UriLenCheckData *)option_data;
    int rval = DETECTION_OPTION_NO_MATCH;
    const HttpBuffer* hb = GetHttpBuffer(udata->uri_buf);
    PROFILE_VARS;

    PREPROC_PROFILE_START(urilenCheckPerfStats);

    if ( !hb )
    {
        PREPROC_PROFILE_END(urilenCheckPerfStats);
        return rval;
    }

    switch (udata->oper)
    {
        case URILEN_CHECK_EQ:
            if (udata->urilen == hb->length)
                rval = DETECTION_OPTION_MATCH;
            break;
        case URILEN_CHECK_GT:
            if (udata->urilen < hb->length)
                rval = DETECTION_OPTION_MATCH;
            break;
        case URILEN_CHECK_LT:
            if (udata->urilen > hb->length)
                rval = DETECTION_OPTION_MATCH;
            break;
        case URILEN_CHECK_RG:
            if ((udata->urilen <= hb->length) && (udata->urilen2 >= hb->length))
                rval = DETECTION_OPTION_MATCH;
            break;
        default:
            break;
    }

    /* if the test isn't successful, return 0 */
    PREPROC_PROFILE_END(urilenCheckPerfStats);
    return rval;
}
