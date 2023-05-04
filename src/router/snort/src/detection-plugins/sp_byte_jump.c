/* $Id$ */
/*
 ** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 ** Copyright (C) 2002-2013 Sourcefire, Inc.
 ** Author: Martin Roesch
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

/* sp_byte_jump
 *
 * Purpose:
 *      Grab some number of bytes, convert them to their numeric
 *      representation, jump the doe_ptr up that many bytes (for
 *      further pattern matching/byte_testing).
 *
 *
 * Arguments:
 *      Required:
 *      <bytes_to_grab>: number of bytes to pick up from the packet
 *      <offset>: number of bytes into the payload to grab the bytes
 *      Optional:
 *      ["relative"]: offset relative to last pattern match
 *      ["big"]: process data as big endian (default)
 *      ["little"]: process data as little endian
 *      ["string"]: converted bytes represented as a string needing conversion
 *      ["hex"]: converted string data is represented in hexidecimal
 *      ["dec"]: converted string data is represented in decimal
 *      ["oct"]: converted string data is represented in octal
 *      ["align"]: round the number of converted bytes up to the next
 *                 32-bit boundry
 *      ["post_offset"]: number of bytes to adjust after applying
 *
 *   sample rules:
 *   alert udp any any -> any 32770:34000 (content: "|00 01 86 B8|"; \
 *       content: "|00 00 00 01|"; distance: 4; within: 4; \
 *       byte_jump: 4, 12, relative, align; \
 *       byte_test: 4, >, 900, 20, relative; \
 *       msg: "statd format string buffer overflow";)
 *
 * Effect:
 *
 *      Reads in the indicated bytes, converts them to an numeric
 *      representation and then jumps the doe_ptr up
 *      that number of bytes.  Returns 1 if the jump is in range (within the
 *      packet) and 0 if it's not.
 *
 * Comments:
 *
 * Any comments?
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
#include "treenodes.h"
#include "decode.h"
#include "plugbase.h"
#include "parser.h"
#include "snort_debug.h"
#include "util.h"
#include "plugin_enum.h"
#include "mstring.h"
#include "byte_extract.h"
#include "sp_byte_jump.h"
#include "sp_byte_extract.h"
#include "sp_byte_math.h"
#include "sfhashfcn.h"

#include "snort.h"
#include "profiler.h"
#ifdef PERF_PROFILING
PreprocStats byteJumpPerfStats;
extern PreprocStats ruleOTNEvalPerfStats;
#endif

#include "sfhashfcn.h"
#include "detection_options.h"
#include "detection_util.h"

typedef struct _ByteJumpOverrideData
{
    char *keyword;
    char *option;
    union
    {
        RuleOptOverrideFunc fptr;
        void *void_fptr;
    } fptr;
    struct _ByteJumpOverrideData *next;
} ByteJumpOverrideData;

ByteJumpOverrideData *byteJumpOverrideFuncs = NULL;

static void ByteJumpOverride(char *keyword, char *option, RuleOptOverrideFunc roo_func);
static void ByteJumpOverrideFuncsFree(void);
static void ByteJumpInit(struct _SnortConfig *, char *, OptTreeNode *, int);
static ByteJumpOverrideData * ByteJumpParse(char *, ByteJumpData *, OptTreeNode *);
static void ByteJumpOverrideCleanup(int, void *);


uint32_t ByteJumpHash(void *d)
{
    uint32_t a,b,c;
    ByteJumpData *data = (ByteJumpData *)d;

    a = data->bytes_to_grab;
    b = data->offset;
    c = data->base;

    mix(a,b,c);

    a += (data->relative_flag << 24 |
          data->data_string_convert_flag << 16 |
          data->from_beginning_flag << 8 |
          data->align_flag);
    b += data->bitmask_val;
    c += data->multiplier;

    mix(a,b,c);

    a += RULE_OPTION_TYPE_BYTE_JUMP;
    b += data->post_offset;
    c += (data->endianess << 24 |
          data->offset_var << 16 |
          data->postoffset_var << 8 |
          data->from_end_flag);

    mix(a,b,c);

#if (defined(__ia64) || defined(__amd64) || defined(_LP64))
    {
        /* Cleanup warning because of cast from 64bit ptr to 32bit int
         * warning on 64bit OSs */
        uint64_t ptr; /* Addresses are 64bits */

        ptr = (uint64_t) data->byte_order_func;
        a += (ptr >> 32);
        b += (ptr & 0xFFFFFFFF);
    }
#else
    a += (uint32_t)data->byte_order_func;
#endif

    final(a,b,c);

    return c;
}

int ByteJumpCompare(void *l, void *r)
{
    ByteJumpData *left = (ByteJumpData *)l;
    ByteJumpData *right = (ByteJumpData *)r;

    if (!left || !right)
        return DETECTION_OPTION_NOT_EQUAL;

    if (( left->bytes_to_grab == right->bytes_to_grab) &&
        ( left->offset == right->offset) &&
        ( left->offset_var == right->offset_var) &&
        ( left->relative_flag == right->relative_flag) &&
        ( left->data_string_convert_flag == right->data_string_convert_flag) &&
        ( left->from_beginning_flag == right->from_beginning_flag) &&
        ( left->from_end_flag == right->from_end_flag) &&
        ( left->align_flag == right->align_flag) &&
        ( left->endianess == right->endianess) &&
        ( left->base == right->base) &&
        ( left->bitmask_val == right->bitmask_val) &&
        ( left->multiplier == right->multiplier) &&
        ( left->post_offset == right->post_offset) &&
        ( left->postoffset_var == right->postoffset_var) &&
        ( left->byte_order_func == right->byte_order_func))
    {
        return DETECTION_OPTION_EQUAL;
    }

    return DETECTION_OPTION_NOT_EQUAL;
}

static void ByteJumpOverride(char *keyword, char *option, RuleOptOverrideFunc roo_func)
{
    ByteJumpOverrideData *new = SnortAlloc(sizeof(ByteJumpOverrideData));

    new->keyword = SnortStrdup(keyword);
    new->option = SnortStrdup(option);
    new->func = roo_func;

    new->next = byteJumpOverrideFuncs;
    byteJumpOverrideFuncs = new;
}

static void ByteJumpOverrideFuncsFree(void)
{
    ByteJumpOverrideData *node = byteJumpOverrideFuncs;

    while (node != NULL)
    {
        ByteJumpOverrideData *tmp = node;

        node = node->next;

        if (tmp->keyword != NULL)
            free(tmp->keyword);

        if (tmp->option != NULL)
            free(tmp->option);

        free(tmp);
    }

    byteJumpOverrideFuncs = NULL;
}

/****************************************************************************
 *
 * Function: SetupByteJump()
 *
 * Purpose: Load 'er up
 *
 * Arguments: None.
 *
 * Returns: void function
 *
 ****************************************************************************/
void SetupByteJump(void)
{
    /* This list is only used during parsing */
    if (byteJumpOverrideFuncs != NULL)
        ByteJumpOverrideFuncsFree();

    /* map the keyword to an initialization/processing function */
    RegisterRuleOption("byte_jump", ByteJumpInit, ByteJumpOverride, OPT_TYPE_DETECTION, NULL);
    AddFuncToCleanExitList(ByteJumpOverrideCleanup, NULL);
    AddFuncToRuleOptParseCleanupList(ByteJumpOverrideFuncsFree);

#ifdef PERF_PROFILING
    RegisterPreprocessorProfile("byte_jump", &byteJumpPerfStats, 3, &ruleOTNEvalPerfStats, NULL);
#endif

    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,"Plugin: ByteJump Setup\n"););
}


/****************************************************************************
 *
 * Function: ByteJumpInit(char *, OptTreeNode *)
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
static void ByteJumpInit(struct _SnortConfig *sc, char *data, OptTreeNode *otn, int protocol)
{
    ByteJumpData *idx;
    OptFpList *fpl;
    ByteJumpOverrideData *override;
    void *idx_dup;

    /* allocate the data structure and attach it to the
       rule's data struct list */
    idx = (ByteJumpData *) calloc(sizeof(ByteJumpData), sizeof(char));

    if(idx == NULL)
    {
        FatalError("%s(%d): Unable to allocate byte_jump data node\n",
                   file_name, file_line);
    }

    /* this is where the keyword arguments are processed and placed into the
       rule option's data structure */
    override = ByteJumpParse(data, idx, otn);
    if (override != NULL)
    {
        /* There is an override function */
        free(idx);
        override->func(sc, override->keyword, override->option, data, otn, protocol);
        return;
    }

    fpl = AddOptFuncToList(ByteJump, otn);
    fpl->type = RULE_OPTION_TYPE_BYTE_JUMP;

    if (add_detection_option(sc, RULE_OPTION_TYPE_BYTE_JUMP, (void *)idx, &idx_dup) == DETECTION_OPTION_EQUAL)
    {
#ifdef DEBUG_RULE_OPTION_TREE

        LogMessage("Duplicate ByteJump:\n%d %d %c %c %c %c %c %c %d %d 0x%x %d %c %c\n"
            "%d %d %c %c %c %c %c %c %d %d 0x%x %d %c %c\n\n",
            idx->bytes_to_grab,
            idx->offset,
            idx->relative_flag,
            idx->data_string_convert_flag,
            idx->from_beginning_flag,
            idx->from_end_flag,
            idx->align_flag,
            idx->endianess,
            idx->base, idx->multiplier,
            idx->bitmask_val,
            idx->post_offset,
            idx->offset_var,
            idx->postoffset_var,
            ((ByteJumpData *)idx_dup)->bytes_to_grab,
            ((ByteJumpData *)idx_dup)->offset,
            ((ByteJumpData *)idx_dup)->relative_flag,
            ((ByteJumpData *)idx_dup)->data_string_convert_flag,
            ((ByteJumpData *)idx_dup)->from_beginning_flag,
            ((ByteJumpData *)idx_dup)->from_end_flag,
            ((ByteJumpData *)idx_dup)->align_flag,
            ((ByteJumpData *)idx_dup)->endianess,
            ((ByteJumpData *)idx_dup)->base,
            ((ByteJumpData *)idx_dup)->multiplier,
            ((ByteJumpData *)idx_dup)->bitmask_val,
            ((ByteJumpData *)idx_dup)->post_offset;
            ((ByteJumpData *)idx_dup)->offset_var;
            ((ByteJumpData *)idx_dup)->postoffset_var);
#endif
        free(idx);
        idx = idx_dup;
    }


    /* attach it to the context node so that we can call each instance
     * individually
     */
    fpl->context = (void *) idx;

    if (idx->relative_flag == 1)
        fpl->isRelative = 1;
}

/****************************************************************************
 *
 * Function: ByteJumpParse(char *, ByteJumpData *, OptTreeNode *)
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
static ByteJumpOverrideData * ByteJumpParse(char *data, ByteJumpData *idx, OptTreeNode *otn)
{
    char **toks;
    char *endp;
    int num_toks;
    char *cptr;
    int i =0;
    RuleOptByteOrderFunc tmp_byte_order_func = NULL;

    toks = mSplit(data, ",", 14, &num_toks, 0);

    if(num_toks < 2)
        ParseError("Bad arguments to byte_jump: %s\n", data);

    /* set how many bytes to process from the packet */
    idx->bytes_to_grab = strtoul(toks[0], &endp, 10);

    if(endp==toks[0])
    {
        ParseError("Unable to parse as byte value %s\n",toks[0]);
    }

    if(*endp != '\0')
    {
        ParseError("byte_jump option has bad input: %s.", toks[0]);
    }
    if(idx->bytes_to_grab > PARSELEN )
    {
        ParseError("byte_jump can't process more than "
                "%d bytes!\n",PARSELEN);
    }

    /* set offset */
    if (isdigit(toks[1][0]) || toks[1][0] == '-')
    {
        idx->offset = strtol(toks[1], &endp, 10);
        idx->offset_var = -1;

        if(endp==toks[1])
        {
            ParseError("Unable to parse as offset %s\n",toks[1]);
        }

        if(*endp != '\0')
        {
            ParseError("byte_jump option has bad offset: %s.", toks[1]);
        }
    }
    else
    {
        idx->offset_var = find_value(toks[1]);
        if (idx->offset_var == BYTE_EXTRACT_NO_VAR)
        {
            ParseError(BYTE_JUMP_INVALID_ERR_FMT, "byte_jump : offset", toks[1]);
        }
    }

    idx->postoffset_var = -1;
    i = 2;

    /* is it a relative offset? */
    if(num_toks > 2)
    {
        while(i < num_toks)
        {
            cptr = toks[i];

            while(isspace((int)*cptr)) {cptr++;}

            if(!strcasecmp(cptr, "relative"))
            {
                /* the offset is relative to the last pattern match */
                idx->relative_flag = 1;
            }
            else if(!strcasecmp(cptr, "from_beginning"))
            {
                idx->from_beginning_flag = 1;
            }
            else if(!strcasecmp(cptr, "from_end"))
            {
                idx->from_end_flag = 1;
            }
            else if(!strcasecmp(cptr, "string"))
            {
                /* the data will be represented as a string that needs
                 * to be converted to an int, binary is assumed otherwise
                 */
                idx->data_string_convert_flag = 1;
            }
            else if(!strcasecmp(cptr, "little"))
            {
                idx->endianess = LITTLE;
            }
            else if(!strcasecmp(cptr, "big"))
            {
                /* this is the default */
                idx->endianess = BIG;
            }
            else if(!strcasecmp(cptr, "hex"))
            {
                idx->base = 16;
            }
            else if(!strcasecmp(cptr, "dec"))
            {
                idx->base = 10;
            }
            else if(!strcasecmp(cptr, "oct"))
            {
                idx->base = 8;
            }
            else if(!strcasecmp(cptr, "align"))
            {
                idx->align_flag = 1;
            }
            else if(!strncasecmp(cptr, "multiplier ", 11))
            {
                /* Format of this option is multiplier xx.
                 * xx is a positive base 10 number.
                 */
                char *mval = &cptr[11];
                long factor = 0;
                int multiplier_len = strlen(cptr);
                if (multiplier_len > 11)
                {
                    factor = strtol(mval, &endp, 10);
                }
                if ((factor <= 0) || (endp != cptr + multiplier_len))
                {
                    ParseError("invalid length multiplier \"%s\"\n",cptr);
                }
                idx->multiplier = factor;
            }
            else if(!strncasecmp(cptr, "post_offset ", 12))
            {
                /* Format of this option is post_offset xx.
                 * xx is a positive or negative base 10 integer.
                 */
                if (!idx->post_offset)
                {
                  char *mval = &cptr[12];
                  int32_t factor = 0;
                  int postoffset_len = strlen(cptr);
                  while(isspace((int)*mval)) {mval++;}
                  if (postoffset_len > 12 && (isdigit(*mval) || *mval == '-' ))
                  {
                      factor = strtol(mval, &endp, 10);
                      idx->postoffset_var=-1;
                      if (endp != cptr + postoffset_len)
                      {
                          ParseError("invalid post_offset \"%s\"\n",cptr);
                      }
                      idx->post_offset = factor;
                  }
                  else
                  {
                      idx->postoffset_var = find_value(mval);
                      if ( idx->postoffset_var == BYTE_EXTRACT_NO_VAR)
                      {
                          ParseError(BYTE_JUMP_INVALID_ERR_FMT, "byte_jump : post_offset", mval);
                      }
                  }
               }
               else
               {
                   ParseError("byte_jump option post_offset is already configured in rule once\n");
               }
            }
            else if ((tmp_byte_order_func = GetByteOrderFunc(cptr)) != NULL)
            {
                idx->byte_order_func = tmp_byte_order_func;
            }
            else if(strncasecmp(cptr,"bitmask ",8) == 0)
            {
               RuleOptionBitmaskParse(&(idx->bitmask_val) , cptr, idx->bytes_to_grab,"BYTE_JUMP");
            }
            else
            {
                ByteJumpOverrideData *override = byteJumpOverrideFuncs;

                while (override != NULL)
                {
                    if (strcasecmp(cptr, override->option) == 0)
                    {
                        mSplitFree(&toks, num_toks);
                        return override;
                    }

                    override = override->next;
                }

                ParseError("unknown modifier \"%s\"\n",cptr);
            }

            i++;
        }
    }

    if (idx->bytes_to_grab > MAX_BYTES_TO_GRAB && !idx->data_string_convert_flag)
    {
        ParseError("byte_jump rule option cannot extract more than %d bytes without valid string prefix.",
                     MAX_BYTES_TO_GRAB);
    }
    /* idx->base is only set if the parameter is specified */
    if(!idx->data_string_convert_flag && idx->base)
    {
        ParseError("hex, dec and oct modifiers must be used in conjunction \n"
                   "        with the 'string' modifier\n");
    }
    if (idx->from_beginning_flag  && idx->from_end_flag)
    {
       ParseError("from_beginning and from_end options together in a rule is invalid config!\n");
    }
    if (idx->offset < MIN_BYTE_EXTRACT_OFFSET || idx->offset > MAX_BYTE_EXTRACT_OFFSET)
    {
        ParseError("byte_jump rule option has invalid offset. "
              "Valid offsets are between %d and %d.",
               MIN_BYTE_EXTRACT_OFFSET, MAX_BYTE_EXTRACT_OFFSET);
    }
    mSplitFree(&toks, num_toks);
    return NULL;
}


/****************************************************************************
 *
 * Function: ByteJump(char *, OptTreeNode *, OptFpList *)
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
int ByteJump(void *option_data, Packet *p)
{
    ByteJumpData *bjd = (ByteJumpData *)option_data;
    int rval = DETECTION_OPTION_NO_MATCH;
    uint32_t value = 0;
    uint32_t jump_value = 0;
    uint32_t payload_bytes_grabbed = 0;
    uint32_t extract_offset,extract_postoffset;
    int32_t offset, tmp = 0;
    int dsize;
    const uint8_t *base_ptr, *end_ptr, *start_ptr;
    uint8_t rst_doe_flags = 1;
    int search_start = 0;
    PROFILE_VARS;

    PREPROC_PROFILE_START(byteJumpPerfStats);

    if (Is_DetectFlag(FLAG_ALT_DETECT))
    {
        dsize = DetectBuffer.len;
        start_ptr = DetectBuffer.data;
        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                    "Using Alternative Detect buffer!\n"););
    }
    else if(Is_DetectFlag(FLAG_ALT_DECODE))
    {
        dsize = DecodeBuffer.len;
        start_ptr = DecodeBuffer.data;
        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                    "Using Alternative Decode buffer!\n"););
    }
    else
    {
        start_ptr = p->data;
        if(IsLimitedDetect(p))
            dsize = p->alt_dsize;
        else
            dsize = p->dsize;
    }

    /* save off whatever our ending pointer is */
    end_ptr = start_ptr + dsize;
    base_ptr = start_ptr;

    DEBUG_WRAP(
            DebugMessage(DEBUG_PATTERN_MATCH,"[*] byte jump firing...\n");
            DebugMessage(DEBUG_PATTERN_MATCH,"payload starts at %p\n", start_ptr);
            DebugMessage(DEBUG_PATTERN_MATCH,"payload ends   at %p\n", end_ptr);
            DebugMessage(DEBUG_PATTERN_MATCH,"doe_ptr           %p\n", doe_ptr);
            );  /* END DEBUG_WRAP */
    /* Get values from byte_extract variables, if present. */
    if (bjd->offset_var >= 0 )
    {
         if(bjd->offset_var == BYTE_MATH_VAR_INDEX )
         {
              bjd->offset = (int32_t) bytemath_variable;
         }

         else if(bjd->offset_var ==  COMMON_VAR_INDEX )
         {
              bjd->offset = (int32_t) common_var;
         }
         else if ( bjd->offset_var < NUM_BYTE_EXTRACT_VARS)
         {
              GetByteExtractValue(&extract_offset, bjd->offset_var);
              bjd->offset = (int32_t) extract_offset;
         }
    }

    if(bjd->relative_flag && doe_ptr)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                                "Checking relative offset!\n"););

        /* @todo: possibly degrade to use the other buffer, seems non-intuitive
         *  Because doe_ptr can be "end" in the last match,
         *  use end + 1 for upper bound
         *  Bound checked also after offset is applied
         *  (see byte_extract() and string_extract())
         */
        if(!inBounds(start_ptr, end_ptr + 1, doe_ptr))
        {
            DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                                    "[*] byte jump bounds check failed..\n"););

            PREPROC_PROFILE_END(byteJumpPerfStats);
            return rval;
        }

        search_start = (doe_ptr - start_ptr) + bjd->offset;
        base_ptr = doe_ptr;
        rst_doe_flags = 0;
    }
    else
    {
        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                                "checking absolute offset %d\n", bjd->offset););
        search_start = bjd->offset;
        base_ptr = start_ptr;
    }

    if (search_start < 0)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                                "[*] byte jump bounds check failed..\n"););

        PREPROC_PROFILE_END(byteJumpPerfStats);
        return rval;
    }

    base_ptr = base_ptr + bjd->offset;
    /* Use byte_order_func to determine endianess, if present */
    if (bjd->byte_order_func)
    {
        offset = (int32_t) (base_ptr - p->data);
        bjd->endianess = bjd->byte_order_func(p, offset);
        if (bjd->endianess == -1)
        {
            PREPROC_PROFILE_END(byteJumpPerfStats);
            return DETECTION_OPTION_NO_MATCH;
        }
    }

    /* Both of the extraction functions contain checks to insure the data
     * is always inbounds */

    if(!bjd->data_string_convert_flag && bjd->bytes_to_grab)
    {
        if(byte_extract(bjd->endianess, bjd->bytes_to_grab,
                        base_ptr, start_ptr, end_ptr, &value))
        {
            DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                                    "Byte Extraction Failed\n"););
            PREPROC_PROFILE_END(byteJumpPerfStats);
            return rval;
        }

        payload_bytes_grabbed = bjd->bytes_to_grab;
    }
    else if (bjd->data_string_convert_flag)
    {
        payload_bytes_grabbed = tmp = string_extract(bjd->bytes_to_grab, bjd->base,
                                               base_ptr, start_ptr, end_ptr, &value);
        if (tmp < 0 && bjd->bytes_to_grab)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                                    "Byte Extraction Failed\n"););

            PREPROC_PROFILE_END(byteJumpPerfStats);
            return rval;
        }

    }

    DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                            "grabbed %d of %d bytes, value = %08X\n",
                            payload_bytes_grabbed, bjd->bytes_to_grab, value););


    if(bjd->bitmask_val != 0 )
    {
        int num_tailing_zeros_bitmask = getNumberTailingZerosInBitmask(bjd->bitmask_val);
        value = value & bjd->bitmask_val ;
        if (value && num_tailing_zeros_bitmask )
        {
            value = value >> num_tailing_zeros_bitmask;
        }
    }

    DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,"post_bitmask operation the extracted value : 0x%08X(%u)\n", value,value););
    /* Adjust the jump_value (# bytes to jump forward) with the multiplier. */
    if (bjd->multiplier)
        jump_value = value * bjd->multiplier;
    else
        jump_value = value;

    DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                            "grabbed %d of %d bytes, after multiplier value = %08X\n",
                            payload_bytes_grabbed, bjd->bytes_to_grab, jump_value););


    /* if we need to align on 32-bit boundries, round up to the next
     * 32-bit value
     */
    if(bjd->align_flag)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                    "offset currently at %d\n", jump_value););
        if ((jump_value % 4) != 0)
        {
            jump_value += (4 - (jump_value % 4));
        }
        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                    "offset aligned to %d\n", jump_value););
    }

    DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                            "Grabbed %d bytes at offset %d, value = 0x%08X\n",
                            payload_bytes_grabbed, bjd->offset, jump_value););

    if(bjd->from_beginning_flag)
    {
        /* Reset base_ptr if from_beginning */
        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                                "jumping from beginning 0x%08X(%u) bytes\n", jump_value,jump_value););
        base_ptr = start_ptr;

        /* from base, push doe_ptr ahead "value" number of bytes */
        SetDoePtr((base_ptr + jump_value), DOE_BUF_STD);

    }
    else if(bjd->from_end_flag)
    {
        /* Reset base_ptr if from_end */
        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                                "jumping from end 0x%08X(%u) bytes\n", jump_value,jump_value););
        base_ptr = end_ptr;
        /* from base, push doe_ptr ahead "value" number of bytes */
        SetDoePtr((base_ptr + jump_value), DOE_BUF_STD);

    }
    else
    {
        UpdateDoePtr((base_ptr + payload_bytes_grabbed + jump_value), rst_doe_flags);
    }
    DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,"updated doe_ptr %p\n", doe_ptr););
    /* Get values from byte_extract/math variables, if present. */
    if (bjd->postoffset_var >=0)
    {
        if(bjd->postoffset_var == BYTE_MATH_VAR_INDEX)
        {
             bjd->post_offset = (int32_t) bytemath_variable;
        }

        else if(bjd->postoffset_var == COMMON_VAR_INDEX)
        {
             bjd->post_offset = (int32_t) common_var;
        }
        else if (bjd->postoffset_var < NUM_BYTE_EXTRACT_VARS)
        {
             GetByteExtractValue(&extract_postoffset, bjd->postoffset_var);
             bjd->post_offset = (int32_t) extract_postoffset;
        }
    }
    /* now adjust using post_offset -- before bounds checking */
    doe_ptr += bjd->post_offset;
    DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,"after applying post_offset to doe_ptr %p\n", doe_ptr););
    if(!inBounds(start_ptr, end_ptr+1, doe_ptr))
    {
        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                                "doe_ptr is not in bounds %p\n", doe_ptr););
        PREPROC_PROFILE_END(byteJumpPerfStats);
        return rval;
    }
    else
    {
        rval = DETECTION_OPTION_MATCH;
    }

    PREPROC_PROFILE_END(byteJumpPerfStats);
    return rval;
}

static void ByteJumpOverrideCleanup(int signal, void *data)
{
    if (byteJumpOverrideFuncs != NULL)
        ByteJumpOverrideFuncsFree();
}

