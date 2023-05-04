/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2005-2013 Sourcefire, Inc.
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

/**************************************************************************
 *
 * spp_smtp.c
 *
 * Author: Andy Mullican
 *
 * Description:
 *
 * This file initializes SMTP as a Snort preprocessor.
 *
 * This file registers the SMTP initialization function,
 * adds the SMTP function into the preprocessor list.
 *
 * In general, this file is a wrapper to SMTP functionality,
 * by interfacing with the Snort preprocessor functions.  The rest
 * of SMTP should be separate from the preprocessor hooks.
 *
 **************************************************************************/

#include <assert.h>
#include <sys/types.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_types.h"
#include "spp_smtp.h"
#include "sf_preproc_info.h"
#include "snort_smtp.h"
#include "smtp_util.h"
#include "smtp_config.h"
#include "smtp_log.h"
#include "smtp_paf.h"

#include "preprocids.h"
#include "sf_snort_packet.h"
#include "sf_dynamic_preprocessor.h"
#include "snort_debug.h"
#include "sfPolicy.h"
#include "sfPolicyUserData.h"
#include "smtp_api.h"

#include "profiler.h"
#ifdef PERF_PROFILING
PreprocStats smtpPerfStats;
PreprocStats smtpDetectPerfStats;
int smtpDetectCalled = 0;
#endif

#include "sf_types.h"
#include "mempool.h"
#include "snort_bounds.h"

#include "file_api.h"

#ifdef REG_TEST
#include "reg_test.h"
#endif

#ifdef DUMP_BUFFER
#include "smtp_buffer_dump.h"
#endif

const int MAJOR_VERSION = 1;
const int MINOR_VERSION = 1;
const int BUILD_VERSION = 9;
const char *PREPROC_NAME = "SF_SMTP";
const char *PROTOCOL_NAME = "SMTP";

#define SetupSMTP DYNAMIC_PREPROC_SETUP

MemPool *smtp_mime_mempool = NULL;
SMTP_Stats smtp_stats;
MemPool *smtp_mempool = NULL;
tSfPolicyUserContextId smtp_config = NULL;
SMTPConfig *smtp_eval_config = NULL;

extern SMTP smtp_no_session;
extern int16_t smtp_proto_id;

static void SMTPInit(struct _SnortConfig *, char *);
static void SMTPDetect(void *, void *context);
static void SMTPCleanExitFunction(int, void *);
static void SMTPResetFunction(int, void *);
static void SMTPResetStatsFunction(int, void *);
static void enablePortStreamServices(struct _SnortConfig *, SMTPConfig *, tSfPolicyId);
static void SMTP_RegXtraDataFuncs(SMTPConfig *config);
static void SMTP_PrintStats(int);
static void DisplaySMTPStats (uint16_t type, void *old_context, struct _THREAD_ELEMENT *te, ControlDataSendFunc f);
#ifdef TARGET_BASED
static void _addServicesToStreamFilter(struct _SnortConfig *, tSfPolicyId);
#endif
static int SMTPCheckConfig(struct _SnortConfig *);

#ifdef SNORT_RELOAD
static int SMTPMempoolFreeUsedBucket(MemPool *memory_pool);
static unsigned SMTPReloadMimeMempoolAdjust(unsigned smtpMaxWork);
static unsigned SMTPReloadLogMempoolAdjust(unsigned smtpMaxWork);
static bool SMTPMimeReloadAdjust(bool idle, tSfPolicyId raPolicyId, void* userData);
static bool SMTPLogReloadAdjust(bool idle, tSfPolicyId raPolicyId, void* userData);
static void SMTPReload(struct _SnortConfig *, char *, void **);
static int SMTPReloadVerify(struct _SnortConfig *, void *);
static void * SMTPReloadSwap(struct _SnortConfig *, void *);
static void SMTPReloadSwapFree(void *);
#endif
void SmtpApiInit(SmtpAPI *api);

/*
 * Function: SetupSMTP()
 *
 * Purpose: Registers the preprocessor keyword and initialization
 *          function into the preprocessor list.  This is the function that
 *          gets called from InitPreprocessors() in plugbase.c.
 *
 * Arguments: None.
 *
 * Returns: void function
 *
 */
void SetupSMTP(void)
{
    /* link the preprocessor keyword to the init function in the preproc list */
#ifndef SNORT_RELOAD
    _dpd.registerPreproc("smtp", SMTPInit);
#else
    _dpd.registerPreproc("smtp", SMTPInit, SMTPReload,
                         SMTPReloadVerify, SMTPReloadSwap, SMTPReloadSwapFree);
#endif
}

#ifdef REG_TEST
static inline void PrintSMTPSize(void)
{
    _dpd.logMsg("\nSMTP Session Size: %lu\n", (long unsigned int)sizeof(SMTP));
}
#endif

/*
 * Function: SMTPInit(char *)
 *
 * Purpose: Calls the argument parsing function, performs final setup on data
 *          structs, links the preproc function into the function list.
 *
 * Arguments: args => ptr to argument string
 *
 * Returns: void function
 *
 */
static void SMTPInit(struct _SnortConfig *sc, char *args)
{
    SMTPToken *tmp;
    tSfPolicyId policy_id = _dpd.getParserPolicy(sc);
    SMTPConfig * pPolicyConfig = NULL;

#ifdef REG_TEST
    PrintSMTPSize();
#endif
    
    _dpd.registerMemoryStatsFunc(PP_SMTP, SMTP_Print_Mem_Stats);

    if (smtp_config == NULL)
    {
        //create a context
        smtp_config = sfPolicyConfigCreate();
        if (smtp_config == NULL)
        {
            DynamicPreprocessorFatalMessage("Not enough memory to create SMTP "
                                            "configuration.\n");
        }

        /* Initialize the searches not dependent on configuration.
         * headers, reponsed, data, mime boundary regular expression */
        SMTP_SearchInit();

        /* zero out static SMTP global used for stateless SMTP or if there
         * is no session pointer */
        memset(&smtp_no_session, 0, sizeof(SMTP));

        /* Put the preprocessor function into the function list */
        /* _dpd.addPreproc(SMTPDetect, PRIORITY_APPLICATION, PP_SMTP, PROTO_BIT__TCP);*/
        _dpd.addPreprocExit(SMTPCleanExitFunction, NULL, PRIORITY_LAST, PP_SMTP);
        _dpd.addPreprocReset(SMTPResetFunction, NULL, PRIORITY_LAST, PP_SMTP);
        _dpd.registerPreprocStats(SMTP_PROTO_REF_STR, SMTP_PrintStats);
        _dpd.addPreprocResetStats(SMTPResetStatsFunction, NULL, PRIORITY_LAST, PP_SMTP);
        _dpd.addPreprocConfCheck(sc, SMTPCheckConfig);
        _dpd.controlSocketRegisterHandler(CS_TYPE_SMTP_STATS, NULL, NULL, &DisplaySMTPStats);
#ifdef TARGET_BASED
        smtp_proto_id = _dpd.findProtocolReference(SMTP_PROTO_REF_STR);
        if (smtp_proto_id == SFTARGET_UNKNOWN_PROTOCOL)
            smtp_proto_id = _dpd.addProtocolReference(SMTP_PROTO_REF_STR);

        // register with session to handle applications
        _dpd.sessionAPI->register_service_handler( PP_SMTP, smtp_proto_id );

        DEBUG_WRAP(DebugMessage(DEBUG_SMTP,"SMTP: Target-based: Proto id for %s: %u.\n",
                                SMTP_PROTO_REF_STR, smtp_proto_id););
#endif

#ifdef PERF_PROFILING
        _dpd.addPreprocProfileFunc("smtp", (void*)&smtpPerfStats, 0, _dpd.totalPerfStats, NULL);
#endif
    }

    sfPolicyUserPolicySet (smtp_config, policy_id);
    pPolicyConfig = (SMTPConfig *)sfPolicyUserDataGetCurrent(smtp_config);
    if (pPolicyConfig != NULL)
    {
        DynamicPreprocessorFatalMessage("Can only configure SMTP preprocessor once.\n");
    }

    pPolicyConfig = (SMTPConfig *)_dpd.snortAlloc(1, sizeof(SMTPConfig), PP_SMTP,
                                       PP_MEM_CATEGORY_CONFIG);

    if (pPolicyConfig == NULL)
    {
        DynamicPreprocessorFatalMessage("Not enough memory to create SMTP "
                                        "configuration.\n");
    }

    sfPolicyUserDataSetCurrent(smtp_config, pPolicyConfig);

    SMTP_RegXtraDataFuncs(pPolicyConfig);
    SMTP_InitCmds(pPolicyConfig);
    SMTP_ParseArgs(pPolicyConfig, args);

    SMTP_CheckConfig(pPolicyConfig, smtp_config);
    SMTP_PrintConfig(pPolicyConfig);

    if(pPolicyConfig->disabled)
        return;

    _dpd.addPreproc(sc, SMTPDetect, PRIORITY_APPLICATION, PP_SMTP, PROTO_BIT__TCP);

    if (_dpd.streamAPI == NULL)
    {
        DynamicPreprocessorFatalMessage("Streaming & reassembly must be enabled "
                "for SMTP preprocessor\n");
    }

    /* Command search - do this here because it's based on configuration */
    pPolicyConfig->cmd_search_mpse = _dpd.searchAPI->search_instance_new();
    if (pPolicyConfig->cmd_search_mpse == NULL)
    {
        DynamicPreprocessorFatalMessage("Could not allocate SMTP "
                                        "command search.\n");
    }

    for (tmp = pPolicyConfig->cmds; tmp->name != NULL; tmp++)
    {
        pPolicyConfig->cmd_search[tmp->search_id].name = tmp->name;
        pPolicyConfig->cmd_search[tmp->search_id].name_len = tmp->name_len;

        _dpd.searchAPI->search_instance_add(pPolicyConfig->cmd_search_mpse, tmp->name,
                                            tmp->name_len, tmp->search_id);
    }

    _dpd.searchAPI->search_instance_prep(pPolicyConfig->cmd_search_mpse);

    enablePortStreamServices(sc, pPolicyConfig, policy_id);

#ifdef TARGET_BASED
    _addServicesToStreamFilter(sc, policy_id);
#endif
#ifdef DUMP_BUFFER
    _dpd.registerBufferTracer(getSMTPBuffers, SMTP_BUFFER_DUMP_FUNC);
#endif
    SmtpApiInit(_dpd.smtpApi);
}

/*
 * Function: SMTPDetect(void *, void *)
 *
 * Purpose: Perform the preprocessor's intended function.  This can be
 *          simple (statistics collection) or complex (IP defragmentation)
 *          as you like.  Try not to destroy the performance of the whole
 *          system by trying to do too much....
 *
 * Arguments: p => pointer to the current packet data struct
 *
 * Returns: void function
 *
 */
static void SMTPDetect(void *pkt, void *context)
{
    SFSnortPacket *p = (SFSnortPacket *)pkt;
    tSfPolicyId policy_id = _dpd.getNapRuntimePolicy();
    PROFILE_VARS;

    // preconditions - what we registered for
    assert(IsTCP(p) && p->payload && p->payload_size);

    PREPROC_PROFILE_START(smtpPerfStats);

    DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "SMTP Start (((((((((((((((((((((((((((((((((((((((\n"););

    sfPolicyUserPolicySet (smtp_config, policy_id);

    SnortSMTP(p);

    DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "SMTP End )))))))))))))))))))))))))))))))))))))))))\n\n"););

    PREPROC_PROFILE_END(smtpPerfStats);
#ifdef PERF_PROFILING
    if (PROFILING_PREPROCS && smtpDetectCalled)
    {
        smtpPerfStats.ticks -= smtpDetectPerfStats.ticks;
        /* And Reset ticks to 0 */
        smtpDetectPerfStats.ticks = 0;
        smtpDetectCalled = 0;
    }
#endif

}


/*
 * Function: SMTPCleanExitFunction(int, void *)
 *
 * Purpose: This function gets called when Snort is exiting, if there's
 *          any cleanup that needs to be performed (e.g. closing files)
 *          it should be done here.
 *
 * Arguments: signal => the code of the signal that was issued to Snort
 *            data => any arguments or data structs linked to this
 *                    function when it was registered, may be
 *                    needed to properly exit
 *
 * Returns: void function
 */
static void SMTPCleanExitFunction(int signal, void *data)
{
    SMTP_Free();
    if (mempool_destroy(smtp_mime_mempool) == 0)
    {
        free(smtp_mime_mempool);
        smtp_mime_mempool = NULL;
    }
    if (mempool_destroy(smtp_mempool) == 0)
    {
        free(smtp_mempool);
        smtp_mempool = NULL;
    }

}


static void SMTPResetFunction(int signal, void *data)
{
    return;
}

static void SMTPResetStatsFunction(int signal, void *data)
{
    return;
}

static void enablePortStreamServices(struct _SnortConfig *sc, SMTPConfig *config, tSfPolicyId policy_id)
{
    uint32_t portNum;

    if (config == NULL)
        return;

    for (portNum = 0; portNum < MAXPORTS; portNum++)
    {
        if(config->ports[(portNum/8)] & (1<<(portNum%8)))
        {
            //Add port the port
            _dpd.streamAPI->set_port_filter_status(sc, IPPROTO_TCP, (uint16_t)portNum,
                                                   PORT_MONITOR_SESSION, policy_id, 1);

            register_smtp_paf_port(sc, portNum, policy_id);
            _dpd.streamAPI->register_reassembly_port( NULL, 
                                                  (uint16_t) portNum, 
                                                  SSN_DIR_FROM_SERVER | SSN_DIR_FROM_CLIENT );
            _dpd.sessionAPI->enable_preproc_for_port( sc, PP_SMTP, PROTO_BIT__TCP, (uint16_t) portNum ); 
        }
    }
}

#ifdef TARGET_BASED
static void _addServicesToStreamFilter(struct _SnortConfig *sc, tSfPolicyId policy_id)
{
    _dpd.streamAPI->set_service_filter_status(sc, smtp_proto_id, PORT_MONITOR_SESSION, policy_id, 1);
    register_smtp_paf_service(sc, smtp_proto_id, policy_id);
}
#endif

static int SMTPEnableDecoding(struct _SnortConfig *sc, tSfPolicyUserContextId config,
            tSfPolicyId policyId, void *pData)
{
    SMTPConfig *context = (SMTPConfig *)pData;

    if (pData == NULL)
        return 0;

    if(context->disabled)
        return 0;

    if(_dpd.fileAPI->is_decoding_enabled(&(context->decode_conf)))
        return 1;

    return 0;
}
static int SMTPLogExtraData(struct _SnortConfig *sc, tSfPolicyUserContextId config,
        tSfPolicyId policyId, void *pData)
{
    SMTPConfig *context = (SMTPConfig *)pData;

    if (pData == NULL)
        return 0;

    if(context->disabled)
        return 0;

    if(context->log_config.log_email_hdrs || context->log_config.log_filename ||
            context->log_config.log_mailfrom || context->log_config.log_rcptto)
        return 1;

    return 0;
}

static int CheckFilePolicyConfig(
        struct _SnortConfig *sc,
        tSfPolicyUserContextId config,
        tSfPolicyId policyId,
        void* pData
        )
{
    SMTPConfig *context = (SMTPConfig *)pData;

     /* Use new Snort config to get the max file depth */
     context->decode_conf.file_depth = _dpd.fileAPI->get_max_file_depth(sc, true);
    if (context->decode_conf.file_depth > -1)
        context->log_config.log_filename = 1;
    updateMaxDepth(context->decode_conf.file_depth, &context->decode_conf.max_depth);

    return 0;
}

static int SMTPCheckPolicyConfig(
        struct _SnortConfig *sc,
        tSfPolicyUserContextId config,
        tSfPolicyId policyId,
        void* pData
        )
{
    SMTPConfig *context = (SMTPConfig *)pData;

    _dpd.setParserPolicy(sc, policyId);

    /* In a multiple-policy setting, the SMTP preproc can be turned on in a
       "disabled" state. In this case, we don't require Stream5. */
    if (context->disabled)
        return 0;

    if (_dpd.streamAPI == NULL)
    {
        DynamicPreprocessorFatalMessage("Streaming & reassembly must be enabled "
                                        "for SMTP preprocessor\n");
    }
    return 0;
}

static void SMTP_RegXtraDataFuncs(SMTPConfig *config)
{
    if ((_dpd.streamAPI == NULL) || !config)
        return;
    config->xtra_filename_id = _dpd.streamAPI->reg_xtra_data_cb(SMTP_GetFilename);
    config->xtra_mfrom_id = _dpd.streamAPI->reg_xtra_data_cb(SMTP_GetMailFrom);
    config->xtra_rcptto_id = _dpd.streamAPI->reg_xtra_data_cb(SMTP_GetRcptTo);
    config->xtra_ehdrs_id = _dpd.streamAPI->reg_xtra_data_cb(SMTP_GetEmailHdrs);

}

static int SMTPCheckConfig(struct _SnortConfig *sc)
{
    sfPolicyUserDataIterate (sc, smtp_config, SMTPCheckPolicyConfig);
    sfPolicyUserDataIterate (sc, smtp_config, CheckFilePolicyConfig);
    {
        SMTPConfig *defaultConfig =
                (SMTPConfig *)sfPolicyUserDataGetDefault(smtp_config);

        if (defaultConfig == NULL)
        {
            _dpd.errMsg(
            "SMTP: Must configure a default configuration if you "
            "want to enable smtp decoding.\n");
            return -1;
        }

        if (sfPolicyUserDataIterate(sc, smtp_config, SMTPEnableDecoding) != 0)
        {
            smtp_mime_mempool = (MemPool *) _dpd.fileAPI->init_mime_mempool(defaultConfig->decode_conf.max_mime_mem,
                defaultConfig->decode_conf.max_depth, smtp_mime_mempool, PROTOCOL_NAME);
        }

        if (sfPolicyUserDataIterate(sc, smtp_config, SMTPLogExtraData) != 0)
        {
            smtp_mempool = (MemPool *)_dpd.fileAPI->init_log_mempool(defaultConfig->log_config.email_hdrs_log_depth,
                defaultConfig->memcap, smtp_mempool, PROTOCOL_NAME);
        }
    }
    return 0;
}

static void DisplaySMTPStats (uint16_t type, void *old_context, struct _THREAD_ELEMENT *te, ControlDataSendFunc f)
{
    char buffer[CS_STATS_BUF_SIZE + 1];
    int len = 0;
    if (smtp_stats.sessions) {
        len += snprintf(buffer, CS_STATS_BUF_SIZE, "SMTP Preprocessor Statistics\n"
                "  Total sessions                                    : " STDu64 "\n"
                "  Max concurrent sessions                           : " STDu64 "\n"
                "  Base64 attachments decoded                        : " STDu64 "\n"
                "  Total Base64 decoded bytes                        : " STDu64 "\n"
                "  Quoted-Printable attachments decoded              : " STDu64 "\n"
                "  Total Quoted decoded bytes                        : " STDu64 "\n"
                "  UU attachments decoded                            : " STDu64 "\n"
                "  Total UU decoded bytes                            : " STDu64 "\n"
                "  Non-Encoded MIME attachments extracted            : " STDu64 "\n"
                "  Total Non-Encoded MIME bytes extracted            : " STDu64 "\n"
                , smtp_stats.sessions
                , smtp_stats.max_conc_sessions
                , smtp_stats.mime_stats.attachments[DECODE_B64]
                , smtp_stats.mime_stats.decoded_bytes[DECODE_B64]
                , smtp_stats.mime_stats.attachments[DECODE_QP]
                , smtp_stats.mime_stats.decoded_bytes[DECODE_QP]
                , smtp_stats.mime_stats.attachments[DECODE_UU]
                , smtp_stats.mime_stats.decoded_bytes[DECODE_UU]
                , smtp_stats.mime_stats.attachments[DECODE_BITENC]
                , smtp_stats.mime_stats.decoded_bytes[DECODE_BITENC]);

        if ( smtp_stats.mime_stats.memcap_exceeded )
            len += snprintf(buffer+len, CS_STATS_BUF_SIZE-len, "  Sessions not decoded due to memory unavailability : "
                    STDu64 "\n", smtp_stats.mime_stats.memcap_exceeded);

        if ( smtp_stats.log_memcap_exceeded )
            len += snprintf(buffer+len, CS_STATS_BUF_SIZE-len, "  SMTP Sessions fastpathed due to memcap exceeded: "
                    STDu64 "\n", smtp_stats.log_memcap_exceeded);
    } else {
        len = snprintf(buffer, CS_STATS_BUF_SIZE, "No available SMTP Sessions\n Total sessions : " STDu64 "\n", smtp_stats.sessions);
    }

    if (-1 == f(te, (const uint8_t *)buffer, len)) {
        _dpd.logMsg("Unable to send data to the frontend\n");
    }
}

static void SMTP_PrintStats(int exiting)
{
    _dpd.logMsg("SMTP Preprocessor Statistics\n");
    _dpd.logMsg("  Total sessions                                    : " STDu64 "\n", smtp_stats.sessions);
    _dpd.logMsg("  Max concurrent sessions                           : " STDu64 "\n", smtp_stats.max_conc_sessions);
    if (smtp_stats.sessions > 0)
    {
        _dpd.logMsg("  Base64 attachments decoded                        : " STDu64 "\n", smtp_stats.mime_stats.attachments[DECODE_B64]);
        _dpd.logMsg("  Total Base64 decoded bytes                        : " STDu64 "\n", smtp_stats.mime_stats.decoded_bytes[DECODE_B64]);
        _dpd.logMsg("  Quoted-Printable attachments decoded              : " STDu64 "\n", smtp_stats.mime_stats.attachments[DECODE_QP]);
        _dpd.logMsg("  Total Quoted decoded bytes                        : " STDu64 "\n", smtp_stats.mime_stats.decoded_bytes[DECODE_QP]);
        _dpd.logMsg("  UU attachments decoded                            : " STDu64 "\n", smtp_stats.mime_stats.attachments[DECODE_UU]);
        _dpd.logMsg("  Total UU decoded bytes                            : " STDu64 "\n", smtp_stats.mime_stats.decoded_bytes[DECODE_UU]);
        _dpd.logMsg("  Non-Encoded MIME attachments extracted            : " STDu64 "\n", smtp_stats.mime_stats.attachments[DECODE_BITENC]);
        _dpd.logMsg("  Total Non-Encoded MIME bytes extracted            : " STDu64 "\n", smtp_stats.mime_stats.decoded_bytes[DECODE_BITENC]);
        if ( smtp_stats.mime_stats.memcap_exceeded )
            _dpd.logMsg("  Sessions not decoded due to memory unavailability : " STDu64 "\n", smtp_stats.mime_stats.memcap_exceeded);
        if ( smtp_stats.log_memcap_exceeded )
            _dpd.logMsg("  SMTP Sessions fastpathed due to memcap exceeded: " STDu64 "\n", smtp_stats.log_memcap_exceeded);
    }

}

#ifdef SNORT_RELOAD
static void SMTPReload(struct _SnortConfig *sc, char *args, void **new_config)
{
    tSfPolicyUserContextId smtp_swap_config = (tSfPolicyUserContextId)*new_config;
    SMTPToken *tmp;
    tSfPolicyId policy_id = _dpd.getParserPolicy(sc);
    SMTPConfig *pPolicyConfig = NULL;

    if (smtp_swap_config == NULL)
    {
        //create a context
        smtp_swap_config = sfPolicyConfigCreate();
        if (smtp_swap_config == NULL)
        {
            DynamicPreprocessorFatalMessage("Not enough memory to create SMTP "
                                            "configuration.\n");
        }
        *new_config = (void *)smtp_swap_config;
    }

    sfPolicyUserPolicySet (smtp_swap_config, policy_id);
    pPolicyConfig = (SMTPConfig *)sfPolicyUserDataGetCurrent(smtp_swap_config);

    if (pPolicyConfig != NULL)
        DynamicPreprocessorFatalMessage("Can only configure SMTP preprocessor once.\n");

    pPolicyConfig = (SMTPConfig *)_dpd.snortAlloc(1, sizeof(SMTPConfig), PP_SMTP, 
                                       PP_MEM_CATEGORY_CONFIG);

    if (pPolicyConfig == NULL)
    {
        DynamicPreprocessorFatalMessage("Not enough memory to create SMTP "
                                        "configuration.\n");
    }

    sfPolicyUserDataSetCurrent(smtp_swap_config, pPolicyConfig);

    SMTP_RegXtraDataFuncs(pPolicyConfig);
    SMTP_InitCmds(pPolicyConfig);
    SMTP_ParseArgs(pPolicyConfig, args);

    SMTP_CheckConfig(pPolicyConfig, smtp_swap_config);
    SMTP_PrintConfig(pPolicyConfig);

    if( pPolicyConfig->disabled )
        return;

    if (_dpd.streamAPI == NULL)
    {
        DynamicPreprocessorFatalMessage("Streaming & reassembly must be enabled "
                                        "for SMTP preprocessor\n");
    }

    /* Command search - do this here because it's based on configuration */
    pPolicyConfig->cmd_search_mpse = _dpd.searchAPI->search_instance_new();
    if (pPolicyConfig->cmd_search_mpse == NULL)
    {
        DynamicPreprocessorFatalMessage("Could not allocate SMTP "
                                        "command search.\n");
    }

    for (tmp = pPolicyConfig->cmds; tmp->name != NULL; tmp++)
    {
        pPolicyConfig->cmd_search[tmp->search_id].name = tmp->name;
        pPolicyConfig->cmd_search[tmp->search_id].name_len = tmp->name_len;

        _dpd.searchAPI->search_instance_add(pPolicyConfig->cmd_search_mpse, tmp->name,
                                            tmp->name_len, tmp->search_id);
    }

    _dpd.searchAPI->search_instance_prep(pPolicyConfig->cmd_search_mpse);

    _dpd.addPreproc(sc, SMTPDetect, PRIORITY_APPLICATION, PP_SMTP, PROTO_BIT__TCP);

    enablePortStreamServices(sc, pPolicyConfig, policy_id);

#ifdef TARGET_BASED
    _addServicesToStreamFilter(sc, policy_id);
#endif
}

static int SMTPMempoolFreeUsedBucket(MemPool *memory_pool)
{
    MemBucket *lru_bucket = NULL;

    lru_bucket = mempool_get_lru_bucket(memory_pool);
    if(lru_bucket)
    {
        /* Deleting least recently used SMTP session data here to adjust to new max_memory */
        _dpd.sessionAPI->set_application_data(lru_bucket->scbPtr, PP_SMTP, NULL, NULL);
        return 1;
    }
    return 0;
}

static unsigned SMTPReloadMimeMempoolAdjust(unsigned smtpMaxWork)
{
    int retVal;

    /* deleting MemBucket from free list in SMTP Mime Mempool */
    smtpMaxWork = mempool_prune_freelist(smtp_mime_mempool, smtp_mime_mempool->max_memory, smtpMaxWork);

    if(!smtpMaxWork)
       return 0;

    for( ; smtpMaxWork && ((smtp_mime_mempool->used_memory + smtp_mime_mempool->free_memory) > smtp_mime_mempool->max_memory); smtpMaxWork--)
    {
        /* deleting least recently used MemBucket from Used list in SMTP Mime Mempool */
        retVal = SMTPMempoolFreeUsedBucket(smtp_mime_mempool);
        if(!retVal)
           break;
    }

    return smtpMaxWork;
}

static unsigned SMTPReloadLogMempoolAdjust(unsigned smtpMaxWork)
{
    int retVal;

    /* deleting MemBucket from free list in SMTP Log Mempool */
    smtpMaxWork = mempool_prune_freelist(smtp_mempool, smtp_mempool->max_memory, smtpMaxWork);

    if(!smtpMaxWork)
       return 0;

    for( ; smtpMaxWork && ((smtp_mempool->used_memory + smtp_mempool->free_memory) > smtp_mempool->max_memory); smtpMaxWork--)
    {
        /* deleting least recently used MemBucket from Used list in SMTP Log Mempool */
        retVal = SMTPMempoolFreeUsedBucket(smtp_mempool);
        if(!retVal)
           break;
    }

    return smtpMaxWork;
}

static bool SMTPMimeReloadAdjust(bool idle, tSfPolicyId raPolicyId, void* userData)
{
    unsigned initialMaxWork = idle ? 512 : 5;
    unsigned maxWork;

    /* If new max_mime_mem is less than old configured max_mime_mem, need to adjust SMTP Mime Mempool.
     * In order to adjust to new max_memory of mime mempool, delete buckets from free list.
     * After deleting buckets from free list, still new max_memory is less than old value , delete buckets
     * (least recently used i.e head node of used list )from used list till total memory reaches to new max_memory.
     */
    maxWork = SMTPReloadMimeMempoolAdjust(initialMaxWork);

    if (maxWork == initialMaxWork)
    {
       smtp_stats.max_conc_sessions = smtp_stats.conc_sessions;
       smtp_stats.mime_stats.memcap_exceeded = 0;
       return true;
    }
    else
       return false;
}

static bool SMTPLogReloadAdjust(bool idle, tSfPolicyId raPolicyId, void* userData)
{
    unsigned initialMaxWork = idle ? 512 : 5;
    unsigned maxWork;

    /* If new memcap is less than old configured memcap, need to adjust SMTP Log Mempool.
     * In order to adjust to new max_memory of log mempool, delete buckets from free list.
     * After deleting buckets from free list, still new max_memory is less than old value , delete buckets
     * (least recently used i.e head node of used list )from used list till total memory reaches to new max_memory.
     */
    maxWork = SMTPReloadLogMempoolAdjust(initialMaxWork);

    if (maxWork == initialMaxWork)
    {
        smtp_stats.max_conc_sessions = smtp_stats.conc_sessions;
        smtp_stats.mime_stats.memcap_exceeded = 0;
        return true;
    }
    else
        return false;
}

static int SMTPReloadVerify(struct _SnortConfig *sc, void *swap_config)
{
    tSfPolicyUserContextId smtp_swap_config = (tSfPolicyUserContextId)swap_config;
    SMTPConfig *config = NULL;
    SMTPConfig *configNext = NULL;
    tSfPolicyId policy_id = 0;


    if (smtp_swap_config == NULL)
        return 0;

    if (smtp_config != NULL)
    {
        config = (SMTPConfig *)sfPolicyUserDataGet(smtp_config, _dpd.getDefaultPolicy());
    }

    configNext = (SMTPConfig *)sfPolicyUserDataGet(smtp_swap_config, _dpd.getDefaultPolicy());

    if (config == NULL)
    {
        return 0;
    }

    sfPolicyUserDataIterate (sc, smtp_swap_config, SMTPCheckPolicyConfig);
    sfPolicyUserDataIterate (sc, smtp_swap_config, CheckFilePolicyConfig);

    policy_id = _dpd.getParserPolicy(sc);

    if (smtp_mime_mempool != NULL)
    {
        /* If max_mime_mem changes, mime mempool need to be adjusted bcz mempool max_memory will be changed.
         * Registering here to adjust Mime memory Pool when max_mime_mem changes.
         */
        if( configNext->decode_conf.max_mime_mem  < config->decode_conf.max_mime_mem )
              _dpd.reloadAdjustRegister(sc, "SMTP-MIME-MEMPOOL", policy_id, &SMTPMimeReloadAdjust, NULL, NULL);

    }

    if (smtp_mempool != NULL)
    {
        if(configNext)
        {
             /* If memcap cahnges, log mempool need to be adjusted bcz mempool max_mempory will be changed.
              * Registering here to adjust Log memory Pool when memcap changes.
              */
             if (configNext->memcap < config->memcap)
                   _dpd.reloadAdjustRegister(sc, "SMTP-MEMPOOL", policy_id, &SMTPLogReloadAdjust, NULL, NULL);
        }
    }
    else if(configNext != NULL)
    {
        if (sfPolicyUserDataIterate(sc, smtp_swap_config, SMTPEnableDecoding) != 0)
            smtp_mime_mempool = (MemPool *)_dpd.fileAPI->init_mime_mempool(configNext->decode_conf.max_mime_mem,
                configNext->decode_conf.max_depth, smtp_mime_mempool, PROTOCOL_NAME);

        if (sfPolicyUserDataIterate(sc, smtp_swap_config, SMTPLogExtraData) != 0)
            smtp_mempool = (MemPool *)_dpd.fileAPI->init_log_mempool(configNext->log_config.email_hdrs_log_depth,
                configNext->memcap, smtp_mempool, PROTOCOL_NAME);

        if ( configNext->disabled )
            return 0;
    }

    return 0;
}

static int SMTPReloadSwapPolicy(
        tSfPolicyUserContextId config,
        tSfPolicyId policyId,
        void* pData
        )
{
    SMTPConfig *pPolicyConfig = (SMTPConfig *)pData;

    if (pPolicyConfig->ref_count == 0)
    {
        sfPolicyUserDataClear (config, policyId);
        SMTP_FreeConfig(pPolicyConfig);
    }

    return 0;
}

static void * SMTPReloadSwap(struct _SnortConfig *sc, void *swap_config)
{
    SMTPConfig *configNew = NULL, *configOld = NULL;
    tSfPolicyUserContextId smtp_swap_config = (tSfPolicyUserContextId)swap_config;
    tSfPolicyUserContextId old_config = smtp_config;

    if (smtp_swap_config == NULL)
        return NULL;

    smtp_config = smtp_swap_config;

    configOld = (SMTPConfig *)sfPolicyUserDataGet(old_config, _dpd.getDefaultPolicy());
    configNew = (SMTPConfig *)sfPolicyUserDataGet(smtp_config, _dpd.getDefaultPolicy());

    if(configNew)
    {
         if(smtp_mime_mempool)
         {
              if( (configOld->decode_conf.max_mime_mem != configNew->decode_conf.max_mime_mem) ||
                  (configOld->decode_conf.max_depth != configNew->decode_conf.max_depth) )
              {
#ifdef REG_TEST
                  _dpd.fileAPI->displayMimeMempool(smtp_mime_mempool,&(configOld->decode_conf), &(configNew->decode_conf));
#endif
                  /* Update the smtp_mime_mempool with new max_memmory and object size when max_mime_mem changes. */
                  _dpd.fileAPI->update_mime_mempool(smtp_mime_mempool, configNew->decode_conf.max_mime_mem, configNew->decode_conf.max_depth);
             }
         }
         if(smtp_mempool)
         {
             if( ( configOld->memcap != configNew->memcap) ||
                 ( configOld->log_config.email_hdrs_log_depth != configNew->log_config.email_hdrs_log_depth ) )
             {
#ifdef REG_TEST
                  if (REG_TEST_EMAIL_FLAG_LOG_MEMPOOL_ADJUST & getRegTestFlagsForEmail())
                  {
                        printf("[SMTP] Email Headers Log depth is: OLD VALUE # %u \n",configOld->log_config.email_hdrs_log_depth);
                        printf("[SMTP] Setting Email Headers Log depth to # (NEW VALUE) %u \n",configNew->log_config.email_hdrs_log_depth);
                        fflush(stdout);
                  }
                 _dpd.fileAPI->displayLogMempool(smtp_mempool, configOld->memcap, configNew->memcap);
#endif
                 /* Update the smtp_mempool with new max_memory and objest size when memcap changes. */
                 _dpd.fileAPI->update_log_mempool(smtp_mempool, configNew->memcap, configNew->log_config.email_hdrs_log_depth);
                 smtp_stats.log_memcap_exceeded = 0;
            }
         }

#ifdef REG_TEST
         _dpd.fileAPI->displayDecodeDepth(&(configOld->decode_conf), &(configNew->decode_conf));
#endif
    }

    sfPolicyUserDataFreeIterate (old_config, SMTPReloadSwapPolicy);

    if (sfPolicyUserPolicyGetActive(old_config) == 0)
        SMTP_FreeConfigs(old_config);

    return NULL;
}

static void SMTPReloadSwapFree(void *data)
{
    if (data == NULL)
        return;

    SMTP_FreeConfigs((tSfPolicyUserContextId)data);
}
#endif

