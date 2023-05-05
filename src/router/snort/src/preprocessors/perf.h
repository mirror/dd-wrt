/*
** $Id$
**
** perf.h
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
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**
*/

#ifndef _PERF_H
#define _PERF_H

#include "perf-base.h"
#include "perf-flow.h"
#include "perf-event.h"
#include "sf_types.h"
#include "snort_debug.h"
#include "decode.h"

// Perf Flags
#define SFPERF_BASE             0x00000001
#define SFPERF_FLOW             0x00000002
#define SFPERF_EVENT            0x00000004
#define SFPERF_BASE_MAX         0x00000008
#define SFPERF_CONSOLE          0x00000010
#define SFPERF_PKTCNT           0x00000020
#define SFPERF_FLOWIP           0x00000040
#define SFPERF_TIME_COUNT       0x00000080
#define SFPERF_MAX_BASE_STATS   0x00000100

#define SFPERF_SUMMARY_BASE     0x00001000
#define SFPERF_SUMMARY_FLOW     0x00002000
#define SFPERF_SUMMARY_FLOWIP   0x00004000
#define SFPERF_SUMMARY_EVENT    0x00008000
#define SFPERF_SUMMARY \
    (SFPERF_SUMMARY_BASE|SFPERF_SUMMARY_FLOW|SFPERF_SUMMARY_FLOWIP|SFPERF_SUMMARY_EVENT)

#define ROLLOVER_THRESH     512
#define MAX_PERF_FILE_SIZE  INT32_MAX
#define MIN_PERF_FILE_SIZE  4096

/* The perfmonitor configuration */
typedef struct _SFPERF
{
    int perf_flags;
    uint32_t pkt_cnt;
    int sample_interval;
    char *file;
    FILE *fh;
    int base_reset;
    int flow_max_port_to_track;
    char *flow_file;
    FILE *flow_fh;
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
#ifdef SNORT_RELOAD
    typedef enum {
        PERF_NOT_RELOADING,
        PERF_RELOAD,
        PERF_RELOAD_VERIFY,
        PERF_RELOAD_SWAP,
        PERF_RELOAD_SWAP_FREE,
        PERF_RELOAD_ADUST
    } PERFRELOAD_STATUS;

    extern PERFRELOAD_STATUS perfmon_reload_status;
#endif


void sfInitPerformanceStatistics(SFPERF *);
FILE * sfOpenBaseStatsFile(const char *);
void sfCloseBaseStatsFile(SFPERF *sfPerf);
FILE * sfOpenFlowStatsFile(const char *);
void sfCloseFlowStatsFile(SFPERF *sfPerf);
FILE * sfOpenFlowIPStatsFile(const char *);
void sfCloseFlowIPStatsFile(SFPERF *sfPerf);
int sfRotateBaseStatsFile(SFPERF *sfPerf);
int sfRotateFlowStatsFile(SFPERF *sfPerf);
void sfPerformanceStats(SFPERF *, Packet *);
void sfPerformanceStatsOOB(SFPERF *, time_t);
void sfPerfStatsSummary(SFPERF *);
void SetSampleTime(SFPERF *, Packet *);
void InitPerfStats(SFPERF *sfPerf);
void syncAllStats(SFPERF *sfPerf, SFPERF *);

static inline void SetRotatePerfFileFlag(void)
{
    perfmon_rotate_perf_file = 1;
}

static inline int IsSetRotatePerfFileFlag(void)
{
    return perfmon_rotate_perf_file;
}

static inline void ClearRotatePerfFileFlag(void)
{
    perfmon_rotate_perf_file = 0;
}

#endif
