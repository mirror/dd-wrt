/* $Id$ */
/*
 ** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 ** Copyright (C) 2002-2013 Sourcefire, Inc.
 ** Author: Daniel Roelker
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

/**
**  @file        sp_asn1.c
**
**  @author      Daniel Roelker <droelker@sourcefire.com>
**
**  @brief       Decode and detect ASN.1 types, lengths, and data.
**
**  This detection plugin adds ASN.1 detection functions on a per rule
**  basis.  ASN.1 detection plugins can be added by editing this file and
**  providing an interface in the configuration code.
**
**  Detection Plugin Interface:
**
**  asn1: [detection function],[arguments],[offset type],[size]
**
**  Detection Functions:
**
**  bitstring_overflow: no arguments
**  double_overflow:    no arguments
**  oversize_length:    max size (if no max size, then just return value)
**
**  alert udp any any -> any 161 (msg:"foo"; \
**      asn1: oversize_length 10000, absolute_offset 0;)
**
**  alert tcp any any -> any 162 (msg:"foo2"; \
**      asn1: bitstring_overflow, oversize_length 500, relative_offset 7;)
**
**
**  Note that further general information about ASN.1 can be found in
**  the file doc/README.asn1.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

#include "sf_types.h"
#include "snort_bounds.h"
#include "rules.h"
#include "treenodes.h"
#include "decode.h"
#include "plugbase.h"
#include "parser.h"
#include "snort_debug.h"
#include "util.h"
#include "plugin_enum.h"
#include "asn1.h"
#include "sp_asn1.h"
#include "sp_asn1_detect.h"
#include "sfhashfcn.h"
#include "detection_util.h"

#define BITSTRING_OPT  "bitstring_overflow"
#define DOUBLE_OPT     "double_overflow"
#define LENGTH_OPT     "oversize_length"
#define DBL_FREE_OPT   "double_free"

#define ABS_OFFSET_OPT "absolute_offset"
#define REL_OFFSET_OPT "relative_offset"
#define PRINT_OPT      "print"

#define DELIMITERS " ,\t\n"

#include "snort.h"
#include "profiler.h"
#ifdef PERF_PROFILING
PreprocStats asn1PerfStats;
extern PreprocStats ruleOTNEvalPerfStats;
#endif

#include "sfhashfcn.h"
#include "detection_options.h"

uint32_t Asn1Hash(void *d)
{
    uint32_t a,b,c;
    ASN1_CTXT *data = (ASN1_CTXT *)d;

    a = data->bs_overflow;
    b = data->double_overflow;
    c = data->print;

    mix(a,b,c);

    a += data->length;
    b += data->max_length;
    c += data->offset;

    mix(a,b,c);

    a += data->offset_type;
    b += RULE_OPTION_TYPE_ASN1;

    final(a,b,c);

    return c;
}

int Asn1Compare(void *l, void *r)
{
    ASN1_CTXT *left = (ASN1_CTXT *)l;
    ASN1_CTXT *right = (ASN1_CTXT *)r;

    if (!left || !right)
        return DETECTION_OPTION_NOT_EQUAL;

    if ((left->bs_overflow == right->bs_overflow) &&
        (left->double_overflow == right->double_overflow) &&
        (left->print == right->print) &&
        (left->length == right->length) &&
        (left->max_length == right->max_length) &&
        (left->offset == right->offset) &&
        (left->offset_type == right->offset_type))
    {
        return DETECTION_OPTION_EQUAL;
    }

    return DETECTION_OPTION_NOT_EQUAL;
}

/*
**  NAME
**    Asn1RuleParse::
*/
/**
**  Parse the detection option arguments.
**    - bitstring_overflow
**    - double_overflow
**    - oversize_length
**    - print
**    - abs_offset
**    - rel_offset
**
**  @return void
*/
static void Asn1RuleParse(char *data, OptTreeNode *otn, ASN1_CTXT *asn1)
{
    char *pcTok;
    char *endTok;

    if(!data)
    {
        FatalError("%s(%d) => No options to 'asn1' detection plugin.\n",
                   file_name, file_line);
    }

    pcTok = strtok(data, DELIMITERS);
    if(!pcTok)
    {
        FatalError("%s(%d) => No options to 'asn1' detection plugin.\n",
                   file_name, file_line);
    }

    while(pcTok)
    {
        if(!strcasecmp(pcTok, BITSTRING_OPT))
        {
            asn1->bs_overflow = 1;
        }
        else if(!strcasecmp(pcTok, DOUBLE_OPT))
        {
            asn1->double_overflow = 1;
        }
        else if(!strcasecmp(pcTok, PRINT_OPT))
        {
            asn1->print = 1;
        }
        else if(!strcasecmp(pcTok, LENGTH_OPT))
        {
            long int max_length;
            char *pcEnd;

            pcTok = strtok(NULL, DELIMITERS);
            if(!pcTok)
            {
                FatalError("%s(%d) => No option to '%s' in 'asn1' detection "
                           "plugin\n", file_name, file_line, LENGTH_OPT);
            }

            max_length = SnortStrtolRange(pcTok, &pcEnd, 10, 0, INT32_MAX);

            if ((pcEnd == pcTok) || (*pcEnd) || (errno == ERANGE))
            {
                FatalError("%s(%d) => Negative size, underflow or overflow "
                           "(of long int) to '%s' in 'asn1' detection plugin. "
                           "Must be positive or zero.\n",
                           file_name, file_line, LENGTH_OPT);
            }

            asn1->length = 1;
            asn1->max_length = (unsigned int)max_length;
        }
        else if(!strcasecmp(pcTok, ABS_OFFSET_OPT))
        {
            pcTok = strtok(NULL, DELIMITERS);
            if(!pcTok)
            {
                FatalError("%s(%d) => No option to '%s' in 'asn1' detection "
                           "plugin\n", file_name, file_line, ABS_OFFSET_OPT);
            }

            asn1->offset_type = ABS_OFFSET;
            asn1->offset = SnortStrtol(pcTok, &endTok, 10);
            if (endTok == pcTok)
            {
                FatalError("%s(%d) => Invalid parameter to '%s' in 'asn1' "
                           "detection plugin\n",
                           file_name, file_line, ABS_OFFSET_OPT);
            }

        }
        else if(!strcasecmp(pcTok, REL_OFFSET_OPT))
        {
            pcTok = strtok(NULL, DELIMITERS);
            if(!pcTok)
            {
                FatalError("%s(%d) => No option to '%s' in 'asn1' detection "
                           "plugin\n", file_name, file_line, REL_OFFSET_OPT);
            }

            asn1->offset_type = REL_OFFSET;
            asn1->offset = SnortStrtol(pcTok, &endTok, 10);
            if (endTok == pcTok)
            {
                FatalError("%s(%d) => Invalid parameter to '%s' in 'asn1' "
                           "detection plugin\n",
                           file_name, file_line, pcTok);
            }
        }
        else
        {
            FatalError("%s(%d) => Unknown ('%s') asn1 detection option.\n",
                       file_name, file_line, pcTok);
        }

        pcTok = strtok(NULL, DELIMITERS);
    }

    return;
}

/*
**  NAME
**    Asn1Detect::
*/
/**
**  The main snort detection function.  We grab the context ptr from the
**  otn and go forth.  We check all the offsets to make sure we're in
**  bounds, etc.
**
**  @return integer
**
**  @retval 0 failed
**  @retval 1 detected
*/
int Asn1Detect(void *context, Packet *p)
{
    ASN1_CTXT *ctxt;
    PROFILE_VARS;

    /*
    **  Failed if there is no data to decode.
    */
    if(!p->data)
        return DETECTION_OPTION_NO_MATCH;

    PREPROC_PROFILE_START(asn1PerfStats);

    ctxt = (ASN1_CTXT *)context;

    if (Asn1DoDetect(p->data, p->dsize, ctxt, doe_ptr))
    {
        PREPROC_PROFILE_END(asn1PerfStats);
        return DETECTION_OPTION_MATCH;
    }

    PREPROC_PROFILE_END(asn1PerfStats);
    return DETECTION_OPTION_NO_MATCH;
}

static void Asn1Init(struct _SnortConfig *sc, char *data, OptTreeNode *otn, int protocol)
{
    ASN1_CTXT *asn1;
    void *ds_ptr_dup;
    OptFpList *ofl;

    /*
     * allocate the data structure and attach
     * it to the rule's data struct list
     */
    asn1 = (ASN1_CTXT *)SnortAlloc(sizeof(ASN1_CTXT));

    Asn1RuleParse(data, otn, asn1);

    if (add_detection_option(sc, RULE_OPTION_TYPE_ASN1, (void *)asn1, &ds_ptr_dup) == DETECTION_OPTION_EQUAL)
    {
        free(asn1);
        asn1 = ds_ptr_dup;
    }

    ofl = AddOptFuncToList(Asn1Detect, otn);

    ofl->context = (void *)asn1;
    ofl->type = RULE_OPTION_TYPE_ASN1;

    if (asn1->offset_type == REL_OFFSET)
        ofl->isRelative = 1;

}

void SetupAsn1(void)
{
    /* map the keyword to an initialization/processing function */
    RegisterRuleOption("asn1", Asn1Init, NULL, OPT_TYPE_DETECTION, NULL);

#ifdef PERF_PROFILING
    RegisterPreprocessorProfile("asn1", &asn1PerfStats, 3, &ruleOTNEvalPerfStats, NULL);
#endif

    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,"Plugin: ASN1 Setup\n"););
}

