/*
** $Id$
**
** perf.h
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
**    These are the basic functions and structures that are needed to call 
**    performance functions.
**
** Dan Roelker
**
**
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
*/

#ifndef _PERF_H
#define _PERF_H

#define SFPERF_BASE         0x0001
#define SFPERF_FLOW         0x0002
#define SFPERF_EVENT        0x0004
#define SFPERF_BASE_MAX     0x0008
#define SFPERF_CONSOLE      0x0010
#define SFPERF_FILE         0x0020
#define SFPERF_PKTCNT       0x0040
#define SFPERF_SUMMARY      0x0080
#define SFPERF_FILECLOSE    0x0100
#define SFPERF_FLOWIP       0x0200

#include "perf-base.h"
#include "perf-flow.h"
#include "perf-event.h"
#include "debug.h"
#include "decode.h"

#define LINUX_FILE_LIMIT    0x80000000  /* 2 GB */
#define ROLLOVER_THRESH     0x200       /* 512 bytes */
#define MAX_PERF_FILE_SIZE (LINUX_FILE_LIMIT)
#define MIN_PERF_FILE_SIZE  0x1000      /* 4096 bytes */


/* The perfmonitor configuration */
typedef struct _SFPERF
{
    int perf_flags;
    unsigned int pkt_cnt;
    int sample_interval;
    time_t sample_time;
    char *file;
    FILE *fh;
    int base_flags;
    int base_reset;
    int flow_max_port_to_track;
    uint32_t max_file_size;
    char *flowip_file;
    FILE *flowip_fh;
    uint32_t flowip_memcap;
} SFPERF;


extern SFBASE sfBase;
extern SFFLOW sfFlow;
extern SFEVENT sfEvent;
extern SFPERF* perfmon_config;
extern int perfmon_rotate_perf_file;

int sfInitPerformanceStatistics(SFPERF *);
int sfSetPerformanceSampleTime(SFPERF *, int);
int sfSetPerformanceAccounting(SFPERF *, int);
int sfSetPerformanceStatistics(SFPERF *, int);
int sfSetPerformanceStatisticsEx(SFPERF *, int, void *);
int sfOpenFlowIPStatsFile(SFPERF *sfPerf);
void sfCloseFlowIPStatsFile(SFPERF *sfPerf);
int sfSetMaxFileSize(SFPERF *sfPerf, uint32_t iSize);
int sfRotatePerformanceStatisticsFile(void);
int sfPerformanceStats(SFPERF *, Packet *, int);
int sfProcessPerfStats(SFPERF *);
int CheckSampleInterval(Packet *, SFPERF *);
int ResetPerfStats(SFPERF *);

static INLINE void SetRotatePerfFileFlag(void)
{
    perfmon_rotate_perf_file = 1;
}

static INLINE int IsSetRotatePerfFileFlag(void)
{
    return perfmon_rotate_perf_file;
}

static INLINE void ClearRotatePerfFileFlag(void)
{
    perfmon_rotate_perf_file = 0;
}

#endif
