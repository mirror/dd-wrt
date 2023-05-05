/* $Id$ */
/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2002-2013 Sourcefire, Inc.
** Copyright (C) 2002 Martin Roesch <roesch@sourcefire.com>
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
*/


#ifndef _MEMPOOL_H
#define _MEMPOOL_H

typedef struct _MemBucket
{
    struct _MemBucket* next;
    struct _MemBucket* prev;
    void *data;
    size_t obj_size;
    void *scbPtr;
} MemBucket;

typedef struct _MemPool
{
    MemBucket* used_list_head;
    MemBucket* used_list_tail;
    MemBucket* free_list;

    size_t obj_size;
    size_t max_memory;
    size_t used_memory;
    size_t free_memory;
} MemPool;

MemBucket* mempool_get_lru_bucket(MemPool *memory_pool);
unsigned mempool_prune_freelist(MemPool *memory_pool, size_t new_max_memory, unsigned maxWork);

int mempool_init(MemPool *mempool, unsigned num_objects, size_t obj_size);
int mempool_init_optional_prealloc(MemPool *mempool, unsigned num_objects, size_t obj_size,
                                   int prealloc);
int mempool_destroy(MemPool *mempool);
MemBucket *mempool_alloc(MemPool *mempool);
MemBucket *mempool_force_alloc(MemPool *mempool);
void mempool_free(MemPool *mempool, MemBucket *obj);
int mempool_free_bucket(MemPool *mempool);
int mempool_clean(MemPool *mempool);

static inline MemBucket* mempool_oldestUsedBucket(
        MemPool *mempool
        )
{
    return mempool->used_list_head;
}
static inline unsigned int mempool_numUsedBuckets(
        MemPool *mempool
        )
{
    return (unsigned int)((mempool->used_memory + mempool->obj_size - 1) / mempool->obj_size);
}

static inline unsigned int mempool_numFreeBuckets(
        MemPool *mempool
        )
{
    return (unsigned int)((mempool->free_memory + mempool->obj_size - 1) / mempool->obj_size);
}
static inline unsigned int mempool_numTotalBuckets(
        MemPool *mempool
        )
{
    return (unsigned int)((mempool->used_memory + mempool->free_memory + mempool->obj_size - 1) / mempool->obj_size);
}

static inline void mempool_setNumObjects(
        MemPool *mempool, unsigned num_objects
        )
{
    mempool->max_memory = num_objects * mempool->obj_size; 
}

static inline void mempool_setObjectSize(
        MemPool *mempool, unsigned num_objects, size_t obj_size
        )
{
    mempool->obj_size = obj_size; 
    mempool->max_memory = num_objects * obj_size; 
}

#endif /* _MEMPOOL_H */


