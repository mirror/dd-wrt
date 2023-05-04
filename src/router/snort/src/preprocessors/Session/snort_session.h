/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2004-2013 Sourcefire, Inc.
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

#ifndef SNORT_SESSION_H_
#define SNORT_SESSION_H_


#include "rules.h"
#include "treenodes.h"
#include "mempool.h"

#include "session_api.h"
#include "session_common.h"

struct oneway_sessions
{
    uint32_t num_sessions;
    uint32_t prune_threshold;
    uint32_t prune_max;
    SessionControlBlock *head;
    SessionControlBlock *tail;
};

typedef struct _SessionCache
{
    SFXHASH *hashTable;
    SFXHASH_NODE *nextTimeoutEvalNode;
    uint32_t timeoutAggressive;
    uint32_t timeoutNominal;
    uint32_t max_sessions;
    uint32_t cleanup_sessions;
    uint32_t prunes;
    uint32_t flags;
    SessionCleanup cleanup_fcn;
    MemPool *protocol_session_pool;
    struct oneway_sessions ows_list;
} SessionCache;

#endif /* SNORT_SESSION_H_ */

