/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2011-2013 Sourcefire, Inc.
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
 * spp_imap.c
 *
 * Author: Bhagyashree Bantwal <bbantwal@cisco.com>
 *
 * Description:
 *
 * This file initializes IMAP as a Snort preprocessor.
 *
 * This file registers the IMAP initialization function,
 * adds the IMAP function into the preprocessor list.
 *
 * In general, this file is a wrapper to IMAP functionality,
 * by interfacing with the Snort preprocessor functions.  The rest
 * of IMAP should be separate from the preprocessor hooks.
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
#include "spp_imap.h"
#include "sf_preproc_info.h"
#include "snort_imap.h"
#include "imap_util.h"
#include "imap_config.h"
#include "imap_log.h"
#include "imap_paf.h"

#include "preprocids.h"
#include "sf_snort_packet.h"
#include "sf_dynamic_preprocessor.h"
#include "snort_debug.h"
#include "sfPolicy.h"
#include "sfPolicyUserData.h"

#include "profiler.h"
#ifdef PERF_PROFILING
PreprocStats imapPerfStats;
PreprocStats imapDetectPerfStats;
int imapDetectCalled = 0;
#endif

#include "sf_types.h"
#include "mempool.h"
#include "snort_bounds.h"

#include "file_api.h"
#ifdef REG_TEST
#include "reg_test.h"
#endif

#ifdef DUMP_BUFFER
#include "imap_buffer_dump.h"
#endif

const int MAJOR_VERSION = 1;
const int MINOR_VERSION = 0;
const int BUILD_VERSION = 1;
const char *PREPROC_NAME = "SF_IMAP";
const char *PROTOCOL_NAME = "IMAP";

#define SetupIMAP DYNAMIC_PREPROC_SETUP

MemPool *imap_mempool = NULL;
MemPool *imap_mime_mempool = NULL;

IMAP_Stats imap_stats;

tSfPolicyUserContextId imap_config = NULL;
IMAPConfig *imap_eval_config = NULL;

extern int16_t imap_proto_id;

static void IMAPInit(struct _SnortConfig *, char *);
static void IMAPDetect(void *, void *context);
static void IMAPCleanExitFunction(int, void *);
static void IMAPResetFunction(int, void *);
static void IMAPResetStatsFunction(int, void *);
static void registerPortsForDispatch( struct _SnortConfig *sc, IMAPConfig *policy );
static void registerPortsForReassembly( IMAPConfig *policy, int direction );
static void _addPortsToStreamFilter(struct _SnortConfig *, IMAPConfig *, tSfPolicyId);
static void IMAP_PrintStats(int);
#ifdef TARGET_BASED
static void _addServicesToStreamFilter(struct _SnortConfig *, tSfPolicyId);
#endif
static int IMAPCheckConfig(struct _SnortConfig *);

#ifdef SNORT_RELOAD
static int IMAPMempoolFreeUsedBucket(MemPool *memory_pool);
static unsigned IMAPReloadMimeMempoolAdjust(unsigned imapMaxWork);
static unsigned IMAPReloadLogMempoolAdjust(unsigned imapMaxWork);
static bool IMAPMimeReloadAdjust(bool idle, tSfPolicyId raPolicyId, void* userData);
static bool IMAPLogReloadAdjust(bool idle, tSfPolicyId raPolicyId, void* userData);
static void IMAPReload(struct _SnortConfig *, char *, void **);
static int IMAPReloadVerify(struct _SnortConfig *, void *);
static void * IMAPReloadSwap(struct _SnortConfig *, void *);
static void IMAPReloadSwapFree(void *);
#endif


/*
 * Function: SetupIMAP()
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
void SetupIMAP(void)
{
    /* link the preprocessor keyword to the init function in the preproc list */
#ifndef SNORT_RELOAD
    _dpd.registerPreproc("imap", IMAPInit);
#else
    _dpd.registerPreproc("imap", IMAPInit, IMAPReload,
                         IMAPReloadVerify, IMAPReloadSwap, IMAPReloadSwapFree);
#endif
#ifdef DUMP_BUFFER
    _dpd.registerBufferTracer(getIMAPBuffers, IMAP_BUFFER_DUMP_FUNC);
#endif

}

#ifdef REG_TEST
static inline void PrintIMAPSize(void)
{
    _dpd.logMsg("\nIMAP Session Size: %lu\n", (long unsigned int)sizeof(IMAP));
}
#endif

/*
 * Function: IMAPInit(char *)
 *
 * Purpose: Calls the argument parsing function, performs final setup on data
 *          structs, links the preproc function into the function list.
 *
 * Arguments: args => ptr to argument string
 *
 * Returns: void function
 *
 */
static void IMAPInit(struct _SnortConfig *sc, char *args)
{
    IMAPToken *tmp;
    tSfPolicyId policy_id = _dpd.getParserPolicy(sc);
    IMAPConfig * pPolicyConfig = NULL;

#ifdef REG_TEST
    PrintIMAPSize();
#endif

    _dpd.registerMemoryStatsFunc(PP_IMAP, IMAP_Print_Mem_Stats);

    if (imap_config == NULL)
    {
        //create a context
        imap_config = sfPolicyConfigCreate();
        if (imap_config == NULL)
        {
            DynamicPreprocessorFatalMessage("Not enough memory to create IMAP "
                                            "configuration.\n");
        }

        /* Initialize the searches not dependent on configuration.
         * headers, reponsed, data, mime boundary regular expression */
        IMAP_SearchInit();

        /* Put the preprocessor function into the function list */
        /* _dpd.addPreproc(IMAPDetect, PRIORITY_APPLICATION, PP_IMAP, PROTO_BIT__TCP);*/
        _dpd.addPreprocExit(IMAPCleanExitFunction, NULL, PRIORITY_LAST, PP_IMAP);
        _dpd.addPreprocReset(IMAPResetFunction, NULL, PRIORITY_LAST, PP_IMAP);
        _dpd.registerPreprocStats(IMAP_PROTO_REF_STR, IMAP_PrintStats);
        _dpd.addPreprocResetStats(IMAPResetStatsFunction, NULL, PRIORITY_LAST, PP_IMAP);
        _dpd.addPreprocConfCheck(sc, IMAPCheckConfig);

#ifdef TARGET_BASED
        imap_proto_id = _dpd.findProtocolReference(IMAP_PROTO_REF_STR);
        if (imap_proto_id == SFTARGET_UNKNOWN_PROTOCOL)
            imap_proto_id = _dpd.addProtocolReference(IMAP_PROTO_REF_STR);

        // register with session to handle applications
        _dpd.sessionAPI->register_service_handler( PP_IMAP, imap_proto_id );

        DEBUG_WRAP(DebugMessage(DEBUG_IMAP,"IMAP: Target-based: Proto id for %s: %u.\n",
                                IMAP_PROTO_REF_STR, imap_proto_id););
#endif

#ifdef PERF_PROFILING
        _dpd.addPreprocProfileFunc("imap", (void*)&imapPerfStats, 0, _dpd.totalPerfStats, NULL);
#endif
    }

    sfPolicyUserPolicySet (imap_config, policy_id);
    pPolicyConfig = (IMAPConfig *)sfPolicyUserDataGetCurrent(imap_config);
    if (pPolicyConfig != NULL)
    {
        DynamicPreprocessorFatalMessage("Can only configure IMAP preprocessor once.\n");
    }

    pPolicyConfig = (IMAPConfig *)_dpd.snortAlloc(1, sizeof(IMAPConfig), PP_IMAP, 
                                       PP_MEM_CATEGORY_CONFIG);
    if (pPolicyConfig == NULL)
    {
        DynamicPreprocessorFatalMessage("Not enough memory to create IMAP "
                                        "configuration.\n");
    }

    sfPolicyUserDataSetCurrent(imap_config, pPolicyConfig);

    IMAP_InitCmds(pPolicyConfig);
    IMAP_ParseArgs(pPolicyConfig, args);

    IMAP_CheckConfig(pPolicyConfig, imap_config);
    IMAP_PrintConfig(pPolicyConfig);

    if(pPolicyConfig->disabled)
        return;

    _dpd.addPreproc(sc, IMAPDetect, PRIORITY_APPLICATION, PP_IMAP, PROTO_BIT__TCP);

    if (_dpd.streamAPI == NULL)
    {
        DynamicPreprocessorFatalMessage("Streaming & reassembly must be enabled "
                "for IMAP preprocessor\n");
    }

    /* Command search - do this here because it's based on configuration */
    pPolicyConfig->cmd_search_mpse = _dpd.searchAPI->search_instance_new();
    if (pPolicyConfig->cmd_search_mpse == NULL)
    {
        DynamicPreprocessorFatalMessage("Could not allocate IMAP "
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

    // register ports with session and stream
    registerPortsForDispatch( sc, pPolicyConfig );
    registerPortsForReassembly( pPolicyConfig, SSN_DIR_FROM_SERVER | SSN_DIR_FROM_CLIENT );
     _addPortsToStreamFilter(sc, pPolicyConfig, policy_id);

#ifdef TARGET_BASED
    _addServicesToStreamFilter(sc, policy_id);
#endif
}

/*
 * Function: IMAPDetect(void *, void *)
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
static void IMAPDetect(void *pkt, void *context)
{
    SFSnortPacket *p = (SFSnortPacket *)pkt;
    tSfPolicyId policy_id = _dpd.getNapRuntimePolicy();
    PROFILE_VARS;

    // preconditions - what we registered for
    assert(IsTCP(p) && p->payload && p->payload_size);

    PREPROC_PROFILE_START(imapPerfStats);

    DEBUG_WRAP(DebugMessage(DEBUG_IMAP, "IMAP Start (((((((((((((((((((((((((((((((((((((((\n"););

    sfPolicyUserPolicySet (imap_config, policy_id);

    SnortIMAP(p);

    DEBUG_WRAP(DebugMessage(DEBUG_IMAP, "IMAP End )))))))))))))))))))))))))))))))))))))))))\n\n"););

    PREPROC_PROFILE_END(imapPerfStats);
#ifdef PERF_PROFILING
    if (PROFILING_PREPROCS && imapDetectCalled)
    {
        imapPerfStats.ticks -= imapDetectPerfStats.ticks;
        /* And Reset ticks to 0 */
        imapDetectPerfStats.ticks = 0;
        imapDetectCalled = 0;
    }
#endif

}


/*
 * Function: IMAPCleanExitFunction(int, void *)
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
static void IMAPCleanExitFunction(int signal, void *data)
{
    IMAP_Free();
    if (mempool_destroy(imap_mime_mempool) == 0)
    {
        free(imap_mime_mempool);
        imap_mime_mempool = NULL;
    }
    if (mempool_destroy(imap_mempool) == 0)
    {
        free(imap_mempool);
        imap_mempool = NULL;
    }

}


static void IMAPResetFunction(int signal, void *data)
{
    return;
}

static void IMAPResetStatsFunction(int signal, void *data)
{
    return;
}

static void registerPortsForDispatch( struct _SnortConfig *sc, IMAPConfig *policy )
{
    int port;

    for ( port = 0; port < MAXPORTS; port++ )
    {
        if( isPortEnabled( policy->ports, port ) )
            _dpd.sessionAPI->enable_preproc_for_port( sc, PP_IMAP, PROTO_BIT__TCP, port );
    }
}

static void registerPortsForReassembly( IMAPConfig *policy, int direction )
{
    uint32_t port;

    for ( port = 0; port < MAXPORTS; port++ )
    {
        if( isPortEnabled( policy->ports, port ) )
            _dpd.streamAPI->register_reassembly_port( NULL, port, direction );
    }
}

static void _addPortsToStreamFilter(struct _SnortConfig *sc, IMAPConfig *config, tSfPolicyId policy_id)
{
    unsigned int portNum;

    if (config == NULL)
        return;

    for (portNum = 0; portNum < MAXPORTS; portNum++)
    {
        if(config->ports[(portNum/8)] & (1<<(portNum%8)))
        {
            //Add port the port
            _dpd.streamAPI->set_port_filter_status(sc, IPPROTO_TCP, (uint16_t)portNum,
                                                   PORT_MONITOR_SESSION, policy_id, 1);
            register_imap_paf_port(sc, portNum, policy_id);
        }
    }
}

#ifdef TARGET_BASED
static void _addServicesToStreamFilter(struct _SnortConfig *sc, tSfPolicyId policy_id)
{
    _dpd.streamAPI->set_service_filter_status(sc, imap_proto_id, PORT_MONITOR_SESSION, policy_id, 1);
    register_imap_paf_service(sc, imap_proto_id, policy_id);
}
#endif

static int CheckFilePolicyConfig(
        struct _SnortConfig *sc,
        tSfPolicyUserContextId config,
        tSfPolicyId policyId,
        void* pData
        )
{
    IMAPConfig *context = (IMAPConfig *)pData;

     /* Use new Snort config to get the max file depth */
    context->decode_conf.file_depth = _dpd.fileAPI->get_max_file_depth(sc, true);
    if (context->decode_conf.file_depth > -1)
        context->log_config.log_filename = 1;
    updateMaxDepth(context->decode_conf.file_depth, &context->decode_conf.max_depth);

    return 0;
}

static int IMAPEnableDecoding(struct _SnortConfig *sc, tSfPolicyUserContextId config,
            tSfPolicyId policyId, void *pData)
{
    IMAPConfig *context = (IMAPConfig *)pData;

    if (pData == NULL)
        return 0;

    if(context->disabled)
        return 0;

    if(_dpd.fileAPI->is_decoding_enabled(&(context->decode_conf)))
        return 1;

    return 0;
}

static int IMAPCheckPolicyConfig(
        struct _SnortConfig *sc,
        tSfPolicyUserContextId config,
        tSfPolicyId policyId,
        void* pData
        )
{
    IMAPConfig *context = (IMAPConfig *)pData;

    _dpd.setParserPolicy(sc, policyId);

    /* In a multiple-policy setting, the IMAP preproc can be turned on in a
       "disabled" state. In this case, we don't require Stream. */
    if (context->disabled)
        return 0;

    if (_dpd.streamAPI == NULL)
     {
        _dpd.errMsg("Streaming & reassembly must be enabled for IMAP preprocessor\n");
        return -1;
    }

    return 0;
}

static int IMAPLogExtraData(struct _SnortConfig *sc, tSfPolicyUserContextId config,
        tSfPolicyId policyId, void *pData)
{
    IMAPConfig *context = (IMAPConfig *)pData;

    if (pData == NULL)
        return 0;

    if(context->disabled)
        return 0;

    if(context->log_config.log_filename)
        return 1;

    return 0;
}

static int IMAPCheckConfig(struct _SnortConfig *sc)
{
    int rval;

    IMAPConfig *defaultConfig =
            (IMAPConfig *)sfPolicyUserDataGetDefault(imap_config);

    if ((rval = sfPolicyUserDataIterate (sc, imap_config, IMAPCheckPolicyConfig)))
        return rval;

    if ((rval = sfPolicyUserDataIterate (sc, imap_config, CheckFilePolicyConfig)))
        return rval;

    if (sfPolicyUserDataIterate(sc, imap_config, IMAPEnableDecoding) != 0)
    {
        if (defaultConfig == NULL)
        {
            /*error message */
            _dpd.errMsg("IMAP: Must configure a default "
                    "configuration if you want to imap decoding.\n");
            return -1;
        }

        imap_mime_mempool = (MemPool *)_dpd.fileAPI->init_mime_mempool(defaultConfig->decode_conf.max_mime_mem,
                       defaultConfig->decode_conf.max_depth, imap_mime_mempool, PROTOCOL_NAME);

    }

    if (sfPolicyUserDataIterate(sc, imap_config, IMAPLogExtraData) != 0)
    {
        if (defaultConfig == NULL)
        {
            /*error message */
            _dpd.errMsg("IMAP: Must configure a default "
                    "configuration if you want to log extra data.\n");
            return -1;
        }
        imap_mempool = (MemPool *)_dpd.fileAPI->init_log_mempool(0,
                defaultConfig->memcap, imap_mempool, PROTOCOL_NAME);
    }
    return 0;
}

static void IMAP_PrintStats(int exiting)
{
    _dpd.logMsg("IMAP Preprocessor Statistics\n");
    _dpd.logMsg("  Total sessions                                    : " STDu64 "\n", imap_stats.sessions);
    _dpd.logMsg("  Max concurrent sessions                           : " STDu64 "\n", imap_stats.max_conc_sessions);
    if (imap_stats.sessions > 0)
    {
        _dpd.logMsg("  Base64 attachments decoded                        : " STDu64 "\n", imap_stats.mime_stats.attachments[DECODE_B64]);
        _dpd.logMsg("  Total Base64 decoded bytes                        : " STDu64 "\n", imap_stats.mime_stats.decoded_bytes[DECODE_B64]);
        _dpd.logMsg("  Quoted-Printable attachments decoded              : " STDu64 "\n", imap_stats.mime_stats.attachments[DECODE_QP]);
        _dpd.logMsg("  Total Quoted decoded bytes                        : " STDu64 "\n", imap_stats.mime_stats.decoded_bytes[DECODE_QP]);
        _dpd.logMsg("  UU attachments decoded                            : " STDu64 "\n", imap_stats.mime_stats.attachments[DECODE_UU]);
        _dpd.logMsg("  Total UU decoded bytes                            : " STDu64 "\n", imap_stats.mime_stats.decoded_bytes[DECODE_UU]);
        _dpd.logMsg("  Non-Encoded MIME attachments extracted            : " STDu64 "\n", imap_stats.mime_stats.attachments[DECODE_BITENC]);
        _dpd.logMsg("  Total Non-Encoded MIME bytes extracted            : " STDu64 "\n", imap_stats.mime_stats.decoded_bytes[DECODE_BITENC]);
        if ( imap_stats.mime_stats.memcap_exceeded )
            _dpd.logMsg("  Sessions not decoded due to memory unavailability : " STDu64 "\n", imap_stats.mime_stats.memcap_exceeded);
        if ( imap_stats.log_memcap_exceeded )
            _dpd.logMsg("  IMAP sessions fastpathed due to memcap exceeded: " STDu64 "\n", imap_stats.log_memcap_exceeded);
    }

}

#ifdef SNORT_RELOAD
static void IMAPReload(struct _SnortConfig *sc, char *args, void **new_config)
{
    tSfPolicyUserContextId imap_swap_config = (tSfPolicyUserContextId)*new_config;
    IMAPToken *tmp;
    tSfPolicyId policy_id = _dpd.getParserPolicy(sc);
    IMAPConfig *pPolicyConfig = NULL;

    if (imap_swap_config == NULL)
    {
        //create a context
        imap_swap_config = sfPolicyConfigCreate();
        if (imap_swap_config == NULL)
        {
            DynamicPreprocessorFatalMessage("Not enough memory to create IMAP "
                                            "configuration.\n");
        }
        *new_config = (void *)imap_swap_config;
    }

    sfPolicyUserPolicySet (imap_swap_config, policy_id);
    pPolicyConfig = (IMAPConfig *)sfPolicyUserDataGetCurrent(imap_swap_config);

    if (pPolicyConfig != NULL)
        DynamicPreprocessorFatalMessage("Can only configure IMAP preprocessor once.\n");

    pPolicyConfig = (IMAPConfig *)_dpd.snortAlloc(1, sizeof(IMAPConfig), PP_IMAP, 
                                       PP_MEM_CATEGORY_CONFIG);
    if (pPolicyConfig == NULL)
    {
        DynamicPreprocessorFatalMessage("Not enough memory to create IMAP "
                                        "configuration.\n");
    }

    sfPolicyUserDataSetCurrent(imap_swap_config, pPolicyConfig);

    IMAP_InitCmds(pPolicyConfig);
    IMAP_ParseArgs(pPolicyConfig, args);

    IMAP_CheckConfig(pPolicyConfig, imap_swap_config);
    IMAP_PrintConfig(pPolicyConfig);

    if( pPolicyConfig->disabled )
        return;

    if (_dpd.streamAPI == NULL)
    {
        DynamicPreprocessorFatalMessage("Streaming & reassembly must be enabled "
                                        "for IMAP preprocessor\n");
    }

    /* Command search - do this here because it's based on configuration */
    pPolicyConfig->cmd_search_mpse = _dpd.searchAPI->search_instance_new();
    if (pPolicyConfig->cmd_search_mpse == NULL)
    {
        DynamicPreprocessorFatalMessage("Could not allocate IMAP "
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

    _dpd.addPreproc(sc, IMAPDetect, PRIORITY_APPLICATION, PP_IMAP, PROTO_BIT__TCP);

    // register ports with session and stream
    registerPortsForDispatch( sc, pPolicyConfig );
    registerPortsForReassembly( pPolicyConfig, SSN_DIR_FROM_SERVER | SSN_DIR_FROM_CLIENT );
    _addPortsToStreamFilter(sc, pPolicyConfig, policy_id);

#ifdef TARGET_BASED
    _addServicesToStreamFilter(sc, policy_id);
#endif
}

static int IMAPMempoolFreeUsedBucket(MemPool *memory_pool)
{
   MemBucket *lru_bucket = NULL;

   lru_bucket = mempool_get_lru_bucket(memory_pool);
   if(lru_bucket)
   {
       /* Deleting least recently used IMAP session data here to adjust to new max_memory */
       _dpd.sessionAPI->set_application_data(lru_bucket->scbPtr, PP_IMAP, NULL, NULL);
       return 1;
   }
   return 0;
}

static unsigned IMAPReloadMimeMempoolAdjust(unsigned imapMaxWork)
{
     int retVal;

     /* deleting MemBucket from free list in IMAP Mime Mempool */
     imapMaxWork = mempool_prune_freelist(imap_mime_mempool, imap_mime_mempool->max_memory, imapMaxWork);

     if(!imapMaxWork)
        return 0;

     for( ; imapMaxWork && ((imap_mime_mempool->used_memory + imap_mime_mempool->free_memory) > imap_mime_mempool->max_memory); imapMaxWork--)
     {
         /* deleting least recently used MemBucket from Used list in IMAP Mime Mempool */
         retVal = IMAPMempoolFreeUsedBucket(imap_mime_mempool);
         if(!retVal)
            break;
     }

     return imapMaxWork;
}

static unsigned IMAPReloadLogMempoolAdjust(unsigned imapMaxWork)
{
     int retVal;

     /* deleting MemBucket from free list in IMAP Log Mempool */
     imapMaxWork = mempool_prune_freelist(imap_mempool, imap_mempool->max_memory, imapMaxWork);

     if(!imapMaxWork)
        return 0;

     for( ; imapMaxWork && ((imap_mempool->used_memory + imap_mempool->free_memory) > imap_mempool->max_memory); imapMaxWork--)
     {
         /* deleting least recently used MemBucket from Used list in IMAP Log Mempool */
         retVal = IMAPMempoolFreeUsedBucket(imap_mempool);
         if(!retVal)
          break;
     }

     return imapMaxWork;
}

static bool IMAPMimeReloadAdjust(bool idle, tSfPolicyId raPolicyId, void* userData)
{
    unsigned initialMaxWork = idle ? 512 : 5;
    unsigned maxWork;

    /* If new max_mime_mem is less than old configured max_mime_mem, need to adjust IMAP Mime Mempool.
     * In order to adjust to new max_memory of mime mempool, delete buckets from free list.
     * After deleting buckets from free list, still new max_memory is less than old value , delete buckets
     * (least recently used i.e head node of used list )from used list till total memory reaches to new max_memory.
     */
    maxWork = IMAPReloadMimeMempoolAdjust(initialMaxWork);

    if(maxWork == initialMaxWork)
    {
        imap_stats.max_conc_sessions = imap_stats.conc_sessions;
        imap_stats.mime_stats.memcap_exceeded = 0;
        return true;
    }
    else
        return false;
}

static bool IMAPLogReloadAdjust(bool idle, tSfPolicyId raPolicyId, void* userData)
{
    unsigned initialMaxWork = idle ? 512 : 5;
    unsigned maxWork;

    /* If new memcap is less than old configured memcap, need to adjust IMAP Log Mempool.
     * In order to adjust to new max_memory of log mempool, delete buckets from free list.
     * After deleting buckets from free list, still new max_memory is less than old value , delete buckets
     * (least recently used i.e head node of used list )from used list till total memory reaches to new max_memory.
     */
    maxWork = IMAPReloadLogMempoolAdjust(initialMaxWork);

    if (maxWork == initialMaxWork)
    {
        imap_stats.max_conc_sessions = imap_stats.conc_sessions;
        imap_stats.mime_stats.memcap_exceeded = 0;
        return true;
    }
    else
        return false;
}

static int IMAPReloadVerify(struct _SnortConfig *sc, void *swap_config)
{
    tSfPolicyUserContextId imap_swap_config = (tSfPolicyUserContextId)swap_config;
    IMAPConfig *config = NULL;
    IMAPConfig *configNext = NULL;
    tSfPolicyId policy_id = 0;
    int rval;

    if (imap_swap_config == NULL)
        return 0;

    if (imap_config != NULL)
        config = (IMAPConfig *)sfPolicyUserDataGet(imap_config, _dpd.getDefaultPolicy());

    configNext = (IMAPConfig *)sfPolicyUserDataGet(imap_swap_config, _dpd.getDefaultPolicy());

    if (config == NULL)
        return 0;

    if ((rval = sfPolicyUserDataIterate( sc, imap_swap_config, IMAPCheckPolicyConfig )))
        return rval;

    if ((rval = sfPolicyUserDataIterate( sc, imap_swap_config, CheckFilePolicyConfig )))
        return rval;

    policy_id = _dpd.getParserPolicy(sc);

    if (imap_mime_mempool != NULL)
    {
        /* If max_mime_mem changes, mime mempool need to be adjusted bcz mempool max_memory will be changed.
           Registering here to adjust Mime memory Pool when max_mime_mem changes. */
        if( configNext->decode_conf.max_mime_mem  < config->decode_conf.max_mime_mem )
              _dpd.reloadAdjustRegister(sc, "IMAP-MIME-MEMPOOL", policy_id, &IMAPMimeReloadAdjust, NULL, NULL);
    }

    if (imap_mempool != NULL)
    {
        if(configNext)
        {
             /* If memcap cahnges, log mempool need to be adjusted bcz mempool max_mempory will be changed.
                Registering here to adjust Log memory Pool when memcap changes. */
             if (configNext->memcap < config->memcap)
                   _dpd.reloadAdjustRegister(sc, "IMAP-LOG-MEMPOOL", policy_id, &IMAPLogReloadAdjust, NULL, NULL);
        }

    }
    else if(configNext != NULL)
    {
        if (sfPolicyUserDataIterate(sc, imap_swap_config, IMAPEnableDecoding) != 0)
        {
            imap_mime_mempool = (MemPool *)_dpd.fileAPI->init_mime_mempool(configNext->decode_conf.max_mime_mem,
                    configNext->decode_conf.max_depth, imap_mime_mempool, PROTOCOL_NAME);
        }

        if (sfPolicyUserDataIterate(sc, imap_swap_config, IMAPLogExtraData) != 0)
        {
            imap_mempool = (MemPool *)_dpd.fileAPI->init_log_mempool(0,  configNext->memcap,
                    imap_mempool, PROTOCOL_NAME);
        }

        if ( configNext->disabled )
            return 0;
    }

    if (_dpd.streamAPI == NULL)
    {
        _dpd.errMsg("Streaming & reassembly must be enabled for IMAP preprocessor\n");
        return -1;
    }

    return 0;
}

static int IMAPReloadSwapPolicy(
        tSfPolicyUserContextId config,
        tSfPolicyId policyId,
        void* pData
        )
{
    IMAPConfig *pPolicyConfig = (IMAPConfig *)pData;

    if (pPolicyConfig->ref_count == 0)
    {
        sfPolicyUserDataClear (config, policyId);
        IMAP_FreeConfig(pPolicyConfig);
    }

    return 0;
}

static void * IMAPReloadSwap(struct _SnortConfig *sc, void *swap_config)
{
    IMAPConfig *configNew = NULL, *configOld = NULL;
    tSfPolicyUserContextId imap_swap_config = (tSfPolicyUserContextId)swap_config;
    tSfPolicyUserContextId old_config = imap_config;

    if (imap_swap_config == NULL)
        return NULL;

    imap_config = imap_swap_config;

    configOld = (IMAPConfig *)sfPolicyUserDataGet(old_config, _dpd.getDefaultPolicy());
    configNew = (IMAPConfig *)sfPolicyUserDataGet(imap_config, _dpd.getDefaultPolicy());

    if(configNew)
    {
          if(imap_mime_mempool)
          {
              if((configOld->decode_conf.max_mime_mem != configNew->decode_conf.max_mime_mem) ||
                 (configOld->decode_conf.max_depth != configNew->decode_conf.max_depth) )
              {
#ifdef REG_TEST
                   _dpd.fileAPI->displayMimeMempool(imap_mime_mempool,&(configOld->decode_conf), &(configNew->decode_conf));
#endif
                   /* Update the imap_mime_mempool with new max_memmory and object size when max_mime_mem changes. */
                   _dpd.fileAPI->update_mime_mempool(imap_mime_mempool, configNew->decode_conf.max_mime_mem, configNew->decode_conf.max_depth);
              }
          }
          if(imap_mempool)
          {
              if(configOld->memcap != configNew->memcap)
              {
#ifdef REG_TEST
                  _dpd.fileAPI->displayLogMempool(imap_mempool, configOld->memcap, configNew->memcap);
#endif
                  /* Update the imap_mempool with new max_memory and objest size when memcap changes. */
                  _dpd.fileAPI->update_log_mempool(imap_mempool, configNew->memcap, 0);
                  imap_stats.log_memcap_exceeded = 0;
              }
          }
#ifdef REG_TEST
          _dpd.fileAPI->displayDecodeDepth(&(configOld->decode_conf), &(configNew->decode_conf));
#endif

    }

    sfPolicyUserDataFreeIterate (old_config, IMAPReloadSwapPolicy);

    if (sfPolicyUserPolicyGetActive(old_config) == 0)
        IMAP_FreeConfigs(old_config);

    return NULL;
}

static void IMAPReloadSwapFree(void *data)
{
    if (data == NULL)
        return;

    IMAP_FreeConfigs((tSfPolicyUserContextId)data);
}
#endif
