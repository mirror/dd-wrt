/*
     This file is part of libmicrohttpd
     Copyright (C) 2007--2019 Daniel Pittman, Christian Grothoff and
     Karlson2k (Evgeny Grin)

     This library is free software; you can redistribute it and/or
     modify it under the terms of the GNU Lesser General Public
     License as published by the Free Software Foundation; either
     version 2.1 of the License, or (at your option) any later version.

     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Lesser General Public License for more details.

     You should have received a copy of the GNU Lesser General Public
     License along with this library; if not, write to the Free Software
     Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

/**
 * @file memorypool.h
 * @brief memory pool; mostly used for efficient (de)allocation
 *        for each connection and bounding memory use for each
 *        request
 * @author Christian Grothoff
 * @author Karlson2k (Evgeny Grin)
 */

#ifndef MEMORYPOOL_H
#define MEMORYPOOL_H

#include "mhd_options.h"
#ifdef HAVE_STDDEF_H
#include <stddef.h>
#endif /* HAVE_STDDEF_H */
#ifdef HAVE_STDBOOL_H
#include <stdbool.h>
#endif

/**
 * Opaque handle for a memory pool.
 * Pools are not reentrant and must not be used
 * by multiple threads.
 */
struct MemoryPool;

/**
 * Initialize values for memory pools
 */
void
MHD_init_mem_pools_ (void);


/**
 * Create a memory pool.
 *
 * @param max maximum size of the pool
 * @return NULL on error
 */
struct MemoryPool *
MHD_pool_create (size_t max);


/**
 * Destroy a memory pool.
 *
 * @param pool memory pool to destroy
 */
void
MHD_pool_destroy (struct MemoryPool *pool);


/**
 * Allocate size bytes from the pool.
 *
 * @param pool memory pool to use for the operation
 * @param size number of bytes to allocate
 * @param from_end allocate from end of pool (set to 'true');
 *        use this for small, persistent allocations that
 *        will never be reallocated
 * @return NULL if the pool cannot support size more
 *         bytes
 */
void *
MHD_pool_allocate (struct MemoryPool *pool,
                   size_t size,
                   bool from_end);


/**
 * Try to allocate @a size bytes memory area from the @a pool.
 *
 * If allocation fails, @a required_bytes is updated with size required to be
 * freed in the @a pool from relocatable area to allocate requested number
 * of bytes.
 * Allocated memory area is always not rellocatable ("from end").
 *
 * @param pool memory pool to use for the operation
 * @param size the size of memory in bytes to allocate
 * @param[out] required_bytes the pointer to variable to be updated with
 *                            the size of the required additional free
 *                            memory area, not updated if function succeed.
 *                            Cannot be NULL.
 * @return the pointer to allocated memory area if succeed,
 *         NULL if the pool doesn't have enough space, required_bytes is updated
 *         with amount of space needed to be freed in relocatable area or
 *         set to SIZE_MAX if requested size is too large for the pool.
 */
void *
MHD_pool_try_alloc (struct MemoryPool *pool,
                    size_t size,
                    size_t *required_bytes);


/**
 * Reallocate a block of memory obtained from the pool.
 * This is particularly efficient when growing or
 * shrinking the block that was last (re)allocated.
 * If the given block is not the most recently
 * (re)allocated block, the memory of the previous
 * allocation may be not released until the pool is
 * destroyed or reset.
 *
 * @param pool memory pool to use for the operation
 * @param old the existing block
 * @param old_size the size of the existing block
 * @param new_size the new size of the block
 * @return new address of the block, or
 *         NULL if the pool cannot support @a new_size
 *         bytes (old continues to be valid for @a old_size)
 */
void *
MHD_pool_reallocate (struct MemoryPool *pool,
                     void *old,
                     size_t old_size,
                     size_t new_size);


/**
 * Check how much memory is left in the @a pool
 *
 * @param pool pool to check
 * @return number of bytes still available in @a pool
 */
size_t
MHD_pool_get_free (struct MemoryPool *pool);


/**
 * Clear all entries from the memory pool except
 * for @a keep of the given @a copy_bytes.  The pointer
 * returned should be a buffer of @a new_size where
 * the first @a copy_bytes are from @a keep.
 *
 * @param pool memory pool to use for the operation
 * @param keep pointer to the entry to keep (maybe NULL)
 * @param copy_bytes how many bytes need to be kept at this address
 * @param new_size how many bytes should the allocation we return have?
 *                 (should be larger or equal to @a copy_bytes)
 * @return addr new address of @a keep (if it had to change)
 */
void *
MHD_pool_reset (struct MemoryPool *pool,
                void *keep,
                size_t copy_bytes,
                size_t new_size);

#endif
