#ifndef _XFS_REPAIR_PREFETCH_H
#define	_XFS_REPAIR_PREFETCH_H

#include <semaphore.h>
#include "incore.h"


extern int 	do_prefetch;

#define PF_THREAD_COUNT	4

typedef struct prefetch_args {
	pthread_mutex_t		lock;
	pthread_t		queuing_thread;
	pthread_t		io_threads[PF_THREAD_COUNT];
	struct btree_root	*io_queue;
	pthread_cond_t		start_reading;
	pthread_cond_t		start_processing;
	int			agno;
	int			dirs_only;
	volatile int		can_start_reading;
	volatile int		can_start_processing;
	volatile int		prefetch_done;
	volatile int		queuing_done;
	volatile int		inode_bufs_queued;
	volatile xfs_fsblock_t	last_bno_read;
	sem_t			ra_count;
	struct prefetch_args	*next_args;
} prefetch_args_t;



void
init_prefetch(
	xfs_mount_t		*pmp);

prefetch_args_t *
start_inode_prefetch(
	xfs_agnumber_t		agno,
	int			dirs_only,
	prefetch_args_t		*prev_args);

void
wait_for_inode_prefetch(
	prefetch_args_t		*args);

void
cleanup_inode_prefetch(
	prefetch_args_t		*args);


#ifdef XR_PF_TRACE
void	pftrace_init(void);
void	pftrace_done(void);

#define pftrace(msg...)	_pftrace(__FUNCTION__, ## msg)
void	_pftrace(const char *, const char *, ...);
#else
static inline void pftrace_init(void) { };
static inline void pftrace_done(void) { };
static inline void pftrace(const char *msg, ...) { };
#endif

#endif /* _XFS_REPAIR_PREFETCH_H */
