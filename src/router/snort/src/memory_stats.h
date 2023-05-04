/*
 **
 **  memory_stats.h
 **
 **  Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 **  Author(s):   Puneeth Kumar C V <puneetku@cisco.com>
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
#ifndef __MEMORY_STATS_H__
#define __MEMORY_STATS_H__

#include <stdio.h>
#include "sfcontrol.h"

#define PP_MEM_CATEGORY_SESSION 0
#define PP_MEM_CATEGORY_CONFIG  1
#define PP_MEM_CATEGORY_MEMPOOL 2
#define PP_MEM_CATEGORY_MISC    3
#define PP_MEM_MAX_CATEGORY     4

#define MEMSTATS_DUMP_INTERVAL 5 * 60  //5 minutes
#define MIN_FILE_SIZE_TO_ROTATE (20 * 1024 * 1024)

typedef struct _PreprocMemNumAlloc {
    uint32_t num_of_alloc;
    uint32_t num_of_free;
    size_t   used_memory;
} PreprocMemNumAlloc;

typedef struct _PreprocMemInfo {
    uint32_t num_of_alloc;
    uint32_t num_of_free;
    size_t   used_memory;
} PreprocMemInfo;


typedef int (*MemoryStatsDisplayFunc)(FILE *fd, char *buffer, PreprocMemInfo *meminfo);

void initMemoryStatsApi();

void rotate_preproc_stats();

void MemoryPostFunction(uint16_t type, void *old_context, struct _THREAD_ELEMENT *te, ControlDataSendFunc f);

int MemoryControlFunction(uint16_t type, void *new_context, void **old_context);

int MemoryPreFunction(uint16_t type, const uint8_t *data, uint32_t length,
                        void **new_context, char *statusBuf, int statusBuf_len);
int RegisterMemoryStatsFunction(uint preproc, MemoryStatsDisplayFunc cb);

int PPMemoryStatsDumpCfg(uint16_t type, const uint8_t *data, uint32_t length,
                      void **new_context, char* statusBuf, int statusBuf_len);
void PPMemoryStatsDumpShow(uint16_t type, void *old_context,
                      struct _THREAD_ELEMENT *te, ControlDataSendFunc f);

void* SnortPreprocAlloc (int num, unsigned long size, uint32_t preproc, uint32_t sub);

void SnortPreprocFree (void *ptr, uint32_t size, uint32_t preproc, uint32_t sub);

void memory_stats_periodic_handler(FILE *fd, bool file_dump);
void dump_preproc_stats(time_t curr_time);

static inline int PopulateMemStatsBuffTrailer(char *buffer, int len, PreprocMemInfo *meminfo)
{
    return snprintf(buffer, CS_STATS_BUF_SIZE-len, "\n   Heap Memory:\n"
                "                   Session: %14zu bytes\n"
                "             Configuration: %14zu bytes\n"
                "             --------------         ------------\n"
                "              Total Memory: %14zu bytes\n"
                "              No of allocs: %14d times\n"
                "               IP sessions: %14d times\n"
                "----------------------------------------------------\n"
                , meminfo[0].used_memory
                , meminfo[1].used_memory
                , meminfo[0].used_memory + 
                                           meminfo[1].used_memory
                , meminfo[0].num_of_alloc +
                                           meminfo[1].num_of_alloc
                , meminfo[0].num_of_free + 
                                           meminfo[1].num_of_free);

}
#endif
