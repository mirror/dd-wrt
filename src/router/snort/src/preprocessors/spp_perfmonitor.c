/* $Id$ 
**
**  spp_perfmonitor.c
**
**  Copyright (C) 2002-2011 Sourcefire, Inc.
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
**  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
**
*/

#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include "plugbase.h"
#include "mstring.h"
#include "util.h"
#include "debug.h"
#include "parser.h"
#include "sfdaq.h"
#include "snort.h"
#include "perf.h"
#include "perf-base.h"
#include "profiler.h"

SFPERF *perfmon_config = NULL;

/*
*  Protype these forward references, and don't clutter up the name space
*/
static void PerfMonitorInit(char *);
static void ParsePerfMonitorArgs(SFPERF *, char *);
static void ProcessPerfMonitor(Packet *, void *);
static void PerfMonitorCleanExit(int, void *);
static void PerfMonitorReset(int, void *);
static void PerfMonitorResetStats(int, void *);
static void PerfMonitorFreeConfig(SFPERF *);

#ifdef SNORT_RELOAD
SFPERF *perfmon_swap_config = NULL;
static void PerfMonitorReload(char *);
static int PerfmonReloadVerify(void);
static void * PerfMonitorReloadSwap(void);
static void PerfMonitorReloadSwapFree(void *);
#endif

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
                         PerfMonitorReloadSwap, PerfMonitorReloadSwapFree);
#endif

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
static void PerfMonitorInit(char *args)
{
    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,"Preprocessor: PerfMonitor Initialized\n"););
    
    //not policy specific. Perf monitor configuration should be in the default
    //configuration file.
    if (getParserPolicy() != 0)
        return;

    if (perfmon_config != NULL)
        ParseError("Permonitor can only be configured once.\n");

    perfmon_config = (SFPERF *)SnortAlloc(sizeof(SFPERF));

    /* parse the argument list from the rules file */
    ParsePerfMonitorArgs(perfmon_config, args);

    if (perfmon_config->file != NULL)
    {
        if (sfSetPerformanceStatisticsEx(perfmon_config, SFPERF_FILE, perfmon_config->file))
            ParseError("Cannot open performance log file '%s'.", perfmon_config->file);
    }

    if (perfmon_config->flowip_file != NULL)
    {
        if (sfOpenFlowIPStatsFile(perfmon_config))
            ParseError("Cannot open Flow-IP log file '%s'.", perfmon_config->flowip_file);
    }

    /* Set the preprocessor function into the function list */
    AddFuncToPreprocList(ProcessPerfMonitor, PRIORITY_SCANNER, PP_PERFMONITOR, PROTO_BIT__ALL);
    AddFuncToPreprocCleanExitList(PerfMonitorCleanExit, NULL, PRIORITY_LAST, PP_PERFMONITOR);
    AddFuncToPreprocResetList(PerfMonitorReset, NULL, PRIORITY_LAST, PP_PERFMONITOR);
    AddFuncToPreprocResetStatsList(PerfMonitorResetStats, NULL, PRIORITY_LAST, PP_PERFMONITOR);

#ifdef PERF_PROFILING
    RegisterPreprocessorProfile("perfmon", &perfmonStats, 0, &totalPerfStats);
#endif
}

/*
 * Function: ParsePerfMonitorArgs(char *)
 *
 * Purpose: Process the preprocessor arguements from the rules file and 
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
static void ParsePerfMonitorArgs(SFPERF *pconfig, char *args)
{
    char **Tokens=NULL;
    int   iTokenNum=0;
    int   i, iTime=60, iFlow=0, iFlowMaxPort=1023, iEvents=0, iMaxPerfStats=0;
    int   iFile=0, iSnortFile=0, iConsole=0, iPkts=10000, iReset=1;
    int   iStatsExit=0, iFlowIP=0;
    uint32_t uiMaxFileSize=MAX_PERF_FILE_SIZE, uiFlowIPMemcap=50*1024*1024;
    char  *file = NULL;
    char  *snortfile = NULL;
    char  *flowipfile = NULL;
    char  *pcEnd;

    if (pconfig == NULL)
        return;

    if (snort_conf_for_parsing == NULL)
    {
        FatalError("%s(%d) Snort config for parsing is NULL.\n",
                   __FILE__, __LINE__);
    }

    if( args )
    {
       Tokens = mSplit(args, " \t", 0, &iTokenNum, 0);
    }
    
    for( i = 0; i < iTokenNum; i++ )
    {
        /* Check for a 'time number' parameter */
        if( strcasecmp( Tokens[i],"time")==0 )
        {
            /* make sure we have at least one more argument */
            if( i == (iTokenNum-1) )
            {
                ParseError("Missing Time.  The value must be a "
                           "positive integer number.");
            }

            iTime = strtol(Tokens[++i], &pcEnd, 10);
            if(iTime <= 0 || *pcEnd)
            {
                ParseError("Invalid Time.  The value must be a "
                           "positive integer number.");
            }
        }
        else if( strcasecmp( Tokens[i],"flow-ports")==0 )
        {
              i++;
              if( (i< iTokenNum) && Tokens[i] )
                  iFlowMaxPort= atoi(Tokens[i]);
              
              if( iFlowMaxPort > SF_MAX_PORT )
                  iFlowMaxPort = SF_MAX_PORT;

              iFlow=1;
        }
        else if( strcasecmp( Tokens[i],"flow")==0 )
        {
            /*
            **  This parameter turns on the flow statistics.
            **  Flow statistics give you the traffic profile
            **  that snort is processing.  This helps in
            **  troubleshooting and performance tuning.
            */
            iFlow = 1;
        }       
        else if( strcasecmp( Tokens[i],"accumulate")==0)
        {
            iReset=0;
        }
        else if( strcasecmp( Tokens[i],"reset")==0 )
        {
            iReset=1;
        }
        else if( strcasecmp( Tokens[i],"events")==0 )
        {
            /*
            **  The events paramenter gives the total number
            **  of qualified and non-qualified events during
            **  the processing sample time.  This allows 
            **  performance problems to be seen in a general
            **  manner.
            */
            iEvents = 1;
        }
        else if(!strcasecmp(Tokens[i], "max"))
        {
            iMaxPerfStats = 1;
        }
        else if(!strcasecmp(Tokens[i], "console"))
        {
            iConsole = 1;
        }
        else if(!strcasecmp(Tokens[i], "file"))
        {
            if( i == (iTokenNum-1) )
            {
                ParseError("Missing 'file' argument.  This value "
                           "is the file that save stats.");
            }

            iFile = 1;

            file = SnortStrdup(Tokens[++i]);
        }
        else if(!strcasecmp(Tokens[i], "snortfile"))
        {
            if( i == (iTokenNum-1) )
            {
                ParseError("Missing 'snortfile' argument.  This "
                           "value is the file that save stats.");
            }

            iSnortFile = 1;

            snortfile = ProcessFileOption(snort_conf_for_parsing, Tokens[++i]);
        }
        else if(!strcasecmp(Tokens[i], "pktcnt"))
        {
            if( i == (iTokenNum-1) )
            {
                ParseError("Missing 'pktcnt' argument.  This value "
                           "should be a positive integer or zero.");
            }

            iPkts = atoi(Tokens[++i]);
            if( iPkts < 0 )
                iPkts = 1000;
        }
        else if (!strcmp(Tokens[i], "max_file_size"))
        {
            if (i == (iTokenNum-1) )
            {
                ParseError("Missing 'max_file_size' argument.  This "
                           "value should be a positive integer.");
            }

            uiMaxFileSize = SnortStrtoul(Tokens[++i], &pcEnd, 10);

            if (*pcEnd || (errno == ERANGE))
            {
                  ParseError("Invalid max_file_size.  The value must "
                             "be a positive integer.");
            }

            if (uiMaxFileSize > MAX_PERF_FILE_SIZE)
            {
                ParseError("'max_file_size' specified as %u, but it cannot exceed %u",
                           uiMaxFileSize, MAX_PERF_FILE_SIZE);
            }

            if (uiMaxFileSize < MIN_PERF_FILE_SIZE)
            {
                ParseError("'max_file_size' specified as %u, but it cannot be less than %u",
                           uiMaxFileSize, MIN_PERF_FILE_SIZE);
            }

            /* Scale it back by the threshold. */
            uiMaxFileSize -= ROLLOVER_THRESH;
        }
        else if (!strcasecmp(Tokens[i], "atexitonly"))
        {
            iStatsExit = 1;
        }
        else if (!strcasecmp(Tokens[i], "flow-ip"))
        {
            iFlowIP = 1;
        }
        else if (!strcasecmp(Tokens[i], "flow-ip-file"))
        {
            if (i == (iTokenNum - 1))
            {
                ParseError("Missing 'flow-ip-file' argument.  This value is the file to save flow-ip stats to.");
            }

            iFlowIP = 1;
            flowipfile = ProcessFileOption(snort_conf_for_parsing, Tokens[++i]);
        }
        else if (!strcasecmp(Tokens[i], "flow-ip-memcap"))
        {
            iFlowIP = 1;
            uiFlowIPMemcap = strtol(Tokens[++i], &pcEnd, 10);
            if(iTime <= 0 || *pcEnd)
            {
                ParseError("Invalid Flow-IP memcap.  The value must be in bytes described by a positive integer.");
            }
        }
        else
        {
            ParseError("Invalid parameter '%s' to preprocessor "
                       "PerfMonitor.", Tokens[i]);
        }
    }

    mSplitFree(&Tokens, iTokenNum);

    /* Initialize the performance system and set flags */
    sfInitPerformanceStatistics(pconfig);
    sfSetMaxFileSize(pconfig, uiMaxFileSize);
    sfSetPerformanceSampleTime(pconfig, iTime);
    sfSetPerformanceStatistics(pconfig, SFPERF_BASE);
    sfSetPerformanceAccounting(pconfig, iReset);

    if (iFlow)
    {
        sfSetPerformanceStatistics(pconfig, SFPERF_FLOW);
        pconfig->flow_max_port_to_track = iFlowMaxPort;

    }

    if (iFlowIP)
    {
        sfSetPerformanceStatistics(pconfig, SFPERF_FLOWIP);
        pconfig->flowip_file = flowipfile;
        pconfig->flowip_memcap = uiFlowIPMemcap;
    }

    if (iEvents)
        sfSetPerformanceStatistics(pconfig, SFPERF_EVENT);

    if (iMaxPerfStats)
        sfSetPerformanceStatistics(pconfig, SFPERF_BASE_MAX);
     
    if (iConsole)
        sfSetPerformanceStatistics(pconfig, SFPERF_CONSOLE);

    if (iFile && iSnortFile)
    {
        if (file != NULL)
            free(file);

        if (snortfile != NULL)
            free(snortfile);

        ParseError("Cannot log to both 'file' and 'snortfile'.");
    }

    if (iFile || iSnortFile || (snort_conf_for_parsing->perf_file != NULL))
    {
        /* use command line override if applicable */
        if (snort_conf_for_parsing->perf_file != NULL)
        {
            iFile=1;
            pconfig->file = SnortStrdup(snort_conf_for_parsing->perf_file);
            if (file != NULL)
                free(file);
            if (snortfile != NULL)
                free(snortfile);

            /* For printing below */
            file = pconfig->file;
        }
        else
        {
            if (iFile)
                pconfig->file = file;
            else
                pconfig->file = snortfile;
        }
    }
    
    if (iPkts)
        sfSetPerformanceStatisticsEx(pconfig, SFPERF_PKTCNT, &iPkts);

    if (iStatsExit)
        sfSetPerformanceStatisticsEx(pconfig, SFPERF_SUMMARY, &iStatsExit);

    LogMessage("PerfMonitor config:\n");
    LogMessage("    Time:           %d seconds\n", iTime);
    LogMessage("    Flow Stats:     %s\n", iFlow ? "ACTIVE" : "INACTIVE");
    LogMessage("    Flow IP Stats:  %s\n", iFlowIP ? "ACTIVE" : "INACTIVE");
    if (iFlowIP)
    {
        LogMessage("       Flow IP Memcap:  %u\n", uiFlowIPMemcap);
        LogMessage("       Flow IP File:    %s\n", flowipfile ? flowipfile : "INACTIVE");
    }
    LogMessage("    Event Stats:    %s\n", iEvents ? "ACTIVE" : "INACTIVE");
    LogMessage("    Max Perf Stats: %s\n", 
               iMaxPerfStats ? "ACTIVE" : "INACTIVE");
    LogMessage("    Console Mode:   %s\n", iConsole ? "ACTIVE" : "INACTIVE");
    LogMessage("    File Mode:      %s\n", 
               iFile ? file : "INACTIVE");
    LogMessage("    SnortFile Mode: %s\n", 
               iSnortFile ? snortfile : "INACTIVE");
    LogMessage("    Packet Count:   %d\n", iPkts);
    LogMessage("    Dump Summary:   %s\n", pconfig->perf_flags & SFPERF_SUMMARY ?
               "Yes" : "No");
    LogMessage("    Max file size:  %u\n", uiMaxFileSize);

    if (pconfig->perf_flags & SFPERF_SUMMARY)
        CheckSampleInterval(NULL, pconfig);
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
    static int first = 1;
    SFSType type = SFS_TYPE_OTHER;
    PROFILE_VARS;

    if( first )
    {
        const DAQ_Stats_t* ps = DAQ_GetStats();
        sfBase.pkt_stats.pkts_recv = ps->hw_packets_received;
        sfBase.pkt_stats.pkts_drop = ps->hw_packets_dropped;
        first = 0;
    }

    if(p == NULL) 
    {
        return;
    }


    PREPROC_PROFILE_START(perfmonStats);
    /*
    *  Performance Statistics  
    */
    if (IsSetRotatePerfFileFlag())
    {
        sfRotatePerformanceStatisticsFile();
        ClearRotatePerfFileFlag();
    }

    if (perfmon_config->sample_interval > 0)
    {
        if(p->pkth)
        {
            sfPerformanceStats(perfmon_config, p, p->packet_flags & PKT_REBUILT_STREAM);
        }
    }
    
    if( p->tcph )
    {
        if((p->tcph->th_flags & TH_SYN) && !(p->tcph->th_flags & TH_ACK))
        {
            /* changed to measure syns */
            sfBase.iSyns++;
        }
        else if((p->tcph->th_flags & TH_SYN) && (p->tcph->th_flags & TH_ACK ))
        {
            /* this is a better approximation of connections */
            sfBase.iSynAcks++;
        }
    }

    /*
    *  TCP Flow Perf
    */
    if(p->pkth && (perfmon_config->perf_flags & SFPERF_FLOW))
    {
        /*
        **  TCP Flow Stats
        */
        if( p->tcph && !(p->packet_flags & PKT_REBUILT_STREAM))
        {
            UpdateTCPFlowStatsEx(&sfFlow, p->sp, p->dp, p->pkth->caplen);
            type = SFS_TYPE_TCP;
        }
        /*
        *  UDP Flow Stats
        */
        else if( p->udph )
        {
            UpdateUDPFlowStatsEx(&sfFlow, p->sp, p->dp, p->pkth->caplen);
            type = SFS_TYPE_UDP;
        }
        /*
        *  Get stats for ICMP packets
        */
        else if( p->icmph )
            UpdateICMPFlowStatsEx(&sfFlow, p->icmph->type, p->pkth->caplen);
    }

    /*
     * IPv4 distribution flow stats
     */
    if (p->pkth && (perfmon_config->perf_flags & SFPERF_FLOWIP) && IsIP(p))
    {
        if (p->tcph && !(p->packet_flags & PKT_REBUILT_STREAM))
            type = SFS_TYPE_TCP;
        else if (p->udph)
            type = SFS_TYPE_UDP;

        UpdateFlowIPStats(&sfFlow, GET_SRC_IP(p), GET_DST_IP(p), p->pkth->caplen, type);
    }

    PREPROC_PROFILE_END(perfmonStats);
    return;
}

/**
 * CleanExit func required by preprocessors
 */
static void PerfMonitorCleanExit(int signal, void *foo)
{
    if (perfmon_config->perf_flags & SFPERF_SUMMARY)
        sfProcessPerfStats(perfmon_config);

    /* Close the performance stats file */
    sfSetPerformanceStatisticsEx(perfmon_config, SFPERF_FILECLOSE, NULL);
    sfCloseFlowIPStatsFile(perfmon_config);
    FreeFlowStats(&sfFlow);
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

    ResetPerfStats(perfmon_config);
}

#ifdef SNORT_RELOAD
static void PerfMonitorReload(char *args)
{
    //not policy specific. Perf monitor configuration should be in the default
    //configuration file.
    if (getParserPolicy() != 0)
        return;

    if (perfmon_swap_config != NULL)
        ParseError("Perfmonitor can only be configured once.\n");

    perfmon_swap_config = (SFPERF *)SnortAlloc(sizeof(SFPERF));

    /* parse the argument list from the rules file */
    ParsePerfMonitorArgs(perfmon_swap_config, args);

    /* Since the file isn't opened again, copy the file handle from the
     * current configuration.  Verification will be done on file name to
     * ensure it didn't change. */
    if (perfmon_config->fh != NULL)
    {
        perfmon_swap_config->fh = perfmon_config->fh;
        perfmon_swap_config->perf_flags |= perfmon_config->perf_flags & SFPERF_FILE;
    }

    AddFuncToPreprocList(ProcessPerfMonitor, PRIORITY_SCANNER, PP_PERFMONITOR, PROTO_BIT__ALL);
    AddFuncToPreprocReloadVerifyList(PerfmonReloadVerify);
}
    
static int PerfmonReloadVerify(void)
{
    if ((perfmon_config == NULL) || (perfmon_swap_config == NULL))
        return 0;

    if ((perfmon_config->file != NULL) && (perfmon_swap_config->file != NULL))
    {
        /* File - don't do case insensitive compare */
        if (strcmp(perfmon_config->file, perfmon_swap_config->file) != 0)
        {
            ErrorMessage("Perfmonitor Reload: Changing the log file requires "
                         "a restart.\n");
            PerfMonitorFreeConfig(perfmon_swap_config);
            perfmon_swap_config = NULL;
            return -1;
        }
    }
    else if (perfmon_config->file != perfmon_swap_config->file)
    {
        ErrorMessage("Perfmonitor Reload: Changing the log file requires "
                     "a restart.\n");
        PerfMonitorFreeConfig(perfmon_swap_config);
        perfmon_swap_config = NULL;
        return -1;
    }

    return 0;
}

static void * PerfMonitorReloadSwap(void)
{
    SFPERF *old_config = perfmon_config;

    if (perfmon_swap_config == NULL)
        return NULL;

    perfmon_config = perfmon_swap_config;
    perfmon_swap_config = NULL;

    return (void *)old_config;
}

static void PerfMonitorReloadSwapFree(void *data)
{
    if (data == NULL)
        return;

    PerfMonitorFreeConfig((SFPERF *)data);
}
#endif
