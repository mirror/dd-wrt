/*
 * Copyright (C) 2019 Trond Myklebust <trond.myklebust@hammerspace.com>
 */
#ifndef WORKQUEUE_H
#define WORKQUEUE_H

struct xthread_workqueue;

struct xthread_workqueue *xthread_workqueue_alloc(void);
void xthread_workqueue_shutdown(struct xthread_workqueue *wq);

void xthread_work_run_sync(struct xthread_workqueue *wq,
		void (*fn)(void *), void *data);

void xthread_workqueue_chroot(struct xthread_workqueue *wq,
		const char *path);

#endif
