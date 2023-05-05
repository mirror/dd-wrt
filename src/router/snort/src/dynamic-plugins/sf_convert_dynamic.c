/* $Id$ */
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_engine/sf_snort_plugin_api.h"
#include "detection_options.h"
#include "rules.h"
#include "treenodes.h"
#include "plugbase.h"

#include "sf_convert_dynamic.h"

#include "sp_asn1_detect.h"
#include "sp_byte_check.h"
#include "sp_byte_math.h"
#include "sp_byte_jump.h"
#include "sp_byte_extract.h"
#include "sp_clientserver.h"
#include "sp_flowbits.h"
#include "sp_isdataat.h"
#include "sp_pattern_match.h"
#include "sp_pcre.h"
#include "sp_hdr_opt_wrap.h"
#include "sp_file_data.h"
#include "sp_pkt_data.h"
#include "sp_base64_decode.h"
#include "sp_base64_data.h"
#include "sp_preprocopt.h"

extern void ParsePattern(char *, OptTreeNode *, int);
extern void ParseProtectedPattern(char *, OptTreeNode *, int);
extern void *pcreCompile(const char *pattern, int options, const char **errptr,
    int *erroffset, const unsigned char *tableptr);
extern void *pcreStudy(struct _SnortConfig *sc, const void *code, int options, const char **errptr);

extern int SnortPcre(void *option_data, Packet *p);
extern int FlowBitsCheck(void *option_data, Packet *p);
extern int CheckFlow(void *option_data, Packet *p);
extern int Asn1Detect(void *option_data, Packet *p);
extern int ByteTest(void *option_data, Packet *p);
extern int ByteMath(void *option_data, Packet *p);
extern int ByteJump(void *option_data, Packet *p);
extern int IsDataAt(void *option_data, Packet *p);
extern int FileDataEval(void *option_data, Packet *p);
extern int PktDataEval(void *option_data, Packet *p);
extern int Base64DataEval(void *option_data, Packet *p);
extern int Base64DecodeEval(void *option_data, Packet *p) ;

static int CheckConvertability(Rule *rule, OptTreeNode *otn);
static int ConvertPreprocessorOption(SnortConfig *, Rule *rule, int index, OptTreeNode *otn);
static int ConvertContentOption(SnortConfig *, Rule *rule, int index, OptTreeNode *otn);
static int ConvertProtectedContentOption(SnortConfig *, Rule *rule, int index, OptTreeNode *otn);
static int ConvertPcreOption(SnortConfig *, Rule *rule, int index, OptTreeNode *otn);
static int ConvertFlowbitOption(SnortConfig *, Rule *rule, int index, OptTreeNode *otn);
static int ConvertFlowflagsOption(SnortConfig *, Rule *rule, int index, OptTreeNode *otn);
static int ConvertAsn1Option(SnortConfig *, Rule *rule, int index, OptTreeNode *otn);
static int ConvertCursorOption(SnortConfig *, Rule *rule, int index, OptTreeNode *otn);
static int ConvertHdrCheckOption(SnortConfig *, Rule *rule, int index, OptTreeNode *otn);
static int ConvertByteTestOption(SnortConfig *, Rule *rule, int index, OptTreeNode *otn);
static int ConvertByteMathOption(SnortConfig *, Rule *rule, int index, OptTreeNode *otn);
static int ConvertByteJumpOption(SnortConfig *, Rule *rule, int index, OptTreeNode *otn);
static int ConvertByteExtractOption(SnortConfig *, Rule *rule, int index, OptTreeNode *otn);
static int ConvertSetCursorOption(SnortConfig *, Rule *rule, int index, OptTreeNode *otn);
static int ConvertLoopOption(SnortConfig *, Rule *rule, int index, OptTreeNode *otn);
static int ConvertFileDataOption(SnortConfig *, Rule *rule, int index, OptTreeNode *otn);
static int ConvertPktDataOption(SnortConfig *, Rule *rule, int index, OptTreeNode *otn);
static int ConvertBase64DataOption(SnortConfig *, Rule *rule, int index, OptTreeNode *otn);
static int ConvertBase64DecodeOption(SnortConfig *, Rule *rule, int index, OptTreeNode *otn);

/* Use an array of callbacks to handle varying option types
 *
 * NOTE:  These MUST align with the values in DynamicOptionType enumeration
 * found in sf_dynamic_define.h
 */
static int (* OptionConverterArray[OPTION_TYPE_MAX])
    (SnortConfig *, Rule *rule, int index, OptTreeNode *otn) =
{
    ConvertPreprocessorOption,
    ConvertContentOption,
    ConvertProtectedContentOption,
    ConvertPcreOption,
    ConvertFlowbitOption,
    ConvertFlowflagsOption,
    ConvertAsn1Option,
    ConvertCursorOption,
    ConvertHdrCheckOption,
    ConvertByteTestOption,
    ConvertByteJumpOption,
    ConvertByteExtractOption,
    ConvertSetCursorOption,
    ConvertLoopOption,
    ConvertFileDataOption,
    ConvertPktDataOption,
    ConvertBase64DataOption,
    ConvertBase64DecodeOption,
    ConvertByteMathOption
};

/* Convert a dynamic rule to native rule structure. */
/* Return -1 on error, 0 if nothing done, 1 on success. */
int ConvertDynamicRule(SnortConfig *sc, Rule *rule, OptTreeNode *otn)
{
    unsigned int i;
    tSfPolicyId policyId = 0;
    RuleTreeNode *rtn = NULL;

    if (CheckConvertability(rule, otn) < 0)
    {
        return -1;
    }

    for (i = 0; i < rule->numOptions; i++) // Iterate through option list
    {
        int ret;
        int optype = rule->options[i]->optionType;
        if (optype < OPTION_TYPE_PREPROCESSOR || optype >= OPTION_TYPE_MAX)
            return -1; // Invalid option type

        ret = OptionConverterArray[optype](sc, rule, i, otn);
        if (ret < 0)
            return -1;
    }

    if(otn->proto_nodes)
    {

         for (policyId = 0;
                 policyId < otn->proto_node_num;policyId++)
         {
             rtn = otn->proto_nodes[policyId];
             if (!rtn)
             {
                 continue;
             }

             setParserPolicy(sc, policyId);

            FinalizeContentUniqueness(sc, otn);
         }
    }
    otn->sigInfo.shared = 0;

    return 1;
}

/* A text-rule byte_extract option can only have NUM_BYTE_EXTRACT_VARS unique
   variables. This function iterates through a Rule and counts the unique names. */
static inline int CheckByteExtractVars(Rule *rule)
{
    unsigned int i, j, unique_names = 0;
    char *names[NUM_BYTE_EXTRACT_VARS];

    for (i = 0; i < rule->numOptions; i++)
    {
        ByteExtract *data;
        int unique_name = 1;

        /* Only need byte_extract options */
        if (rule->options[i]->optionType != OPTION_TYPE_BYTE_EXTRACT)
            continue;

        /* Check name against other unique names */
        data = rule->options[i]->option_u.byteExtract;
        for (j = 0; j < unique_names; j++)
        {
            if (strcmp(names[j], data->refId) == 0)
            {
                unique_name = 0;
                break;
            }
        }

        /* Add unique names to the array */
        if (unique_name)
        {
            if (unique_names == NUM_BYTE_EXTRACT_VARS)
                return -1; /* Too many variables! */

            names[unique_names] = data->refId;
            unique_names++;
        }
    }

    return 0;
}

static int CheckConvertability(Rule *rule, OptTreeNode *otn)
{
    /* We need to check for any conversion problems up-front. That way,
     * a rule won't get partially added to the tree. */
    unsigned int i;

    if (!rule || !otn)
        return -1;

    /* Custom detection function */
    if (rule->evalFunc)
        return -1;

    for (i = 0; i < rule->numOptions; i++)
    {
        int optype = rule->options[i]->optionType;

        /* Invalid option type */
        if (optype < OPTION_TYPE_PREPROCESSOR || optype >= OPTION_TYPE_MAX)
            return -1;

        switch (optype)
        {
            /* Option types not supported for conversion */
            case OPTION_TYPE_SET_CURSOR:
            case OPTION_TYPE_LOOP:
                return -1;
        }
    }

    /* Check for too many byte_extract variables. These can't be converted
       because the detection plugin only supports a specific number per rule. */
    if (CheckByteExtractVars(rule) < 0)
        return -1;

    /* We're good! */
    return 1;
}

/* Option-converting functions */
static int ConvertPreprocessorOption(SnortConfig *sc, Rule *rule, int index, OptTreeNode *otn)
{
    PreprocessorOption *preprocOpt = rule->options[index]->option_u.preprocOpt;
    PreprocessorOptionInfo *preprocOptInfo = SnortAlloc(sizeof(PreprocessorOptionInfo));
    OptFpList *fpl;
    void *option_dup;

    preprocOptInfo->optionInit = preprocOpt->optionInit;
    preprocOptInfo->optionEval = preprocOpt->optionEval;
    preprocOptInfo->optionFpFunc = preprocOpt->optionFpFunc;
    preprocOptInfo->data = preprocOpt->dataPtr;

    /* FreeOneRule() already calls the cleanup function. Left NULL here
       to avoid a double-free. */
    preprocOptInfo->optionCleanup = NULL;

    preprocOptInfo->optionHash = NULL;
    preprocOptInfo->optionKeyCompare = NULL;
    preprocOptInfo->otnHandler = NULL;

    //  Add to option chain with generic callback
    fpl = AddOptFuncToList(PreprocessorOptionFunc, otn);

    /*
     * attach custom info to the context node so that we can call each instance
     * individually
     */
    fpl->context = (void *) preprocOptInfo;

    if (add_detection_option(sc, RULE_OPTION_TYPE_PREPROCESSOR,
                             (void *)preprocOptInfo, &option_dup) == DETECTION_OPTION_EQUAL)
    {
        PreprocessorRuleOptionsFreeFunc(preprocOptInfo);
        fpl->context = preprocOptInfo = option_dup;
    }
    fpl->type = RULE_OPTION_TYPE_PREPROCESSOR;

    return 1;
}

static int ConvertContentOption(SnortConfig *sc, Rule *rule, int index, OptTreeNode *otn)
{
    ContentInfo *content = rule->options[index]->option_u.content;
    PatternMatchData *pmd = NULL;
    OptFpList *fpl;
    char *pattern;
    unsigned int pattern_size, i;

    /* ParsePattern expects quotations marks around the pattern. */
    if (content->pattern[0] != '"')
    {
        pattern_size = strlen((const char*)content->pattern) + 3;
        pattern = SnortAlloc(sizeof(char) * pattern_size);
        pattern[0] = '"';
        memcpy(pattern+1, content->pattern, pattern_size-3);
        pattern[pattern_size-2] = '"';
    }
    else
    {
        pattern = (char*)content->pattern;
    }

    /* Allocate a new node, based on the type of content option. */
    if ( HTTP_CONTENT(content->flags) )
    {
        pmd = NewNode(otn, PLUGIN_PATTERN_MATCH_URI);
        ParsePattern(pattern, otn, PLUGIN_PATTERN_MATCH_URI);
        fpl = AddOptFuncToList(CheckUriPatternMatch, otn);
        fpl->type = RULE_OPTION_TYPE_CONTENT_URI;
        pmd->buffer_func = CHECK_URI_PATTERN_MATCH;
    }
    else
    {
        pmd = NewNode(otn, PLUGIN_PATTERN_MATCH);
        ParsePattern(pattern, otn, PLUGIN_PATTERN_MATCH);
        fpl = AddOptFuncToList(CheckANDPatternMatch, otn);
        fpl->type = RULE_OPTION_TYPE_CONTENT;
        pmd->buffer_func = CHECK_AND_PATTERN_MATCH;
    }

    /* Initialize var numbers */
    if (content->flags & CONTENT_RELATIVE)
    {
        pmd->distance_var = GetVarByName(content->offset_refId);
        pmd->within_var = GetVarByName(content->depth_refId);
        pmd->offset_var = -1;
        pmd->depth_var = -1;
    }
    else
    {
        pmd->offset_var = GetVarByName(content->offset_refId);
        pmd->depth_var = GetVarByName(content->depth_refId);
        pmd->distance_var = -1;
        pmd->within_var = -1;
    }

    /* Set URI buffer flags */
    pmd->http_buffer = HTTP_CONTENT(content->flags);

    if (content->flags & CONTENT_BUF_RAW)
    {
        pmd->rawbytes = 1;
    }

    /* Handle options */
    if (content->flags & CONTENT_NOCASE)
    {
        pmd->nocase = 1;
        for (i = 0; i < pmd->pattern_size; i++)
        {
            pmd->pattern_buf[i] = toupper(pmd->pattern_buf[i]);
        }
        make_precomp(pmd);
        pmd->search = uniSearchCI;
    }
    if (content->flags & CONTENT_RELATIVE)
    {
        pmd->distance = content->offset;
        pmd->within = (content->depth) ? content->depth : PMD_WITHIN_UNDEFINED;
        pmd->use_doe = 1;
        fpl->isRelative = 1;
    }
    else
    {
        pmd->offset = content->offset;
        pmd->depth = content->depth;
    }

    if (content->flags & CONTENT_FAST_PATTERN)
        pmd->fp = 1;

    /* Fast pattern only and specifying an offset and length are
     * technically mutually exclusive - see
     * detection-plugins/sp_pattern_match.c */
    if (content->flags & CONTENT_FAST_PATTERN_ONLY)
    {
        pmd->fp_only = 1;
    }
    else
    {
        pmd->fp_offset = content->fp_offset;
        pmd->fp_length = content->fp_length;
    }

    if (content->flags & NOT_FLAG)
        pmd->exception_flag = 1;

    fpl->context = pmd;
    pmd->fpl = fpl;

    if (pattern != (char *)content->pattern)
        free(pattern);
    return 1;
}

static int ConvertProtectedContentOption(SnortConfig *sc, Rule *rule, int index, OptTreeNode *otn)
{
    ProtectedContentInfo *content = rule->options[index]->option_u.protectedContent;
    PatternMatchData *pmd = NULL;
    OptFpList *fpl;
    char *pattern;
    unsigned int pattern_size;

    /* ParseProtectedPattern expects quotations marks around the pattern. */
    if (content->pattern[0] != '"')
    {
        pattern_size = strlen((const char*)content->pattern) + 3;
        pattern = SnortAlloc(sizeof(char) * pattern_size);
        pattern[0] = '"';
        memcpy(pattern+1, content->pattern, pattern_size-3);
        pattern[pattern_size-2] = '"';
    }
    else
    {
        pattern = (char*)content->pattern;
    }

    /* Allocate a new node, based on the type of content option. */
    if ( HTTP_CONTENT(content->flags) )
    {
        pmd = NewNode(otn, PLUGIN_PATTERN_MATCH_URI);
        ParseProtectedPattern(pattern, otn, PLUGIN_PATTERN_MATCH_URI);
        fpl = AddOptFuncToList(CheckUriPatternMatch, otn);
        fpl->type = RULE_OPTION_TYPE_CONTENT_URI;
        pmd->buffer_func = CHECK_URI_PATTERN_MATCH;
    }
    else
    {
        pmd = NewNode(otn, PLUGIN_PATTERN_MATCH);
        ParseProtectedPattern(pattern, otn, PLUGIN_PATTERN_MATCH);
        fpl = AddOptFuncToList(CheckANDPatternMatch, otn);
        fpl->type = RULE_OPTION_TYPE_CONTENT;
        pmd->buffer_func = CHECK_AND_PATTERN_MATCH;
    }

    pmd->protected_pattern = true;
    pmd->protected_length = content->protected_length;

    if (pattern != (char *)content->pattern)
        free(pattern);
    switch( content->hash_type )
    {
        case( PROTECTED_CONTENT_HASH_MD5 ):
            {
                pmd->pattern_type = SECHASH_MD5;
                break;
            }
        case( PROTECTED_CONTENT_HASH_SHA256 ):
            {
                pmd->pattern_type = SECHASH_SHA256;
                break;
            }
        case( PROTECTED_CONTENT_HASH_SHA512 ):
            {
                pmd->pattern_type = SECHASH_SHA512;
                break;
            }
        default:
            return( 0 );
    }

    /* Initialize var numbers */
    if (content->flags & CONTENT_RELATIVE)
    {
        pmd->distance_var = GetVarByName(content->offset_refId);
        pmd->within_var = -1;
        pmd->offset_var = -1;
        pmd->depth_var = -1;
    }
    else
    {
        pmd->offset_var = GetVarByName(content->offset_refId);
        pmd->depth_var = -1;
        pmd->distance_var = -1;
        pmd->within_var = -1;
    }

    /* Set URI buffer flags */
    pmd->http_buffer = HTTP_CONTENT(content->flags);

    if (content->flags & CONTENT_BUF_RAW)
    {
        pmd->rawbytes = 1;
    }

    /* Handle options */
    if (content->flags & CONTENT_RELATIVE)
    {
        pmd->distance = content->offset;
        pmd->use_doe = 1;
        fpl->isRelative = 1;
    }
    else
    {
        pmd->offset = content->offset;
    }

    if (content->flags & NOT_FLAG)
        pmd->exception_flag = 1;

    fpl->context = pmd;
    pmd->fpl = fpl;


    return 1;
}

static int ConvertPcreOption(SnortConfig *sc, Rule *rule, int index, OptTreeNode *otn)
{
    PcreData *pcre_data = (PcreData *) SnortAlloc(sizeof(PcreData));
    PCREInfo *pcre_info = rule->options[index]->option_u.pcre;
    OptFpList *fpl;
    void *pcre_dup;
    const char *error;
    int erroroffset;

    /* Need to recompile the expression so double free doesn't occur
     * during reload */

    /* Compile & Study PCRE */
    pcre_data->re = pcreCompile(
        pcre_info->expr,
        pcre_info->compile_flags,
        &error,
        &erroroffset,
        NULL
        );

    if (pcre_data->re == NULL)
    {
        free(pcre_data);
        return -1;
    }

    pcre_data->pe = pcreStudy(sc,
        pcre_data->re,
        pcre_info->compile_flags,
        &error
        );

    if (error)
    {
        free(pcre_data->re);
        free(pcre_data);
        return -1;
    }

    /* Copy to struct used for normal PCRE rules */
    pcre_data->expression = SnortStrdup(pcre_info->expr);

    /* Option values differ between PCREInfo and PcreData,
     * so a straight copy of the options variable won't work. */
    if (pcre_info->flags & CONTENT_RELATIVE)
        pcre_data->options |= SNORT_PCRE_RELATIVE;

    if (pcre_info->flags & NOT_FLAG)
        pcre_data->options |= SNORT_PCRE_INVERT;

    if (pcre_info->flags & CONTENT_BUF_RAW)
        pcre_data->options |= SNORT_PCRE_RAWBYTES;

    if (pcre_info->flags & CONTENT_BUF_NORMALIZED)
        pcre_data->options &= ~SNORT_PCRE_RAWBYTES;

    pcre_data->options |= HTTP_CONTENT(pcre_info->flags);

    PcreCheckAnchored(pcre_data);

    /* Attach option to tree, checking for duplicate */
    otn->pcre_flag = 1;
    fpl = AddOptFuncToList(SnortPcre, otn);
    fpl->type = RULE_OPTION_TYPE_PCRE;

    if (add_detection_option(sc, RULE_OPTION_TYPE_PCRE, (void *)pcre_data, &pcre_dup) == DETECTION_OPTION_EQUAL)
    {
        if (pcre_data->expression)
            free(pcre_data->expression);
        if (pcre_data->pe)
            free(pcre_data->pe);
        if (pcre_data->re)
            free(pcre_data->re);

        free(pcre_data);
        pcre_data = pcre_dup;
    }

    fpl->context = (void *)pcre_data;
    if (pcre_data->options & SNORT_PCRE_RELATIVE)
        fpl->isRelative = 1;

    if (otn->ds_list[PLUGIN_PCRE] == NULL)
        otn->ds_list[PLUGIN_PCRE] = (void *)pcre_data;

    return 1;
}

static int ConvertFlowbitOption(SnortConfig *sc, Rule *rule, int index, OptTreeNode *otn)
{
    FlowBitsInfo *flowbitInfo = rule->options[index]->option_u.flowBit;
    FLOWBITS_OP *flowbits = (FLOWBITS_OP *) SnortAlloc(sizeof(FLOWBITS_OP));
    OptFpList *fpl;
    void *idx_dup;

    /* Convert struct for rule option */
    if (!flowbitInfo)
    {
        free(flowbits);
        return 1;
    }

    flowbits->type = flowbitInfo->operation;
    processFlowBitsWithGroup(flowbitInfo->flowBitsName, flowbitInfo->groupName, flowbits);

    /* Add detection option to hash table */
    otn->ds_list[PLUGIN_FLOWBIT] = (void *)1;
    if (add_detection_option(sc, RULE_OPTION_TYPE_FLOWBIT, (void *)flowbits, &idx_dup) == DETECTION_OPTION_EQUAL)
    {
        if (flowbits->name)
            free(flowbits->name);

        if (flowbits->ids)
            free(flowbits->ids);

        free(flowbits);
        flowbits = idx_dup;
    }

    /* Add detection function to otn */
    fpl = AddOptFuncToList(FlowBitsCheck, otn);
    fpl->type = RULE_OPTION_TYPE_FLOWBIT;
    fpl->context = (void *) flowbits;
    return 1;
}

static int ConvertFlowflagsOption(SnortConfig *sc, Rule *rule, int index, OptTreeNode *otn)
{
    FlowFlags *flow = rule->options[index]->option_u.flowFlags;
    ClientServerData *csdata = SnortAlloc(sizeof(ClientServerData));
    OptFpList *fpl;
    void *dup;

    if (flow->flags & FLOW_FR_SERVER)
        csdata->from_server = 1;
    if (flow->flags & FLOW_FR_CLIENT)
        csdata->from_client = 1;
    if (flow->flags & FLOW_IGNORE_REASSEMBLED)
        csdata->ignore_reassembled |= IGNORE_STREAM;
    if (flow->flags & FLOW_ONLY_REASSEMBLED)
        csdata->only_reassembled |= ONLY_STREAM;
    if (flow->flags & FLOW_ESTABLISHED)
        csdata->established = 1;
    csdata->stateless = 0;
    csdata->unestablished = 0;

    otn->established = csdata->established;
    otn->stateless = 0;
    otn->unestablished = 0;

    if (add_detection_option(sc, RULE_OPTION_TYPE_FLOW, (void *)csdata, &dup) == DETECTION_OPTION_EQUAL)
    {
        free(csdata);
        csdata = dup;
    }

    fpl = AddOptFuncToList(CheckFlow, otn);
    fpl->type = RULE_OPTION_TYPE_FLOW;
    fpl->context = (void *)csdata;
    otn->ds_list[PLUGIN_CLIENTSERVER] = (void *)csdata;

    return 1;
}

static int ConvertAsn1Option(SnortConfig *sc, Rule *rule, int index, OptTreeNode *otn)
{
    Asn1Context *asn1_dynamic = rule->options[index]->option_u.asn1;
    ASN1_CTXT *asn1 = (ASN1_CTXT *) SnortAlloc(sizeof(ASN1_CTXT));
    OptFpList *fpl;
    void *dup;

    /* Copy over options */
    asn1->bs_overflow = asn1_dynamic->bs_overflow;
    asn1->double_overflow = asn1_dynamic->double_overflow;
    asn1->print = asn1_dynamic->print;
    asn1->length = asn1_dynamic->length;
    asn1->max_length = asn1_dynamic->max_length;
    asn1->offset = asn1_dynamic->offset;
    asn1->offset_type = asn1_dynamic->offset_type;

    if (add_detection_option(sc, RULE_OPTION_TYPE_ASN1, (void *)asn1, &dup) == DETECTION_OPTION_EQUAL)
    {
        free(asn1);
        asn1 = dup;
    }

    fpl = AddOptFuncToList(Asn1Detect, otn);
    fpl->context = (void *)asn1;
    fpl->type = RULE_OPTION_TYPE_ASN1;

    if (asn1->offset_type == REL_OFFSET)
        fpl->isRelative = 1;

    return 1;
}

static int ConvertCursorOption(SnortConfig *sc, Rule *rule, int index, OptTreeNode *otn)
{
    CursorInfo *cursor = rule->options[index]->option_u.cursor;
    IsDataAtData *data = (IsDataAtData *) SnortAlloc(sizeof(IsDataAtData));
    OptFpList *fpl;
    void *dup;

    data->offset = cursor->offset;
    data->offset_var = GetVarByName(cursor->offset_refId);
    if (cursor->flags & CONTENT_RELATIVE)
        data->flags |= ISDATAAT_RELATIVE_FLAG;
    if (cursor->flags & CONTENT_BUF_RAW)
        data->flags |= ISDATAAT_RAWBYTES_FLAG;
    if (cursor->flags & NOT_FLAG)
        data->flags |= ISDATAAT_NOT_FLAG;

    if (add_detection_option(sc, RULE_OPTION_TYPE_IS_DATA_AT, (void *)data, &dup) == DETECTION_OPTION_EQUAL)
    {
        free(data);
        data = dup;
    }

    fpl = AddOptFuncToList(IsDataAt, otn);
    fpl->context = (void *)data;
    fpl->type = RULE_OPTION_TYPE_IS_DATA_AT;

    if (cursor->flags & CONTENT_RELATIVE)
        fpl->isRelative = 1;

    return 1;
}

static int ConvertHdrCheckOption(SnortConfig *sc, Rule *rule, int index, OptTreeNode *otn)
{
    HdrOptCheck *hdrData = rule->options[index]->option_u.hdrData;
    OptFpList *fpl;
    void *dup;

    /* This option type gets added directly to the tree. */
    if (add_detection_option(sc, RULE_OPTION_TYPE_HDR_OPT_CHECK, (void *)hdrData, &dup) == DETECTION_OPTION_EQUAL)
    {
        hdrData = dup;
    }

    fpl = AddOptFuncToList(HdrOptEval, otn);
    fpl->context = hdrData;
    fpl->type = RULE_OPTION_TYPE_HDR_OPT_CHECK;
    return 1;
}

static int ConvertByteTestOption(SnortConfig *sc, Rule *rule, int index, OptTreeNode *otn)
{
    ByteData *byte = rule->options[index]->option_u.byte;
    ByteTestData *byte_test = (ByteTestData *) SnortAlloc(sizeof(ByteTestData));
    OptFpList *fpl;
    void *idx_dup;

    byte_test->bytes_to_compare = byte->bytes;
    byte_test->cmp_value = byte->value;
    byte_test->cmp_value_var = GetVarByName_check(byte->value_refId);
    byte_test->offset = byte->offset;
    byte_test->offset_var = GetVarByName_check(byte->offset_refId);

    byte_test->bitmask_val = byte->bitmask_val;

    if (byte->flags & NOT_FLAG)
        byte_test->not_flag = 1;

    byte_test->operator = byte->op;
    if (byte->op == CHECK_NEQ)
    {
        byte_test->operator = CHECK_EQ;
        byte_test->not_flag = !(byte_test->not_flag);
    }

    if (byte->flags & CONTENT_RELATIVE)
        byte_test->relative_flag = 1;
    if (byte->flags & EXTRACT_AS_STRING)
        byte_test->data_string_convert_flag = 1;

    if (byte->flags & BYTE_BIG_ENDIAN)
        byte_test->endianess = BIG;
    else
        byte_test->endianess = LITTLE;

    if (byte->flags & EXTRACT_AS_DEC)
        byte_test->base = 10;
    if (byte->flags & EXTRACT_AS_OCT)
        byte_test->base = 8;
    if (byte->flags & EXTRACT_AS_HEX)
        byte_test->base = 16;

    fpl = AddOptFuncToList(ByteTest, otn);
    fpl->type = RULE_OPTION_TYPE_BYTE_TEST;

    if (add_detection_option(sc, RULE_OPTION_TYPE_BYTE_TEST, (void *)byte_test, &idx_dup) == DETECTION_OPTION_EQUAL)
    {
        free(byte_test);
        byte_test = idx_dup;
    }

    fpl->context = (void *)byte_test;

    if (byte_test->relative_flag == 1)
        fpl->isRelative = 1;

    return 1;
}

static int ConvertByteJumpOption(SnortConfig *sc, Rule *rule, int index, OptTreeNode *otn)
{
    ByteData *byte = rule->options[index]->option_u.byte;
    ByteJumpData *byte_jump = SnortAlloc(sizeof(ByteJumpData));
    OptFpList *fpl;
    void *dup;

    byte_jump->bytes_to_grab = byte->bytes;
    byte_jump->offset = byte->offset;
    byte_jump->offset_var = GetVarByName_check(byte->offset_refId);
    byte_jump->multiplier = byte->multiplier;
    byte_jump->post_offset = byte->post_offset;
    byte_jump->postoffset_var = GetVarByName_check(byte->postoffset_refId);

    byte_jump->bitmask_val = byte->bitmask_val;

    if (byte->flags & CONTENT_RELATIVE)
        byte_jump->relative_flag = 1;
    if (byte->flags & EXTRACT_AS_STRING)
        byte_jump->data_string_convert_flag = 1;
    if (byte->flags & JUMP_FROM_BEGINNING)
        byte_jump->from_beginning_flag = 1;
    if (byte->flags & JUMP_FROM_END)
    {
        byte_jump->from_end_flag = 1;
    }
    if (byte->flags & JUMP_ALIGN)
        byte_jump->align_flag = 1;
    if (byte->flags & BYTE_BIG_ENDIAN)
        byte_jump->endianess = BIG;
    else
        byte_jump->endianess = LITTLE;
    if (byte->flags & EXTRACT_AS_HEX)
        byte_jump->base = 16;
    if (byte->flags & EXTRACT_AS_DEC)
        byte_jump->base = 10;
    if (byte->flags & EXTRACT_AS_OCT)
        byte_jump->base = 8;

    fpl = AddOptFuncToList(ByteJump, otn);
    fpl->type = RULE_OPTION_TYPE_BYTE_JUMP;
    if (add_detection_option(sc, RULE_OPTION_TYPE_BYTE_JUMP, (void *)byte_jump, &dup) == DETECTION_OPTION_EQUAL)
    {
        free(byte_jump);
        byte_jump = dup;
    }
    fpl->context = (void *)byte_jump;

    if (byte_jump->relative_flag == 1)
        fpl->isRelative = 1;

    return 1;
}

static int ConvertByteExtractOption(SnortConfig *sc, Rule *rule, int index, OptTreeNode *otn)
{
    ByteExtract *so_byte = rule->options[index]->option_u.byteExtract;
    ByteExtractData *snort_byte = SnortAlloc(sizeof(ByteExtractData));
    OptFpList *fpl;
    void *dup;

    /* Clear out sp_byte_extract.c's variable_names array if this is the first
       byte_extract option in the rule. */
    ClearVarNames(otn->opt_func);

    /* Copy over the various struct members */
    snort_byte->bytes_to_grab = so_byte->bytes;
    snort_byte->offset = so_byte->offset;
    snort_byte->align = so_byte->align;
    snort_byte->name = SnortStrdup(so_byte->refId);

    snort_byte->bitmask_val = so_byte->bitmask_val;

    /* In an SO rule, setting multiplier to 0 means that the multiplier is
       ignored. This is not the case in the text rule version of byte_extract. */
    if (so_byte->multiplier)
        snort_byte->multiplier = so_byte->multiplier;
    else
        snort_byte->multiplier = 1;

    if (so_byte->flags & CONTENT_RELATIVE)
        snort_byte->relative_flag = 1;

    if (so_byte->flags & EXTRACT_AS_STRING)
        snort_byte->data_string_convert_flag = 1;

    if (so_byte->flags & BYTE_BIG_ENDIAN)
        snort_byte->endianess = BIG;
    else
        snort_byte->endianess = LITTLE;

    if (so_byte->flags & EXTRACT_AS_HEX)
        snort_byte->base = 16;
    if (so_byte->flags & EXTRACT_AS_DEC)
        snort_byte->base = 10;
    if (so_byte->flags & EXTRACT_AS_OCT)
        snort_byte->base = 8;

    snort_byte->var_number = AddVarNameToList(snort_byte);
    snort_byte->byte_order_func = NULL;

    /* Add option to list */
    fpl = AddOptFuncToList(DetectByteExtract, otn);
    fpl->type = RULE_OPTION_TYPE_BYTE_EXTRACT;
    if (add_detection_option(sc, RULE_OPTION_TYPE_BYTE_EXTRACT, (void *)snort_byte, &dup) == DETECTION_OPTION_EQUAL)
    {
        free(snort_byte->name);
        free(snort_byte);
        snort_byte = dup;
    }

    fpl->context = (void *) snort_byte;
    if (snort_byte->relative_flag)
        fpl->isRelative = 1;

    return 0;
}

static int ConvertSetCursorOption(SnortConfig *sc, Rule *rule, int index, OptTreeNode *otn)
{
    return 0;
}

static int ConvertLoopOption(SnortConfig *sc, Rule *rule, int index, OptTreeNode *otn)
{
    return 0;
}

static int ConvertFileDataOption(SnortConfig *sc, Rule *rule, int index, OptTreeNode *otn)
{
    CursorInfo *cursor = rule->options[index]->option_u.cursor;
    FileData *data = (FileData *) SnortAlloc(sizeof(FileData));
    OptFpList *fpl;
    void *dup;

    if (cursor->flags & BUF_FILE_DATA_MIME)
        data->mime_decode_flag = 1;
    else
        data->mime_decode_flag = 0;

    if (add_detection_option(sc, RULE_OPTION_TYPE_FILE_DATA, (void *)data, &dup) == DETECTION_OPTION_EQUAL)
    {
        free(data);
        data = dup;
    }

    fpl = AddOptFuncToList(FileDataEval, otn);
    fpl->type = RULE_OPTION_TYPE_FILE_DATA;
    fpl->context = (void *)data;

    return 1;
}

static int ConvertPktDataOption(SnortConfig *sc, Rule *rule, int index, OptTreeNode *otn)
{
    OptFpList *fpl;
    fpl = AddOptFuncToList(PktDataEval, otn);
    fpl->type = RULE_OPTION_TYPE_PKT_DATA;

    return 1;
}


static int ConvertBase64DataOption(SnortConfig *sc, Rule *rule, int index, OptTreeNode *otn)
{
    OptFpList *fpl;
    fpl = AddOptFuncToList(Base64DataEval, otn);
    fpl->type = RULE_OPTION_TYPE_BASE64_DATA;

    return 1;
}

static int ConvertBase64DecodeOption(SnortConfig *sc, Rule *rule, int index, OptTreeNode *otn)
{
    base64DecodeData *bData = rule->options[index]->option_u.bData;
    Base64DecodeData *data = (Base64DecodeData *) SnortAlloc(sizeof(Base64DecodeData));
    OptFpList *fpl;
    void *dup;

    if (bData->relative)
        data->flags |= BASE64DECODE_RELATIVE_FLAG;
    else
        data->flags = 0;

    data->offset = bData->offset;
    data->bytes_to_decode = bData->bytes;

    otn->ds_list[PLUGIN_BASE64_DECODE] = data;

    if (add_detection_option(sc, RULE_OPTION_TYPE_BASE64_DECODE, (void *)data, &dup) == DETECTION_OPTION_EQUAL)
    {
        free(data);
        data = dup;
    }

    fpl = AddOptFuncToList(Base64DecodeEval, otn);
    fpl->type = RULE_OPTION_TYPE_BASE64_DECODE;
    fpl->context = (void *)data;

    if (data->flags & BASE64DECODE_RELATIVE_FLAG)
        fpl->isRelative = 1;
    return 1;
}

static int ConvertByteMathOption(SnortConfig *sc, Rule *rule, int index, OptTreeNode *otn)
{
    ByteData *byte = rule->options[index]->option_u.byte;
    ByteMathData *byte_math = (ByteMathData *) SnortAlloc(sizeof(ByteMathData));
    OptFpList *fpl;
    void *idx_dup;

    ClearByteMathVarNames(otn->opt_func);
    byte_math->bytes_to_extract = byte->bytes;
    byte_math->rvalue = byte->value;
    byte_math->rvalue_var = GetVarByName(byte->value_refId);
    byte_math->offset = byte->offset;
    byte_math->offset_var = GetVarByName(byte->offset_refId);
    byte_math->operator = byte->op;
    byte_math->bitmask_val = byte->bitmask_val;
    byte_math->result_var = SnortStrdup(byte->refId);

    if (byte->flags & CONTENT_RELATIVE)
        byte_math->relative_flag = 1;
    if (byte->flags & EXTRACT_AS_STRING)
        byte_math->data_string_convert_flag = 1;

    if (byte->flags & BYTE_BIG_ENDIAN)
        byte_math->endianess = BIG;
    else
        byte_math->endianess = LITTLE;

    if (byte->flags & EXTRACT_AS_DEC)
        byte_math->base = 10;
    if (byte->flags & EXTRACT_AS_OCT)
        byte_math->base = 8;
    if (byte->flags & EXTRACT_AS_HEX)
        byte_math->base = 16;

    AddVarName_Bytemath(byte_math);
    fpl = AddOptFuncToList(ByteMath, otn);
    fpl->type = RULE_OPTION_TYPE_BYTE_MATH;

    if (add_detection_option(sc, RULE_OPTION_TYPE_BYTE_MATH, (void *)byte_math, &idx_dup) == DETECTION_OPTION_EQUAL)
    {
        free(byte_math->result_var);
        free(byte_math);
        byte_math = idx_dup;
    }

    fpl->context = (void *)byte_math;

    if (byte_math->relative_flag == 1)
        fpl->isRelative = 1;
    return 1;
}

