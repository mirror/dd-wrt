/* $Id$ */
/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2011-2013 Sourcefire, Inc.
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
 ****************************************************************************/

// @file    shmem_mgmt.h
// @author  Pramod Chandrashekar <pramod@sourcefire.com>

#ifndef _SHMEMMGMT_H_
#define _SHMEMMGMT_H_

#include <time.h>
#include <stdint.h>
#include "shmem_config.h"

typedef struct _shmemInstance {
    int             active; 
    int             goInactive; 
    uint32_t        version;
    time_t          updateTime;
    int             activeSegment;
    int             prevSegment;
    int             shmemSegActiveFlag[MAX_SEGMENTS];
    void*           shmemSegmentPtr[MAX_SEGMENTS];
    void*           shmemCurrPtr;
    void*           shmemZeroPtr;
} shmemInstance;

typedef struct _shmemSegment {
    int             active;
    uint32_t        version;
    uint32_t        size;
} shmemSegment;

typedef struct _shmemMgmtData {
    shmemSegment    segment[MAX_SEGMENTS];
    int             activeSegment;
    unsigned int    maxInstances;
    shmemInstance   instance[];
} ShmemMgmtData;

extern void *zeroseg_ptr;

//reader
int   InitShmemReader(uint32_t instance_num, int dataset, int group_id, int numa_node,
                      const char* path, void*** data_ptr, uint32_t instance_polltime,
                      unsigned int max_instances);
int   CheckForSharedMemSegment(unsigned int max_instances);
//writer
int   InitShmemWriter(uint32_t instance_num, int dataset, int group_id, int numa_node,
                      const char* path, void*** data_ptr, uint32_t instance_polltime,
                      unsigned int max_instances);
int   LoadSharedMemDataSegmentForWriter(int startup);
void  SwitchToActiveSegment(int segment_num,void*** data_ptr);
void  UnmapInactiveSegments(void);
void  ManageUnusedSegments(void);
int   ShutdownSharedMemory(void);
void ShmemMgmtInfo(char *buf, int bufLen);
void  PrintShmemMgmtInfo(void);

#endif

