#include <libxfs.h>
#include <pthread.h>
#include <signal.h>
#include "threads.h"
#include "err_protos.h"
#include "protos.h"
#include "globals.h"

static void *
worker_thread(void *arg)
{
	work_queue_t	*wq;
	work_item_t	*wi;

	wq = (work_queue_t*)arg;

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
			ASSERT(wq->item_count == 0);
			pthread_cond_wait(&wq->wakeup, &wq->lock);
		}
		if (wq->next_item == NULL && wq->terminate) {
			pthread_mutex_unlock(&wq->lock);
			break;
		}

		/*
		 *  Dequeue work from the head of the list.
		 */
		ASSERT(wq->item_count > 0);
		wi = wq->next_item;
		wq->next_item = wi->next;
		wq->item_count--;

		pthread_mutex_unlock(&wq->lock);

		(wi->function)(wi->queue, wi->agno, wi->arg);
		free(wi);
	}

	return NULL;
}

void
thread_init(void)
{
	sigset_t	blocked;

	/*
	 *  block delivery of progress report signal to all threads
         */
	sigemptyset(&blocked);
	sigaddset(&blocked, SIGHUP);
	sigaddset(&blocked, SIGALRM);
	pthread_sigmask(SIG_BLOCK, &blocked, NULL);
}


void
create_work_queue(
	work_queue_t		*wq,
	xfs_mount_t		*mp,
	int			nworkers)
{
	int			err;
	int			i;

	memset(wq, 0, sizeof(work_queue_t));

	pthread_cond_init(&wq->wakeup, NULL);
	pthread_mutex_init(&wq->lock, NULL);

	wq->mp = mp;
	wq->thread_count = nworkers;
	wq->threads = malloc(nworkers * sizeof(pthread_t));
	wq->terminate = 0;

	for (i = 0; i < nworkers; i++) {
		err = pthread_create(&wq->threads[i], NULL, worker_thread, wq);
		if (err != 0) {
			do_error(_("cannot create worker threads, error = [%d] %s\n"),
				err, strerror(err));
		}
	}

}

void
queue_work(
	work_queue_t	*wq,
	work_func_t	func,
	xfs_agnumber_t	agno,
	void		*arg)
{
	work_item_t	*wi;

	wi = (work_item_t *)malloc(sizeof(work_item_t));
	if (wi == NULL)
		do_error(_("cannot allocate worker item, error = [%d] %s\n"),
			errno, strerror(errno));

	wi->function = func;
	wi->agno = agno;
	wi->arg = arg;
	wi->queue = wq;
	wi->next = NULL;

	/*
	 *  Now queue the new work structure to the work queue.
	 */
	pthread_mutex_lock(&wq->lock);
	if (wq->next_item == NULL) {
		wq->next_item = wi;
		ASSERT(wq->item_count == 0);
		pthread_cond_signal(&wq->wakeup);
	} else {
		wq->last_item->next = wi;
	}
	wq->last_item = wi;
	wq->item_count++;
	pthread_mutex_unlock(&wq->lock);
}

void
destroy_work_queue(
	work_queue_t	*wq)
{
	int		i;

	pthread_mutex_lock(&wq->lock);
	wq->terminate = 1;
	pthread_mutex_unlock(&wq->lock);

	pthread_cond_broadcast(&wq->wakeup);

	for (i = 0; i < wq->thread_count; i++)
		pthread_join(wq->threads[i], NULL);

	free(wq->threads);
	pthread_mutex_destroy(&wq->lock);
	pthread_cond_destroy(&wq->wakeup);
}
