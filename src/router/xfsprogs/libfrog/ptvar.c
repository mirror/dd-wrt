// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Oracle.  All Rights Reserved.
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 */
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include "platform_defs.h"
#include "ptvar.h"

/*
 * Per-thread Variables
 *
 * This data structure manages a lockless per-thread variable.  We
 * implement this by allocating an array of memory regions, and as each
 * thread tries to acquire its own region, we hand out the array
 * elements to each thread.  This way, each thread gets its own
 * cacheline and (after the first access) doesn't have to contend for a
 * lock for each access.
 */
struct ptvar {
	pthread_key_t	key;
	pthread_mutex_t	lock;
	size_t		nr_used;
	size_t		nr_counters;
	size_t		data_size;
	unsigned char	data[0];
};
#define PTVAR_SIZE(nr, sz) (sizeof(struct ptvar) + ((nr) * (size)))

/* Allocate a new per-thread counter. */
int
ptvar_alloc(
	size_t		nr,
	size_t		size,
	struct ptvar	**pptv)
{
	struct ptvar	*ptv;
	int		ret;

#ifdef _SC_LEVEL1_DCACHE_LINESIZE
	long		l1_dcache;

	/* Try to prevent cache pingpong by aligning to cacheline size. */
	l1_dcache = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
	if (l1_dcache > 0)
		size = roundup(size, l1_dcache);
#endif

	ptv = malloc(PTVAR_SIZE(nr, size));
	if (!ptv)
		return -errno;
	ptv->data_size = size;
	ptv->nr_counters = nr;
	ptv->nr_used = 0;
	memset(ptv->data, 0, nr * size);
	ret = -pthread_mutex_init(&ptv->lock, NULL);
	if (ret)
		goto out;
	ret = -pthread_key_create(&ptv->key, NULL);
	if (ret)
		goto out_mutex;

	*pptv = ptv;
	return 0;
out_mutex:
	pthread_mutex_destroy(&ptv->lock);
out:
	free(ptv);
	return ret;
}

/* Free per-thread counter. */
void
ptvar_free(
	struct ptvar	*ptv)
{
	pthread_key_delete(ptv->key);
	pthread_mutex_destroy(&ptv->lock);
	free(ptv);
}

/* Get a reference to this thread's variable. */
void *
ptvar_get(
	struct ptvar	*ptv,
	int		*retp)
{
	void		*p;
	int		ret;

	p = pthread_getspecific(ptv->key);
	if (!p) {
		pthread_mutex_lock(&ptv->lock);
		assert(ptv->nr_used < ptv->nr_counters);
		p = &ptv->data[(ptv->nr_used++) * ptv->data_size];
		ret = -pthread_setspecific(ptv->key, p);
		if (ret)
			goto out_unlock;
		pthread_mutex_unlock(&ptv->lock);
	}
	*retp = 0;
	return p;

out_unlock:
	ptv->nr_used--;
	pthread_mutex_unlock(&ptv->lock);
	*retp = ret;
	return NULL;
}

/* Iterate all of the per-thread variables. */
int
ptvar_foreach(
	struct ptvar	*ptv,
	ptvar_iter_fn	fn,
	void		*foreach_arg)
{
	size_t		i;
	int		ret = 0;

	pthread_mutex_lock(&ptv->lock);
	for (i = 0; i < ptv->nr_used; i++) {
		ret = fn(ptv, &ptv->data[i * ptv->data_size], foreach_arg);
		if (ret)
			break;
	}
	pthread_mutex_unlock(&ptv->lock);

	return ret;
}
