/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.  
 * Copyright (C) 2004-2011 Red Hat, Inc. All rights reserved.
 *
 * This file is part of the device-mapper userspace tools.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "libdm/misc/dmlib.h"
#include <sys/mman.h>
#include <pthread.h>

static DM_LIST_INIT(_dm_pools);
static pthread_mutex_t _dm_pools_mutex = PTHREAD_MUTEX_INITIALIZER;
void dm_pools_check_leaks(void);

#ifdef DEBUG_ENFORCE_POOL_LOCKING
#ifdef DEBUG_POOL
#error Do not use DEBUG_POOL with DEBUG_ENFORCE_POOL_LOCKING
#endif

/*
 * Use mprotect system call to ensure all locked pages are not writable.
 * Generates segmentation fault with write access to the locked pool.
 *
 * - Implementation is using posix_memalign() to get page aligned
 *   memory blocks (could be implemented also through malloc).
 * - Only pool-fast is properly handled for now.
 * - Checksum is slower compared to mprotect.
 */
static size_t _pagesize = 0;
static size_t _pagesize_mask = 0;
#define ALIGN_ON_PAGE(size) (((size) + (_pagesize_mask)) & ~(_pagesize_mask))
#endif

#ifdef DEBUG_POOL
#include "pool-debug.c"
#else
#include "pool-fast.c"
#endif

char *dm_pool_strdup(struct dm_pool *p, const char *str)
{
	size_t len = strlen(str) + 1;
	char *ret = dm_pool_alloc(p, len);

	if (ret)
		memcpy(ret, str, len);

	return ret;
}

char *dm_pool_strndup(struct dm_pool *p, const char *str, size_t n)
{
	size_t slen = strlen(str);
	size_t len = (slen < n) ? slen : n;
	char *ret = dm_pool_alloc(p, n + 1);

	if (ret) {
		ret[len] = '\0';
		memcpy(ret, str, len);
	}

	return ret;
}

void *dm_pool_zalloc(struct dm_pool *p, size_t s)
{
	void *ptr = dm_pool_alloc(p, s);

	if (ptr)
		memset(ptr, 0, s);

	return ptr;
}

void dm_pools_check_leaks(void)
{
	struct dm_pool *p;

	pthread_mutex_lock(&_dm_pools_mutex);
	if (dm_list_empty(&_dm_pools)) {
		pthread_mutex_unlock(&_dm_pools_mutex);
		return;
	}

	log_error("You have a memory leak (not released memory pool):");
	dm_list_iterate_items(p, &_dm_pools) {
#ifdef DEBUG_POOL
		log_error(" [%p] %s (%u bytes)",
			  p->orig_pool,
			  p->name, p->stats.bytes);
#else
		log_error(" [%p] %s", p, p->name);
#endif
	}
	pthread_mutex_unlock(&_dm_pools_mutex);
	log_error(INTERNAL_ERROR "Unreleased memory pool(s) found.");
}

/**
 * Status of locked pool.
 *
 * \param p
 * Pool to be tested for lock status.
 *
 * \return
 * 1 when the pool is locked, 0 otherwise.
 */
int dm_pool_locked(struct dm_pool *p)
{
	return p->locked;
}

/**
 * Lock memory pool.
 *
 * \param p
 * Pool to be locked.
 *
 * \param crc
 * Bool specifies whether to store the pool crc/hash checksum.
 *
 * \return
 * 1 (success) when the pool was preperly locked, 0 otherwise.
 */
int dm_pool_lock(struct dm_pool *p, int crc)
{
	if (p->locked) {
		log_error(INTERNAL_ERROR "Pool %s is already locked.",
			  p->name);
		return 0;
	}

	if (crc)
		p->crc = _pool_crc(p);  /* Get crc for pool */

	if (!_pool_protect(p, PROT_READ)) {
		_pool_protect(p, PROT_READ | PROT_WRITE);
		return_0;
	}

	p->locked = 1;

	log_debug_mem("Pool %s is locked.", p->name);

	return 1;
}

/**
 * Unlock memory pool.
 *
 * \param p
 * Pool to be unlocked.
 *
 * \param crc
 * Bool enables compare of the pool crc/hash with the stored value
 * at pool lock. The pool is not properly unlocked if there is a mismatch.
 *
 * \return
 * 1 (success) when the pool was properly unlocked, 0 otherwise.
 */
int dm_pool_unlock(struct dm_pool *p, int crc)
{
	if (!p->locked) {
		log_error(INTERNAL_ERROR "Pool %s is already unlocked.",
			  p->name);
		return 0;
	}

	p->locked = 0;

	if (!_pool_protect(p, PROT_READ | PROT_WRITE))
		return_0;

	log_debug_mem("Pool %s is unlocked.", p->name);

	if (crc && (p->crc != _pool_crc(p))) {
		log_error(INTERNAL_ERROR "Pool %s crc mismatch.", p->name);
		return 0;
	}

	return 1;
}
