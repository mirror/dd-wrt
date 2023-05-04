/* $Id$
**
**  spp_perfmonitor.c
**
**  Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
**  Copyright (C) 2002-2013 Sourcefire, Inc.
**  Marc Norton <mnorton@sourcefire.com>
**  Dan Roelker <droelker@sourcefire.com>
**
**  NOTES
**  6.4.02 - Initial Source Code.  Norton/Roelker
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License Version 2 as
**  published by the Free Software Foundation.  You may not use, modify or
**  distribute this program under any other version of the GNU General
**  Public License.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**
*/

#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

#ifndef WIN32
# include <unistd.h>
# include <sys/stat.h>
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_types.h"
#include "plugbase.h"
#include "mstring.h"
#include "util.h"
#include "snort_debug.h"
#include "parser.h"
#include "sfdaq.h"
#include "snort.h"
#include "perf.h"
#include "perf-base.h"
#include "profiler.h"
#include "session_api.h"
#include "reload.h"
#include "control/sfcontrol_funcs.h"
#include "control/sfcontrol.h"
#ifdef SNORT_RELOAD
#ifdef REG_TEST
#include "reg_test.h"
#endif
#endif

// Performance statistic types
//#define PERFMON_ARG__BASE          "base"
#define PERFMON_ARG__FLOW          "flow"
#define PERFMON_ARG__FLOW_IP       "flow-ip"
#define PERFMON_ARG__EVENTS        "events"

// Logging
#define PERFMON_ARG__FILE           "file"
#define PERFMON_ARG__LOG_DIR_FILE   "snortfile"
#define PERFMON_ARG__FLOW_FILE      "flow-file"
#define PERFMON_ARG__FLOW_IP_FILE   "flow-ip-file"
#define PERFMON_ARG__CONSOLE        "console"
#define PERFMON_ARG__MAX_FILE_SIZE  "max_file_size"

// When to log
#define PERFMON_ARG__TIME             "time"
#define PERFMON_ARG__PKT_COUNT        "pktcnt"
#define PERFMON_ARG__SUMMARY          "atexitonly"
#define PERFMON_SUMMARY_OPT__BASE     "base-stats"
#define PERFMON_SUMMARY_OPT__FLOW     "flow-stats"
#define PERFMON_SUMMARY_OPT__FLOW_IP  "flow-ip-stats"
#define PERFMON_SUMMARY_OPT__EVENTS   "events-stats"

// Misc
#define PERFMON_ARG__FLOW_PORTS      "flow-ports"
#define PERFMON_ARG__ACCUMULATE      "accumulate"
#define PERFMON_ARG__RESET           "reset"
#define PERFMON_ARG__MAX_STATS       "max"
#define PERFMON_ARG__FLOW_IP_MEMCAP  "flow-ip-memcap"
#define PERFMON_ARG__FLOW_IP_MEMCAP_MIN  8200


SFPERF *perfmon_config = NULL;

/*
*  Protype these forward references, and don't clutter up the name space
*/
static void PerfMonitorInit(struct _SnortConfig *, char *);
static void ParsePerfMonitorArgs(struct _SnortConfig *, SFPERF *, char *);
static void ProcessPerfMonitor(Packet *, void *);
static void PerfMonitorCleanExit(int, void *);
static void PerfMonitorReset(int, void *);
static void PerfMonitorResetStats(int, void *);
static int  PerfMonitorVerifyConfig(struct _SnortConfig *sc);
static void PerfMonitorFreeConfig(SFPERF *);
static void PerfMonitorOpenLogFiles(struct _SnortConfig *, void *);

#ifndef WIN32
static void PerfMonitorChangeLogFilesPermission(void);
#endif

#ifdef SNORT_RELOAD
static void PerfMonitorReload(struct _SnortConfig *, char *, void **);
static int PerfmonReloadVerify(struct _SnortConfig *, void *);
static void * PerfMonitorReloadSwap(struct _SnortConfig *, void *);
static void PerfMonitorReloadSwapFree(void *);

static bool PerfmonitorReloadAdjustFunc(bool idle, tSfPolicyId raPolicyId, void* userData);
#endif

static int FlowIPStart(uint16_t type, const uint8_t *data, uint32_t length,
                        void **new_context, char *statusBuf, int statusBuf_len);
static int FlowIPStop(uint16_t type, const uint8_t *data, uint32_t length,
                        void **new_context, char *statusBuf, int statusBuf_len);
static void FlowIPShow(uint16_t type, void *old_context, struct _THREAD_ELEMENT *te, ControlDataSendFunc f);

#ifdef PERF_PROFILING
PreprocStats perfmonStats;
#endif

/*
 * Function: SetupPerfMonitor()
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
void SetupPerfMonitor(void)
{
    /* link the preprocessor keyword to the init function in
       the preproc list */
#ifndef SNORT_RELOAD
    RegisterPreprocessor("PerfMonitor", PerfMonitorInit);
#else
    RegisterPreprocessor("PerfMonitor", PerfMonitorInit, PerfMonitorReload,
                         PerfmonReloadVerify, PerfMonitorReloadSwap,
                         PerfMonitorReloadSwapFree);
#endif
    ControlSocketRegisterHandler(CS_TYPE_FLOWIP_START, &FlowIPStart, NULL, NULL);
    ControlSocketRegisterHandler(CS_TYPE_FLOWIP_STOP, &FlowIPStop, NULL, NULL);
    ControlSocketRegisterHandler(CS_TYPE_FLOWIP_SHOW, NULL, NULL, &FlowIPShow);

    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,"Preprocessor: PerfMonitor is setup...\n"););
}

/*
 * Function: PerfMonitorInit(char *)
 *
 * Purpose: Calls the argument parsing function, performs final setup on data
 *          structs, links the preproc function into the function list.
 *
 * Arguments: args => ptr to argument string
 *
 * Returns: void function
 *
 */
static void PerfMonitorInit(struct _SnortConfig *sc, char *args)
{
    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,"Preprocessor: PerfMonitor Initialized\n"););

    //not policy specific. Perf monitor configuration should be in the default
    //configuration file.
    if( ( perfmon_config != NULL ) || getParserPolicy( sc ) != 0 )
    {
#if 0
        // TBD-EDM
         ParseError("Perfmonitor can only be configured in default policy and only once.\n");
#else
        WarningMessage( "Perfmonitor can only be configured in default policy and only once.\n");
#endif
         return;
    }

    perfmon_config = (SFPERF *)SnortAlloc(sizeof(SFPERF));

    /* parse the argument list from the rules file */
    ParsePerfMonitorArgs(sc, perfmon_config, args);
    InitPerfStats(perfmon_config);

#ifndef WIN32
    PerfMonitorChangeLogFilesPermission();
#endif

    /*  register callbacks */
    AddFuncToPreprocCleanExitList(PerfMonitorCleanExit, NULL, PRIORITY_LAST, PP_PERFMONITOR);
    AddFuncToPreprocResetList(PerfMonitorReset, NULL, PRIORITY_LAST, PP_PERFMONITOR);
    AddFuncToPreprocResetStatsList(PerfMonitorResetStats, NULL, PRIORITY_LAST, PP_PERFMONITOR);
    AddFuncToConfigCheckList( sc, PerfMonitorVerifyConfig );
    AddFuncToPreprocPostConfigList(sc, PerfMonitorOpenLogFiles, NULL);

#ifdef PERF_PROFILING
    RegisterPreprocessorProfile("perfmon", &perfmonStats, 0, &totalPerfStats, NULL);
#endif
}

/*
 * Function: ParsePerfMonitorArgs(struct _SnortConfig *, char *)
 *
 * Purpose: Process the preprocessor arguments from the rules file and
 *          initialize the preprocessor's data struct.  This function doesn't
 *          have to exist if it makes sense to parse the args in the init
 *          function.
 *
 * Arguments: args => argument list
 *
 *
 *  perfmonitor: [ time 10 flow ]
 *
 * Returns: void function
 *
 */
static void ParsePerfMonitorArgs(struct _SnortConfig *sc, SFPERF *pconfig, char *args)
{
    char **toks = NULL;
    int num_toks = 0;
    char *base_stats_file = NULL;
    int i;
    char *endptr;

    if (pconfig == NULL)
        return;

    if (sc == NULL)
    {
        FatalError("%s(%d) Snort config for parsing is NULL.\n",
                   __FILE__, __LINE__);
    }

    // Initialize the performance system and set defaults
    sfInitPerformanceStatistics(pconfig);

    if (args != NULL)
       toks = mSplit(args, " \t", 0, &num_toks, 0);

    for (i = 0; i < num_toks; i++)
    {
        if (strcasecmp(toks[i], PERFMON_ARG__FLOW) == 0)
        {
            // This parameter turns on the flow statistics.
            // Flow statistics give you the traffic profile
            // that snort is processing.  This helps in
            // troubleshooting and performance tuning.
            pconfig->perf_flags |= SFPERF_FLOW;
        }
        else if (strcasecmp(toks[i], PERFMON_ARG__FLOW_PORTS) == 0)
        {
            uint32_t value = 0;

            // Requires an integer argument
            if (i == (num_toks - 1))
            {
                ParseError("Perfmonitor:  Missing argument to \"%s\".  The "
                        "value must be a positive integer between 1 and %d.",
                        PERFMON_ARG__FLOW_PORTS, SF_MAX_PORT);
            }

            if ((SnortStrToU32(toks[++i], &endptr, &value, 10) != 0)
                    || (value == 0) || (value > SF_MAX_PORT)
                    || *endptr || (errno == ERANGE))
            {
                ParseError("Perfmonitor:  Invalid argument to \"%s\".  The "
                        "value must be a positive integer between 1 and %d.",
                        PERFMON_ARG__FLOW_PORTS, SF_MAX_PORT);
            }

            pconfig->flow_max_port_to_track = (int)value;
            pconfig->perf_flags |= SFPERF_FLOW;
        }
        else if (strcasecmp(toks[i], PERFMON_ARG__FLOW_FILE) == 0)
        {
            if (pconfig->flow_file != NULL)
                free(pconfig->flow_file);

            // Requires a file name/path argument
            if (i == (num_toks - 1))
            {
                ParseError("Perfmonitor:  Missing file name/path argument "
                        "to \"%s\".", PERFMON_ARG__FLOW_FILE);
            }

            pconfig->perf_flags |= SFPERF_FLOW;
            pconfig->flow_file = ProcessFileOption(sc, toks[++i]);
        }
        else if (strcasecmp(toks[i], PERFMON_ARG__EVENTS) == 0)
        {
            // The events paramenter gives the total number
            // of qualified and non-qualified events during
            // the processing sample time.  This allows
            // performance problems to be seen in a general
            // manner.
            pconfig->perf_flags |= SFPERF_EVENT;
        }
        else if (strcasecmp(toks[i], PERFMON_ARG__FLOW_IP) == 0)
        {
            pconfig->perf_flags |= SFPERF_FLOWIP;
        }
        else if (strcasecmp(toks[i], PERFMON_ARG__FLOW_IP_FILE) == 0)
        {
            if (pconfig->flowip_file != NULL)
                free(pconfig->flowip_file);

            // Requires a file name/path argument
            if (i == (num_toks - 1))
            {
                ParseError("Perfmonitor:  Missing file name/path argument "
                        "to \"%s\".", PERFMON_ARG__FLOW_IP_FILE);
            }

            pconfig->perf_flags |= SFPERF_FLOWIP;
            pconfig->flowip_file = ProcessFileOption(sc, toks[++i]);
        }
        else if (strcasecmp(toks[i], PERFMON_ARG__FLOW_IP_MEMCAP) == 0)
        {
            uint32_t value = 0;

            // Requires an integer argument
            if (i == (num_toks - 1))
            {
                ParseError("Perfmonitor:  Missing argument to \"%s\".  The "
                        "value must be a positive integer.", PERFMON_ARG__FLOW_IP_MEMCAP);
            }

            if ((SnortStrToU32(toks[++i], &endptr, &value, 10) != 0)
                    || (value < PERFMON_ARG__FLOW_IP_MEMCAP_MIN) || *endptr || (errno == ERANGE))
            {
                ParseError("Perfmonitor:  Invalid argument to \"%s\".  The "
                        "value must be a positive integer between %u and %u.",
                        PERFMON_ARG__FLOW_IP_MEMCAP, PERFMON_ARG__FLOW_IP_MEMCAP_MIN, UINT32_MAX);
            }

            pconfig->flowip_memcap = value;
            pconfig->perf_flags |= SFPERF_FLOWIP;
        }
        else if (strcasecmp(toks[i], PERFMON_ARG__TIME) == 0)
        {
            uint32_t value = 0;

            // Requires an integer argument
            if (i == (num_toks - 1))
            {
                ParseError("Perfmonitor:  Missing argument to \"%s\".  The "
                        "value must be a positive integer.", PERFMON_ARG__TIME);
            }

            if ((SnortStrToU32(toks[++i], &endptr, &value, 10) != 0)
                    || (value == 0) || (value > INT32_MAX)
                    || *endptr || (errno == ERANGE))
            {
                ParseError("Perfmonitor:  Invalid argument to \"%s\".  The "
                        "value must be a positive integer between 1 and %d.",
                        PERFMON_ARG__TIME, INT32_MAX);
            }

            pconfig->sample_interval = (int)value;
        }
        else if (strcasecmp(toks[i], PERFMON_ARG__PKT_COUNT) == 0)
        {
            uint32_t value = 0;

            // Requires an integer argument
            if (i == (num_toks - 1))
            {
                ParseError("Perfmonitor:  Missing argument to \"%s\".  The "
                        "value must be an integer.", PERFMON_ARG__PKT_COUNT);
            }

            if ((SnortStrToU32(toks[++i], &endptr, &value, 10) != 0)
                    || *endptr || (errno == ERANGE))
            {
                ParseError("Perfmonitor:  Invalid argument to \"%s\".  The "
                        "value must be an integer between 0 and %u.",
                        PERFMON_ARG__PKT_COUNT, UINT32_MAX);
            }

            pconfig->pkt_cnt = value;
        }
        else if (strcasecmp(toks[i], PERFMON_ARG__ACCUMULATE) == 0)
        {
            pconfig->base_reset = 0;
        }
        else if (strcasecmp(toks[i], PERFMON_ARG__RESET) == 0)
        {
            pconfig->base_reset = 1;
        }
        else if (strcasecmp(toks[i], PERFMON_ARG__MAX_STATS) == 0)
        {
#ifndef LINUX_SMP
            pconfig->perf_flags |= SFPERF_MAX_BASE_STATS;
#endif
        }
        else if (strcasecmp(toks[i], PERFMON_ARG__CONSOLE) == 0)
        {
            pconfig->perf_flags |= SFPERF_CONSOLE;
        }
        else if (strcasecmp(toks[i], PERFMON_ARG__FILE) == 0)
        {
            // Requires a file name/path argument
            if (i == (num_toks - 1))
            {
                ParseError("Perfmonitor:  Missing file name/path argument "
                        "to \"%s\".", PERFMON_ARG__FILE);
            }

            if (base_stats_file != NULL)
            {
                ParseError("Perfmonitor:  \"%s\" can only be specified once "
                        "and cannot be used in combination with \"%s\".",
                        PERFMON_ARG__FILE, PERFMON_ARG__LOG_DIR_FILE);
            }

            base_stats_file = SnortStrdup(toks[++i]);
        }
        else if (strcasecmp(toks[i], PERFMON_ARG__LOG_DIR_FILE) == 0)
        {
            // Requires a file name/path argument
            if (i == (num_toks - 1))
            {
                ParseError("Perfmonitor:  Missing file name/path argument "
                        "to \"%s\".", PERFMON_ARG__LOG_DIR_FILE);
            }

            if (base_stats_file != NULL)
            {
                ParseError("Perfmonitor:  \"%s\" can only be specified once "
                        "and cannot be used in combination with \"%s\".",
                        PERFMON_ARG__LOG_DIR_FILE, PERFMON_ARG__FILE);
            }

            base_stats_file = ProcessFileOption(sc, toks[++i]);
        }
        else if (strcasecmp(toks[i], PERFMON_ARG__MAX_FILE_SIZE) == 0)
        {
            uint32_t value = 0;

            // Requires an integer argument
            if (i == (num_toks - 1))
            {
                ParseError("Perfmonitor:  Missing argument to \"%s\".  The "
                        "value must be an integer between %d and %d.",
                        PERFMON_ARG__MAX_FILE_SIZE,
                        MIN_PERF_FILE_SIZE, MAX_PERF_FILE_SIZE);
            }

            if ((SnortStrToU32(toks[++i], &endptr, &value, 10) != 0)
                    || (value < MIN_PERF_FILE_SIZE) || (value > MAX_PERF_FILE_SIZE)
                    || *endptr || (errno == ERANGE))
            {
                ParseError("Perfmonitor:  Invalid argument to \"%s\".  The "
                        "value must be an integer between %d and %d.",
                        PERFMON_ARG__MAX_FILE_SIZE,
                        MIN_PERF_FILE_SIZE, MAX_PERF_FILE_SIZE);
            }

            // Scale it back by the threshold.
            pconfig->max_file_size = value - ROLLOVER_THRESH;
        }
        else if (strcasecmp(toks[i], PERFMON_ARG__SUMMARY) == 0)
        {
            int summary_flags = 0;

            while (i < num_toks)
            {
                i++;

                if (i == num_toks)
                {
                    // No arguments left.  Keep old behavior where no argument is
                    // needed to atexitonly and apply to all stats.
                    if (!summary_flags)
                        summary_flags |= SFPERF_SUMMARY;
                }
                else if (strcasecmp(toks[i], PERFMON_SUMMARY_OPT__BASE) == 0)
                {
                    summary_flags |= SFPERF_SUMMARY_BASE;
                }
                else if (strcasecmp(toks[i], PERFMON_SUMMARY_OPT__FLOW) == 0)
                {
                    summary_flags |= SFPERF_SUMMARY_FLOW;
                }
                else if (strcasecmp(toks[i], PERFMON_SUMMARY_OPT__FLOW_IP) == 0)
                {
                    summary_flags |= SFPERF_SUMMARY_FLOWIP;
                }
                else if (strcasecmp(toks[i], PERFMON_SUMMARY_OPT__EVENTS) == 0)
                {
                    summary_flags |= SFPERF_SUMMARY_EVENT;
                }
                else
                {
                    // Keep old behaviour of no argument to atexitonly and apply to
                    // all stats.  Decrement index so as to parse next argument out
                    // of this context.
                    if (!summary_flags)
                        summary_flags |= SFPERF_SUMMARY;
                    i--;
                    break;
                }
            }

            if (summary_flags)
                pconfig->perf_flags |= summary_flags;
        }
        else
        {
            ParseError("Perfmonitor:  Invalid parameter - \"%s\".", toks[i]);
        }
    }

    mSplitFree(&toks, num_toks);

    if ((base_stats_file != NULL) || (sc->perf_file != NULL))
    {
        // Use command line override if applicable
        if (sc->perf_file != NULL)
        {
            pconfig->file = SnortStrdup(sc->perf_file);

            if (base_stats_file != NULL)
                free(base_stats_file);
        }
        else
        {
            pconfig->file = base_stats_file;
        }
    }

    if ((pconfig->perf_flags & SFPERF_SUMMARY) != SFPERF_SUMMARY)
        pconfig->perf_flags |= SFPERF_TIME_COUNT;

    LogMessage("PerfMonitor config:\n");
    LogMessage("  Sample Time:      %d seconds\n", pconfig->sample_interval);
    LogMessage("  Packet Count:     %d\n", pconfig->pkt_cnt);
    LogMessage("  Max File Size:    %u\n", pconfig->max_file_size);
    LogMessage("  Base Stats:       %s%s\n",
            pconfig->perf_flags & SFPERF_BASE ? "ACTIVE" : "INACTIVE",
            pconfig->perf_flags & SFPERF_SUMMARY_BASE ? " (SUMMARY)" : "");
    if (pconfig->perf_flags & SFPERF_BASE)
    {
        LogMessage("    Base Stats File:  %s\n",
                (pconfig->file != NULL) ? pconfig->file : "INACTIVE");
        LogMessage("    Max Perf Stats:   %s\n",
                (pconfig->perf_flags & SFPERF_MAX_BASE_STATS) ? "ACTIVE" : "INACTIVE");
    }
    LogMessage("  Flow Stats:       %s%s\n",
            pconfig->perf_flags & SFPERF_FLOW ? "ACTIVE" : "INACTIVE",
            pconfig->perf_flags & SFPERF_SUMMARY_FLOW ? " (SUMMARY)" : "");
    if (pconfig->perf_flags & SFPERF_FLOW)
    {
        LogMessage("    Max Flow Port:    %u\n", pconfig->flow_max_port_to_track);
        LogMessage("    Flow File:        %s\n",
                (pconfig->flow_file != NULL) ? pconfig->flow_file : "INACTIVE");
    }
    LogMessage("  Event Stats:      %s%s\n",
            pconfig->perf_flags & SFPERF_EVENT ? "ACTIVE" : "INACTIVE",
            pconfig->perf_flags & SFPERF_SUMMARY_EVENT ? " (SUMMARY)" : "");
    LogMessage("  Flow IP Stats:    %s%s\n",
            pconfig->perf_flags & SFPERF_FLOWIP ? "ACTIVE" : "INACTIVE",
            pconfig->perf_flags & SFPERF_SUMMARY_FLOWIP ? " (SUMMARY)" : "");
    if (pconfig->perf_flags & SFPERF_FLOWIP)
    {
        LogMessage("    Flow IP Memcap:   %u\n", pconfig->flowip_memcap);
        LogMessage("    Flow IP File:     %s\n",
                (pconfig->flowip_file != NULL) ? pconfig->flowip_file : "INACTIVE");
    }
    LogMessage("  Console Mode:     %s\n",
            (pconfig->perf_flags & SFPERF_CONSOLE) ? "ACTIVE" : "INACTIVE");
}

/*
 * Function: ProcessPerfMonitor(Packet *)
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
static void ProcessPerfMonitor(Packet *p, void *context)
{
    static bool first = true;
    PROFILE_VARS;
    PREPROC_PROFILE_START(perfmonStats);

    if (first)
    {
        if (ScReadMode())
        {
            sfBase.pkt_stats.pkts_recv = pc.total_from_daq;
            sfBase.pkt_stats.pkts_drop = 0;
        }
        else
        {
            const DAQ_Stats_t* ps = DAQ_GetStats();
            sfBase.pkt_stats.pkts_recv = ps->hw_packets_received;
            sfBase.pkt_stats.pkts_drop = ps->hw_packets_dropped;
        }

        first = false;
    }

    if (IsSetRotatePerfFileFlag())
    {
        sfRotateBaseStatsFile(perfmon_config);
        sfRotateFlowStatsFile(perfmon_config);
        ClearRotatePerfFileFlag();
    }

    sfPerformanceStats(perfmon_config, p);

    PREPROC_PROFILE_END(perfmonStats);
    return;
}

/**
 * CleanExit func required by preprocessors
 */
static void PerfMonitorCleanExit(int signal, void *foo)
{
    if (perfmon_config->perf_flags & SFPERF_SUMMARY)
        sfPerfStatsSummary(perfmon_config);

    /* Close the performance stats file */
    sfCloseBaseStatsFile(perfmon_config);
    sfCloseFlowStatsFile(perfmon_config);
    sfCloseFlowIPStatsFile(perfmon_config);
    FreeFlowStats(&sfFlow);
    FreeFlowIPStats(&sfFlow);
#ifdef LINUX_SMP
    FreeProcPidStats(&sfBase.sfProcPidStats);
#endif

    PerfMonitorFreeConfig(perfmon_config);
    perfmon_config = NULL;
}

static void PerfMonitorFreeConfig(SFPERF *config)
{
    if (config == NULL)
        return;

    if (config->file != NULL)
        free(config->file);

    if (config->flow_file != NULL)
        free(config->flow_file);

    if (config->flowip_file != NULL)
        free(config->flowip_file);

    free(config);
}

static void PerfMonitorReset(int signal, void *foo)
{
    return;
}

static void PerfMonitorResetStats(int signal, void *foo)
{
    if (perfmon_config == NULL)
        return;

    InitPerfStats(perfmon_config);
}

/* This function changes the perfmon log files permission if exists.
   It is done in the  PerfMonitorInit() before Snort changed its user & group.
 */
#ifndef WIN32
static void PerfMonitorChangeLogFilesPermission(void)
{
    struct stat pt;
    mode_t mode =  S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH;

    if (perfmon_config == NULL)
        return;

    if (perfmon_config->file != NULL)
    {
        /*Check file before change permission*/
        if (stat(perfmon_config->file, &pt) == 0)
        {
            /*Only change permission for file owned by root*/
            if ((0 == pt.st_uid) || (0 == pt.st_gid))
            {
                if (chmod(perfmon_config->file, mode) != 0)
                {
                    ParseError("Perfmonitor: Unable to change mode of "
                            "base stats file \"%s\" to mode:%d: %s.",
                            perfmon_config->file, mode, strerror(errno));
                }

                if (chown(perfmon_config->file, ScUid(), ScGid()) != 0)
                {
                    ParseError("Perfmonitor: Unable to change permissions of "
                            "base stats file \"%s\" to user:%d and group:%d: %s.",
                            perfmon_config->file, ScUid(), ScGid(), strerror(errno));
                }
            }
        }
    }

    if (perfmon_config->flow_file != NULL)
    {
        /*Check file before change permission*/
        if (stat(perfmon_config->flow_file, &pt) == 0)
        {
            /*Only change permission for file owned by root*/
            if ((0 == pt.st_uid) || (0 == pt.st_gid))
            {
                if (chmod(perfmon_config->flow_file, mode) != 0)
                {
                    ParseError("Perfmonitor: Unable to change mode of "
                            "flow stats file \"%s\" to mode:%d: %s.",
                            perfmon_config->flow_file, mode, strerror(errno));
                }

                if (chown(perfmon_config->flow_file, ScUid(), ScGid()) != 0)
                {
                    ParseError("Perfmonitor: Unable to change permissions of "
                            "flow stats file \"%s\" to user:%d and group:%d: %s.",
                            perfmon_config->flow_file, ScUid(), ScGid(), strerror(errno));
                }
            }
        }
    }

    if (perfmon_config->flowip_file != NULL)
    {
        /*Check file before change permission*/
        if (stat(perfmon_config->flowip_file, &pt) == 0)
        {
            /*Only change permission for file owned by root*/
            if ((0 == pt.st_uid) || (0 == pt.st_gid))
            {
                if (chmod(perfmon_config->flowip_file, mode) != 0)
                {
                    ParseError("Perfmonitor: Unable to change mode of "
                            "flow-ip stats file \"%s\" to mode:%d: %s.",
                            perfmon_config->flowip_file, mode, strerror(errno));
                }

                if (chown(perfmon_config->flowip_file, ScUid(), ScGid()) != 0)
                {
                    ParseError("Perfmonitor: Unable to change permissions of "
                            "flow-ip stats file \"%s\" to user:%d and group:%d: %s.",
                            perfmon_config->flowip_file, ScUid(), ScGid(), strerror(errno));
                }
            }
        }
    }
}
#endif

static void initializePerfmonForDispatch( struct _SnortConfig *sc )
{
    AddFuncToPreprocListAllNapPolicies( sc, ProcessPerfMonitor, PRIORITY_SCANNER, PP_PERFMONITOR, PROTO_BIT__ALL );
    session_api->enable_preproc_all_ports_all_policies( sc, PP_PERFMONITOR, PROTO_BIT__ALL );
}

static int PerfMonitorVerifyConfig(struct _SnortConfig *sc)
{
    if (perfmon_config == NULL)
        return 0;

    // register perfmon callback with policy and session
    initializePerfmonForDispatch( sc );

    return 0;
}

/* This function opens the perfmon log files.
   The logic was moved out of PerfMonitorInit() to avoid creating files
   before Snort changed its user & group.
*/
static void PerfMonitorOpenLogFiles(struct _SnortConfig *sc, void *data)
{
    if (perfmon_config == NULL)
        return;

    if ((perfmon_config->file != NULL)
            && ((perfmon_config->fh = sfOpenBaseStatsFile(perfmon_config->file)) == NULL))
    {
        ParseError("Perfmonitor: Cannot open base stats file \"%s\".", perfmon_config->file);
    }

    if ((perfmon_config->flow_file != NULL)
            && ((perfmon_config->flow_fh = sfOpenFlowStatsFile(perfmon_config->flow_file)) == NULL))
    {
        ParseError("Perfmonitor: Cannot open flow stats file \"%s\".", perfmon_config->flow_file);
    }

    if ((perfmon_config->flowip_file != NULL)
            && ((perfmon_config->flowip_fh = sfOpenFlowIPStatsFile(perfmon_config->flowip_file)) == NULL))
    {
        ParseError("Perfmonitor: Cannot open flow-ip stats file \"%s\".", perfmon_config->flowip_file);
    }
}

#ifdef SNORT_RELOAD

static void PerfMonitorReload(struct _SnortConfig *sc, char *args, void **new_config)
{
    SFPERF *perfmon_swap_config = (SFPERF *)*new_config;

    // Perf monitor configuration must be defined in the default configuration file only.
    if( ( perfmon_swap_config != NULL ) || getParserPolicy( sc ) != 0 )
    {
#if 0
        // TBD-EDM
         ParseError("Perfmonitor can only be configured in default policy and only once.\n");
#else
        WarningMessage( "Perfmonitor can only be configured in default policy and only once.\n");
#endif
        return;
    }

    perfmon_swap_config = (SFPERF *)SnortAlloc(sizeof(SFPERF));
    *new_config = (void *)perfmon_swap_config;

    /* parse the argument list from the rules file */
    ParsePerfMonitorArgs(sc, perfmon_swap_config, args);
}

static int PerfmonReloadVerify(struct _SnortConfig *sc, void *swap_config)
{
    SFPERF *perfmon_swap_config = (SFPERF *)swap_config;

    if (perfmon_config == NULL && perfmon_swap_config == NULL)
        return 0;
    //start from scratch
    if (perfmon_config == NULL && perfmon_swap_config !=NULL)
    {
        initializePerfmonForDispatch( sc );
        return 0;
    }
    //keep perfmon_config
    if (perfmon_config != NULL && perfmon_swap_config ==NULL)
        return 0;

    //the only way to prevent this would be to change the number of rows in the
    //sfFlow.ipMap hash table. That solution seems extreme, plus as it stands
    //snort wouldn't function on fresh start with a flowip_memcap that low becuase the hash table
    //rows are hard coded in InitFlowIPStats in preprocessors/perf-flow.c
    if (perfmon_config->perf_flags & SFPERF_FLOWIP && perfmon_swap_config->perf_flags & SFPERF_FLOWIP
            && sfxhash_overhead_bytes(sfFlow.ipMap) > perfmon_swap_config->flowip_memcap)
    {
        ErrorMessage("Perfmonitor Reload: flowip_memcap is too low.\n");
        return -1;
    }

    perfmon_reload_status = PERF_RELOAD_VERIFY;
    syncAllStats(perfmon_config, perfmon_swap_config);
    perfmon_reload_status = PERF_NOT_RELOADING;

    // register perfmon callback with policy and session
    initializePerfmonForDispatch( sc );

    return 0;
}

static void * PerfMonitorReloadSwap(struct _SnortConfig *sc, void *swap_config)
{
    SFPERF *perfmon_swap_config = (SFPERF *)swap_config;
    SFPERF *old_config = perfmon_config;

    if (perfmon_swap_config == NULL)
        return NULL;

    //start from scratch
    if (perfmon_config == NULL)
    {
        InitPerfStats(perfmon_swap_config);
        perfmon_config = perfmon_swap_config;
        return NULL;
    }

    perfmon_reload_status = PERF_RELOAD_SWAP;
    syncAllStats(perfmon_config, perfmon_swap_config);
    perfmon_reload_status = PERF_NOT_RELOADING;

    if (perfmon_swap_config->perf_flags & SFPERF_FLOWIP && perfmon_config->perf_flags & SFPERF_FLOWIP)
    {
        //this is mainly intended for when FLOWIP is in summary mode and we may have to free objects
        //in use, but it's also used in other situations to decrase the size of an empty table
        tSfPolicyId policy_id = getParserPolicy(sc);
        ReloadAdjustRegister(sc,"perfmon", policy_id, &PerfmonitorReloadAdjustFunc, NULL, NULL);
    }

    perfmon_config = perfmon_swap_config;

    return (void *)old_config;
}

static void PerfMonitorReloadSwapFree(void *data)
{
    if (data == NULL)
        return;

    perfmon_reload_status = PERF_RELOAD_SWAP_FREE;
    syncAllStats((SFPERF *) data, perfmon_config);
    perfmon_reload_status = PERF_NOT_RELOADING;

    PerfMonitorFreeConfig((SFPERF *)data);
}


static bool PerfmonitorReloadAdjustFunc(bool idle, tSfPolicyId raPolicyId, void* userData)
{
    unsigned max_work = idle ? 512 : 32;
    int ret = sfxhash_change_memcap(sfFlow.ipMap, perfmon_config->flowip_memcap, &max_work);
#ifdef REG_TEST
    if (REG_TEST_FLAG_PERFMON_RELOAD & getRegTestFlags())
    {
        printf("memused:%lu\n",sfFlow.ipMap->mc.memused);
        printf("memcap:%lu\n",sfFlow.ipMap->mc.memcap);
    }
#endif
    return ret == SFXHASH_OK;
}

#endif

static int FlowIPStart(uint16_t type, const uint8_t *data, uint32_t length,
                        void **new_context, char *statusBuf, int statusBuf_len)
{
    char *args = (char*) data;
    ParsePerfMonitorArgs(snort_conf, perfmon_config, args);
    InitPerfStats(perfmon_config);
#ifndef WIN32
    PerfMonitorChangeLogFilesPermission();
#endif
    PerfMonitorOpenLogFiles(snort_conf,NULL);
    return 0;
}

static int FlowIPStop(uint16_t type, const uint8_t *data, uint32_t length,
                        void **new_context, char *statusBuf, int statusBuf_len)
{
    sfCloseFlowIPStatsFile(perfmon_config);
    FreeFlowIPStats(&sfFlow);

    if (perfmon_config->flowip_file != NULL)
        free(perfmon_config->flowip_file);
 
    perfmon_config->perf_flags &= ~SFPERF_FLOWIP;
    return 0;
}

static void FlowIPShow(uint16_t type, void *old_context, struct _THREAD_ELEMENT *te, ControlDataSendFunc f)
{
    char buffer[CS_STATS_BUF_SIZE + 1];
    int len = 0;
    if(perfmon_config->perf_flags & SFPERF_FLOWIP)
        len = snprintf(buffer, CS_STATS_BUF_SIZE, "\nFlow IP Profiling is enabled.\n\n");
    else
        len = snprintf(buffer, CS_STATS_BUF_SIZE, "\nFlow IP Profiling is disabled.\n\n");

    if(-1 == f(te, (const uint8_t *)buffer, len))
        LogMessage("Unable to send data to the frontend\n");
}
