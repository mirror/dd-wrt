/*
 **
 **
 **  Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 **  Copyright (C) 2013-2013 Sourcefire, Inc.
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
 **  This mempool implementation has very efficient alloc/free operations.
 **  In addition, it provides thread-safe alloc/free for one allocation/free
 **  thread and one release thread. One more bonus: Double free detection is
 **  also added into this library
 **
 **  Author(s):  Hui Cao <hcao@sourcefire.com>
 **
 **  NOTES
 **  5.25.13 - Initial Source Code. Hui Cao
 **
 **  A
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "file_mempool.h"
#include "memory_stats.h"
#include "preprocids.h"
#include "util.h"

/*This magic is used for double free detection*/

#define FREE_MAGIC    0x2525252525252525
typedef uint64_t      MagicType;
#ifdef DEBUG_MSGS

static inline void safe_mempool_verify(SafeMemPool *mempool)
{
    uint64_t free_size;
    uint64_t release_size;

    free_size = cbuffer_used(mempool->free_list);
    release_size = cbuffer_used(mempool->released_list);

    if (free_size > cbuffer_size(mempool->free_list))
    {
        ErrorMessage("%s(%d) safe_mempool: failed to verify free list!\n",
                __FILE__, __LINE__ );
    }

    if (release_size > cbuffer_size(mempool->released_list))
    {
        ErrorMessage("%s(%d) safe_mempool: failed to verify release list!\n",
                __FILE__, __LINE__ );
    }

    /* The free mempool and size of release mempool should be smaller than
     * or equal to the size of mempool
     */
    if (free_size + release_size > mempool->total)
    {
        ErrorMessage("%s(%d) safe_mempool: failed to verify mempool size!\n",
                __FILE__, __LINE__ );
    }
}

#endif

static inline void safe_mempool_free_pools(SafeMemPool *mempool)
{
    if (mempool == NULL)
        return;

    if (mempool->datapool != NULL)
    {
        SnortPreprocFree(mempool->datapool, mempool->obj_size * mempool->total, PP_FILE, 
                PP_MEM_CATEGORY_MEMPOOL);
        mempool->datapool = NULL;
    }

    cbuffer_free(mempool->free_list);
    cbuffer_free(mempool->released_list);

}

/* Function: int safe_mempool_init(SafeMemPool *SafeMemPool,
 *                            PoolCount num_objects, size_t obj_size)
 *
 * Purpose: initialize a SafeMemPool object and allocate memory for it
 * Args:
 *   SafeMemPool - pointer to a SafeMemPool struct
 *   num_objects - number of items in this pool
 *   obj_size    - size of the items
 *
 * Returns:
 *   SAFE_MEM_SUCCESS
 *   SAFE_MEM_FAIL
 */

int safe_mempool_init(SafeMemPool *mempool, uint64_t num_objects, size_t obj_size)
{
    unsigned int i;

    if ((mempool == NULL) || (num_objects < 1) || (obj_size < 1))
        return SAFE_MEM_FAIL;

    mempool->obj_size = obj_size;

    /* this is the basis pool that represents all the *data pointers
     * in the list
     */
    mempool->datapool = SnortPreprocAlloc(num_objects, obj_size, PP_FILE, PP_MEM_CATEGORY_MEMPOOL);
    if(mempool->datapool == NULL)
    {
        ErrorMessage("%s(%d) safe_mempool_init(): Failed to init datapool\n",
                __FILE__, __LINE__);
        safe_mempool_free_pools(mempool);
        return SAFE_MEM_FAIL;
    }

    /* sets up the memory list */
    mempool->free_list = cbuffer_init(num_objects);
    if (!mempool->free_list)
    {
        ErrorMessage("%s(%d) safe_mempool_init(): Failed to init free list\n",
                __FILE__, __LINE__);
        safe_mempool_free_pools(mempool);
        return SAFE_MEM_FAIL;
    }

    mempool->released_list = cbuffer_init(num_objects);
    if (!mempool->released_list)
    {
        ErrorMessage("%s(%d) safe_mempool_init(): "
                "Failed to init release list\n", __FILE__, __LINE__);
        safe_mempool_free_pools(mempool);
        return SAFE_MEM_FAIL;

    }

    for(i=0; i<num_objects; i++)
    {
        void *data = ((char *) mempool->datapool) + (i * mempool->obj_size);

        if (cbuffer_write(mempool->free_list,  data))
        {
            ErrorMessage("%s(%d) safe_mempool_init(): "
                    "Failed to add to free list\n",
                    __FILE__, __LINE__);
            safe_mempool_free_pools(mempool);
            return SAFE_MEM_FAIL;
        }
        *(MagicType *)data = (uint64_t)FREE_MAGIC;
         mempool->total++;
    }

    return SAFE_MEM_SUCCESS;
}



/*
 * Destroy a set of SafeMemPool objects
 *
 * Args:
 *   SafeMemPool: pointer to a SafeMemPool struct
 *
 * Return:
 *   SAFE_MEM_SUCCESS
 *   SAFE_MEM_FAIL
 */
int safe_mempool_destroy(SafeMemPool *mempool)
{
    if(mempool == NULL)
        return SAFE_MEM_FAIL;

    safe_mempool_free_pools(mempool);

    return SAFE_MEM_SUCCESS;
}


/*
 * Allocate a new object from the SafeMemPool
 *
 * Args:
 *   SafeMemPool: pointer to a SafeMemPool struct
 *
 * Returns: a pointer to the SafeMemPool object on success, NULL on failure
 */

void *safe_mempool_alloc(SafeMemPool *mempool)
{

    void  *b = NULL;

    if(mempool == NULL)
    {
        return NULL;
    }

    if(cbuffer_read(mempool->free_list, &b))
    {
        if(cbuffer_read(mempool->released_list, &b))
        {
            return NULL;
        }
    }

    if (*(MagicType *)b != ((uint64_t)FREE_MAGIC))
    {
        ErrorMessage("%s(%d) safe_mempool_alloc(): Possible memory corruption! \n",
                __FILE__, __LINE__);
    }

    DEBUG_WRAP(safe_mempool_verify(mempool););

    return b;
}

/*
 * Free a new object from the buffer
 * We use circular buffer to synchronize one reader and one writer
 *
 * Args:
 *   SafeMemPool: pointer to a circular buffer struct
 *   void *obj  : memory object
 *
 * Return:
 *   SAFE_MEM_SUCCESS
 *   SAFE_MEM_FAIL
 */
static inline int _safe__mempool_remove(CircularBuffer *cb, void *obj)
{
    if (obj == NULL)
        return SAFE_MEM_FAIL;

    if (*(MagicType *)obj == ((uint64_t)FREE_MAGIC))
    {
        DEBUG_WRAP(ErrorMessage("%s(%d) safe_mempool_remove(): Double free! \n",
                __FILE__, __LINE__););
        return SAFE_MEM_FAIL;
    }

    if (cbuffer_write(cb, obj))
    {
        return SAFE_MEM_FAIL;
    }

    *(MagicType *)obj = (uint64_t)FREE_MAGIC;

    return SAFE_MEM_SUCCESS;
}

/*
 * Free a new object from the SafeMemPool
 *
 * Args:
 *   SafeMemPool: pointer to a SafeMemPool struct
 *   void *obj  : memory object
 *
 * Return:
 *   SAFE_MEM_SUCCESS
 *   SAFE_MEM_FAIL
 */

int safe_mempool_free(SafeMemPool *mempool, void *obj)
{
    int ret;

    assert(mempool);

    ret = _safe__mempool_remove(mempool->free_list, obj);

    DEBUG_WRAP(safe_mempool_verify(mempool););

    return ret;
}

/*
 * Release a new object from the SafeMemPool
 * This can be called by a different thread calling
 * safe_mempool_alloc()
 *  *
 * Args:
 *   SafeMemPool: pointer to a SafeMemPool struct
 *   void *obj  : memory object
 *
 * Return:
 *   SAFE_MEM_SUCCESS
 *   SAFE_MEM_FAIL
 */

int safe_mempool_release(SafeMemPool *mempool, void *obj)
{
    int ret;

    if (mempool == NULL)
        return SAFE_MEM_FAIL;

    /*A writer that might from different thread*/
    ret = _safe__mempool_remove(mempool->released_list, obj);

    DEBUG_WRAP(safe_mempool_verify(mempool););

    return ret;
}

/* Returns number of elements allocated in current buffer*/
uint64_t safe_mempool_allocated(SafeMemPool *mempool)
{
    uint64_t total_freed =
            safe_mempool_released(mempool) + safe_mempool_freed(mempool);
    return (mempool->total - total_freed);
}

/* Returns number of elements freed in current buffer*/
uint64_t safe_mempool_freed(SafeMemPool *mempool)
{
    return (cbuffer_used(mempool->free_list));
}

/* Returns number of elements released in current buffer*/
uint64_t safe_mempool_released(SafeMemPool *mempool)
{
    return (cbuffer_used(mempool->released_list));
}




