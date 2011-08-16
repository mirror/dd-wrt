/* $Id$ */
/*
 ** Copyright (C) 1998-2011 Sourcefire, Inc.
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

/* sp_isdataat
 * 
 * Purpose:
 *    Test a specific byte to see if there is data.  (Basicly, rule keyword
 *    into inBounds)
 *
 * Arguments:
 *    <int>         byte location to check if there is data
 *    ["relative"]  look for byte location relative to the end of the last
 *                  pattern match
 *    ["rawbytes"]  force use of the non-normalized buffer.
 *   
 * Sample:
 *   alert tcp any any -> any 110 (msg:"POP3 user overflow"; \
 *      content:"USER"; isdataat:30,relative; content:!"|0a|"; within:30;)
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

#include "bounds.h"
#include "rules.h"
#include "treenodes.h"
#include "decode.h"
#include "plugbase.h"
#include "parser.h"
#include "debug.h"
#include "util.h"
#include "mstring.h"

#include "snort.h"
#include "profiler.h"
#include "sp_isdataat.h"
#include "sp_byte_extract.h"
#ifdef PERF_PROFILING
PreprocStats isDataAtPerfStats;
extern PreprocStats ruleOTNEvalPerfStats;
#endif

#include "sfhashfcn.h"
#include "detection_options.h"
#include "detection_util.h"

extern char *file_name;  /* this is the file name from rules.c, generally used
                            for error messages */

extern int file_line;    /* this is the file line number from rules.c that is
                            used to indicate file lines for error messages */

void IsDataAtInit(char *, OptTreeNode *, int);
void IsDataAtParse(char *, IsDataAtData *, OptTreeNode *);
int  IsDataAt(void *option_data, Packet *p);

uint32_t IsDataAtHash(void *d)
{
    uint32_t a,b,c;
    IsDataAtData *data = (IsDataAtData *)d;

    a = data->offset;
    b = data->flags;
    c = RULE_OPTION_TYPE_IS_DATA_AT;

    mix(a,b,c);

    a += data->offset_var;

    final(a,b,c);

    return c;
}

int IsDataAtCompare(void *l, void *r)
{
    IsDataAtData *left = (IsDataAtData *)l;
    IsDataAtData *right = (IsDataAtData *)r;

    if (!left || !right)
        return DETECTION_OPTION_NOT_EQUAL;
                                
    if (( left->offset == right->offset) &&
        ( left->flags == right->flags) &&
        ( left->offset_var == right->offset_var) )
    {
        return DETECTION_OPTION_EQUAL;
    }

    return DETECTION_OPTION_NOT_EQUAL;
}

/****************************************************************************
 * 
 * Function: SetupIsDataAt()
 *
 * Purpose: Load 'er up
 *
 * Arguments: None.
 *
 * Returns: void function
 *
 ****************************************************************************/
void SetupIsDataAt(void)
{
    /* map the keyword to an initialization/processing function */
    RegisterRuleOption("isdataat", IsDataAtInit, NULL, OPT_TYPE_DETECTION, NULL);
#ifdef PERF_PROFILING
    RegisterPreprocessorProfile("isdataat", &isDataAtPerfStats, 3, &ruleOTNEvalPerfStats);
#endif

    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,"Plugin: IsDataAt Setup\n"););
}


/****************************************************************************
 * 
 * Function: IsDataAt(char *, OptTreeNode *, int protocol)
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
void IsDataAtInit(char *data, OptTreeNode *otn, int protocol)
{
    IsDataAtData *idx;
    OptFpList *fpl;
    void *idx_dup;

    /* allocate the data structure and attach it to the
       rule's data struct list */
    idx = (IsDataAtData *) SnortAlloc(sizeof(IsDataAtData));

    if(idx == NULL)
    {
        FatalError("%s(%d): Unable to allocate IsDataAt data node\n", 
                file_name, file_line);
    }

    /* this is where the keyword arguments are processed and placed into the 
       rule option's data structure */
    IsDataAtParse(data, idx, otn);

    if (add_detection_option(RULE_OPTION_TYPE_IS_DATA_AT, (void *)idx, &idx_dup) == DETECTION_OPTION_EQUAL)
    {
        free(idx);
        idx = idx_dup;
     }

    fpl = AddOptFuncToList(IsDataAt, otn);
    fpl->type = RULE_OPTION_TYPE_IS_DATA_AT;
    
    /* attach it to the context node so that we can call each instance
     * individually
     */
    fpl->context = (void *) idx;

    if (idx->flags & ISDATAAT_RELATIVE_FLAG)
        fpl->isRelative = 1;
}



/****************************************************************************
 * 
 * Function: IsDataAt(char *, IsDataAtData *, OptTreeNode *)
 *
 * Purpose: This is the function that is used to process the option keyword's
 *          arguments and attach them to the rule's data structures.
 *
 * Arguments: data => argument data
 *            idx => pointer to the processed argument storage
 *            otn => pointer to the current rule's OTN
 *
 * Returns: void function
 *
 ****************************************************************************/
void IsDataAtParse(char *data, IsDataAtData *idx, OptTreeNode *otn)
{
    char **toks;
    int num_toks;
    int i;
    char *cptr;
    char *endp;
    char *offset;

    toks = mSplit(data, ",", 3, &num_toks, 0);

    if(num_toks > 3) 
        FatalError("%s (%d): Bad arguments to IsDataAt: %s\n", file_name,
                file_line, data);
    offset = toks[0]; 
    if(*offset == '!')
    {
        idx->flags |= ISDATAAT_NOT_FLAG;
        offset++;
        while(isspace((int)*offset)) {offset++;}
    }

    /* set how many bytes to process from the packet */
    if (isdigit(offset[0]) || offset[0] == '-')
    {
        idx->offset = strtol(offset, &endp, 10);
        idx->offset_var = -1;

        if(offset == endp)
        {
            FatalError("%s(%d): Unable to parse as byte value %s\n",
                       file_name, file_line, toks[0]);
        }

        if(idx->offset > 65535)
        {
            FatalError("%s(%d): IsDataAt offset greater than max IPV4 packet size",
                    file_name, file_line);
        }
    }
    else
    {
        idx->offset_var = GetVarByName(offset);
        if (idx->offset_var == BYTE_EXTRACT_NO_VAR)
        {
            FatalError("%s (%d): %s\n", file_name, file_line, BYTE_EXTRACT_INVALID_ERR_STR);
        }
    }

    for (i=1; i< num_toks; i++)
    {
        cptr = toks[i];

        while(isspace((int)*cptr)) {cptr++;}

        if(!strcasecmp(cptr, "relative"))
        {
            /* the offset is relative to the last pattern match */
            idx->flags |= ISDATAAT_RELATIVE_FLAG;
        }
        else if(!strcasecmp(cptr, "rawbytes"))
        {
            /* the offset is to be applied to the non-normalized buffer */
            idx->flags |= ISDATAAT_RAWBYTES_FLAG;
        }
        else
        {
            FatalError("%s(%d): unknown modifier \"%s\"\n",
                    file_name, file_line, toks[1]);
        }
    }

    mSplitFree(&toks,num_toks);
}


/****************************************************************************
 * 
 * Function: IsDataAt(char *, OptTreeNode *, OptFpList *)
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
int IsDataAt(void *option_data, Packet *p)
{
    IsDataAtData *isdata = (IsDataAtData *)option_data;
    int rval = DETECTION_OPTION_NO_MATCH;
    int dsize;
    const uint8_t *base_ptr, *end_ptr, *start_ptr;
    PROFILE_VARS;

    PREPROC_PROFILE_START(isDataAtPerfStats);

    if (!isdata)
    {
        PREPROC_PROFILE_END(isDataAtPerfStats);
        return rval;
    }

    /* Get values from byte_extract variables, if present. */
    if (isdata->offset_var >= 0 && isdata->offset_var < NUM_BYTE_EXTRACT_VARS)
    {
        GetByteExtractValue(&(isdata->offset), isdata->offset_var);
    }

    if (isdata->flags & ISDATAAT_RAWBYTES_FLAG)
    {
        /* Rawbytes specified, force use of that buffer */
        dsize = p->dsize;
        start_ptr = p->data;
        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, 
                    "Using RAWBYTES buffer!\n"););
    }
    else if( IsMimeDecodeBuf(doe_ptr) )
    {
        dsize = mime_decode_size;
        start_ptr = file_data_ptr;
        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                    "Using MIME Decode buffer!\n"););
    }
    else if( IsBase64DecodeBuf(doe_ptr) )
    {
        dsize = base64_decode_size;
        start_ptr = base64_decode_buf;
        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                                "Using Base64 Decode buffer!\n"););
    }
    else if(p->packet_flags & PKT_ALT_DECODE)
    {
        /* If normalized buffer available, use it... */
        dsize = DecodeBuffer.len;
        start_ptr = DecodeBuffer.data;
        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, 
                    "Using Alternative Decode buffer!\n"););
    }
    else
    {
        if(IsLimitedDetect(p))
            dsize = p->alt_dsize;
        else
            dsize = p->dsize;
        start_ptr = p->data;
    }

    base_ptr = start_ptr;
    end_ptr = start_ptr + dsize;
    
    if((isdata->flags & ISDATAAT_RELATIVE_FLAG) && doe_ptr)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                                "Checking relative offset!\n"););

        if(!inBounds(start_ptr, end_ptr, doe_ptr))
        {
            DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                                    "[*] isdataat bounds check failed..\n"););
            PREPROC_PROFILE_END(isDataAtPerfStats);
            return rval;
        }

        base_ptr = doe_ptr + isdata->offset;
    }
    else
    {
        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                                "checking absolute offset %d\n", isdata->offset););
        base_ptr = start_ptr + isdata->offset;
    }

    if(inBounds(start_ptr, end_ptr, base_ptr))
    {
        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                    "[*] IsDataAt succeeded!  there is data...\n"););
        rval = DETECTION_OPTION_MATCH;
    }

    if (isdata->flags & ISDATAAT_NOT_FLAG)
    {
        rval = !rval;
    }


    /* otherwise dump */
    PREPROC_PROFILE_END(isDataAtPerfStats);
    return rval;
}
