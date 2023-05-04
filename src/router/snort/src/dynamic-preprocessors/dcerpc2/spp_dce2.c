/****************************************************************************
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2008-2013 Sourcefire, Inc.
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
 ****************************************************************************
 *
 ****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <assert.h>
#include <stddef.h>
#include <time.h> 

#include "sf_types.h"
#include "spp_dce2.h"
#include "sf_preproc_info.h"
#include "dce2_memory.h"
#include "dce2_list.h"
#include "dce2_utils.h"
#include "dce2_config.h"
#include "dce2_roptions.h"
#include "dce2_stats.h"
#include "dce2_event.h"
#include "dce2_paf.h"
#include "dce2_smb.h"
#include "dce2_smb2.h"
#include "snort_dce2.h"
#include "preprocids.h"
#include "profiler.h"
#include "sfrt.h"
#include "sf_snort_packet.h"
#include "sf_dynamic_preprocessor.h"
#include "stream_api.h"
#include "sfPolicy.h"
#include "sfPolicyUserData.h"

#ifdef SNORT_RELOAD
#include "appdata_adjuster.h"
#include "dce2_session.h"
#ifdef REG_TEST
#include "reg_test.h"
#endif
#endif

#ifdef DCE2_LOG_EXTRA_DATA
#include "Unified2_common.h"
#endif

#ifdef DUMP_BUFFER
#include "dcerpc2_buffer_dump.h"
#endif

/********************************************************************
 * Global variables
 ********************************************************************/
#ifdef PERF_PROFILING
PreprocStats dce2_pstat_main;
PreprocStats dce2_pstat_session;
PreprocStats dce2_pstat_new_session;
PreprocStats dce2_pstat_session_state;
PreprocStats dce2_pstat_detect;
PreprocStats dce2_pstat_log;
PreprocStats dce2_pstat_smb_seg;
PreprocStats dce2_pstat_smb_req;
PreprocStats dce2_pstat_smb_uid;
PreprocStats dce2_pstat_smb_tid;
PreprocStats dce2_pstat_smb_fid;
PreprocStats dce2_pstat_smb_file;
PreprocStats dce2_pstat_smb_file_detect;
PreprocStats dce2_pstat_smb_file_api;
PreprocStats dce2_pstat_smb_fingerprint;
PreprocStats dce2_pstat_smb_negotiate;
PreprocStats dce2_pstat_co_seg;
PreprocStats dce2_pstat_co_frag;
PreprocStats dce2_pstat_co_reass;
PreprocStats dce2_pstat_co_ctx;
PreprocStats dce2_pstat_cl_acts;
PreprocStats dce2_pstat_cl_frag;
PreprocStats dce2_pstat_cl_reass;
#endif

const int MAJOR_VERSION = 1;
const int MINOR_VERSION = 0;
const int BUILD_VERSION = 3;
const char *PREPROC_NAME = "SF_DCERPC2";

#define DCE2_RegisterPreprocessor DYNAMIC_PREPROC_SETUP

/********************************************************************
 * Macros
 ********************************************************************/
#ifdef PERF_PROFILING
#define DCE2_PSTAT__MAIN         "DceRpcMain"
#define DCE2_PSTAT__SESSION      "DceRpcSession"
#define DCE2_PSTAT__NEW_SESSION  "DceRpcNewSession"
#define DCE2_PSTAT__SSN_STATE    "DceRpcSessionState"
#define DCE2_PSTAT__DETECT       "DceRpcDetect"
#define DCE2_PSTAT__LOG          "DceRpcLog"
#define DCE2_PSTAT__SMB_SEG      "DceRpcSmbSeg"
#define DCE2_PSTAT__SMB_REQ      "DceRpcSmbReq"
#define DCE2_PSTAT__SMB_UID      "DceRpcSmbUid"
#define DCE2_PSTAT__SMB_TID      "DceRpcSmbTid"
#define DCE2_PSTAT__SMB_FID      "DceRpcSmbFid"
#define DCE2_PSTAT__SMB_FILE     "DceRpcSmbFile"
#define DCE2_PSTAT__SMB_FILE_DETECT "DceRpcSmbFileDetect"
#define DCE2_PSTAT__SMB_FILE_API "DceRpcSmbFileAPI"
#define DCE2_PSTAT__SMB_FP       "DceRpcSmbFingerprint"
#define DCE2_PSTAT__SMB_NEG      "DceRpcSmbNegotiate"
#define DCE2_PSTAT__CO_SEG       "DceRpcCoSeg"
#define DCE2_PSTAT__CO_FRAG      "DceRpcCoFrag"
#define DCE2_PSTAT__CO_REASS     "DceRpcCoReass"
#define DCE2_PSTAT__CO_CTX       "DceRpcCoCtx"
#define DCE2_PSTAT__CL_ACTS      "DceRpcClActs"
#define DCE2_PSTAT__CL_FRAG      "DceRpcClFrag"
#define DCE2_PSTAT__CL_REASS     "DceRpcClReass"
#endif  /* PERF_PROFILING */

/********************************************************************
 * Private function prototypes
 ********************************************************************/
static void DCE2_InitGlobal(struct _SnortConfig *, char *);
static void DCE2_InitServer(struct _SnortConfig *, char *);
static int DCE2_CheckConfig(struct _SnortConfig *);
static void DCE2_Main(void *, void *);
static void DCE2_PrintStats(int);
static void DCE2_Reset(int, void *);
static void DCE2_ResetStats(int, void *);
static void DCE2_CleanExit(int, void *);
#ifdef DCE2_LOG_EXTRA_DATA
static int DCE2_LogSmbFileName(void *, uint8_t **, uint32_t *, uint32_t *);
#endif

#ifdef SNORT_RELOAD
static void DCE2_ReloadGlobal(struct _SnortConfig *, char *, void **);
static void DCE2_ReloadServer(struct _SnortConfig *, char *, void **);
static int DCE2_ReloadVerify(struct _SnortConfig *, void *);
static bool DCE2_ReloadAdjust(bool, tSfPolicyId, void *);
static void * DCE2_ReloadSwap(struct _SnortConfig *, void *);
static void DCE2_ReloadSwapFree(void *);
#endif

static void DCE2_AddPortsToPaf(struct _SnortConfig *, DCE2_Config *, tSfPolicyId);
static void DCE2_ScAddPortsToPaf(struct _SnortConfig *, void *);
static uint32_t max(uint32_t a, uint32_t b);
static uint32_t DCE2_GetReloadSafeMemcap();

static bool dce2_file_cache_is_enabled = false;
static bool dce2_file_cache_was_enabled = false;
#ifdef SNORT_RELOAD
static bool dce2_ada_was_enabled = false;
static bool dce2_ada_is_enabled = false;
#endif
int dce_print_mem_stats(FILE *, char *, PreprocMemInfo *);

/********************************************************************
 * Function: DCE2_RegisterPreprocessor()
 *
 * Purpose: Registers the DCE/RPC preprocessor with Snort
 *
 * Arguments: None
 *
 * Returns: None
 *
 ********************************************************************/
void DCE2_RegisterPreprocessor(void)
{
#ifndef SNORT_RELOAD
    _dpd.registerPreproc(DCE2_GNAME, DCE2_InitGlobal);
    _dpd.registerPreproc(DCE2_SNAME, DCE2_InitServer);
#else
    _dpd.registerPreproc(DCE2_GNAME, DCE2_InitGlobal, DCE2_ReloadGlobal,
                         DCE2_ReloadVerify, DCE2_ReloadSwap,
                         DCE2_ReloadSwapFree);
    _dpd.registerPreproc(DCE2_SNAME, DCE2_InitServer,
                         DCE2_ReloadServer, NULL, NULL, NULL);
#endif
#ifdef DUMP_BUFFER
    _dpd.registerBufferTracer(getDCERPC2Buffers, DCERPC2_BUFFER_DUMP_FUNC);
#endif
    _dpd.registerMemoryStatsFunc(PP_DCE2, dce_print_mem_stats);
}

/*********************************************************************
 * Function: DCE2_InitGlobal()
 *
 * Purpose: Initializes the global DCE/RPC preprocessor config.
 *
 * Arguments: snort.conf argument line for the DCE/RPC preprocessor.
 *
 * Returns: None
 *
 *********************************************************************/
static void DCE2_InitGlobal(struct _SnortConfig *sc, char *args)
{
    tSfPolicyId policy_id = _dpd.getParserPolicy(sc);
    DCE2_Config *pDefaultPolicyConfig = NULL;
    DCE2_Config *pCurrentPolicyConfig = NULL;

    if ((_dpd.streamAPI == NULL) || (_dpd.streamAPI->version != STREAM_API_VERSION5))
    {
        DCE2_Die("%s(%d) \"%s\" configuration: "
            "Stream must be enabled with TCP and UDP tracking.",
            *_dpd.config_file, *_dpd.config_line, DCE2_GNAME);
    }

    if (dce2_config == NULL)
    {
        dce2_config = sfPolicyConfigCreate();
        dce2_file_cache_is_enabled = false;
        dce2_file_cache_was_enabled = false;
#ifdef SNORT_RELOAD
        dce2_ada_was_enabled = false;
        dce2_ada_is_enabled = false;
#endif
        if (dce2_config == NULL)
        {
            DCE2_Die("%s(%d) \"%s\" configuration: Could not allocate memory "
                     "configuration.\n",
                     *_dpd.config_file, *_dpd.config_line, DCE2_GNAME);
        }

        DCE2_MemInit();
        DCE2_StatsInit();
        DCE2_EventsInit();
        smb_file_name[0] = '\0';

        /* Initialize reassembly packet */
        DCE2_InitRpkts();

#ifdef ACTIVE_RESPONSE
        DCE2_SmbInitDeletePdu();
#endif

        DCE2_SmbInitGlobals();

        _dpd.addPreprocConfCheck(sc, DCE2_CheckConfig);
        _dpd.registerPreprocStats(DCE2_GNAME, DCE2_PrintStats);
        _dpd.addPreprocReset(DCE2_Reset, NULL, PRIORITY_LAST, PP_DCE2);
        _dpd.addPreprocResetStats(DCE2_ResetStats, NULL, PRIORITY_LAST, PP_DCE2);
        _dpd.addPreprocExit(DCE2_CleanExit, NULL, PRIORITY_LAST, PP_DCE2);

#ifdef PERF_PROFILING
        _dpd.addPreprocProfileFunc(DCE2_PSTAT__MAIN, &dce2_pstat_main, 0, _dpd.totalPerfStats, NULL);
        _dpd.addPreprocProfileFunc(DCE2_PSTAT__SESSION, &dce2_pstat_session, 1, &dce2_pstat_main, NULL);
        _dpd.addPreprocProfileFunc(DCE2_PSTAT__NEW_SESSION, &dce2_pstat_new_session, 2, &dce2_pstat_session, NULL);
        _dpd.addPreprocProfileFunc(DCE2_PSTAT__SSN_STATE, &dce2_pstat_session_state, 2, &dce2_pstat_session, NULL);
        _dpd.addPreprocProfileFunc(DCE2_PSTAT__LOG, &dce2_pstat_log, 1, &dce2_pstat_main, NULL);
        _dpd.addPreprocProfileFunc(DCE2_PSTAT__DETECT, &dce2_pstat_detect, 1, &dce2_pstat_main, NULL);
        _dpd.addPreprocProfileFunc(DCE2_PSTAT__SMB_SEG, &dce2_pstat_smb_seg, 1, &dce2_pstat_main, NULL);
        _dpd.addPreprocProfileFunc(DCE2_PSTAT__SMB_REQ, &dce2_pstat_smb_req, 1, &dce2_pstat_main, NULL);
        _dpd.addPreprocProfileFunc(DCE2_PSTAT__SMB_UID, &dce2_pstat_smb_uid, 1, &dce2_pstat_main, NULL);
        _dpd.addPreprocProfileFunc(DCE2_PSTAT__SMB_TID, &dce2_pstat_smb_tid, 1, &dce2_pstat_main, NULL);
        _dpd.addPreprocProfileFunc(DCE2_PSTAT__SMB_FID, &dce2_pstat_smb_fid, 1, &dce2_pstat_main, NULL);
        _dpd.addPreprocProfileFunc(DCE2_PSTAT__SMB_FILE, &dce2_pstat_smb_file, 1, &dce2_pstat_main, NULL);
        _dpd.addPreprocProfileFunc(DCE2_PSTAT__SMB_FILE_DETECT, &dce2_pstat_smb_file_detect, 2, &dce2_pstat_smb_file, NULL);
        _dpd.addPreprocProfileFunc(DCE2_PSTAT__SMB_FILE_API, &dce2_pstat_smb_file_api, 2, &dce2_pstat_smb_file, NULL);
        _dpd.addPreprocProfileFunc(DCE2_PSTAT__SMB_FP, &dce2_pstat_smb_fingerprint, 1, &dce2_pstat_main, NULL);
        _dpd.addPreprocProfileFunc(DCE2_PSTAT__SMB_NEG, &dce2_pstat_smb_negotiate, 1, &dce2_pstat_main, NULL);
        _dpd.addPreprocProfileFunc(DCE2_PSTAT__CO_SEG, &dce2_pstat_co_seg, 1, &dce2_pstat_main, NULL);
        _dpd.addPreprocProfileFunc(DCE2_PSTAT__CO_FRAG, &dce2_pstat_co_frag, 1, &dce2_pstat_main, NULL);
        _dpd.addPreprocProfileFunc(DCE2_PSTAT__CO_REASS, &dce2_pstat_co_reass, 1, &dce2_pstat_main, NULL);
        _dpd.addPreprocProfileFunc(DCE2_PSTAT__CO_CTX, &dce2_pstat_co_ctx, 1, &dce2_pstat_main, NULL);
        _dpd.addPreprocProfileFunc(DCE2_PSTAT__CL_ACTS, &dce2_pstat_cl_acts, 1, &dce2_pstat_main, NULL);
        _dpd.addPreprocProfileFunc(DCE2_PSTAT__CL_FRAG, &dce2_pstat_cl_frag, 1, &dce2_pstat_main, NULL);
        _dpd.addPreprocProfileFunc(DCE2_PSTAT__CL_REASS, &dce2_pstat_cl_reass, 1, &dce2_pstat_main, NULL);
#endif

#ifdef TARGET_BASED
        dce2_proto_ids.dcerpc = _dpd.findProtocolReference(DCE2_PROTO_REF_STR__DCERPC);
        if (dce2_proto_ids.dcerpc == SFTARGET_UNKNOWN_PROTOCOL)
            dce2_proto_ids.dcerpc = _dpd.addProtocolReference(DCE2_PROTO_REF_STR__DCERPC);

        /* smb and netbios-ssn refer to the same thing */
        dce2_proto_ids.nbss = _dpd.findProtocolReference(DCE2_PROTO_REF_STR__NBSS);
        if (dce2_proto_ids.nbss == SFTARGET_UNKNOWN_PROTOCOL)
            dce2_proto_ids.nbss = _dpd.addProtocolReference(DCE2_PROTO_REF_STR__NBSS);

        // register with session to handle service 
        _dpd.sessionAPI->register_service_handler( PP_DCE2, dce2_proto_ids.dcerpc );
        _dpd.sessionAPI->register_service_handler( PP_DCE2, dce2_proto_ids.nbss );
#endif
    }

    sfPolicyUserPolicySet(dce2_config, policy_id);
    pDefaultPolicyConfig = (DCE2_Config *)sfPolicyUserDataGetDefault(dce2_config);
    pCurrentPolicyConfig = (DCE2_Config *)sfPolicyUserDataGetCurrent(dce2_config);

    if ((policy_id != 0) && (pDefaultPolicyConfig == NULL))
    {
        DCE2_Die("%s(%d) \"%s\" configuration: Must configure default policy "
                 "if other policies are to be configured.\n",
                 *_dpd.config_file, *_dpd.config_line, DCE2_GNAME);
    }

    /* Can only do one global configuration */
    if (pCurrentPolicyConfig != NULL)
    {
        DCE2_Die("%s(%d) \"%s\" configuration: Only one global configuration can be specified.",
                 *_dpd.config_file, *_dpd.config_line, DCE2_GNAME);
    }

    DCE2_RegRuleOptions(sc);

    pCurrentPolicyConfig = (DCE2_Config *)DCE2_Alloc(sizeof(DCE2_Config), DCE2_MEM_TYPE__CONFIG);
    sfPolicyUserDataSetCurrent(dce2_config, pCurrentPolicyConfig);

    /* Parse configuration args */
    DCE2_GlobalConfigure(pCurrentPolicyConfig, args);

    if (policy_id != 0)
        pCurrentPolicyConfig->gconfig->memcap = pDefaultPolicyConfig->gconfig->memcap;

    if ( pCurrentPolicyConfig->gconfig->disabled )
        return;

    /* Register callbacks */
    _dpd.addPreproc(sc, DCE2_Main, PRIORITY_APPLICATION, PP_DCE2, PROTO_BIT__TCP | PROTO_BIT__UDP);

#ifdef TARGET_BASED
    _dpd.streamAPI->set_service_filter_status
        (sc, dce2_proto_ids.dcerpc, PORT_MONITOR_SESSION, policy_id, 1);

    _dpd.streamAPI->set_service_filter_status
        (sc, dce2_proto_ids.nbss, PORT_MONITOR_SESSION, policy_id, 1);
#endif

#ifdef SNORT_RELOAD
    if (!ada)
    {
        size_t memcap = DCE2_GetReloadSafeMemcap(dce2_config);
        ada = ada_init(DCE2_MemInUse, PP_DCE2, memcap);
        if (!ada)
            _dpd.fatalMsg("Failed to initialize DCE ADA session cache.\n");
    }
    dce2_ada_is_enabled = true;
#endif
}

/*********************************************************************
 * Function: DCE2_InitServer()
 *
 * Purpose: Initializes a DCE/RPC server configuration
 *
 * Arguments: snort.conf argument line for the DCE/RPC preprocessor.
 *
 * Returns: None
 *
 *********************************************************************/
static void DCE2_InitServer(struct _SnortConfig *sc, char *args)
{
    tSfPolicyId policy_id = _dpd.getParserPolicy(sc);
    DCE2_Config *pPolicyConfig = NULL;

    if (dce2_config != NULL)
    {
        sfPolicyUserPolicySet (dce2_config, policy_id);
        pPolicyConfig = (DCE2_Config *)sfPolicyUserDataGetCurrent(dce2_config);
    }

    if ((dce2_config == NULL) || (pPolicyConfig == NULL)
            || (pPolicyConfig->gconfig == NULL))
    {
        DCE2_Die("%s(%d) \"%s\" configuration: \"%s\" must be configured "
                 "before \"%s\".", *_dpd.config_file, *_dpd.config_line,
                 DCE2_SNAME, DCE2_GNAME, DCE2_SNAME);
    }

    /* Parse configuration args */
    DCE2_ServerConfigure(sc, pPolicyConfig, args);

    // enable preproc for ports of interest...
    // TBD-EDM - verify...
    DCE2_RegisterPortsWithSession( sc, pPolicyConfig->dconfig );
}

static int DCE2_CheckConfigPolicy(
        struct _SnortConfig *sc,
        tSfPolicyUserContextId config,
        tSfPolicyId policyId,
        void* pData
        )
{
    int rval;
    DCE2_Config *pPolicyConfig = (DCE2_Config *)pData;
    DCE2_ServerConfig *dconfig;

    if ( pPolicyConfig->gconfig->disabled )
        return 0;

    _dpd.setParserPolicy(sc, policyId);
    // config_file/config_line are not set here
    if (!_dpd.isPreprocEnabled(sc, PP_STREAM))
    {
        DCE2_Log(DCE2_LOG_TYPE__WARN, "Stream must be enabled with TCP and UDP tracking.");
        return -1;
    }

    dconfig = pPolicyConfig->dconfig;

    if (dconfig == NULL)
    {
        if ((rval = DCE2_CreateDefaultServerConfig(sc, pPolicyConfig, policyId)))
            return rval;
    }

#ifdef TARGET_BASED
    if (!_dpd.isAdaptiveConfiguredForSnortConfig(sc))
#endif
    {
        if ((rval = DCE2_ScCheckTransports(pPolicyConfig)))
            return rval;
    }

    DCE2_AddPortsToPaf(sc, pPolicyConfig, policyId);
#ifdef TARGET_BASED
    DCE2_PafRegisterService(sc, dce2_proto_ids.nbss, policyId, DCE2_TRANS_TYPE__SMB);
    DCE2_PafRegisterService(sc, dce2_proto_ids.dcerpc, policyId, DCE2_TRANS_TYPE__TCP);
#endif

#ifdef DCE2_LOG_EXTRA_DATA
    pPolicyConfig->xtra_logging_smb_file_name_id =
        _dpd.streamAPI->reg_xtra_data_cb(DCE2_LogSmbFileName);
#endif

    /* Register routing table memory */
    if (pPolicyConfig->sconfigs != NULL)
        DCE2_RegMem(sfrt_usage(pPolicyConfig->sconfigs), DCE2_MEM_TYPE__RT);

    if (!pPolicyConfig->gconfig->legacy_mode)
    {
        DCE2_Smb2Init(pPolicyConfig->gconfig->memcap);
        dce2_file_cache_is_enabled = true;
    }

    return 0;
}

/*********************************************************************
 * Function: DCE2_CheckConfig()
 *
 * Purpose: Verifies the DCE/RPC preprocessor configuration
 *
 * Arguments: None
 *
 * Returns: None
 *
 *********************************************************************/
static int DCE2_CheckConfig(struct _SnortConfig *sc)
{
    int rval;

    if ((rval = sfPolicyUserDataIterate (sc, dce2_config, DCE2_CheckConfigPolicy)))
    {
        return rval;
    }
    return 0;
}

/*********************************************************************
 * Function: DCE2_Main()
 *
 * Purpose: Main entry point for DCE/RPC processing.
 *
 * Arguments:
 *  void * - pointer to packet structure
 *  void * - pointer to context
 *
 * Returns: None
 *
 *********************************************************************/
static void DCE2_Main(void *pkt, void *context)
{
    SFSnortPacket *p = (SFSnortPacket *)pkt;
    PROFILE_VARS;

    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ALL, "%s\n", DCE2_DEBUG__START_MSG));

    sfPolicyUserPolicySet (dce2_config, _dpd.getNapRuntimePolicy());

#ifdef DUMP_BUFFER
    dumpBufferInit();
#endif

#ifdef DEBUG_MSGS
    if (DCE2_SsnFromServer(p))
    {
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__MAIN, "Packet from Server.\n"));
    }
    else
    {
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__MAIN, "Packet from Client.\n"));
    }
#endif

    // preconditions - what we registered for
    assert((IsUDP(p) || IsTCP(p)) && p->payload && p->payload_size);

    /* No inspection to do */
    if ( !_dpd.sessionAPI->is_session_verified( p->stream_session ) )
    {
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__MAIN, "Session not established - not inspecting.\n"));
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ALL, "%s\n", DCE2_DEBUG__END_MSG));
        return;
    }

    PREPROC_PROFILE_START(dce2_pstat_main);

    if (DCE2_Process(p) == DCE2_RET__INSPECTED)
        DCE2_DisableDetect(p);

    PREPROC_PROFILE_END(dce2_pstat_main);

    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ALL, "%s\n", DCE2_DEBUG__END_MSG));
}

#ifdef DCE2_LOG_EXTRA_DATA
/******************************************************************
 * Function: DCE2_LogSmbFileName
 *
 * Purpose: Callback for unified2 logging of extra data, in this
 *  case the SMB file name.
 *
 * Arguments:
 *  void *      - stream session pointer
 *  uint8_t **  - pointer to buffer for extra data
 *  uint32_t *  - pointer to length of extra data
 *  uint32_t *  - pointer to type of extra data
 *
 * Returns:
 *  int - 1 for success
 *        0 for failure
 *
 ******************************************************************/
static int DCE2_LogSmbFileName(void *ssn_ptr, uint8_t **buf, uint32_t *len, uint32_t *type)
{
    if ((_dpd.streamAPI->get_application_data(ssn_ptr, PP_DCE2) == NULL)
            || (smb_file_name_len == 0))
        return 0;

    *buf = (uint8_t *)smb_file_name;
    *len = smb_file_name_len;
    *type = EVENT_INFO_SMB_FILENAME;

    return 1;
}
#endif

/******************************************************************
 * Function: DCE2_PrintStats()
 *
 * Purpose: Print statistics being kept by the preprocessor.
 *
 * Arguments:
 *  int - whether Snort is exiting or not
 *
 * Returns: None
 *
 ******************************************************************/
static void DCE2_PrintStats(int exiting)
{
    int smb_com;
    int sub_com;

    _dpd.logMsg("dcerpc2 Preprocessor Statistics\n");
    _dpd.logMsg("  Total sessions: "STDu64"\n", dce2_stats.sessions);
    _dpd.logMsg(" Active sessions: "STDu64"\n", dce2_stats.sessions_active);
    if (dce2_stats.sessions > 0)
    {
        if (dce2_stats.sessions_autodetected > 0)
            _dpd.logMsg("  Total sessions autodetected: "STDu64"\n", dce2_stats.sessions_autodetected);
        if (dce2_stats.sessions_aborted > 0)
            _dpd.logMsg("  Total sessions aborted: "STDu64"\n", dce2_stats.sessions_aborted);
        if (dce2_stats.bad_autodetects > 0)
            _dpd.logMsg("  Bad autodetects: "STDu64"\n", dce2_stats.bad_autodetects);
        if (dce2_stats.events > 0)
            _dpd.logMsg("  Preprocessor events: "STDu64"\n", dce2_stats.events);
#ifdef DEBUG
        {
            unsigned int port;
            int first = 1;

            for (port = 0; port < (sizeof(dce2_stats.autoports) / sizeof(dce2_stats.autoports[0])); port++)
            {
                DCE2_TransType ttype;

                for (ttype = DCE2_TRANS_TYPE__NONE; ttype < DCE2_TRANS_TYPE__MAX; ttype++)
                {
                    if ((dce2_stats.autoports[port][ttype] > 0) && (dce2_trans_strs[ttype] != NULL))
                    {
                        if (first)
                        {
                            _dpd.logMsg("\n");
                            _dpd.logMsg("  Autodetected ports:\n");
                            _dpd.logMsg("  %7s%15s%15s\n", "Port", "Transport", "Total");
                            first = 0;
                        }

                        _dpd.logMsg("  %7u%15s"FMTu64("15")"\n",
                                    port, dce2_trans_strs[ttype], dce2_stats.autoports[port][ttype]);
                    }
                }
            }
        }
#endif

        _dpd.logMsg("\n");
        _dpd.logMsg("  Transports\n");
        if (dce2_stats.smb_sessions > 0)
        {
            _dpd.logMsg("    SMB\n");
            _dpd.logMsg("      Total sessions: "STDu64"\n", dce2_stats.smb_sessions);
            _dpd.logMsg("      Packet stats\n");
            _dpd.logMsg("        Packets: "STDu64"\n", dce2_stats.smb_pkts);
            if (dce2_stats.smb_ignored_bytes > 0)
                _dpd.logMsg("        Ignored bytes: "STDu64"\n", dce2_stats.smb_ignored_bytes);
            if (dce2_stats.smb_files_processed > 0)
                _dpd.logMsg("        Files processed: "STDu64"\n", dce2_stats.smb_files_processed);
            if (dce2_stats.smb_cli_seg_reassembled > 0)
                _dpd.logMsg("        Client TCP reassembled: "STDu64"\n", dce2_stats.smb_cli_seg_reassembled);
            if (dce2_stats.smb_srv_seg_reassembled > 0)
                _dpd.logMsg("        Server TCP reassembled: "STDu64"\n", dce2_stats.smb_srv_seg_reassembled);

            _dpd.logMsg("        Maximum outstanding requests: "STDu64"\n",
                    dce2_stats.smb_max_outstanding_requests);

            // SMB command stats
            _dpd.logMsg("        SMB command requests/responses processed\n");
            for (smb_com = 0; smb_com < SMB_MAX_NUM_COMS; smb_com++)
            {
                SmbAndXCom andx = smb_chain_map[smb_com];

                // Print out the stats for command requests
                if ((dce2_stats.smb_com_stats[SMB_TYPE__REQUEST][smb_com] != 0)
                        || (dce2_stats.smb_com_stats[SMB_TYPE__RESPONSE][smb_com] != 0))
                {
                    _dpd.logMsg("          %s (0x%02X) : "STDu64"/"STDu64"\n",
                            smb_com_strings[smb_com], smb_com,
                            dce2_stats.smb_com_stats[SMB_TYPE__REQUEST][smb_com],
                            dce2_stats.smb_com_stats[SMB_TYPE__RESPONSE][smb_com]);

                    switch (smb_com)
                    {
                        case SMB_COM_TRANSACTION:
                            for (sub_com = 0; sub_com < TRANS_SUBCOM_MAX+1; sub_com++)
                            {
                                if ((dce2_stats.smb_trans_subcom_stats[SMB_TYPE__REQUEST][sub_com] != 0)
                                        || (dce2_stats.smb_trans_subcom_stats[SMB_TYPE__RESPONSE][sub_com] != 0))
                                {
                                    _dpd.logMsg("            %s (0x%04X) : "STDu64"/"STDu64"\n",
                                            (sub_com < TRANS_SUBCOM_MAX)
                                            ? smb_transaction_sub_command_strings[sub_com] : "Unknown",
                                            sub_com,
                                            dce2_stats.smb_trans_subcom_stats[SMB_TYPE__REQUEST][sub_com],
                                            dce2_stats.smb_trans_subcom_stats[SMB_TYPE__RESPONSE][sub_com]);
                                }
                            }
                            break;
                        case SMB_COM_TRANSACTION2:
                            for (sub_com = 0; sub_com < TRANS2_SUBCOM_MAX+1; sub_com++)
                            {
                                if ((dce2_stats.smb_trans2_subcom_stats[SMB_TYPE__REQUEST][sub_com] != 0)
                                        || (dce2_stats.smb_trans2_subcom_stats[SMB_TYPE__RESPONSE][sub_com] != 0))
                                {
                                    _dpd.logMsg("            %s (0x%04X) : "STDu64"/"STDu64"\n",
                                            (sub_com < TRANS2_SUBCOM_MAX)
                                            ? smb_transaction2_sub_command_strings[sub_com] : "Unknown",
                                            sub_com,
                                            dce2_stats.smb_trans2_subcom_stats[SMB_TYPE__REQUEST][sub_com],
                                            dce2_stats.smb_trans2_subcom_stats[SMB_TYPE__RESPONSE][sub_com]);
                                }
                            }
                            break;
                        case SMB_COM_NT_TRANSACT:
                            for (sub_com = 0; sub_com < NT_TRANSACT_SUBCOM_MAX+1; sub_com++)
                            {
                                if ((dce2_stats.smb_nt_transact_subcom_stats[SMB_TYPE__REQUEST][sub_com] != 0)
                                        || (dce2_stats.smb_nt_transact_subcom_stats[SMB_TYPE__RESPONSE][sub_com] != 0))
                                {
                                    _dpd.logMsg("            %s (0x%04X) : "STDu64"/"STDu64"\n",
                                            (sub_com < NT_TRANSACT_SUBCOM_MAX)
                                            ? smb_nt_transact_sub_command_strings[sub_com] : "Unknown",
                                            sub_com,
                                            dce2_stats.smb_nt_transact_subcom_stats[SMB_TYPE__REQUEST][sub_com],
                                            dce2_stats.smb_nt_transact_subcom_stats[SMB_TYPE__RESPONSE][sub_com]);
                                }
                            }
                            break;
                        default:
                            break;
                    }
                }

                // Print out chaining stats for AndX command requests
                if (andx != SMB_ANDX_COM__NONE)
                {
                    int chained_com;

                    for (chained_com = 0; chained_com < SMB_MAX_NUM_COMS; chained_com++)
                    {
                        if ((dce2_stats.smb_chained_stats[SMB_TYPE__REQUEST][andx][chained_com] != 0)
                                || (dce2_stats.smb_chained_stats[SMB_TYPE__RESPONSE][andx][chained_com] != 0))
                        {
                            _dpd.logMsg("            => %s (0x%02X) : "STDu64"/"STDu64"\n",
                                    smb_com_strings[chained_com], chained_com,
                                    dce2_stats.smb_chained_stats[SMB_TYPE__REQUEST][andx][chained_com],
                                    dce2_stats.smb_chained_stats[SMB_TYPE__RESPONSE][andx][chained_com]);
                        }
                    }
                }
            }

            _dpd.logMsg("      Memory stats (bytes)\n");
            _dpd.logMsg("        Current total: %u\n", dce2_memory.smb_total);
            _dpd.logMsg("        Maximum total: %u\n", dce2_memory.smb_total_max);
            _dpd.logMsg("        Current session data: %u\n", dce2_memory.smb_ssn);
            _dpd.logMsg("        Maximum session data: %u\n", dce2_memory.smb_ssn_max);
            _dpd.logMsg("        Current segmentation buffering: %u\n", dce2_memory.smb_seg);
            _dpd.logMsg("        Maximum segmentation buffering: %u\n", dce2_memory.smb_seg_max);
            _dpd.logMsg("        Current uid tracking: %u\n", dce2_memory.smb_uid);
            _dpd.logMsg("        Maximum uid tracking: %u\n", dce2_memory.smb_uid_max);
            _dpd.logMsg("        Current tid tracking: %u\n", dce2_memory.smb_tid);
            _dpd.logMsg("        Maximum tid tracking: %u\n", dce2_memory.smb_tid_max);
            _dpd.logMsg("        Current fid tracking: %u\n", dce2_memory.smb_fid);
            _dpd.logMsg("        Maximum fid tracking: %u\n", dce2_memory.smb_fid_max);
            _dpd.logMsg("        Current file tracking: %u\n", dce2_memory.smb_file);
            _dpd.logMsg("        Maximum file tracking: %u\n", dce2_memory.smb_file_max);
            _dpd.logMsg("        Current request tracking: %u\n", dce2_memory.smb_req);
            _dpd.logMsg("        Maximum request tracking: %u\n", dce2_memory.smb_req_max);
            /* SMB2 stats */
            if (!exiting)
            {
            	DCE2_Smb2UpdateStats();
            }
            _dpd.logMsg("    SMB2\n");
            _dpd.logMsg("      Smb2 prunes: "STDu64"\n", dce2_stats.smb2_prunes);
            _dpd.logMsg("      Memory used for smb2 processing: "STDu64"\n", dce2_stats.smb2_memory_in_use);
            _dpd.logMsg("      Maximum memory used for smb2 processing: "STDu64"\n", dce2_stats.smb2_memory_in_use_max);
            _dpd.logMsg("      SMB2 command requests/responses processed\n");
            _dpd.logMsg("        smb2 create         : "STDu64"\n", dce2_stats.smb2_create);
            _dpd.logMsg("        smb2 write          : "STDu64"\n", dce2_stats.smb2_write);
            _dpd.logMsg("        smb2 read           : "STDu64"\n", dce2_stats.smb2_read);
            _dpd.logMsg("        smb2 set info       : "STDu64"\n", dce2_stats.smb2_set_info);
            _dpd.logMsg("        smb2 tree connect   : "STDu64"\n", dce2_stats.smb2_tree_connect);
            _dpd.logMsg("        smb2 tree disconnect: "STDu64"\n", dce2_stats.smb2_tree_disconnect);
            _dpd.logMsg("        smb2 close          : "STDu64"\n", dce2_stats.smb2_close);
        }

        if (dce2_stats.tcp_sessions > 0)
        {
            _dpd.logMsg("    TCP\n");
            _dpd.logMsg("      Total sessions: "STDu64"\n", dce2_stats.tcp_sessions);
            _dpd.logMsg("      Packet stats\n");
            _dpd.logMsg("        Packets: "STDu64"\n", dce2_stats.tcp_pkts);
            _dpd.logMsg("      Memory stats (bytes)\n");
            _dpd.logMsg("        Current total: %u\n", dce2_memory.tcp_total);
            _dpd.logMsg("        Maximum total: %u\n", dce2_memory.tcp_total_max);
            _dpd.logMsg("        Current session data: %u\n", dce2_memory.tcp_ssn);
            _dpd.logMsg("        Maximum session data: %u\n", dce2_memory.tcp_ssn_max);
        }

        if (dce2_stats.udp_sessions > 0)
        {
            _dpd.logMsg("    UDP\n");
            _dpd.logMsg("      Total sessions: "STDu64"\n", dce2_stats.udp_sessions);
            _dpd.logMsg("      Packet stats\n");
            _dpd.logMsg("        Packets: "STDu64"\n", dce2_stats.udp_pkts);
            _dpd.logMsg("      Memory stats (bytes)\n");
            _dpd.logMsg("        Current total: %u\n", dce2_memory.udp_total);
            _dpd.logMsg("        Maximum total: %u\n", dce2_memory.udp_total_max);
            _dpd.logMsg("        Current session data: %u\n", dce2_memory.udp_ssn);
            _dpd.logMsg("        Maximum session data: %u\n", dce2_memory.udp_ssn_max);
        }

        if ((dce2_stats.http_server_sessions > 0) || (dce2_stats.http_proxy_sessions > 0))
        {
            _dpd.logMsg("    RPC over HTTP\n");
            if (dce2_stats.http_server_sessions > 0)
                _dpd.logMsg("      Total server sessions: "STDu64"\n", dce2_stats.http_server_sessions);
            if (dce2_stats.http_proxy_sessions > 0)
                _dpd.logMsg("      Total proxy sessions: "STDu64"\n", dce2_stats.http_proxy_sessions);
            _dpd.logMsg("      Packet stats\n");
            if (dce2_stats.http_server_sessions > 0)
                _dpd.logMsg("        Server packets: "STDu64"\n", dce2_stats.http_server_pkts);
            if (dce2_stats.http_proxy_sessions > 0)
                _dpd.logMsg("        Proxy packets: "STDu64"\n", dce2_stats.http_proxy_pkts);
            _dpd.logMsg("      Memory stats (bytes)\n");
            _dpd.logMsg("        Current total: %u\n", dce2_memory.http_total);
            _dpd.logMsg("        Maximum total: %u\n", dce2_memory.http_total_max);
            _dpd.logMsg("        Current session data: %u\n", dce2_memory.http_ssn);
            _dpd.logMsg("        Maximum session data: %u\n", dce2_memory.http_ssn_max);
        }

        if ((dce2_stats.co_pdus > 0) || (dce2_stats.cl_pkts > 0))
        {
            _dpd.logMsg("\n");
            _dpd.logMsg("  DCE/RPC\n");
            if (dce2_stats.co_pdus > 0)
            {
                _dpd.logMsg("    Connection oriented\n");
                _dpd.logMsg("      Packet stats\n");
                _dpd.logMsg("        PDUs: "STDu64"\n", dce2_stats.co_pdus);
                if ((dce2_stats.co_bind > 0) || (dce2_stats.co_bind_ack > 0))
                {
                    _dpd.logMsg("          Bind: "STDu64"\n", dce2_stats.co_bind);
                    _dpd.logMsg("          Bind Ack: "STDu64"\n", dce2_stats.co_bind_ack);
                }
                if ((dce2_stats.co_alter_ctx > 0) || (dce2_stats.co_alter_ctx_resp > 0))
                {
                    _dpd.logMsg("          Alter context: "STDu64"\n", dce2_stats.co_alter_ctx);
                    _dpd.logMsg("          Alter context response: "STDu64"\n", dce2_stats.co_alter_ctx_resp);
                }
                if (dce2_stats.co_bind_nack > 0)
                    _dpd.logMsg("          Bind Nack: "STDu64"\n", dce2_stats.co_bind_nack);
                if ((dce2_stats.co_request > 0) || (dce2_stats.co_response > 0))
                {
                    _dpd.logMsg("          Request: "STDu64"\n", dce2_stats.co_request);
                    _dpd.logMsg("          Response: "STDu64"\n", dce2_stats.co_response);
                }
                if (dce2_stats.co_fault > 0)
                    _dpd.logMsg("          Fault: "STDu64"\n", dce2_stats.co_fault);
                if (dce2_stats.co_reject > 0)
                    _dpd.logMsg("          Reject: "STDu64"\n", dce2_stats.co_reject);
                if (dce2_stats.co_auth3 > 0)
                    _dpd.logMsg("          Auth3: "STDu64"\n", dce2_stats.co_auth3);
                if (dce2_stats.co_shutdown > 0)
                    _dpd.logMsg("          Shutdown: "STDu64"\n", dce2_stats.co_shutdown);
                if (dce2_stats.co_cancel > 0)
                    _dpd.logMsg("          Cancel: "STDu64"\n", dce2_stats.co_cancel);
                if (dce2_stats.co_orphaned > 0)
                    _dpd.logMsg("          Orphaned: "STDu64"\n", dce2_stats.co_orphaned);
                if (dce2_stats.co_ms_pdu > 0)
                    _dpd.logMsg("          Microsoft Request To Send RPC over HTTP: "STDu64"\n", dce2_stats.co_ms_pdu);
                if (dce2_stats.co_other_req > 0)
                    _dpd.logMsg("          Other request type: "STDu64"\n", dce2_stats.co_other_req);
                if (dce2_stats.co_other_resp > 0)
                    _dpd.logMsg("          Other response type: "STDu64"\n", dce2_stats.co_other_resp);
                _dpd.logMsg("        Request fragments: "STDu64"\n", dce2_stats.co_req_fragments);
                if (dce2_stats.co_req_fragments > 0)
                {
                    _dpd.logMsg("          Min fragment size: "STDu64"\n", dce2_stats.co_cli_min_frag_size);
                    _dpd.logMsg("          Max fragment size: "STDu64"\n", dce2_stats.co_cli_max_frag_size);
                    _dpd.logMsg("          Frag reassembled: "STDu64"\n", dce2_stats.co_cli_frag_reassembled);
                }
                _dpd.logMsg("        Response fragments: "STDu64"\n", dce2_stats.co_resp_fragments);
                if (dce2_stats.co_resp_fragments > 0)
                {
                    _dpd.logMsg("          Min fragment size: "STDu64"\n", dce2_stats.co_srv_min_frag_size);
                    _dpd.logMsg("          Max fragment size: "STDu64"\n", dce2_stats.co_srv_max_frag_size);
                    _dpd.logMsg("          Frag reassembled: "STDu64"\n", dce2_stats.co_srv_frag_reassembled);
                }
                _dpd.logMsg("        Client PDU segmented reassembled: "STDu64"\n",
                        dce2_stats.co_cli_seg_reassembled);
                _dpd.logMsg("        Server PDU segmented reassembled: "STDu64"\n",
                        dce2_stats.co_srv_seg_reassembled);
                _dpd.logMsg("      Memory stats (bytes)\n");
                _dpd.logMsg("        Current segmentation buffering: %u\n", dce2_memory.co_seg);
                _dpd.logMsg("        Maximum segmentation buffering: %u\n", dce2_memory.co_seg_max);
                _dpd.logMsg("        Current fragment tracker: %u\n", dce2_memory.co_frag);
                _dpd.logMsg("        Maximum fragment tracker: %u\n", dce2_memory.co_frag_max);
                _dpd.logMsg("        Current context tracking: %u\n", dce2_memory.co_ctx);
                _dpd.logMsg("        Maximum context tracking: %u\n", dce2_memory.co_ctx_max);
            }

            if (dce2_stats.cl_pkts > 0)
            {
                _dpd.logMsg("    Connectionless\n");
                _dpd.logMsg("      Packet stats\n");
                _dpd.logMsg("        Packets: "STDu64"\n", dce2_stats.cl_pkts);
                if ((dce2_stats.cl_request > 0) || (dce2_stats.cl_response > 0))
                {
                    _dpd.logMsg("        Request: "STDu64"\n", dce2_stats.cl_request);
                    _dpd.logMsg("        Response: "STDu64"\n", dce2_stats.cl_response);
                }
                if (dce2_stats.cl_ack > 0)
                    _dpd.logMsg("        Ack: "STDu64"\n", dce2_stats.cl_ack);
                if (dce2_stats.cl_cancel > 0)
                    _dpd.logMsg("        Cancel: "STDu64"\n", dce2_stats.cl_cancel);
                if (dce2_stats.cl_cli_fack > 0)
                    _dpd.logMsg("        Client Fack: "STDu64"\n", dce2_stats.cl_cli_fack);
                if (dce2_stats.cl_ping > 0)
                    _dpd.logMsg("        Ping: "STDu64"\n", dce2_stats.cl_ping);
                if (dce2_stats.cl_reject > 0)
                    _dpd.logMsg("        Reject: "STDu64"\n", dce2_stats.cl_reject);
                if (dce2_stats.cl_cancel_ack > 0)
                    _dpd.logMsg("        Cancel Ack: "STDu64"\n", dce2_stats.cl_cancel_ack);
                if (dce2_stats.cl_srv_fack > 0)
                    _dpd.logMsg("        Server Fack: "STDu64"\n", dce2_stats.cl_srv_fack);
                if (dce2_stats.cl_fault > 0)
                    _dpd.logMsg("        Fault: "STDu64"\n", dce2_stats.cl_fault);
                if (dce2_stats.cl_nocall > 0)
                    _dpd.logMsg("        NoCall: "STDu64"\n", dce2_stats.cl_nocall);
                if (dce2_stats.cl_working > 0)
                    _dpd.logMsg("        Working: "STDu64"\n", dce2_stats.cl_working);
                if (dce2_stats.cl_other_req > 0)
                    _dpd.logMsg("        Other request type: "STDu64"\n", dce2_stats.cl_other_req);
                if (dce2_stats.cl_other_resp > 0)
                    _dpd.logMsg("        Other response type: "STDu64"\n", dce2_stats.cl_other_resp);
                _dpd.logMsg("        Fragments: "STDu64"\n", dce2_stats.cl_fragments);
                _dpd.logMsg("        Max fragment size: "STDu64"\n", dce2_stats.cl_max_frag_size);
                _dpd.logMsg("        Reassembled: "STDu64"\n", dce2_stats.cl_frag_reassembled);
                if (dce2_stats.cl_max_seqnum > 0)
                    _dpd.logMsg("        Max seq num: "STDu64"\n", dce2_stats.cl_max_seqnum);
                _dpd.logMsg("      Memory stats (bytes)\n");
                _dpd.logMsg("        Current activity tracker: %u\n", dce2_memory.cl_act);
                _dpd.logMsg("        Maximum activity tracker: %u\n", dce2_memory.cl_act_max);
                _dpd.logMsg("        Current fragment tracker: %u\n", dce2_memory.cl_frag);
                _dpd.logMsg("        Maximum fragment tracker: %u\n", dce2_memory.cl_frag_max);
            }
        }
    }

    /* Have to free it here because CleanExit is called before stats functions
     * (so anything flushed by stream can go through and count towards stats) */
    if (exiting)
        DCE2_StatsFree();

    _dpd.logMsg("\n");
    _dpd.logMsg("  Memory stats (bytes)\n");
    _dpd.logMsg("    Current total: %u\n", dce2_memory.total);
    _dpd.logMsg("    Maximum total: %u\n", dce2_memory.total_max);
    _dpd.logMsg("    Current runtime total: %u\n", dce2_memory.rtotal);
    _dpd.logMsg("    Maximum runtime total: %u\n", dce2_memory.rtotal_max);
    _dpd.logMsg("    Current config total: %u\n", dce2_memory.config);
    _dpd.logMsg("    Maximum config total: %u\n", dce2_memory.config_max);
    _dpd.logMsg("    Current rule options total: %u\n", dce2_memory.roptions);
    _dpd.logMsg("    Maximum rule options total: %u\n", dce2_memory.roptions_max);
    _dpd.logMsg("    Current routing table total: %u\n", dce2_memory.rt);
    _dpd.logMsg("    Maximum routing table total: %u\n", dce2_memory.rt_max);
    _dpd.logMsg("    Current initialization total: %u\n", dce2_memory.init);
    _dpd.logMsg("    Maximum initialization total: %u\n", dce2_memory.init_max);
}

uint32_t dce_total_memcap(void)
{
    DCE2_Config *pDefaultPolicyConfig = NULL;
    if (dce2_config)
    { 
         pDefaultPolicyConfig = (DCE2_Config *)sfPolicyUserDataGetDefault(dce2_config);
         return pDefaultPolicyConfig->gconfig->memcap;
    }
    return 0;
}

uint32_t dce_free_total_memcap(void)
{
    if (dce2_config)
    {
         return dce_total_memcap() - dce2_memory.total;
    }
    return 0;
}

int dce_print_mem_stats(FILE *fd, char* buffer, PreprocMemInfo *meminfo)
{
    time_t curr_time = time(NULL);
    int len = 0;
    size_t total_heap_memory = meminfo[PP_MEM_CATEGORY_SESSION].used_memory 
                              + meminfo[PP_MEM_CATEGORY_CONFIG].used_memory 
                              + meminfo[PP_MEM_CATEGORY_MISC].used_memory;
    if (fd)
    {
        len = fprintf(fd, ","STDu64","STDu64","STDu64""
                       ",%u,%u,%u,%u"
                       ","STDu64",%u,%u,%u,%u"
                       ","STDu64",%u,%u,%u,%u"
                       ","STDu64","STDu64",%u,%u,%u,%u"
                       ",%lu,%u,%u"
                       ",%lu,%u,%u"
                       ",%lu,%u,%u,%lu"
                       , dce2_stats.sessions 
                       , dce2_stats.sessions_active
                       , dce2_stats.smb_sessions
                       , dce2_memory.smb_total
                       , dce2_memory.smb_total_max
                       , dce2_memory.smb_ssn
                       , dce2_memory.smb_ssn_max 
                       , dce2_stats.tcp_sessions
                       , dce2_memory.tcp_total
                       , dce2_memory.tcp_total_max
                       , dce2_memory.tcp_ssn
                       , dce2_memory.tcp_ssn_max
                       , dce2_stats.udp_sessions
                       , dce2_memory.udp_total
                       , dce2_memory.udp_total_max
                       , dce2_memory.udp_ssn
                       , dce2_memory.udp_ssn_max
                       , dce2_stats.http_server_sessions 
                       , dce2_stats.http_proxy_sessions
                       , dce2_memory.http_total
                       , dce2_memory.http_total_max
                       , dce2_memory.http_ssn
                       , dce2_memory.http_ssn_max
                       , meminfo[PP_MEM_CATEGORY_SESSION].used_memory
                       , meminfo[PP_MEM_CATEGORY_SESSION].num_of_alloc
                       , meminfo[PP_MEM_CATEGORY_SESSION].num_of_free
                       , meminfo[PP_MEM_CATEGORY_CONFIG].used_memory
                       , meminfo[PP_MEM_CATEGORY_CONFIG].num_of_alloc
                       , meminfo[PP_MEM_CATEGORY_CONFIG].num_of_free
                       , meminfo[PP_MEM_CATEGORY_MISC].used_memory
                       , meminfo[PP_MEM_CATEGORY_MISC].num_of_alloc
                       , meminfo[PP_MEM_CATEGORY_MISC].num_of_free
                       , total_heap_memory);
       return len;
    }

    if (buffer)
    {
        len = snprintf(buffer, CS_STATS_BUF_SIZE, "\n\nMemory Statistics for DCE at: %s\n"
        "dcerpc2 Preprocessor Statistics:\n"
        "                  Total sessions :  "STDu64"\n"
        "                 Active sessions :  "STDu64"\n"
        "              Total SMB sessions :  "STDu64"\n"
        "              Total TCP sessions :  "STDu64"\n"
        "              Total UDP sessions :  "STDu64"\n"
        "      Total HTTP server sessions :  "STDu64"\n"
        "       Total HTTP proxy sessions :  "STDu64"\n"
        "\nTotal Memory stats :\n"
        "                  Current memory :  %u\n"
        "                  Maximum memory :  %u\n"
        "                    Total memcap :  %u\n"
        "                     Free memory :  %u\n"
        "\nSMB Memory stats :\n"
        "                  Current memory :  %u\n"
        "                  Maximum memory :  %u\n"
        "            Current session data :  %u\n"
        "            Maximum session data :  %u\n"
        "  Current segmentation buffering :  %u\n"
        "  Maximum segmentation buffering :  %u\n"
        "\nTCP Memory stats :\n"
        "                  Current memory :  %u\n"
        "                  Maximum memory :  %u\n"
        "            Current session data :  %u\n"
        "            Maximum session data :  %u\n"
        "\nUDP Memory stats :\n"
        "                  Current memory :  %u\n"
        "                  Maximum memory :  %u\n"
        "            Current session data :  %u\n"
        "            Maximum session data :  %u\n"
        "\nHTTP Memory stats :\n"
        "                  Current memory :  %u\n"
        "                  Maximum memory :  %u\n"
        "            Current session data :  %u\n"
        "            Maximum session data :  %u\n"
        , ctime(&curr_time)
        , dce2_stats.sessions
        , dce2_stats.sessions_active
        , dce2_stats.smb_sessions
        , dce2_stats.tcp_sessions
        , dce2_stats.udp_sessions
        , dce2_stats.http_server_sessions
        , dce2_stats.http_proxy_sessions
        , dce2_memory.total
        , dce2_memory.total_max
        , dce_total_memcap()
        , dce_free_total_memcap()
        , dce2_memory.smb_total
        , dce2_memory.smb_total_max
        , dce2_memory.smb_ssn
        , dce2_memory.smb_ssn_max
        , dce2_memory.smb_seg
        , dce2_memory.smb_seg_max
        , dce2_memory.tcp_total
        , dce2_memory.tcp_total_max
        , dce2_memory.tcp_ssn
        , dce2_memory.tcp_ssn_max
        , dce2_memory.udp_total
        , dce2_memory.udp_total_max
        , dce2_memory.udp_ssn
        , dce2_memory.udp_ssn_max
        , dce2_memory.http_total
        , dce2_memory.http_total_max
        , dce2_memory.http_ssn
        , dce2_memory.http_ssn_max);
    }
    else 
    {
        _dpd.logMsg("\n");
        _dpd.logMsg("Memory Statistics of DCE at: %s\n",ctime(&curr_time));
        _dpd.logMsg("dcerpc2 Preprocessor Statistics:\n");
        _dpd.logMsg("                Total sessions :    "STDu64"\n", dce2_stats.sessions);
        _dpd.logMsg("               Active sessions :    "STDu64"\n", dce2_stats.sessions_active);
        _dpd.logMsg("            Total SMB sessions :    "STDu64"\n", dce2_stats.smb_sessions);
        _dpd.logMsg("            Total TCP sessions :    "STDu64"\n", dce2_stats.tcp_sessions);
        _dpd.logMsg("            Total UDP sessions :    "STDu64"\n", dce2_stats.udp_sessions);
        _dpd.logMsg("    Total HTTP server sessions :    "STDu64"\n", dce2_stats.http_server_sessions);
        _dpd.logMsg("     Total HTTP proxy sessions :    "STDu64"\n", dce2_stats.http_proxy_sessions);
        _dpd.logMsg("Total Memory stats :\n");
        _dpd.logMsg("                 Current total :    %u\n", dce2_memory.total);
        _dpd.logMsg("                 Maximum total :    %u\n", dce2_memory.total_max);
        _dpd.logMsg("                  Total memcap :    %u\n", dce_total_memcap());
        _dpd.logMsg("                    Free total :    %u\n", dce_free_total_memcap());
        _dpd.logMsg("SMB Memory stats :\n");
        _dpd.logMsg("                 Current total :    %u\n", dce2_memory.smb_total);
        _dpd.logMsg("                 Maximum total :    %u\n", dce2_memory.smb_total_max);
        _dpd.logMsg("          Current session data :    %u\n", dce2_memory.smb_ssn);
        _dpd.logMsg("          Maximum session data :    %u\n", dce2_memory.smb_ssn_max);
        _dpd.logMsg("   Current segmentation buffer :    %u\n", dce2_memory.smb_seg);
        _dpd.logMsg("   Maximum segmentation buffer :    %u\n", dce2_memory.smb_seg_max);
        _dpd.logMsg("TCP Memory stats :\n");
        _dpd.logMsg("                 Current total :    %u\n", dce2_memory.tcp_total);
        _dpd.logMsg("                 Maximum total :    %u\n", dce2_memory.tcp_total_max);
        _dpd.logMsg("          Current session data :    %u\n", dce2_memory.tcp_ssn);
        _dpd.logMsg("          Maximum session data :    %u\n", dce2_memory.tcp_ssn_max);
        _dpd.logMsg("UDP Memory stats :\n");	
        _dpd.logMsg("                 Current total :    %u\n", dce2_memory.udp_total);
        _dpd.logMsg("                 Maximum total :    %u\n", dce2_memory.udp_total_max);
        _dpd.logMsg("          Current session data :    %u\n", dce2_memory.udp_ssn);
        _dpd.logMsg("          Maximum session data :    %u\n", dce2_memory.udp_ssn_max);
        _dpd.logMsg("HTTP Memory stats :\n");
        _dpd.logMsg("                 Current total :    %u\n", dce2_memory.http_total);
        _dpd.logMsg("                 Maximum total :    %u\n", dce2_memory.http_total_max);
        _dpd.logMsg("          Current session data :    %u\n", dce2_memory.http_ssn);
        _dpd.logMsg("          Maximum session data :    %u\n", dce2_memory.http_ssn_max);
    }
    return len;
}

/******************************************************************
 * Function: DCE2_Reset()
 *
 * Purpose: Reset the preprocessor to a post configuration state.
 *
 * Arguments:
 *  int - signal that caused the reset
 *  void * - pointer to data
 *
 * Returns: None
 *
 ******************************************************************/
static void DCE2_Reset(int signal, void *data)
{
    if (!DCE2_CStackIsEmpty(dce2_pkt_stack))
    {
        DCE2_Log(DCE2_LOG_TYPE__ERROR,
                 "%s(%d) Packet stack is not empty when it should be.",
                 __FILE__, __LINE__);

        DCE2_CStackEmpty(dce2_pkt_stack);
    }
}

/******************************************************************
 * Function: DCE2_ResetStats()
 *
 * Purpose: Reset any statistics being kept by the preprocessor.
 *
 * Arguments:
 *  int - signal that caused function to be called
 *  void * - pointer to data
 *
 * Returns: None
 *
 ******************************************************************/
static void DCE2_ResetStats(int signal, void *data)
{
    DCE2_StatsInit();
}

/******************************************************************
 * Function: DCE2_CleanExit()
 *
 * Purpose: Do any cleanup necessary when Snort exits.
 *
 * Arguments:
 *  int - signal that caused Snort to exit
 *  void * - pointer to data
 *
 * Returns: None
 *
 ******************************************************************/
static void DCE2_CleanExit(int signal, void *data)
{
    DCE2_FreeConfigs(dce2_config);
    dce2_config = NULL;
#ifdef SNORT_RELOAD
    ada_delete( ada );
    ada = NULL;
#endif

    DCE2_FreeGlobals();
    DCE2_Smb2Close();
}

#ifdef SNORT_RELOAD
/*********************************************************************
 * Function: DCE2_ReloadGlobal()
 *
 * Purpose: Creates a new global DCE/RPC preprocessor config.
 *
 * Arguments: snort.conf argument line for the DCE/RPC preprocessor.
 *
 * Returns: None
 *
 *********************************************************************/
static void DCE2_ReloadGlobal(struct _SnortConfig *sc, char *args, void **new_config)
{
    tSfPolicyUserContextId dce2_swap_config = (tSfPolicyUserContextId)*new_config;
    tSfPolicyId policy_id = _dpd.getParserPolicy(sc);
    DCE2_Config *pDefaultPolicyConfig = NULL;
    DCE2_Config *pCurrentPolicyConfig = NULL;

    if ((_dpd.streamAPI == NULL) || (_dpd.streamAPI->version != STREAM_API_VERSION5))
    {
        DCE2_Die("%s(%d) \"%s\" configuration: "
            "Stream must be enabled with TCP and UDP tracking.",
            *_dpd.config_file, *_dpd.config_line, DCE2_GNAME);
    }

    if (dce2_swap_config == NULL)
    {
        //create a context
        dce2_swap_config = sfPolicyConfigCreate();
        dce2_file_cache_was_enabled = !DCE2_IsFileCache(NULL);
        dce2_file_cache_is_enabled = false;
        dce2_ada_is_enabled = false;
        dce2_ada_was_enabled = ada != NULL;
        if (dce2_swap_config == NULL)
        {
            DCE2_Die("%s(%d) \"%s\" configuration: Could not allocate memory "
                     "configuration.\n",
                     *_dpd.config_file, *_dpd.config_line, DCE2_GNAME);
        }
        *new_config = (void *)dce2_swap_config;
    }

    sfPolicyUserPolicySet(dce2_swap_config, policy_id);
    pDefaultPolicyConfig = (DCE2_Config *)sfPolicyUserDataGetDefault(dce2_swap_config);
    pCurrentPolicyConfig = (DCE2_Config *)sfPolicyUserDataGetCurrent(dce2_swap_config);

    if ((policy_id != 0) && (pDefaultPolicyConfig == NULL))
    {
        DCE2_Die("%s(%d) \"%s\" configuration: Must configure default policy "
                 "if other policies are to be configured.\n",
                 *_dpd.config_file, *_dpd.config_line, DCE2_GNAME);
    }

    /* Can only do one global configuration */
    if (pCurrentPolicyConfig != NULL)
    {
        DCE2_Die("%s(%d) \"%s\" configuration: Only one global configuration can be specified.",
                 *_dpd.config_file, *_dpd.config_line, DCE2_GNAME);
    }

    DCE2_RegRuleOptions(sc);

    pCurrentPolicyConfig = (DCE2_Config *)DCE2_Alloc(sizeof(DCE2_Config),
        DCE2_MEM_TYPE__CONFIG);

    sfPolicyUserDataSetCurrent(dce2_swap_config, pCurrentPolicyConfig);

    /* Parse configuration args */
    DCE2_GlobalConfigure(pCurrentPolicyConfig, args);

    if ( pCurrentPolicyConfig->gconfig->disabled )
        return;

    _dpd.addPreproc(sc, DCE2_Main, PRIORITY_APPLICATION, PP_DCE2,
        PROTO_BIT__TCP | PROTO_BIT__UDP);

#ifdef TARGET_BASED
    _dpd.streamAPI->set_service_filter_status
        (sc, dce2_proto_ids.dcerpc, PORT_MONITOR_SESSION, policy_id, 1);

    _dpd.streamAPI->set_service_filter_status
        (sc, dce2_proto_ids.nbss, PORT_MONITOR_SESSION, policy_id, 1);
#endif

    if (policy_id != 0)
        pCurrentPolicyConfig->gconfig->memcap = pDefaultPolicyConfig->gconfig->memcap;

    if (!ada)
    {
        size_t memcap = DCE2_GetReloadSafeMemcap(dce2_swap_config);
        ada = ada_init(DCE2_MemInUse, PP_DCE2, memcap);
        if (!ada)
            _dpd.fatalMsg("Failed to initialize DCE ADA session cache.\n");
    }
    dce2_ada_is_enabled = true;
}

/*********************************************************************
 * Function: DCE2_ReloadServer()
 *
 * Purpose: Creates a new DCE/RPC server configuration
 *
 * Arguments: snort.conf argument line for the DCE/RPC preprocessor.
 *
 * Returns: None
 *
 *********************************************************************/
static void DCE2_ReloadServer(struct _SnortConfig *sc, char *args, void **new_config)
{
    tSfPolicyUserContextId dce2_swap_config;
    tSfPolicyId policy_id = _dpd.getParserPolicy(sc);
    DCE2_Config *pPolicyConfig = NULL;

    dce2_swap_config = (tSfPolicyUserContextId)_dpd.getRelatedReloadData(sc, DCE2_GNAME);

    if (dce2_swap_config != NULL)
    {
        sfPolicyUserPolicySet (dce2_swap_config, policy_id);
        pPolicyConfig = (DCE2_Config *)sfPolicyUserDataGetCurrent(dce2_swap_config);
    }

    if ((dce2_swap_config == NULL) || (pPolicyConfig == NULL)
            || (pPolicyConfig->gconfig == NULL))
    {
        DCE2_Die("%s(%d) \"%s\" configuration: \"%s\" must be configured "
                 "before \"%s\".", *_dpd.config_file, *_dpd.config_line,
                 DCE2_SNAME, DCE2_GNAME, DCE2_SNAME);
    }

    /* Parse configuration args */
    DCE2_ServerConfigure(sc, pPolicyConfig, args);
}

static int DCE2_ReloadVerifyPolicy(
        struct _SnortConfig *sc,
        tSfPolicyUserContextId config,
        tSfPolicyId policyId,
        void* pData
        )
{
    int rval;
    DCE2_Config *swap_config = (DCE2_Config *)pData;
    DCE2_Config *current_config = (DCE2_Config *)sfPolicyUserDataGet(dce2_config, policyId);
    DCE2_ServerConfig *dconfig;

    //do any housekeeping before freeing DCE2_Config

    if ( swap_config == NULL || swap_config->gconfig->disabled )
        return 0;

    if (!_dpd.isPreprocEnabled(sc, PP_STREAM))
    {
        DCE2_Log(DCE2_LOG_TYPE__WARN, "%s(%d) \"%s\" configuration: "
            "Stream must be enabled with TCP and UDP tracking.",
            *_dpd.config_file, *_dpd.config_line, DCE2_GNAME);
        return -1;
    }

    dconfig = swap_config->dconfig;

    if (dconfig == NULL)
    {
        if ((rval = DCE2_CreateDefaultServerConfig(sc, swap_config, policyId)))
            return rval;
    }

#ifdef TARGET_BASED
    if (!_dpd.isAdaptiveConfiguredForSnortConfig(sc))
#endif
    {
        if ((rval = DCE2_ScCheckTransports(swap_config)))
            return rval;
    }

    DCE2_AddPortsToPaf(sc, swap_config, policyId);
#ifdef TARGET_BASED
    DCE2_PafRegisterService(sc, dce2_proto_ids.nbss, policyId, DCE2_TRANS_TYPE__SMB);
    DCE2_PafRegisterService(sc, dce2_proto_ids.dcerpc, policyId, DCE2_TRANS_TYPE__TCP);
#endif

#ifdef DCE2_LOG_EXTRA_DATA
    swap_config->xtra_logging_smb_file_name_id =
        _dpd.streamAPI->reg_xtra_data_cb(DCE2_LogSmbFileName);
#endif

    /* Register routing table memory */
    if (swap_config->sconfigs != NULL)
        DCE2_RegMem(sfrt_usage(swap_config->sconfigs), DCE2_MEM_TYPE__RT);

    if (!swap_config->gconfig->legacy_mode)
    {
        DCE2_Smb2Init(swap_config->gconfig->memcap);
        dce2_file_cache_is_enabled = true;
    }

    if (current_config == NULL)
        return 0;

    return 0;
}

/*********************************************************************
 * Function: DCE2_ReloadVerify()
 * WARNING This runs in a thread that is Asynchronous with the
 * packet loop thread
 *
 * Purpose: Verifies a reloaded DCE/RPC preprocessor configuration
 *
 * Arguments: None
 *
 * Returns:
 *  int
 *      -1 if changed configuration value requires a restart
 *       0 if configuration is ok
 *
 *********************************************************************/
static int DCE2_ReloadVerify(struct _SnortConfig *sc, void *swap_config)
{
    tSfPolicyUserContextId dce2_swap_config = (tSfPolicyUserContextId)swap_config;

    if ((dce2_swap_config == NULL) || (dce2_config == NULL))
        return 0;

    if (sfPolicyUserDataIterate(sc, dce2_swap_config, DCE2_ReloadVerifyPolicy) != 0)
    {
        return -1;
    }

    tSfPolicyId policy_id = _dpd.getParserPolicy(sc);
    /* Look in DCE2_InitGlobal and DCE2_ReloadGlobal to find that DefaultPolicy can't be NULL
    *  You'll also find that all memcaps are synched to DefaultPolicy as the memcap is global
    *  to all policies
    *  Here we are setting up the work that needs to be done to adjust to the new configuration
    */
    uint32_t current_memcap     = DCE2_GetReloadSafeMemcap(dce2_config);
    uint32_t new_memcap         = DCE2_GetReloadSafeMemcap(dce2_swap_config);

    if (dce2_ada_was_enabled && !dce2_ada_is_enabled)
    {
        ada_set_new_cap(ada, 0);
        _dpd.reloadAdjustRegister(sc, "dce2-mem-reloader", policy_id, DCE2_ReloadAdjust, NULL, NULL);
    }
    else if (new_memcap != current_memcap ||
        (dce2_file_cache_was_enabled && !dce2_file_cache_is_enabled))
    {
        ada_set_new_cap(ada, (size_t)(new_memcap));
        _dpd.reloadAdjustRegister(sc, "dce2-mem-reloader", policy_id, DCE2_ReloadAdjust, NULL, NULL);
    }

    return 0;
}

/*********************************************************************
 * Function: DCE2_ReloadAdjust
 *
 * Purpose: After reloading with a different configuration that
 * that requres work to adapt, this function will perform the
 * work in small bursts.
 *
 * Arguments:   idle - if snort is idling (low packets) do more work
 *              raPolicyId - identifies which policy the config is for
 *              userData - data needed by this function
 *
 * Returns:
 *  bool
 *      return false if it needs to work more
 *      return true if there is no work left
 *
 *********************************************************************/
static bool DCE2_ReloadAdjust(bool idle, tSfPolicyId raPolicyId, void *userData)
{
    /* delete files until the file cache memcaps are met
     * delete file cache if not needed
     * delete dce session data from sessions until dce global memcap is met
     * delete ada cache if not needed
     * return true when completed successfully, else false
    */
    int maxWork = idle ? 512 : 32;
    bool memcaps_satisfied =    DCE2_Smb2AdjustFileCache(maxWork, dce2_file_cache_is_enabled) &&
                                ada_reload_adjust_func(idle, raPolicyId, (void *) ada);
    if (memcaps_satisfied && dce2_ada_was_enabled && !dce2_ada_is_enabled)
    {
        ada_delete(ada);
        ada = NULL;
    }
#ifdef REG_TEST
    if (memcaps_satisfied && REG_TEST_FLAG_RELOAD & getRegTestFlags())
        printf("dcerpc2-reload reload-adjust %d %d \n", ada != NULL, !DCE2_IsFileCache(NULL));
#endif
    return memcaps_satisfied;
}

static int DCE2_ReloadSwapPolicy(
        tSfPolicyUserContextId config,
        tSfPolicyId policyId,
        void* pData
        )
{
    DCE2_Config *pPolicyConfig = (DCE2_Config *)pData;

    //do any housekeeping before freeing config
    if (pPolicyConfig->ref_count == 0)
    {
        sfPolicyUserDataClear (config, policyId);
        DCE2_FreeConfig(pPolicyConfig);
    }
    return 0;
}

/*********************************************************************
 * Function: DCE2_ReloadSwap()
 *
 * Purpose: Swaps a new config for the old one.
 *
 * Arguments: None
 *
 * Returns: None
 *
 *********************************************************************/
static void * DCE2_ReloadSwap(struct _SnortConfig *sc, void *swap_config)
{
    tSfPolicyUserContextId dce2_swap_config = (tSfPolicyUserContextId)  swap_config;
    tSfPolicyUserContextId old_config       =                           dce2_config;

    if (dce2_swap_config == NULL)
        return NULL;

    /* Set file cache's new memcaps so it doesn't misbehave
    *  Ensure that the correct amount of memory is registered
    *  See DCE2_Smb2Init for inspiration
    */
    uint32_t current_memcap = 0;
    if (dce2_file_cache_was_enabled)
        current_memcap = DCE2_GetReloadSafeMemcap(dce2_config);
    uint32_t swap_memcap = 0;
    if (dce2_file_cache_is_enabled)
        swap_memcap    = DCE2_GetReloadSafeMemcap(dce2_swap_config);

    DCE2_SetSmbMemcap((uint64_t) swap_memcap >> 1);

    if (dce2_file_cache_was_enabled)
    {
        DCE2_UnRegMem(current_memcap >> 1, DCE2_MEM_TYPE__SMB_SSN);
        if (dce2_file_cache_is_enabled)
            DCE2_RegMem(swap_memcap >> 1, DCE2_MEM_TYPE__SMB_SSN);
    }

    dce2_config = dce2_swap_config;
    sfPolicyUserDataFreeIterate (old_config, DCE2_ReloadSwapPolicy);
    if (sfPolicyUserPolicyGetActive(old_config) == 0)
        return (void *)old_config;

    return NULL;
}

static void DCE2_ReloadSwapFree(void *data)
{
    if (data == NULL)
        return;

    DCE2_FreeConfigs((tSfPolicyUserContextId)data);
}
#endif

// Used for iterate function below since we can't pass it
static tSfPolicyId dce2_paf_tmp_policy_id = 0;

/*********************************************************************
 * Function: DCE2_AddPortsToPaf()
 *
 * Add detect and autodetect ports to stream5 paf
 *
 * Arguments:
 *  DCE2_Config *
 *      Pointer to configuration structure.
 *
 * Returns: None
 *
 *********************************************************************/
static void DCE2_AddPortsToPaf(struct _SnortConfig *sc, DCE2_Config *config, tSfPolicyId policy_id)
{
    if (config == NULL)
        return;

    dce2_paf_tmp_policy_id = policy_id;

    DCE2_ScAddPortsToPaf(sc, config->dconfig);

    if (config->sconfigs != NULL)
        sfrt_iterate_with_snort_config(sc, config->sconfigs, DCE2_ScAddPortsToPaf);

    dce2_paf_tmp_policy_id = 0;
}

static void DCE2_ScAddPortsToPaf(struct _SnortConfig *snortConf, void *data)
{
    DCE2_ServerConfig *sc = (DCE2_ServerConfig *)data;
    unsigned int port;
    tSfPolicyId policy_id = dce2_paf_tmp_policy_id;

    if (data == NULL)
        return;

    for (port = 0; port < DCE2_PORTS__MAX; port++)
    {
        if (DCE2_IsPortSet(sc->smb_ports, (uint16_t)port))
        {
            DCE2_PafRegisterPort(snortConf, (uint16_t)port, policy_id, DCE2_TRANS_TYPE__SMB);
        }

        if (DCE2_IsPortSet(sc->auto_smb_ports, (uint16_t)port))
        {
            DCE2_PafRegisterPort(snortConf, (uint16_t)port, policy_id, DCE2_TRANS_TYPE__SMB);
        }

        if (DCE2_IsPortSet(sc->tcp_ports, (uint16_t)port))
        {
            DCE2_PafRegisterPort(snortConf, (uint16_t)port, policy_id, DCE2_TRANS_TYPE__TCP);
        }

        if (DCE2_IsPortSet(sc->auto_tcp_ports, (uint16_t)port))
        {
            DCE2_PafRegisterPort(snortConf, (uint16_t)port, policy_id, DCE2_TRANS_TYPE__TCP);
        }

#if 0
        if (DCE2_IsPortSet(sc->http_proxy_ports, (uint16_t)port))
        {
            /* TODO Implement PAF registration and callback. */
        }

        if (DCE2_IsPortSet(sc->http_server_ports, (uint16_t)port))
        {
            /* TODO Implement PAF registration and callback. */
        }
#endif
    }
}

static uint32_t max(uint32_t a, uint32_t b)
{
    if (a >= b)
        return a;
    return b;
}

/*********************************************************************
 * Function: DCE2_GetReloadSafeMemcap
 *
 * Purpose: Provide a safe memcap that can be used durring
 * packet processing.
 * This is based on how the memcaps are set in InitGlobal and
 * ReloadGlobal
 *
 * The memcap in the config for the policyId currently running is
 * what is used as a memcap, according to DCE2_Process calling 
 * DCE2_GcMemcap.
 *
 * Based on InitGlobal and ReloadGlobal all memcaps are equal to
 * the memcap in the default policy except for policy_id 0
 *
 * So to be safe we'll just return the max of the two.
 *
 * This function also helps to prevent segmentation faults caused
 * by accessing null pointers.
 *
 * Arguments: pConfig - mapping between policyIDs and configs
 *
 * Returns: A safe memcap
 *
 *********************************************************************/
static uint32_t DCE2_GetReloadSafeMemcap(tSfPolicyUserContextId pConfig)
{
  DCE2_Config *pDefaultPolicyConfig = sfPolicyUserDataGetDefault(pConfig);
  DCE2_Config *pPolicyConfig        = sfPolicyUserDataGet(pConfig, 0);
  uint32_t defaultMem = pDefaultPolicyConfig == NULL ?
                        0 : pDefaultPolicyConfig->gconfig->memcap;
  uint32_t policyMem  = pPolicyConfig == NULL ?
                        0 : pPolicyConfig->gconfig->memcap;

  return max(defaultMem,policyMem);
}
