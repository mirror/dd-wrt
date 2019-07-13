#include <stdlib.h>
#include <unistd.h>

#include "config.h"
#include "workqueue.h"
#include "xlog.h"

#if defined(HAVE_SCHED_H) && defined(HAVE_LIBPTHREAD) && defined(HAVE_UNSHARE)
#include <sched.h>
#include <pthread.h>

struct xwork_struct {
	struct xwork_struct *next;
	void (*fn)(void *);
	void *data;
};

struct xwork_queue {
	struct xwork_struct *head;
	struct xwork_struct **tail;

	unsigned char shutdown : 1;
};

static void xwork_queue_init(struct xwork_queue *queue)
{
	queue->head = NULL;
	queue->tail = &queue->head;
	queue->shutdown = 0;
}

static void xwork_enqueue(struct xwork_queue *queue,
		struct xwork_struct *entry)
{
	entry->next = NULL;
	*queue->tail = entry;
	queue->tail = &entry->next;
}

static struct xwork_struct *xwork_dequeue(struct xwork_queue *queue)
{
	struct xwork_struct *entry = NULL;
	if (queue->head) {
		entry = queue->head;
		queue->head = entry->next;
		if (!queue->head)
			queue->tail = &queue->head;
	}
	return entry;
}

struct xthread_work {
	struct xwork_struct work;

	pthread_cond_t cond;
};

struct xthread_workqueue {
	struct xwork_queue queue;

	pthread_mutex_t mutex;
	pthread_cond_t cond;
};

static void xthread_workqueue_init(struct xthread_workqueue *wq)
{
	xwork_queue_init(&wq->queue);
	pthread_mutex_init(&wq->mutex, NULL);
	pthread_cond_init(&wq->cond, NULL);
}

static void xthread_workqueue_fini(struct xthread_workqueue *wq)
{
	pthread_cond_destroy(&wq->cond);
	pthread_mutex_destroy(&wq->mutex);
}

static int xthread_work_enqueue(struct xthread_workqueue *wq,
		struct xthread_work *work)
{
	xwork_enqueue(&wq->queue, &work->work);
	pthread_cond_signal(&wq->cond);
	return 0;
}

static struct xthread_work *xthread_work_dequeue(struct xthread_workqueue *wq)
{
	return (struct xthread_work *)xwork_dequeue(&wq->queue);
}

static void xthread_workqueue_do_work(struct xthread_workqueue *wq)
{
	struct xthread_work *work;

	pthread_mutex_lock(&wq->mutex);
	/* Signal the caller that we're up and running */
	pthread_cond_signal(&wq->cond);
	for (;;) {
		work = xthread_work_dequeue(wq);
		if (work) {
			work->work.fn(work->work.data);
			pthread_cond_signal(&work->cond);
			continue;
		}
		if (wq->queue.shutdown)
			break;
		pthread_cond_wait(&wq->cond, &wq->mutex);
	}
	pthread_mutex_unlock(&wq->mutex);
}

void xthread_workqueue_shutdown(struct xthread_workqueue *wq)
{
	pthread_mutex_lock(&wq->mutex);
	wq->queue.shutdown = 1;
	pthread_cond_signal(&wq->cond);
	pthread_mutex_unlock(&wq->mutex);
}

static void xthread_workqueue_free(struct xthread_workqueue *wq)
{
	xthread_workqueue_fini(wq);
	free(wq);
}

static void xthread_workqueue_cleanup(void *data)
{
	xthread_workqueue_free(data);
}

static void *xthread_workqueue_worker(void *data)
{
	pthread_cleanup_push(xthread_workqueue_cleanup, data);
	xthread_workqueue_do_work(data);
	pthread_cleanup_pop(1);
	return NULL;
}

struct xthread_workqueue *xthread_workqueue_alloc(void)
{
	struct xthread_workqueue *ret;
	pthread_t thread;

	ret = malloc(sizeof(*ret));
	if (ret) {
		xthread_workqueue_init(ret);

		pthread_mutex_lock(&ret->mutex);
		if (pthread_create(&thread, NULL,
					xthread_workqueue_worker,
					ret) == 0) {
			/* Wait for thread to start */
			pthread_cond_wait(&ret->cond, &ret->mutex);
			pthread_mutex_unlock(&ret->mutex);
			return ret;
		}
		pthread_mutex_unlock(&ret->mutex);
		xthread_workqueue_free(ret);
		ret = NULL;
	}
	return NULL;
}

void xthread_work_run_sync(struct xthread_workqueue *wq,
		void (*fn)(void *), void *data)
{
	struct xthread_work work = {
		{
			NULL,
			fn,
			data
		},
		PTHREAD_COND_INITIALIZER,
	};
	pthread_mutex_lock(&wq->mutex);
	xthread_work_enqueue(wq, &work);
	pthread_cond_wait(&work.cond, &wq->mutex);
	pthread_mutex_unlock(&wq->mutex);
	pthread_cond_destroy(&work.cond);
}

static void xthread_workqueue_do_chroot(void *data)
{
	const char *path = data;

	if (unshare(CLONE_FS) != 0) {
		xlog_err("unshare() failed: %m");
		return;
	}
	if (chroot(path) != 0)
		xlog_err("chroot(%s) failed: %m", path);
}

void xthread_workqueue_chroot(struct xthread_workqueue *wq,
		const char *path)
{
	xthread_work_run_sync(wq, xthread_workqueue_do_chroot, (void *)path);
}

#else

struct xthread_workqueue {
};

static struct xthread_workqueue ret;

struct xthread_workqueue *xthread_workqueue_alloc(void)
{
	return &ret;
}

void xthread_workqueue_shutdown(struct xthread_workqueue *wq)
{
}

void xthread_work_run_sync(struct xthread_workqueue *wq,
		void (*fn)(void *), void *data)
{
	fn(data);
}

void xthread_workqueue_chroot(struct xthread_workqueue *wq,
		const char *path)
{
	xlog_err("Unable to run as chroot");
}

#endif /* defined(HAVE_SCHED_H) && defined(HAVE_LIBPTHREAD) && defined(HAVE_UNSHARE) */
