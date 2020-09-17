// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Oracle.  All Rights Reserved.
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 */
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <assert.h>
#include "workqueue.h"

/* Main processing thread */
static void *
workqueue_thread(void *arg)
{
	struct workqueue	*wq = arg;
	struct workqueue_item	*wi;

	/*
	 * Loop pulling work from the passed in work queue.
	 * Check for notification to exit after every chunk of work.
	 */
	while (1) {
		pthread_mutex_lock(&wq->lock);

		/*
		 * Wait for work.
		 */
		while (wq->next_item == NULL && !wq->terminate) {
			assert(wq->item_count == 0);
			pthread_cond_wait(&wq->wakeup, &wq->lock);
		}
		if (wq->next_item == NULL && wq->terminate) {
			pthread_mutex_unlock(&wq->lock);
			break;
		}

		/*
		 *  Dequeue work from the head of the list.
		 */
		assert(wq->item_count > 0);
		wi = wq->next_item;
		wq->next_item = wi->next;
		wq->item_count--;

		pthread_mutex_unlock(&wq->lock);

		(wi->function)(wi->queue, wi->index, wi->arg);
		free(wi);
	}

	return NULL;
}

/* Allocate a work queue and threads.  Returns zero or negative error code. */
int
workqueue_create(
	struct workqueue	*wq,
	void			*wq_ctx,
	unsigned int		nr_workers)
{
	unsigned int		i;
	int			err = 0;

	memset(wq, 0, sizeof(*wq));
	err = -pthread_cond_init(&wq->wakeup, NULL);
	if (err)
		return err;
	err = -pthread_mutex_init(&wq->lock, NULL);
	if (err)
		goto out_cond;

	wq->wq_ctx = wq_ctx;
	wq->thread_count = nr_workers;
	wq->threads = malloc(nr_workers * sizeof(pthread_t));
	if (!wq->threads) {
		err = -errno;
		goto out_mutex;
	}
	wq->terminate = false;
	wq->terminated = false;

	for (i = 0; i < nr_workers; i++) {
		err = -pthread_create(&wq->threads[i], NULL, workqueue_thread,
				wq);
		if (err)
			break;
	}

	/*
	 * If we encounter errors here, we have to signal and then wait for all
	 * the threads that may have been started running before we can destroy
	 * the workqueue.
	 */
	if (err)
		workqueue_destroy(wq);
	return err;
out_mutex:
	pthread_mutex_destroy(&wq->lock);
out_cond:
	pthread_cond_destroy(&wq->wakeup);
	return err;
}

/*
 * Create a work item consisting of a function and some arguments and schedule
 * the work item to be run via the thread pool.  Returns zero or a negative
 * error code.
 */
int
workqueue_add(
	struct workqueue	*wq,
	workqueue_func_t	func,
	uint32_t		index,
	void			*arg)
{
	struct workqueue_item	*wi;
	int			ret;

	assert(!wq->terminated);

	if (wq->thread_count == 0) {
		func(wq, index, arg);
		return 0;
	}

	wi = malloc(sizeof(struct workqueue_item));
	if (!wi)
		return -errno;

	wi->function = func;
	wi->index = index;
	wi->arg = arg;
	wi->queue = wq;
	wi->next = NULL;

	/* Now queue the new work structure to the work queue. */
	pthread_mutex_lock(&wq->lock);
	if (wq->next_item == NULL) {
		assert(wq->item_count == 0);
		ret = -pthread_cond_signal(&wq->wakeup);
		if (ret) {
			pthread_mutex_unlock(&wq->lock);
			free(wi);
			return ret;
		}
		wq->next_item = wi;
	} else {
		wq->last_item->next = wi;
	}
	wq->last_item = wi;
	wq->item_count++;
	pthread_mutex_unlock(&wq->lock);

	return 0;
}

/*
 * Wait for all pending work items to be processed and tear down the
 * workqueue thread pool.  Returns zero or a negative error code.
 */
int
workqueue_terminate(
	struct workqueue	*wq)
{
	unsigned int		i;
	int			ret;

	pthread_mutex_lock(&wq->lock);
	wq->terminate = true;
	pthread_mutex_unlock(&wq->lock);

	ret = -pthread_cond_broadcast(&wq->wakeup);
	if (ret)
		return ret;

	for (i = 0; i < wq->thread_count; i++) {
		ret = -pthread_join(wq->threads[i], NULL);
		if (ret)
			return ret;
	}

	pthread_mutex_lock(&wq->lock);
	wq->terminated = true;
	pthread_mutex_unlock(&wq->lock);

	return 0;
}

/* Tear down the workqueue. */
void
workqueue_destroy(
	struct workqueue	*wq)
{
	assert(wq->terminated);

	free(wq->threads);
	pthread_mutex_destroy(&wq->lock);
	pthread_cond_destroy(&wq->wakeup);
	memset(wq, 0, sizeof(*wq));
}
