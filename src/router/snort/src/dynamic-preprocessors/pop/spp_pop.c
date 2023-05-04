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
 * spp_pop.c
 *
 * Author: Bhagyashree Bantwal <bbantwal@cisco.com>
 *
 * Description:
 *
 * This file initializes POP as a Snort preprocessor.
 *
 * This file registers the POP initialization function,
 * adds the POP function into the preprocessor list.
 *
 * In general, this file is a wrapper to POP functionality,
 * by interfacing with the Snort preprocessor functions.  The rest
 * of POP should be separate from the preprocessor hooks.
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
#include "spp_pop.h"
#include "sf_preproc_info.h"
#include "snort_pop.h"
#include "pop_config.h"
#include "pop_log.h"
#include "pop_paf.h"
#include "pop_util.h"

#include "preprocids.h"
#include "sf_snort_packet.h"
#include "sf_dynamic_preprocessor.h"
#include "snort_debug.h"
#include "sfPolicy.h"
#include "sfPolicyUserData.h"

#include "profiler.h"
#ifdef PERF_PROFILING
PreprocStats popPerfStats;
PreprocStats popDetectPerfStats;
int popDetectCalled = 0;
#endif

#include "sf_types.h"
#include "mempool.h"
#include "snort_bounds.h"

#include "file_api.h"
#ifdef REG_TEST
#include "reg_test.h"
#endif

#ifdef DUMP_BUFFER
#include "pop_buffer_dump.h"
#endif

const int MAJOR_VERSION = 1;
const int MINOR_VERSION = 0;
const int BUILD_VERSION = 1;
const char *PREPROC_NAME = "SF_POP";
const char *PROTOCOL_NAME = "POP";

#define SetupPOP DYNAMIC_PREPROC_SETUP

MemPool *pop_mime_mempool = NULL;
MemPool *pop_mempool = NULL;
POP_Stats pop_stats;

tSfPolicyUserContextId pop_config = NULL;
POPConfig *pop_eval_config = NULL;

extern POP pop_no_session;
extern int16_t pop_proto_id;

static void POPInit(struct _SnortConfig *, char *);
static void POPDetect(void *, void *context);
static void POPCleanExitFunction(int, void *);
static void POPResetFunction(int, void *);
static void POPResetStatsFunction(int, void *);
static void registerPortsForDispatch( struct _SnortConfig *sc, POPConfig *policy );
static void registerPortsForReassembly( POPConfig *policy, int direction );
static void _addPortsToStreamFilter(struct _SnortConfig *, POPConfig *, tSfPolicyId);
static void POP_PrintStats(int);
#ifdef TARGET_BASED
static void _addServicesToStreamFilter(struct _SnortConfig *, tSfPolicyId);
#endif
static int POPCheckConfig(struct _SnortConfig *);

#ifdef SNORT_RELOAD
static int POPMempoolFreeUsedBucket(MemPool *memory_pool);
static unsigned POPReloadMimeMempoolAdjust(unsigned popMaxWork);
static unsigned POPReloadLogMempoolAdjust(unsigned popMaxWork);
static bool POPMimeReloadAdjust(bool idle, tSfPolicyId raPolicyId, void* userData);
static bool POPLogReloadAdjust(bool idle, tSfPolicyId raPolicyId, void* userData);
static void POPReload(struct _SnortConfig *, char *, void **);
static int POPReloadVerify(struct _SnortConfig *, void *);
static void * POPReloadSwap(struct _SnortConfig *, void *);
static void POPReloadSwapFree(void *);
#endif


/*
 * Function: SetupPOP()
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
void SetupPOP(void)
{
    /* link the preprocessor keyword to the init function in the preproc list */
#ifndef SNORT_RELOAD
    _dpd.registerPreproc("pop", POPInit);
#else
    _dpd.registerPreproc("pop", POPInit, POPReload,
                         POPReloadVerify, POPReloadSwap, POPReloadSwapFree);
#endif
#ifdef DUMP_BUFFER
    _dpd.registerBufferTracer(getPOPBuffers, POP_BUFFER_DUMP_FUNC);
#endif


}

#ifdef REG_TEST
static inline void PrintPOPSize(void)
{
    _dpd.logMsg("\nPOP Session Size: %lu\n", (long unsigned int)sizeof(POP));
}
#endif

/*
 * Function: POPInit(char *)
 *
 * Purpose: Calls the argument parsing function, performs final setup on data
 *          structs, links the preproc function into the function list.
 *
 * Arguments: args => ptr to argument string
 *
 * Returns: void function
 *
 */
static void POPInit(struct _SnortConfig *sc, char *args)
{
    POPToken *tmp;
    tSfPolicyId policy_id = _dpd.getParserPolicy(sc);
    POPConfig * pPolicyConfig = NULL;

#ifdef REG_TEST
    PrintPOPSize();
#endif

    _dpd.registerMemoryStatsFunc(PP_POP, POP_Print_Mem_Stats);

    if (pop_config == NULL)
    {
        //create a context
        pop_config = sfPolicyConfigCreate();
        if (pop_config == NULL)
        {
            DynamicPreprocessorFatalMessage("Not enough memory to create POP "
                                            "configuration.\n");
        }

        /* Initialize the searches not dependent on configuration.
         * headers, reponsed, data, mime boundary regular expression */
        POP_SearchInit();

        /* zero out static POP global used for stateless POP or if there
         * is no session pointer */
        memset(&pop_no_session, 0, sizeof(POP));

        /* Put the preprocessor function into the function list */
        /* _dpd.addPreproc(POPDetect, PRIORITY_APPLICATION, PP_POP, PROTO_BIT__TCP);*/
        _dpd.addPreprocExit(POPCleanExitFunction, NULL, PRIORITY_LAST, PP_POP);
        _dpd.addPreprocReset(POPResetFunction, NULL, PRIORITY_LAST, PP_POP);
        _dpd.registerPreprocStats(POP_PROTO_REF_STR, POP_PrintStats);
        _dpd.addPreprocResetStats(POPResetStatsFunction, NULL, PRIORITY_LAST, PP_POP);
        _dpd.addPreprocConfCheck(sc, POPCheckConfig);

#ifdef TARGET_BASED
        pop_proto_id = _dpd.findProtocolReference(POP_PROTO_REF_STR);
        if (pop_proto_id == SFTARGET_UNKNOWN_PROTOCOL)
            pop_proto_id = _dpd.addProtocolReference(POP_PROTO_REF_STR);

        // register with session to handle applications
        _dpd.sessionAPI->register_service_handler( PP_POP, pop_proto_id );

        DEBUG_WRAP(DebugMessage(DEBUG_POP,"POP: Target-based: Proto id for %s: %u.\n",
                                POP_PROTO_REF_STR, pop_proto_id););
#endif

#ifdef PERF_PROFILING
        _dpd.addPreprocProfileFunc("pop", (void*)&popPerfStats, 0, _dpd.totalPerfStats, NULL);
#endif
    }

    sfPolicyUserPolicySet (pop_config, policy_id);
    pPolicyConfig = (POPConfig *)sfPolicyUserDataGetCurrent(pop_config);
    if (pPolicyConfig != NULL)
    {
        DynamicPreprocessorFatalMessage("Can only configure POP preprocessor once.\n");
    }

    pPolicyConfig = (POPConfig *)_dpd.snortAlloc(1, sizeof(POPConfig), PP_POP, 
                                      PP_MEM_CATEGORY_CONFIG);
    if (pPolicyConfig == NULL)
    {
        DynamicPreprocessorFatalMessage("Not enough memory to create POP "
                                        "configuration.\n");
    }

    sfPolicyUserDataSetCurrent(pop_config, pPolicyConfig);

    POP_InitCmds(pPolicyConfig);
    POP_ParseArgs(pPolicyConfig, args);

    POP_CheckConfig(pPolicyConfig, pop_config);
    POP_PrintConfig(pPolicyConfig);

    if(pPolicyConfig->disabled)
        return;

    _dpd.addPreproc(sc, POPDetect, PRIORITY_APPLICATION, PP_POP, PROTO_BIT__TCP);

    if (_dpd.streamAPI == NULL)
    {
        DynamicPreprocessorFatalMessage("Streaming & reassembly must be enabled "
                "for POP preprocessor\n");
    }

    /* Command search - do this here because it's based on configuration */
    pPolicyConfig->cmd_search_mpse = _dpd.searchAPI->search_instance_new();
    if (pPolicyConfig->cmd_search_mpse == NULL)
    {
        DynamicPreprocessorFatalMessage("Could not allocate POP "
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
 * Function: POPDetect(void *, void *)
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
static void POPDetect(void *pkt, void *context)
{
    SFSnortPacket *p = (SFSnortPacket *)pkt;
    tSfPolicyId policy_id = _dpd.getNapRuntimePolicy();
    PROFILE_VARS;

    // preconditions - what we registered for
    assert(IsTCP(p) && p->payload && p->payload_size);

    PREPROC_PROFILE_START(popPerfStats);

    DEBUG_WRAP(DebugMessage(DEBUG_POP, "POP Start (((((((((((((((((((((((((((((((((((((((\n"););

    sfPolicyUserPolicySet (pop_config, policy_id);

    SnortPOP(p);

    DEBUG_WRAP(DebugMessage(DEBUG_POP, "POP End )))))))))))))))))))))))))))))))))))))))))\n\n"););

    PREPROC_PROFILE_END(popPerfStats);
#ifdef PERF_PROFILING
    if (PROFILING_PREPROCS && popDetectCalled)
    {
        popPerfStats.ticks -= popDetectPerfStats.ticks;
        /* And Reset ticks to 0 */
        popDetectPerfStats.ticks = 0;
        popDetectCalled = 0;
    }
#endif

}


/*
 * Function: POPCleanExitFunction(int, void *)
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
static void POPCleanExitFunction(int signal, void *data)
{
    POP_Free();
    if (mempool_destroy(pop_mime_mempool) == 0)
    {
        free(pop_mime_mempool);
        pop_mime_mempool = NULL;
    }
    if (mempool_destroy(pop_mempool) == 0)
    {
        free(pop_mempool);
        pop_mempool = NULL;
    }

}


static void POPResetFunction(int signal, void *data)
{
    return;
}

static void POPResetStatsFunction(int signal, void *data)
{
    return;
}

static void registerPortsForDispatch( struct _SnortConfig *sc, POPConfig *policy )
{
    int port;

    for ( port = 0; port < MAXPORTS; port++ )
    {
        if( isPortEnabled( policy->ports, port ) )
            _dpd.sessionAPI->enable_preproc_for_port( sc, PP_POP, PROTO_BIT__TCP, port ); 
    }
}

static void registerPortsForReassembly( POPConfig *policy, int direction )
{
    uint32_t port;

    for ( port = 0; port < MAXPORTS; port++ )
    {
        if( isPortEnabled( policy->ports, port ) )
            _dpd.streamAPI->register_reassembly_port( NULL, port, direction );
    }
}

static void _addPortsToStreamFilter(struct _SnortConfig *sc, POPConfig *config, tSfPolicyId policy_id)
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
            register_pop_paf_port(sc, portNum, policy_id);
        }
    }
}

#ifdef TARGET_BASED
static void _addServicesToStreamFilter(struct _SnortConfig *sc, tSfPolicyId policy_id)
{
    _dpd.streamAPI->set_service_filter_status(sc, pop_proto_id, PORT_MONITOR_SESSION, policy_id, 1);
     register_pop_paf_service(sc, pop_proto_id, policy_id);
}
#endif

static int CheckFilePolicyConfig(
        struct _SnortConfig *sc,
        tSfPolicyUserContextId config,
        tSfPolicyId policyId,
        void* pData
        )
{
    POPConfig *context = (POPConfig *)pData;

     /* Use new Snort config to get the max file depth */
    context->decode_conf.file_depth = _dpd.fileAPI->get_max_file_depth(sc, true);
    if (context->decode_conf.file_depth > -1)
        context->log_config.log_filename = 1;
    updateMaxDepth(context->decode_conf.file_depth, &context->decode_conf.max_depth);

    return 0;
}

static int POPEnableDecoding(struct _SnortConfig *sc, tSfPolicyUserContextId config,
            tSfPolicyId policyId, void *pData)
{
    POPConfig *context = (POPConfig *)pData;

    if (pData == NULL)
        return 0;

    if(context->disabled)
        return 0;

    if(_dpd.fileAPI->is_decoding_enabled(&(context->decode_conf)))
        return 1;

    return 0;
}

static int POPLogExtraData(struct _SnortConfig *sc, tSfPolicyUserContextId config,
        tSfPolicyId policyId, void *pData)
{
    POPConfig *context = (POPConfig *)pData;

    if (pData == NULL)
        return 0;

    if(context->disabled)
        return 0;

    if(context->log_config.log_filename)
        return 1;

    return 0;
}

static int POPCheckPolicyConfig(
        struct _SnortConfig *sc,
        tSfPolicyUserContextId config,
        tSfPolicyId policyId,
        void* pData
        )
{
    POPConfig *context = (POPConfig *)pData;

    _dpd.setParserPolicy(sc, policyId);

    /* In a multiple-policy setting, the POP preproc can be turned on in a
       "disabled" state. In this case, we don't require Stream. */
    if (context->disabled)
        return 0;

    if (_dpd.streamAPI == NULL)
    {
        _dpd.errMsg("Streaming & reassembly must be enabled for POP preprocessor\n");
        return -1;
    }

    return 0;
}

static int POPCheckConfig(struct _SnortConfig *sc)
{
    int rval;
    POPConfig *defaultConfig =
            (POPConfig *)sfPolicyUserDataGetDefault(pop_config);

    if ((rval = sfPolicyUserDataIterate (sc, pop_config, POPCheckPolicyConfig)))
        return rval;

    if ((rval = sfPolicyUserDataIterate (sc, pop_config, CheckFilePolicyConfig)))
        return rval;

    if (sfPolicyUserDataIterate(sc, pop_config, POPEnableDecoding) != 0)
    {
        if (defaultConfig == NULL)
        {
            /*error message */
            _dpd.errMsg("POP: Must configure a default "
                    "configuration if you want to pop decoding.\n");
            return -1;
        }

        pop_mime_mempool = (MemPool *)_dpd.fileAPI->init_mime_mempool(defaultConfig->decode_conf.max_mime_mem,
                defaultConfig->decode_conf.max_depth, pop_mime_mempool, PROTOCOL_NAME);
    }

    if (sfPolicyUserDataIterate(sc, pop_config, POPLogExtraData) != 0)
    {
        pop_mempool = (MemPool *)_dpd.fileAPI->init_log_mempool(0,  defaultConfig->memcap, pop_mempool, PROTOCOL_NAME);
    }
    return 0;
}

static void POP_PrintStats(int exiting)
{
     _dpd.logMsg("POP Preprocessor Statistics\n");
     _dpd.logMsg("  Total sessions                                    : " STDu64 "\n", pop_stats.sessions);
     _dpd.logMsg("  Max concurrent sessions                           : " STDu64 "\n", pop_stats.max_conc_sessions);
     if (pop_stats.sessions > 0)
     {
          _dpd.logMsg("  Base64 attachments decoded                        : " STDu64 "\n", pop_stats.mime_stats.attachments[DECODE_B64]);
          _dpd.logMsg("  Total Base64 decoded bytes                        : " STDu64 "\n", pop_stats.mime_stats.decoded_bytes[DECODE_B64]);
          _dpd.logMsg("  Quoted-Printable attachments decoded              : " STDu64 "\n", pop_stats.mime_stats.attachments[DECODE_QP]);
          _dpd.logMsg("  Total Quoted decoded bytes                        : " STDu64 "\n", pop_stats.mime_stats.decoded_bytes[DECODE_QP]);
          _dpd.logMsg("  UU attachments decoded                            : " STDu64 "\n", pop_stats.mime_stats.attachments[DECODE_UU]);
          _dpd.logMsg("  Total UU decoded bytes                            : " STDu64 "\n", pop_stats.mime_stats.decoded_bytes[DECODE_UU]);
          _dpd.logMsg("  Non-Encoded MIME attachments extracted            : " STDu64 "\n", pop_stats.mime_stats.attachments[DECODE_BITENC]);
          _dpd.logMsg("  Total Non-Encoded MIME bytes extracted            : " STDu64 "\n", pop_stats.mime_stats.decoded_bytes[DECODE_BITENC]);
          if ( pop_stats.mime_stats.memcap_exceeded )
               _dpd.logMsg("  Sessions not decoded due to memory unavailability : " STDu64 "\n", pop_stats.mime_stats.memcap_exceeded);
          if ( pop_stats.log_memcap_exceeded )
            _dpd.logMsg("  POP Sessions fastpathed due to memcap exceeded: " STDu64 "\n", pop_stats.log_memcap_exceeded);
     }

}

#ifdef SNORT_RELOAD
static void POPReload(struct _SnortConfig *sc, char *args, void **new_config)
{
    tSfPolicyUserContextId pop_swap_config = (tSfPolicyUserContextId)*new_config;
    POPToken *tmp;
    tSfPolicyId policy_id = _dpd.getParserPolicy(sc);
    POPConfig *pPolicyConfig = NULL;

    if (pop_swap_config == NULL)
    {
        //create a context
        pop_swap_config = sfPolicyConfigCreate();
        if (pop_swap_config == NULL)
        {
            DynamicPreprocessorFatalMessage("Not enough memory to create POP "
                                            "configuration.\n");
        }
        *new_config = (void *)pop_swap_config;
    }

    sfPolicyUserPolicySet (pop_swap_config, policy_id);
    pPolicyConfig = (POPConfig *)sfPolicyUserDataGetCurrent(pop_swap_config);

    if (pPolicyConfig != NULL)
        DynamicPreprocessorFatalMessage("Can only configure POP preprocessor once.\n");

    pPolicyConfig = (POPConfig *)_dpd.snortAlloc(1, sizeof(POPConfig), PP_POP, 
                                      PP_MEM_CATEGORY_CONFIG);
    if (pPolicyConfig == NULL)
    {
        DynamicPreprocessorFatalMessage("Not enough memory to create POP "
                                        "configuration.\n");
    }

    sfPolicyUserDataSetCurrent(pop_swap_config, pPolicyConfig);

    POP_InitCmds(pPolicyConfig);
    POP_ParseArgs(pPolicyConfig, args);

    POP_CheckConfig(pPolicyConfig, pop_swap_config);
    POP_PrintConfig(pPolicyConfig);

    if( pPolicyConfig->disabled )
        return;

    if (_dpd.streamAPI == NULL)
    {
        DynamicPreprocessorFatalMessage("Streaming & reassembly must be enabled "
                                        "for POP preprocessor\n");
    }

    /* Command search - do this here because it's based on configuration */
    pPolicyConfig->cmd_search_mpse = _dpd.searchAPI->search_instance_new();
    if (pPolicyConfig->cmd_search_mpse == NULL)
    {
        DynamicPreprocessorFatalMessage("Could not allocate POP "
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

    _dpd.addPreproc(sc, POPDetect, PRIORITY_APPLICATION, PP_POP, PROTO_BIT__TCP);

    // register ports with session and stream
    registerPortsForDispatch( sc, pPolicyConfig );
    registerPortsForReassembly( pPolicyConfig, SSN_DIR_FROM_SERVER | SSN_DIR_FROM_CLIENT );
    _addPortsToStreamFilter(sc, pPolicyConfig, policy_id);

#ifdef TARGET_BASED
    _addServicesToStreamFilter(sc, policy_id);
#endif
}

static int POPMempoolFreeUsedBucket(MemPool *memory_pool)
{
   MemBucket *lru_bucket = NULL;

   lru_bucket = mempool_get_lru_bucket(memory_pool);
   if(lru_bucket)
   {
       /* Deleting least recently used POP session data here to adjust to new max_memory */
       _dpd.sessionAPI->set_application_data(lru_bucket->scbPtr, PP_POP, NULL, NULL);
       return 1;
   }
   return 0;
}

static unsigned POPReloadMimeMempoolAdjust(unsigned popMaxWork)
{
     int retVal;

     /* deleting MemBucket from free list in POP Mime Mempool */
     popMaxWork = mempool_prune_freelist(pop_mime_mempool, pop_mime_mempool->max_memory, popMaxWork);

     if(!popMaxWork)
        return 0;

     for( ; popMaxWork && ((pop_mime_mempool->used_memory + pop_mime_mempool->free_memory) > pop_mime_mempool->max_memory); popMaxWork--)
     {
         /* deleting least recently used MemBucket from Used list in POP Mime Mempool */
         retVal = POPMempoolFreeUsedBucket(pop_mime_mempool);
         if(!retVal)
            break;
     }

     return popMaxWork;
}

static unsigned POPReloadLogMempoolAdjust(unsigned popMaxWork)
{
    int retVal;

     /* deleting MemBucket from free list in POP Log Mempool */
     popMaxWork = mempool_prune_freelist(pop_mempool, pop_mempool->max_memory, popMaxWork);

     if(!popMaxWork)
        return 0;

     for( ; popMaxWork && ((pop_mempool->used_memory + pop_mempool->free_memory) > pop_mempool->max_memory); popMaxWork--)
     {
         /* deleting least recently used MemBucket from Used list in POP Log Mempool */
         retVal = POPMempoolFreeUsedBucket(pop_mempool);
         if(!retVal)
          break;
     }

     return popMaxWork;
}

static bool POPMimeReloadAdjust(bool idle, tSfPolicyId raPolicyId, void* userData)
{
    unsigned initialMaxWork = idle ? 512 : 5;
    unsigned maxWork;

    /* If new max_mime_mem is less than old configured max_mime_mem, need to adjust POP Mime Mempool.
     * In order to adjust to new max_memory of mime mempool, delete buckets from free list.
     * After deleting buckets from free list, still new max_memory is less than old value , delete buckets
     * (least recently used i.e head node of used list )from used list till total memory reaches to new max_memory.
     */
    maxWork = POPReloadMimeMempoolAdjust(initialMaxWork);

    if(maxWork == initialMaxWork)
    {
        pop_stats.max_conc_sessions = pop_stats.conc_sessions;
        pop_stats.mime_stats.memcap_exceeded = 0;
        return true;
    }
    else
        return false;
}

static bool POPLogReloadAdjust(bool idle, tSfPolicyId raPolicyId, void* userData)
{
    unsigned initialMaxWork = idle ? 512 : 5;
    unsigned maxWork;

    /* If new memcap is less than old configured memcap, need to adjust POP Log Mempool.
     * In order to adjust to new max_memory of log mempool, delete buckets from free list.
     * After deleting buckets from free list, still new max_memory is less than old value , delete buckets
     * (least recently used i.e head node of used list )from used list till total memory reaches to new max_memory.
     */
    maxWork = POPReloadLogMempoolAdjust(initialMaxWork);

    if(maxWork == initialMaxWork)
    {
        pop_stats.max_conc_sessions = pop_stats.conc_sessions;
        pop_stats.mime_stats.memcap_exceeded = 0;
        return true;
    }
    else
        return false;
}

static int POPReloadVerify(struct _SnortConfig *sc, void *swap_config)
{
    int rval;
    tSfPolicyUserContextId pop_swap_config = (tSfPolicyUserContextId)swap_config;
    POPConfig *config = NULL;
    POPConfig *configNext = NULL;
    tSfPolicyId policy_id = 0;

    if (pop_swap_config == NULL)
        return 0;

    if (pop_config != NULL)
        config = (POPConfig *)sfPolicyUserDataGet(pop_config, _dpd.getDefaultPolicy());

    configNext = (POPConfig *)sfPolicyUserDataGet(pop_swap_config, _dpd.getDefaultPolicy());

    if (config == NULL)
        return 0;

    if ((rval = sfPolicyUserDataIterate (sc, pop_swap_config, POPCheckPolicyConfig)))
        return rval;

    if ((rval = sfPolicyUserDataIterate (sc, pop_swap_config, CheckFilePolicyConfig)))
        return rval;

    policy_id = _dpd.getParserPolicy(sc);

    if (pop_mime_mempool != NULL)
    {
        /* If max_mime_mem changes, mime mempool need to be adjusted bcz mempool max_memory will be changed.
         * Registering here to adjust Mime memory Pool when max_mime_mem changes.
         */
        if( configNext->decode_conf.max_mime_mem  < config->decode_conf.max_mime_mem )
              _dpd.reloadAdjustRegister(sc, "POP-MIME-MEMPOOL", policy_id, &POPMimeReloadAdjust, NULL, NULL);
    }

    if (pop_mempool != NULL)
    {
        if(configNext)
        {
             /* If memcap cahnges, log mempool need to be adjusted bcz mempool max_mempory will be changed.
              * Registering here to adjust Log memory Pool when memcap changes.
              */
             if (configNext->memcap < config->memcap)
                   _dpd.reloadAdjustRegister(sc, "POP-LOG-MEMPOOL", policy_id, &POPLogReloadAdjust, NULL, NULL);
        }
    }
    else if(configNext != NULL)
    {
        if (sfPolicyUserDataIterate(sc, pop_swap_config, POPEnableDecoding) != 0)
        {
            pop_mime_mempool =  (MemPool *)_dpd.fileAPI->init_mime_mempool(configNext->decode_conf.max_mime_mem,
                    configNext->decode_conf.max_depth, pop_mime_mempool, PREPROC_NAME);
        }

        if (sfPolicyUserDataIterate(sc, pop_swap_config, POPLogExtraData) != 0)
        {
            pop_mempool = (MemPool *)_dpd.fileAPI->init_log_mempool(0,  configNext->memcap,
                    pop_mempool, PREPROC_NAME);

        }

        if ( configNext->disabled )
            return 0;
    }

   return 0;
}

static int POPReloadSwapPolicy(
        tSfPolicyUserContextId config,
        tSfPolicyId policyId,
        void* pData
        )
{
    POPConfig *pPolicyConfig = (POPConfig *)pData;

    if (pPolicyConfig->ref_count == 0)
    {
        sfPolicyUserDataClear (config, policyId);
        POP_FreeConfig(pPolicyConfig);
    }

    return 0;
}

static void * POPReloadSwap(struct _SnortConfig *sc, void *swap_config)
{
    POPConfig *configNew = NULL, *configOld = NULL;
    tSfPolicyUserContextId pop_swap_config = (tSfPolicyUserContextId)swap_config;
    tSfPolicyUserContextId old_config = pop_config;

    if (pop_swap_config == NULL)
        return NULL;

    pop_config = pop_swap_config;

    configOld = (POPConfig *)sfPolicyUserDataGet(old_config, _dpd.getDefaultPolicy());
    configNew = (POPConfig *)sfPolicyUserDataGet(pop_config, _dpd.getDefaultPolicy());

    if(configNew && configOld)
    {
          if(pop_mime_mempool)
          {
              if((configOld->decode_conf.max_mime_mem != configNew->decode_conf.max_mime_mem) ||
                 (configOld->decode_conf.max_depth != configNew->decode_conf.max_depth) )
              {
#ifdef REG_TEST
                   _dpd.fileAPI->displayMimeMempool(pop_mime_mempool,&(configOld->decode_conf), &(configNew->decode_conf));
#endif
                   /* Update the pop_mime_mempool with new max_memmory and object size when max_mime_mem changes. */
                   _dpd.fileAPI->update_mime_mempool(pop_mime_mempool, configNew->decode_conf.max_mime_mem, configNew->decode_conf.max_depth);
              }
          }
          if(pop_mempool)
          {
              if(configOld->memcap != configNew->memcap )
              {
#ifdef REG_TEST
                  _dpd.fileAPI->displayLogMempool(pop_mempool, configOld->memcap, configNew->memcap);
#endif
                  /* Update the pop_mempool with new max_memory and objest size when memcap changes. */
                  _dpd.fileAPI->update_log_mempool(pop_mempool, configNew->memcap, 0);
                  pop_stats.log_memcap_exceeded = 0;
              }
          }
#ifdef REG_TEST
          _dpd.fileAPI->displayDecodeDepth(&(configOld->decode_conf), &(configNew->decode_conf));
#endif

    }

    sfPolicyUserDataFreeIterate (old_config, POPReloadSwapPolicy);

    if (sfPolicyUserPolicyGetActive(old_config) == 0)
        return old_config;

    return NULL;
}

static void POPReloadSwapFree(void *data)
{
    if (data == NULL)
        return;

    POP_FreeConfigs((tSfPolicyUserContextId)data);
}
#endif
