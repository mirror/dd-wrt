/*
** Copyright (C) 2014 Cisco and/or its affiliates. All rights reserved.
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
#include "appInfoTable.h"

#ifdef PERF_PROFILING
PreprocStats appMatchPerfStats;
#endif

const int MAJOR_VERSION = 1;
const int MINOR_VERSION = 1;
const int BUILD_VERSION = 4;

static uint16_t appid_preproc_status_bit = 0;


SO_PUBLIC const char *PREPROC_NAME = "APPID";

static void AppIdProcess(SFSnortPacket *p, void *context)
{
    PROFILE_VARS;
    PREPROC_PROFILE_START(appMatchPerfStats);
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

#ifdef SNORT_RELOAD
static void AppIdReload(struct _SnortConfig *sc, char *args, void **new_config)
{
    tSfPolicyId policy_id = _dpd.getParserPolicy(sc);

    _dpd.addPreproc(sc, (void (*)(void *, void *))AppIdProcess, (PRIORITY_TRANSPORT + 1), PP_APP_ID, PROTO_BIT__ALL);
    _dpd.sessionAPI->enable_preproc_all_ports( sc, PP_APP_ID, PROTO_BIT__ALL );

    if (policy_id == _dpd.getDefaultPolicy())
        AppIdAddPortsToStream5Filter(sc, policy_id);
}

static void *AppIdReloadSwap(struct _SnortConfig *sc, void *swap_config)
{
    AppIdCommonReload();
    return NULL;
}

static int AppIdReconfigure(uint16_t type, void *new_context, void **old_context)
{
    *old_context = NULL;
    AppIdCommonReload();
    _dpd.logMsg( "Reconfigured");
    return 0;
}

static void AppIdReloadFree(void *old_context)
{
}
#endif

void AppIdDumpStats(int exit_flag)
{
    _dpd.logMsg("Application Identification Preprocessor:\n");
    _dpd.logMsg("   Total packets received : %lu\n", app_id_raw_packet_count);
    _dpd.logMsg("  Total packets processed : %lu\n", app_id_processed_packet_count);
    _dpd.logMsg("    Total packets ignored : %lu\n", app_id_ignored_packet_count);
    AppIdServiceStateDumpStats();
    RNAPndDumpLuaStats();
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
}

static void AppIdCleanExit(int signal, void *unused)
{
    AppIdCommonFini();
}

static void AppIdInit(struct _SnortConfig *sc, char *args)
{
    static int once = 0;
    tSfPolicyId policy_id = _dpd.getParserPolicy(sc);

    if (!once)
    {
        _dpd.addPreprocExit(AppIdCleanExit, NULL, PRIORITY_LAST, PP_APP_ID);
#       ifdef PERF_PROFILING
        _dpd.addPreprocProfileFunc("fwApp", &appMatchPerfStats, 0, _dpd.totalPerfStats);
        _dpd.addPreprocProfileFunc("fwClientPat", &clientMatchPerfStats, 1, &appMatchPerfStats);
        _dpd.addPreprocProfileFunc("fwLuaClient", &luaClientPerfStats, 2, &clientMatchPerfStats);
        _dpd.addPreprocProfileFunc("fwHTTP", &httpPerfStats, 2, &clientMatchPerfStats);
        _dpd.addPreprocProfileFunc("fwServicePat", &serviceMatchPerfStats, 1, &appMatchPerfStats);
        _dpd.addPreprocProfileFunc("fwLuaServer", &luaServerPerfStats, 2, &serviceMatchPerfStats);
#       endif
        appid_preproc_status_bit = _dpd.sessionAPI->get_preprocessor_status_bit();
        appIdConfigParse(args);

        AppIdCommonInit(appIdCommandConfig->app_id_memcap);
        if (appIdCommandConfig->app_id_dump_ports)
        {
            dumpPorts(stdout);
            appInfoTableDump();
            exit(0);
        }
        _dpd.addPreprocResetStats(AppIdResetStats, NULL, PRIORITY_LAST, PP_APP_ID);
        _dpd.registerPreprocStats(PREPROC_NAME, AppIdDumpStats);
        /* Hook into control socket to handle reload */
        _dpd.controlSocketRegisterHandler(73, NULL, AppIdReconfigure, NULL);
        _dpd.controlSocketRegisterHandler(74, AppIdDebug, NULL, NULL);

        _dpd.registerIdleHandler(appIdIdleProcessing);
        _dpd.registerGetAppId(getOpenAppId);
        _dpd.streamAPI->register_http_header_callback(httpHeaderCallback);
        if (_dpd.streamAPI->service_event_subscribe(PP_SIP, SIP_EVENT_TYPE_SIP_DIALOG, SipSessionSnortCallback) == false)
            DynamicPreprocessorFatalMessage("failed to subscribe to SIP_DIALOG\n");

        once = 1;
    }

    _dpd.addPreproc(sc, (void (*)(void *, void *))AppIdProcess, (PRIORITY_TRANSPORT + 1), PP_APP_ID, PROTO_BIT__ALL);
    _dpd.sessionAPI->enable_preproc_all_ports( sc, PP_APP_ID, PROTO_BIT__ALL );

    if (policy_id == _dpd.getDefaultPolicy())
        AppIdAddPortsToStream5Filter(sc, policy_id);
}

#define SetupApplicationPreproc DYNAMIC_PREPROC_SETUP

SO_PUBLIC void SetupApplicationPreproc(void)
{
#ifndef SNORT_RELOAD
    _dpd.registerPreproc(PREPROC_NAME, AppIdInit);
#else
    _dpd.registerPreproc(PREPROC_NAME, AppIdInit, AppIdReload, NULL, AppIdReloadSwap, AppIdReloadFree);
#endif
}


