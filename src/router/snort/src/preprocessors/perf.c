/*
**  $Id$
**
**  perf.c
**
** Copyright (C) 2002-2011 Sourcefire, Inc.
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
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
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

#include "util.h"
#include "perf.h"
#include "sf_types.h"
#include "decode.h"
#include "snort.h"

SFBASE sfBase;
SFFLOW sfFlow;
SFEVENT sfEvent;
int perfmon_rotate_perf_file = 0;

extern SFPERF *perfmon_config;


int InitPerfStats(SFPERF *sfPerf, Packet *p);
int UpdatePerfStats(SFPERF *sfPerf, const unsigned char *pucPacket, uint32_t len,
        int iRebuiltPkt);
int ProcessPerfStats(SFPERF *sfPerf);


int sfInitPerformanceStatistics(SFPERF *sfPerf)
{
    memset(sfPerf, 0x00, sizeof(SFPERF));
    sfSetPerformanceSampleTime(sfPerf, 0);
    sfSetPerformanceStatistics(sfPerf, 0);

    return 0;
}

int sfSetMaxFileSize(SFPERF *sfPerf, uint32_t iSize)
{
    sfPerf->max_file_size = iSize;

    return 0;
}

int sfSetPerformanceSampleTime(SFPERF *sfPerf, int iSeconds)
{
    sfPerf->sample_time = 0;
    
    if(iSeconds < 0)
    {
        iSeconds = 0;
    }

    sfPerf->sample_interval = iSeconds;

    return 0;
}


int sfSetPerformanceAccounting(SFPERF *sfPerf, int iReset)
{
    sfPerf->base_reset = iReset;
    
    return 0;
}


int sfSetPerformanceStatistics(SFPERF *sfPerf, int iFlag)
{
    if(iFlag & SFPERF_BASE)
    {
        sfPerf->perf_flags = sfPerf->perf_flags | SFPERF_BASE;
    }

#ifndef LINUX_SMP

    if(iFlag & SFPERF_BASE_MAX)
    {
        sfPerf->base_flags |= MAX_PERF_STATS;
    }

#endif

    if(iFlag & SFPERF_FLOW)
    {
        sfPerf->perf_flags = sfPerf->perf_flags | SFPERF_FLOW;
    }
    if(iFlag & SFPERF_FLOWIP)
    {
        sfPerf->perf_flags = sfPerf->perf_flags | SFPERF_FLOWIP;
    }
    if(iFlag & SFPERF_EVENT)
    {
        sfPerf->perf_flags = sfPerf->perf_flags | SFPERF_EVENT;
    }
    if(iFlag & SFPERF_CONSOLE)
    {
        sfPerf->perf_flags = sfPerf->perf_flags | SFPERF_CONSOLE;
    }
    
    return 0;
}

int sfSetPerformanceStatisticsEx(SFPERF *sfPerf, int iFlag, void * p)
{
#ifndef WIN32    
    mode_t old_umask;
#endif 
    
    if(iFlag & SFPERF_FILE)
    {
        static char start_up = 1;

        sfPerf->perf_flags = sfPerf->perf_flags | SFPERF_FILE;
        
        /* this file needs to be readable by everyone */
#ifndef WIN32
        old_umask = umask(022);
#endif         

        /* append to existing perfmon file if just starting up */
        if (start_up)
        {
            sfPerf->fh = fopen(sfPerf->file, "a");
            start_up = 0;
        }
        /* otherwise we've rotated - start a new one */
        else
        {
            sfPerf->fh = fopen(sfPerf->file, "w");
        }

#ifndef WIN32
        umask(old_umask);
#endif
        
        if( !sfPerf->fh )
            return -1;
    }
    else if(iFlag & SFPERF_FILECLOSE)
    {
        if (sfPerf->fh)
        {
            fclose(sfPerf->fh);
            sfPerf->fh = NULL;
        }
    }
    else if(iFlag & SFPERF_PKTCNT)
    {
        sfPerf->pkt_cnt = *(int*)p;
    }
    else if (iFlag & SFPERF_SUMMARY)
    {
        sfPerf->perf_flags |= SFPERF_SUMMARY;
    }
    return 0;
}

int sfOpenFlowIPStatsFile(SFPERF *sfPerf)
{
#ifndef WIN32
    mode_t old_umask;
#endif
    static char start_up = 1;

    /* this file needs to be readable by everyone */
#ifndef WIN32
    old_umask = umask(022);
#endif

    /* append to existing perfmon file if just starting up */
    if (start_up)
    {
        sfPerf->flowip_fh = fopen(sfPerf->flowip_file, "a");
        start_up = 0;
    }
    /* otherwise we've rotated - start a new one */
    else
    {
        sfPerf->flowip_fh = fopen(sfPerf->flowip_file, "w");
    }

#ifndef WIN32
    umask(old_umask);
#endif

    if (!sfPerf->flowip_fh)
        return -1;

    return 0;
}

void sfCloseFlowIPStatsFile(SFPERF *sfPerf)
{
    if (sfPerf->flowip_fh)
    {
        fclose(sfPerf->flowip_fh);
        sfPerf->flowip_fh = NULL;
    }
}

static INLINE FILE *OpenPerfFileAndCheckMaxSize(struct tm *tm,
                                                FILE *oldfh,
                                                SFPERF *sfPerf,
                                                struct stat *file_stats,
                                                int prefix_len, 
                                                int *file_index,
                                                int *newfd,
                                                const char *filename,
                                                uint32_t max_file_size)
{
    char       newfile[PATH_MAX];
    FILE *newfh = oldfh;
    while ((uint32_t)file_stats->st_size > max_file_size)
    {
        if (newfh)
            fclose(newfh);

        /* Go to a new file */
        SnortSnprintf(newfile, PATH_MAX, "%.*s%d-%02d-%02d.%d",
          prefix_len, filename, tm->tm_year + 1900,
          tm->tm_mon + 1, tm->tm_mday, *file_index);

        (*file_index)++;

        newfh = fopen(newfile, "a");
        if (newfh == NULL)
        {
            LogMessage("Cannot open performance log archive file "
                       "'%s' for writing: %s\n",
                               newfile, strerror(errno));
            break;
        }
        stat(newfile, file_stats);
    }
    if (newfh)
        *newfd = fileno(newfh);
    else
        *newfd = -1;

    return newfh;
}

static int sfRotateFile(const char *filename, FILE *oldfh)
{
    int        ret;
    time_t     t;
    struct tm *tm;
    char       newfile[PATH_MAX];
    char      *ptr;
    int        prefix_len = 0;
    struct stat  file_stats;

    if (filename == NULL)
        return 1;

    /* Close current stats file - if it is open already */
    if (!oldfh)
    {
        LogMessage("Performance log file '%s' not open", filename);
        return 1;
    }
    
    ret = fclose(oldfh);
    if (ret != 0)
    {
        FatalError("Cannot close performance log file '%s': %s\n", filename, strerror(errno));
    }
    
    /* Rename current stats file with yesterday's date */
#ifndef WIN32
    ptr = strrchr(filename, '/');
#else
    ptr = strrchr(filename, '\\');
#endif

    if (ptr != NULL)
    {
        /* take length of string up to path separator and add 
         * one to include path separator */
        prefix_len = (ptr - &filename[0]) + 1;
    }

    /* Get current time, then subtract one day to get yesterday */
    t = time(&t);
    t -= (24*60*60);
    tm = localtime(&t);
    SnortSnprintf(newfile, PATH_MAX, "%.*s%d-%02d-%02d",
                  prefix_len, filename, tm->tm_year + 1900,
                  tm->tm_mon + 1, tm->tm_mday);

    /* Checking return code from rename */
    if (stat(newfile, &file_stats) == -1)
    {
        /* newfile doesn't exist - just rename filename to newfile */
        if(rename(filename, newfile) != 0)
        {
            LogMessage("Cannot move performance log file '%s' to '%s': %s\n",
                       filename, newfile, strerror(errno));
        }
    }
    else
    {
        /* append to current archive file */
        FILE *newfh = NULL, *curfh;
        char read_buf[1024];
        size_t num_read, num_wrote;
        int file_index = 0;
        int newfd = -1;

#ifndef WIN32    
        mode_t old_umask;
        old_umask = umask(022);
#endif         
        do
        {
            newfh = fopen(newfile, "a");
            if (newfh == NULL)
            {
                LogMessage("Cannot open performance log archive file '%s' for writing: %s\n", newfile, strerror(errno));
                break;
            }

            newfh = OpenPerfFileAndCheckMaxSize(tm, newfh, perfmon_config, &file_stats,
                                                prefix_len, &file_index, &newfd, filename, perfmon_config->max_file_size);

            if (newfh == NULL)
            {
                /* Already logged error message */
                break;
            }

            curfh = fopen(filename, "r");
            if (curfh == NULL)
            {
                LogMessage("Cannot open performance log file '%s' for reading: %s\n", filename, strerror(errno));
                fclose(newfh);
                break;
            }

            while (!feof(curfh))
            {
                /* This includes the newline from the file. */
                if (fgets(read_buf, sizeof(read_buf), curfh) == NULL)
                {
                    if (feof(curfh))
                    {
                        break;
                    }

                    if (ferror(curfh))
                    {
                        /* a read error occurred */
                        LogMessage("Error reading performance log file '%s': %s\n", filename, strerror(errno));
                        break;
                    }
                }

                num_read = strlen(read_buf);

                if (num_read > 0)
                {
                    num_wrote = fprintf(newfh, "%s", read_buf);
                    if (num_wrote != num_read)
                    {
                        if (ferror(newfh))
                        {
                            /* a bad write occurred */
                            LogMessage("Error writing to performance log archive file '%s': %s\n", newfile, strerror(errno));
                            break;
                        }
                    }

                    fflush(newfh);
                    fstat(newfd, &file_stats);
                    newfh = OpenPerfFileAndCheckMaxSize(tm, newfh, perfmon_config, &file_stats, prefix_len,
                                                        &file_index, &newfd, filename, perfmon_config->max_file_size);
                    if (newfh == NULL)
                    {
                        /* Already logged error message */
                        break;
                    }
                }
            }

            if (newfh != NULL)
                fclose(newfh);
            fclose(curfh);

        } while (0);
#ifndef WIN32
        umask(old_umask);
#endif
    }

    return 0;
}

int sfRotatePerformanceStatisticsFile(void)
{
    int ret = 0;
   
    if (perfmon_config != NULL)
    {
        ret = sfRotateFile(perfmon_config->file, perfmon_config->fh);

        if (ret != 0)
            return ret;

        if (sfSetPerformanceStatisticsEx(perfmon_config, SFPERF_FILE, perfmon_config->file) != 0)
            FatalError("Cannot open performance log file '%s': %s\n", perfmon_config->file, strerror(errno));
    }

    return 0;
}

int sfRotateFlowIPStatisticsFile(void)
{
    int ret;
    
    ret = sfRotateFile(perfmon_config->flowip_file, perfmon_config->flowip_fh);
    if (ret != 0)
        return ret;

    if (sfOpenFlowIPStatsFile(perfmon_config) != 0)
        FatalError("Cannot open performance log file '%s': %s\n", perfmon_config->file, strerror(errno));

    return 0;
}

int sfPerformanceStats(SFPERF *sfPerf, Packet *p, int iRebuiltPkt)
{
    static unsigned int cnt=0;

    if (( cnt==0 || cnt >= sfPerf->pkt_cnt ) &&
        !(sfPerf->perf_flags & SFPERF_SUMMARY))
    {
        cnt=1;
        CheckSampleInterval(p, sfPerf);
    }

    cnt++;

    UpdatePerfStats(sfPerf, p->pkt, p->pkth->caplen, iRebuiltPkt);

    return 0;
}

int CheckSampleInterval(Packet *p, SFPERF *sfPerf)
{
    time_t prev_time = sfPerf->sample_time;
    time_t curr_time = time(NULL);

    if (ScReadMode())
    {
        if (p)
        {
            curr_time = p->pkth->ts.tv_sec;
            sfBase.time = p->pkth->ts.tv_sec;
        }
        else
        {
            sfBase.time = curr_time;
        }
    }

    /*
    *  This is for when sfBasePerformance is
    *  starting up.
    */
    if(prev_time == 0)
    {
        InitPerfStats(sfPerf, p);

        /****** Log an empty line in the file to indicate a restart *********/
        if ((sfPerf->perf_flags & SFPERF_BASE) && (sfPerf->perf_flags & SFPERF_FILE))
        {
            if( sfPerf->fh )
            {
                fprintf(sfPerf->fh,
                    "################################### "
                    "New Log File: %lu"
                    "###################################\n",
                    (unsigned long)curr_time);
                fflush(sfPerf->fh);
            }
        }
    }
    else if((curr_time - prev_time) >= sfPerf->sample_interval)
    {
        ProcessPerfStats(sfPerf);
        InitPerfStats(sfPerf, p);
    }

    return 0;
}

int InitPerfStats(SFPERF *sfPerf, Packet *p)
{
    /*
    *  Reset sample time for next sampling
    */
    if (!ScReadMode())
    {
        sfPerf->sample_time = time(NULL);
    }
    else
    {
        if (p)
            sfPerf->sample_time = p->pkth->ts.tv_sec;
        else
            sfPerf->sample_time = 0;
    }

    if(sfPerf->perf_flags & SFPERF_BASE)
    {  
        InitBaseStats(&sfBase);
    }

    if(sfPerf->perf_flags & SFPERF_FLOW)
    {  
        InitFlowStats(&sfFlow);
    }

    if(sfPerf->perf_flags & SFPERF_FLOWIP)
    {  
        InitFlowIPStats(&sfFlow);
    }

    if(sfPerf->perf_flags & SFPERF_EVENT)
    {  
        InitEventStats(&sfEvent);
    }

    return 0;
}

int ResetPerfStats(SFPERF *sfPerf)
{
    return InitPerfStats(sfPerf, NULL);
}

int UpdatePerfStats(SFPERF *sfPerf, const unsigned char *pucPacket, uint32_t len,
                    int iRebuiltPkt)
{
    if(sfPerf->perf_flags & SFPERF_BASE)
    {
        UpdateBaseStats(&sfBase, len, iRebuiltPkt);
    }
    if(sfPerf->perf_flags & SFPERF_FLOW)
    {
        UpdateFlowStats(&sfFlow, pucPacket, len, iRebuiltPkt);
    }

    return 0;
}

int sfProcessPerfStats(SFPERF *sfPerf)
{
    return ProcessPerfStats(sfPerf);
}

int ProcessPerfStats(SFPERF *sfPerf)
{
    if(sfPerf->perf_flags & SFPERF_BASE)
    {
        /* Allow this to go out to console and/or a file */
        ProcessBaseStats(&sfBase,
                         sfPerf->perf_flags & SFPERF_CONSOLE,
                         sfPerf->perf_flags & SFPERF_FILE,
                         sfPerf->fh );

        /* If writing to a file, check for max file size */
        if (sfPerf->perf_flags & SFPERF_FILE)
        {
            int fd = fileno(sfPerf->fh);
            struct stat file_stats;
            if (fstat(fd, &file_stats) == 0)
            {
                if ((uint32_t)file_stats.st_size > sfPerf->max_file_size)
                {
                    sfRotatePerformanceStatisticsFile();
                }
            }
        }
    }
    
    /* Always goes to the console */
    if(sfPerf->perf_flags & SFPERF_FLOW)
    {
        if (sfPerf->perf_flags & SFPERF_CONSOLE)
            ProcessFlowStats(&sfFlow);
    }

    if (sfPerf->perf_flags & SFPERF_FLOWIP)
    {
        ProcessFlowIPStats(&sfFlow, sfPerf->flowip_fh);
        if (sfPerf->flowip_file)
        {
            int fd = fileno(sfPerf->flowip_fh);
            struct stat file_stats;
            if (fstat(fd, &file_stats) == 0)
            {
                if ((uint32_t)file_stats.st_size > sfPerf->max_file_size)
                {
                    sfRotateFlowIPStatisticsFile();
                }
            }
        }
    }
   
    if (sfPerf->perf_flags & SFPERF_EVENT)
    {
        if (sfPerf->perf_flags & SFPERF_CONSOLE)
            ProcessEventStats(&sfEvent);
    }

    return 0;
}

