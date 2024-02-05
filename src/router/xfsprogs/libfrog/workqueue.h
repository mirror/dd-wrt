// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Oracle.  All Rights Reserved.
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 */
#ifndef	__LIBFROG_WORKQUEUE_H__
#define	__LIBFROG_WORKQUEUE_H__

#include <pthread.h>

struct workqueue;

typedef void workqueue_func_t(struct workqueue *wq, uint32_t index, void *arg);

struct workqueue_item {
	struct workqueue	*queue;
	struct workqueue_item	*next;
	workqueue_func_t	*function;
	void			*arg;
	uint32_t		index;
};

struct workqueue {
	void			*wq_ctx;
	pthread_t		*threads;
	struct workqueue_item	*next_item;
	struct workqueue_item	*last_item;
	pthread_mutex_t		lock;
	pthread_cond_t		wakeup;
	unsigned int		item_count;
	unsigned int		thread_count;
	unsigned int		active_threads;
	bool			terminate;
	bool			terminated;
	int			max_queued;
	pthread_cond_t		queue_full;
};

int workqueue_create(struct workqueue *wq, void *wq_ctx,
		unsigned int nr_workers);
int workqueue_create_bound(struct workqueue *wq, void *wq_ctx,
		unsigned int nr_workers, unsigned int max_queue);
int workqueue_add(struct workqueue *wq, workqueue_func_t fn,
		uint32_t index, void *arg);
int workqueue_terminate(struct workqueue *wq);
void workqueue_destroy(struct workqueue *wq);

#endif	/* __LIBFROG_WORKQUEUE_H__ */
