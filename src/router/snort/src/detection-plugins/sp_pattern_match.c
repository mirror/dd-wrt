/* $Id$ */
/*
** Copyright (C) 2002-2011 Sourcefire, Inc.
** Copyright (C) 1998-2002 Martin Roesch <roesch@sourcefire.com>
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

/* 
 * 06/07/2007 - tw
 * Commented out 'content-list' code since it's considered broken and there
 * are no plans to fix it
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <errno.h>
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif
#ifdef DEBUG
# include <assert.h>
#endif

#include "sp_pattern_match.h"
#include "sp_replace.h"
#include "bounds.h"
#include "rules.h"
#include "treenodes.h"
#include "plugbase.h"
#include "debug.h"
#include "mstring.h"
#include "util.h" 
#include "parser.h"
#include "plugin_enum.h"
#include "checksum.h"
#include "sfhashfcn.h"
#include "spp_httpinspect.h"
#include "snort.h"
#include "profiler.h"
#include "sfhashfcn.h"
#include "detection_options.h"
#include "sp_byte_extract.h"
#include "detection_util.h"

/********************************************************************
 * Macros
 ********************************************************************/
#define MAX_PATTERN_SIZE 2048
#define PM_FP_ONLY  "only"
#define URIBUFS_SET(pmd, flags) (((pmd)->uri_buffer & (flags)) == (flags))

/********************************************************************
 * Global variables
 ********************************************************************/
#ifdef PERF_PROFILING
PreprocStats contentPerfStats;
PreprocStats uricontentPerfStats;
#endif
int lastType = PLUGIN_PATTERN_MATCH;

#if 0
/* For OR patterns - not currently used */
int list_file_line;     /* current line being processed in the list file */
#endif

/********************************************************************
 * Extern variables
 ********************************************************************/
#ifdef PERF_PROFILING
extern PreprocStats ruleOTNEvalPerfStats;
#endif

/********************************************************************
 * Private function prototypes
 ********************************************************************/
static void PayloadSearchInit(char *, OptTreeNode *, int);
static void PayloadSearchUri(char *, OptTreeNode *, int);
static void PayloadSearchHttpMethod(char *, OptTreeNode *, int);
static void PayloadSearchHttpUri(char *, OptTreeNode *, int);
static void PayloadSearchHttpHeader(char *, OptTreeNode *, int);
static void PayloadSearchHttpCookie(char *, OptTreeNode *, int);
static void PayloadSearchHttpBody(char *, OptTreeNode *, int);
static void PayloadSearchHttpRawUri(char *, OptTreeNode *, int);
static void PayloadSearchHttpRawHeader(char *, OptTreeNode *, int);
//static void PayloadSearchHttpRawBody(char *, OptTreeNode *, int);
static void PayloadSearchHttpRawCookie(char *, OptTreeNode *, int);
static void PayloadSearchHttpStatCode(char *, OptTreeNode *, int);
static void PayloadSearchHttpStatMsg(char *, OptTreeNode *, int);
static void PayloadSearchOffset(char *, OptTreeNode *, int);
static void PayloadSearchDepth(char *, OptTreeNode *, int);
static void PayloadSearchDistance(char *, OptTreeNode *, int);
static void PayloadSearchWithin(char *, OptTreeNode *, int);
static void PayloadSearchNocase(char *, OptTreeNode *, int);
static void PayloadSearchRawbytes(char *, OptTreeNode *, int);
static void PayloadSearchFastPattern(char *, OptTreeNode *, int);
static INLINE int HasFastPattern(OptTreeNode *, int);
static int32_t ParseInt(const char *, const char *);
static INLINE PatternMatchData * GetLastPmdError(OptTreeNode *, int, const char *);
static INLINE PatternMatchData * GetLastPmd(OptTreeNode *, int);
static void ValidateHttpContentModifiers(PatternMatchData *);
static void MovePmdToUriDsList(OptTreeNode *, PatternMatchData *);
static char *PayloadExtractParameter(char *, int *);
static INLINE void ValidateContent(PatternMatchData *, int);
static unsigned int GetMaxJumpSize(char *, int);
static INLINE int computeWithin(int, PatternMatchData *);
static int uniSearch(const char *, int, PatternMatchData *);
static int uniSearchReal(const char *data, int dlen, PatternMatchData *pmd, int nocase);

#if 0
/* Not currently used - DO NOT REMOVE */
static INLINE int computeDepth(int dlen, PatternMatchData * pmd);
static int uniSearchREG(char * data, int dlen, PatternMatchData * pmd);
#endif

#if 0
static const char *format_uri_buffer_str(int, int, char *);
static void PayloadSearchListInit(char *, OptTreeNode *, int);
static void ParseContentListFile(char *, OptTreeNode *, int);
static void PrintDupDOTPmds(PatternMatchData *pmd,
        PatternMatchData *pmd_dup, option_type_t type)
#endif

/********************************************************************
 * Setup and parsing functions
 ********************************************************************/
void SetupPatternMatch(void)
{
    /* initial pmd setup options */
    RegisterRuleOption("content", PayloadSearchInit, NULL, OPT_TYPE_DETECTION, NULL);
    RegisterRuleOption("uricontent", PayloadSearchUri, NULL, OPT_TYPE_DETECTION, NULL);

    /* http content modifiers */
    RegisterRuleOption("http_method", PayloadSearchHttpMethod, NULL, OPT_TYPE_DETECTION, NULL);
    RegisterRuleOption("http_uri", PayloadSearchHttpUri, NULL, OPT_TYPE_DETECTION, NULL);
    RegisterRuleOption("http_header", PayloadSearchHttpHeader, NULL, OPT_TYPE_DETECTION, NULL);
    RegisterRuleOption("http_cookie", PayloadSearchHttpCookie, NULL, OPT_TYPE_DETECTION, NULL);
    RegisterRuleOption("http_client_body", PayloadSearchHttpBody, NULL, OPT_TYPE_DETECTION, NULL);
    RegisterRuleOption("http_raw_uri", PayloadSearchHttpRawUri, NULL, OPT_TYPE_DETECTION, NULL);
    RegisterRuleOption("http_raw_header", PayloadSearchHttpRawHeader, NULL, OPT_TYPE_DETECTION, NULL);
    /*RegisterRuleOption("http_raw_client_body", PayloadSearchHttpRawBody, NULL, OPT_TYPE_DETECTION, NULL);*/
    RegisterRuleOption("http_raw_cookie", PayloadSearchHttpRawCookie, NULL, OPT_TYPE_DETECTION, NULL);
    RegisterRuleOption("http_stat_code", PayloadSearchHttpStatCode, NULL, OPT_TYPE_DETECTION, NULL);
    RegisterRuleOption("http_stat_msg", PayloadSearchHttpStatMsg, NULL, OPT_TYPE_DETECTION, NULL);

    /* pattern offsets and depths */
    RegisterRuleOption("offset", PayloadSearchOffset, NULL, OPT_TYPE_DETECTION, NULL);
    RegisterRuleOption("depth", PayloadSearchDepth, NULL, OPT_TYPE_DETECTION, NULL);

    /* distance and within are offset and depth, but relative to last match */
    RegisterRuleOption("distance", PayloadSearchDistance, NULL, OPT_TYPE_DETECTION, NULL);
    RegisterRuleOption("within", PayloadSearchWithin, NULL, OPT_TYPE_DETECTION, NULL);

    /* other modifiers */
    RegisterRuleOption("nocase", PayloadSearchNocase, NULL, OPT_TYPE_DETECTION, NULL);
    RegisterRuleOption("rawbytes", PayloadSearchRawbytes, NULL, OPT_TYPE_DETECTION, NULL);
    RegisterRuleOption("fast_pattern", PayloadSearchFastPattern, NULL, OPT_TYPE_DETECTION, NULL);
    RegisterRuleOption("replace", PayloadReplaceInit, NULL, OPT_TYPE_DETECTION, NULL);

#if 0
    /* Not implemented yet */
    RegisterRuleOption("content-list", PayloadSearchListInit, NULL, OPT_TYPE_DETECTION, NULL);
#endif

#ifdef PERF_PROFILING
    RegisterPreprocessorProfile("content", &contentPerfStats, 3, &ruleOTNEvalPerfStats);
    RegisterPreprocessorProfile("uricontent", &uricontentPerfStats, 3, &ruleOTNEvalPerfStats);
#endif
    DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, 
                "Plugin: PatternMatch Initialized!\n"););
}

static void PayloadSearchInit(char *data, OptTreeNode * otn, int protocol)
{
    OptFpList *fpl;
    PatternMatchData *pmd;
    char *data_end;
    char *data_dup;
    char *opt_data;
    int opt_len = 0;
    char *next_opt;

    DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, "In PayloadSearchInit()\n"););

    /* whack a new node onto the list */
    pmd = NewNode(otn, PLUGIN_PATTERN_MATCH);
    lastType = PLUGIN_PATTERN_MATCH;

    if (!data)
        ParseError("No Content Pattern specified!");

    data_dup = SnortStrdup(data);
    data_end = data_dup + strlen(data_dup);

    opt_data = PayloadExtractParameter(data_dup, &opt_len);

    /* set up the pattern buffer */
    ParsePattern(opt_data, otn, PLUGIN_PATTERN_MATCH);
    next_opt = opt_data + opt_len;

    /* link the plugin function in to the current OTN */
    fpl = AddOptFuncToList(CheckANDPatternMatch, otn);
    fpl->type = RULE_OPTION_TYPE_CONTENT;
    pmd->buffer_func = CHECK_AND_PATTERN_MATCH;

    fpl->context = pmd;
    pmd->fpl = fpl;

    // if content is followed by any comma separated options,
    // we have to parse them here.  content related options
    // separated by semicolons go straight to the callbacks.
    while (next_opt < data_end)
    {
        char **opts;        /* dbl ptr for mSplit call, holds rule tokens */
        int num_opts;       /* holds number of tokens found by mSplit */
        char* opt1;

        next_opt++;
        if (next_opt == data_end)
            break;

        opt_len = 0;
        opt_data = PayloadExtractParameter(next_opt, &opt_len);
        if (!opt_data)
            break;

        next_opt = opt_data + opt_len;

        opts = mSplit(opt_data, " \t", 2, &num_opts, 0);

        if (!opts)
            continue;
        opt1 = (num_opts == 2) ? opts[1] : NULL;

        if (!strcasecmp(opts[0], "offset"))
        {
            PayloadSearchOffset(opt1, otn, protocol);
        }
        else if (!strcasecmp(opts[0], "depth"))
        {
            PayloadSearchDepth(opt1, otn, protocol);
        }
        else if (!strcasecmp(opts[0], "nocase"))
        {
            PayloadSearchNocase(opt1, otn, protocol);
        }
        else if (!strcasecmp(opts[0], "rawbytes"))
        {
            PayloadSearchRawbytes(opt1, otn, protocol);
        }
        else if (!strcasecmp(opts[0], "http_uri"))
        {
            PayloadSearchHttpUri(opt1, otn, protocol);
        }
        else if (!strcasecmp(opts[0], "http_client_body"))
        {
            PayloadSearchHttpBody(opt1, otn, protocol);
        }
        else if (!strcasecmp(opts[0], "http_header"))
        {
            PayloadSearchHttpHeader(opt1, otn, protocol);
        }
        else if (!strcasecmp(opts[0], "http_method"))
        {
            PayloadSearchHttpMethod(opt1, otn, protocol);
        }
        else if (!strcasecmp(opts[0], "http_cookie"))
        {
            PayloadSearchHttpCookie(opt1, otn, protocol);
        }
        else if (!strcasecmp(opts[0], "http_raw_uri"))
        {
            PayloadSearchHttpRawUri(opt1, otn, protocol);
        }
        else if (!strcasecmp(opts[0], "http_raw_header"))
        {
            PayloadSearchHttpRawHeader(opt1, otn, protocol);
        }
        else if (!strcasecmp(opts[0], "http_raw_cookie"))
        {
            PayloadSearchHttpRawCookie(opt1, otn, protocol);
        }
        else if (!strcasecmp(opts[0], "http_stat_code"))
        {
            PayloadSearchHttpStatCode(opt1, otn, protocol);
        }
        else if (!strcasecmp(opts[0], "http_stat_msg"))
        {
            PayloadSearchHttpStatMsg(opt1, otn, protocol);
        }
        else if (!strcasecmp(opts[0], "fast_pattern"))
        {
            PayloadSearchFastPattern(opt1, otn, protocol);
        }
        else if (!strcasecmp(opts[0], "distance"))
        {
            PayloadSearchDistance(opt1, otn, protocol);
        }
        else if (!strcasecmp(opts[0], "within"))
        {
            PayloadSearchWithin(opt1, otn, protocol);
        }
        else if (!strcasecmp(opts[0], "replace"))
        {
            PayloadReplaceInit(opt1, otn, protocol);
        }
        else
        {
            ParseError("Invalid Content parameter specified!");
        }
        mSplitFree(&opts, num_opts);
    }

    free(data_dup);

    if(pmd->use_doe == 1)
        fpl->isRelative = 1;

    DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, 
                "OTN function PatternMatch Added to rule!\n"););
}

static void PayloadSearchUri(char *data, OptTreeNode * otn, int protocol)
{
    PatternMatchData *pmd = NewNode(otn, PLUGIN_PATTERN_MATCH_URI);
    OptFpList *fpl;

    DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, "In PayloadSearchUri()\n"););

    lastType = PLUGIN_PATTERN_MATCH_URI;

    /* set up the pattern buffer */
    ParsePattern(data, otn, PLUGIN_PATTERN_MATCH_URI);

    pmd->uri_buffer |= HTTP_SEARCH_URI;

    /* link the plugin function in to the current OTN */
    fpl = AddOptFuncToList(CheckUriPatternMatch, otn);

    fpl->type = RULE_OPTION_TYPE_CONTENT_URI;
    pmd->buffer_func = CHECK_URI_PATTERN_MATCH;

    fpl->context = pmd;
    pmd->fpl = fpl;

    if (pmd->use_doe == 1)
        fpl->isRelative = 1;

    DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, 
                "OTN function PatternMatch Added to rule!\n"););
}

static void PayloadSearchHttpMethod(char *data, OptTreeNode * otn, int protocol)
{
    PatternMatchData *pmd = GetLastPmdError(otn, lastType, "http_method");

    if (data != NULL)
        ParseError("'http_method' does not take an argument");

    pmd->uri_buffer |= HTTP_SEARCH_METHOD;
    MovePmdToUriDsList(otn, pmd);
}

static void PayloadSearchHttpUri(char *data, OptTreeNode * otn, int protocol)
{
    PatternMatchData *pmd = GetLastPmdError(otn, lastType, "http_uri");

    if (data != NULL)
        ParseError("'http_uri' does not take an argument");

    pmd->uri_buffer |= HTTP_SEARCH_URI;
    MovePmdToUriDsList(otn, pmd);
}

static void PayloadSearchHttpHeader(char *data, OptTreeNode * otn, int protocol)
{
    PatternMatchData *pmd = GetLastPmdError(otn, lastType, "http_header");

    if (data != NULL)
        ParseError("'http_header' does not take an argument");

    pmd->uri_buffer |= HTTP_SEARCH_HEADER;
    MovePmdToUriDsList(otn, pmd);
}

static void PayloadSearchHttpCookie(char *data, OptTreeNode * otn, int protocol)
{
    PatternMatchData *pmd = GetLastPmdError(otn, lastType, "http_cookie");

    if (data != NULL)
        ParseError("'http_cookie' does not take an argument");

    pmd->uri_buffer |= HTTP_SEARCH_COOKIE;
    MovePmdToUriDsList(otn, pmd);
}

static void PayloadSearchHttpBody(char *data, OptTreeNode * otn, int protocol)
{
    PatternMatchData *pmd = GetLastPmdError(otn, lastType, "http_client_body");

    if (data != NULL)
        ParseError("'http_client_body' does not take an argument");

    pmd->uri_buffer |= HTTP_SEARCH_CLIENT_BODY;
    MovePmdToUriDsList(otn, pmd);
}

static void PayloadSearchHttpRawUri(char *data, OptTreeNode * otn, int protocol)
{
    PatternMatchData *pmd = GetLastPmdError(otn, lastType, "http_raw_uri");

    if (data != NULL)
        ParseError("'http_raw_uri' does not take an argument");

    pmd->uri_buffer |= HTTP_SEARCH_RAW_URI;
    MovePmdToUriDsList(otn, pmd);
}

static void PayloadSearchHttpRawHeader(char *data, OptTreeNode * otn, int protocol)
{
    PatternMatchData *pmd = GetLastPmdError(otn, lastType, "http_raw_header");

    if (data != NULL)
        ParseError("'http_raw_header' does not take an argument");

    pmd->uri_buffer |= HTTP_SEARCH_RAW_HEADER;
    MovePmdToUriDsList(otn, pmd);
}
static void PayloadSearchHttpRawCookie(char *data, OptTreeNode * otn, int protocol)
{
    PatternMatchData *pmd = GetLastPmdError(otn, lastType, "http_raw_cookie");

    if (data != NULL)
        ParseError("'http_raw_cookie' does not take an argument");

    pmd->uri_buffer |= HTTP_SEARCH_RAW_COOKIE;
    MovePmdToUriDsList(otn, pmd);
}
static void PayloadSearchHttpStatCode(char *data, OptTreeNode * otn, int protocol)
{
    PatternMatchData *pmd = GetLastPmdError(otn, lastType, "http_stat_code");

    if (data != NULL)
        ParseError("'http_stat_code' does not take an argument");

    pmd->uri_buffer |= HTTP_SEARCH_STAT_CODE;
    MovePmdToUriDsList(otn, pmd);
}
static void PayloadSearchHttpStatMsg(char *data, OptTreeNode * otn, int protocol)
{
    PatternMatchData *pmd = GetLastPmdError(otn, lastType, "http_stat_msg");

    if (data != NULL)
        ParseError("'http_stat_msg' does not take an argument");

    pmd->uri_buffer |= HTTP_SEARCH_STAT_MSG;
    MovePmdToUriDsList(otn, pmd);
}

typedef enum {
    CMF_DISTANCE = 0x1, CMF_WITHIN = 0x2, CMF_OFFSET = 0x4, CMF_DEPTH = 0x8
} ContentModifierFlags;

static unsigned GetCMF (PatternMatchData* pmd)
{
    unsigned cmf = 0;
    if ( (pmd->distance != 0) || (pmd->distance_var != -1) ) cmf |= CMF_DISTANCE;
    if ( (pmd->within != 0) || (pmd->within_var != -1) ) cmf |= CMF_WITHIN;
    if ( (pmd->offset != 0) || (pmd->offset_var != -1) ) cmf |= CMF_OFFSET;
    if ( (pmd->depth != 0) || (pmd->depth_var != -1) ) cmf |= CMF_DEPTH;
    return cmf;
}

#define BAD_DISTANCE (CMF_DISTANCE | CMF_OFFSET | CMF_DEPTH)
#define BAD_WITHIN (CMF_WITHIN | CMF_OFFSET | CMF_DEPTH)
#define BAD_OFFSET (CMF_OFFSET | CMF_DISTANCE | CMF_WITHIN)
#define BAD_DEPTH (CMF_DEPTH | CMF_DISTANCE | CMF_WITHIN)

static void PayloadSearchOffset(char *data, OptTreeNode * otn, int protocol)
{
    PatternMatchData *pmd = GetLastPmdError(otn, lastType, "offset");

    if ( GetCMF(pmd) & BAD_OFFSET )
        ParseError("offset can't be used with itself, distance, or within");

    if (data == NULL)
        ParseError("Missing argument to 'offset' option");

    if (isdigit(data[0]) || data[0] == '-')
    {
        pmd->offset = ParseInt(data, "offset");
    }
    else
    {
        pmd->offset_var = GetVarByName(data);
        if (pmd->offset_var == BYTE_EXTRACT_NO_VAR)
        {
            ParseError(BYTE_EXTRACT_INVALID_ERR_STR);
        }
    }

    DEBUG_WRAP(DebugMessage(DEBUG_PARSER, "Pattern offset = %d\n", 
                pmd->offset););
}

static void PayloadSearchDepth(char *data, OptTreeNode * otn, int protocol)
{
    PatternMatchData *pmd = GetLastPmdError(otn, lastType, "depth");

    if ( GetCMF(pmd) & BAD_DEPTH )
        ParseError("depth can't be used with itself, distance, or within");

    if (data == NULL)
        ParseError("Missing argument to 'depth' option");

    if (isdigit(data[0]) || data[0] == '-')
    {
        pmd->depth = ParseInt(data, "depth");

        /* check to make sure that this the depth allows this rule to fire */
        if ((pmd->depth != 0) && (pmd->depth < (int)pmd->pattern_size))
        {
            ParseError("The depth (%d) is less than the size of the content(%u)!",
                    pmd->depth, pmd->pattern_size);
        }
    }
    else
    {
        pmd->depth_var = GetVarByName(data);
        if (pmd->depth_var == BYTE_EXTRACT_NO_VAR)
        {
            ParseError(BYTE_EXTRACT_INVALID_ERR_STR);
        }
    }

    DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, "Pattern depth = %d\n", 
                pmd->depth););
}

static void PayloadSearchDistance(char *data, OptTreeNode *otn, int protocol)
{
    PatternMatchData *pmd = GetLastPmdError(otn, lastType, "distance");

    if ( GetCMF(pmd) & BAD_DISTANCE )
        ParseError("distance can't be used with itself, offset, or depth");

    if (data == NULL)
        ParseError("Missing argument to 'distance' option");

    if (isdigit(data[0]) || data[0] == '-')
    {
        pmd->distance = ParseInt(data, "distance");
    }
    else
    {
        pmd->distance_var = GetVarByName(data);
        if (pmd->distance_var == BYTE_EXTRACT_NO_VAR)
        {
            ParseError(BYTE_EXTRACT_INVALID_ERR_STR);
        }
    }

    DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, "Pattern distance = %d\n", 
                pmd->distance););

    /* Only do a relative search if this is a normal content match. */
    if (lastType == PLUGIN_PATTERN_MATCH ||  lastType == PLUGIN_PATTERN_MATCH_URI)
    {
        pmd->use_doe = 1;
        pmd->fpl->isRelative = 1;
    }
}

static void PayloadSearchWithin(char *data, OptTreeNode *otn, int protocol)
{
    PatternMatchData *pmd = GetLastPmdError(otn, lastType, "within");

    if ( GetCMF(pmd) & BAD_WITHIN )
        ParseError("within can't be used with itself, offset, or depth");

    if (data == NULL)
        ParseError("Missing argument to 'within' option");

    if (isdigit(data[0]) || data[0] == '-')
    {
        pmd->within = ParseInt(data, "within");

        if (pmd->within < pmd->pattern_size)
            ParseError("within (%d) is smaller than size of pattern", pmd->within);
    }
    else
    {
        pmd->within_var = GetVarByName(data);
        if (pmd->within_var == BYTE_EXTRACT_NO_VAR)
        {
            ParseError(BYTE_EXTRACT_INVALID_ERR_STR);
        }
    }

    DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, "Pattern within = %d\n", 
                pmd->within););

    /* Only do a relative search if this is a normal content match. */
    if (lastType == PLUGIN_PATTERN_MATCH || lastType == PLUGIN_PATTERN_MATCH_URI)
    {
        pmd->use_doe = 1;
        pmd->fpl->isRelative = 1;
    }
}

static void PayloadSearchNocase(char *data, OptTreeNode * otn, int protocol)
{
    unsigned int i;
    PatternMatchData *pmd = GetLastPmdError(otn, lastType, "nocase");

    if (data != NULL)
        ParseError("'nocase' does not take an argument");

    for (i = 0; i < pmd->pattern_size; i++)
        pmd->pattern_buf[i] = toupper((int)pmd->pattern_buf[i]);

    pmd->nocase = 1;

    pmd->search = uniSearchCI;
    make_precomp(pmd);
}

static void PayloadSearchRawbytes(char *data, OptTreeNode * otn, int protocol)
{
    PatternMatchData *pmd = GetLastPmdError(otn, lastType, "rawbytes");

    if (data != NULL)
        ParseError("'rawbytes' does not take an argument");

    /* mark this as inspecting a raw pattern match rather than a
     * decoded application buffer */
    pmd->rawbytes = 1;    
}

static void PayloadSearchFastPattern(char *data, OptTreeNode *otn, int protocol)
{
    PatternMatchData *pmd = GetLastPmdError(otn, lastType, "fast_pattern");

    /* There can only be one fast pattern content in the rule, whether
     * normal, http or other */
    if (pmd->fp)
    {
        ParseError("Cannot set fast_pattern modifier more than once "
                "for the same \"content\".");
    }

    if (HasFastPattern(otn, PLUGIN_PATTERN_MATCH))
        ParseError("Can only use the fast_pattern modifier once in a rule.");
    if (HasFastPattern(otn, PLUGIN_PATTERN_MATCH_URI))
        ParseError("Can only use the fast_pattern modifier once in a rule.");
    //if (HasFastPattern(otn, PLUGIN_PATTERN_MATCH_OR))
    //    ParseError("Can only use the fast_pattern modifier once in a rule.");

    pmd->fp = 1;

    if (data != NULL)
    {
        char *error_str = "Rule option \"fast_pattern\": Invalid parameter: "
            "\"%s\".  Valid parameters are: \"only\" | <offset>,<length>.  "
            "Offset and length must be integers less than 65536, offset cannot "
            "be negative, length must be positive and (offset + length) must "
            "evaluate to less than or equal to the actual pattern length.  "
            "Pattern length: %u";

        if (isdigit((int)*data))
        {
            /* Specifying offset and length of pattern to use for
             * fast pattern matcher */

            long int offset, length;
            char *endptr;
            char **toks;
            int num_toks;

            toks = mSplit(data, ",", 0, &num_toks, 0);
            if (num_toks != 2)
            {
                mSplitFree(&toks, num_toks);
                ParseError(error_str, data, pmd->pattern_size);
            }

            offset = SnortStrtol(toks[0], &endptr, 0);
            if ((errno == ERANGE) || (*endptr != '\0')
                    || (offset < 0) || (offset > UINT16_MAX))
            {
                mSplitFree(&toks, num_toks);
                ParseError(error_str, data, pmd->pattern_size);
            }

            length = SnortStrtol(toks[1], &endptr, 0);
            if ((errno == ERANGE) || (*endptr != '\0')
                    || (length <= 0) || (length > UINT16_MAX))
            {
                mSplitFree(&toks, num_toks);
                ParseError(error_str, data, pmd->pattern_size);
            }

            mSplitFree(&toks, num_toks);

            if ((int)pmd->pattern_size < (offset + length))
                ParseError(error_str, data, pmd->pattern_size);

            pmd->fp_offset = (uint16_t)offset;
            pmd->fp_length = (uint16_t)length;
        }
        else
        {
            /* Specifies that this content should only be used for
             * fast pattern matching */

            if (strcasecmp(data, PM_FP_ONLY) != 0)
                ParseError(error_str, data, pmd->pattern_size);

            pmd->fp_only = 1;
        }
    }
}

static INLINE int HasFastPattern(OptTreeNode *otn, int list_type)
{
    PatternMatchData *tmp;

    if ((otn == NULL) || (otn->ds_list[list_type] == NULL))
        return 0;

    for (tmp = otn->ds_list[list_type]; tmp != NULL; tmp = tmp->next)
    {
        if (tmp->fp)
            return 1;
    }

    return 0;
}

PatternMatchData * NewNode(OptTreeNode *otn, int type)
{
    PatternMatchData *pmd = NULL;

    if (otn->ds_list[type] == NULL)
    {
        otn->ds_list[type] = (PatternMatchData *)SnortAlloc(sizeof(PatternMatchData));
        pmd = otn->ds_list[type];
    }
    else
    {
        pmd = GetLastPmd(otn, type);
        if (pmd != NULL)
        {
            pmd->next = (PatternMatchData *)SnortAlloc(sizeof(PatternMatchData));
            pmd->next->prev = pmd;
            pmd = pmd->next;
        }
    }

    /* Set any non-zero default values here. */
    pmd->offset_var = BYTE_EXTRACT_NO_VAR;
    pmd->depth_var = BYTE_EXTRACT_NO_VAR;
    pmd->distance_var = BYTE_EXTRACT_NO_VAR;
    pmd->within_var = BYTE_EXTRACT_NO_VAR;

    return pmd;
}

void PatternMatchFree(void *d)
{
    PatternMatchData *pmd = (PatternMatchData *)d;

    if (pmd == NULL)
        return;

    (void)RemovePmdFromList(pmd);

    if (pmd->pattern_buf)
        free(pmd->pattern_buf);
    if (pmd->replace_buf)
        free(pmd->replace_buf);
    if(pmd->skip_stride)
        free(pmd->skip_stride);
    if(pmd->shift_stride)
        free(pmd->shift_stride);

    free(pmd);
}

static int32_t ParseInt(const char* data, const char* tag)
{
    int32_t value = 0;
    char *endptr = NULL;
    
    value = SnortStrtol(data, &endptr, 10);

    if (*endptr)
        ParseError("Invalid '%s' format.", tag);

    if (errno == ERANGE)
        ParseError("Range problem on '%s' value.", tag);

    if ((value > 65535) || (value < -65535))
        ParseError("'%s' must in -65535:65535", tag);

    return value;
}

/* Used for content modifiers that are used as rule options - need to get the
 * last pmd which is the one they are modifying.  If there isn't a last pmd
 * error that a content must be specified before the modifier */
static INLINE PatternMatchData * GetLastPmdError(OptTreeNode *otn, int type, const char *option)
{
    PatternMatchData *pmd = GetLastPmd(otn, type);

    if (pmd == NULL)
    {
        ParseError("Please place \"content\" rules before \"%s\" modifier",
                option == NULL ? "unknown" : option);
    }

    return pmd;
}

/* Gets the last pmd in the ds_list specified */
static INLINE PatternMatchData * GetLastPmd(OptTreeNode *otn, int type)
{
    PatternMatchData *pmd;

    if ((otn == NULL) || (otn->ds_list[type] == NULL))
        return NULL;

    for (pmd = otn->ds_list[type]; pmd->next != NULL; pmd = pmd->next);
    return pmd;
}


/* Options that can't be used with http content modifiers.  Additionally
 * http_inspect preprocessor needs to be enabled */
static void ValidateHttpContentModifiers(PatternMatchData *pmd)
{
    if (pmd == NULL)
        ParseError("Please place \"content\" rules before http content modifiers");

    if (!IsPreprocEnabled(PP_HTTPINSPECT))
    {
        ParseError("Please enable the HTTP Inspect preprocessor "
                "before using the http content modifiers");
    }

    if (pmd->replace_buf != NULL)
    {
        ParseError("\"replace\" option is not supported in conjunction with "
                "http content modifiers");
    }

    if (pmd->rawbytes == 1)
    {
        ParseError("Cannot use 'rawbytes' and http content as modifiers for "
                "the same \"content\"");
    }

    if ( URIBUFS_SET(pmd , (HTTP_SEARCH_URI | HTTP_SEARCH_RAW_URI)) )
    {
        ParseError("Cannot use 'http_uri' and 'http_raw_uri' modifiers for "
                "the same \"content\"");
    }

    if ( URIBUFS_SET(pmd , (HTTP_SEARCH_HEADER | HTTP_SEARCH_RAW_HEADER)) )
    {
        ParseError("Cannot use 'http_header' and 'http_raw_header' modifiers for "
                "the same \"content\"");
    }
    if ( URIBUFS_SET(pmd , (HTTP_SEARCH_COOKIE | HTTP_SEARCH_RAW_COOKIE)) )
    {
        ParseError("Cannot use 'http_cookie' and 'http_raw_cookie' modifiers for "
                "the same \"content\"");
    }
}

/* This is used if we get an http content modifier, since specifying "content"
 * defaults to the PLUGIN_PATTERN_MATCH list.  We need to move the pmd to the
 * PLUGIN_PATTERN_MATCH_URI list */
static void MovePmdToUriDsList(OptTreeNode *otn, PatternMatchData *pmd)
{
    int type = PLUGIN_PATTERN_MATCH_URI;

    /* It's not currently in the correct list */
    if (lastType != type)
    {
        /* Just in case it's moved from the middle of the list */
        if (pmd->prev != NULL)
            pmd->prev->next = pmd->next;
        if (pmd->next != NULL)
            pmd->next->prev = pmd->prev;

        /* Reset pointers */
        pmd->next = NULL;
        pmd->prev = NULL;

        if (otn->ds_list[type] == NULL)
        {
            otn->ds_list[type] = pmd;
        }
        else
        {
            /* Make it the last in the URI list */
            PatternMatchData *tmp;
            for (tmp = otn->ds_list[type]; tmp->next != NULL; tmp = tmp->next);
            tmp->next = pmd;
            pmd->prev = tmp;
        }

        /* Set the last type to the URI list */
        lastType = type;

        /* Reset these to URI type */
        pmd->fpl->OptTestFunc = CheckUriPatternMatch;
        pmd->fpl->type = RULE_OPTION_TYPE_CONTENT_URI;
        pmd->buffer_func = CHECK_URI_PATTERN_MATCH;
    }
}

#if 0
/* Not currently used */
static void PrintDupDOTPmds(PatternMatchData *pmd,
        PatternMatchData *pmd_dup, option_type_t type)
{
    int i;

    if ((pmd == NULL) || (pmd_dup == NULL))
        return;

    LogMessage("Duplicate %sContent:\n"
            "%d %d %d %d %d %d %d %d %d %d\n"
            "%d %d %d %d %d %d %d %d %d %d\n",
            option_type == RULE_OPTION_TYPE_CONTENT ? "" : "Uri",
            pmd->exception_flag,
            pmd->offset,
            pmd->depth,
            pmd->distance,
            pmd->within,
            pmd->rawbytes,
            pmd->nocase,
            pmd->use_doe,
            pmd->uri_buffer,
            pmd->pattern_max_jump_size,
            pmd_dup->exception_flag,
            pmd_dup->offset,
            pmd_dup->depth,
            pmd_dup->distance,
            pmd_dup->within,
            pmd_dup->rawbytes,
            pmd_dup->nocase,
            pmd_dup->use_doe,
            pmd_dup->uri_buffer,
            pmd_dup->pattern_max_jump_size);

    for (i = 0; i < pmd->pattern_size; i++)
        LogMessage("0x%x 0x%x", pmd->pattern_buf[i], pmd_dup->pattern_buf[i]);
    LogMessage("\n");
    for (i = 0; i < pmd->replace_size; i++)
        LogMessage("0x%x 0x%x", pmd->replace_buf[i], pmd_dup->replace_buf[i]);
    LogMessage("\n");
    LogMessage("\n");
}
#endif

/********************************************************************
 * Functions for detection option tree hashing and comparison
 * and other detection option tree uses
 ********************************************************************/
uint32_t PatternMatchHash(void *d)
{
    uint32_t a,b,c,tmp;
    unsigned int i,j,k,l;
    PatternMatchData *pmd = (PatternMatchData *)d;

    a = pmd->exception_flag;
    b = pmd->offset;
    c = pmd->depth;

    mix(a,b,c);

    a += pmd->distance;
    b += pmd->within;
    c += pmd->rawbytes;
    
    mix(a,b,c);

    a += pmd->nocase;
    b += pmd->use_doe;
    c += pmd->uri_buffer;
    
    mix(a,b,c);

    a += pmd->pattern_size;
    b += pmd->replace_size;
    c += pmd->pattern_max_jump_size;
    
    mix(a,b,c);

    for (i=0,j=0;i<pmd->pattern_size;i+=4)
    {
        tmp = 0;
        k = pmd->pattern_size - i;
        if (k > 4)
            k=4;
       
        for (l=0;l<k;l++)
        {
            tmp |= *(pmd->pattern_buf + i + l) << l*8;
        }

        switch (j)
        {
            case 0:
                a += tmp;
                break;
            case 1:
                b += tmp;
                break;
            case 2:
                c += tmp;
                break;
        }
        j++;

        if (j == 3)
        {
            mix(a,b,c);
            j = 0;
        }
    }

    for (i=0;i<pmd->replace_size;i+=4)
    {
        tmp = 0;
        k = pmd->replace_size - i;
        if (k > 4)
            k=4;
       
        for (l=0;l<k;l++)
        {
            tmp |= *(pmd->replace_buf + i + l) << l*8;
        }

        switch (j)
        {
            case 0:
                a += tmp;
                break;
            case 1:
                b += tmp;
                break;
            case 2:
                c += tmp;
                break;
        }
        j++;

        if (j == 3)
        {
            mix(a,b,c);
            j = 0;
        }
    }

    if (j != 0)
    {
        mix(a,b,c);
    }

    if (pmd->uri_buffer)
    {
        a += RULE_OPTION_TYPE_CONTENT_URI;
    }
    else
    {
        a += RULE_OPTION_TYPE_CONTENT;
    }

    b += pmd->fp;
    c += pmd->fp_only;

    mix(a,b,c);

    a += pmd->fp_offset;
    b += pmd->fp_length;
    c += pmd->offset_var;

    mix(a,b,c);

    a += pmd->depth_var;
    b += pmd->distance_var;
    c += pmd->within_var;

    final(a,b,c); 

    return c;
}

int PatternMatchCompare(void *l, void *r)
{
    PatternMatchData *left = (PatternMatchData *)l;
    PatternMatchData *right = (PatternMatchData *)r;
    unsigned int i;

    if (!left || !right)
        return DETECTION_OPTION_NOT_EQUAL;

    if (left->buffer_func != right->buffer_func)
        return DETECTION_OPTION_NOT_EQUAL;

    /* Sizes will be most different, check that first */
    if ((left->pattern_size != right->pattern_size) ||
        (left->replace_size != right->replace_size) ||
        (left->nocase != right->nocase))
        return DETECTION_OPTION_NOT_EQUAL;

    /* Next compare the patterns for uniqueness */
    if (left->pattern_size)
    {
        if (left->nocase)
        {
            /* If nocase is set, do case insensitive compare on pattern */
            for (i=0;i<left->pattern_size;i++)
            {
                if (toupper(left->pattern_buf[i]) != toupper(right->pattern_buf[i]))
                {
                    return DETECTION_OPTION_NOT_EQUAL;
                }
            }
        }
        else
        {
            /* If nocase is not set, do case sensitive compare on pattern */
            if (memcmp(left->pattern_buf, right->pattern_buf, left->pattern_size) != 0)
            {
                return DETECTION_OPTION_NOT_EQUAL;
            }
        }
    }

    /* Check the replace pattern if exists */
    if (left->replace_size)
    {
        if (memcmp(left->replace_buf, right->replace_buf, left->replace_size) != 0)
        {
            return DETECTION_OPTION_NOT_EQUAL;
        }
    }

    /* Now check the rest of the options */
    if ((left->exception_flag == right->exception_flag) &&
        (left->offset == right->offset) &&
        (left->depth == right->depth) &&
        (left->distance == right->distance) &&
        (left->within == right->within) &&
        (left->rawbytes == right->rawbytes) &&
        (left->use_doe == right->use_doe) &&
        (left->uri_buffer == right->uri_buffer) &&
        (left->search == right->search) &&
        (left->pattern_max_jump_size == right->pattern_max_jump_size) &&
        (left->fp == right->fp) &&
        (left->fp_only == right->fp_only) &&
        (left->fp_offset == right->fp_offset) &&
        (left->fp_length == right->fp_length) &&
        (left->offset_var == right->offset_var) &&
        (left->depth_var == right->depth_var) &&
        (left->distance_var == right->distance_var) &&
        (left->within_var == right->within_var) )
    {
        return DETECTION_OPTION_EQUAL;
    }

    return DETECTION_OPTION_NOT_EQUAL;
}

/* This function is called in parser.c after the rule has been
 * completely parsed */
void FinalizeContentUniqueness(OptTreeNode *otn)
{
    OptFpList *opt_fp = otn->opt_func;

    while (opt_fp)
    {
        if ((opt_fp->type == RULE_OPTION_TYPE_CONTENT)
                || (opt_fp->type == RULE_OPTION_TYPE_CONTENT_URI))
        {
            PatternMatchData *pmd = (PatternMatchData *)opt_fp->context;
            option_type_t option_type = opt_fp->type;
            void *pmd_dup;

            /* Since each content modifier can be parsed as a rule option,
             * do this check now that the entire rule has been parsed */
            if (option_type == RULE_OPTION_TYPE_CONTENT_URI)
                ValidateContent(pmd, PLUGIN_PATTERN_MATCH_URI);
            else
                ValidateContent(pmd, PLUGIN_PATTERN_MATCH);

            if (add_detection_option(option_type, (void *)pmd, &pmd_dup) == DETECTION_OPTION_EQUAL)
            {
                 /* Don't do anything if they are the same pointer.  This might happen when
                  * converting an so rule to a text rule via ConvertDynamicRule() in sf_convert_dynamic.c
                  * since we need to iterate through the RTN list in the OTN to verify that for http
                  * contents, the http_inspect preprocessor is enabled in the policy that is using a
                  * rule with http contents. */
                if (pmd != pmd_dup)
                {
#if 0
                 PrintDupDOTPmds(pmd, (PatternMatchData *)pmd_dup, option_type);
#endif

                    /* Hack since some places check for non-nullness of ds_list.
                    * Beware of iterating the pmd lists after this point since
                    * they'll be messed up - only check for non-nullness */
                    if (option_type == RULE_OPTION_TYPE_CONTENT)
                    {
                        if (pmd == otn->ds_list[PLUGIN_PATTERN_MATCH])
                            otn->ds_list[PLUGIN_PATTERN_MATCH] = pmd_dup;
                    }
                    else
                    {
                        if (pmd == otn->ds_list[PLUGIN_PATTERN_MATCH_URI])
                            otn->ds_list[PLUGIN_PATTERN_MATCH_URI] = pmd_dup;
                    }
    
                    PatternMatchFree(pmd);
                    opt_fp->context = pmd_dup;
                }
            }
#if 0
            else
            {
                LogMessage("Unique %sContent\n",
                    (opt_fp->OptTestFunc == CheckANDPatternMatch) ? "" : "Uri");
            }
#endif
        }

        opt_fp = opt_fp->next;
    }
}

void make_precomp(PatternMatchData * idx)
{
    if(idx->skip_stride)
       free(idx->skip_stride);
    if(idx->shift_stride)
       free(idx->shift_stride);

    idx->skip_stride = make_skip(idx->pattern_buf, idx->pattern_size);

    idx->shift_stride = make_shift(idx->pattern_buf, idx->pattern_size);
}

static char *PayloadExtractParameter(char *data, int *result_len)
{
    char *quote_one = NULL, *quote_two = NULL;
    char *comma = NULL;

    quote_one = index(data, '"');
    if (quote_one)
    {
        quote_two = index(quote_one+1, '"');
        while ( quote_two && quote_two[-1] == '\\' )
            quote_two = index(quote_two+1, '"');
    }

    if (quote_one && quote_two)
    {
        comma = index(quote_two, ',');
    }
    else if (!quote_one)
    {
        comma = index(data, ',');
    }

    if (comma)
    {
        *result_len = comma - data;
        *comma = '\0';
    }
    else
    {
        *result_len = strlen(data);
    }

    return data;
}

/* Since each content modifier can be parsed as a rule option, do this check
 * after parsing the entire rule in FinalizeContentUniqueness() */
static INLINE void ValidateContent(PatternMatchData *pmd, int type)
{
    if (pmd == NULL)
        return;

    if (pmd->fp)
    {
        if ((type == PLUGIN_PATTERN_MATCH_URI) && !IsHttpBufFpEligible(pmd->uri_buffer))

        {
            ParseError("Cannot use the fast_pattern content modifier for a lone "
                    "http cookie/http raw uri /http raw header /http raw cookie /status code / status msg buffer content.");
        }

        if (pmd->use_doe || (pmd->offset != 0) || (pmd->depth != 0))
        {
            if (pmd->exception_flag)
            {
                ParseError("Cannot use the fast_pattern modifier for negated, "
                        "relative or non-zero offset/depth content searches.");
            }

            if (pmd->fp_only)
            {
                ParseError("Fast pattern only contents cannot be relative or "
                        "have non-zero offset/depth content modifiers.");
            }
        }

        if (pmd->fp_only)
        {
            if (pmd->replace_buf != NULL)
            { 
                ParseError("Fast pattern only contents cannot use "
                        "replace modifier.");
            }

            if (pmd->exception_flag)
                ParseError("Fast pattern only contents cannot be negated.");
        }
    }

    if (type == PLUGIN_PATTERN_MATCH_URI)
        ValidateHttpContentModifiers(pmd);
}

/****************************************************************************
 *
 * Function: GetMaxJumpSize(char *, int)
 *
 * Purpose: Find the maximum number of characters we can jump ahead
 *          from the current offset when checking for this pattern again.
 *
 * Arguments: data => the pattern string
 *            data_len => length of pattern string
 *
 * Returns: int => number of bytes before pattern repeats within itself
 *
 ***************************************************************************/
static unsigned int GetMaxJumpSize(char *data, int data_len)
{
    int i, j;
    
    j = 0;
    for ( i = 1; i < data_len; i++ )
    {
        if ( data[j] != data[i] )
        {
            j = 0;
            continue;
        }
        if ( i == (data_len - 1) )
        {
            return (data_len - j - 1);
        }
        j++;
    }
    return data_len;
}

/****************************************************************************
 *
 * Function: ParsePattern(char *)
 *
 * Purpose: Process the application layer patterns and attach them to the
 *          appropriate rule.  My god this is ugly code.
 *
 * Arguments: rule => the pattern string
 *
 * Returns: void function
 *
 ***************************************************************************/
void ParsePattern(char *rule, OptTreeNode * otn, int type)
{
    char tmp_buf[MAX_PATTERN_SIZE];

    /* got enough ptrs for you? */
    char *start_ptr;
    char *end_ptr;
    char *idx;
    char *dummy_idx;
    char *dummy_end;
    char *tmp;
    char hex_buf[3];
    u_int dummy_size = 0;
    int size;
    int hexmode = 0;
    int hexsize = 0;
    int pending = 0;
    int cnt = 0;
    int literal = 0;
    int exception_flag = 0;
    PatternMatchData *ds_idx;

    /* clear out the temp buffer */
    bzero(tmp_buf, MAX_PATTERN_SIZE);

    if (rule == NULL)
        ParseError("ParsePattern Got Null enclosed in quotation marks (\")!");

    while(isspace((int)*rule))
        rule++;

    if(*rule == '!')
    {
        exception_flag = 1;
        while(isspace((int)*++rule));
    }

    /* find the start of the data */
    start_ptr = index(rule, '"');

    if (start_ptr != rule)
        ParseError("Content data needs to be enclosed in quotation marks (\")!");

    /* move the start up from the beggining quotes */
    start_ptr++;

    /* find the end of the data */
    end_ptr = strrchr(start_ptr, '"');

    if (end_ptr == NULL)
        ParseError("Content data needs to be enclosed in quotation marks (\")!");

    /* Move the null termination up a bit more */
    *end_ptr = '\0';

    /* Is there anything other than whitespace after the trailing
     * double quote? */
    tmp = end_ptr + 1;
    while (*tmp != '\0' && isspace ((int)*tmp))
        tmp++;

    if (strlen (tmp) > 0)
    {
        ParseError("Bad data (possibly due to missing semicolon) after "
                "trailing double quote.");
    }

    /* how big is it?? */
    size = end_ptr - start_ptr;

    /* uh, this shouldn't happen */
    if (size <= 0)
        ParseError("Bad pattern length!");

    /* set all the pointers to the appropriate places... */
    idx = start_ptr;

    /* set the indexes into the temp buffer */
    dummy_idx = tmp_buf;
    dummy_end = (dummy_idx + size);

    /* why is this buffer so small? */
    bzero(hex_buf, 3);
    memset(hex_buf, '0', 2);

    /* BEGIN BAD JUJU..... */
    while(idx < end_ptr)
    {
        if (dummy_size >= MAX_PATTERN_SIZE-1)
        {
            /* Have more data to parse and pattern is about to go beyond end of buffer */
            ParseError("ParsePattern() dummy buffer overflow, make a smaller "
                    "pattern please! (Max size = %d)", MAX_PATTERN_SIZE-1);
        }

        DEBUG_WRAP(DebugMessage(DEBUG_PARSER, "processing char: %c\n", *idx););
        switch(*idx)
        {
            case '|':
                DEBUG_WRAP(DebugMessage(DEBUG_PARSER, "Got bar... "););
                if(!literal)
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_PARSER, "not in literal mode... "););
                    if(!hexmode)
                    {
                        DEBUG_WRAP(DebugMessage(DEBUG_PARSER, "Entering hexmode\n"););
                        hexmode = 1;
                    }
                    else
                    {
                        DEBUG_WRAP(DebugMessage(DEBUG_PARSER, "Exiting hexmode\n"););

                        /*
                        **  Hexmode is not even.
                        */
                        if(!hexsize || hexsize % 2)
                        {
                            ParseError("Content hexmode argument has invalid "
                                    "number of hex digits.  The argument '%s' "
                                    "must contain a full even byte string.", start_ptr);
                        }

                        hexmode = 0;
                        pending = 0;
                    }

                    if(hexmode)
                        hexsize = 0;
                }
                else
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_PARSER, "literal set, Clearing\n"););
                    literal = 0;
                    tmp_buf[dummy_size] = start_ptr[cnt];
                    dummy_size++;
                }

                break;

            case '\\':
                DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, "Got literal char... "););

                if(!literal)
                {
                    /* Make sure the next char makes this a valid
                     * escape sequence.
                     */
                    if (idx [1] != '\0' && strchr ("\\\":;", idx [1]) == NULL)
                    {
                        ParseError("Bad escape sequence starting with \"%s\".", idx);
                    }

                    DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, "Setting literal\n"););

                    literal = 1;
                }
                else
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, "Clearing literal\n"););
                    tmp_buf[dummy_size] = start_ptr[cnt];
                    literal = 0;
                    dummy_size++;
                }

                break;
            case '"':
                if (!literal)
                    ParseError("Non-escaped '\"' character!");
                /* otherwise process the character as default */
            default:
                if(hexmode)
                {
                    if(isxdigit((int) *idx))
                    {
                        hexsize++;

                        if(!pending)
                        {
                            hex_buf[0] = *idx;
                            pending++;
                        }
                        else
                        {
                            hex_buf[1] = *idx;
                            pending--;

                            if(dummy_idx < dummy_end)
                            {                            
                                tmp_buf[dummy_size] = (u_char) 
                                    strtol(hex_buf, (char **) NULL, 16)&0xFF;

                                dummy_size++;
                                bzero(hex_buf, 3);
                                memset(hex_buf, '0', 2);
                            }
                            else
                            {
                                ParseError("ParsePattern() dummy buffer "
                                        "overflow, make a smaller pattern "
                                        "please! (Max size = %d)", MAX_PATTERN_SIZE-1);
                            }
                        }
                    }
                    else
                    {
                        if(*idx != ' ')
                        {
                            ParseError("What is this \"%c\"(0x%X) doing in "
                                    "your binary buffer?  Valid hex values "
                                    "only please! (0x0 - 0xF) Position: %d",
                                    (char) *idx, (char) *idx, cnt);
                        }
                    }
                }
                else
                {
                    if(*idx >= 0x1F && *idx <= 0x7e)
                    {
                        if(dummy_idx < dummy_end)
                        {
                            tmp_buf[dummy_size] = start_ptr[cnt];
                            dummy_size++;
                        }
                        else
                        {
                            ParseError("ParsePattern() dummy buffer "
                                    "overflow, make a smaller pattern "
                                    "please! (Max size = %d)", MAX_PATTERN_SIZE-1);
                        }

                        if(literal)
                        {
                            literal = 0;
                        }
                    }
                    else
                    {
                        if(literal)
                        {
                            tmp_buf[dummy_size] = start_ptr[cnt];
                            dummy_size++;
                            DEBUG_WRAP(DebugMessage(DEBUG_PARSER, "Clearing literal\n"););
                            literal = 0;
                        }
                        else
                        {
                            ParseError("Character value out of range, try a "
                                    "binary buffer.");
                        }
                    }
                }

                break;
        }

        dummy_idx++;
        idx++;
        cnt++;
    }
    /* ...END BAD JUJU */

    /* error prunning */

    if (literal)
        ParseError("Backslash escape is not completed.");

    if (hexmode)
        ParseError("Hexmode is not completed.");

    ds_idx = (PatternMatchData *) otn->ds_list[type];

    while(ds_idx->next != NULL)
        ds_idx = ds_idx->next;

    ds_idx->pattern_buf = (char *)SnortAlloc(dummy_size+1);
    memcpy(ds_idx->pattern_buf, tmp_buf, dummy_size);

    ds_idx->pattern_size = dummy_size;
    ds_idx->search = uniSearch;
    
    make_precomp(ds_idx);
    ds_idx->exception_flag = exception_flag;

    ds_idx->pattern_max_jump_size = GetMaxJumpSize(ds_idx->pattern_buf, ds_idx->pattern_size);
}

/********************************************************************
 * Runtime functions
 ********************************************************************/
/*
 * Figure out how deep the into the packet from the base_ptr we can go
 *
 * base_ptr = the offset into the payload relative to the last match plus the offset
 *            contained within the current pmd
 *
 * dlen = amount of data in the packet from the base_ptr to the end of the packet
 *
 * pmd = the patterm match data struct for this test
 */
static INLINE int computeWithin(int dlen, PatternMatchData *pmd)
{
    /* do we want to check more bytes than there are in the buffer? */
    if(pmd->within > (unsigned int)dlen)
    {
        /* should we just return -1 here since the data might actually be within 
         * the stream but not the current packet's payload?
         */
        
        /* if the buffer size is greater than the size of the pattern to match */
        if(dlen >= (int)pmd->pattern_size)
        {
            /* return the size of the buffer */
            return dlen;
        }
        else
        {
            /* failed, pattern size is greater than number of bytes in the buffer */
            return -1;
        }
    }

    /* the within vaule is in range of the number of buffer bytes */
    return pmd->within;
}

/* 
 * case sensitive search
 *
 * data = ptr to buffer to search
 * dlen = distance to the back of the buffer being tested, validated 
 *        against offset + depth before function entry (not distance/within)
 * pmd = pointer to pattern match data struct
 */

static int uniSearch(const char *data, int dlen, PatternMatchData *pmd)
{
    return uniSearchReal(data, dlen, pmd, 0);
}

/* 
 * case insensitive search
 *
 * data = ptr to buffer to search
 * dlen = distance to the back of the buffer being tested, validated 
 *        against offset + depth before function entry (not distance/within)
 * pmd = pointer to pattern match data struct
 *
 * NOTE - this is used in sf_convert_dynamic.c so cannot be static
 */
int uniSearchCI(const char *data, int dlen, PatternMatchData *pmd)
{
    return uniSearchReal(data, dlen, pmd, 1);
}

/* 
 * single search function. 
 *
 * data = ptr to buffer to search
 * dlen = distance to the back of the buffer being tested, validated 
 *        against offset + depth before function entry (not distance/within)
 * pmd = pointer to pattern match data struct
 * nocase = 0 means case sensitve, 1 means case insensitive
 *
 * return  1 for found
 * return  0 for not found
 * return -1 for error (search out of bounds)
 */       
static int uniSearchReal(const char *data, int dlen, PatternMatchData *pmd, int nocase)
{
    /* 
     * in theory computeDepth doesn't need to be called because the 
     * depth + offset adjustments have been made by the calling function
     */
    int depth = dlen;
    int old_depth = dlen;
    int success = 0;
    const char *start_ptr = data;
    const char *end_ptr = data + dlen;
    const char *base_ptr = start_ptr;
    uint32_t extract_offset, extract_depth, extract_distance, extract_within;
    
    DEBUG_WRAP(char *hexbuf;);


    if(pmd->use_doe != 1)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                                "NOT Using Doe Ptr\n"););
        UpdateDoePtr(NULL, 0); /* get rid of all our pattern match state */
    }

    /* Get byte_extract variables */
    if (pmd->offset_var >= 0 && pmd->offset_var < NUM_BYTE_EXTRACT_VARS)
    {
        GetByteExtractValue(&extract_offset, pmd->offset_var);
        pmd->offset = (int) extract_offset;
    }
    if (pmd->depth_var >= 0 && pmd->depth_var < NUM_BYTE_EXTRACT_VARS)
    {
        GetByteExtractValue(&extract_depth, pmd->depth_var);
        pmd->depth = (int) extract_depth;
    }
    if (pmd->distance_var >= 0 && pmd->distance_var < NUM_BYTE_EXTRACT_VARS)
    {
        GetByteExtractValue(&extract_distance, pmd->distance_var);
        pmd->distance = (int) extract_distance;
    }
    if (pmd->within_var >= 0 && pmd->within_var < NUM_BYTE_EXTRACT_VARS)
    {
        GetByteExtractValue(&extract_within, pmd->within_var);
        pmd->within = (u_int) extract_within;
    }

    /* check to see if we've got a stateful start point */
    if(doe_ptr)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                                "Using Doe Ptr\n"););

        base_ptr = (const char *)doe_ptr;
        depth = dlen - ((char *) doe_ptr - data);
    }
    else
    {
        base_ptr = start_ptr;
        depth = dlen;
    }

    /* if we're using a distance call */
    if(pmd->distance)
    {
        /* set the base pointer up for the distance */
        base_ptr += pmd->distance;
        depth -= pmd->distance;
    }
    else /* otherwise just use the offset (validated by calling function) */
    {
        base_ptr += pmd->offset;
        depth -= pmd->offset;
    }
    
    if(pmd->within != 0)
    {
        /* 
         * calculate the "real" depth based on the current base and available
         * number of bytes in the buffer
         *
         * this should account for the current base_ptr as it relates to 
         * the back of the buffer being tested
         */
        old_depth = depth;
        
        depth = computeWithin(depth, pmd);
        
        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, "Changing Depth from %d to %d\n", old_depth, depth););
    }

    /* make sure we and in range */
    if(!inBounds((const uint8_t *)start_ptr, (const uint8_t *)end_ptr, (const uint8_t *)base_ptr))
    {
        
        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                                "returning because base_ptr"
                                " is out of bounds start_ptr: %p end: %p base: %p\n",
                                start_ptr, end_ptr, base_ptr););
        return -1;
    }

    if(depth < 0)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                                "returning because depth is negative (%d)\n",
                                depth););
        return -1;        
    }

    if(depth > dlen)
    {
        /* if offsets are negative but somehow before the start of the
           packet, let's make sure that we get everything going
           straight */
        depth = dlen;
    }

    if((pmd->depth > 0) && (depth > pmd->depth))
    {
        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                                "Setting new depth to %d from %d\n",
                                pmd->depth, depth););

        depth = pmd->depth;
    }
    
    /* make sure we end in range */
    if(!inBounds((const uint8_t *)start_ptr, (const uint8_t *)end_ptr, (const uint8_t *)(base_ptr + depth - 1)))
    {
        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                                "returning because base_ptr + depth - 1"
                                " is out of bounds start_ptr: %p end: %p base: %p\n",
                                start_ptr, end_ptr, base_ptr););
        return 0;
    }

#ifdef DEBUG
    assert(depth <= old_depth);

    DebugMessage(DEBUG_PATTERN_MATCH, "uniSearchReal:\n ");

    hexbuf = hex((u_char *)pmd->pattern_buf, pmd->pattern_size);
    DebugMessage(DEBUG_PATTERN_MATCH, "   p->data: %p\n   doe_ptr: %p\n   "
                 "base_ptr: %p\n   depth: %d\n   searching for: %s\n", 
                 data, doe_ptr, base_ptr, depth, hexbuf);
    free(hexbuf);
#endif /* DEBUG */
    
    if(nocase)
    {
        success = mSearchCI(base_ptr, depth, 
                            pmd->pattern_buf,
                            pmd->pattern_size,
                            pmd->skip_stride, 
                            pmd->shift_stride);
    }
    else
    {
        success = mSearch(base_ptr, depth,
                          pmd->pattern_buf,
                          pmd->pattern_size,
                          pmd->skip_stride,
                          pmd->shift_stride);
    }


#ifdef DEBUG
    if(success)
    {
        DebugMessage(DEBUG_PATTERN_MATCH, "matched, doe_ptr: %p (%d)\n", 
                     doe_ptr, ((char *)doe_ptr - data));
    }
#endif

    return success;
}

int CheckANDPatternMatch(void *option_data, Packet *p)
{
    int rval = DETECTION_OPTION_NO_MATCH;
    int found = 0;
    int dsize;
    char *dp;
    int origUseDoe;
    char *orig_doe;
    PatternMatchData *idx;
    PROFILE_VARS;

    PREPROC_PROFILE_START(contentPerfStats);

    DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, "CheckPatternANDMatch: "););

    idx = (PatternMatchData *)option_data;
    origUseDoe = idx->use_doe;
   
    if(idx->rawbytes == 0) 
    {
        if(IsMimeDecodeBuf(doe_ptr))
        {
            dsize = mime_decode_size;
            dp = (char *)file_data_ptr;
            DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                        "Using MIME Decode Buffer!\n"););
        }
        else if(IsBase64DecodeBuf(doe_ptr) )
        {
            dsize = base64_decode_size;
            dp = (char *)base64_decode_buf;
            DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                        "Using Base64 Decode Buffer!\n"););
        }
        else if((p->packet_flags & PKT_ALT_DECODE))
        {
            dsize = DecodeBuffer.len;
            dp = (char *) DecodeBuffer.data; /* decode.c */
            DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, 
                        "Using Alternative Decode buffer!\n"););
        }
        else
        {
            if(IsLimitedDetect(p))
            {
                dsize = p->alt_dsize;
                DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, 
                    "Using Limited Packet Data!\n"););
            }
            else
            {
                dsize = p->dsize;
                DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, 
                    "Using Full Packet Data!\n"););
            }
            dp = (char *) p->data;
        }
    } /*Check if the packet is a HTTP Response and is not compressed*/
    else if(p->packet_flags & PKT_HTTP_RESP_BODY)
    {
        u_char *end_of_packet = (u_char *) (p->data + p->dsize);
        /* check if the file_data_ptr is within the server_flow_depth of HttpInspect */
        if(file_data_ptr <= end_of_packet)
        {
            if((end_of_packet - file_data_ptr) < p->alt_dsize )
            {
                dsize = end_of_packet - file_data_ptr;
                DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, 
                    "Using Limited Packet Data!\n"););
            }
            else
            {
                dsize = p->alt_dsize;
                DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, 
                    "Using Full File Data!\n"););
            }
            dp = (char *)file_data_ptr;
        }
        else
        {
            PREPROC_PROFILE_END(contentPerfStats);
            return rval;
        }
        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, 
                    "Using HTTP RESP BODY buffer (raw bytes)!\n"););
    }
    else
    {
        dsize = p->dsize;
        dp = (char *) p->data;
        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, 
            "Using Full Packet Data!\n"););
    }

    /* this now takes care of all the special cases where we'd run
     * over the buffer */
    orig_doe = (char *)doe_ptr;

    if(doe_buf_flags & DOE_BUF_URI)
        UpdateDoePtr(NULL, 0);

    doe_buf_flags = DOE_BUF_STD;

#ifndef NO_FOUND_ERROR
    found = idx->search(dp, dsize, idx);
    if ( found == -1 )
    {
        /* On error, mark as not found.  This is necessary to handle !content
           cases.  In that case, a search that is outside the given buffer will
           return 0, and !0 is 1, so a !content out of bounds will return true,
           which is not what we want.  */
        found = 0;
    }
    else
    {
        found = found ^ idx->exception_flag;
    }
#else
    /* Original code.  Does not account for searching outside the buffer. */
    found = (idx->search(dp, dsize, idx) ^ idx->exception_flag);
#endif

    if ( found )
    {
        if ( idx->replace_buf && !PacketWasCooked(p) )
        {
            //fix the packet buffer to have the new string
            int detect_depth = (char *)doe_ptr - idx->pattern_size - dp;

            if (detect_depth < 0)
            {
                PREPROC_PROFILE_END(contentPerfStats);
                return rval;
            }
            Replace_StoreOffset(idx, detect_depth);
        }
        rval = DETECTION_OPTION_MATCH;
        DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "Pattern match found\n"););
    }
    else
    {
        DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "Pattern match failed\n"););
    }
#if 0
    while (found)
    {
        /* save where we last did the pattern match */
        tmp_doe = (char *)doe_ptr;

        /* save start doe as beginning of this pattern + non-repeating length*/
        start_doe = (char *)doe_ptr - idx->pattern_size + idx->pattern_max_jump_size;

        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, "Pattern Match successful!\n"););      
        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, "Check next functions!\n"););
        /* PROFILING Don't count rest of options towards content */
        PREPROC_PROFILE_TMPEND(contentPerfStats);

        /* Try evaluating the rest of the rules chain */
        next_found= fp_list->next->OptTestFunc(p, otn_idx, fp_list->next);

        /* PROFILING Don't count rest of options towards content */
        PREPROC_PROFILE_TMPSTART(contentPerfStats);

        if(next_found != 0) 
        {
            DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, 
                                    "Next functions matched!\n"););

            /* We found a successful match, return that this rule has fired off */
            PREPROC_PROFILE_END(contentPerfStats);
            return next_found;
        }
        else if(tmp_doe != NULL)
        {
            int new_dsize = dsize-(start_doe-dp);

            /* if the next option isn't relative and it failed, we're done */
            if (fp_list->next->isRelative == 0)
            {
                PREPROC_PROFILE_END(contentPerfStats);
                return 0;
            }

            if(new_dsize <= 0 || new_dsize > dsize)
            {
                DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, 
                                        "The new dsize is less than <= 0 or > "
                                        "the the original dsize;returning "
                                        "false\n"););
                idx->use_doe = origUseDoe;
                PREPROC_PROFILE_END(contentPerfStats);
                return 0;
            }

            if (orig_doe)
            {
                /* relative to a previously found pattern */
                if (((idx->distance != 0) && (start_doe - orig_doe > idx->distance)) ||
                    ((idx->offset != 0) && (start_doe - orig_doe > idx->offset)) )
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, 
                                            "The next starting point to search "
                                            "from is beyond the original "
                                            "distance;returning false\n"););
                    idx->use_doe = origUseDoe;
                    PREPROC_PROFILE_END(contentPerfStats);
                    return 0;
                }

                if (((idx->within != 0) &&
                     (start_doe - orig_doe + idx->pattern_size > (unsigned int)idx->within)) ||
                    ((idx->depth != 0) &&
                     (start_doe - orig_doe + idx->pattern_size > (unsigned int)idx->depth)) )
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, 
                                            "The next starting point to search "
                                            "from is beyond the original "
                                            "within;returning false\n"););
                    idx->use_doe = origUseDoe;
                    PREPROC_PROFILE_END(contentPerfStats);
                    return 0;
                }
            }
            else
            {
                /* relative to beginning of data */
                if (((idx->distance != 0) && (start_doe - dp > idx->distance)) ||
                    ((idx->offset != 0) && (start_doe - dp > idx->offset)) )
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, 
                                            "The next starting point to search "
                                            "from is beyond the original "
                                            "distance;returning false\n"););
                    idx->use_doe = origUseDoe;
                    PREPROC_PROFILE_END(contentPerfStats);
                    return 0;
                }

                if (((idx->within != 0) &&
                     (start_doe - dp + idx->pattern_size > (unsigned int)idx->within)) ||
                    ((idx->depth != 0) &&
                     (start_doe - dp + idx->pattern_size > (unsigned int)idx->depth)) )
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, 
                                            "The next starting point to search "
                                            "from is beyond the original "
                                            "within;returning false\n"););
                    idx->use_doe = origUseDoe;
                    PREPROC_PROFILE_END(contentPerfStats);
                    return 0;
                }
            }

            DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, 
                                    "At least ONE of the next functions does to match!\n"););      
            DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, 
                                    "Start search again from a next point!\n"););

            /* Start the search again from the last set of contents, with a new depth and dsize */
            doe_ptr = (uint8_t *)start_doe;
            idx->use_doe = 1;
            found = (idx->search(start_doe, new_dsize,idx) ^ idx->exception_flag);
            
            /*
            **  If we haven't updated doe since we set it at the beginning
            **  of the loop, then that means we have already done the exact 
            **  same search previously, and have nothing else to gain from
            **  doing the same search again.
            */
            if(start_doe == (char *)doe_ptr)
            {
                idx->use_doe = origUseDoe;
                PREPROC_PROFILE_END(contentPerfStats);
                return 0;
            }
        }
        else
        {
            DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, 
                                    "Returning 0 because tmp_doe is NULL\n"););
            
            idx->use_doe = origUseDoe;
            PREPROC_PROFILE_END(contentPerfStats);
            return 0;
        }
        
    }
#endif
    
    //idx->use_doe = origUseDoe;
    PREPROC_PROFILE_END(contentPerfStats);
    return rval;
}

int CheckUriPatternMatch(void *option_data, Packet *p)
{
    int rval = DETECTION_OPTION_NO_MATCH;
    int found = 0;
    int i = 0;
    PatternMatchData *idx = (PatternMatchData *)option_data;
    PROFILE_VARS;

    if(p->uri_count <= 0)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_HTTP_DECODE,"CheckUriPatternMatch: no "
            "HTTP buffers set, retuning"););
        return rval;
    }

    PREPROC_PROFILE_START(uricontentPerfStats);
    for (i = 0; i<p->uri_count; i++)

    {
        DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "CheckUriPatternMatch: "););

        if (!UriBufs[i].uri || (UriBufs[i].length == 0))
        {
            DEBUG_WRAP(DebugMessage(DEBUG_HTTP_DECODE,"Checking for %s pattern in "
                "buffer %d: HTTP buffer not set/zero length, returning",
                uri_buffer_name[i], i););
            continue;
        }

        if (!(idx->uri_buffer & (1 << i)))
        {
            DEBUG_WRAP(DebugMessage(DEBUG_HTTP_DECODE,"Skipping %s pattern in "
                "buffer %d: buffer not part of inspection set",
                uri_buffer_name[i], i););
            continue;
        }

        DEBUG_WRAP(DebugMessage(DEBUG_HTTP_DECODE,"Checking for %s pattern in "
                "buffer %d ",
                uri_buffer_name[i], i););

#ifdef DEBUG /* for variable declaration */
        {
            int j;
    
            DebugMessage(DEBUG_HTTP_DECODE,"Checking against HTTP data (%s): ", uri_buffer_name[idx->uri_buffer]);
            for(j=0; j<UriBufs[i].length; j++)
            {
                DebugMessage(DEBUG_HTTP_DECODE, "%c", UriBufs[i].uri[j]);
            }
            DebugMessage(DEBUG_HTTP_DECODE,"\n");
        }
#endif /* DEBUG */

        /* 
        * have to reset the doe_ptr for each new UriBuf 
        */
        if(idx->use_doe != 1)
            UpdateDoePtr(NULL, 0);

        else if(!(doe_buf_flags & DOE_BUF_URI))
            SetDoePtr(UriBufs[i].uri, DOE_BUF_URI);
    
        /* this now takes care of all the special cases where we'd run
         * over the buffer */
        found = (idx->search((const char *)UriBufs[i].uri, UriBufs[i].length, idx) ^ idx->exception_flag);
    
        if(found > 0 )
        {
            doe_buf_flags = DOE_BUF_URI; // FIXTHIS why is this necessary?

            DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, "Pattern Match successful!\n"););
            /*if found print the normalized and unnormalized buffer */
#ifdef DEBUG
            if ( idx->uri_buffer & (HTTP_SEARCH_URI | HTTP_SEARCH_COOKIE | HTTP_SEARCH_HEADER ))
            {
                DEBUG_WRAP(
                    if(UriBufs[i].uri)
                        DebugMessage(DEBUG_HTTP_DECODE, "Normalized contents of the matched Http buffer is: %s\n",
                                    UriBufs[i].uri);
                    if(UriBufs[i+1].uri)
                        DebugMessage(DEBUG_HTTP_DECODE, "Unnormalized/Raw contents of the matched Http buffer is: %s\n",
                                    UriBufs[i+1].uri);
                        );
            }
            else
            {
                DEBUG_WRAP(
                        if(UriBufs[i].uri)
                            DebugMessage(DEBUG_HTTP_DECODE, "Unnormalized/Raw contents of the matched Http buffer is: %s\n",
                                UriBufs[i].uri);
                        );
            }
#endif
            /* call the next function in the OTN */
            PREPROC_PROFILE_END(uricontentPerfStats);
            return DETECTION_OPTION_MATCH;
        }

        DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "Pattern match failed\n"););
    }

    PREPROC_PROFILE_END(uricontentPerfStats);
    return rval;
}

void PatternMatchDuplicatePmd(void *src, PatternMatchData *pmd_dup)
{
    /* Oh, C++ where r u?  can't we have a friggin' copy constructor? */
    PatternMatchData *pmd_src = (PatternMatchData *)src;
    if (!pmd_src || !pmd_dup)
        return;

    pmd_dup->exception_flag = pmd_src->exception_flag;
    pmd_dup->offset = pmd_src->offset;
    pmd_dup->depth = pmd_src->depth;
    pmd_dup->distance = pmd_src->distance;
    pmd_dup->within = pmd_src->within;
    pmd_dup->offset_var = pmd_src->offset_var;
    pmd_dup->depth_var = pmd_src->depth_var;
    pmd_dup->distance_var = pmd_src->distance_var;
    pmd_dup->within_var = pmd_src->within_var;
    pmd_dup->rawbytes = pmd_src->rawbytes;
    pmd_dup->nocase = pmd_src->nocase;
    pmd_dup->use_doe = pmd_src->use_doe;
    pmd_dup->uri_buffer = pmd_src->uri_buffer;
    pmd_dup->buffer_func = pmd_src->buffer_func;
    pmd_dup->pattern_size = pmd_src->pattern_size;
    pmd_dup->replace_size = pmd_src->replace_size;
    pmd_dup->replace_buf = pmd_src->replace_buf;
    pmd_dup->pattern_buf = pmd_src->pattern_buf;
    pmd_dup->search = pmd_src->search;
    pmd_dup->skip_stride = pmd_src->skip_stride;
    pmd_dup->shift_stride = pmd_src->shift_stride;
    pmd_dup->pattern_max_jump_size = pmd_src->pattern_max_jump_size;
    pmd_dup->fp = pmd_src->fp;
    pmd_dup->fp_only = pmd_src->fp_only;
    pmd_dup->fp_offset = pmd_src->fp_offset;
    pmd_dup->fp_length = pmd_src->fp_length;

    pmd_dup->last_check.ts.tv_sec = pmd_src->last_check.ts.tv_sec;
    pmd_dup->last_check.ts.tv_usec = pmd_src->last_check.ts.tv_usec;
    pmd_dup->last_check.packet_number = pmd_src->last_check.packet_number;
    pmd_dup->last_check.rebuild_flag = pmd_src->last_check.rebuild_flag;

    pmd_dup->prev = NULL;
    pmd_dup->next = NULL;
    pmd_dup->fpl = NULL;

    Replace_ResetOffset(pmd_dup);
}

/* current_cursor should be the doe_ptr after this content rule option matched
 * orig_cursor is the place from where we first did evaluation of this content */
int PatternMatchAdjustRelativeOffsets(PatternMatchData *orig_pmd, PatternMatchData *dup_pmd,
        const u_int8_t *current_cursor, const u_int8_t *orig_cursor)
{
    /* Adjust for repeating patterns, e.g. ABAB
     * This is where the new search for this content should start */
    const uint8_t *start_cursor =
        (current_cursor - dup_pmd->pattern_size) + dup_pmd->pattern_max_jump_size;

    if (orig_pmd->depth != 0)
    {
        /* This was relative to a previously found pattern.  No space left to
         * search, we're done */
        if ((start_cursor + dup_pmd->pattern_size)
                > (orig_cursor + dup_pmd->offset + dup_pmd->depth))
        {
            return 0;
        }

        /* Adjust offset and depth to reflect new position */
        /* Lop off what we used */
        dup_pmd->depth -= start_cursor - (orig_cursor + dup_pmd->offset);
        /* Make offset where we will start the next search */
        dup_pmd->offset = start_cursor - orig_cursor;
    }
    else if (orig_pmd->within != 0)
    {
        /* This was relative to a previously found pattern.  No space left to
         * search, we're done */
        if ((start_cursor + dup_pmd->pattern_size)
                > (orig_cursor + dup_pmd->distance + dup_pmd->within))
        {
            return 0;
        }

        /* Adjust distance and within to reflect new position */
        /* Lop off what we used */
        dup_pmd->within -= start_cursor - (orig_cursor + dup_pmd->distance);
        /* Make distance where we will start the next search */
        dup_pmd->distance = start_cursor - orig_cursor;
    }
    else if (orig_pmd->use_doe)
    {
        dup_pmd->distance = start_cursor - orig_cursor;
    }
    else
    {
        dup_pmd->offset = start_cursor - orig_cursor;
    }

    return 1;
}

#if 0
/* Not currently in use - DO NOT REMOVE */
static INLINE int computeDepth(int dlen, PatternMatchData * pmd) 
{
    /* do some tests to make sure we stay in bounds */
    if((pmd->depth + pmd->offset) > dlen)
    {
        /* we want to check only depth bytes anyway */
        int sub_depth = dlen - pmd->offset; 

        if((sub_depth > 0) && (sub_depth >= (int)pmd->pattern_size))
        {
            return  sub_depth;
        }
        else
        {
            DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, 
                        "Pattern Match failed -- sub_depth: %d < "
                        "(int)pmd->pattern_size: %d!\n",
                        sub_depth, (int)pmd->pattern_size););

            return -1;
        }
    }
    else
    {      
        if(pmd->depth && (dlen - pmd->offset > pmd->depth))
        {
            return pmd->depth;
        }
        else
        {
            return dlen - pmd->offset;
        }
    }
}

static int uniSearchREG(char * data, int dlen, PatternMatchData * pmd)
{
    int depth = computeDepth(dlen, pmd);
    /* int distance_adjustment = 0;
     *  int depth_adjustment = 0;
     */
    int success = 0;

    if (depth < 0)
        return 0;

    /* XXX DESTROY ME */
    /*success =  mSearchREG(data + pmd->offset + distance_adjustment, 
            depth_adjustment!=0?depth_adjustment:depth, 
            pmd->pattern_buf, pmd->pattern_size, pmd->skip_stride, 
            pmd->shift_stride);*/

    return success;
}
#endif

#if 0
/* XXX Not completetly implemented */
static void PayloadSearchListInit(char *data, OptTreeNode * otn, int protocol)
{
    char *sptr;
    char *eptr;

    lastType = PLUGIN_PATTERN_MATCH_OR;

    DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, "In PayloadSearchListInit()\n"););

    /* get the path/file name from the data */
    while(isspace((int) *data))
        data++;

    /* grab everything between the starting " and the end one */
    sptr = index(data, '"');
    eptr = strrchr(data, '"');

    if(sptr != NULL && eptr != NULL)
    {
        /* increment past the first quote */
        sptr++;

        /* zero out the second one */
        *eptr = 0;
    }
    else
    {
        sptr = data;
    }

    /* read the content keywords from the list file */
    ParseContentListFile(sptr, otn, protocol);

    /* link the plugin function in to the current OTN */
    AddOptFuncToList(CheckORPatternMatch, otn);

    return;
}

/****************************************************************************
 *
 * Function: ParseContentListFile(char *, OptTreeNode *, int protocol)
 *
 * Purpose:  Read the content_list file a line at a time, put the content of
 *           the line into buffer
 *
 * Arguments:otn => rule including the list
 *           file => list file filename
 *           protocol => protocol
 *
 * Returns: void function
 *
 ***************************************************************************/
static void ParseContentListFile(char *file, OptTreeNode * otn, int protocol)
{
    FILE *thefp;                /* file pointer for the content_list file */
    char buf[STD_BUF+1];        /* file read buffer */
    char rule_buf[STD_BUF+1];   /* content keyword buffer */
    int frazes_count;           /* frazes counter */


#ifdef DEBUG
    PatternMatchData *idx;
    DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, "Opening content_list file: %s\n", file););
#endif /* DEBUG */
    /* open the list file */
    thefp = fopen(file, "r");
    if (thefp == NULL)
    {
        ParseError("Unable to open list file: %s", file);
    }

    /* clear the line and rule buffers */
    bzero((char *) buf, STD_BUF);
    bzero((char *) rule_buf, STD_BUF);
    frazes_count = 0;

    /* loop thru each list_file line and content to the rule */
    while((fgets(buf, STD_BUF-2, thefp)) != NULL)
    {
        /* inc the line counter */
        list_file_line++;

        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, "Got line %d: %s", 
                list_file_line, buf););

        /* if it's not a comment or a <CR>, send it to the parser */
        if((buf[0] != '#') && (buf[0] != 0x0a) && (buf[0] != ';'))
        {
            DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, 
                    "Adding content keyword: %s", buf););

            frazes_count++;
            strip(buf);

            NewNode(otn, PLUGIN_PATTERN_MATCH_OR);

            /* check and add content keyword */
            ParsePattern(buf, otn, PLUGIN_PATTERN_MATCH_OR);

            DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                        "Content keyword %s\" added!\n", buf););
        }
    }
#ifdef DEBUG
    DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, "%d frazes read...\n", frazes_count););
    idx = (PatternMatchData *) otn->ds_list[PLUGIN_PATTERN_MATCH_OR];
    
    if(idx == NULL)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, "No patterns loaded\n"););
    }
    else
    {
        while(idx != NULL)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, "Pattern = %s\n", 
                    idx->pattern_buf););
            idx = idx->next;
        }
    }
#endif /* DEBUG */
    
    fclose(thefp);

    return;
}

int CheckORPatternMatch(Packet * p, OptTreeNode * otn_idx, OptFpList * fp_list)
{
    int found = 0;
    int dsize;
    char *dp;
    

    PatternMatchData *idx;

    DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, "CheckPatternORMatch: "););
    
    idx = otn_idx->ds_list[PLUGIN_PATTERN_MATCH_OR];

    while(idx != NULL)
    {

        if((p->packet_flags & PKT_ALT_DECODE) && (idx->rawbytes == 0))
        {
            dsize = DecodeBuffer.len;
            dp = (char *) DecodeBuffer.data; /* decode.c */
            DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, 
                                    "Using Alternative Decode buffer!\n"););
        }
        else
        {
            if(IsLimitedDetect(p))
                dsize = p->alt_dsize;
            else
                dsize = p->dsize;
            dp = (char *) p->data;
        }
        

        if(idx->offset > dsize)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, 
                        "Initial offset larger than payload!\n"););

            goto sizetoosmall;
        }
        else
        {
            DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, 
                        "testing pattern: %s\n", idx->pattern_buf););
            found = idx->search(dp, dsize, idx);

            if(!found)
            {
                DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, 
                            "Pattern Match failed!\n"););
            }
        }

        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, 
                    "Checking the results\n"););

        if(found)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, "Pattern Match "
                    "successful: %s!\n", idx->pattern_buf););

            return fp_list->next->OptTestFunc(p, otn_idx, fp_list->next);

        }
        else
        {
            DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, 
                        "Pattern match failed\n"););
        }

        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, 
                    "Stepping to next content keyword\n"););

    sizetoosmall:

        idx = idx->next;
    }

    DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, 
                "No more keywords, exiting... \n"););

    return 0;
}
#endif

#if 0
/* Not currently used */
static const char *format_uri_buffer_str(int uri_buffer, int search_buf, char *first_buf)
{
    if (uri_buffer & search_buf)
    {
        if (*first_buf == 1)
        {
            switch (search_buf)
            {
                case HTTP_SEARCH_URI:
                    return "http_uri";
                    break;
                case HTTP_SEARCH_CLIENT_BODY:
                    return "http_client_body";
                    break;
                case HTTP_SEARCH_HEADER:
                    return "http_header";
                    break;
                case HTTP_SEARCH_METHOD:
                    return "http_method";
                    break;
                case HTTP_SEARCH_COOKIE:
                    return "http_cookie";
                    break;
            }
            *first_buf = 0;
        }
        else
        {
            switch (search_buf)
            {
                case HTTP_SEARCH_URI:
                    return " | http_uri";
                    break;
                case HTTP_SEARCH_CLIENT_BODY:
                    return " | http_client_body";
                    break;
                case HTTP_SEARCH_HEADER:
                    return " | http_header";
                    break;
                case HTTP_SEARCH_METHOD:
                    return " | http_method";
                    break;
                case HTTP_SEARCH_COOKIE:
                    return " | http_cookie";
                    break;
            }
        }
    }
    return "";
}
#endif

