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
 **  Author(s):  Hui Cao <hcao@sourcefire.com>
 **
 **  This mempool implementation has very efficient alloc/free operations.
 **  In addition, it provides thread-safe alloc/free for one allocation/free
 **  thread and one release thread.
 **  One more bonus: Double free detection is also added into this library
 **
 **  NOTES
 **  5.25.13 - Initial Source Code. Hcao
 **
 **  This is a thread safe version of memory pool for one writer and one reader thread
 */

#ifndef __FILE_MEMPOOL_H__
#define __FILE_MEMPOOL_H__

#include "sf_types.h"
#include "circular_buffer.h"

#define SAFE_MEM_SUCCESS   0
#define SAFE_MEM_FAIL      -1


typedef struct _SafeMemPool
{
    void **datapool; /* memory buffer */

    uint64_t total;

    CircularBuffer* free_list;
    CircularBuffer* released_list;
    size_t obj_size;

} SafeMemPool;

/* Initialize mempool
 *
 * Args:
 *   SafeMemPool: pointer to a SafeMemPool struct
 *   uint64_t num_objects: number of objects
 *   size_t obj_size: size of object
 *
 * Return:
 *   SAFE_MEM_SUCCESS
 *   SAFE_MEM_FAIL
 */
int safe_mempool_init(SafeMemPool *mempool, uint64_t num_objects,
        size_t obj_size);

/* Free mempool memory objects
 *
 * Args:
 *   SafeMemPool: pointer to a SafeMemPool struct
 *
 * Return:
 *   SAFE_MEM_SUCCESS
 *   SAFE_MEM_FAIL
 */
int safe_mempool_destroy(SafeMemPool *mempool);

/*
 * Allocate a new object from the SafeMemPool
 * Memory block will not be zeroed for performance
 *
 * Args:
 *   SafeMemPool: pointer to a SafeMemPool struct
 *
 * Returns: a pointer to the SafeMemPool object on success, NULL on failure
 */
void *safe_mempool_alloc(SafeMemPool *mempool);


/*
 * Free a new object from the SafeMemPool
 * This must be called by the same thread calling
 * safe_mempool_alloc()
 *
 * Args:
 *   SafeMemPool: pointer to a SafeMemPool struct
 *   void *obj  : memory object
 *
 * Return:
 *   SAFE_MEM_SUCCESS
 *   SAFE_MEM_FAIL
 */
int safe_mempool_free(SafeMemPool *mempool, void *obj);

/*
 * Release a new object from the SafeMemPool
 * This can be called by a different thread calling
 * safe_mempool_alloc()
 *
 * Args:
 *   SafeMemPool: pointer to a SafeMemPool struct
 *   void *obj  : memory object
 *
 * Return:
 *   SAFE_MEM_SUCCESS
 *   SAFE_MEM_FAIL
 */
int safe_mempool_release(SafeMemPool *mempool, void *obj);

/* Returns number of elements allocated in current buffer*/
uint64_t safe_mempool_allocated(SafeMemPool *mempool);

/* Returns number of elements freed in current buffer*/
uint64_t safe_mempool_freed(SafeMemPool *mempool);

/* Returns number of elements released in current buffer*/
uint64_t safe_mempool_released(SafeMemPool *mempool);

#endif

