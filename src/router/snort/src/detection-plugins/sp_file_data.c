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

/* sp_file_data
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
#include "sp_file_data.h"
#ifdef PERF_PROFILING
PreprocStats fileDataPerfStats;
extern PreprocStats ruleOTNEvalPerfStats;
#endif

#include "detection_options.h"
#include "detection_util.h"

extern char *file_name;  /* this is the file name from rules.c, generally used
                            for error messages */

extern int file_line;    /* this is the file line number from rules.c that is
                            used to indicate file lines for error messages */

static void FileDataInit(struct _SnortConfig *, char *, OptTreeNode *, int);
void FileDataParse(char *, FileData *, OptTreeNode *);
int  FileDataEval(void *option_data, Packet *p);

uint32_t FileDataHash(void *d)
{
    uint32_t a,b,c;

    FileData *data = (FileData *)d;

    a = data->mime_decode_flag;
    b = RULE_OPTION_TYPE_FILE_DATA;
    c = 0;

    final(a,b,c);

    return c;
}

int FileDataCompare(void *l, void *r)
{
    FileData *left = (FileData *)l;
    FileData *right = (FileData *)r;
    if (!left || !right)
        return DETECTION_OPTION_NOT_EQUAL;
    if( left->mime_decode_flag == right->mime_decode_flag )
        return DETECTION_OPTION_EQUAL;

    return DETECTION_OPTION_NOT_EQUAL;
}



/****************************************************************************
 *
 * Function: SetupFileData()
 *
 * Purpose: Load 'er up
 *
 * Arguments: None.
 *
 * Returns: void function
 *
 ****************************************************************************/
void SetupFileData(void)
{
    /* map the keyword to an initialization/processing function */
    RegisterRuleOption("file_data", FileDataInit, NULL, OPT_TYPE_DETECTION, NULL);
#ifdef PERF_PROFILING
    RegisterPreprocessorProfile("file_data", &fileDataPerfStats, 3, &ruleOTNEvalPerfStats, NULL);
#endif

    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,"Plugin: file_data Setup\n"););
}


/****************************************************************************
 *
 * Function: FileDataInit(struct _SnortConfig *, char *, OptTreeNode *, int protocol)
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
static void FileDataInit(struct _SnortConfig *sc, char *data, OptTreeNode *otn, int protocol)
{
    FileData *idx;
    OptFpList *fpl;
    void *idx_dup;

    idx = (FileData *) SnortAlloc(sizeof(FileData));

    if(idx == NULL)
    {
        FatalError("%s(%d): Unable to allocate file_data node\n",
                    file_name, file_line);
    }



    otn->ds_list[PLUGIN_FILE_DATA] = (void *)1;

    FileDataParse(data, idx, otn);

    if (add_detection_option(sc, RULE_OPTION_TYPE_FILE_DATA, (void *)idx, &idx_dup) == DETECTION_OPTION_EQUAL)
    {
        free(idx);
        idx = idx_dup;
    }

    fpl = AddOptFuncToList(FileDataEval, otn);
    fpl->type = RULE_OPTION_TYPE_FILE_DATA;
    fpl->context = (void *)idx;

    return;
}



/****************************************************************************
 *
 * Function: FileDataParse(char *, OptTreeNode *)
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
void FileDataParse(char *data, FileData *idx, OptTreeNode *otn)
{

    if (IsEmptyStr(data))
    {
        idx->mime_decode_flag = 0;
    }
    else if(!strcasecmp("mime",data))
    {
        ParseWarning("The argument 'mime' to 'file_data' rule option is deprecated.\n");
    }
    else
    {
        FatalError("%s(%d) file_data: Invalid token %s\n",
                    file_name, file_line, data);
    }

    return;

}

/****************************************************************************
 *
 * Function: FileDataEval(char *, OptTreeNode *, OptFpList *)
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
int FileDataEval(void *option_data, Packet *p)
{
    int rval = DETECTION_OPTION_NO_MATCH;
    const uint8_t *data;
    uint16_t len;
    FileData *idx;
    PROFILE_VARS;

    PREPROC_PROFILE_START(fileDataPerfStats);
    idx = (FileData *)option_data;

    data = file_data_ptr.data;
    len = file_data_ptr.len;

    if ((p->dsize == 0) || !idx)
    {
        PREPROC_PROFILE_END(fileDataPerfStats);
        return rval;
    }

    if ((data == NULL)|| (len == 0))
    {
        data = p->data;
        len = p->dsize;
    }

    if(idx->mime_decode_flag)
        mime_present = 1;
    else
        mime_present = 0;

    SetDoePtr(data,  DOE_BUF_STD);
    SetAltDetect(data, len);
    rval = DETECTION_OPTION_MATCH;

    PREPROC_PROFILE_END(fileDataPerfStats);
    return rval;
}
