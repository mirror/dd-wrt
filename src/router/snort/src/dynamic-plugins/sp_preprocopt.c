/* $Id$ */
/*
 * sp_preprocopt.c
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
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2005-2013 Sourcefire, Inc.
 *
 * Author: Steven Sturges
 *
 * Purpose:
 *      Supports preprocessor defined rule options.
 *
 * Arguments:
 *      Required:
 *        None
 *      Optional:
 *        None
 *
 *   sample rules:
 *   alert tcp any any -> any any (msg: "DynamicRuleCheck"; );
 *
 * Effect:
 *
 *      Returns 1 if the option matches, 0 if it doesn't.
 *
 * Comments:
 *
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
#include "plugbase.h"
#include "rules.h"
#include "treenodes.h"

#include "snort_debug.h"
#include "util.h"

#include "sf_dynamic_engine.h"

#include "sfghash.h"
#include "sfhashfcn.h"

#include "snort.h"
#include "profiler.h"
#include "util.h"
#include "parser.h"
#include "detection_util.h"

#ifdef PERF_PROFILING
PreprocStats preprocRuleOptionPerfStats;
#endif

extern const uint8_t *doe_ptr;

SFGHASH * PreprocessorRuleOptionsNew(void)
{
    return sfghash_new(10, 0, 0, free);
}

void PreprocessorRuleOptionsFreeFunc(void *data)
{
    PreprocessorOptionInfo *opt_info = (PreprocessorOptionInfo *)data;

    if (opt_info == NULL)
        return;

    if (opt_info->optionCleanup != NULL)
        opt_info->optionCleanup(opt_info->data);

    free(opt_info);
}

void PreprocessorRuleOptionsFree(SFGHASH *preproc_rule_options)
{
    if (preproc_rule_options == NULL)
        return;

    sfghash_delete(preproc_rule_options);
}

int RegisterPreprocessorRuleOption(
        struct _SnortConfig *sc,
        char *optionName,
        PreprocOptionInit initFunc,
        PreprocOptionEval evalFunc,
        PreprocOptionCleanup cleanupFunc,
        PreprocOptionHash hashFunc,
        PreprocOptionKeyCompare keyCompareFunc,
        PreprocOptionOtnHandler otnHandler,
        PreprocOptionFastPatternFunc fpFunc
        )
{
    int ret;
    PreprocessorOptionInfo *optionInfo;

    if (sc == NULL)
    {
        FatalError("%s(%d) Snort conf for parsing is NULL.\n",
                   __FILE__, __LINE__);
    }

    if (sc->preproc_rule_options == NULL)
    {
        FatalError("Preprocessor Rule Option storage not initialized\n");
    }

    optionInfo = sfghash_find(sc->preproc_rule_options, optionName);
    if (optionInfo != NULL)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_STREAM, "Duplicate Preprocessor Rule Option '%s'\n", optionName););
        return 0;
    }

    optionInfo = (PreprocessorOptionInfo *)SnortAlloc(sizeof(PreprocessorOptionInfo));
    optionInfo->optionEval = evalFunc;
    optionInfo->optionInit = initFunc;
    optionInfo->optionCleanup = cleanupFunc;
    optionInfo->optionHash = hashFunc;
    optionInfo->optionKeyCompare = keyCompareFunc;
    optionInfo->optionFpFunc = fpFunc;
    optionInfo->otnHandler = otnHandler;

    ret = sfghash_add(sc->preproc_rule_options, optionName, optionInfo);
    if (ret != SFGHASH_OK)
    {
        FatalError("Failed to initialize Preprocessor Rule Option '%s'\n",
            optionName);
    }

    return 0;
}

int GetPreprocessorRuleOptionFuncs(
    SnortConfig *sc,
    char *optionName,
    PreprocOptionInit* initFunc,
    PreprocOptionEval* evalFunc,
    PreprocOptionOtnHandler* otnHandler,
    PreprocOptionFastPatternFunc* fpFunc,
    PreprocOptionCleanup* cleanupFunc
    )
{
    PreprocessorOptionInfo *optionInfo;

    if (sc == NULL)
    {
        FatalError("%s(%d) Snort conf for parsing is NULL.\n", __FILE__, __LINE__);
    }

    if (sc->preproc_rule_options == NULL)
    {
        FatalError("Preprocessor Rule Option storage not initialized\n");
    }

    optionInfo = sfghash_find(sc->preproc_rule_options, optionName);
    if (!optionInfo)
    {
        return 0;
    }

    *initFunc = (PreprocOptionInit)optionInfo->optionInit;
    *evalFunc = (PreprocOptionEval)optionInfo->optionEval;
    *fpFunc = (PreprocOptionFastPatternFunc)optionInfo->optionFpFunc;
    *otnHandler = (PreprocOptionOtnHandler)optionInfo->otnHandler;
    *cleanupFunc = (PreprocOptionCleanup)optionInfo->optionCleanup;

    return 1;
}

uint32_t PreprocessorRuleOptionHash(void *d)
{
    uint32_t a,b,c;
    PreprocessorOptionInfo *option_data = (PreprocessorOptionInfo *)d;

#if (defined(__ia64) || defined(__amd64) || defined(_LP64))
    {
        /* Cleanup warning because of cast from 64bit ptr to 32bit int
         * warning on 64bit OSs */
        uint64_t ptr; /* Addresses are 64bits */

        if (option_data->optionHash != NULL)
        {
            a = option_data->optionHash(option_data->data);
            b = 0;
        }
        else
        {
            ptr = (uint64_t)option_data->data;
            a = (ptr >> 32);
            b = (ptr & 0xFFFFFFFF);
        }

        ptr = (uint64_t)option_data->optionInit;
        c = (ptr >> 32);
        mix(a,b,c);

        a += (ptr & 0xFFFFFFFF); /* mix in the last half of optionInit */

        ptr = (uint64_t)option_data->optionEval;
        b += (ptr >> 32);
        c += (ptr & 0xFFFFFFFF);

        mix(a,b,c);
    }
#else
    if (option_data->optionHash != NULL)
        a = option_data->optionHash(option_data->data);
    else
        a = (uint32_t)option_data->data;

    b = (uint32_t)option_data->optionInit;
    c = (uint32_t)option_data->optionEval;
    mix(a,b,c);
#endif
    a += RULE_OPTION_TYPE_PREPROCESSOR;

    final(a,b,c);

    return c;
}

int PreprocessorRuleOptionCompare(void *l, void *r)
{
    PreprocessorOptionInfo *left = (PreprocessorOptionInfo *)l;
    PreprocessorOptionInfo *right = (PreprocessorOptionInfo *)r;

    if (!left || !right)
        return DETECTION_OPTION_NOT_EQUAL;

    if ((left->optionInit == right->optionInit) &&
        (left->optionEval == right->optionEval) &&
        (left->optionHash == right->optionHash) &&
        (left->optionKeyCompare == right->optionKeyCompare))
    {
        if (left->optionKeyCompare != NULL)
        {
            if (left->optionKeyCompare(left->data, right->data) == PREPROC_OPT_EQUAL)
                return DETECTION_OPTION_EQUAL;
        }
        else if (left->data == right->data)
        {
            return DETECTION_OPTION_EQUAL;
        }
    }

    return DETECTION_OPTION_NOT_EQUAL;
}

/* Callback function for dynamic preprocessor options */
int PreprocessorOptionFunc(void *option_data, Packet *p)
{
    PreprocessorOptionInfo *optionInfo = (PreprocessorOptionInfo *)option_data;
    const uint8_t *cursor = doe_ptr;
    int       rval;
    PROFILE_VARS;

    PREPROC_PROFILE_START(preprocRuleOptionPerfStats);

    //  Call eval function
    rval = optionInfo->optionEval(p, &cursor, optionInfo->data);

    if ( cursor )
        SetDoePtr(cursor, DOE_BUF_STD);

    //  return the value from the preprocessor function
    PREPROC_PROFILE_END(preprocRuleOptionPerfStats);
    return rval;
}

int GetPreprocFastPatterns(void *data, int proto, int direction, FPContentInfo **fp_contents)
{
    PreprocessorOptionInfo *info = (PreprocessorOptionInfo *)data;

    if ((data == NULL) || (fp_contents == NULL))
        return -1;

    if (info->optionFpFunc != NULL)
        return info->optionFpFunc(info->data, proto, direction, fp_contents);

    return -1;
}

int AddPreprocessorRuleOption(SnortConfig *sc, char *optionName, OptTreeNode *otn, void *data, PreprocOptionEval evalFunc)
{
    OptFpList *fpl;
    PreprocessorOptionInfo *optionInfo;
    PreprocessorOptionInfo *saveOptionInfo;
    void *option_dup;

    if (sc == NULL)
    {
        FatalError("%s(%d) Snort conf for parsing is NULL.\n", __FILE__, __LINE__);
    }

    optionInfo = sfghash_find(sc->preproc_rule_options, optionName);

    if (!optionInfo)
        return 0;

    saveOptionInfo = (PreprocessorOptionInfo *)SnortAlloc(sizeof(PreprocessorOptionInfo));

    memcpy(saveOptionInfo, optionInfo, sizeof(PreprocessorOptionInfo));

    saveOptionInfo->data = data;

    //  Add to option chain with generic callback
    fpl = AddOptFuncToList(PreprocessorOptionFunc, otn);

    /*
     * attach custom info to the context node so that we can call each instance
     * individually
     */
    fpl->context = (void *) saveOptionInfo;

    if (add_detection_option(sc, RULE_OPTION_TYPE_PREPROCESSOR,
                             (void *)saveOptionInfo, &option_dup) == DETECTION_OPTION_EQUAL)
    {
        PreprocessorRuleOptionsFreeFunc(saveOptionInfo);
        fpl->context = saveOptionInfo = option_dup;
    }
    fpl->type = RULE_OPTION_TYPE_PREPROCESSOR;

    return 1;
}

void PreprocessorRuleOptionOverrideFunc(SnortConfig *sc, char *keyword, char *option,
                                        char *args, OptTreeNode *otn, int protocol)
{
    PreprocessorOptionInfo *optionInfo;
    char *keyword_plus_option;
    void *opt_data;
    int name_len = strlen(keyword) + strlen(option) + 2;

    if (sc == NULL)
    {
        FatalError("%s(%d) Snort conf for parsing is NULL.\n", __FILE__, __LINE__);
    }

    keyword_plus_option = (char *)SnortAlloc(name_len);
    SnortSnprintf(keyword_plus_option, name_len, "%s %s", keyword, option);

    optionInfo = sfghash_find(sc->preproc_rule_options, keyword_plus_option);
    if (optionInfo == NULL)
    {
        free(keyword_plus_option);
        return;
    }

    optionInfo->optionInit(sc, keyword, args, &opt_data);
    AddPreprocessorRuleOption(sc, keyword_plus_option, otn, opt_data, optionInfo->optionEval);

    free(keyword_plus_option);
}

void RegisterPreprocessorRuleOptionOverride(
        struct _SnortConfig *sc,
        char *keyword,
        char *option,
        PreprocOptionInit initFunc,
        PreprocOptionEval evalFunc,
        PreprocOptionCleanup cleanupFunc,
        PreprocOptionHash hashFunc,
        PreprocOptionKeyCompare keyCompareFunc,
        PreprocOptionOtnHandler otnHandler,
        PreprocOptionFastPatternFunc fpFunc
        )
{
    int ret;
    char *keyword_plus_option;
    int name_len = strlen(keyword) + strlen(option) + 2;

    keyword_plus_option = (char *)SnortAlloc(name_len);
    SnortSnprintf(keyword_plus_option, name_len, "%s %s", keyword, option);

    ret = RegisterPreprocessorRuleOption(sc, keyword_plus_option, initFunc, evalFunc,
            cleanupFunc, hashFunc, keyCompareFunc, otnHandler, fpFunc);

    /* Hash table allocs and manages keys internally */
    free(keyword_plus_option);

    if (ret != 0)
        return;

    RegisterOverrideKeyword(keyword, option, &PreprocessorRuleOptionOverrideFunc);
}

void RegisterPreprocessorRuleOptionByteOrder(char *keyword, PreprocOptionByteOrderFunc boo_func)
{
    RegisterByteOrderKeyword(keyword, boo_func);
}
