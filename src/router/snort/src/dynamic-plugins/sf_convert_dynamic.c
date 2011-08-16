/* $Id$ */
/****************************************************************************
 *
 * Copyright (C) 2003-2011 Sourcefire, Inc.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 ****************************************************************************/

#ifdef DYNAMIC_PLUGIN

#include "sf_engine/sf_snort_plugin_api.h"
#include "detection_options.h"
#include "rules.h"
#include "treenodes.h"
#include "plugbase.h"

#include "sf_convert_dynamic.h"

#include "sp_asn1_detect.h"
#include "sp_byte_check.h"
#include "sp_byte_jump.h"
#include "sp_clientserver.h"
#include "sp_flowbits.h"
#include "sp_isdataat.h"
#include "sp_pattern_match.h"
#include "sp_pcre.h"
#include "sp_hdr_opt_wrap.h"
#include "sp_file_data.h"
#include "sp_base64_decode.h"

extern void ParsePattern(char *, OptTreeNode *, int);
// extern int PCRESetup(Rule *rule, PCREInfo *pcreInfo);
extern void *pcreCompile(const char *pattern, int options, const char **errptr,
    int *erroffset, const unsigned char *tableptr);
extern void *pcreStudy(const void *code, int options, const char **errptr);

extern int SnortPcre(void *option_data, Packet *p);
extern int FlowBitsCheck(void *option_data, Packet *p);
extern int CheckFlow(void *option_data, Packet *p);
extern int Asn1Detect(void *option_data, Packet *p);
extern int ByteTest(void *option_data, Packet *p);
extern int ByteJump(void *option_data, Packet *p);
extern int IsDataAt(void *option_data, Packet *p);
extern int FileDataEval(void *option_data, Packet *p);
extern int Base64DecodeEval(void *option_data, Packet *p) ;

static int CheckConvertability(Rule *rule, OptTreeNode *otn);
static int ConvertContentOption(Rule *rule, int index, OptTreeNode *otn);
static int ConvertPcreOption(Rule *rule, int index, OptTreeNode *otn);
static int ConvertFlowbitOption(Rule *rule, int index, OptTreeNode *otn);
static int ConvertFlowflagsOption(Rule *rule, int index, OptTreeNode *otn);
static int ConvertAsn1Option(Rule *rule, int index, OptTreeNode *otn);
static int ConvertCursorOption(Rule *rule, int index, OptTreeNode *otn);
static int ConvertHdrCheckOption(Rule *rule, int index, OptTreeNode *otn);
static int ConvertByteTestOption(Rule *rule, int index, OptTreeNode *otn);
static int ConvertByteJumpOption(Rule *rule, int index, OptTreeNode *otn);
static int ConvertByteExtractOption(Rule *rule, int index, OptTreeNode *otn);
static int ConvertSetCursorOption(Rule *rule, int index, OptTreeNode *otn);
static int ConvertLoopOption(Rule *rule, int index, OptTreeNode *otn);
static int ConvertFileDataOption(Rule *rule, int index, OptTreeNode *otn);
static int ConvertBase64DecodeOption(Rule *rule, int index, OptTreeNode *otn);

/* Use an array of callbacks to handle varying option types
 *
 * NOTE:  These MUST align with the values in DynamicOptionType enumeration
 * found in sf_dynamic_define.h
 */
static int (* OptionConverterArray[OPTION_TYPE_MAX])
    (Rule *rule, int index, OptTreeNode *otn) =
{
    NULL,
    ConvertContentOption,
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
    ConvertBase64DecodeOption
};

/* Convert a dynamic rule to native rule structure. */
/* Return -1 on error, 0 if nothing done, 1 on success. */
int ConvertDynamicRule(Rule *rule, OptTreeNode *otn)
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
        if (optype < OPTION_TYPE_CONTENT || optype >= OPTION_TYPE_MAX)
            return -1; // Invalid option type
        
        ret = OptionConverterArray[optype](rule, i, otn);
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

             setParserPolicy(policyId);

            FinalizeContentUniqueness(otn);
         }
    }
    otn->sigInfo.shared = 0;

    return 1;
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
            case OPTION_TYPE_PREPROCESSOR:
            case OPTION_TYPE_BYTE_EXTRACT:
            case OPTION_TYPE_SET_CURSOR:
            case OPTION_TYPE_LOOP:
                return -1;
        }
    }

    /* We're good! */
    return 1;
}

/* Option-converting functions */
static int ConvertContentOption(Rule *rule, int index, OptTreeNode *otn)
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
    if ( content->flags & URI_CONTENT_BUFS )
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
    pmd->offset_var = -1;
    pmd->depth_var = -1;
    pmd->distance_var = -1;
    pmd->within_var = -1;

    /* Set URI buffer flags */
    if (content->flags & CONTENT_BUF_URI)
        pmd->uri_buffer |= HTTP_SEARCH_URI;
    if (content->flags & CONTENT_BUF_HEADER)
        pmd->uri_buffer |= HTTP_SEARCH_HEADER;
    if (content->flags & CONTENT_BUF_POST)
        pmd->uri_buffer |= HTTP_SEARCH_CLIENT_BODY;
    if (content->flags & CONTENT_BUF_METHOD)
        pmd->uri_buffer |= HTTP_SEARCH_METHOD;
    if (content->flags & CONTENT_BUF_COOKIE)
        pmd->uri_buffer |= HTTP_SEARCH_COOKIE;
    if (content->flags & CONTENT_BUF_RAW_URI)
        pmd->uri_buffer |= HTTP_SEARCH_RAW_URI;
    if (content->flags & CONTENT_BUF_RAW_HEADER)
        pmd->uri_buffer |= HTTP_SEARCH_RAW_HEADER;
    if (content->flags & CONTENT_BUF_RAW_COOKIE)
        pmd->uri_buffer |= HTTP_SEARCH_RAW_COOKIE;
    if (content->flags & CONTENT_BUF_STAT_CODE)
        pmd->uri_buffer |= HTTP_SEARCH_STAT_CODE;
    if (content->flags & CONTENT_BUF_STAT_MSG)
        pmd->uri_buffer |= HTTP_SEARCH_STAT_MSG;


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
        pmd->within = content->depth;
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

static int ConvertPcreOption(Rule *rule, int index, OptTreeNode *otn)
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

    pcre_data->pe = pcreStudy(
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

    if (pcre_info->flags & CONTENT_BUF_POST)
        pcre_data->options |= SNORT_PCRE_HTTP_BODY;

    if (pcre_info->flags & CONTENT_BUF_HEADER)
        pcre_data->options |= SNORT_PCRE_HTTP_HEADER;

    if (pcre_info->flags & CONTENT_BUF_METHOD)
        pcre_data->options |= SNORT_PCRE_HTTP_METHOD;

    if (pcre_info->flags & CONTENT_BUF_COOKIE)
        pcre_data->options |= SNORT_PCRE_HTTP_COOKIE;

    if (pcre_info->flags & CONTENT_BUF_RAW_URI)
        pcre_data->options |= SNORT_PCRE_HTTP_RAW_URI;

    if (pcre_info->flags & CONTENT_BUF_STAT_CODE)
        pcre_data->options |= SNORT_PCRE_HTTP_STAT_CODE;

    if (pcre_info->flags & CONTENT_BUF_STAT_MSG)
        pcre_data->options |= SNORT_PCRE_HTTP_STAT_MSG;

    if (pcre_info->flags & CONTENT_BUF_RAW_URI)
        pcre_data->options |= SNORT_PCRE_HTTP_RAW_URI;

    if (pcre_info->flags & CONTENT_BUF_RAW_HEADER)
        pcre_data->options |= SNORT_PCRE_HTTP_RAW_HEADER;

    if (pcre_info->flags & CONTENT_BUF_RAW_COOKIE)
        pcre_data->options |= SNORT_PCRE_HTTP_RAW_COOKIE;

    if (pcre_info->flags & CONTENT_BUF_STAT_CODE)
        pcre_data->options |= SNORT_PCRE_HTTP_STAT_CODE;

    if (pcre_info->flags & CONTENT_BUF_STAT_MSG)
        pcre_data->options |= SNORT_PCRE_HTTP_STAT_MSG;

    PcreCheckAnchored(pcre_data);

    /* Attach option to tree, checking for duplicate */
    otn->pcre_flag = 1;
    fpl = AddOptFuncToList(SnortPcre, otn);
    fpl->type = RULE_OPTION_TYPE_PCRE;

    if (add_detection_option(RULE_OPTION_TYPE_PCRE, (void *)pcre_data, &pcre_dup) == DETECTION_OPTION_EQUAL)
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

static int ConvertFlowbitOption(Rule *rule, int index, OptTreeNode *otn)
{
    FlowBitsInfo *flowBit = rule->options[index]->option_u.flowBit;
    FLOWBITS_OP *fop = (FLOWBITS_OP *) SnortAlloc(sizeof(FLOWBITS_OP));
    OptFpList *fpl;
    void *idx_dup;

    /* Convert struct for rule option */
    fop->name = SnortStrdup(flowBit->flowBitsName);
    fop->type = flowBit->operation;
    fop->id = flowBit->id;

    /* Add detection option to hash table */
    otn->ds_list[PLUGIN_FLOWBIT] = (void *)1;
    if (add_detection_option(RULE_OPTION_TYPE_FLOWBIT, (void *)fop, &idx_dup) == DETECTION_OPTION_EQUAL)
    {
        free(fop->name);
        free(fop);
        fop = idx_dup;
    }

    /* Add detection function to otn */
    fpl = AddOptFuncToList(FlowBitsCheck, otn);
    fpl->type = RULE_OPTION_TYPE_FLOWBIT;
    fpl->context = (void *) fop;
    return 1;
}

static int ConvertFlowflagsOption(Rule *rule, int index, OptTreeNode *otn)
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
    else
        csdata->unestablished = 1;
    csdata->stateless = 0;

    otn->stateless = csdata->stateless;
    otn->established = csdata->established;
    otn->unestablished = csdata->unestablished;

    if (add_detection_option(RULE_OPTION_TYPE_FLOW, (void *)csdata, &dup) == DETECTION_OPTION_EQUAL)
    {
        free(csdata);
        csdata = dup;
    }

    fpl = AddOptFuncToList(CheckFlow, otn);
    fpl->type = RULE_OPTION_TYPE_FLOW;
    fpl->context = (void *)csdata;

    return 1;
}

static int ConvertAsn1Option(Rule *rule, int index, OptTreeNode *otn)
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

    if (add_detection_option(RULE_OPTION_TYPE_ASN1, (void *)asn1, &dup) == DETECTION_OPTION_EQUAL)
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

static int ConvertCursorOption(Rule *rule, int index, OptTreeNode *otn)
{
    CursorInfo *cursor = rule->options[index]->option_u.cursor;
    IsDataAtData *data = (IsDataAtData *) SnortAlloc(sizeof(IsDataAtData));
    OptFpList *fpl;
    void *dup;

    data->offset = cursor->offset;
    data->offset_var = -1;
    if (cursor->flags & CONTENT_RELATIVE)
        data->flags |= ISDATAAT_RELATIVE_FLAG;
    if (cursor->flags & CONTENT_BUF_RAW)
        data->flags |= ISDATAAT_RAWBYTES_FLAG;
    if (cursor->flags & NOT_FLAG)
        data->flags |= ISDATAAT_NOT_FLAG;

    if (add_detection_option(RULE_OPTION_TYPE_IS_DATA_AT, (void *)data, &dup) == DETECTION_OPTION_EQUAL)
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

static int ConvertHdrCheckOption(Rule *rule, int index, OptTreeNode *otn)
{
    HdrOptCheck *hdrData = rule->options[index]->option_u.hdrData;
    OptFpList *fpl;
    void *dup;

    /* This option type gets added directly to the tree. */
    if (add_detection_option(RULE_OPTION_TYPE_HDR_OPT_CHECK, (void *)hdrData, &dup) == DETECTION_OPTION_EQUAL)
    {
        hdrData = dup;
    }

    fpl = AddOptFuncToList(HdrOptEval, otn);
    fpl->context = hdrData;
    fpl->type = RULE_OPTION_TYPE_HDR_OPT_CHECK;
    return 1;
}

static int ConvertByteTestOption(Rule *rule, int index, OptTreeNode *otn)
{
    ByteData *byte = rule->options[index]->option_u.byte;
    ByteTestData *byte_test = (ByteTestData *) SnortAlloc(sizeof(ByteTestData));
    OptFpList *fpl;
    void *idx_dup;

    byte_test->bytes_to_compare = byte->bytes;
    byte_test->cmp_value = byte->value;
    byte_test->cmp_value_var = -1;
    byte_test->offset = byte->offset;
    byte_test->offset_var = -1;
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
    
    if (add_detection_option(RULE_OPTION_TYPE_BYTE_TEST, (void *)byte_test, &idx_dup) == DETECTION_OPTION_EQUAL)
    {
        free(byte_test);
        byte_test = idx_dup;
    }

    fpl->context = (void *)byte_test;

    if (byte_test->relative_flag == 1)
        fpl->isRelative = 1;

    return 1;
}

static int ConvertByteJumpOption(Rule *rule, int index, OptTreeNode *otn)
{
    ByteData *byte = rule->options[index]->option_u.byte;
    ByteJumpData *byte_jump = SnortAlloc(sizeof(ByteJumpData));
    OptFpList *fpl;
    void *dup;

    byte_jump->bytes_to_grab = byte->bytes;
    byte_jump->offset = byte->offset;
    byte_jump->offset_var = -1;
    byte_jump->multiplier = byte->multiplier;
    byte_jump->post_offset = byte->post_offset;

    if (byte->flags & CONTENT_RELATIVE)
        byte_jump->relative_flag = 1;
    if (byte->flags & EXTRACT_AS_STRING)
        byte_jump->data_string_convert_flag = 1;
    if (byte->flags & JUMP_FROM_BEGINNING)
        byte_jump->from_beginning_flag = 1;
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
    if (add_detection_option(RULE_OPTION_TYPE_BYTE_JUMP, (void *)byte_jump, &dup) == DETECTION_OPTION_EQUAL)
    {
        free(byte_jump);
        byte_jump = dup;
    }
    fpl->context = (void *)byte_jump;

    if (byte_jump->relative_flag == 1)
        fpl->isRelative = 1;

    return 1;
}

static int ConvertByteExtractOption(Rule *rule, int index, OptTreeNode *otn)
{
    return 0;
}

static int ConvertSetCursorOption(Rule *rule, int index, OptTreeNode *otn)
{
    return 0;
}

static int ConvertLoopOption(Rule *rule, int index, OptTreeNode *otn)
{
    return 0;
}

static int ConvertFileDataOption(Rule *rule, int index, OptTreeNode *otn)
{
    CursorInfo *cursor = rule->options[index]->option_u.cursor;
    FileData *data = (FileData *) SnortAlloc(sizeof(FileData));
    OptFpList *fpl;
    void *dup;

    if (cursor->flags & BUF_FILE_DATA_MIME)
        data->mime_decode_flag = 1;
    else
        data->mime_decode_flag = 0;

    if (add_detection_option(RULE_OPTION_TYPE_FILE_DATA, (void *)data, &dup) == DETECTION_OPTION_EQUAL)
    {
        free(data);
        data = dup;
    }

    fpl = AddOptFuncToList(FileDataEval, otn);
    fpl->type = RULE_OPTION_TYPE_FILE_DATA;
    fpl->context = (void *)data;

    return 1;
}

static int ConvertBase64DecodeOption(Rule *rule, int index, OptTreeNode *otn)
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

    if (add_detection_option(RULE_OPTION_TYPE_BASE64_DECODE, (void *)data, &dup) == DETECTION_OPTION_EQUAL)
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

#endif /* DYNAMIC_PLUGIN */
