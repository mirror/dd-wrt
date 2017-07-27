/*
 * Copyright (c) 1998 The Santa Cruz Operation, Inc.. All Rights Reserved. 
 *                                                                         
 *        THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF THE               
 *                   SANTA CRUZ OPERATION INC.                             
 *                                                                         
 *   The copyright notice above does not evidence any actual or intended   
 *   publication of such source code.                                      
 */

#ifndef _FS_FIFOFS_FIFONODE_H	/* wrapper symbol for kernel use */
#define _FS_FIFOFS_FIFONODE_H	/* subject to change without notice */

#ident	"@(#)kern:fs/fifofs/fifonode.h	1.14"
#ident	"$Header: $"

#if defined(__cplusplus)
extern "C" {
#endif

#ifdef _KERNEL_HEADERS

#include <util/types.h>		/* REQUIRED */
#include <util/ksynch.h>	/* REQUIRED */
#include <fs/vnode.h>		/* REQUIRED */

#elif defined(_KERNEL) || defined(_KMEMUSER)

#include <sys/types.h>		/* REQUIRED */
#include <sys/ksynch.h>	/* REQUIRED */
#include <sys/vnode.h>		/* REQUIRED */

#endif /* _KERNEL_HEADERS */

#if defined(_KERNEL) || defined(_KMEMUSER)

struct nodelock {
	lock_t	n_lock;
	int	n_count;
};

/*
 * Each FIFOFS object is identified by a struct fifonode/vnode pair.
 */
struct fifonode {
	struct vnode	fn_vnode;	/* represents the fifo/pipe */
	struct vnode	*fn_mate;	/* the other end of a pipe */
	struct vnode	*fn_realvp;	/* node being shadowed by fifo */
	sleep_t		fn_iolock;	/* fifonode iolock */
	sv_t		fn_rwait;	/* wait for first reader */
	sv_t		fn_wwait;	/* wait for first writer */
	sv_t		fn_fdwait;	/* to synchronize fd passing */
	sv_t		fn_openwait;	/* to serialize fifo_open */
	ino_t		fn_ino;		/* node id for pipes */
	short		fn_wcnt;	/* number of writers */
	short		fn_rcnt;	/* number of readers */
	short		fn_open;	/* open count of node*/
	ushort		fn_flag;	/* flags as defined below */
	struct vnode	*fn_unique;	/* new vnode created by CONNLD */
	time_t		fn_atime;	/* creation times for pipe */
	time_t		fn_mtime;
	time_t		fn_ctime;
	struct fifonode	*fn_nextp;	/* next link in the linked list */
	struct fifonode	*fn_backp;	/* back link in linked list */
	struct nodelock *fn_nodelp;	/* lock shared by both ends of a pipe */
};

#endif /* _KERNEL || _KMEMUSER */

#ifdef _KERNEL

/*
 * Valid flags for fifonodes.
 */
#define ISPIPE		01	/* fifonode is that of a pipe */
#define FIFOWOPEN	02	/* open in progress */
#define FIFOWCLOSE      04	/* close in progress */
#define FIFOSEND       010	/* file descriptor at stream head of pipe */
#define FIFOPASS       020	/* CONNLD passed a new vnode in fn_unique */
#define	FIFOMACPRIV    040	/* bypass MAC checks for privilege process */
#define CONNLDPUSHED  0100	/* CONNLD is pushed */
#define FIFOMODTIME   0200	/* access/modification/change time modified */

#define FIFOBSIZE       1024    /* FIFOFS block size */

/* #ifdef MERGE */
extern int fifo_rdchk(vnode_t *);
/* #endif MERGE */

/*
 * Macros to convert a vnode to a fifonode, and vice versa.
 */
#define VTOF(vp) ((struct fifonode *)((vp)->v_data))
#define FTOV(fp) (&(fp)->fn_vnode)

/*
 * Functions used in multiple places.
 */
extern int fifo_rdchk(vnode_t *);
extern int fifo_mkpipe(vnode_t **, vnode_t **, cred_t *);
extern void fifo_rmpipe(vnode_t *, vnode_t *, cred_t *);

/*
 * Macros for manipulating locks and synchronization variables
 */

/*
 * Macros called by fifovp() and fifo_inactive()
 * to avoid a race on the vnode.
 */
#define FIFO_LOCK(fp, lockp) \
	SLEEP_LOCK_RELLOCK(&(fp)->fn_iolock, PRIPIPE, lockp)
#define FIFO_TRYLOCK(fp) SLEEP_TRYLOCK(&(fp)->fn_iolock)
#define FIFO_LOCKBLKD(fp) SLEEP_LOCKBLKD(&(fp)->fn_iolock)
#define FIFO_UNLOCK(fp) SLEEP_UNLOCK(&(fp)->fn_iolock)

/*
 * Macro to manipulate fn_nodelp->n_lock.
 */
#define PIPE_LOCK(fp) LOCK(&(fp)->fn_nodelp->n_lock, PLFIFO)
#define PIPE_UNLOCK(fp, pl) UNLOCK(&(fp)->fn_nodelp->n_lock, pl)

/*
 * Macro to manipulate fn_fdwait.
 * While waiting on fn_fdwait, fn_nodelp->n_lock is dropped.
 */
#define FIFO_FDWAIT(fp) \
	SV_WAIT_SIG(&(fp)->fn_fdwait, PRIPIPE, &(fp)->fn_nodelp->n_lock)
#define FIFO_FDWAKEUP(fp) SV_SIGNAL(&(fp)->fn_fdwait, 0)

/*
 * Macro to obtain and release the stream head mutex, sd_mutex
 */
#define	STREAM_LOCK(stp) LOCK((stp)->sd_mutex, PLSTR);
#define	STREAM_UNLOCK(stp, pl) UNLOCK((stp)->sd_mutex, pl);

#endif	/* _KERNEL */

#if defined(__cplusplus)
	}
#endif

#endif	/* _FS_FIFOFS_FIFONODE_H */
