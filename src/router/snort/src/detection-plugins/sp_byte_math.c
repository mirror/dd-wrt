/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2003-2013 Sourcefire, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 ****************************************************************************/

/*
*  Filename    sp_byte_math.c
*
*  Authors     Krishnakanth <vkambala@cisco.com>
*              Seshaiah     <serugu@cisco.com>
*
* Description
* byte math is tied to the other byte operations – byte_test,byte_extract,byte_jump.
* allowed operations are +,-,*,/,<<,>> on the data that was extracted.
* options order can be change as they are prefix with identifier.
* Parser can identify the option with its prefix.
* result varaible should mention in the rule along with bytes_to extract,value and offset.
* result varaible stores the output of the byte_math opeation
* Byte_math output can be given input to Byte_extarct offset, Byte_Jump offset and Byte_test offset and value options

*Eg : byte_math: bytes 2, offset 0, oper *, rvalue 10, result area; byte_test:2,>,area,16;
*     At the zero offset of the paylod, extract 2 bytes and apply multiplication operation with value 10,store result in variable area. The area variable output is given as
*     input to byte_test value.
*
*         Lets consider 2 bytes of extarcted data in byte_math is 5.
*         The rvalue is 10. after multiplication operator applied between rvalue and extracted data,it became 50.
*         result option variable area holds value 50.
*         the byte_test can use the area varaible as input to it in either offset/value options.
*
*    Rule Examples :
*     alert tcp any any -> any any  (sid :1;byte_math:bytes 4,oper +,rvalue 123, offset 12,result var; msg: "Byte_math_valid";content : "|74 63 6c 61|";)
*
*     alert tcp any any -> any any  (sid :1;byte_math:bytes 1,oper <<,rvalue 123, offset 12,result var; msg: "Byte_math_valid";content : "|74 63 6c 61|";)
*
*     alert tcp any any -> any any (msg:"byte_math IT : byte_jump ####### CASE-5 # byte_math byte_extract and byte_jump with bitmask in each rule"; byte_math:oper /,rvalue 2, relative, result OFF1,offset 0, endian big,bytes 1,bitmask 0xA;byte_extract:1,1,OFF2,bitmask 0xA;byte_jump:1,OFF2,bitmask 0xA;byte_extract:1,3,VALUE,bitmask 0x5; byte_test:1,<,VALUE,OFF1,bitmask 0xCB;content:"SKIPME"; sid:2;)

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
#include "limits.h"
#include "sf_types.h"
#include "snort_bounds.h"
#include "byte_extract.h"
#include "rules.h"
#include "treenodes.h"
#include "decode.h"
#include "plugbase.h"
#include "parser.h"
#include "snort_debug.h"
#include "util.h"
#include "plugin_enum.h"
#include "mstring.h"
#include "sfhashfcn.h"
#include "sp_byte_math.h"
#include "sp_byte_extract.h"

#define PARSELEN 10
#define TEXTLEN  (PARSELEN + 2)

#include "snort.h"
#include "profiler.h"
#include "sfhashfcn.h"
#include "detection_options.h"
#include "detection_util.h"

#ifdef PERF_PROFILING
PreprocStats byteMathPerfStats;
extern PreprocStats ruleOTNEvalPerfStats;
#endif



typedef struct _ByteMathOverrideData
{
    char *keyword;
    char *option;
    union
    {
        RuleOptOverrideFunc fptr;
        void *void_fptr;
    } fptr;
    struct _ByteMathOverrideData *next;

} ByteMathOverrideData;

ByteMathOverrideData *byteMathOverrideFuncs = NULL;

static void ByteMathOverride(char *keyword, char *option, RuleOptOverrideFunc roo_func);
static void ByteMathOverrideFuncsFree(void);
static void ByteMathInit(struct _SnortConfig *, char *, OptTreeNode *, int);
static ByteMathOverrideData * ByteMathParse(char *data, ByteMathData *idx, OptTreeNode *otn);
static void ByteMathOverrideCleanup(int, void *);
static char* ByteMath_tok_extract(char *,char *);
void AddVarName_Bytemath(ByteMathData *);

char *bytemath_variable_name = NULL;
uint32_t bytemath_variable;
uint32_t common_var;

uint32_t find_value (char *token)
{
    if (token == NULL)
        return BYTE_EXTRACT_NO_VAR;

    uint32_t match_e = 0 ,match_b = 0;
    /* check byte_math already has the same name */
    if ( bytemath_variable_name && (strcmp(bytemath_variable_name,token) == 0) )
        match_b = BYTE_MATH_VAR_INDEX;

    /* check byte_extract already has the same name */
     match_e = GetVarByName(token);

    /* if same name found in both pick the latest one else the matched one */
     if ( (match_e != BYTE_EXTRACT_NO_VAR)  && (match_b == BYTE_MATH_VAR_INDEX) )
     {
         return COMMON_VAR_INDEX;
     }
     else if ( (match_e != BYTE_EXTRACT_NO_VAR)  && (match_b != BYTE_MATH_VAR_INDEX) )
     {
         return  match_e;
     }
     else if ( (match_e == BYTE_EXTRACT_NO_VAR)  && (match_b == BYTE_MATH_VAR_INDEX) )
     {
         return  BYTE_MATH_VAR_INDEX;
     }
     return  BYTE_EXTRACT_NO_VAR;
}

uint32_t ByteMathHash(void *d)
{
    uint32_t a,b,c;
    ByteMathData *data = (ByteMathData *)d;

    a = data->bytes_to_extract;
    b = data->rvalue;
    c = data->operator;

    mix(a,b,c);

    a += data->offset;
    b += (data->rvalue_var << 24 |
          data->relative_flag << 16 |
          data->data_string_convert_flag << 8 |
          data->endianess);
    c += data->base;

    mix(a,b,c);

    a += RULE_OPTION_TYPE_BYTE_MATH;
    b += data->bitmask_val;
    c += data->offset_var;

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


int ByteMathCompare(void *l, void *r)
{
    ByteMathData *left = (ByteMathData *)l;
    ByteMathData *right = (ByteMathData *)r;

    if (!left || !right)
        return DETECTION_OPTION_NOT_EQUAL;

    if (( left->bytes_to_extract == right->bytes_to_extract) &&
        ( left->rvalue == right->rvalue) &&
        ( left->operator == right->operator) &&
        ( left->offset == right->offset) &&
        ( left->relative_flag == right->relative_flag) &&
        ( left->data_string_convert_flag == right->data_string_convert_flag) &&
        ( left->endianess == right->endianess) &&
        ( left->base == right->base) &&
        ( left->bitmask_val == right->bitmask_val) &&
        ( left->rvalue_var == right->rvalue_var) &&
        ( left->offset_var == right->offset_var) &&
        ( left->byte_order_func == right->byte_order_func))
    {
        return DETECTION_OPTION_EQUAL;
    }

    return DETECTION_OPTION_NOT_EQUAL;
}

static void ByteMathOverride(char *keyword, char *option, RuleOptOverrideFunc roo_func)
{
    ByteMathOverrideData *new = SnortAlloc(sizeof(ByteMathOverrideData));

    new->keyword = SnortStrdup(keyword);
    new->option = SnortStrdup(option);
    new->func = roo_func;

    new->next = byteMathOverrideFuncs;
    byteMathOverrideFuncs = new;
}


static void ByteMathOverrideFuncsFree(void)
{
    ByteMathOverrideData *node = byteMathOverrideFuncs;

    while (node != NULL)
    {
        ByteMathOverrideData *tmp = node;

        node = node->next;

        if (tmp->keyword != NULL)
            free(tmp->keyword);

        if (tmp->option != NULL)
            free(tmp->option);

        free(tmp);
    }

    byteMathOverrideFuncs = NULL;
}

static void ByteMathOverrideCleanup(int signal, void *data)
{
    if (byteMathOverrideFuncs != NULL)
        ByteMathOverrideFuncsFree();
}

/****************************************************************************
 * Function: SetupByteMath()
 *
 * Purpose: Register byte_math name and initialization function
 *
 * Arguments: None.
 *
 * Returns: void function
 *
 ****************************************************************************/
void SetupByteMath(void)
{
    /* map the keyword to an initialization/processing function */
    RegisterRuleOption("byte_math", ByteMathInit, ByteMathOverride, OPT_TYPE_DETECTION, NULL);
    AddFuncToCleanExitList(ByteMathOverrideCleanup, NULL);
    AddFuncToRuleOptParseCleanupList(ByteMathOverrideFuncsFree);
#ifdef PERF_PROFILING
    RegisterPreprocessorProfile("byte_math", &byteMathPerfStats, 3, &ruleOTNEvalPerfStats, NULL);
#endif
    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,"Plugin: ByteMath Setup\n"););
}

/****************************************************************************
 *
 * Function: ByteMathInit(char *, OptTreeNode *)
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
static void ByteMathInit(struct _SnortConfig *sc, char *data, OptTreeNode *otn, int protocol)
{

    ByteMathData *idx;
    OptFpList *fpl;
    ByteMathOverrideData *override;
    void *idx_dup;

    /* allocate the data structure and attach it to the
       rule's data struct list */
    idx = (ByteMathData *) calloc(sizeof(ByteMathData), sizeof(char));

    if(idx == NULL)
    {
        ParseError("Byte_Math Unable to allocate byte_Math data node\n");
    }
    /* this is where the keyword arguments are processed and placed into the
       rule option's data structure */
    override = ByteMathParse(data, idx, otn);
    if (override)
    {
        /* There is an override function */
        free(idx->result_var);
        free(idx);
        override->func(sc, override->keyword, override->option, data, otn, protocol);
        return;
    }

    AddVarName_Bytemath(idx);

    fpl = AddOptFuncToList(ByteMath, otn);
    fpl->type = RULE_OPTION_TYPE_BYTE_MATH;

    if (add_detection_option(sc, RULE_OPTION_TYPE_BYTE_MATH, (void *)idx, &idx_dup) == DETECTION_OPTION_EQUAL)
    {
#ifdef DEBUG_RULE_OPTION_TREE
        LogMessage("Byte_Math Duplicate ByteCheck:\n%d %d %c %d %d %c %c %c %c 0x%x %d\n"
            "%d %d %c %d %d %c %c %c %c 0x%x %d\n\n",
            idx->bytes_to_extract,
            idx->rvalue,idx->rvalue_var,
            idx->operator,
            idx->offset,idx->offset_var,
            idx->relative_flag,
            idx->data_string_convert_flag,
            idx->endianess, idx->base,
            idx->bitmask_val,
            ((ByteMathData *)idx_dup)->bytes_to_extract,
            ((ByteMathData *)idx_dup)->rvalue,
            ((ByteMathData *)idx_dup)->rvalue_var,
            ((ByteMathData *)idx_dup)->operator,
            ((ByteMathData *)idx_dup)->offset,
            ((ByteMathData *)idx_dup)->offset_var,
            ((ByteMathData *)idx_dup)->relative_flag,
            ((ByteMathData *)idx_dup)->data_string_convert_flag,
            ((ByteMathData *)idx_dup)->bitmask_val,
            ((ByteMathData *)idx_dup)->endianess, ((ByteMathData *)idx_dup)->base);
#endif
        free(idx->result_var);
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
 * Function: ByteMath_tok_extract(char *,char *)
 *
 * Purpose: This function does extracting token content in rule options
 *
 * Arguments: src => Input token from the rule
 *            del => Option the token to be mapped
 *
 * Returns: On success returns the resultant string address else trigger fatal error
 *
 ****************************************************************************/

static char* ByteMath_tok_extract(char *src,char *del)
{
        char *ret_tok=NULL;
        ret_tok = strtok(src," ");
        if (ret_tok && !strcmp(ret_tok,del))
        {
           ret_tok = strtok(NULL, ",");
           if (ret_tok)
           {
                while(isspace((int)*ret_tok)) {ret_tok++;}
                return (ret_tok);
           }
        }
        ParseError("Byte_Math token input[%s] is invalid one for options[%s]\n",ret_tok,del);
        return ret_tok;
}


/****************************************************************************
 *
 * Function: ByteMathParse(char *, ByteMathData *, OptTreeNode *)
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


static ByteMathOverrideData * ByteMathParse(char *data, ByteMathData *idx, OptTreeNode *otn)
{

    char **toks;
    char *endp;
    int num_toks;
    char *cptr;
    int i = 0;
    bool offset_flag = false;
    RuleOptByteOrderFunc tmp_byte_order_func;

    idx->rvalue_var = -1;

    toks = mSplit(data, ",", 13, &num_toks, 0);

    while(i < num_toks)
    {
        cptr=toks[i];
        while(isspace((int)*cptr)) {cptr++;}
        /* set how many bytes to process from the packet */
        if (!strncmp(cptr,"bytes",5))
        {
            if (!idx->bytes_to_extract)
            {
                cptr=ByteMath_tok_extract(cptr,"bytes");
                idx->bytes_to_extract = strtol(cptr, &endp, 10);
                if (*endp != '\0')
                {
                    ParseError("byte_math option has bad input: %s.", cptr);
                }
                if(idx->bytes_to_extract > PARSELEN || idx->bytes_to_extract == 0)
                {
                    ParseError("byte_math option bytes_to_extract has invalid input.valid range is 1 to 10 bytes\n");
                }
                i++;
                continue;

           }
        }

        else if (!strncmp(cptr,"oper",4))
        {
           if (!idx->operator)
           {
              cptr=ByteMath_tok_extract(cptr,"oper");
              /* set the operator */
              switch(*cptr)
              {
               case '+': idx->operator = BM_PLUS;
                      break;

               case '-': idx->operator = BM_MINUS;
                      break;

               case '*': idx->operator = BM_MULTIPLY;
                      break;

               case '/': idx->operator = BM_DIVIDE;
                      break;

               case '<': cptr++;
                       if (*cptr == '<')
                           idx->operator = BM_LEFT_SHIFT;
                       else
                          ParseError("byte_math unknown operator [%s]\n",--cptr);
                       break;

               case '>': cptr++;
                       if (*cptr == '>')
                          idx->operator = BM_RIGHT_SHIFT;
                       else
                          ParseError("byte_math unknown operator [%s]\n",--cptr);
                       break;

               default:
                          ParseError("byte_math unknown operator [%s]\n",cptr);
              }

           cptr++;
           if (*cptr)
           {
              ParseError("byte_math unknown operator[%s]\n",--cptr);
           }
           i++;
           continue;
        }
        else
        {
           ParseError("byte_math option OPERATOR is already configured in rule\n");
        }
      }

     else if (!strncmp(cptr,"rvalue",6))
     {
        if (!idx->rvalue)
        {
           /* set the value to test against */
           cptr=ByteMath_tok_extract(cptr,"rvalue");
           if (isdigit(*cptr) || *cptr == '-')
           {
               int64_t rval = SnortStrtoul(cptr, &endp,0);
               if (rval > MAX_RVAL || !rval)
               {
                  ParseError("byte_math rule option has invalid rvalue."
                     "Valid rvalue range %u-%u.",
                     MIN_RVAL,MAX_RVAL);
               }
               idx->rvalue=rval;
               idx->rvalue_var = -1;
               if(*endp != '\0')
               {
                   ParseError("byte_math option has bad rvalue: %s", cptr);
               }
           }
           else
           {
               idx->rvalue_var = GetVarByName(cptr);
               if (idx->rvalue_var == BYTE_EXTRACT_NO_VAR)
               {
                   ParseError(BYTE_EXTRACT_INVALID_ERR_FMT, "byte_Math", cptr);
               }
           }
           i++;
           continue;
        }
        else
        {
           ParseError("byte_math rvalue is already configured in rule once\n");
        }
     }

     else if (!strncmp(cptr,"offset",6))
     {
        if (!idx->offset)
        {
           cptr=ByteMath_tok_extract(cptr,"offset");
           /* set offset */
           if (isdigit(*cptr) || *cptr == '-')
           {
               idx->offset = SnortStrtoul(cptr, &endp,0);
               idx->offset_var = -1;
               if(*endp != '\0')
               {
                   ParseError("byte_math option has bad offset: %s", cptr);
               }
           }
           else
           {
               idx->offset_var = GetVarByName(cptr);
               if (idx->offset_var == BYTE_EXTRACT_NO_VAR)
               {
                   ParseError(BYTE_EXTRACT_INVALID_ERR_FMT, "byte_Math", cptr);
               }
           }
           offset_flag = true;
           i++;
           continue;
       }
       else
       {
           ParseError("byte_math option offset is Already configured in rule once\n");
       }
    }
    else if (!strncmp(cptr,"result",6))
    {
        if (!idx->result_var)
        {
           cptr=ByteMath_tok_extract(cptr,"result");
           /* set result variable */
            idx->result_var =  SnortStrdup(cptr);
            if (!idx->result_var)
                ParseError("byte_Math::result_var malloc failure");

           if (idx->result_var && isdigit(idx->result_var[0]))
           {
              free(idx->result_var);
              ParseError("byte_Math rule option has a name which starts with a digit. "
                   "Variable names must start with a letter.");
           }
           isvalidstr(idx->result_var,"byte_math");
           i++;
           continue;
       }
       else
       {

           ParseError("byte_math result is Already configured in rule once\n");
       }
    }

    else if (!strncmp(cptr,"relative",8))
    {
           /* the offset is relative to the last pattern match */
        idx->relative_flag = 1;
        i++;
        continue;
    }
    else if(!strncmp(cptr, "string",6))
    {
        if (!idx->data_string_convert_flag )
        {
            /* the data will be represented as a string that needs
            * to be converted to an int, binary is assumed otherwise
            */
            idx->data_string_convert_flag = 1;
            cptr=ByteMath_tok_extract(cptr,"string");
            if(!strcasecmp(cptr, "hex"))
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
            else
            {
                ParseError("byte_math Unable to parse string option\n");
            }
       }
       else
       {
          ParseError("byte_math string is Already configured in rule once\n");
       }
       i++;
       continue;
     }

     else if(strncasecmp(cptr,"bitmask ",8) == 0)
     {
         RuleOptionBitmaskParse(&(idx->bitmask_val), cptr, idx->bytes_to_extract,"BYTE_MATH");
         i++;
         continue;
     }
     else if(!strncmp(cptr, "endian",6))
     {
        if (!idx->endianess)
        {
           cptr=ByteMath_tok_extract(cptr,"endian");
           if(!strcasecmp(cptr, "little"))
           {
               idx->endianess = LITTLE;
           }
           else if(!strcasecmp(cptr, "big"))
           {
              /* this is the default */
              idx->endianess = BIG;
           }
           else
           {
                ParseError("byte_math Unable to parse Endian option\n");
           }
        }
        else
        {

           ParseError("byte_math Endian is Already configured in rule\n");
        }
        i++;
        continue;
     }
     else if((tmp_byte_order_func = GetByteOrderFunc(cptr)) != NULL)
     {
        idx->byte_order_func = tmp_byte_order_func;
        i++;
        continue;
     }
     else
     {
         ByteMathOverrideData *override = byteMathOverrideFuncs;

        while (override != NULL)
       {
           if (!strcasecmp(cptr, override->option))
           {
              mSplitFree(&toks, num_toks);
              return override;
           }

           override = override->next;
       }

       ParseError("byte_math unknown modifier \"%s\"\n",cptr);
     }


    }
    if ( (!idx->bytes_to_extract) || (!idx->operator) || ( (!idx->rvalue) && (idx->rvalue_var == -1) ) || (!idx->result_var) || (!offset_flag) )
    {
           ParseError("Check the bytes_to_extract/operator/offset/rvalue/result variable\n");
    }
    if ( ( (idx->operator == BM_LEFT_SHIFT) || (idx->operator == BM_RIGHT_SHIFT) ) && (idx->rvalue >32) )
    {
           ParseError("Number of bits in rvalue input [%d] should be less than 32 bits for operator\n",idx->rvalue);
    }
    if ( ( (idx->operator == BM_LEFT_SHIFT) || (idx->operator == BM_RIGHT_SHIFT) ) && (idx->bytes_to_extract >4) )
    {
           ParseError("for operators << and  >> valid bytes_to_extract input range is 1 to 4 bytes\n");
    }
    if (idx->offset < MIN_BYTE_EXTRACT_OFFSET || idx->offset > MAX_BYTE_EXTRACT_OFFSET)
    {
        ParseError("byte_math rule option has invalid offset. "
              "Valid offsets are between %d and %d.",
               MIN_BYTE_EXTRACT_OFFSET, MAX_BYTE_EXTRACT_OFFSET);
    }
    if (idx->bytes_to_extract > MAX_BYTES_TO_GRAB && !idx->data_string_convert_flag)
    {
        ParseError("byte_math rule option cannot extract more than %d bytes without valid string prefix.",
                     MAX_BYTES_TO_GRAB);
    }
     mSplitFree(&toks, num_toks);
     return NULL;

}


/****************************************************************************
 *
 * Function: ByteMath(char *, OptTreeNode *, OptFpList *)
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
int ByteMath(void *option_data, Packet *p)
{
    ByteMathData *btd = (ByteMathData *)option_data;
    int rval = DETECTION_OPTION_NO_MATCH;
    uint32_t *value = 0;
    int success = 0;
    int dsize;
    const char *base_ptr, *end_ptr, *start_ptr;
    int payload_bytes_grabbed;
    int32_t offset;
    uint32_t extract_offset, extract_rvalue;
    int search_start = 0;
    PROFILE_VARS;

    PREPROC_PROFILE_START(byteMathPerfStats);

    if (Is_DetectFlag(FLAG_ALT_DETECT))
    {
        dsize = DetectBuffer.len;
        start_ptr = (const char*)DetectBuffer.data;
        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                "Using Alternative Detect buffer!\n"););
    }
    else if(Is_DetectFlag(FLAG_ALT_DECODE))
    {
        dsize = DecodeBuffer.len;
        start_ptr = (char *)DecodeBuffer.data;
        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                    "Using Alternative Decode buffer!\n"););
    }
    else
    {
        if(IsLimitedDetect(p))
            dsize = p->alt_dsize;
        else
            dsize = p->dsize;
        start_ptr = (char *) p->data;
    }

    base_ptr = start_ptr;
    end_ptr = start_ptr + dsize;

    DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                "[*] byte Math firing...\n");
            DebugMessage(DEBUG_PATTERN_MATCH,"payload starts at %p\n", start_ptr);
            DebugMessage(DEBUG_PATTERN_MATCH,"payload ends   at %p\n", end_ptr);
            DebugMessage(DEBUG_PATTERN_MATCH,"doe_ptr           %p\n", doe_ptr);
	);

    value = &bytemath_variable;

    /* Get values from byte_extract variables, if present. */
    if (btd->rvalue_var >= 0 && btd->rvalue_var < NUM_BYTE_EXTRACT_VARS)
    {
        GetByteExtractValue(&extract_rvalue, btd->rvalue_var);
        btd->rvalue = (int32_t) extract_rvalue;
        if (!btd->rvalue && (btd->operator == BM_DIVIDE))
        {
           DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                      "byte math input value zero for Divide operator is invalid\n"););
           PREPROC_PROFILE_END(byteMathPerfStats);
           return DETECTION_OPTION_NO_MATCH;
        }
    }

    if (btd->offset_var >= 0 && btd->offset_var < NUM_BYTE_EXTRACT_VARS)
    {
        GetByteExtractValue(&extract_offset, btd->offset_var);
        btd->offset = (int32_t) extract_offset;
    }

    if(btd->relative_flag && doe_ptr)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                                "Checking relative offset!\n"););

        /* @todo: possibly degrade to use the other buffer, seems non-intuitive
         *  Because doe_ptr can be "end" in the last match,
         *  use end + 1 for upper bound
         *  Bound checked also after offset is applied
         *  (see byte_extract() and string_extract())
         */
        if(!inBounds((const uint8_t *)start_ptr, (const uint8_t *)end_ptr + 1, doe_ptr))
        {
            DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                                    "[*] byte Math bounds check failed..\n"););
            PREPROC_PROFILE_END(byteMathPerfStats);
            return rval;
        }

        search_start = (doe_ptr - (const uint8_t *)start_ptr) + btd->offset;
        base_ptr = (const char *)doe_ptr;
    }
    else
    {
        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                                "checking absolute offset %d\n", btd->offset););
        search_start = btd->offset;
        base_ptr = start_ptr;
    }

    if( search_start < 0 )
    {
        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                                "[*] byte Math bounds check failed..\n"););
        PREPROC_PROFILE_END(byteMathPerfStats);
        return rval;
    }

    base_ptr = base_ptr + btd->offset;

    /* Use byte_order_func to determine endianess, if present */
    if (btd->byte_order_func)
    {
        offset = (int32_t) ((const uint8_t *)base_ptr - p->data);
        btd->endianess = btd->byte_order_func(p, offset);
        if (btd->endianess == -1)
        {
            PREPROC_PROFILE_END(byteMathPerfStats);
            return DETECTION_OPTION_NO_MATCH;
        }
    }

    /* both of these functions below perform their own bounds checking within
     * byte_extract.c
     */
    if(!btd->data_string_convert_flag)
    {
        if(byte_extract(btd->endianess, btd->bytes_to_extract,
                        (const uint8_t *)base_ptr, (const uint8_t *)start_ptr, (const uint8_t *)end_ptr, value))
        {
            DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                                    "Byte Extraction Failed\n"););

            PREPROC_PROFILE_END(byteMathPerfStats);
            return rval;
        }
        payload_bytes_grabbed = (int)btd->bytes_to_extract;
    }
    else
    {
        payload_bytes_grabbed = string_extract(
                btd->bytes_to_extract, btd->base,
                (const uint8_t *)base_ptr, (const uint8_t *)start_ptr,
                (const uint8_t *)end_ptr, value);

        if ( payload_bytes_grabbed < 0 )
        {
            DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                                    "String Extraction Failed\n"););

            PREPROC_PROFILE_END(byteMathPerfStats);
            return rval;
        }

    }

    if(btd->bitmask_val != 0 )
    {
        int num_tailing_zeros_bitmask = getNumberTailingZerosInBitmask(btd->bitmask_val);
        *value = (*value) & btd->bitmask_val ;
        if ( (*value ) && num_tailing_zeros_bitmask )
        {
           *value = (*value) >> num_tailing_zeros_bitmask;
        }
    }

    DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
        "Grabbed %d bytes at offset %d rvalue = 0x%08X(%u) value = 0x%08X(%u)\n",
        payload_bytes_grabbed, btd->offset, btd->rvalue, btd->rvalue, value, value); );
    switch(btd->operator)
    {
        case BM_PLUS: if ((UINT_MAX - *value) < btd->rvalue)
                      {
                          LogMessage("%s-%d > %s-%d Buffer Overflow during ADDITION\n",
                                     inet_ntoa(GET_SRC_IP(p)),ntohs(p->tcph->th_sport),
                                     inet_ntoa(GET_DST_IP(p)),ntohs(p->tcph->th_dport));   
                          return DETECTION_OPTION_NO_MATCH;
                      }
                      else
                      {
                          *value += btd->rvalue;
                          success = 1;
                          break;
                      }

        case BM_MINUS: if (*value < btd->rvalue)
                       { 
                           LogMessage("%s-%d > %s-%d Buffer Underflow during SUBTRACTION\n",
                                      inet_ntoa(GET_SRC_IP(p)),ntohs(p->tcph->th_sport),
                                      inet_ntoa(GET_DST_IP(p)),ntohs(p->tcph->th_dport));   
                           return DETECTION_OPTION_NO_MATCH;
                       }
                       else
                       {
                           *value -= btd->rvalue;
                           success = 1;
                           break;
                       }

        case BM_MULTIPLY: if ( (*value) && ((UINT_MAX/(*value)) < btd->rvalue))
                          {
                              LogMessage("%s-%d > %s-%d Buffer Overflow during MULTIPLY\n",
                                          inet_ntoa(GET_SRC_IP(p)),ntohs(p->tcph->th_sport),
                                          inet_ntoa(GET_DST_IP(p)),ntohs(p->tcph->th_dport));   
                              return DETECTION_OPTION_NO_MATCH;
                          }
                          else
                          {
                             *value *= btd->rvalue;
                             success = 1;
                             break;
                          }

        case BM_DIVIDE: *value = (*value/ btd->rvalue);
                     success = 1;
                     break;

        case BM_LEFT_SHIFT: *value = (*value << btd->rvalue);
                    success = 1;
                    break;

        case BM_RIGHT_SHIFT: *value = (*value >> btd->rvalue);
                    success = 1;
                    break;
    }

    DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
        "byte_math result final value = 0x%08X(%u)\n",
        value, value); );
    if (success)
    {
        rval = DETECTION_OPTION_MATCH;
        common_var = *value;
    }

    /* if the test isn't successful, this function *must* return 0 */
    PREPROC_PROFILE_END(byteMathPerfStats);
    return rval;
}

void ByteMathFree(void *d)
{
    ByteMathData *data = (ByteMathData *)d;
    if (data && data->result_var)
    {
      free(data->result_var);
      data->result_var=NULL;
    }

    if ( bytemath_variable_name != NULL )
    {
        free( bytemath_variable_name );
        bytemath_variable_name = NULL;
    }
    free(data);
}

/*for given an OptFpList,clear the variable_name */
void ClearByteMathVarNames(OptFpList *fpl)
{
    while (fpl != NULL)
    {
        if (fpl->type == RULE_OPTION_TYPE_BYTE_MATH)
            return;

        fpl = fpl->next;
    }
    if (bytemath_variable_name != NULL)
    {
       free(bytemath_variable_name);
       bytemath_variable_name = NULL;
    }
}

/* Given a variable name, retrieve its index. For use by other options.dynamic-plugin support */
int8_t GetVarByName_check(char *name)
{
    return (find_value(name));
}

void AddVarName_Bytemath(ByteMathData *data)
{
  if (bytemath_variable_name != NULL)
    {
       free(bytemath_variable_name);
       bytemath_variable_name = NULL;
    }
    bytemath_variable_name = SnortStrdup(data->result_var);

}

int8_t Var_check_byte_math(char *name)
{
   if (!strcmp(bytemath_variable_name,name))
   {
      return 1;
   }
   return 0;
}
