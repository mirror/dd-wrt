/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2005-2013 Sourcefire, Inc.
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

#include <stdint.h>
#include <stdbool.h>
#include <strings.h>
#include <stdio.h>
#include <syslog.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <inttypes.h>
#include <sys/time.h>
#include <pthread.h>

#include "sf_snort_packet.h"
#include "sf_dynamic_preprocessor.h"
#include "common_util.h"
#include "sf_preproc_info.h"

#include "spp_appid.h"
#include "fw_appid.h"
#include "flow.h"
#include "service_base.h"
#include "luaDetectorModule.h"
#include "appIdConfig.h"
#include "appIdStats.h"
#ifdef SIDE_CHANNEL
#include "appId_ss.h"
#endif
#include "appInfoTable.h"
#include "thirdparty_appid_utils.h"

#include "cip_common.h"
#include "detector_cip.h"
#include "memory_stats.h"

#ifdef PERF_PROFILING
PreprocStats appMatchPerfStats;
#endif

const int MAJOR_VERSION = 1;
const int MINOR_VERSION = 1;
const int BUILD_VERSION = 5;

static uint16_t appid_preproc_status_bit = 0;


SF_SO_PUBLIC const char *PREPROC_NAME = "appid";

static pthread_mutex_t appIdReloadMutex = PTHREAD_MUTEX_INITIALIZER;
static bool appIdReloadInProgress = false;

extern void appIdApiInit(struct AppIdApi*);

static void AppIdProcess(SFSnortPacket *p, void *context)
{
    PROFILE_VARS;
    PREPROC_PROFILE_START(appMatchPerfStats);
    /* Trust */
    if (p->stream_session && _dpd.sessionAPI->get_ignore_direction(p->stream_session) == SSN_DIR_BOTH)
    {
        _dpd.sessionAPI->disable_preproc_for_session( p->stream_session, PP_NETWORK_DISCOVERY );
        PREPROC_PROFILE_END(appMatchPerfStats);
        return;
    }
    fwAppIdSearch(p);
    PREPROC_PROFILE_END(appMatchPerfStats);
}

static void AppIdAddPortsToStream5Filter(struct _SnortConfig *sc, tSfPolicyId policy_id)
{
    unsigned portNum;

    for (portNum = 0; portNum < 65536; portNum++)
    {
        /*Add port the port */
        _dpd.streamAPI->set_port_filter_status(sc, IPPROTO_TCP, (uint16_t)portNum,
                                               appid_preproc_status_bit, policy_id, 1);
        _dpd.streamAPI->set_port_filter_status(sc, IPPROTO_UDP, (uint16_t)portNum,
                                               appid_preproc_status_bit, policy_id, 1);
    }
}

static void initializeAppIDForDispatch(struct _SnortConfig *sc)
{
    _dpd.sessionAPI->enable_preproc_all_ports_all_policies(sc, PP_APP_ID, PROTO_BIT__IP);
    _dpd.addPreprocAllPolicies(sc, (void (*)(void *, void *))AppIdProcess, PRIORITY_TRANSPORT + 1,
                               PP_APP_ID, PROTO_BIT__IP);
}

static int AppIDCheckConfig(struct _SnortConfig *sc)
{
    initializeAppIDForDispatch(sc);
    return 0;
}

static void AppIdStaticConfigFree(tAppidStaticConfig* appidSC)
{
    if (appidSC)
    {
        free((char *)(appidSC->appid_thirdparty_dir));
        free(appidSC->tp_config_path);
        free(appidSC->app_id_detector_path);
        free(appidSC->conf_file);
        free(appidSC->app_stats_filename);
#ifdef SIDE_CHANNEL
        if (appidSC->appId_ss_config)
            AppIdSSConfigFree(appidSC->appId_ss_config);
#endif
        if (appidSC->newAppIdConfig)
            AppIdCommonUnload(appidSC->newAppIdConfig);

        _dpd.snortFree(appidSC, sizeof(*appidSC), PP_APP_ID, PP_MEM_CATEGORY_CONFIG);
    }
}

#ifdef SNORT_RELOAD

/********** AppId Reload Functions **********/

static void reloadWait(void)
{
    const struct timespec reloadPollTime = {0, 1000000};    // 1 msec wait time to poll reload status

    for (;;)
    {
        pthread_mutex_lock(&appIdReloadMutex);
        if (!appIdReloadInProgress)
        {
            appIdReloadInProgress = true;
            pthread_mutex_unlock(&appIdReloadMutex);
            return;
        }
        pthread_mutex_unlock(&appIdReloadMutex);
        nanosleep(&reloadPollTime, NULL);
    }
}


static void reloadUnlock(void)
{
    pthread_mutex_lock(&appIdReloadMutex);
    appIdReloadInProgress = false;
    pthread_mutex_unlock(&appIdReloadMutex);
}

STATIC bool AppIdReloadAdjust(bool idle, tSfPolicyId raPolicyId, void* userData)
{
    return AppIdServiceStateReloadAdjust(idle, appidStaticConfig->memcap);
}

static int AppIdReloadReloadVerify(struct _SnortConfig *sc, void *swap_config)
{
    initializeAppIDForDispatch(sc);
    if (swap_config)
    {
        tAppidStaticConfig* newConfig = (tAppidStaticConfig*)swap_config;
        if (newConfig->memcap != appidStaticConfig->memcap)
        {
            _dpd.logMsg("AppId: old memcap %lu, new memcap %lu\n", appidStaticConfig->memcap, newConfig->memcap);
            _dpd.reloadAdjustRegister(sc, "AppID", 0, AppIdReloadAdjust, NULL, NULL);
        }
    }
    return 0;
}

/**
 * \brief Callback function that handles AppId reload
 *
 * This function gets called on Snort reload in a separate thread. It starts
 * loading AppId configuration in new_config.
 *
 * @param sc
 * @param args
 * @param new_config return parameter to hold AppId configuration
 * @return void
 */
STATIC void AppIdReload(struct _SnortConfig *sc, char *args, void **new_config)
{
    tSfPolicyId policy_id;

    reloadWait();

    policy_id = _dpd.getParserPolicy(sc);

    if (policy_id == _dpd.getDefaultPolicy())
        AppIdAddPortsToStream5Filter(sc, policy_id);

    if (*new_config == NULL)
    {
        tAppidStaticConfig* newConfig;
        if (NULL == (newConfig = (tAppidStaticConfig*)_dpd.snortAlloc(1,
                sizeof(*newConfig), PP_APP_ID, PP_MEM_CATEGORY_CONFIG)))
        {
            _dpd.fatalMsg("AppID failed to allocate memory for new configuration\n");
        }
        appIdConfigParse(newConfig, args);
        // Start loading AppId configuration into new_config
        AppIdCommonReload(newConfig, (void**)&newConfig->newAppIdConfig);
        *new_config = (void*)newConfig;
    }
}

/**
 * \brief Callback function that handles configuration swap on reload
 *
 * This function gets called after AppIdReload() returns. At this point,
 * AppIdReload() is done with loading the configuration into swap_config
 * and swap_config is ready to use.
 *
 * @param sc
 * @param swap_config pointer to data structure containing new configuration.
 *        This data structure was populated by AppIdReload().
 * @return pointer to old configuration
 */
STATIC void *AppIdReloadSwap(struct _SnortConfig *sc, void *swap_config)
{
    struct timeval  startTime;
    struct timeval  endTime;
    double          elapsedTime;
    tAppidStaticConfig* tmpConfig = NULL;

    if (swap_config)
    {
        gettimeofday(&startTime, NULL);

        tmpConfig = appidStaticConfig;
        appidStaticConfig = (tAppidStaticConfig*)swap_config;

        tmpConfig->newAppIdConfig = AppIdCommonReloadSwap(appidStaticConfig->newAppIdConfig);
        appidStaticConfig->newAppIdConfig = NULL;
        ThirdPartyAppIDReconfigure();

        gettimeofday(&endTime, NULL);
        elapsedTime = (endTime.tv_sec*1000.0) + (endTime.tv_usec/1000.0) - (startTime.tv_sec*1000.0) - (startTime.tv_usec/1000.0);

        _dpd.logMsg("AppId reload swap time = %.3f msec\n", elapsedTime);
    }

    // Return old configuration data structure
    return (void*)tmpConfig;
}

/**
 * \brief Callback function that handles freeing of old configuration after
 *        configuration is swapped on a reload
 *
 * This function gets called after AppIdReloadSwap() is done. It frees the data
 * structure that contains the old configuration.
 *
 * @param old_context pointer to old configuration
 * @return void
 */
STATIC void AppIdReloadFree(void *old_context)
{
    AppIdStaticConfigFree((tAppidStaticConfig*)old_context);
    reloadUnlock();
}

#endif // SNORT_RELOAD

/******** AppId Reconfigure Functions ********/

/**
 * \brief Callback function that handles AppId reconfiguration
 *
 * This function gets called on AppId reconfiguration in a separate thread. It
 * starts loading AppId configuration into new_context.
 *
 * @param type
 * @param data
 * @param length
 * @param new_context return parameter to hold AppId configuration
 * @param statusBuf
 * @param statusBuf_Len
 * @return 0 on success
 */
STATIC int AppIdReconfigure(uint16_t type, const uint8_t *data, uint32_t length, void **new_context,
                            char* statusBuf, int statusBuf_len)
{
    reloadWait();

    if (*new_context == NULL)
    {
        AppIdCommonReload(appidStaticConfig, new_context);
    }

    return 0;
}

/**
 * \brief Callback function that handles AppId reconfiguration swap
 *
 * This function gets called adter AppIdReconfigure() returns. At this point,
 * AppIdReconfigure() is done with loading the configuration into new_context
 * and new_context is ready to use.
 *
 * @param type
 * @param new_context pointer to data structure that contains AppId's new
 *        configuration. This data structure was populated by AppIdReconfigure().
 * @param old_context return parameter that points to old configuration
 * @return 0 on success
 */
STATIC int AppIdReconfigureSwap(uint16_t type, void *new_context, void **old_context)
{
    struct timeval  startTime;
    struct timeval  endTime;
    double          elapsedTime;

    gettimeofday(&startTime, NULL);

    if (new_context)
    {
        if (*old_context == NULL)
        {
            // Return current configuration in old_context
            *old_context = AppIdCommonReloadSwap(new_context);
            ThirdPartyAppIDReconfigure();
        }
    }

    _dpd.logMsg("AppId", "Reconfigured");

    gettimeofday(&endTime, NULL);
    elapsedTime = (endTime.tv_sec*1000.0) + (endTime.tv_usec/1000.0) - (startTime.tv_sec*1000.0) - (startTime.tv_usec/1000.0);

    _dpd.logMsg("AppId reconfigure swap time = %.3f msec\n", elapsedTime);

    return 0;
}

/**
 * \brief Callback function that handles freeing of old AppId configuration
 *
 * This function gets called after AppIdReconfigureSwap() returns. It frees the
 * data strcuture that contains the old configuration.
 *
 * @param type
 * @param old_context pointer to data structure that contains AppId's old
 *        configuration. This pointer was returned by AppIdReconfigureSwap().
 * @param te
 * @param f
 * @return void
 */
STATIC void AppIdReconfigureFree(uint16_t type, void *old_context, struct _THREAD_ELEMENT *te, ControlDataSendFunc f)
{
    if (old_context)
    {
        AppIdCommonUnload(old_context);
    }

    reloadUnlock();
}

void AppIdDumpStats(int exit_flag)
{
    _dpd.logMsg("Application Identification Preprocessor:\n");
    _dpd.logMsg("   Total packets received : %lu\n", app_id_raw_packet_count);
    _dpd.logMsg("  Total packets processed : %lu\n", app_id_processed_packet_count);
    _dpd.logMsg("    Total packets ignored : %lu\n", app_id_ignored_packet_count);
    _dpd.logMsg("    Total ongoing AppId sessions : %lu\n", app_id_ongoing_session);
    _dpd.logMsg("    Total AppId sessions allocated : %lu\n", app_id_total_alloc);
    _dpd.logMsg("    AppId session size : %lu\n", sizeof(tAppIdData));
    _dpd.logMsg("         Total AppId sessions allocated from heap : %lu\n", app_id_session_heap_alloc_count);
    _dpd.logMsg("Total AppId sessions allocated from AppID Memory Pool : %lu\n", app_id_session_freelist_alloc_count);
    _dpd.logMsg("   AppID session Memory Pool free count : %lu\n", app_id_flow_data_free_list_count);
    _dpd.logMsg("       Flow-data Memory Pool free count : %lu\n", app_id_data_free_list_count);
    _dpd.logMsg("             Tmp Memory Pool free count : %lu\n", app_id_tmp_free_list_count);
    if (exit_flag == 0)    // Snort's SigDumpStatsHandler dumping stats intentionally
    {
        if (thirdparty_appid_module)
            thirdparty_appid_module->print_stats();
        AppIdServiceStateDumpStats();
        RNAPndDumpLuaStats();
#ifdef SIDE_CHANNEL
        AppIdPrintSSStats();
#endif
    }
}

static void appIdIdleProcessing(void)
{
    appIdStatsIdleFlush();
}

static void AppIdResetStats(int signal, void *data)
{
    app_id_raw_packet_count = 0;
    app_id_processed_packet_count = 0;
    app_id_ignored_packet_count = 0;
    app_id_ongoing_session = 0;
    app_id_total_alloc = 0;
    app_id_session_heap_alloc_count = 0;
    app_id_session_freelist_alloc_count = 0;
    app_id_flow_data_free_list_count = 0;
    app_id_data_free_list_count = 0;
    app_id_tmp_free_list_count = 0;
    if (thirdparty_appid_module)
        thirdparty_appid_module->reset_stats();
#ifdef SIDE_CHANNEL
    AppIdResetSSStats();
#endif
}

static void AppIdCleanExit(int signal, void *unused)
{
    AppIdCommonFini();
#ifdef SIDE_CHANNEL
    AppIdCleanSS();
#endif
    AppIdStaticConfigFree(appidStaticConfig);
}

static int ThirdPartyReload(uint16_t type, void *new_context, void **old_context)
{
    if (thirdparty_appid_module != NULL)
    {
        thirdparty_appid_module->print_stats();
    }
    ThirdPartyAppIDFini();
    ThirdPartyAppIDInit(appidStaticConfig);
    return 0;
}

int AppId_Print_Mem_Stats(FILE *fd, char *buffer, PreprocMemInfo *meminfo)
{
    time_t curr_time = time(NULL);
    int len = 0;

    if (fd)
    {
        len = fprintf(fd, ",%lu,%lu,%lu,%lu"
                ",%lu,%lu,%lu"
                ",%lu,%lu,%lu"
                ",%lu,%lu,%lu"
                ",%lu,%u,%u"
                ",%lu,%u,%u"
                ",%lu,%u,%u"
                ",%lu,%u,%u"
                , app_id_total_alloc, app_id_ongoing_session, app_id_session_freelist_alloc_count, app_id_session_heap_alloc_count
                , app_id_data_free_list_count, app_id_data_free_list_count*sizeof(tAppIdData)
                , app_id_flow_data_free_list_count, app_id_flow_data_free_list_count*sizeof(AppIdFlowData)
                , app_id_tmp_free_list_count, app_id_tmp_free_list_count*sizeof(tTmpAppIdData)
				, app_id_raw_packet_count, app_id_processed_packet_count, app_id_ignored_packet_count
                , meminfo[PP_MEM_CATEGORY_SESSION].used_memory, meminfo[PP_MEM_CATEGORY_SESSION].num_of_alloc, meminfo[PP_MEM_CATEGORY_SESSION].num_of_free
                , meminfo[PP_MEM_CATEGORY_CONFIG].used_memory,  meminfo[PP_MEM_CATEGORY_CONFIG].num_of_alloc, meminfo[PP_MEM_CATEGORY_CONFIG].num_of_free
                , meminfo[PP_MEM_CATEGORY_MISC].used_memory, meminfo[PP_MEM_CATEGORY_MISC].num_of_alloc, meminfo[PP_MEM_CATEGORY_MISC].num_of_free
                , meminfo[PP_MEM_CATEGORY_SESSION].used_memory + meminfo[PP_MEM_CATEGORY_CONFIG].used_memory + meminfo[PP_MEM_CATEGORY_MISC].used_memory
                , meminfo[PP_MEM_CATEGORY_SESSION].num_of_alloc + meminfo[PP_MEM_CATEGORY_CONFIG].num_of_alloc + meminfo[PP_MEM_CATEGORY_MISC].num_of_alloc
                , meminfo[PP_MEM_CATEGORY_SESSION].num_of_free + meminfo[PP_MEM_CATEGORY_CONFIG].num_of_free + meminfo[PP_MEM_CATEGORY_MISC].num_of_free);
    }
    else if (buffer)
    {
       /*
       * Old buffer output for control socket comm,
       * like via, "show snort preprocessor-memory-usage"
       * There are no support for the preproc APPID here,
       * Hence keeping this block as empty
       */
    }
    else
    {
        _dpd.logMsg("\n");
        _dpd.logMsg("\n");
        _dpd.logMsg("Memory Statistics of AppID on: %s\n", ctime(&curr_time));
        _dpd.logMsg("        AppID Session Statistics:\n");
        _dpd.logMsg("                    Total Sessions seen: %14lu\n", app_id_total_alloc);
        _dpd.logMsg("                Current Active sessions: %14lu\n", app_id_ongoing_session);
        _dpd.logMsg("Total allocs from MemPool-AppID session: %14lu\n", app_id_session_freelist_alloc_count);
        _dpd.logMsg("                 Total allocs from Heap: %14lu\n", app_id_session_heap_alloc_count);
        _dpd.logMsg("    AppID Memory Pool Statistics:\n");
        _dpd.logMsg("        Memory Pool-AppID session:\n");
        _dpd.logMsg("                        Free count: %14lu\n", app_id_data_free_list_count);
        _dpd.logMsg("                         Free size: %14lu bytes\n", app_id_data_free_list_count*sizeof(tAppIdData));
        _dpd.logMsg("            Memory Pool-Flow data:\n");
        _dpd.logMsg("                        Free count: %14lu\n", app_id_flow_data_free_list_count);
        _dpd.logMsg("                         Free size: %14lu bytes\n", app_id_flow_data_free_list_count*sizeof(AppIdFlowData));
        _dpd.logMsg("                  Memory Pool-Tmp:\n");
        _dpd.logMsg("                        Free count: %14lu \n", app_id_tmp_free_list_count);
        _dpd.logMsg("                         Free size: %14lu bytes\n", app_id_tmp_free_list_count*sizeof(tTmpAppIdData));
        _dpd.logMsg("         AppID Packet Statistics:\n");
        _dpd.logMsg("            Total packets received: %14lu\n", app_id_raw_packet_count);
        _dpd.logMsg("           Total packets processed: %14lu\n", app_id_processed_packet_count);
        _dpd.logMsg("             Total packets ignored: %14lu\n", app_id_ignored_packet_count);
  }
  return len;
}

static void AppIdInit(struct _SnortConfig *sc, char *args)
{
    static int once = 0;
    tSfPolicyId policy_id = _dpd.getParserPolicy(sc);

    _dpd.registerMemoryStatsFunc(PP_APP_ID, AppId_Print_Mem_Stats);

    if (!once)
    {
        _dpd.addPreprocExit(AppIdCleanExit, NULL, PRIORITY_LAST, PP_APP_ID);
#       ifdef PERF_PROFILING
        _dpd.addPreprocProfileFunc("fwApp", &appMatchPerfStats, 0, _dpd.totalPerfStats, NULL);
        _dpd.addPreprocProfileFunc("fwAppTP", &tpPerfStats, 1, &appMatchPerfStats, NULL);
        _dpd.addPreprocProfileFunc("fwLibAppTP", &tpLibPerfStats, 2, &tpPerfStats, NULL);
        _dpd.addPreprocProfileFunc("fwHTTP", &httpPerfStats, 2, &tpPerfStats, NULL);
        _dpd.addPreprocProfileFunc("fwClientPat", &clientMatchPerfStats, 1, &appMatchPerfStats, NULL);
        _dpd.addPreprocProfileFunc("fwServicePat", &serviceMatchPerfStats, 1, &appMatchPerfStats, NULL);
        _dpd.addPreprocProfileFunc("luaDetectors", &luaDetectorsPerfStats, 1, &appMatchPerfStats, NULL);
        _dpd.addPreprocProfileFunc("cisco", &luaCiscoPerfStats, 2, &luaDetectorsPerfStats, NULL);
        _dpd.addPreprocProfileFunc("custom", &luaCustomPerfStats, 2, &luaDetectorsPerfStats, NULL);
#       endif
        appid_preproc_status_bit = _dpd.sessionAPI->get_preprocessor_status_bit();

        if (NULL == (appidStaticConfig = (tAppidStaticConfig*)_dpd.snortAlloc(1,
                sizeof(*appidStaticConfig), PP_APP_ID, PP_MEM_CATEGORY_CONFIG)))
        {
            _dpd.fatalMsg("AppID failed to allocate memory for the configuration\n"); 
        }
        appIdConfigParse(appidStaticConfig, args);

        AppIdCommonInit(appidStaticConfig);
        ThirdPartyAppIDInit(appidStaticConfig);
        if (appidStaticConfig->app_id_dump_ports)
        {
            dumpPorts(stdout, pAppidActiveConfig);
            appInfoTableDump(pAppidActiveConfig);
            exit(0);
        }
        _dpd.addPreprocResetStats(AppIdResetStats, NULL, PRIORITY_LAST, PP_APP_ID);
        _dpd.registerPreprocStats(PREPROC_NAME, AppIdDumpStats);
        /* Hook into control socket to handle reload */
        _dpd.controlSocketRegisterHandler(73, AppIdReconfigure, AppIdReconfigureSwap, AppIdReconfigureFree);
        _dpd.controlSocketRegisterHandler(74, AppIdDebug, NULL, NULL);
        _dpd.controlSocketRegisterHandler(56, NULL, ThirdPartyReload, NULL);

        _dpd.registerIdleHandler(appIdIdleProcessing);
        _dpd.registerGetAppId(getOpenAppId);
        if (!thirdparty_appid_module)
            _dpd.streamAPI->register_http_header_callback(httpHeaderCallback);
        _dpd.registerSslAppIdLookup(sslAppGroupIdLookup);
        if (_dpd.streamAPI->service_event_subscribe(PP_SIP, SIP_EVENT_TYPE_SIP_DIALOG, SipSessionSnortCallback) == false)
            DynamicPreprocessorFatalMessage("failed to subscribe to SIP_DIALOG\n");

        if (_dpd.streamAPI->service_event_subscribe(PP_CIP, CIP_EVENT_TYPE_CIP_DATA, CipSessionSnortCallback) == false)
            DynamicPreprocessorFatalMessage("failed to subscribe to CIP_EVENT_TYPE_CIP_DATA\n");
        
        _dpd.registerSetTlsHostAppId(setTlsHost);

        appIdApiInit(_dpd.appIdApi);
#ifdef SIDE_CHANNEL
        _dpd.addFuncToPostConfigList(sc, AppIdSSPostConfigInit, NULL);
#endif
        once = 1;
    }

    _dpd.addPreprocConfCheck(sc, AppIDCheckConfig);

    if (policy_id == _dpd.getDefaultPolicy())
        AppIdAddPortsToStream5Filter(sc, policy_id);
}

#define SetupApplicationPreproc DYNAMIC_PREPROC_SETUP

SF_SO_PUBLIC void SetupApplicationPreproc(void)
{
#ifndef SNORT_RELOAD
    _dpd.registerPreproc(PREPROC_NAME, AppIdInit);
#else
    _dpd.registerPreproc(PREPROC_NAME, AppIdInit, AppIdReload, AppIdReloadReloadVerify, AppIdReloadSwap, AppIdReloadFree);
#endif
}


