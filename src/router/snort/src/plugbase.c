/* $Id$ */
/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2002-2013 Sourcefire, Inc.
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
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#ifndef WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif /* !WIN32 */
#include <time.h>
#include <errno.h>

#include "sf_types.h"
#include "plugbase.h"
#include "spo_plugbase.h"
#include "snort.h"
#include "snort_debug.h"
#include "util.h"
#include "log.h"
#include "detect.h"

/* built-in preprocessors */
#include "preprocessors/spp_rpc_decode.h"
#include "preprocessors/spp_bo.h"
#include "preprocessors/spp_session.h"
#include "preprocessors/spp_stream6.h"
#include "preprocessors/spp_arpspoof.h"
#include "preprocessors/spp_perfmonitor.h"
#include "preprocessors/spp_httpinspect.h"
#include "preprocessors/spp_sfportscan.h"
#include "preprocessors/spp_frag3.h"
#include "preprocessors/spp_normalize.h"

/* built-in detection plugins */
#include "detection-plugins/sp_pattern_match.h"
#include "detection-plugins/sp_tcp_flag_check.h"
#include "detection-plugins/sp_icmp_type_check.h"
#include "detection-plugins/sp_icmp_code_check.h"
#include "detection-plugins/sp_ttl_check.h"
#include "detection-plugins/sp_ip_id_check.h"
#include "detection-plugins/sp_tcp_ack_check.h"
#include "detection-plugins/sp_tcp_seq_check.h"
#include "detection-plugins/sp_dsize_check.h"
#include "detection-plugins/sp_ipoption_check.h"
#include "detection-plugins/sp_rpc_check.h"
#include "detection-plugins/sp_icmp_id_check.h"
#include "detection-plugins/sp_icmp_seq_check.h"
#include "detection-plugins/sp_session.h"
#include "detection-plugins/sp_ip_tos_check.h"
#include "detection-plugins/sp_ip_fragbits.h"
#include "detection-plugins/sp_tcp_win_check.h"
#include "detection-plugins/sp_ip_same_check.h"
#include "detection-plugins/sp_ip_proto.h"
#include "detection-plugins/sp_ip_same_check.h"
#include "detection-plugins/sp_clientserver.h"
#include "detection-plugins/sp_byte_check.h"
#include "detection-plugins/sp_byte_jump.h"
#include "detection-plugins/sp_byte_extract.h"
#include "detection-plugins/sp_byte_math.h"
#include "detection-plugins/sp_isdataat.h"
#include "detection-plugins/sp_pcre.h"
#include "detection-plugins/sp_flowbits.h"
#include "detection-plugins/sp_file_data.h"
#include "detection-plugins/sp_base64_decode.h"
#include "detection-plugins/sp_base64_data.h"
#include "detection-plugins/sp_pkt_data.h"
#include "detection-plugins/sp_asn1.h"

#ifdef ENABLE_REACT
#include "detection-plugins/sp_react.h"
#endif

#ifdef ENABLE_RESPOND
#include "detection-plugins/sp_respond.h"
#endif

#include "detection-plugins/sp_ftpbounce.h"
#include "detection-plugins/sp_urilen_check.h"
#include "detection-plugins/sp_cvs.h"
#include "detection-plugins/sp_file_type.h"

#if defined(FEAT_OPEN_APPID)
#include "detection-plugins/sp_appid.h"
#endif /* defined(FEAT_OPEN_APPID) */

/* built-in output plugins */
#include "output-plugins/spo_alert_syslog.h"
#include "output-plugins/spo_log_tcpdump.h"
#include "output-plugins/spo_alert_fast.h"
#include "output-plugins/spo_alert_full.h"
#include "output-plugins/spo_alert_unixsock.h"
#include "output-plugins/spo_csv.h"
#include "output-plugins/spo_log_null.h"
#include "output-plugins/spo_log_ascii.h"
#include "output-plugins/spo_unified2.h"

#ifdef DUMP_BUFFER
#include "output-plugins/spo_log_buffer_dump.h"
#endif

#ifdef LINUX
#include "output-plugins/spo_alert_sf_socket.h"
#endif

#include "output-plugins/spo_alert_test.h"

extern ListHead *head_tmp;
extern PreprocConfigFuncNode *preproc_config_funcs;
extern OutputConfigFuncNode *output_config_funcs;
extern RuleOptConfigFuncNode *rule_opt_config_funcs;
extern RuleOptOverrideInitFuncNode *rule_opt_override_init_funcs;
extern RuleOptParseCleanupNode *rule_opt_parse_cleanup_list;
extern RuleOptByteOrderFuncNode *rule_opt_byte_order_funcs;
extern PreprocSignalFuncNode *preproc_clean_exit_funcs;
extern PreprocSignalFuncNode *preproc_shutdown_funcs;
extern PreprocSignalFuncNode *preproc_reset_funcs;
extern PreprocSignalFuncNode *preproc_reset_stats_funcs;
extern PreprocStatsFuncNode *preproc_stats_funcs;
extern PluginSignalFuncNode *plugin_shutdown_funcs;
extern PluginSignalFuncNode *plugin_clean_exit_funcs;
extern OutputFuncNode *AlertList;
extern OutputFuncNode *LogList;

/**************************** Detection Plugin API ****************************/
/* For translation from enum to char* */
#ifdef DEBUG_MSGS
static const char *optTypeMap[OPT_TYPE_MAX] =
{
    "action",
    "logging",
    "detection"
};

#define ENUM2STR(num, map) \
    ((num < sizeof(map)/sizeof(map[0])) ? map[num] : "undefined")
#endif

static GetHttpXffFieldsFunc getHttpXffFieldsFunc = NULL;

void RegisterRuleOptions(void)
{
    LogMessage("Initializing Plug-ins!\n");

    SetupPatternMatch();
    SetupTCPFlagCheck();
    SetupIcmpTypeCheck();
    SetupIcmpCodeCheck();
    SetupTtlCheck();
    SetupIpIdCheck();
    SetupTcpAckCheck();
    SetupTcpSeqCheck();
    SetupDsizeCheck();
    SetupIpOptionCheck();
    SetupRpcCheck();
    SetupIcmpIdCheck();
    SetupIcmpSeqCheck();
    SetupSession();
    SetupIpTosCheck();
    SetupFragBits();
    SetupFragOffset();
    SetupTcpWinCheck();
    SetupIpProto();
    SetupIpSameCheck();
    SetupClientServer();
    SetupPktData();
    SetupByteTest();
    SetupByteJump();
    SetupByteExtract();
    SetupByteMath();
    SetupIsDataAt();
    SetupFileData();
    SetupBase64Decode();
    SetupBase64Data();
    SetupPcre();
    SetupFlowBits();
    SetupAsn1();
#ifdef ENABLE_REACT
    SetupReact();
#endif
#ifdef ENABLE_RESPOND
    SetupRespond();
#endif
    SetupFTPBounce();
    SetupUriLenCheck();
    SetupCvs();
    SetupFileType();
#if defined(FEAT_OPEN_APPID)
    SetupAppId();
#endif /* defined(FEAT_OPEN_APPID) */
}

/****************************************************************************
 *
 * Function: RegisterRuleOption(char *, void (*func)(), enum OptionType)
 *
 * Purpose:  Associates a rule option keyword with an option setup/linking
 *           function.
 *
 * Arguments: keyword => The option keyword to associate with the option
 *                       handler
 *            *func => function pointer to the handler
 *            type => used to determine where keyword is allowed
 *
 * Returns: void function
 *
 ***************************************************************************/
void RegisterRuleOption(char *opt_name, RuleOptConfigFunc ro_config_func,
                        RuleOptOverrideInitFunc override_init_func,
                        RuleOptType opt_type,
                        RuleOptOtnHandler otn_handler)
{
    RuleOptConfigFuncNode *node;

    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "Registering keyword:func => %s/%s:%p\n",
                            ENUM2STR(opt_type, optTypeMap), opt_name, ro_config_func););

    node = (RuleOptConfigFuncNode *)SnortAlloc(sizeof(RuleOptConfigFuncNode));

    if (rule_opt_config_funcs == NULL)
    {
        rule_opt_config_funcs = node;
    }
    else
    {
        RuleOptConfigFuncNode *tmp = rule_opt_config_funcs;
        RuleOptConfigFuncNode *last;

        do
        {
            if (strcasecmp(tmp->keyword, opt_name) == 0)
            {
                free(node);
                FatalError("%s(%d) Duplicate detection plugin keyword: %s.\n",
                           file_name, file_line, opt_name);
            }

            last = tmp;
            tmp = tmp->next;

        } while (tmp != NULL);

        last->next = node;
    }

    node->keyword = SnortStrdup(opt_name);
    node->type = opt_type;
    node->func = ro_config_func;
    node->otn_handler = otn_handler;

    if (override_init_func != NULL)
    {
        RuleOptOverrideInitFuncNode *node_override =
            (RuleOptOverrideInitFuncNode *)SnortAlloc(sizeof(RuleOptOverrideInitFuncNode));

        if (rule_opt_override_init_funcs == NULL)
        {
            rule_opt_override_init_funcs = node_override;
        }
        else
        {
            RuleOptOverrideInitFuncNode *tmp = rule_opt_override_init_funcs;
            RuleOptOverrideInitFuncNode *last;

            do
            {
                if (strcasecmp(tmp->keyword, opt_name) == 0)
                {
                    free(node_override);
                    FatalError("RegisterRuleOption: Duplicate detection plugin keyword:"
                            " (%s) (%s)!\n", tmp->keyword, opt_name);
                }

                last = tmp;
                tmp = tmp->next;

            } while (tmp != NULL);

            last->next = node_override;
        }

        node_override->keyword = SnortStrdup(opt_name);
        node_override->type = opt_type;
        node_override->func = override_init_func;
        node_override->otn_handler = otn_handler;
    }
}

void RegisterOverrideKeyword(char *keyword, char *option, RuleOptOverrideFunc roo_func)
{
    RuleOptOverrideInitFuncNode *node = rule_opt_override_init_funcs;

    while (node != NULL)
    {
        if (strcasecmp(node->keyword, keyword) == 0)
        {
            node->func(keyword, option, roo_func);
            break;
        }

        node = node->next;
    }
}

void RegisterByteOrderKeyword(char *keyword, RuleOptByteOrderFunc roo_func)
{
    RuleOptByteOrderFuncNode *node = (RuleOptByteOrderFuncNode *)SnortAlloc(sizeof(RuleOptByteOrderFuncNode));
    RuleOptByteOrderFuncNode *list = rule_opt_byte_order_funcs;
    RuleOptByteOrderFuncNode *last;

    node->keyword = SnortStrdup(keyword);
    node->func = roo_func;
    node->next = NULL;

    if (list == NULL)
        rule_opt_byte_order_funcs = node;
    else
    {
        while (list != NULL)
        {
            if (strcasecmp(node->keyword, list->keyword) == 0)
            {
                free(node->keyword);
                free(node);
                return;
            }

            last = list;
            list = list->next;
        }

        last->next = node;
    }
}

RuleOptByteOrderFunc GetByteOrderFunc(char *keyword)
{
    RuleOptByteOrderFuncNode *node = rule_opt_byte_order_funcs;

    while (node != NULL)
    {
        if (strcasecmp(keyword, node->keyword) == 0)
            return node->func;

        node = node->next;
    }

    return NULL;
}

/****************************************************************************
 *
 * Function: DumpPlugIns()
 *
 * Purpose:  Prints the keyword->function list
 *
 * Arguments: None.
 *
 * Returns: void function
 *
 ***************************************************************************/
void DumpRuleOptions(void)
{
    RuleOptConfigFuncNode *node;

    node = rule_opt_config_funcs;

    LogMessage("-------------------------------------------------\n");
    LogMessage(" Keyword     |      Plugin Registered @\n");
    LogMessage("-------------------------------------------------\n");

    while (node != NULL)
    {
        LogMessage("%-13s:      %p\n", node->keyword, (void *)node->vfunc);
        node = node->next;
    }

    LogMessage("-------------------------------------------------\n");
    LogMessage("\n");
}


/****************************************************************************
 *
 * Function: AddOptFuncToList(int (*func)(), OptTreeNode *)
 *
 * Purpose: Links the option detection module to the OTN
 *
 * Arguments: (*func)() => function pointer to the detection module
 *            otn =>  pointer to the current OptTreeNode
 *
 * Returns: void function
 *
 ***************************************************************************/
OptFpList * AddOptFuncToList(RuleOptEvalFunc ro_eval_func, OptTreeNode *otn)
{
    OptFpList *ofp = (OptFpList *)SnortAlloc(sizeof(OptFpList));

    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"Adding new rule to list\n"););

    /* if there are no nodes on the function list... */
    if (otn->opt_func == NULL)
    {
        otn->opt_func = ofp;
    }
    else
    {
        OptFpList *tmp = otn->opt_func;

        /* walk to the end of the list */
        while (tmp->next != NULL)
            tmp = tmp->next;

        tmp->next = ofp;
    }

    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"Set OptTestFunc to %p\n", ro_eval_func););

    ofp->OptTestFunc = ro_eval_func;

    return ofp;
}

/****************************************************************************
 *
 * Function: AddRspFuncToList(int (*func)(), OptTreeNode *)
 *
 * Purpose: Adds Response function to OTN
 *
 * Arguments: (*func)() => function pointer to the response module
 *            otn =>  pointer to the current OptTreeNode
 *
 * Returns: void function
 *
 ***************************************************************************/
// TBD this can prolly be replaced with a single item
// because we allow at most one response per packet
void AddRspFuncToList(ResponseFunc resp_func, OptTreeNode *otn, void *params)
{
    RspFpList *rsp = (RspFpList *)SnortAlloc(sizeof(RspFpList));

    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"Adding response to list\n"););

    /* if there are no nodes on the function list... */
    if (otn->rsp_func == NULL)
    {
        otn->rsp_func = rsp;
    }
    else
    {
        RspFpList *tmp = otn->rsp_func;

        /* walk to the end of the list */
        while (tmp->next != NULL)
            tmp = tmp->next;

        tmp->next = rsp;
    }

    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"Set ResponseFunc to %p\n", resp_func););

    rsp->func = resp_func;
    rsp->params = params;
}

void PostConfigInitPlugins(struct _SnortConfig *sc, PostConfigFuncNode *post_config_funcs)
{
    while (post_config_funcs != NULL)
    {
        post_config_funcs->func(sc, 0, post_config_funcs->arg);
        post_config_funcs = post_config_funcs->next;
    }
}

void FreeRuleOptConfigFuncs(RuleOptConfigFuncNode *head)
{

    while (head != NULL)
    {
        RuleOptConfigFuncNode *tmp = head;

        head = head->next;

        if (tmp->keyword != NULL)
            free(tmp->keyword);

        free(tmp);
    }
}

void FreeRuleOptOverrideInitFuncs(RuleOptOverrideInitFuncNode *head)
{

    while (head != NULL)
    {
        RuleOptOverrideInitFuncNode *tmp = head;

        head = head->next;

        if (tmp->keyword != NULL)
            free(tmp->keyword);

        free(tmp);
    }
}

void FreeRuleOptByteOrderFuncs(RuleOptByteOrderFuncNode *head)
{
    while (head != NULL)
    {
        RuleOptByteOrderFuncNode *tmp = head;

        head = head->next;

        if (tmp->keyword != NULL)
            free(tmp->keyword);

        free(tmp);
    }
}

void FreePluginSigFuncs(PluginSignalFuncNode *head)
{
    while (head != NULL)
    {
        PluginSignalFuncNode *tmp = head;

        head = head->next;

        /* don't free sig->arg, that's free'd by the CleanExit func */
        free(tmp);
    }
}

void FreePluginPostConfigFuncs(PostConfigFuncNode *head)
{
    while (head != NULL)
    {
        PostConfigFuncNode *tmp = head;

        head = head->next;

        /* don't free sig->arg, that's free'd by the CleanExit func */
        free(tmp);
    }
}

/************************** Non Rule Detection Plugin API *********************/
DetectionEvalFuncNode * AddFuncToDetectionList(SnortConfig *sc,
                                            DetectionEvalFunc detect_eval_func,
                                            uint16_t priority, uint32_t detect_id,
                                            uint32_t proto_mask)
{
    DetectionEvalFuncNode *node;
    tSfPolicyId policy_id = getParserPolicy(sc);
    SnortPolicy *p;

    if (sc == NULL)
    {
        FatalError("%s(%d) Snort config for parsing is NULL.\n",
                   __FILE__, __LINE__);
    }

    p = sc->targeted_policies[policy_id];
    if (p == NULL)
        return NULL;

    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,
                            "Adding detection function ID %d/bit %d/pri %d to list\n",
                            detect_id, p->num_detects, priority););

    node = (DetectionEvalFuncNode *)SnortAlloc(sizeof(DetectionEvalFuncNode));

    if (p->detect_eval_funcs == NULL)
    {
        p->detect_eval_funcs = node;
    }
    else
    {
        DetectionEvalFuncNode *tmp = p->detect_eval_funcs;
        DetectionEvalFuncNode *last = NULL;

        do
        {
            if (tmp->detect_id == detect_id)
            {
                free(node);
                FatalError("Detection function already registered with ID %d\n",
                           detect_id);
            }

            /* Insert higher priority preprocessors first.  Lower priority
             * number means higher priority */
            if (priority < tmp->priority)
                break;

            last = tmp;
            tmp = tmp->next;

        } while (tmp != NULL);

        /* Priority higher than first item in list */
        if (last == NULL)
        {
            node->next = tmp;
            p->detect_eval_funcs = node;
        }
        else
        {
            node->next = tmp;
            last->next = node;
        }
    }

    node->func = detect_eval_func;
    node->priority = priority;
    node->detect_id = detect_id;
    //node->detect_bit = (1 << detect_id);
    node->proto_mask = proto_mask;

    p->num_detects++;
    p->detect_proto_mask |= proto_mask;
    //p->detect_bit_mask |= node->detect_bit;

    return node;
}

void FreeDetectionEvalFuncs(DetectionEvalFuncNode *head)
{
    DetectionEvalFuncNode *tmp;

    while (head != NULL)
    {
        tmp = head->next;
        //if (head->context)
        //    free(head->context);
        free(head);
        head = tmp;
    }
}

/************************** Buffer Dump Plugin API ***************************/
#ifdef DUMP_BUFFER
void RegisterBufferTracer(TraceBuffer *(*bdfunc)(), BUFFER_DUMP_FUNC type)
{
    getBuffers[type] = bdfunc;
    bdmask |= (UINT64_C(1) << type);
}
#endif

/************************** Preprocessor Plugin API ***************************/
static void AddFuncToPreprocSignalList(PreprocSignalFunc, void *,
                                       PreprocSignalFuncNode **, uint16_t, uint32_t);


void RegisterPreprocessors(void)
{
    LogMessage("Initializing Preprocessors!\n");

    SetupARPspoof();
#ifdef NORMALIZER
    SetupNormalizer();
#endif
    SetupFrag3();
    SetupSessionManager();
    SetupStream6();
    SetupRpcDecode();
    SetupBo();
    SetupHttpInspect();
    SetupPerfMonitor();
    SetupSfPortscan();
}

/****************************************************************************
 *
 * Function: RegisterPreprocessor(char *, void (*)(char *))
 *
 * Purpose:  Associates a preprocessor statement with its function.
 *
 * Arguments: keyword => The option keyword to associate with the
 *                       preprocessor
 *            *func => function pointer to the handler
 *
 * Returns: void function
 *
 ***************************************************************************/
#ifndef SNORT_RELOAD
void RegisterPreprocessor(const char *keyword, PreprocConfigFunc pp_config_func)
#else
void RegisterPreprocessor(const char *keyword, PreprocConfigFunc pp_config_func,
                          PreprocReloadFunc rfunc, PreprocReloadVerifyFunc rvfunc,
                          PreprocReloadSwapFunc sfunc, PreprocReloadSwapFreeFunc ffunc)
#endif
{
    PreprocConfigFuncNode *node =
        (PreprocConfigFuncNode *)SnortAlloc(sizeof(PreprocConfigFuncNode));

    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,"Registering keyword:preproc => %s:%p\n", keyword, pp_config_func););

    if (preproc_config_funcs == NULL)
    {
        preproc_config_funcs = node;
    }
    else
    {
        PreprocConfigFuncNode *tmp = preproc_config_funcs;
        PreprocConfigFuncNode *last;

        do
        {
            if (strcasecmp(tmp->keyword, keyword) == 0)
            {
                free(node);
                FatalError("Duplicate preprocessor keyword: %s.\n", keyword);
            }

            last = tmp;
            tmp = tmp->next;

        } while (tmp != NULL);

        last->next = node;
    }

    node->keyword = SnortStrdup(keyword);
    node->config_func = pp_config_func;

#ifdef SNORT_RELOAD
    node->reload_func = rfunc;
    node->reload_verify_func = rvfunc;
    node->reload_swap_func = sfunc;
    node->reload_swap_free_func = ffunc;
#endif
}

#ifdef SNORT_RELOAD
void *GetRelatedReloadData(SnortConfig *sc, const char *keyword)
{
    PreprocessorSwapData *swapData;
    for (swapData = sc->preprocSwapData; swapData; swapData = swapData->next)
    {
        if (swapData->preprocNode && swapData->preprocNode->keyword &&
            strcasecmp(swapData->preprocNode->keyword, keyword) == 0)
        {
            return swapData->data;
        }
    }
    return NULL;
}

void *GetReloadStreamConfig(SnortConfig *sc)
{
    return sc->streamReloadConfig;
}
#endif

PreprocConfigFuncNode * GetPreprocConfig(char *keyword)
{
    PreprocConfigFuncNode *head = preproc_config_funcs;

    if (keyword == NULL)
        return NULL;

    while (head != NULL)
    {
        if (strcasecmp(head->keyword, keyword) == 0)
           return head;

        head = head->next;
    }

    return NULL;
}

PreprocConfigFunc GetPreprocConfigFunc(char *keyword)
{
    PreprocConfigFuncNode *head = preproc_config_funcs;

    if (keyword == NULL)
        return NULL;

    while (head != NULL)
    {
        if (strcasecmp(head->keyword, keyword) == 0)
           return head->config_func;

        head = head->next;
    }

    return NULL;
}

/****************************************************************************
 *
 * Function: RegisterPreprocStats(char *keyword, void (*func)(int))
 *
 * Purpose: Registers a function for printing preprocessor final stats
 *          (or other if it has a use for printing final stats)
 *
 * Arguments: keyword => keyword (preprocessor) whose stats will print
 *            func => function pointer to the handler
 *
 * Returns: void function
 *
 ***************************************************************************/
void RegisterPreprocStats(const char *keyword, PreprocStatsFunc pp_stats_func)
{
    PreprocStatsFuncNode *node;

    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,"Registering final stats function: "
                            "preproc => %s:%p\n", keyword, pp_stats_func););

    node = (PreprocStatsFuncNode *)SnortAlloc(sizeof(PreprocStatsFuncNode));

    if (preproc_stats_funcs == NULL)
    {
        preproc_stats_funcs = node;
    }
    else
    {
        PreprocStatsFuncNode *tmp = preproc_stats_funcs;
        PreprocStatsFuncNode *last;

        do
        {
            if (strcasecmp(tmp->keyword, keyword) == 0)
            {
                free(node);
                FatalError("Duplicate preprocessor keyword: %s.\n", keyword);
            }

            last = tmp;
            tmp = tmp->next;

        } while (tmp != NULL);

        last->next = node;
    }

    node->keyword = SnortStrdup(keyword);
    node->func = pp_stats_func;
}

/****************************************************************************
 *
 * Function: DumpPreprocessors()
 *
 * Purpose:  Prints the keyword->preprocess list
 *
 * Arguments: None.
 *
 * Returns: void function
 *
 ***************************************************************************/
void DumpPreprocessors(void)
{
    PreprocConfigFuncNode *node = preproc_config_funcs;

    LogMessage("-------------------------------------------------\n");
    LogMessage(" Keyword     |       Preprocessor @ \n");
    LogMessage("-------------------------------------------------\n");

    while (node != NULL)
    {
        LogMessage("%-13s:       %p\n", node->keyword, node->config_vfunc);
        node = node->next;
    }

    LogMessage("-------------------------------------------------\n\n");
}

int IsPreprocEnabled(SnortConfig *sc, uint32_t preproc_id)
{
    PreprocEvalFuncNode *node;
    tSfPolicyId policy_id = getParserPolicy(sc);
    SnortPolicy *p;

    if (sc == NULL)
    {
        FatalError("%s(%d) Snort config for parsing is NULL.\n",
                   __FILE__, __LINE__);
    }

    p = sc->targeted_policies[policy_id];
    if (p == NULL)
        return 0;

    for (node = p->preproc_eval_funcs; node != NULL; node = node->next)
    {
        if (node->preproc_id == preproc_id)
            return 1;
    }

    return 0;
}

PreprocEvalFuncNode * AddFuncToPreprocList(SnortConfig *sc, PreprocEvalFunc pp_eval_func, uint16_t priority,
                                           uint32_t preproc_id, uint32_t proto_mask)
{
    PreprocEvalFuncNode *node;
    tSfPolicyId policy_id = getParserPolicy(sc);
    SnortPolicy *p;

    if (sc == NULL)
    {
        FatalError("%s(%d) Snort config for parsing is NULL.\n",
                   __FILE__, __LINE__);
    }

    p = sc->targeted_policies[policy_id];
    if (p == NULL)
        return NULL;

    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,
                            "Adding preprocessor function ID %d/bit %d/pri %d to list\n",
                            preproc_id, p->num_preprocs, priority););

    node = (PreprocEvalFuncNode *)SnortAlloc(sizeof(PreprocEvalFuncNode));

    if (p->preproc_eval_funcs == NULL)
    {
        p->preproc_eval_funcs = node;
    }
    else
    {
        PreprocEvalFuncNode *tmp = p->preproc_eval_funcs;
        PreprocEvalFuncNode *last = NULL;

        do
        {
            if (tmp->preproc_id == preproc_id)
            {
                free(node);
                FatalError("Preprocessor already registered with ID %d\n",
                           preproc_id);
            }

            /* Insert higher priority preprocessors first.  Lower priority
             * number means higher priority */
            if (priority < tmp->priority)
                break;

            last = tmp;
            tmp = tmp->next;

        } while (tmp != NULL);

        /* Priority higher than first item in list */
        if (last == NULL)
        {
            node->next = tmp;
            p->preproc_eval_funcs = node;
        }
        else
        {
            node->next = tmp;
            last->next = node;
        }
    }

    node->func = pp_eval_func;
    node->priority = priority;
    node->preproc_id = preproc_id;
    node->preproc_bit = (UINT64_C(1) << preproc_id);
    node->proto_mask = proto_mask;

    p->num_preprocs++;
    p->preproc_proto_mask |= proto_mask;
    p->preproc_bit_mask |= node->preproc_bit;

    return node;
}

void AddFuncToPreprocListAllNapPolicies(struct _SnortConfig *sc, PreprocEvalFunc pp_eval_func, uint16_t priority,
                                        uint32_t preproc_id, uint32_t proto_mask)
{
    tSfPolicyId save_policy_id = getParserPolicy( sc );
    uint32_t i;

    if (sc == NULL)
        FatalError("%s(%d) Snort config for parsing is NULL.\n", __FILE__, __LINE__);

    // preprocs are only registered in NAP policies so if num_prerocs > 0 then policy is NAP, this works here
    //  because this func is always called after all policies have been parsed and preprocs configured per policy
    //  have already registered...
    for( i = 0; i < sc->num_policies_allocated; i++ )
        if( ( sc->targeted_policies[ i ] != NULL ) && ( sc->targeted_policies[ i ]->num_preprocs > 0 ) )
        {
            setParserPolicy( sc, i );
            AddFuncToPreprocList( sc, pp_eval_func, priority, preproc_id, proto_mask );
        }

    setParserPolicy( sc, save_policy_id );
 }


PreprocMetaEvalFuncNode * AddFuncToPreprocMetaEvalList(
    SnortConfig *sc,
    PreprocMetaEvalFunc pp_meta_eval_func,
    uint16_t priority,
    uint32_t preproc_id)
{
    PreprocMetaEvalFuncNode *node;
    tSfPolicyId policy_id = getDefaultPolicy( );
    SnortPolicy *p;

    if (sc == NULL)
    {
        FatalError("%s(%d) Snort config for parsing is NULL.\n",
                   __FILE__, __LINE__);
    }

#ifndef HAVE_DAQ_ACQUIRE_WITH_META
    WarningMessage("Metadata not available for processing.  Not registering Preprocessor Meta Eval id %d\n", preproc_id);
    return NULL; // Not supported
#endif

    p = sc->targeted_policies[policy_id];
    if (p == NULL)
        return NULL;

    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,
                            "Adding preprocessor function ID %d/bit %d/pri %d to list\n",
                            preproc_id, p->num_preprocs, priority););

    node = (PreprocMetaEvalFuncNode *)SnortAlloc(sizeof(PreprocMetaEvalFuncNode));

    if (p->preproc_meta_eval_funcs == NULL)
    {
        p->preproc_meta_eval_funcs = node;
        SetupMetadataCallback();
    }
    else
    {
        PreprocMetaEvalFuncNode *tmp = p->preproc_meta_eval_funcs;
        PreprocMetaEvalFuncNode *last = NULL;

        do
        {
            if (tmp->preproc_id == preproc_id)
            {
                free(node);
                FatalError("Preprocessor Meta Eval already registered with ID %d\n",
                           preproc_id);
            }

            /* Insert higher priority preprocessors first.  Lower priority
             * number means higher priority */
            if (priority < tmp->priority)
                break;

            last = tmp;
            tmp = tmp->next;

        } while (tmp != NULL);

        /* Priority higher than first item in list */
        if (last == NULL)
        {
            node->next = tmp;
            p->preproc_meta_eval_funcs = node;
        }
        else
        {
            node->next = tmp;
            last->next = node;
        }
    }

    node->func = pp_meta_eval_func;
    node->priority = priority;
    node->preproc_id = preproc_id;
    node->preproc_bit = (UINT64_C(1) << preproc_id);

    p->num_meta_preprocs++;
    p->preproc_meta_bit_mask |= node->preproc_bit;

    return node;
}

void AddFuncToPreprocPostConfigList(SnortConfig *sc, PreprocPostConfigFunc pp_post_config_func,
                                    void *data)
{
    PreprocPostConfigFuncNode *node;

    if (sc == NULL)
    {
        FatalError("%s(%d) Snort config for parsing is NULL.\n",
                   __FILE__, __LINE__);
    }

    node = (PreprocPostConfigFuncNode *)SnortAlloc(sizeof(PreprocPostConfigFuncNode));

    if (sc->preproc_post_config_funcs == NULL)
    {
        sc->preproc_post_config_funcs = node;
    }
    else
    {
        PreprocPostConfigFuncNode *tmp = sc->preproc_post_config_funcs;

        while (tmp->next != NULL)
            tmp = tmp->next;

        tmp->next = node;
    }

    node->data = data;
    node->func = pp_post_config_func;
}

void PostConfigPreprocessors(SnortConfig *sc)
{
    PreprocPostConfigFuncNode *list;

    if (sc == NULL)
    {
        FatalError("%s(%d) Snort config is NULL.\n",
                   __FILE__, __LINE__);
    }

    list = sc->preproc_post_config_funcs;

    for (; list != NULL; list = list->next)
    {
        if (list->func != NULL)
            list->func(sc, list->data);
    }
}

void FilterConfigPreprocessors(SnortConfig *sc)
{
    tSfPolicyId policy_id;
    SnortPolicy *p;
    PreprocEvalFuncNode *node;
    PreprocEvalFuncNode **list;
    PreprocEvalFuncNode **free_list;

    if (sc == NULL)
    {
        ParseError("%s(%d) Snort config is NULL.\n",
                   __FILE__, __LINE__);
    }

    if (!sc->disable_all_policies)
        return;

    policy_id = getParserPolicy(sc);
    p = sc->targeted_policies[policy_id];
    if (p == NULL)
        return;

    list = &p->preproc_eval_funcs;
    free_list = &p->unused_preproc_eval_funcs;

    while ((node = *list) != NULL)
    {
        if (node->preproc_bit & sc->reenabled_preprocessor_bits)
        {
            list = &node->next;
        }
        else
        {
            *list = node->next;
            node->next = NULL;
            *free_list = node;
            free_list = &node->next;
        }
    }
}

#ifdef SNORT_RELOAD
void SwapPreprocConfigurations(SnortConfig *sc)
{
    PreprocessorSwapData *node;
    PreprocConfigFuncNode *preproc;

    for (node = sc->preprocSwapData; node != NULL; node = node->next)
    {
        if ((preproc = node->preprocNode) && preproc->reload_swap_func)
            node->data = preproc->reload_swap_func(sc, node->data);
    }
}

void FreeSwappedPreprocConfigurations(struct _SnortConfig *sc)
{
    PreprocessorSwapData *node;
    PreprocConfigFuncNode *preproc;

    for (node = sc->preprocSwapData; node != NULL; node = node->next)
    {
        if (node->data && (preproc = node->preprocNode) && preproc->reload_swap_free_func)
        {
            preproc->reload_swap_free_func(node->data);
            node->data = NULL;
        }
    }
}
#endif

void AddFuncToConfigCheckList(SnortConfig *sc, PreprocCheckConfigFunc pp_chk_config_func)
{
    PreprocCheckConfigFuncNode *node;

    if (sc == NULL)
    {
        FatalError("%s(%d) Snort config for parsing is NULL.\n",
                   __FILE__, __LINE__);
    }

    node = (PreprocCheckConfigFuncNode *)SnortAlloc(sizeof(PreprocCheckConfigFuncNode));

    if (sc->preproc_config_check_funcs == NULL)
    {
        sc->preproc_config_check_funcs = node;
    }
    else
    {
        PreprocCheckConfigFuncNode *tmp = sc->preproc_config_check_funcs;

        while (tmp->next != NULL)
            tmp = tmp->next;

        tmp->next = node;
    }

    node->func = pp_chk_config_func;
}

/* functions to aid in cleaning up after plugins */
void AddFuncToPreprocCleanExitList(PreprocSignalFunc pp_sig_func, void *arg,
                                   uint16_t priority, uint32_t preproc_id)
{
    AddFuncToPreprocSignalList(pp_sig_func, arg, &preproc_clean_exit_funcs, priority, preproc_id);
}

void AddFuncToPreprocShutdownList(PreprocSignalFunc pp_shutdown_func, void *arg,
                                  uint16_t priority, uint32_t preproc_id)
{
    AddFuncToPreprocSignalList(pp_shutdown_func, arg, &preproc_shutdown_funcs, priority, preproc_id);
}

void AddFuncToPreprocResetList(PreprocSignalFunc pp_sig_func, void *arg,
                               uint16_t priority, uint32_t preproc_id)
{
    AddFuncToPreprocSignalList(pp_sig_func, arg, &preproc_reset_funcs, priority, preproc_id);
}

void AddFuncToPreprocResetStatsList(PreprocSignalFunc pp_sig_func, void *arg,
                                    uint16_t priority, uint32_t preproc_id)
{
    AddFuncToPreprocSignalList(pp_sig_func, arg, &preproc_reset_stats_funcs, priority, preproc_id);
}

static void AddFuncToPreprocSignalList(PreprocSignalFunc pp_sig_func, void *arg,
                                       PreprocSignalFuncNode **list,
                                       uint16_t priority, uint32_t preproc_id)
{
    PreprocSignalFuncNode *node;

    if (list == NULL)
        return;

    node = (PreprocSignalFuncNode *)SnortAlloc(sizeof(PreprocSignalFuncNode));

    if (*list == NULL)
    {
        *list = node;
    }
    else
    {
        PreprocSignalFuncNode *tmp = *list;
        PreprocSignalFuncNode *last = NULL;

        do
        {
            /* Insert higher priority stuff first.  Lower priority
             * number means higher priority */
            if (priority < tmp->priority)
                break;

            last = tmp;
            tmp = tmp->next;

        } while (tmp != NULL);

        /* Priority higher than first item in list */
        if (last == NULL)
        {
            node->next = tmp;
            *list = node;
        }
        else
        {
            node->next = tmp;
            last->next = node;
        }
    }

    node->func = pp_sig_func;
    node->arg = arg;
    node->preproc_id = preproc_id;
    node->priority = priority;
}

void AddFuncToPeriodicCheckList(PeriodicFunc periodic_func, void *arg,
        uint16_t priority, uint32_t preproc_id, uint32_t period )
{
    PeriodicCheckFuncNode **list= &periodic_check_funcs;
    PeriodicCheckFuncNode *node;


    node = (PeriodicCheckFuncNode *)SnortAlloc(sizeof(PeriodicCheckFuncNode));

    if (*list == NULL)
    {
        *list = node;
    }
    else
    {
        PeriodicCheckFuncNode *tmp = *list;
        PeriodicCheckFuncNode *last = NULL;

        do
        {
            /* Insert higher priority stuff first.  Lower priority
             * number means higher priority */
            if (priority < tmp->priority)
                break;

            last = tmp;
            tmp = tmp->next;

        } while (tmp != NULL);

        /* Priority higher than first item in list */
        if (last == NULL)
        {
            node->next = tmp;
            *list = node;
        }
        else
        {
            node->next = tmp;
            last->next = node;
        }
    }

    node->func = periodic_func;
    node->arg = arg;
    node->preproc_id = preproc_id;
    node->priority = priority;
    node->period = period;
    node->time_left = period;
}

void FreePreprocConfigFuncs(void)
{
    PreprocConfigFuncNode *head = preproc_config_funcs;
    PreprocConfigFuncNode *tmp;

    while (head != NULL)
    {
        tmp = head->next;
        if (head->keyword != NULL)
            free(head->keyword);
        free(head);
        head = tmp;
    }
}

void FreePreprocCheckConfigFuncs(PreprocCheckConfigFuncNode *head)
{
    PreprocCheckConfigFuncNode *tmp;

    while (head != NULL)
    {
        tmp = head->next;
        free(head);
        head = tmp;
    }
}

void FreePreprocPostConfigFuncs(PreprocPostConfigFuncNode *head)
{
    PreprocPostConfigFuncNode *tmp;

    while (head != NULL)
    {
        tmp = head->next;
        free(head);
        head = tmp;
    }
}

void FreePreprocStatsFuncs(PreprocStatsFuncNode *head)
{
    PreprocStatsFuncNode *tmp;

    while (head != NULL)
    {
        tmp = head->next;
        if (head->keyword != NULL)
            free(head->keyword);
        free(head);
        head = tmp;
    }
}

void FreePreprocEvalFuncs(PreprocEvalFuncNode *head)
{
    PreprocEvalFuncNode *tmp;

    while (head != NULL)
    {
        tmp = head->next;
        //if (head->context)
        //    free(head->context);
        free(head);
        head = tmp;
    }
}

void FreePreprocMetaEvalFuncs(PreprocMetaEvalFuncNode *head)
{
    PreprocMetaEvalFuncNode *tmp;

    while (head != NULL)
    {
        tmp = head->next;
        //if (head->context)
        //    free(head->context);
        free(head);
        head = tmp;
    }
}

void FreePreprocSigFuncs(PreprocSignalFuncNode *head)
{
    PreprocSignalFuncNode *tmp;

    while (head != NULL)
    {
        tmp = head->next;
        /* don't free sig->arg, that's free'd by the CleanExit func */
        free(head);
        head = tmp;
    }
}

void FreePeriodicFuncs(PeriodicCheckFuncNode *head)
{
    PeriodicCheckFuncNode *tmp;

    while (head != NULL)
    {
        tmp = head->next;
        /* don't free sig->arg, that's free'd by the CleanExit func */
        free(head);
        head = tmp;
    }
}

int CheckPreprocessorsConfig(SnortConfig *sc)
{
    PreprocCheckConfigFuncNode *idx;
    int rval;

    if (sc == NULL)
    {
        FatalError("%s(%d) Snort config is NULL.\n",
                   __FILE__, __LINE__);
    }

    idx = sc->preproc_config_check_funcs;

    LogMessage("Verifying Preprocessor Configurations!\n");

    while(idx != NULL)
    {
        if ((rval = idx->func(sc)))
            return rval;
        idx = idx->next;
    }
    return 0;
}

#ifdef SNORT_RELOAD
int VerifyReloadedPreprocessors(SnortConfig *sc)
{
    int rval;
    PreprocessorSwapData *node;
    PreprocConfigFuncNode *preproc;

    for (node = sc->preprocSwapData; node != NULL; node = node->next)
    {
        if (node->data && (preproc = node->preprocNode) && preproc->reload_verify_func &&
            (rval = preproc->reload_verify_func(sc, node->data)))
        {
            return rval;
        }
    }

    return 0;
}

void FreePreprocessorReloadData(SnortConfig *sc)
{
    PreprocessorSwapData *node;
    PreprocConfigFuncNode *preproc;

    while ((node = sc->preprocSwapData))
    {
        sc->preprocSwapData = node->next;
        if (node->data && (preproc = node->preprocNode) && preproc->reload_swap_free_func)
            preproc->reload_swap_free_func(node->data);
        free(node);
    }
}
#endif

void DisableAllPolicies(SnortConfig *sc)
{
    if (!sc->disable_all_policies)
    {
        sc->disable_all_policies = 1;
        sc->reenabled_preprocessor_bits = (UINT64_C(1) << PP_FRAG3);
        sc->reenabled_preprocessor_bits |= (UINT64_C(1) << PP_STREAM);
        sc->reenabled_preprocessor_bits |= (UINT64_C(1) << PP_PERFMONITOR);
    }
}

int ReenablePreprocBit(SnortConfig *sc, unsigned int preproc_id)
{
    sc->reenabled_preprocessor_bits |= (UINT64_C(1) << preproc_id);
    return 0;
}

/***************************** Output Plugin API  *****************************/
extern OutputConfigFuncNode *output_config_funcs;

static void AppendOutputFuncList(OutputFunc, void *, OutputFuncNode **);

void RegisterOutputPlugins(void)
{
    LogMessage("Initializing Output Plugins!\n");

    AlertSyslogSetup();
    LogTcpdumpSetup();
    AlertFastSetup();
    AlertFullSetup();
#ifndef WIN32
    /* Win32 doesn't support AF_UNIX sockets */
    AlertUnixSockSetup();
#endif /* !WIN32 */
    AlertCSVSetup();
    LogNullSetup();
    Unified2Setup();
    LogAsciiSetup();

#ifdef DUMP_BUFFER
    LogBufferDumpSetup();
#endif

#ifdef LINUX
    /* This uses linux only capabilities */
    AlertSFSocket_Setup();
#endif

    AlertTestSetup();
}

/****************************************************************************
 *
 * Function: RegisterOutputPlugin(char *, void (*func)(Packet *, u_char *))
 *
 * Purpose:  Associates an output statement with its function.
 *
 * Arguments: keyword => The output keyword to associate with the
 *                       output processor
 *            type => alert or log types
 *            *func => function pointer to the handler
 *
 * Returns: void function
 *
 ***************************************************************************/
void RegisterOutputPlugin(char *keyword, int type_flags, OutputConfigFunc oc_func)
{
    OutputConfigFuncNode *node = (OutputConfigFuncNode *)SnortAlloc(sizeof(OutputConfigFuncNode));


    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,"Registering keyword:output => %s:%p\n",
                            keyword, oc_func););

    if (output_config_funcs == NULL)
    {
        output_config_funcs = node;
    }
    else
    {
        OutputConfigFuncNode *tmp = output_config_funcs;
        OutputConfigFuncNode *last;

        do
        {
            if (strcasecmp(tmp->keyword, keyword) == 0)
            {
                free(node);
                FatalError("Duplicate output keyword: %s\n", keyword);
            }

            last = tmp;
            tmp = tmp->next;

        } while (tmp != NULL);

        last->next = node;
    }

    node->keyword = SnortStrdup(keyword);
    node->config_func = oc_func;
    node->output_type_flags = type_flags;
}

void RemoveOutputPlugin(char *keyword)
{
    OutputConfigFuncNode *head = output_config_funcs;

    if (!head ||(keyword == NULL))
        return;

    /*If head node, remove head*/
    if(head->keyword != NULL)
    {
        if (strcasecmp(head->keyword, keyword) == 0)
        {
            output_config_funcs = head->next;
            free(head->keyword);
            free(head);
            return;
        }
    }

    while (head->next != NULL)
    {
        OutputConfigFuncNode *next;
        next = head->next;
        if(next->keyword != NULL )
        {
            if (strcasecmp(next->keyword, keyword) == 0)
            {
                head->next = next->next;
                free(next->keyword);
                free(next);
                break;
            }
        }
        head = head->next;
    }

    return;
}

OutputConfigFunc GetOutputConfigFunc(char *keyword)
{
    OutputConfigFuncNode *head = output_config_funcs;

    if (keyword == NULL)
        return NULL;

    while (head != NULL)
    {
        if (strcasecmp(head->keyword, keyword) == 0)
           return head->config_func;

        head = head->next;
    }

    return NULL;
}


int GetOutputTypeFlags(char *keyword)
{
    OutputConfigFuncNode *head = output_config_funcs;

    if (keyword == NULL)
        return 0;

    while (head != NULL)
    {
        if (strcasecmp(head->keyword, keyword) == 0)
           return head->output_type_flags;

        head = head->next;
    }

    return 0;
}

void FreeOutputConfigFuncs(void)
{
    OutputConfigFuncNode *head = output_config_funcs;
    OutputConfigFuncNode *tmp;

    while (head != NULL)
    {
        tmp = head->next;
        if (head->keyword != NULL)
            free(head->keyword);
        free(head);
        head = tmp;
    }
}

void FreeOutputList(OutputFuncNode *list)
{
    while (list != NULL)
    {
        OutputFuncNode *tmp = list;

        list = list->next;
        free(tmp);
    }
}

/****************************************************************************
 *
 * Function: DumpOutputPlugins()
 *
 * Purpose:  Prints the keyword->preprocess list
 *
 * Arguments: None.
 *
 * Returns: void function
 *
 ***************************************************************************/
void DumpOutputPlugins(void)
{
    OutputConfigFuncNode *idx = output_config_funcs;

    LogMessage("-------------------------------------------------\n");
    LogMessage(" Keyword     |          Output @ \n");
    LogMessage("-------------------------------------------------\n");
    while(idx != NULL)
    {
        LogMessage("%-13s:       %p\n", idx->keyword, idx->config_vfunc);
        idx = idx->next;
    }
    LogMessage("-------------------------------------------------\n\n");
}

void AddFuncToOutputList(SnortConfig *sc, OutputFunc o_func, OutputType type, void *arg)
{
    switch (type)
    {
        case OUTPUT_TYPE__ALERT:
            if (sc->head_tmp != NULL)
                AppendOutputFuncList(o_func, arg, &sc->head_tmp->AlertList);
            else
                AppendOutputFuncList(o_func, arg, &AlertList);

            break;

        case OUTPUT_TYPE__LOG:
            if (sc->head_tmp != NULL)
                AppendOutputFuncList(o_func, arg, &sc->head_tmp->LogList);
            else
                AppendOutputFuncList(o_func, arg, &LogList);

            break;

        default:
            /* just to be error-prone */
            FatalError("Unknown output type: %i. Possible bug, please "
                       "report.\n", type);
    }
}

#ifdef DUMP_BUFFER
/****************************************************************************
 *
 * Function: AddBDFuncToOutputList()
 *
 * Purpose: This function is called only when buffer dump is enabled. For
 * BufferDump output plugin, bdfptr points to LogBufferDump function. For all
 * other output plugins, bdfptr points to NULL.
 *
 * Arguments: sc => snort config
 *            o_func => output plugin function
 *            type => alert or log types
 *            arg => pointer to output stream to which buffers will be dumped
 *
 * Returns: void function
 *
 ***************************************************************************/

void AddBDFuncToOutputList(SnortConfig *sc, OutputFunc o_func, OutputType type, void *arg)
{

    OutputFuncNode *node;

    if (sc->head_tmp != NULL)
        node = sc->head_tmp->LogList;

    else
        node = LogList;

    while (node->next != NULL)
        node = node->next;

    node->bdfptr = o_func;
}
#endif

void AppendOutputFuncList(OutputFunc o_func, void *arg, OutputFuncNode **list)
{
    OutputFuncNode *node;

    if (list == NULL)
        return;

    node = (OutputFuncNode *)SnortAlloc(sizeof(OutputFuncNode));

    if (*list == NULL)
    {
        *list = node;
    }
    else
    {
        OutputFuncNode *tmp = *list;

        while (tmp->next != NULL)
            tmp = tmp->next;

        tmp->next = node;
    }

    node->func = o_func;
    node->arg = arg;

#ifdef DUMP_BUFFER
    node->bdfptr = NULL;
#endif

}


/************************** Miscellaneous Functions  **************************/

/* functions to aid in cleaning up after plugins
 * Used for both rule options and output.  Preprocessors have their own */
static inline void _AddFuncToPostConfigList(PostConfigFunc pl_post_func, void *arg, PostConfigFuncNode **list)
{
    PostConfigFuncNode *node;

    node = (PostConfigFuncNode *)SnortAlloc(sizeof(PostConfigFuncNode));

    if (*list == NULL)
    {
        *list = node;
    }
    else
    {
        PostConfigFuncNode *tmp = *list;

        while (tmp->next != NULL)
            tmp = tmp->next;

        tmp->next = node;
    }

    node->func = pl_post_func;
    node->arg = arg;
}

#ifdef SNORT_RELOAD
void AddFuncToReloadList(PostConfigFunc pl_post_func, void *arg)
{
    _AddFuncToPostConfigList(pl_post_func, arg, &plugin_reload_funcs);
}
#endif

void AddFuncToCleanExitList(PluginSignalFunc pl_sig_func, void *arg)
{
    AddFuncToSignalList(pl_sig_func, arg, &plugin_clean_exit_funcs);
}

void AddFuncToShutdownList(PluginSignalFunc pl_sig_func, void *arg)
{
    AddFuncToSignalList(pl_sig_func, arg, &plugin_shutdown_funcs);
}

void AddFuncToPostConfigList(SnortConfig *sc, PostConfigFunc pl_post_func, void *arg)
{
    _AddFuncToPostConfigList(pl_post_func, arg, &sc->plugin_post_config_funcs);
}

void AddFuncToSignalList(PluginSignalFunc pl_sig_func, void *arg, PluginSignalFuncNode **list)
{
    PluginSignalFuncNode *node;

    if (list == NULL)
        return;

    node = (PluginSignalFuncNode *)SnortAlloc(sizeof(PluginSignalFuncNode));

    if (*list == NULL)
    {
        *list = node;
    }
    else
    {
        PluginSignalFuncNode *tmp = *list;

        while (tmp->next != NULL)
            tmp = tmp->next;

        tmp->next = node;
    }

    node->func = pl_sig_func;
    node->arg = arg;
}

void AddFuncToRuleOptParseCleanupList(RuleOptParseCleanupFunc ro_parse_clean_func)
{
    RuleOptParseCleanupNode *node =
        (RuleOptParseCleanupNode *)SnortAlloc(sizeof(RuleOptParseCleanupNode));

    if (rule_opt_parse_cleanup_list == NULL)
    {
        rule_opt_parse_cleanup_list = node;
    }
    else
    {
        RuleOptParseCleanupNode *tmp = rule_opt_parse_cleanup_list;

        while (tmp->next != NULL)
            tmp = tmp->next;

        tmp->next = node;
    }

    node->func = ro_parse_clean_func;
}

void RuleOptParseCleanup(void)
{
    RuleOptParseCleanupNode *list = rule_opt_parse_cleanup_list;

    for (; list != NULL; list = list->next)
    {
        if (list->func != NULL)
            list->func();
    }
}

void FreeRuleOptParseCleanupList(RuleOptParseCleanupNode *head)
{
    while (head != NULL)
    {
        RuleOptParseCleanupNode *tmp = head;

        head = head->next;
        free(tmp);
    }
}

void RegisterGetHttpXffFields(GetHttpXffFieldsFunc fn)
{
    if (!getHttpXffFieldsFunc) getHttpXffFieldsFunc = fn;
}

char** GetHttpXffFields(int* nFields)
{
    if (getHttpXffFieldsFunc) return getHttpXffFieldsFunc(nFields);
    else return NULL;
}
