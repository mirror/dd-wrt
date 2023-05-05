/* $Id$ */
/*
 * sp_dynamic.c
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
 *      Supports dynamically loaded detection plugin to check the packet.
 *
 *      does not update the doe_ptr
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
 *      Returns 1 if the dynamic detection plugin matches, 0 if it doesn't.
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
#include "rules.h"
#include "treenodes.h"
#include "decode.h"
#include "bitop_funcs.h"
#include "plugbase.h"
#include "parser.h"
#include "snort_debug.h"
#include "util.h"
#include "plugin_enum.h"
#include "sp_dynamic.h"
#include "sf_dynamic_engine.h"
#include "detection-plugins/sp_flowbits.h"
#include "detection-plugins/sp_asn1_detect.h"
#include "dynamic-plugins/sf_engine/sf_snort_plugin_api.h"
#include "sf_convert_dynamic.h"
#include "sfhashfcn.h"
#include "sp_preprocopt.h"
#include "sfutil/sf_base64decode.h"
#include "detection_util.h"
#include "stream_api.h"

#include "snort.h"
#include "profiler.h"
#include "reload.h"

#ifdef PERF_PROFILING
PreprocStats dynamicRuleEvalPerfStats;
extern PreprocStats ruleOTNEvalPerfStats;
#endif

extern SFGHASH *flowbits_hash;
extern SF_QUEUE *flowbits_bit_queue;
extern uint32_t flowbits_count;

void DynamicInit(struct _SnortConfig *, char *, OptTreeNode *, int);
void DynamicParse(char *, OptTreeNode *);
int DynamicCheck(void *option_data, Packet *p);

uint32_t DynamicRuleHash(void *d)
{
    uint32_t a,b,c;
    DynamicData *dynData = (DynamicData *)d;
#if (defined(__ia64) || defined(__amd64) || defined(_LP64))
    {
        /* Cleanup warning because of cast from 64bit ptr to 32bit int
         * warning on 64bit OSs */
        uint64_t ptr; /* Addresses are 64bits */
        ptr = (uint64_t)dynData->contextData;
        a = (ptr >> 32);
        b = (ptr & 0xFFFFFFFF);

        ptr = (uint64_t)dynData->checkFunction;
        c = (ptr >> 32);

        mix (a,b,c);

        a += (ptr & 0xFFFFFFFF);

        ptr = (uint64_t)dynData->hasOptionFunction;
        b += (ptr >> 32);
        c += (ptr & 0xFFFFFFFF);

        ptr = (uint64_t)dynData->getDynamicContents;
        a += (ptr >> 32);
        b += (ptr & 0xFFFFFFFF);
        c += dynData->contentFlags;

        mix (a,b,c);

        a += RULE_OPTION_TYPE_DYNAMIC;
    }
#else
    {
        a = (uint32_t)dynData->contextData;
        b = (uint32_t)dynData->checkFunction;
        c = (uint32_t)dynData->hasOptionFunction;
        mix(a,b,c);

        a += (uint32_t)dynData->getDynamicContents;
        b += dynData->contentFlags;
        c += RULE_OPTION_TYPE_DYNAMIC;
    }
#endif

    final(a,b,c);

    return c;
}

int DynamicRuleCompare(void *l, void *r)
{
    DynamicData *left = (DynamicData *)l;
    DynamicData *right = (DynamicData *)r;

    if (!left || !right)
        return DETECTION_OPTION_NOT_EQUAL;

    if ((left->contextData == right->contextData) &&
        (left->checkFunction == right->checkFunction) &&
        (left->hasOptionFunction == right->hasOptionFunction) &&
        (left->getDynamicContents == right->getDynamicContents) &&
        (left->contentFlags == right->contentFlags))
    {
        return DETECTION_OPTION_EQUAL;
    }

    return DETECTION_OPTION_NOT_EQUAL;
}

/****************************************************************************
 *
 * Function: SetupDynamic()
 *
 * Purpose: Load it up
 *
 * Arguments: None.
 *
 * Returns: void function
 *
 ****************************************************************************/
void SetupDynamic(void)
{
    /* map the keyword to an initialization/processing function */
    RegisterRuleOption("dynamic", DynamicInit, NULL, OPT_TYPE_DETECTION, NULL);

#ifdef PERF_PROFILING
    RegisterPreprocessorProfile("dynamic_rule", &dynamicRuleEvalPerfStats, 3, &ruleOTNEvalPerfStats, NULL);
#endif
    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,"Plugin: Dynamic Setup\n"););
}


/****************************************************************************
 *
 * Function: DynamicInit(struct _SnortConfig *, char *, OptTreeNode *)
 *
 * Purpose: Configuration function.  Handles parsing the rule
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
void DynamicInit(struct _SnortConfig *sc, char *data, OptTreeNode *otn, int protocol)
{
    OptFpList *fpl;
    DynamicData *dynData;
    void *option_dup;

    dynData = (DynamicData *)otn->ds_list[PLUGIN_DYNAMIC];

    fpl = AddOptFuncToList(DynamicCheck, otn);

    /* attach it to the context node so that we can call each instance
     * individually
     */
    fpl->context = (void *) dynData;

    if (add_detection_option(sc, RULE_OPTION_TYPE_DYNAMIC, (void *)dynData, &option_dup) == DETECTION_OPTION_EQUAL)
    {
        free(dynData);
        fpl->context = dynData = option_dup;
    }
    fpl->type = RULE_OPTION_TYPE_DYNAMIC;
}

/****************************************************************************
 *
 * Function: DynamicCheck(char *, OptTreeNode *, OptFpList *)
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
int DynamicCheck(void *option_data, Packet *p)
{
    DynamicData *dynData = (DynamicData *)option_data;
    int result = 0;
    PROFILE_VARS;

    PREPROC_PROFILE_START(dynamicRuleEvalPerfStats);

    if (!dynData)
    {
        LogMessage("Dynamic Rule with no context data available");
        PREPROC_PROFILE_END(dynamicRuleEvalPerfStats);
        return DETECTION_OPTION_NO_MATCH;
    }

    result = dynData->checkFunction((void *)p, dynData->contextData);
    if (result)
    {
        PREPROC_PROFILE_END(dynamicRuleEvalPerfStats);
        return result;
    }

    /* Detection failed */
    PREPROC_PROFILE_END(dynamicRuleEvalPerfStats);
    return DETECTION_OPTION_NO_MATCH;
}

void DynamicRuleListFree(DynamicRuleNode *head)
{
    while (head != NULL)
    {
        DynamicRuleNode *tmp = head->next;

        /* 
         * Clean up will be executed only when snort exits or
         * dynamic libs have changed
         */
        if (head->freeFunc)
        {
            head->freeFunc((void *)head->rule);
        }

        free(head);
        head = tmp;
    }
}

/****************************************************************************
 *
 * Function: RegisterDynamicRule(Snortconfig *, uint32_t, uint32_t, char *,
 *                               void *,
 *                               OTNCheckFunction, int, GetFPContentFunction)
 *
 * Purpose: A dynamically loaded detection engine library can use this
 *          function to register a dynamically loaded rule/preprocessor.  It
 *          provides a pointer to context specific data for the
 *          rule/preprocessor and a reference to the function used to
 *          check the rule.
 *
 * Arguments: sid => Signature ID
 *            gid => Generator ID
 *            info => context specific data
 *            chkFunc => Function to call to check if the rule matches
 *            has*Funcs => Functions used to categorize this rule
 *            contentFlags => Flags indicating which contents are available
 *            contentsFunc => Function to call to get list of rule contents
 *
 * Returns: 0 on success
 *
 ****************************************************************************/
int RegisterDynamicRule(
    SnortConfig *sc,
    uint32_t sid,
    uint32_t gid,
    void *info,
    OTNCheckFunction chkFunc,
    OTNHasFunction hasFunc,
    int contentFlags,
    GetDynamicContentsFunction contentsFunc,
    RuleFreeFunc freeFunc,
    GetDynamicPreprocOptFpContentsFunc preprocFpFunc
    )
{
    DynamicData *dynData;
    struct _OptTreeNode *otn = NULL;
    OptFpList *idx;     /* index pointer */
    OptFpList *fpl;
    char done_once = 0;
    void *option_dup;
    DynamicRuleNode *node = NULL;

    if (sc == NULL)
    {
        FatalError("%s(%d) Snort config is NULL.\n",
                   __FILE__, __LINE__);
    }

    if ( SnortDynamicLibsChanged() || SnortIsInitializing() )
    {
        node = (DynamicRuleNode *)SnortAlloc(sizeof(DynamicRuleNode));

        if (sc->dynamic_rules == NULL)
        {
            sc->dynamic_rules = node;
        }
        else
        {
            DynamicRuleNode *tmp = sc->dynamic_rules;

            while (tmp->next != NULL)
                tmp = tmp->next;

            tmp->next = node;
        }

        node->rule = (Rule *)info;
        node->chkFunc = chkFunc;
        node->hasFunc = hasFunc;
        node->contentFlags = contentFlags;
        node->contentsFunc = contentsFunc;
        node->freeFunc = freeFunc;
        node->preprocFpContentsFunc = preprocFpFunc;
    }

    /* Get OTN/RTN from SID */
    otn = SoRuleOtnLookup(sc->so_rule_otn_map, gid, sid);
    if (!otn)
    {
        if (ScConfErrorOut())
        {
            FatalError("DynamicPlugin: Rule [%u:%u] not enabled in configuration.\n", gid, sid);
        }
        else
        {
#ifndef SOURCEFIRE
            LogMessage("DynamicPlugin: Rule [%u:%u] not enabled in "
                       "configuration, rule will not be used.\n", gid, sid);
#endif
        }

        return -1;
    }

    /* If this dynamic rule can be expressed as a regular rule, break it down
     * and convert it to use the rule option tree. */
    if (ConvertDynamicRule(sc, (Rule *)info, otn) > 0)
    {
        if (node != NULL)
            node->converted = 1;

        return 0;
    }

    /* allocate the data structure and attach it to the
     * rule's data struct list */
    dynData = (DynamicData *)SnortAlloc(sizeof(DynamicData));
    dynData->contextData = info;
    dynData->checkFunction = chkFunc;
    dynData->hasOptionFunction = hasFunc;
    dynData->getDynamicContents = contentsFunc;
    dynData->contentFlags = contentFlags;
    dynData->getPreprocFpContents = preprocFpFunc;

    while (otn)
    {
        OptFpList *prev = NULL;

        otn->ds_list[PLUGIN_DYNAMIC] = (void *)dynData;

        /* And add this function into the tail of the list */
        fpl = AddOptFuncToList(DynamicCheck, otn);
        fpl->context = dynData;
        fpl->type = RULE_OPTION_TYPE_DYNAMIC;

        if (done_once == 0)
        {
            if (add_detection_option(sc, RULE_OPTION_TYPE_DYNAMIC,
                                     (void *)dynData, &option_dup) == DETECTION_OPTION_EQUAL)
            {
                free(dynData);
                fpl->context = dynData = option_dup;
            }

            done_once = 1;
        }

        /* Arrgh.  Because we read this rule in earlier, there is
         * already an OptListEnd node there.  Need to move this new
         * one to just before it.
         */
        DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"Adding new rule to list\n"););

        /* set the index pointer to the start of this OTN's function list */
        idx = otn->opt_func;

        /* if there are no nodes on the function list... */
        while(idx != NULL)
        {
            if (idx->next == fpl) /* The last one in the list before us */
            {
                if (prev)
                {
                    prev->next = fpl;
                    fpl->next = idx;
                    idx->next = NULL;
                }
                else /* idx is the head of the list */
                {
                    otn->opt_func = fpl;
                    fpl->next = idx;
                    idx->next = NULL;
                }
            }
            prev = idx;
            idx = idx->next;
        }

        otn = SoRuleOtnLookupNext(gid, sid);
    }

    return 0;
}

#ifdef SNORT_RELOAD
int ReloadDynamicRules(SnortConfig *sc)
{
    /*
     * We are registering the dynamic rules from old
     * snort config to new one. Hence this should be
     * snort_conf. This code will not be execute if
     * dynamic detection has changed.
     */
    DynamicRuleNode *node = snort_conf->dynamic_rules;
  
    sc->dynamic_rules = snort_conf->dynamic_rules;
    sc->loadedDetectionPlugins = snort_conf->loadedDetectionPlugins;
    snort_conf->loadedDetectionPlugins = NULL;
    snort_conf->dynamic_rules = NULL;

    for (; node != NULL; node = node->next)
    {
        int i;

        if (node->rule == NULL || (!node->rule->initialized) || node->rule->options == NULL)
            continue;

        for (i = 0; node->rule->options[i] != NULL; i++)
        {
            RuleOption *option = node->rule->options[i];

            switch (option->optionType)
            {
                case OPTION_TYPE_FLOWBIT:
                    {
                        FlowBitsInfo *flowbits = option->option_u.flowBit;
                        flowbits = DynamicFlowbitRegister(flowbits);
                    }

                    break;

                default:
                    break;
            }
        }

        if (RegisterDynamicRule(sc, node->rule->info.sigID, node->rule->info.genID,
                    (void *)node->rule, node->chkFunc, node->hasFunc,
                    node->contentFlags, node->contentsFunc,
                    node->freeFunc, node->preprocFpContentsFunc) == -1)
        {
            for (i = 0; node->rule->options[i] != NULL; i++)
            {
                RuleOption *option = node->rule->options[i];
                switch (option->optionType)
                {
                    case OPTION_TYPE_FLOWBIT:
                        {
                            FlowBitsInfo *flowbits = option->option_u.flowBit;
                            DynamicFlowbitUnregister(flowbits);
                        }
                        break;

                    default:
                        break;
                }
            }

        }
    }

    return 0;
}
#endif

int DynamicPreprocRuleOptInit(struct _SnortConfig *sc, void *opt)
{
    PreprocessorOption *preprocOpt = (PreprocessorOption *)opt;
    PreprocOptionInit optionInit;
    PreprocOptionOtnHandler otnHandler;
    char *option_name = NULL;
    char *option_params = NULL;
    char *tmp;
    int result;

    if (preprocOpt == NULL)
        return -1;

    if (preprocOpt->optionName == NULL)
        return -1;

    result = GetPreprocessorRuleOptionFuncs(sc, (char *)preprocOpt->optionName,
                                     &preprocOpt->optionInit,
                                     &preprocOpt->optionEval,
                                     &otnHandler,
                                     &preprocOpt->optionFpFunc,
                                     &preprocOpt->optionCleanup);
    if (!result)
        return -1;

    optionInit = (PreprocOptionInit)preprocOpt->optionInit;

    option_name = SnortStrdup(preprocOpt->optionName);

    /* XXX Hack right now for override options where the rule
     * option is stored as <option> <override>, e.g.
     * "byte_test dce"
     * Since name is passed in to init function, the function
     * is expecting the first word in the option name and not
     * the whole string */
    tmp = option_name;
    while ((*tmp != '\0') && !isspace((int)*tmp)) tmp++;
    *tmp = '\0';

    if (preprocOpt->optionParameters != NULL)
        option_params = SnortStrdup(preprocOpt->optionParameters);

    result = optionInit(sc, option_name, option_params, &preprocOpt->dataPtr);

    free(option_name);
    if (option_params != NULL) free(option_params);

    if (!result)
        return -1;

    return 0;
}

void *DynamicFlowbitRegister(void *info)
{
    FlowBitsInfo *flowbitsInfo = (FlowBitsInfo *)info;
    FLOWBITS_OP *flowbits;

    if (!info)
        return NULL;

    flowbits = (FLOWBITS_OP *) SnortAlloc(sizeof(FLOWBITS_OP));
    flowbits->type = flowbitsInfo->operation;
    processFlowBitsWithGroup(flowbitsInfo->flowBitsName, flowbitsInfo->groupName, flowbits);

    // SO rules sometimes reuse the same option structure for multiple rule
    // definitions.  Also on snort reload we can't muck with the structure
    // since it'll possibly be in use.  So if flowbitsInfo->ids is already set,
    // the flowbits structure has already been parsed and flowbits->ids will
    // remain unchanged, so just return.
    // processFlowBitsWithGroup() is called again, mainly for reload in
    // case flowbits have been removed or added and/or the actual id values
    // have changed.
    if (flowbitsInfo->ids != NULL)
    {
        if (flowbits->ids != NULL)
            free(flowbits->ids);
        free(flowbits);
        return flowbitsInfo;
    }

    flowbitsInfo->eval = flowbits->eval;
    flowbitsInfo->ids = flowbits->ids;
    flowbitsInfo->num_ids = flowbits->num_ids;
    free(flowbits);
    return flowbitsInfo;
}

static void unregisterFlowbit(char *name, int op)
{
    FLOWBITS_OBJECT *flowbits_item;

    if (flowbits_hash == NULL)
        return;

    flowbits_item = sfghash_find(flowbits_hash, name);
    if (flowbits_item == NULL)
        return;

    switch (op)
    {
        case FLOWBITS_SET:
        case FLOWBITS_SETX:
        case FLOWBITS_UNSET:
        case FLOWBITS_TOGGLE:
        case FLOWBITS_RESET:
            if (flowbits_item->set == 0)
                return;
            flowbits_item->set--;
            break;

        case FLOWBITS_ISSET:
        case FLOWBITS_ISNOTSET:
            if (flowbits_item->isset == 0)
                return;
            flowbits_item->isset--;
            break;

        default:
            break;
    }
}

void DynamicFlowbitUnregister(void *info)
{
    FlowBitsInfo *flowbitsInfo = (FlowBitsInfo *)info;
    char *names;
    char *flowbitName;
    char *nextName;
    int op;

    if ((!flowbitsInfo)||(!flowbitsInfo->flowBitsName))
        return;

    op = flowbitsInfo->operation;
    names = SnortStrdup(flowbitsInfo->flowBitsName);
    flowbitName = strtok_r(names, "|&", &nextName);
    while ( flowbitName )
    {
        unregisterFlowbit(flowbitName, op);
        flowbitName = strtok_r(nextName, "|&", &nextName);
    }

    // Don't free flowbits->ids here for SO rules as it may cause a segfault
    // with rules that share the same flowbits structure when not all of the
    // stub rules are enabled.  The ids array will be free'd at shutdown in
    // FreeOneRule().

    free(names);
}

int DynamicFlowbitCheck(void *pkt, void *info)
{
    Packet *p = (Packet *)pkt;
    FlowBitsInfo *flowbitsInfo = (FlowBitsInfo *)info;
    int result = 0;
    result = checkFlowBits(flowbitsInfo->operation, flowbitsInfo->eval, flowbitsInfo->ids,
            flowbitsInfo->num_ids,flowbitsInfo->groupName, p);
    return result;
}


int DynamicAsn1Detect(void *pkt, void *ctxt, const uint8_t *cursor)
{
    Packet *p    = (Packet *) pkt;
    ASN1_CTXT *c = (ASN1_CTXT *) ctxt;

    /* Call same detection function that snort calls */
    return Asn1DoDetect(p->data, p->dsize, c, cursor);
}

int DynamicsfUnfold(const uint8_t *inbuf, uint32_t insize, uint8_t *outbuf, uint32_t outsize, uint32_t *read)
{
    return sf_unfold_header(inbuf, insize, outbuf, outsize, read, 0, 0);
}

int Dynamicsfbase64decode(uint8_t *inbuf, uint32_t insize, uint8_t *outbuf, uint32_t outsize, uint32_t *read)
{
    return sf_base64decode(inbuf, insize, outbuf, outsize, read);
}

int DynamicGetAltDetect(uint8_t **bufPtr, uint16_t *altLenPtr)
{
    return GetAltDetect(bufPtr, altLenPtr);
}

void DynamicSetAltDetect(uint8_t *buf, uint16_t altLen)
{
    SetAltDetect(buf, altLen);
}

int DynamicIsDetectFlag(SFDetectFlagType df)
{
    return Is_DetectFlag((DetectFlagType)df);
}

void DynamicDetectFlagDisable(SFDetectFlagType df)
{
        DetectFlag_Disable((DetectFlagType)df);
}


static inline int DynamicHasOption(
    OptTreeNode *otn, DynamicOptionType optionType, int flowFlag
) {
    DynamicData *dynData;

    dynData = (DynamicData *)otn->ds_list[PLUGIN_DYNAMIC];
    if (!dynData)
    {
        return 0;
    }

    return dynData->hasOptionFunction(dynData->contextData, optionType, flowFlag);
}

int DynamicHasFlow(OptTreeNode *otn)
{
    return DynamicHasOption(otn, OPTION_TYPE_FLOWFLAGS, 0);
}

int DynamicHasFlowbit(OptTreeNode *otn)
{
    return DynamicHasOption(otn, OPTION_TYPE_FLOWBIT, 0);
}

int DynamicHasContent(OptTreeNode *otn)
{
    return DynamicHasOption(otn, OPTION_TYPE_CONTENT, 0);
}

int DynamicHasByteTest(OptTreeNode *otn)
{
    return DynamicHasOption(otn, OPTION_TYPE_BYTE_TEST, 0);
}

int DynamicHasByteMath(OptTreeNode *otn)
{
    return DynamicHasOption(otn, OPTION_TYPE_BYTE_MATH, 0);
}

int DynamicHasPCRE(OptTreeNode *otn)
{
    return DynamicHasOption(otn, OPTION_TYPE_PCRE, 0);
}
