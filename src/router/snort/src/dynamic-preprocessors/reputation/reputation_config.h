/****************************************************************************
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
 ****************************************************************************
 * Provides convenience functions for parsing and querying configuration.
 *
 * 6/11/2011 - Initial implementation ... Hui Cao <hcao@sourcefire.com>
 *
 ****************************************************************************/

#ifndef _REPUTATION_CONFIG_H_
#define _REPUTATION_CONFIG_H_
#include "sf_types.h"
#include "sfPolicyUserData.h"
#include "snort_bounds.h"
#include "reputation_debug.h"
#include "sf_ip.h"
#include "sfrt_flat.h"
#ifdef SHARED_REP
#include "./shmem/shmem_mgmt.h"
#endif
#define REPUTATION_NAME  "reputation"

typedef enum _NestedIP
{
    INNER,
    OUTER,
    BOTH
}NestedIP;

typedef enum _WhiteAction
{
    UNBLACK,
    TRUST
}WhiteAction;

typedef struct _SharedMem
{
    char *path;
    uint32_t updateInterval;
    uint16_t maxInstances;
}SharedMem;


typedef enum _IPdecision
{
    DECISION_NULL ,
    MONITORED,
    BLACKLISTED ,
    WHITELISTED_UNBLACK,
    WHITELISTED_TRUST,
    DECISION_MAX
}IPdecision;


typedef struct _ListInfo{
    uint8_t       listIndex;
    uint8_t       listType;
    uint32_t      listId;
#ifdef SHARED_REP
    bool zones[MAX_NUM_ZONES];
    char padding[2 + MAX_NUM_ZONES - MAX_NUM_ZONES/4*4];
#endif

} ListInfo;

/*
 * Reputation preprocessor configuration.
 *
 * memcap:  the memcap for IP table.
 * numEntries: number of entries in the table
 * scanlocal: to scan local network
 * prioirity: the priority of whitelist, blacklist
 * nestedIP: which IP address to use when IP encapsulation
 * iplist: the IP table
 * ref_count: reference account
 */
typedef struct _reputationConfig
{
    uint32_t memcap;
    int numEntries;
    uint8_t  scanlocal;
    IPdecision priority;
    NestedIP nestedIP;
    WhiteAction whiteAction;
    MEM_OFFSET local_black_ptr;
    MEM_OFFSET local_white_ptr;
    void *emptySegment;
    void *localSegment;
    SharedMem sharedMem;
    int segment_version;
    uint32_t memsize;
    bool memCapReached;
    table_flat_t *iplist;
    ListInfo *listInfo;
    int ref_count;
    char *statusBuf;
    int  statusBuf_len;

} ReputationConfig;


#define NUM_INDEX_PER_ENTRY 4

typedef struct _IPrepInfo{
    char listIndexes[NUM_INDEX_PER_ENTRY];
    MEM_OFFSET    next;
} IPrepInfo;


/********************************************************************
 * Public function prototypes
 ********************************************************************/
void  Reputation_FreeConfig(ReputationConfig *);
void  ParseReputationArgs(ReputationConfig *, u_char*);
void initShareMemory(struct _SnortConfig *sc, void *config);
void ReputationRepInfo(IPrepInfo *, uint8_t *, char *, int);
DEBUG_WRAP(void ReputationPrintRepInfo(IPrepInfo * repInfo, uint8_t *base);)
#endif
