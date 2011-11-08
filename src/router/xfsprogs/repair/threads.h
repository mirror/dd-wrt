#ifndef	_XFS_REPAIR_THREADS_H_
#define	_XFS_REPAIR_THREADS_H_

void	thread_init(void);

struct  work_queue;

typedef void work_func_t(struct work_queue *, xfs_agnumber_t, void *);

typedef struct work_item {
	struct work_item	*next;
	work_func_t		*function;
	struct work_queue	*queue;
	xfs_agnumber_t		agno;
	void			*arg;
} work_item_t;

typedef struct  work_queue {
	work_item_t		*next_item;
	work_item_t		*last_item;
	int			item_count;
	int			thread_count;
	pthread_t		*threads;
	xfs_mount_t		*mp;
	pthread_mutex_t		lock;
	pthread_cond_t		wakeup;
	int			terminate;
} work_queue_t;

void
create_work_queue(
	work_queue_t		*wq,
	xfs_mount_t		*mp,
	int			nworkers);

void
queue_work(
	work_queue_t		*wq,
	work_func_t 		func,
	xfs_agnumber_t 		agno,
	void			*arg);

void
destroy_work_queue(
	work_queue_t		*wq);

#endif	/* _XFS_REPAIR_THREADS_H_ */
