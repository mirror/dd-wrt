/*
 ** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 ** Copyright (C) 2010-2013 Sourcefire, Inc.
 ** Author: Ryan Jordan <ryan.jordan@sourcefire.com>
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

/* sp_byte_extract
 *
 * Description goes here. Snort rule interface for byte_extract functionality.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_types.h"
#include "snort.h"
#include "parser.h"
#include "plugbase.h"
#include "preprocids.h"
#include "detection_options.h"
#include "detection_util.h"
#include "sfhashfcn.h"
#include "profiler.h"
#include "byte_extract.h"

#include "sp_byte_extract.h"
#include "sp_byte_math.h"

#ifdef PERF_PROFILING
PreprocStats byteExtractPerfStats;
extern PreprocStats ruleOTNEvalPerfStats;
#endif

extern int file_line;
extern char *file_name;

uint32_t Byte_Extract_Offset_Var;

/* Storage for extracted variables */
static uint32_t extracted_values[NUM_BYTE_EXTRACT_VARS];
static char *variable_names[NUM_BYTE_EXTRACT_VARS];

/* Prototypes */
static void ByteExtractInit(struct _SnortConfig *, char *, OptTreeNode *, int);
static void ByteExtractCleanup(int, void *);

/* Setup function */
void SetupByteExtract(void)
{
    RegisterRuleOption("byte_extract", ByteExtractInit, NULL, OPT_TYPE_DETECTION, NULL);
    AddFuncToCleanExitList(ByteExtractCleanup, NULL);

#ifdef PERF_PROFILING
    RegisterPreprocessorProfile("byte_extract", &byteExtractPerfStats, 3, &ruleOTNEvalPerfStats, NULL);
#endif
}

/* Clean up some strings left over from parsing */
static void ByteExtractCleanup(int signal, void *data)
{
    int i;
    for (i = 0; i < NUM_BYTE_EXTRACT_VARS; i++)
    {
        free(variable_names[i]);
        variable_names[i] = NULL;
    }
}

#ifdef DEBUG_MSGS
/* Print a byte_extract option to console. For debugging purposes. */
void PrintByteExtract(ByteExtractData *data)
{
    if (data == NULL)
        return;

    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,
           "bytes_to_grab = %d, offset = %d, relative = %d, convert = %d, "
           "align = %d, endianess = %d, base = %d, "
           "multiplier = %d, var_num = %d, name = %s,  bitmask_val = 0x%x\n",
           data->bytes_to_grab,
           data->offset,
           data->relative_flag,
           data->data_string_convert_flag,
           data->align,
           data->endianess,
           data->base,
           data->multiplier,
           data->var_number,
           data->name,
           data->bitmask_val););
}
#endif

/* Hash functions. Make sure to update these when the data struct changes! */
uint32_t ByteExtractHash(void *d)
{
    uint32_t a,b,c;
    ByteExtractData *data = (ByteExtractData *)d;

    a = data->bytes_to_grab;
    b = data->offset;
    c = data->base;

    mix(a,b,c);

    a += (data->relative_flag << 24 |
          data->data_string_convert_flag << 16 |
          data->align << 8 |
          data->endianess);
    b += data->multiplier;
    c += data->bitmask_val;

    mix(a,b,c);

    a += RULE_OPTION_TYPE_BYTE_EXTRACT;
    b += data->var_number;

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

int ByteExtractCompare(void *l, void *r)
{
    ByteExtractData *left = (ByteExtractData *) l;
    ByteExtractData *right = (ByteExtractData *) r;

    if (!left || !right)
        return DETECTION_OPTION_NOT_EQUAL;

    if ((left->bytes_to_grab == right->bytes_to_grab) &&
        (left->offset == right->offset) &&
        (left->relative_flag == right->relative_flag) &&
        (left->data_string_convert_flag == right->data_string_convert_flag) &&
        (left->align == right->align) &&
        (left->endianess == right->endianess) &&
        (left->base == right->base) &&
        (left->multiplier == right->multiplier) &&
        (left->var_number == right->var_number) &&
        (left->byte_order_func == right->byte_order_func) &&
        (left->bitmask_val == right->bitmask_val))
    {
        return DETECTION_OPTION_EQUAL;
    }

    return DETECTION_OPTION_NOT_EQUAL;
}

void ByteExtractFree(void *d)
{
    ByteExtractData *data = (ByteExtractData *)d;
    free(data->name);
    free(data);
}

/* Checks a ByteExtractData instance for errors. */
static int ByteExtractVerify(ByteExtractData *data)
{
    if (data->bytes_to_grab > MAX_BYTES_TO_GRAB && data->data_string_convert_flag == 0)
    {
        ParseError("byte_extract rule option cannot extract more than %d bytes.",
                     MAX_BYTES_TO_GRAB);
    }

    if (data->bytes_to_grab > PARSELEN && data->data_string_convert_flag == 1)
    {
        ParseError("byte_extract rule cannot process more than %d bytes for "
                   "string extraction.",  PARSELEN);
    }

    if (Byte_Extract_Offset_Var != BYTE_MATH_VAR_INDEX )
    {
         if (data->offset < MIN_BYTE_EXTRACT_OFFSET || data->offset > MAX_BYTE_EXTRACT_OFFSET)
         {
              ParseError("byte_extract rule option has invalid offset. "
                   "Valid offsets are between %d and %d.",
                     MIN_BYTE_EXTRACT_OFFSET, MAX_BYTE_EXTRACT_OFFSET);
         }
    }

    if (data->multiplier < MIN_BYTE_EXTRACT_MULTIPLIER || data->multiplier > MAX_BYTE_EXTRACT_MULTIPLIER)
    {
        ParseError("byte_extract rule option has invalid multiplier. "
                   "Valid multipliers are between %d and %d.",
                    MIN_BYTE_EXTRACT_MULTIPLIER, MAX_BYTE_EXTRACT_MULTIPLIER);
    }

    if (data->bytes_to_grab == 0)
        ParseError("byte_extract rule option extracts zero bytes. "
                   "\"bytes_to_extract\" must be 1 or greater.");

    if (data->align != 0 && data->align != 2 && data->align != 4)
        ParseError("byte_extract rule option has an invalid argument "
                   "to \"align\". Valid arguments are \'2\' and \'4\'.");

    if (data->offset < 0 && data->relative_flag == 0)
        ParseError("byte_extract rule option has a negative offset, but does "
                   "not use the \"relative\" option.");

    if (data->name && isdigit(data->name[0]))
    {
        ParseError("byte_extract rule option has a name which starts with a digit. "
                   "Variable names must start with a letter.");
    }

    if (data->base && !data->data_string_convert_flag)
    {
        ParseError("byte_extract rule option has a string converstion type "
                   "(\"dec\", \"hex\", or \"oct\") without the \"string\" "
                   "argument.");
    }

    return BYTE_EXTRACT_SUCCESS;
}


int numBytesInBitmask(uint32_t bitmask_value)
{
   int num_bytes;
   if( bitmask_value <= 0xFF )
       num_bytes = 1;
   else if ( bitmask_value <= 0xFFFF )
       num_bytes = 2;
   else if ( bitmask_value <= 0xFFFFFF )
       num_bytes = 3;
   else
       num_bytes = 4;

   return num_bytes;
}

void RuleOptionBitmaskParse(uint32_t* bitmask_val, char *cptr, uint32_t bytes_to_extract,char* ruleOptionName)
{
     char* endp = NULL;
     char* startp = (cptr + 8);
     uint32_t bitmask_value;
     uint32_t num_bytes=0;

     if(*bitmask_val == 0 )
     {
          if(SnortStrToU32(startp,&endp,&bitmask_value,16) == -1 )
               ParseError("%s :: Invalid input value for \"bitmask\" rule option.\n", ruleOptionName);
          else if( errno == ERANGE )
               ParseError("%s :: \"bitmask\" value is OUT OF RANGE.\n", ruleOptionName);
          else if (bitmask_value == 0 )
               ParseError("%s :: \"bitmask\" value is ZERO.\n", ruleOptionName);
          else if (*endp != '\0')
                ParseError("%s :: Rule option has invalid argument to \"bitmask\".\n", ruleOptionName);
          else
          {
               num_bytes = numBytesInBitmask(bitmask_value);
               if (bytes_to_extract  <= MAX_BYTES_TO_EXTRACT)
               {
                   if(bytes_to_extract >= num_bytes )
                   {
                       *bitmask_val = bitmask_value;
                   }
                   else
                       ParseError("%s :: Number of bytes in \"bitmask\" value is greater than  bytes_to_grab.\n", ruleOptionName);
               }
               else
                    ParseError("%s :: Number of extracted bytes from packet are more than MAX_BYTES_TO_GRAB.\n", ruleOptionName);
          }
     }
     else
          ParseError("%s :: Rule option includes the \"bitmask\" argument twice.\n",ruleOptionName);
}

/* String Validation for all special characters in the varaibale name */
void isvalidstr(char *str,char *feature)
{
   int cnt=0,pos=0;
   for (;str[pos]!='\0';pos++)
   {
      if (!((str[pos] >= 'a' && str[pos] <= 'z') || (str[pos] >= 'A' && str[pos] <= 'Z') || (str[pos] >= '0' && str[pos] <='9')))
           cnt++;
   }
   if (pos == cnt)
           ParseError("%s input %s.Complete string with special characters are not allowed in variable name field",feature,str);
}

/* Parsing function. */
static int ByteExtractParse(ByteExtractData *data, char *args)
{
    char *args_copy = SnortStrdup(args);
    char *endptr, *saveptr = args_copy;
    char *token = strtok_r(args_copy, ",", &saveptr);
    RuleOptByteOrderFunc tmp_byte_order_func = NULL;

    /* set defaults / sentinels */
    data->multiplier = 1;
    data->endianess = ENDIAN_NONE;

    /* first: bytes_to_extract */
    if (token)
    {
        data->bytes_to_grab = SnortStrtoul(token, &endptr, 10);
        if (*endptr != '\0')
            ParseError("byte_extract rule option has non-digits in the "
                    "\"bytes_to_extract\" field.");
        token = strtok_r(NULL, ",", &saveptr);
    }

    /* second: offset */
    if (token)
    {
        if (isdigit(token[0]) || token[0] == '-')
        {
             data->offset = SnortStrtoul(token, &endptr, 10);
             if (*endptr != '\0')
                 ParseError("byte_extract rule option has non-digits in the "
                        "\"offset\" field.");
        }
        else
        {
            if ( bytemath_variable_name  && (!strcmp(bytemath_variable_name,token)))
            {
                    data->offset = (int32_t) bytemath_variable;
                    Byte_Extract_Offset_Var = BYTE_MATH_VAR_INDEX;
            }
            else
            {
                  ParseError("byte_extract rule option has invalid Variable name in the "
                        "\"offset\" field.");
            }
        }
        token = strtok_r(NULL, ",", &saveptr);
    }

    /* third: variable name */
    if (token)
    {
        data->name = SnortStrdup(token);
        isvalidstr(token,"byte_extract");
        token = strtok_r(NULL, ",", &saveptr);
    }

    /* optional arguments */
    while (token)
    {
        while(isspace((int)*token)) {token++;}
        if (strcmp(token, "relative") == 0)
        {
            data->relative_flag = 1;
        }

        else if (strncmp(token, "align ", 6) == 0)
        {
            char *value = (token+6);

            if (data->align == 0)
                data->align = (uint8_t)SnortStrtoul(value, &endptr, 10);
            else
                ParseError("byte_extract rule option includes the "
                        "\"align\" argument twice.");

            if (*endptr != '\0')
                ParseError("byte_extract rule option has non-digits in the "
                        "argument to \"align\". ");
        }

        else if (strcmp(token, "little") == 0)
        {
            if (data->endianess == ENDIAN_NONE)
                data->endianess = LITTLE;
            else
                ParseError("byte_extract rule option specifies the "
                        "byte order twice. Use only one of \"big\", \"little\", "
                        "or \"dce\".");
        }

        else if (strcmp(token, "big") == 0)
        {
            if (data->endianess == ENDIAN_NONE)
                data->endianess = BIG;
            else
                ParseError("byte_extract rule option specifies the "
                        "byte order twice. Use only one of \"big\", \"little\", "
                        "or \"dce\".");
        }

        else if (strncmp(token, "multiplier ", 11) == 0)
        {
            char *value = (token+11);
            if (value[0] == '\0')
                ParseError("byte_extract rule option has a \"multiplier\" "
                        "argument with no value specified.");

            if (data->multiplier == 1)
            {
                data->multiplier = SnortStrtoul(value, &endptr, 10);

                if (*endptr != '\0')
                    ParseError("byte_extract rule option has non-digits in the "
                            "argument to \"multiplier\". ");
            }
            else
                ParseError("byte_extract rule option has multiple "
                        "\"multiplier\" arguments. Use only one.");
        }

        else if (strcmp(token, "string") == 0)
        {
            if (data->data_string_convert_flag == 0)
                data->data_string_convert_flag = 1;
            else
                ParseError("byte_extract rule option has multiple "
                        "\"string\" arguments. Use only one.");
        }

        else if (strcmp(token, "dec") == 0)
        {
            if (data->base == 0)
                data->base = 10;
            else
                ParseError("byte_extract rule option has multiple arguments "
                        "specifying the type of string conversion. Use only "
                        "one of \"dec\", \"hex\", or \"oct\".");
        }

        else if (strcmp(token, "hex") == 0)
        {
            if (data->base == 0)
                data->base = 16;
            else
                ParseError("byte_extract rule option has multiple arguments "
                        "specifying the type of string conversion. Use only "
                        "one of \"dec\", \"hex\", or \"oct\".");
        }

        else if (strcmp(token, "oct") == 0)
        {
            if (data->base == 0)
                data->base = 8;
            else
                ParseError("byte_extract rule option has multiple arguments "
                        "specifying the type of string conversion. Use only "
                        "one of \"dec\", \"hex\", or \"oct\".");
        }

        else if ((tmp_byte_order_func = GetByteOrderFunc(token)) != NULL)
        {
            if (data->endianess == ENDIAN_NONE)
            {
                data->endianess = ENDIAN_FUNC;
                data->byte_order_func = tmp_byte_order_func;
            }
            else
            {
                ParseError("byte_extract rule option specifies the "
                        "byte order twice. Use only one of \"big\", \"little\", "
                        "or \"dce\".");
            }
        }

        else if(strncasecmp(token,"bitmask ",8) == 0)
        {
            RuleOptionBitmaskParse(&(data->bitmask_val) , token, data->bytes_to_grab, "BYTE_EXTRACT" );
        }

        else
        {
            ParseError("byte_extract rule option has invalid argument \"%s\".", token);
        }

        token = strtok_r(NULL, ",", &saveptr);
    }

    free(args_copy);

    /* Need to check this error before the sentinel gets replaced */
    if (data->endianess != ENDIAN_NONE && data->data_string_convert_flag == 1)
    {
        ParseError("byte_extract rule option can't have \"string\" specified "
            "at the same time as a byte order (\"big\" or \"little\").");
    }

    /* Replace sentinels with defaults */
    if (data->endianess == ENDIAN_NONE)
        data->endianess = BIG;

    if (data->data_string_convert_flag && (data->base == 0))
        data->base = 10;

    /* At this point you could verify the data and return something. */
    return ByteExtractVerify(data);
}

/* Given a variable name, retrieve its index. For use by other options. */
int8_t GetVarByName(char *name)
{
    int i;

    if (name == NULL)
        return BYTE_EXTRACT_NO_VAR;

    for (i = 0; i < NUM_BYTE_EXTRACT_VARS; i++)
    {
        if (variable_names[i] != NULL && strcmp(variable_names[i], name) == 0)
            return i;
    }

    return BYTE_EXTRACT_NO_VAR;
}

/* If given an OptFpList with no byte_extracts, clear the variable_names array */
void ClearVarNames(OptFpList *fpl)
{
    int i;

    while (fpl != NULL)
    {
        if (fpl->type == RULE_OPTION_TYPE_BYTE_EXTRACT)
            return;

        fpl = fpl->next;
    }

    for (i = 0; i < NUM_BYTE_EXTRACT_VARS; i++)
    {
        free(variable_names[i]);
        variable_names[i] = NULL;
    }
}

/* Add a variable's name to the variable_names array
   Returns: variable index
*/
int8_t AddVarNameToList(ByteExtractData *data)
{
    int i;

    for (i = 0; i < NUM_BYTE_EXTRACT_VARS; i++)
    {
        if (variable_names[i] == NULL)
        {
            variable_names[i] = SnortStrdup(data->name);
            break;
        }

        else if ( strcmp(variable_names[i], data->name) == 0 )
        {
            break;
        }
    }

    return i;
}


/* Inititialization function. Handles rule parsing. */
static void ByteExtractInit(struct _SnortConfig *sc, char *data, OptTreeNode *otn, int protocol)
{
    ByteExtractData *idx;
    OptFpList *fpl;
    void *idx_dup;

    idx = (ByteExtractData *) SnortAlloc(sizeof(ByteExtractData));

    /* Clear out the variable_names array if this is the first byte_extract in a rule. */
    ClearVarNames(otn->opt_func);

    /* Parse the options */
    ByteExtractParse(idx, data);

    /* There can only be two unique variables names in a rule. */
    idx->var_number = AddVarNameToList(idx);
    if (idx->var_number >= NUM_BYTE_EXTRACT_VARS)
    {
        ParseError("Rule has more than %d byte_extract variables.", NUM_BYTE_EXTRACT_VARS);
    }
#ifdef DEBUG_MSGS
    PrintByteExtract(idx);
#endif

    fpl = AddOptFuncToList(DetectByteExtract, otn);
    fpl->type = RULE_OPTION_TYPE_BYTE_EXTRACT;
    if (add_detection_option(sc, RULE_OPTION_TYPE_BYTE_EXTRACT, (void *)idx, &idx_dup) == DETECTION_OPTION_EQUAL)
    {
        /* duplicate exists. */
        free(idx->name);
        free(idx);
        idx = idx_dup;
    }

    fpl->context = (void *) idx;

    if (idx->relative_flag == 1)
        fpl->isRelative = 1;
}

uint32_t getNumberTailingZerosInBitmask(uint32_t bitmask)
{
   unsigned int num_tailing_zeros;

   if (bitmask & 0x1)
   {
       num_tailing_zeros = 0;
   }
   else
   {
       num_tailing_zeros = 1;
       if ((bitmask & 0xffff) == 0)
       {
            bitmask >>= 16;
            num_tailing_zeros += 16;
       }
       if ((bitmask & 0xff) == 0)
       {
            bitmask >>= 8;
            num_tailing_zeros += 8;
       }
       if ((bitmask & 0xf) == 0)
       {
             bitmask >>= 4;
             num_tailing_zeros += 4;
       }
       if ((bitmask & 0x3) == 0)
       {
             bitmask >>= 2;
             num_tailing_zeros += 2;
       }
       num_tailing_zeros -= bitmask & 0x1;
   }

   return num_tailing_zeros;
}

/* Main detection callback */
int DetectByteExtract(void *option_data, Packet *p)
{
    ByteExtractData *data = (ByteExtractData *) option_data;
    int ret, bytes_read, dsize;
    const uint8_t *ptr, *start, *end;
    uint32_t *value;
    int32_t offset;
    uint8_t rst_doe_flags = 1;
    PROFILE_VARS;

    PREPROC_PROFILE_START(byteExtractPerfStats);

    if (data == NULL || p == NULL)
    {
        PREPROC_PROFILE_END(byteExtractPerfStats);
        return DETECTION_OPTION_NO_MATCH;
    }

    /* setup our fun pointers */
    if (Is_DetectFlag(FLAG_ALT_DETECT))
    {
        dsize = DetectBuffer.len;
        start = DetectBuffer.data;
    }
    else if (Is_DetectFlag(FLAG_ALT_DECODE))
    {
        dsize = DecodeBuffer.len;
        start = DecodeBuffer.data;
    }
    else
    {
        if(IsLimitedDetect(p))
            dsize = p->alt_dsize;
        else
            dsize = p->dsize;
        start = p->data;
    }

    if (data->relative_flag)
    {
        ptr = doe_ptr;
        rst_doe_flags = 0;
    }
    else
        ptr = start;

    ptr += data->offset;
    end = start + dsize;
    value = &(extracted_values[data->var_number]);

    /* check bounds */
    if (ptr < start || ptr >= end)
    {
        PREPROC_PROFILE_END(byteExtractPerfStats);
        return DETECTION_OPTION_NO_MATCH;
    }

    /* get the endianess at run-time if we have a byte_order_func */
    if (data->byte_order_func)
    {
        offset = (int32_t) (ptr - start);
        data->endianess = data->byte_order_func((void *)p, offset);
    }
    if (data->endianess == -1)
    {
        /* Sometimes the byte_order_func deems that the packet should be skipped */
        PREPROC_PROFILE_END(byteExtractPerfStats);
        return DETECTION_OPTION_NO_MATCH;
    }

    if (data->data_string_convert_flag == 0)
    {
        ret = byte_extract(data->endianess, data->bytes_to_grab, ptr, start, end, value);
        if (ret < 0)
        {
            PREPROC_PROFILE_END(byteExtractPerfStats);
            return DETECTION_OPTION_NO_MATCH;
        }
        bytes_read = data->bytes_to_grab;
    }
    else
    {
        ret = string_extract(data->bytes_to_grab, data->base, ptr, start, end, value);
        if (ret < 0)
        {
             PREPROC_PROFILE_END(byteExtractPerfStats);
             return DETECTION_OPTION_NO_MATCH;
        }
        bytes_read = ret;
    }

    if(data->bitmask_val != 0 )
    {
        int num_tailing_zeros_bitmask = getNumberTailingZerosInBitmask(data->bitmask_val);
        *value = (*value) & data->bitmask_val ;
         if ( (*value) && num_tailing_zeros_bitmask )
         {
            *value = (*value) >> num_tailing_zeros_bitmask;
         }
    }

    /* mulitply */
    *value *= data->multiplier;

    /* align to next 32-bit or 16-bit boundary */
    if ((data->align == 4) && (*value % 4))
    {
        *value = *value + 4 - (*value % 4);
    }
    else if ((data->align == 2) && (*value % 2))
    {
        *value = *value + 2 - (*value % 2);
    }

    /* push doe_ptr */
    UpdateDoePtr((ptr + bytes_read), rst_doe_flags);

    /* this rule option always "matches" if the read is performed correctly */
    PREPROC_PROFILE_END(byteExtractPerfStats);
    common_var = *value;
    return DETECTION_OPTION_MATCH;
}

/* Setters & Getters for extracted values */
int GetByteExtractValue(uint32_t *dst, int8_t var_number)
{
    if (dst == NULL || var_number >= NUM_BYTE_EXTRACT_VARS)
        return BYTE_EXTRACT_NO_VAR;

    *dst = extracted_values[var_number];
    return 0;
}

int SetByteExtractValue(uint32_t value, int8_t var_number)
{
    if (var_number >= NUM_BYTE_EXTRACT_VARS)
        return BYTE_EXTRACT_NO_VAR;

    extracted_values[var_number] = value;

    return 0;
}
