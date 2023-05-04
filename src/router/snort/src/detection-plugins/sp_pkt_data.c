/*
 ** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 ** Copyright (C) 1998-2013 Sourcefire, Inc.
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

/* sp_pkt_data
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <stdlib.h>
#include <ctype.h>
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#include <errno.h>

#include "sf_types.h"
#include "snort_bounds.h"
#include "rules.h"
#include "decode.h"
#include "plugbase.h"
#include "parser.h"
#include "snort_debug.h"
#include "util.h"
#include "mstring.h"

#include "snort.h"
#include "profiler.h"
#include "sp_pkt_data.h"
#ifdef PERF_PROFILING
PreprocStats pktDataPerfStats;
extern PreprocStats ruleOTNEvalPerfStats;
#endif

#include "detection_options.h"
#include "detection_util.h"

extern char *file_name;  /* this is the file name from rules.c, generally used
                            for error messages */

extern int file_line;    /* this is the file line number from rules.c that is
                            used to indicate file lines for error messages */

static void PktDataInit(struct _SnortConfig *, char *, OptTreeNode *, int);
void PktDataParse(char *, OptTreeNode *);
int  PktDataEval(void *option_data, Packet *p);

/****************************************************************************
 *
 * Function: SetupPktData()
 *
 * Purpose: Load 'er up
 *
 * Arguments: None.
 *
 * Returns: void function
 *
 ****************************************************************************/
void SetupPktData(void)
{
    /* map the keyword to an initialization/processing function */
    RegisterRuleOption("pkt_data", PktDataInit, NULL, OPT_TYPE_DETECTION, NULL);
#ifdef PERF_PROFILING
    RegisterPreprocessorProfile("pkt_data", &pktDataPerfStats, 3, &ruleOTNEvalPerfStats, NULL);
#endif

    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,"Plugin: pkt_data Setup\n"););
}


/****************************************************************************
 *
 * Function: PktDataInit(struct _SnortConfig *, char *, OptTreeNode *, int protocol)
 *
 * Purpose: Generic rule configuration function.  Handles parsing the rule
 *          information and attaching the associated detection function to
 *          the OTN.
 *
 * Arguments: data => rule arguments/data
 *            otn => pointer to the current rule option list node
 *            protocol => protocol the rule is on (we don't care in this case)
 *
 * Returns: void function
 *
 ****************************************************************************/
static void PktDataInit(struct _SnortConfig *sc, char *data, OptTreeNode *otn, int protocol)
{
    OptFpList *fpl;

    PktDataParse(data, otn);

    fpl = AddOptFuncToList(PktDataEval, otn);
    fpl->type = RULE_OPTION_TYPE_PKT_DATA;

}



/****************************************************************************
 *
 * Function: PktDataParse(char *, OptTreeNode *)
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
void PktDataParse(char *data, OptTreeNode *otn)
{
    if (!IsEmptyStr(data))
    {
        FatalError("%s(%d): pkt_data takes no arguments\n",
                file_name, file_line);
    }

}


/****************************************************************************
 *
 * Function: PktDataEval(char *, OptTreeNode *, OptFpList *)
 *
 * Purpose: Use this function to perform the particular detection routine
 *          that this rule keyword is supposed to encompass.
 *
 * Arguments: p => pointer to the decoded packet
 *            otn => pointer to the current rule's OTN
 *            fp_list => pointer to the function pointer list
 *
 * Returns: If the detection test fails, this function *must* return a zero!
 *          On success, it calls the next function in the detection list
 *
 ****************************************************************************/
int PktDataEval(void *option_data, Packet *p)
{
    int rval = DETECTION_OPTION_MATCH;
    PROFILE_VARS;

    PREPROC_PROFILE_START(pktDataPerfStats);

    SetDoePtr(NULL, DOE_BUF_STD);
    DetectFlag_Disable(FLAG_ALT_DETECT);

    PREPROC_PROFILE_END(pktDataPerfStats);
    return rval;
}
