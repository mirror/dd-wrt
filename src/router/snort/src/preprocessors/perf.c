/*
**  $Id$
**
**  perf.c
**
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2002-2013 Sourcefire, Inc.
** Dan Roelker <droelker@sourcefire.com>
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
**
**
**  DESCRIPTION
**    These are the basic functions that are needed to call performance
**    functions.
**
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#ifndef WIN32
# include <time.h>
# include <unistd.h>
#endif /* WIN32 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "util.h"
#include "perf.h"
#include "sf_types.h"
#include "decode.h"
#include "snort.h"

SFBASE sfBase;
SFFLOW sfFlow;
SFEVENT sfEvent;
int perfmon_rotate_perf_file = 0;
static uint32_t pkt_cnt = 0;

#ifdef SNORT_RELOAD
    extern SFPERF* perfmon_config;
    PERFRELOAD_STATUS perfmon_reload_status = PERF_NOT_RELOADING;
#endif

static void UpdatePerfStats(SFPERF *, Packet *p);
static bool CheckSampleInterval(SFPERF *, time_t);
static inline bool sfCheckFileSize(FILE *, uint32_t);
static inline void sfProcessBaseStats(SFPERF *);
static inline void sfProcessFlowStats(SFPERF *);
static inline void sfProcessFlowIpStats(SFPERF *);
static inline void sfProcessEventStats(SFPERF *);
static inline int sfRotateFlowIPStatsFile(SFPERF *);
static int sfRotateFile(const char *, FILE *, const char *, uint32_t);

void sfInitPerformanceStatistics(SFPERF *sfPerf)
{
    memset(sfPerf, 0, sizeof(SFPERF));
    sfPerf->sample_interval = 60;
    sfPerf->flow_max_port_to_track = 1023;
    sfPerf->perf_flags |= SFPERF_BASE;
    sfPerf->pkt_cnt = 10000;
    sfPerf->max_file_size = MAX_PERF_FILE_SIZE;
    sfPerf->flowip_memcap = 50*1024*1024;
    sfPerf->base_reset = 1;

#ifdef LINUX_SMP
    sfInitProcPidStats(&(sfBase.sfProcPidStats));
#endif
}

static void WriteTimeStamp(FILE *fh, const char *action)
{
    time_t curr_time = time(NULL);

    if (fh == NULL)
        return;

    fprintf(fh,
        "################################### "
        "Perfmon %s: pid=%u at=%.24s (%lu) "
        "###################################\n",
        action, getpid(),
        ctime(&curr_time), (unsigned long)curr_time);

    fflush(fh);
}

FILE * sfOpenBaseStatsFile(const char *file)
{
    static bool start_up = true;
    FILE *fh = NULL;
    bool reload = false;

#ifdef SNORT_RELAOD
    reload = perfmon_reload_status != PERF_NOT_RELOADING;
#endif

#ifndef WIN32

    // This file needs to be readable by everyone
    mode_t old_umask = umask(022);
#endif

    if (file != NULL)
    {
        // Append to the existing file if just starting up or reloading, otherwise we've
        // rotated so start a new one.
        bool append = start_up || reload;
        fh = fopen(file, append ? "a" : "w");
        if (fh != NULL)
        {
            WriteTimeStamp(fh, append ? "start" : "rotate");
            LogBasePerfHeader(fh);
        }
    }

#ifndef WIN32
    umask(old_umask);
#endif

    if (start_up)
        start_up = false;

    return fh;
}

void sfCloseBaseStatsFile(SFPERF *sfPerf)
{
    if (sfPerf->fh == NULL)
        return;

    WriteTimeStamp(sfPerf->fh, "stop");
    fclose(sfPerf->fh);
    sfPerf->fh = NULL;
}

FILE * sfOpenFlowStatsFile(const char *file)
{
    static bool start_up = true;
    FILE *fh = NULL;
    bool  reload = false;

#ifdef SNORT_RELOAD
    reload = perfmon_reload_status != PERF_NOT_RELOADING;
#endif

#ifndef WIN32
    // This file needs to be readable by everyone
    mode_t old_umask = umask(022);
#endif

    if (file != NULL)
    {
        // Append to the existing file if just starting up or reloading, otherwise we've
        // rotated so start a new one.
        bool append = start_up || reload;
        fh = fopen(file, append ? "a" : "w");
        if (fh != NULL)
        {
            WriteTimeStamp(fh, append ? "start" : "rotate");
            LogFlowPerfHeader(fh);
        }
    }

#ifndef WIN32
    umask(old_umask);
#endif

    if (start_up)
        start_up = false;

    return fh;
}

void sfCloseFlowStatsFile(SFPERF *sfPerf)
{
    if (sfPerf->flow_fh == NULL)
        return;

    WriteTimeStamp(sfPerf->flow_fh, "stop");
    fclose(sfPerf->flow_fh);
    sfPerf->flow_fh = NULL;
}

FILE * sfOpenFlowIPStatsFile(const char *file)
{
    static bool start_up = true;
    FILE *fh = NULL;
    bool  reload = false;

#ifdef SNORT_RELOAD
    reload = perfmon_reload_status != PERF_NOT_RELOADING;
#endif

#ifndef WIN32
    // This file needs to be readable by everyone
    mode_t old_umask = umask(022);
#endif

    if (file != NULL)
    {
        // Append to the existing file if just starting up or reloading, otherwise we've
        // rotated so start a new one.
        fh = fopen(file, start_up || reload ? "a" : "w");
    }

#ifndef WIN32
    umask(old_umask);
#endif

    if (start_up)
        start_up = false;

    return fh;
}

void sfCloseFlowIPStatsFile(SFPERF *sfPerf)
{
    if (sfPerf->flowip_fh == NULL)
        return;

    fclose(sfPerf->flowip_fh);
    sfPerf->flowip_fh = NULL;
}

static int sfRotateFile(const char *old_file, FILE *old_fh,
        const char *rotate_prefix, uint32_t max_file_size)
{
    time_t ts;
    struct tm *tm;
    char rotate_file[PATH_MAX];
    char *path_ptr;
    int path_len = 0;
    struct stat file_stats;

    if (old_file == NULL)
        return -1;

    if (old_fh == NULL)
    {
        ErrorMessage("Perfmonitor: Performance stats file \"%s\" "
                "isn't open.\n", old_file);
        return -1;
    }

    // Close the current stats file if it's already open
    fclose(old_fh);
    old_fh = NULL;

    // Rename current stats file with yesterday's date
#ifndef WIN32
    path_ptr = strrchr(old_file, '/');
#else
    path_ptr = strrchr(old_file, '\\');
#endif

    if (path_ptr != NULL)
    {
        // Take the length of the file/path name up to the path separator and
        // add one to include path separator
        path_len = (path_ptr - old_file) + 1;
    }

    // Get current time, then subtract one day to get yesterday
    ts = time(NULL);
    ts -= (24*60*60);
    tm = localtime(&ts);

    // Create rotate file name based on path, optional prefix and date
    SnortSnprintf(rotate_file, PATH_MAX, "%.*s%s%s%d-%02d-%02d", path_len, old_file,
            (rotate_prefix != NULL) ? rotate_prefix : "", (rotate_prefix != NULL) ? "-" : "",
            tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);

    // If the rotate file doesn't exist, just rename the old one to the new one
    if (stat(rotate_file, &file_stats) != 0)
    {
        if (rename(old_file, rotate_file) != 0)
        {
            ErrorMessage("Perfmonitor: Could not rename performance stats "
                    "file from \"%s\" to \"%s\": %s.\n",
                    old_file, rotate_file, strerror(errno));
        }
    }
    else  // Otherwise, if it does exist, append data from current stats file to it
    {
        char read_buf[4096];
        size_t num_read, num_wrote;
        FILE *rotate_fh;
        int rotate_index = 0;
        char rotate_file_with_index[PATH_MAX];
#ifndef WIN32
        // This file needs to be readable by everyone
        mode_t old_umask = umask(022);
#endif

        do
        {
            do
            {
                rotate_index++;

                // Check to see if there are any files already rotated and indexed
                SnortSnprintf(rotate_file_with_index, PATH_MAX, "%s.%02d",
                        rotate_file, rotate_index);

            } while (stat(rotate_file_with_index, &file_stats) == 0);

            // Subtract one to append to last existing file
            rotate_index--;

            if (rotate_index == 0)
            {
                rotate_file_with_index[0] = 0;
                rotate_fh = fopen(rotate_file, "a");
            }
            else
            {
                SnortSnprintf(rotate_file_with_index, PATH_MAX, "%s.%02d",
                        rotate_file, rotate_index);
                rotate_fh = fopen(rotate_file_with_index, "a");
            }

            if (rotate_fh == NULL)
            {
                ErrorMessage("Perfmonitor: Could not open performance stats "
                        "archive file \"%s\" for appending: %s.\n",
                        *rotate_file_with_index ? rotate_file_with_index : rotate_file,
                        strerror(errno));
                break;
            }

            old_fh = fopen(old_file, "r");
            if (old_fh == NULL)
            {
                ErrorMessage("Perfmonitor: Could not open performance stats file "
                        "\"%s\" for reading to copy to archive \"%s\": %s.\n",
                        old_file, (*rotate_file_with_index ? rotate_file_with_index :
                        rotate_file), strerror(errno));
                break;
            }

            while (!feof(old_fh))
            {
                // This includes the newline from the file.
                if (fgets(read_buf, sizeof(read_buf), old_fh) == NULL)
                {
                    if (feof(old_fh))
                        break;

                    if (ferror(old_fh))
                    {
                        // A read error occurred
                        ErrorMessage("Perfmonitor: Error reading performance stats "
                                "file \"%s\": %s.\n", old_file, strerror(errno));
                        break;
                    }
                }

                num_read = strlen(read_buf);

                if (num_read > 0)
                {
                    int rotate_fd = fileno(rotate_fh);

                    if (fstat(rotate_fd, &file_stats) != 0)
                    {
                        ErrorMessage("Perfmonitor: Error getting file "
                                "information for \"%s\": %s.\n",
                                *rotate_file_with_index ? rotate_file_with_index : rotate_file,
                                strerror(errno));
                        break;
                    }

                    if (((uint32_t)file_stats.st_size + num_read) > max_file_size)
                    {
                        fclose(rotate_fh);

                        rotate_index++;

                        // Create new file same as before but with an index added to the end
                        SnortSnprintf(rotate_file_with_index, PATH_MAX, "%s.%02d",
                                rotate_file, rotate_index);

                        rotate_fh = fopen(rotate_file_with_index, "a");
                        if (rotate_fh == NULL)
                        {
                            ErrorMessage("Perfmonitor: Could not open performance "
                                    "stats archive file \"%s\" for writing: %s.\n",
                                    rotate_file_with_index, strerror(errno));
                            break;
                        }
                    }

                    num_wrote = fprintf(rotate_fh, "%s", read_buf);
                    if ((num_wrote != num_read) && ferror(rotate_fh))
                    {
                        // A bad write occurred
                        ErrorMessage("Perfmonitor: Error writing to performance "
                                "stats archive file \"%s\": %s.\n", rotate_file, strerror(errno));
                        break;
                    }

                    fflush(rotate_fh);
                }
            }
        } while (0);

        if (rotate_fh != NULL)
            fclose(rotate_fh);

        if (old_fh != NULL)
            fclose(old_fh);

#ifndef WIN32
        umask(old_umask);
#endif
    }

    return 0;
}

int sfRotateBaseStatsFile(SFPERF *sfPerf)
{
    if ((sfPerf != NULL) && (sfPerf->file != NULL))
    {
        int ret = sfRotateFile(sfPerf->file, sfPerf->fh, NULL, sfPerf->max_file_size);
        if (ret != 0)
            return ret;

        if ((sfPerf->fh = sfOpenBaseStatsFile(sfPerf->file)) == NULL)
        {
            FatalError("Cannot open performance stats file \"%s\": %s.\n",
                    sfPerf->file, strerror(errno));
        }
    }

    return 0;
}

int sfRotateFlowStatsFile(SFPERF *sfPerf)
{
    if ((sfPerf != NULL) && (sfPerf->flow_file != NULL))
    {
        int ret = sfRotateFile(sfPerf->flow_file, sfPerf->flow_fh, "flow", sfPerf->max_file_size);
        if (ret != 0)
            return ret;

        if ((sfPerf->flow_fh = sfOpenFlowStatsFile(sfPerf->flow_file)) == NULL)
        {
            FatalError("Perfmonitor: Cannot open flow stats file \"%s\": %s.\n",
                    sfPerf->flow_file, strerror(errno));
        }
    }

    return 0;
}

static inline int sfRotateFlowIPStatsFile(SFPERF *sfPerf)
{
    if ((sfPerf != NULL) && (sfPerf->flowip_file != NULL))
    {
        int ret = sfRotateFile(sfPerf->flowip_file, sfPerf->flowip_fh, "flow-ip", sfPerf->max_file_size);
        if (ret != 0)
            return ret;

        if ((sfPerf->flowip_fh = sfOpenFlowIPStatsFile(sfPerf->flowip_file)) == NULL)
        {
            FatalError("Perfmonitor: Cannot open flow-ip stats file \"%s\": %s.\n",
                    sfPerf->flowip_file, strerror(errno));
        }
    }

    return 0;
}

void sfPerformanceStats(SFPERF *sfPerf, Packet *p)
{
    // Update stats first since other stats from various places like frag3 and
    // stream5 have been added.
    UpdatePerfStats(sfPerf, p);

    if ((sfPerf->perf_flags & SFPERF_TIME_COUNT) && !PacketIsRebuilt(p))
    {
        pkt_cnt++;
        sfPerformanceStatsOOB(sfPerf, p->pkth->ts.tv_sec);
    }
}

void sfPerformanceStatsOOB(SFPERF *sfPerf, time_t curr_time)
{
    if (sfPerf && pkt_cnt >= sfPerf->pkt_cnt) //For perfect alignment, sfPerf->pkt_count should be set to 0 in config
    {
        if (CheckSampleInterval(sfPerf, curr_time))
        {
            pkt_cnt = 0;

            if (!(sfPerf->perf_flags & SFPERF_SUMMARY_BASE))
            {
                if (sfPerf->perf_flags & SFPERF_BASE)
                {
                    sfProcessBaseStats(sfPerf);
                    InitBaseStats(&sfBase);
                }
            }

            if (!(sfPerf->perf_flags & SFPERF_SUMMARY_FLOW))
            {
                if (sfPerf->perf_flags & SFPERF_FLOW)
                {
                    sfProcessFlowStats(sfPerf);
                    InitFlowStats(&sfFlow);
                }
            }

            if (!(sfPerf->perf_flags & SFPERF_SUMMARY_FLOWIP))
            {
                if (sfPerf->perf_flags & SFPERF_FLOWIP)
                {
                    sfProcessFlowIpStats(sfPerf);
                    InitFlowIPStats(&sfFlow);
                }
            }

            if (!(sfPerf->perf_flags & SFPERF_SUMMARY_EVENT))
            {
                if (sfPerf->perf_flags & SFPERF_EVENT)
                {
                    sfProcessEventStats(sfPerf);
                    InitEventStats(&sfEvent);
                }
            }
        }
    }
}

static bool CheckSampleInterval(SFPERF *sfPerf, time_t curr_time)
{
    static time_t last_true = 0;

    //Initial alignment
    if (last_true == 0)
    {
        last_true = curr_time;
        last_true -= last_true % sfPerf->sample_interval;
    }

    //Dump on aligned time if possible. If not, get close and say we did.
    if (curr_time - last_true >= sfPerf->sample_interval)
    {
        curr_time -= curr_time % sfPerf->sample_interval;
        sfBase.time = curr_time;
        sfFlow.time = curr_time;
        last_true = curr_time;
        return true;
    }

    return false;
}

void InitPerfStats(SFPERF *sfPerf)
{
#ifdef LINUX_SMP
    memset(&sfBase, 0, offsetof(SFBASE, sfProcPidStats));
#else
    memset(&sfBase, 0, sizeof(SFBASE));
#endif
    memset(&sfFlow, 0, sizeof(SFFLOW));
    memset(&sfEvent, 0, sizeof(SFEVENT));
    if (sfPerf->perf_flags & SFPERF_BASE)
        InitBaseStats(&sfBase);

    if (sfPerf->perf_flags & SFPERF_FLOW)
        InitFlowStats(&sfFlow);

    if (sfPerf->perf_flags & SFPERF_FLOWIP)
        InitFlowIPStats(&sfFlow);

    if (sfPerf->perf_flags & SFPERF_EVENT)
        InitEventStats(&sfEvent);
}

static void UpdatePerfStats(SFPERF *sfPerf, Packet *p)
{
    bool rebuilt = PacketIsRebuilt(p);

    if (sfPerf->perf_flags & SFPERF_BASE)
        UpdateBaseStats(&sfBase, p, rebuilt);

    if ((sfPerf->perf_flags & SFPERF_FLOW) && !rebuilt)
        UpdateFlowStats(&sfFlow, p);

    if ((sfPerf->perf_flags & SFPERF_FLOWIP) && IsIP(p) && !rebuilt)
    {
        SFSType type = SFS_TYPE_OTHER;

        if (p->tcph != NULL)
            type = SFS_TYPE_TCP;
        else if (p->udph != NULL)
            type = SFS_TYPE_UDP;

        UpdateFlowIPStats(&sfFlow, GET_SRC_IP(p), GET_DST_IP(p), p->pkth->caplen, type);
    }
}

static inline bool sfCheckFileSize(FILE *fh, uint32_t max_file_size)
{
    int fd;
    struct stat file_stats;

    if (fh == NULL)
        return false;

    fd = fileno(fh);
    if ((fstat(fd, &file_stats) == 0)
            && ((uint32_t)file_stats.st_size >= max_file_size))
        return true;

    return false;
}

static inline void sfProcessBaseStats(SFPERF *sfPerf)
{
    if (!(sfPerf->perf_flags & SFPERF_BASE))
        return;

    ProcessBaseStats(&sfBase, sfPerf->fh,
            sfPerf->perf_flags & SFPERF_CONSOLE,
            sfPerf->perf_flags & SFPERF_MAX_BASE_STATS);

    if ((sfPerf->fh != NULL)
            && sfCheckFileSize(sfPerf->fh, sfPerf->max_file_size))
    {
        sfRotateBaseStatsFile(sfPerf);
    }
}

static inline void sfProcessFlowStats(SFPERF *sfPerf)
{
    if (!(sfPerf->perf_flags & SFPERF_FLOW))
        return;

    ProcessFlowStats(&sfFlow, sfPerf->flow_fh,
            sfPerf->perf_flags & SFPERF_CONSOLE);

    if ((sfPerf->flow_fh != NULL)
            && sfCheckFileSize(sfPerf->flow_fh, sfPerf->max_file_size))
    {
        sfRotateFlowStatsFile(sfPerf);
    }
}

static inline void sfProcessFlowIpStats(SFPERF *sfPerf)
{
    if (!(sfPerf->perf_flags & SFPERF_FLOWIP))
        return;

    ProcessFlowIPStats(&sfFlow, sfPerf->flowip_fh,
            sfPerf->perf_flags & SFPERF_CONSOLE);

    if ((sfPerf->flowip_fh != NULL)
            && sfCheckFileSize(sfPerf->flowip_fh, sfPerf->max_file_size))
    {
        sfRotateFlowIPStatsFile(sfPerf);
    }
}

static inline void sfProcessEventStats(SFPERF *sfPerf)
{
    if (!(sfPerf->perf_flags & SFPERF_EVENT))
        return;

    if (sfPerf->perf_flags & SFPERF_CONSOLE)
        ProcessEventStats(&sfEvent);
}

void sfPerfStatsSummary(SFPERF *sfPerf)
{
    if (sfPerf == NULL)
        return;

    if (sfPerf->perf_flags & SFPERF_SUMMARY_BASE)
        sfProcessBaseStats(sfPerf);

    if (sfPerf->perf_flags & SFPERF_SUMMARY_FLOW)
        sfProcessFlowStats(sfPerf);

    if (sfPerf->perf_flags & SFPERF_SUMMARY_FLOWIP)
        sfProcessFlowIpStats(sfPerf);

    if (sfPerf->perf_flags & SFPERF_SUMMARY_EVENT)
        sfProcessEventStats(sfPerf);
}

static void sfInitBaseStats(SFPERF *sfPerf)
{
    if (sfPerf->perf_flags & SFPERF_BASE)
        InitBaseStats(&sfBase);
}

static void sfInitFlowStats(SFPERF *sfPerf)
{
    if (sfPerf->perf_flags & SFPERF_FLOW)
        InitFlowStats(&sfFlow);
}

static void sfInitFlowIPStats(SFPERF *sfPerf)
{
    if (sfPerf->perf_flags & SFPERF_FLOWIP)
        InitFlowIPStats(&sfFlow);
}

static void sfInitEventStats(SFPERF *sfPerf)
{
    if (sfPerf->perf_flags & SFPERF_EVENT)
        InitEventStats(&sfEvent);
}

static void sfFreeFlowStats(SFPERF *sfPerf)
{
    FreeFlowStats(&sfFlow);
}

static void sfFreeFlowIPStats(SFPERF *sfPerf)
{
    FreeFlowIPStats(&sfFlow);
}

#ifdef SNORT_RELOAD
//this function determines what is necessary to swap configs and keep stats and files synchronized
//it will perform the actions necessary durring the appropriate stage of ReloadSwap
// or you can force it to perform all actions by setting fullSync to a non-zero value
//
//if fullSync is set to 0 then actions are taken under the following conditions
//If files need to be opened they are opened durring ReloadVerify
//If stats need to be written to files and flushed it is done durring ReloadSwap
//If files need to be closed it is done durring ReloadSwapFree
static void syncStats(  SFPERF *currentSFPerf, SFPERF *newSFPerf,
                        int flag, int summaryFlag, //if summaryFlag is set then flag is set
                        char *currentFile, char *newFile,
                        FILE **currentFH, FILE **newFH,
                        void (*processFunc)(SFPERF *),
                        void (*initFunc)(SFPERF *), void (*freeFunc)(SFPERF *),
                        FILE* (*openFileFunc)(const char *), void (*closeFileFunc)(SFPERF *),
                        int fullSync)
{
    int old_flags = currentSFPerf->perf_flags;
    int new_flags = newSFPerf->perf_flags;
    int config_flags = old_flags | new_flags;

    //summaryFlag implies that flag is set
    //otherwise code breaks <- I got this implication from other sets of code (i.e. InitPerfStats)

    //these stats were recorded, but are not recorded in new config
    if (old_flags & flag && !(new_flags & flag))
    {
        //we need to flush the stats at the correct time
        if (perfmon_reload_status == PERF_RELOAD_SWAP || fullSync)
            //write stats for last period (or summary)
            (*processFunc)(currentSFPerf);
        //only close files in ReloadSwapFree to avoid latencies
        if (perfmon_reload_status == PERF_RELOAD_SWAP_FREE || fullSync)
        {
            //not every stat group has a file
            if (closeFileFunc != NULL)
                (*closeFileFunc)(currentSFPerf);
            //not every stat group uses dynamic memory
            if (freeFunc != NULL)
                (*freeFunc)(currentSFPerf);
        }
    }
    //these stats were not recorded, but will be recorded in new config
    else if ( !(old_flags & flag) && new_flags & flag )
    {
        if (perfmon_reload_status == PERF_RELOAD_VERIFY || fullSync)
        {
           //init/reset stats
           (*initFunc)(newSFPerf);
           //open up new file
           if (openFileFunc)
               *newFH = (*openFileFunc)(newFile);
        }
    }
    // both configs have flag
    else if (config_flags & flag)
    {
        //one config is not summary
       if (!(old_flags & summaryFlag && new_flags & summaryFlag))
        {
            if (perfmon_reload_status == PERF_RELOAD_SWAP || fullSync)
            {
                //log and clear stats
                (*processFunc)(currentSFPerf);
                (*initFunc)(currentSFPerf);
            }
        }
        //stat group must have log files
        if ( openFileFunc != NULL && closeFileFunc != NULL)
        {
            //the new file has a new name
            if ( newFile != NULL
                    && (currentFile == NULL || strcmp(currentFile,newFile) != 0))
            {
                if (perfmon_reload_status == PERF_RELOAD_VERIFY || fullSync)
                    *newFH = (*openFileFunc)(newFile);
                if (perfmon_reload_status == PERF_RELOAD_SWAP_FREE || fullSync)
                    (*closeFileFunc)(currentSFPerf);
            }
            //use the same file
            else if (*currentFH != NULL)
            {
                if (perfmon_reload_status == PERF_RELOAD_SWAP || fullSync)
                    *newFH = *currentFH;
            }
        }
    }
    //else neither config has stats logged
}

//you can have SFPERF_SUMMARY_FLAG without a SFPERF_FLAG, but 
//perfmon acts as though neither flag is set and doesn't do any logging
//for those statisitcs
void syncAllStats(SFPERF *currentSFPerf, SFPERF *newSFPerf)
{
    if (currentSFPerf == NULL || newSFPerf == NULL)
        return;

    //base stats
    syncStats(  currentSFPerf, newSFPerf,
                SFPERF_BASE, SFPERF_SUMMARY_BASE,
                currentSFPerf->file, newSFPerf->file,
                &(currentSFPerf->fh), &(newSFPerf->fh),
                &sfProcessBaseStats,
                &sfInitBaseStats, NULL,
                &sfOpenBaseStatsFile, &sfCloseBaseStatsFile,
                0);
    //flow stats
    syncStats(  currentSFPerf,newSFPerf,
                SFPERF_FLOW, SFPERF_SUMMARY_FLOW,
                currentSFPerf->flow_file, newSFPerf->flow_file,
                &(currentSFPerf->flow_fh), &(newSFPerf->flow_fh),
                &sfProcessFlowStats,
                &sfInitFlowStats, &sfFreeFlowStats,
                &sfOpenFlowStatsFile, &sfCloseFlowStatsFile,
                0);

   //flowip stats
    syncStats(  currentSFPerf, newSFPerf,
                SFPERF_FLOWIP, SFPERF_SUMMARY_FLOWIP,
                currentSFPerf->flowip_file, newSFPerf->flowip_file,
                &(currentSFPerf->flowip_fh), &(newSFPerf->flowip_fh),
                &sfProcessFlowIpStats,
                &sfInitFlowIPStats, &sfFreeFlowIPStats,
                &sfOpenFlowIPStatsFile, &sfCloseFlowIPStatsFile,
                0);

   //event stats
   syncStats(   currentSFPerf, newSFPerf,
                SFPERF_EVENT, SFPERF_SUMMARY_EVENT,
                NULL, NULL,
                NULL, NULL,
                &sfProcessEventStats,
                &sfInitEventStats, NULL,
                NULL, NULL,
                0);
}
#endif

