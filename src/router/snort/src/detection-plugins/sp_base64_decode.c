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

/* sp_base64_decode
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
#include "sp_base64_decode.h"
#include "sfutil/sf_base64decode.h"
#ifdef PERF_PROFILING
PreprocStats base64DecodePerfStats;
extern PreprocStats ruleOTNEvalPerfStats;
#endif

#include "detection_options.h"
#include "detection_util.h"


extern char *file_name;  /* this is the file name from rules.c, generally used
                            for error messages */

extern int file_line;    /* this is the file line number from rules.c that is
                            used to indicate file lines for error messages */

void Base64DecodeInit(struct _SnortConfig *, char *, OptTreeNode *, int);
void Base64DecodeParse(char *, Base64DecodeData *, OptTreeNode *);
int  Base64DecodeEval(void *option_data, Packet *p);

uint32_t Base64DecodeHash(void *d)
{
    uint32_t a,b,c;
    Base64DecodeData *data = (Base64DecodeData *)d;

    a = data->bytes_to_decode;
    b = data->offset;
    c = data->flags;

    mix(a,b,c);

    a += RULE_OPTION_TYPE_BASE64_DECODE;

    final(a,b,c);

    return c;
}

int Base64DecodeCompare(void *l, void *r)
{
    Base64DecodeData *left = (Base64DecodeData *)l;
    Base64DecodeData *right = (Base64DecodeData *)r;

    if (!left || !right)
        return DETECTION_OPTION_NOT_EQUAL;

    if ((left->bytes_to_decode == right->bytes_to_decode) &&
            ( left->offset == right->offset) &&
            ( left->flags == right->flags))
    {
        return DETECTION_OPTION_EQUAL;
    }

    return DETECTION_OPTION_NOT_EQUAL;
}


/****************************************************************************
 *
 * Function: SetupBase64Decode()
 *
 * Purpose: Load 'er up
 *
 * Arguments: None.
 *
 * Returns: void function
 *
 ****************************************************************************/
void SetupBase64Decode(void)
{
    /* map the keyword to an initialization/processing function */
    RegisterRuleOption("base64_decode", Base64DecodeInit, NULL, OPT_TYPE_DETECTION, NULL);
#ifdef PERF_PROFILING
    RegisterPreprocessorProfile("base64_decode", &base64DecodePerfStats, 3, &ruleOTNEvalPerfStats, NULL);
#endif

    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,"Plugin: base64_decode Setup\n"););
}


/****************************************************************************
 *
 * Function: Base64DecodeInit(struct _SnortConfig *, char *, OptTreeNode *, int protocol)
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
void Base64DecodeInit(struct _SnortConfig *sc, char *data, OptTreeNode *otn, int protocol)
{
    Base64DecodeData *idx;
    OptFpList *fpl;
    void *idx_dup;

    if(otn->ds_list[PLUGIN_BASE64_DECODE])
    {
        FatalError("%s(%d): Multiple base64_decode options in rule\n", file_name,
                file_line);
    }


    idx = (Base64DecodeData *) SnortAlloc(sizeof(Base64DecodeData));

    if(idx == NULL)
    {
        FatalError("%s(%d): Unable to allocate Base64Decode data node\n",
                        file_name, file_line);
    }

    otn->ds_list[PLUGIN_BASE64_DECODE] = idx;

    Base64DecodeParse(data, idx, otn);

    if (add_detection_option(sc, RULE_OPTION_TYPE_BASE64_DECODE, (void *)idx, &idx_dup) == DETECTION_OPTION_EQUAL)
    {
        free(idx);
        idx = otn->ds_list[PLUGIN_BASE64_DECODE] = idx_dup;
     }

    fpl = AddOptFuncToList(Base64DecodeEval, otn);
    fpl->type = RULE_OPTION_TYPE_BASE64_DECODE;

    fpl->context = (void *) idx;

    if (idx->flags & BASE64DECODE_RELATIVE_FLAG)
        fpl->isRelative = 1;
}

/****************************************************************************
 *
 * Function: Base64DecodeParse(char *, Base64DecodeData *, OptTreeNode *)
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
void Base64DecodeParse(char *data, Base64DecodeData *idx, OptTreeNode *otn)
{
    char **toks;
    char **toks1;
    int num_toks;
    int num_toks1;
    char *token;
    int i=0;
    char *endptr;
    int value = 0;


    /*no arguments*/
    if (IsEmptyStr(data))
    {
        idx->offset = 0;
        idx->bytes_to_decode = 0;
        idx->flags = 0;
        return;
    }

    toks = mSplit(data, ",", 0, &num_toks, 0);

    if (num_toks > 3 )
    {
         FatalError("%s (%d): Bad arguments to base64_decode.\n",
                 file_name, file_line);

    }

    while (i < num_toks )
    {
        token = toks[i];

        if( strcmp(token , "relative") == 0 )
        {
            idx->flags |= BASE64DECODE_RELATIVE_FLAG;
            i++;
            continue;
        }

        toks1 = mSplit(token, " \t", 0, &num_toks1, 0);

        if ( num_toks1 != 2 )
        {
            FatalError("%s (%d): Bad arguments to base64_decode.\n",
                     file_name, file_line);
        }

        if( strcmp(toks1[0], "offset") == 0 )
        {
            value = SnortStrtol(toks1[1], &endptr, 10);
            if(*endptr || value < 0)
            {
                FatalError("%s (%d): Bad arguments to base64_decode.\n",
                      file_name, file_line);
            }
            idx->offset = value;
        }
        else if( strcmp(toks1[0], "bytes") == 0 )
        {
            value = SnortStrtol(toks1[1], &endptr, 10);
            if(*endptr || (value < 0) )
            {
                FatalError("%s (%d): Bad arguments to base64_decode.\n",
                       file_name, file_line);
            }

            if(!value)
            {
                FatalError("%s (%d): \"bytes\" option to base64_decode cannot be"
                        " zero.\n", file_name, file_line);
            }
            idx->bytes_to_decode = value;
        }
        else
        {
            FatalError("%s (%d): Bad arguments to base64_decode.\n",
                     file_name, file_line);
        }

        mSplitFree(&toks1,num_toks1);
        i++;
    }

    mSplitFree(&toks,num_toks);

    if (otn && otn->ds_list[PLUGIN_PATTERN_MATCH_URI])
    {
        PatternMatchData *pmd = (PatternMatchData *)(otn->ds_list[PLUGIN_PATTERN_MATCH_URI]);
        idx->buffer_type = pmd->http_buffer;
    }
    else
    {
        idx->buffer_type = HTTP_BUFFER_NONE;
    }
    return;

}


/****************************************************************************
 *
 * Function: Base64DecodeEval(char *, OptTreeNode *, OptFpList *)
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
int Base64DecodeEval(void *option_data, Packet *p)
{
    int rval = DETECTION_OPTION_NO_MATCH;
    const uint8_t *start_ptr = NULL;
    uint8_t base64_buf[DECODE_BLEN];
    uint32_t base64_size =0;
    Base64DecodeData *idx;
    uint32_t buff_size;
    PROFILE_VARS;

    PREPROC_PROFILE_START(base64DecodePerfStats);

    base64_decode_size = 0;

    if ((!p->dsize) || (!p->data))
    {
        PREPROC_PROFILE_END(base64DecodePerfStats);
        return rval;
    }

    idx = (Base64DecodeData *)option_data;

    if(!idx)
    {
        PREPROC_PROFILE_END(base64DecodePerfStats);
        return rval;
    }

    if ((idx->flags & BASE64DECODE_RELATIVE_FLAG) && doe_ptr)
    {
        int dsize;
        const uint8_t *doe_start;
        if (Is_DetectFlag(FLAG_ALT_DETECT))
        {
            dsize = DetectBuffer.len;
            doe_start = DetectBuffer.data;
        }
        else if(Is_DetectFlag(FLAG_ALT_DECODE))
        {
            dsize = DecodeBuffer.len;
            doe_start = (uint8_t *)DecodeBuffer.data;
        }
        else if (doe_buf_flags & DOE_BUF_URI)
        {
            const HttpBuffer* hb = GetHttpBuffer(idx->buffer_type);
            if (!hb)
            {
                PREPROC_PROFILE_END(base64DecodePerfStats);
                return rval;
            }

            doe_start = hb->buf;
            dsize = hb->length;

        }
        else
        {
            if(IsLimitedDetect(p))
                dsize = p->alt_dsize;
            else
                dsize = p->dsize;
            doe_start = (uint8_t *) p->data;
        }

        start_ptr = doe_ptr + idx->offset;

        if(start_ptr >= (doe_start + dsize) || (doe_ptr < doe_start))
        {
            PREPROC_PROFILE_END(base64DecodePerfStats);
            return rval;
        }

        buff_size = doe_start + dsize - start_ptr;
    }
    else
    {
        start_ptr = p->data + idx->offset;

        if(start_ptr >= (p->data + p->dsize) )
        {
            PREPROC_PROFILE_END(base64DecodePerfStats);
            return rval;
        }

        buff_size = p->data + p->dsize - start_ptr;
    }

    if(sf_unfold_header(start_ptr, buff_size, base64_buf, sizeof(base64_buf), &base64_size, 0, 0) != 0)
    {
        PREPROC_PROFILE_END(base64DecodePerfStats);
        return rval;
    }


    if (idx->bytes_to_decode && (base64_size > idx->bytes_to_decode))
    {
        base64_size = idx->bytes_to_decode;
    }

    if(sf_base64decode(base64_buf, base64_size, (uint8_t *)base64_decode_buf, sizeof(base64_decode_buf), &base64_decode_size) != 0)
    {
        PREPROC_PROFILE_END(base64DecodePerfStats);
        return rval;
    }

    PREPROC_PROFILE_END(base64DecodePerfStats);

    return DETECTION_OPTION_MATCH;
}
