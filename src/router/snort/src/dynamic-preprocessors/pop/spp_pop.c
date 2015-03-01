/****************************************************************************
 *
 * Copyright (C) 2014 Cisco and/or its affiliates. All rights reserved.
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

const int MAJOR_VERSION = 1;
const int MINOR_VERSION = 0;
const int BUILD_VERSION = 1;
const char *PREPROC_NAME = "SF_POP";
const char *PROTOCOL_NAME = "POP";

#define SetupPOP DYNAMIC_PREPROC_SETUP

MemPool *pop_mime_mempool = NULL;
MemPool *pop_mempool = NULL;

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
#ifdef TARGET_BASED
static void _addServicesToStreamFilter(struct _SnortConfig *, tSfPolicyId);
#endif
static int POPCheckConfig(struct _SnortConfig *);

#ifdef SNORT_RELOAD
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
}


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
        _dpd.addPreprocProfileFunc("pop", (void*)&popPerfStats, 0, _dpd.totalPerfStats);
#endif
    }

    sfPolicyUserPolicySet (pop_config, policy_id);
    pPolicyConfig = (POPConfig *)sfPolicyUserDataGetCurrent(pop_config);
    if (pPolicyConfig != NULL)
    {
        DynamicPreprocessorFatalMessage("Can only configure POP preprocessor once.\n");
    }

    pPolicyConfig = (POPConfig *)calloc(1, sizeof(POPConfig));
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

    context->decode_conf.file_depth = _dpd.fileAPI->get_max_file_depth();
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

    pPolicyConfig = (POPConfig *)calloc(1, sizeof(POPConfig));
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

static int POPReloadVerify(struct _SnortConfig *sc, void *swap_config)
{
    int rval;
    tSfPolicyUserContextId pop_swap_config = (tSfPolicyUserContextId)swap_config;
    POPConfig *config = NULL;
    POPConfig *configNext = NULL;

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

    if (pop_mime_mempool != NULL)
    {
        if(_dpd.fileAPI->is_decoding_conf_changed(&(configNext->decode_conf),
                &(config->decode_conf), "POP"))
        {
            return -1;
        }
    }

    if (pop_mempool != NULL)
    {
        if (configNext == NULL)
        {
            _dpd.errMsg("POP reload: Changing the memcap requires a restart.\n");
            return -1;
        }
        if (configNext->memcap != config->memcap)
        {
            _dpd.errMsg("POP reload: Changing the memcap requires a restart.\n");
            return -1;
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
    tSfPolicyUserContextId pop_swap_config = (tSfPolicyUserContextId)swap_config;
    tSfPolicyUserContextId old_config = pop_config;

    if (pop_swap_config == NULL)
        return NULL;

    pop_config = pop_swap_config;

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
