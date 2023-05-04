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

/*
 * This is used to allocate a list of fixed size objects in a static
 * memory pool aside from the concerns of alloc/free
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#include "mempool.h"
/*SharedObjectAddStarts
#include "sf_types.h"
#include "sf_dynamic_preprocessor.h"
SharedObjectAddEnds */

//#define TEST_MEMPOOL
#ifdef TEST_MEMPOOL
#define ErrorMessage    printf
#else
#include "util.h"
#endif
#ifdef REG_TEST
#include "reg_test.h"
#endif

static inline void mempool_free_pools(MemPool *mempool)
{
    MemBucket* item;

    if (mempool == NULL)
        return;

    while((item = mempool->used_list_head)) 
    {
        mempool->used_list_head = item->next;
        free(item);
    }
    while((item = mempool->free_list)) 
    {
        mempool->free_list = item->next; 
        free(item);
    }
}

/* Function: int mempool_init_optional_prealloc(MemPool *mempool,
 *                                              PoolCount num_objects,
 *                                              size_t obj_size,
 *                                              int prealloc)
 *
 * Purpose: initialize a mempool object and allocate memory for it
 * Args: mempool - pointer to a MemPool struct
 *       num_objects - number of items in this pool
 *       obj_size    - size of the items
 *       prealloc    - flag for preallocating the objects
 *
 * Returns: 0 on success, 1 on failure
 */

int mempool_init_optional_prealloc(MemPool *mempool, unsigned num_objects, size_t obj_size,
                                   int prealloc)
{
    unsigned i;

    if(mempool == NULL)
        return 1;

    if(num_objects < 1)
        return 1;

    if(obj_size < 1)
        return 1;

    memset(mempool, 0, sizeof(*mempool));
    mempool_setObjectSize(mempool, num_objects, obj_size);

    if (prealloc)
    {
        for (i = 0; i < num_objects; i++)
        {
            MemBucket *bp;

            if((bp = malloc(sizeof(*bp) + obj_size)) == NULL)
            {
                ErrorMessage("%s(%d) mempool_init(): membucket is null\n",
                             __FILE__, __LINE__);
                mempool_destroy(mempool);
                return 1;
            }

            bp->data = bp + 1;
            bp->obj_size = obj_size;
            bp->scbPtr = NULL;
            bp->next = mempool->free_list;
            mempool->free_list = bp;
            mempool->free_memory += obj_size;
        }
    }

    return 0;
}

/* Function: int mempool_init(MemPool *mempool,
 *                            PoolCount num_objects, size_t obj_size)
 *
 * Purpose: initialize a mempool object and allocate memory for it
 * Args: mempool - pointer to a MemPool struct
 *       num_objects - number of items in this pool
 *       obj_size    - size of the items
 *
 * Returns: 0 on success, 1 on failure
 */

int mempool_init(MemPool *mempool, unsigned num_objects, size_t obj_size)
{
    return mempool_init_optional_prealloc(mempool, num_objects, obj_size, 0);
}

/* Function: int mempool_clean(MemPool *mempool)
 *
 * Purpose: return all memory to free list
 * Args: mempool - pointer to a MemPool struct
 *
 * Returns: 0 on success, -1 on failure
 */
int mempool_clean(MemPool *mempool)
{
    if (mempool == NULL)
        return -1;

    while (mempool->used_list_head)
        mempool_free(mempool, mempool->used_list_head);

    return 0;
}

/* Function: int mempool_destroy(MemPool *mempool)
 *
 * Purpose: destroy a set of mempool objects
 * Args: mempool - pointer to a MemPool struct
 *
 * Returns: 0 on success, 1 on failure
 */
int mempool_destroy(MemPool *mempool)
{
    if(mempool == NULL)
        return 1;

    mempool_free_pools(mempool);

    /* TBD - callback to free up every stray pointer */
    memset(mempool, 0, sizeof(MemPool));

    return 0;
}


static inline MemBucket *_mempool_alloc(MemPool *mempool, bool force_alloc)
{
    MemBucket *b;

    if(mempool == NULL)
        return NULL;

    /* get one item off the free_list,
       put one item on the usedlist
     */

    while ((b = mempool->free_list))
    {
        if (b->obj_size == mempool->obj_size)
            break;
        mempool_free_bucket(mempool);
    }

    if (!b)
    {
        if (force_alloc || ((mempool->used_memory + mempool->obj_size) <= mempool->max_memory))
        {
            if ((b = malloc(sizeof(*b) + mempool->obj_size)) == NULL)
            {
                ErrorMessage("%s(%d) mempool_init(): membucket is null\n",
                             __FILE__, __LINE__);
                return NULL;
            }
            b->data = b + 1;
            b->obj_size = mempool->obj_size;
            b->scbPtr = NULL;
#ifdef REG_TEST
            if (REG_TEST_FLAG_SESSION_FORCE_RELOAD & getRegTestFlags())
            {
                if ((mempool->used_memory + mempool->obj_size) > mempool->max_memory)
                    printf("%s: force alloc hit\n", __FUNCTION__);
            }
#endif
        }
        else
        {
#ifdef TEST_MEMPOOL
            printf("No free mempool objects\n");
#endif
            return NULL;
        }
    }
    else
    {
        mempool->free_list = b->next;
        mempool->free_memory -= b->obj_size;
    }

    b->next = NULL;
    b->prev = mempool->used_list_tail;
    if (mempool->used_list_tail)
        mempool->used_list_tail->next = b;
    mempool->used_list_tail = b;
    if (mempool->used_list_head == NULL)
        mempool->used_list_head = b;
    mempool->used_memory += b->obj_size; 
    /* TBD -- make configurable */
    memset(b->data, 0, b->obj_size);

    return b;
}

/* Function: MemBucket *mempool_alloc(MemPool *mempool);
 *
 * Purpose: allocate a new object from the mempool
 * Args: mempool - pointer to a MemPool struct
 *
 * Returns: a pointer to the mempool object on success, NULL on failure
 */
MemBucket *mempool_alloc(MemPool *mempool)
{
    return _mempool_alloc(mempool, FALSE);
}

/* Function: MemBucket *mempool_force_alloc(MemPool *mempool);
 *
 * Purpose: allocate a new object from the mempool even it cross its max_memory
 *          This has to be used when the mempool memory gets free at end of this
 *          path.
 *          Ex: during Reload adjust, we know that mempool gets free at end of Pkt processing
 *              So, to ensure session allocation success mempool_force_alloc will be called.
 * Args: mempool - pointer to a MemPool struct
 *
 * Returns: a pointer to the mempool object on success, NULL on failure
 */
MemBucket *mempool_force_alloc(MemPool *mempool)
{
    return _mempool_alloc(mempool, TRUE);
}

void mempool_free(MemPool *mempool, MemBucket *obj)
{
    if ((mempool == NULL) || (obj == NULL))
        return;

    if (obj->prev)
        obj->prev->next = obj->next;
    else
        mempool->used_list_head = obj->next;

    if (obj->next)
        obj->next->prev = obj->prev;
    else
        mempool->used_list_tail = obj->prev;

    mempool->used_memory -= obj->obj_size;

    if (obj->obj_size == mempool->obj_size)
    {
        obj->next = mempool->free_list;
        mempool->free_list = obj;
        mempool->free_memory += obj->obj_size;
    }
    else
        free(obj);
}

int mempool_free_bucket(MemPool *mempool)
{
    MemBucket *obj;

    if (mempool == NULL || mempool->free_list == NULL) 
        return -1;

    obj = mempool->free_list;
    mempool->free_list = obj->next;
    mempool->free_memory -= obj->obj_size;
    free(obj);
    return 0;
}

MemBucket* mempool_get_lru_bucket(MemPool *memory_pool)
{
     MemBucket *lru_bucket = NULL;

     lru_bucket = mempool_oldestUsedBucket(memory_pool);
     return lru_bucket;
}

unsigned mempool_prune_freelist(MemPool *memory_pool, size_t new_max_memory, unsigned maxWork)
{
     for( ; maxWork && (memory_pool->used_memory + memory_pool->free_memory) > new_max_memory; maxWork--)
     {
           if(mempool_free_bucket(memory_pool))
               break;
     }

     return maxWork;
}

#ifdef TEST_MEMPOOL

#define SIZE 36
int main(void)
{
    MemPool test;
    MemBucket *bucks[SIZE];
    MemBucket *bucket = NULL;
    int i;

    //char *stuffs[4] = { "eenie", "meenie", "minie", "moe" };
    char *stuffs2[36] =
        {  "1eenie", "2meenie", "3minie", " 4moe",
           "1xxxxx", "2yyyyyy", "3zzzzz", " 4qqqq",
           "1eenie", "2meenie", "3minie", " 4moe",
           "1eenie", "2meenie", "3minie", " 4moe",
           "1eenie", "2meenie", "3minie", " 4moe",
           "1eenie", "2meenie", "3minie", " 4moe",
           "1eenie", "2meenie", "3minie", " 4moe",
           "1eenie", "2meenie", "3minie", " 4moe",
           "1eenie", "2meenie", "3minie", " 4moe"
        };

    if(mempool_init(&test, 36, 256))
    {
        printf("error in mempool initialization\n");
    }

    printf("free: %" PRIu64 ", used: %" PRIu64 "\n", (uint64_t)test.free_memory, (uint64_t)test.used_memory);

    for(i = 0; i < 36; i++)
    {
        if((bucks[i] = mempool_alloc(&test)) == NULL)
        {
            printf("error in mempool_alloc: i=%d\n", i);
            continue;
        }

        bucket = bucks[i];

        bucket->data = strncpy(bucket->data, stuffs2[i], 256);
        printf("bucket->data: %s\n", (char *) bucket->data);
    }

    printf("free: %" PRIu64 ", used: %" PRIu64 "\n", (uint64_t)test.free_memory, (uint64_t)test.used_memory);

    for(i = 0; i < 2; i++)
    {
        mempool_free(&test, bucks[i]);
        bucks[i] = NULL;
    }

    printf("free: %" PRIu64 ", used: %" PRIu64 "\n", (uint64_t)test.free_memory, (uint64_t)test.used_memory);

    for(i = 0; i < 14; i++)
    {
        if((bucks[i] = mempool_alloc(&test)) == NULL)
        {
            printf("error in mempool_alloc: i=%d\n", i);
            continue;
        }

        bucket = bucks[i];

        bucket->data = strncpy(bucket->data, stuffs2[i], 256);
        printf("bucket->data: %s\n", (char *) bucket->data);
    }

    printf("free: %" PRIu64 ", used: %" PRIu64 "\n", (uint64_t)test.free_memory, (uint64_t)test.used_memory);

    if (mempool_clean(&test))
    {
        printf("error in mempool_clean\n");
    }

    printf("free: %" PRIu64 ", used: %" PRIu64 "\n", (uint64_t)test.free_memory, (uint64_t)test.used_memory);

    for(i = 0; i < 14; i++)
    {
        if((bucks[i] = mempool_alloc(&test)) == NULL)
        {
            printf("error in mempool_alloc: i=%d\n", i);
            continue;
        }

        bucket = bucks[i];

        bucket->data = strncpy(bucket->data, stuffs2[i], 256);
        printf("bucket->data: %s\n", (char *) bucket->data);
    }

    printf("free: %" PRIu64 ", used: %" PRIu64 "\n", (uint64_t)test.free_memory, (uint64_t)test.used_memory);

    if (mempool_destroy(&test))
    {
        printf("error in mempool_destroy\n");
    }

    return 0;
}
#endif /* TEST_MEMPOOL */

