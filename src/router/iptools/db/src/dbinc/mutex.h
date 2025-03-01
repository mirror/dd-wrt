/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

#ifndef _DB_MUTEX_H_
#define	_DB_MUTEX_H_

#ifdef HAVE_MUTEX_SUPPORT
/* The inlined trylock calls need access to the details of mutexes. */
#define	LOAD_ACTUAL_MUTEX_CODE
#include "dbinc/mutex_int.h"

#ifndef HAVE_SHARED_LATCHES
 #error "Shared latches are required in DB 4.8 and above"
#endif
#endif

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * These specify the default spin parameters for test-and-set mutexes. A single
 * processor system spins just once, a multiprocessor system spins 50 times per
 * processor up to a default maximum of 200. This limit reduces excessive
 * busy-waiting on machines with many hyperthreads. We have anecdotal evidence
 * that these are reasonable default values.
 */
#define	MUTEX_SPINS_PER_PROCESSOR	50
#define	MUTEX_SPINS_DEFAULT_MAX		200

/*
 * Mutexes are represented by unsigned, 32-bit integral values.  As the
 * OOB value is 0, mutexes can be initialized by zero-ing out the memory
 * in which they reside.
 */
#define	MUTEX_INVALID	0

/*
 * We track mutex allocations by ID.  They are stored in the mutex only when
 * statistics are enabled.
 */
#define	MTX_APPLICATION		 1
#define	MTX_ATOMIC_EMULATION	 2
#define	MTX_DB_HANDLE		 3
#define	MTX_ENV_DBLIST		 4
#define	MTX_ENV_EXCLDBLIST	 5
#define	MTX_ENV_HANDLE		 6
#define	MTX_ENV_REGION		 7
#define	MTX_LOCK_REGION		 8
#define	MTX_LOGICAL_LOCK	 9
#define	MTX_LOG_FILENAME	10
#define	MTX_LOG_FLUSH		11
#define	MTX_LOG_HANDLE		12
#define	MTX_LOG_REGION		13
#define	MTX_LSN_HISTORY		14
#define	MTX_MPOOLFILE_HANDLE	15
#define	MTX_MPOOL_BH		16
#define	MTX_MPOOL_FH		17
#define	MTX_MPOOL_FILE_BUCKET	18
#define	MTX_MPOOL_HANDLE	19
#define	MTX_MPOOL_HASH_BUCKET	20
#define	MTX_MPOOL_REGION	21
#define	MTX_MUTEX_REGION	22
#define	MTX_MUTEX_TEST		23
#define	MTX_REP_CHKPT		24
#define	MTX_REP_DATABASE	25
#define	MTX_REP_DIAG		26
#define	MTX_REP_EVENT		27
#define	MTX_REP_REGION		28
#define	MTX_REP_START		29
#define	MTX_REP_WAITER		30
#define	MTX_REPMGR		31
#define	MTX_SEQUENCE		32
#define	MTX_TWISTER		33
#define	MTX_TCL_EVENTS		34
#define	MTX_TXN_ACTIVE		35
#define	MTX_TXN_CHKPT		36
#define	MTX_TXN_COMMIT		37
#define	MTX_TXN_MVCC		38
#define	MTX_TXN_REGION		39

#define	MTX_MAX_ENTRY		39

/*
 * These are the primary macros to lock and unlock a mutex or shared latch:
 *	void MUTEX_LOCK(env, mutex)
 *	void MUTEX_READLOCK(env, mutex)
 *	void MUTEX_UNLOCK(env, mutex)
 *
 * These do not wait for the mutex, but return 0 or DB_LOCK_NOTGRANTED
 *	int MUTEX_TRYLOCK(env, mutex)
 *	int MUTEX_TRY_READLOCK(env, mutex)
 *
 * If 'mutex' is MUTEX_INVALID (0), then syncronization is not required in this
 * particular instance, and the macros do nothing or, int the _TRY versions,
 * return 0.  Otherwise they invoke one of the following:
 *
 *	__mutex_lock(env, mutex, timeout, flags)
 *	__mutex_unlock(env, mutex, ip, flags)
 *	__mutex_rdlock(env, mutex, flags)
 *
 * The timeout parameter, if non-zero, limits the duration of the wait for a
 * busy mutex to at most the specified number of microseconds.
 *
 * The ip parameter is NULL or the DB_THREAD_INFO pointer of the thread that
 * acquired the mutex.  Failchk uses it to clean up after a dead process.
 *
 * The flags values are:
 *   MUTEX_WAIT	- Tell a mutex acquisition call to wait until it is available
 *		  or, if the timeout parameter is non-zero, until that many
 *		  microseconds have passed.  If this is not included and the
 *		  mutex is busy, it will return DB_LOCK_NOTGRANTED.
 *		  
 *   MUTEX_CTR	- Tell successful exclusive mutex calls to update failchk's
 *		  counter (in DB_THREAD_INFO.mtx_ctr) of the number of locked
 *		  mutexes.  This flag is valid for both locking and unlocking.
 *		  It has no effect on shared latches.  Most mutexes are locked
 *		  with this flag.
 *
 * If any of these return DB_RUNRECOVERY, then the upper-case macros immediately
 * return that error code back to their caller.  Thus, functions calling
 * MUTEX_LOCK() need to return an integer error code.
 *
 * Whenever a function has a specific error path on a mutex failure, use
 *	int MUTEX_LOCK_RET(env, mutex)
 *	int MUTEX_UNLOCK_RET(env, mutex)
 * so that it may check for panic and perform some localized cleanup before
 * returning the DB_RUNRECOVERY.
 *
 * In rare circumstances a thread needs to unlock a non-self-blocking mutex that
 * was locked by another thread.  In particular, this is needed by failchk when
 * it is trying to perform a "soft" failchk cleanup after a thread has died while
 * in the Berkeley DB API, but which is not holding any exclusive mutexes.
 *
 *	int MUTEX_UNLOCK_IP(env, mutex, ip)
 */
#define MUTEX_WAIT		0x0001	/* Wait for it to become available. */
#define MUTEX_CTR		0x0002	/* Adjust the thread's mtx_ctr. */

/* The following macros are defined on some platforms, e.g. QNX. */
#undef __mutex_init
#undef __mutex_lock
#undef __mutex_unlock
#undef __mutex_destroy

#ifdef HAVE_MUTEX_SUPPORT

/*
 * Redirect mutex macros calls to one of the mutex implementation 'classes'.
 *	pthreads
 *	win32
 *	tas (test-and-set)
 * Hybrid mutexes are a variant of tas mutexes, using pthread condition variables
 * when busy, instead of a timed wait loop.
 */
#if !defined(HAVE_MUTEX_HYBRID) && (					\
    defined(HAVE_MUTEX_PTHREADS) ||					\
    defined(HAVE_MUTEX_SOLARIS_LWP) ||					\
    defined(HAVE_MUTEX_UI_THREADS))
#define	__mutex_init(e, m, f)		__db_pthread_mutex_init(e, m, f)
#define	__mutex_lock(e, m, t, f)	__db_pthread_mutex_lock(e, m, t, f)
#define	__mutex_unlock(e, m, ip, f)	__db_pthread_mutex_unlock(e, m, ip, f)
#define	__mutex_destroy(e, m)		__db_pthread_mutex_destroy(e, m)
#ifdef HAVE_SHARED_LATCHES
#define	__mutex_rdlock(e, m, f)		__db_pthread_mutex_readlock(e, m, f)
#endif
#elif defined(HAVE_MUTEX_WIN32) || defined(HAVE_MUTEX_WIN32_GCC)
#define	__mutex_init(e, m, f)		__db_win32_mutex_init(e, m, f)
#define	__mutex_lock(e, m, t, f)	__db_win32_mutex_lock(e, m, t, f)
#define	__mutex_unlock(e, m, ip, f)	__db_win32_mutex_unlock(e, m, ip, f)
#define	__mutex_destroy(e, m)		__db_win32_mutex_destroy(e, m)
#ifdef HAVE_SHARED_LATCHES
#define	__mutex_rdlock(e, m, f)		__db_win32_mutex_readlock(e, m, f)
#endif
#else
#define	__mutex_init(e, m, f)		__db_tas_mutex_init(e, m, f)
#define	__mutex_lock(e, m, t, f)	__db_tas_mutex_lock(e, m, t, f)
#define	__mutex_unlock(e, m, ip, f)	__db_tas_mutex_unlock(e, m, ip, f)
#define	__mutex_destroy(e, m)		__db_tas_mutex_destroy(e, m)
#if defined(HAVE_SHARED_LATCHES)
#define	__mutex_rdlock(e, m, f)		__db_tas_mutex_readlock(e, m, f)
#endif
#endif

/*
 * When there is no method to get a shared latch, fall back to implementing
 * __mutex_rdlock() as an exclusive one.
 */
#ifndef __mutex_rdlock
#define	__mutex_rdlock(a, b, f)		__mutex_lock(a, b, 0, f)
#endif

#define	MUTEX_LOCK_RET(env, mutex)					\
	((mutex) == MUTEX_INVALID ? 0 :					\
	__mutex_lock(env, mutex, 0, MUTEX_WAIT | MUTEX_CTR))

#define	MUTEX_LOCK(env, mutex) do {					\
	if (MUTEX_LOCK_RET((env), (mutex)) != 0)			\
		return (USR_ERR(env, DB_RUNRECOVERY));			\
} while (0)

#define	MUTEX_LOCK_NO_CTR(env, mutex) do {				\
	if ((mutex) != MUTEX_INVALID &&					\
	    __mutex_lock(env, mutex, 0, MUTEX_WAIT) != 0)		\
		return (USR_ERR(env, DB_RUNRECOVERY));			\
} while (0)

/*
 * MUTEX_TRYLOCK() --
 *	Try to exclusively lock a mutex without ever blocking - ever!
 *
 *	Returns 0 on success,
 *		DB_LOCK_NOTGRANTED if it is busy.
 *		Possibly DB_RUNRECOVERY if DB_ENV_FAILCHK or panic.
 *
 *	This can be called on shared latches too, though it always tries
 *	for exclusive access.
 * Always check the return value of MUTEX_TRYLOCK()!
 */
#define	MUTEX_TRYLOCK(env, mutex)					\
	((mutex) == MUTEX_INVALID ? 0 : __mutex_lock(env, mutex, 0, MUTEX_CTR))


/*
 * MUTEX_LOCK_TIMEOUT - Wait for at most 'timeout' microseconds to get a mutex.
 *
 * This call is currently used for a self blocking replication mutex, so it does
 * not need to be counted.  That may change if another mutex begins to use it.
 */
#define	MUTEX_LOCK_TIMEOUT(env, mutex, timeout) do {			\
	int __ret;							\
	if ((mutex) != MUTEX_INVALID &&	(__ret =			\
	    __mutex_lock(env, mutex, timeout, MUTEX_WAIT)) != 0 &&	\
	    __ret != DB_TIMEOUT)					\
		return (USR_ERR(env, DB_RUNRECOVERY));			\
} while (0)

/*
 * Unlock a mutex which belong to a specific thread, not necessarily the current
 * one.  This is used in some failchk code paths, specifically __memp_fput().
 */
#define	MUTEX_UNLOCK_IP(env, mutex, ip) do {					\
	if ((mutex) != MUTEX_INVALID &&					\
	    __mutex_unlock(env, mutex, ip, MUTEX_CTR) != 0)		\
		return (USR_ERR(env, DB_RUNRECOVERY));			\
} while (0)

#define	MUTEX_UNLOCK_RET(env, mutex)					\
    ((mutex) == MUTEX_INVALID ? 0 : __mutex_unlock(env, mutex, NULL, MUTEX_CTR))

#define	MUTEX_UNLOCK(env, mutex)	MUTEX_UNLOCK_IP(env, mutex, NULL)

/*
 * Used by __lock_promote when when waking up a waiting thread.  The mutex it is
 * releasing was locked by another thread so the counter does not include it.
 */
#define	MUTEX_UNLOCK_NO_CTR(env, mutex) do {				\
	if ((mutex) != MUTEX_INVALID &&					\
	    __mutex_unlock(env, mutex, NULL, 0) != 0)			\
		return (USR_ERR(env, DB_RUNRECOVERY));			\
} while (0)


/* Acquire a latch (a DB_MUTEX_SHARED "mutex") in shared mode. */
#define	MUTEX_READLOCK(env, mutex) do {					\
	if ((mutex) != MUTEX_INVALID &&					\
	    __mutex_rdlock(env, mutex, MUTEX_WAIT) != 0)		\
		return (USR_ERR(env, DB_RUNRECOVERY));			\
} while (0)

#define	MUTEX_TRY_READLOCK(env, mutex)					\
	((mutex) != MUTEX_INVALID ? __mutex_rdlock(env, mutex, 0) : 0)

/*
 * Check that a particular mutex isn't missing a MUTEX_LOCK().  This does not
 * determine whether the current thread is the owner.
 */
#define	MUTEX_IS_OWNED(env, mutex)					\
	(mutex == MUTEX_INVALID || !MUTEX_ON(env) ||			\
	F_ISSET(env->dbenv, DB_ENV_NOLOCKING) ||			\
	F_ISSET(MUTEXP_SET(env, mutex), DB_MUTEX_LOCKED))

#else
/*
 * There are calls to lock/unlock mutexes outside of #ifdef HAVE_MUTEX_SUPPORT.
 * Replace the call with something the compiler can discard, but which will make
 * if-then-else blocks work correctly, and suppress unused variable messages.
 */
#define	MUTEX_LOCK(env, mutex)		{ env = (env); mutex = (mutex); }
#define	MUTEX_UNLOCK(env, mutex)	{ env = (env); mutex = (mutex); }
#define	MUTEX_UNLOCK_IP(env, mutex, ip) { env = (env); mutex = (mutex); ip = (ip); }
#define	MUTEX_LOCK_NO_CTR(env, mutex)	{ env = (env); mutex = (mutex); }
#define	MUTEX_UNLOCK_NO_CTR(env, mutex)	{ env = (env); mutex = (mutex); }
#define	MUTEX_LOCK_RET(env, mutex)	( env = (env), mutex = (mutex), 0)
#define	MUTEX_UNLOCK_RET(env, mutex)	( env = (env), mutex = (mutex), 0)
#define	MUTEX_LOCK_TIMEOUT(env, mutex, timeout) \
	{ env = (env); mutex = (mutex); timeout = (timeout); }
#define	MUTEX_TRYLOCK(env, mutex)	( env = (env), mutex = (mutex), 0)
#define	MUTEX_READLOCK(env, mutex)	{ env = (env); mutex = (mutex); }
#define	MUTEX_TRY_READLOCK(env, mutex)	( env = (env), mutex = (mutex), 0 )
#define	MUTEX_REQUIRED(env, mutex)	{ env = (env); mutex = (mutex); }
#define	MUTEX_REQUIRED_READ(env, mutex)	{ env = (env); mutex = (mutex); }
#define	__mutex_unlock(env, mutex, ip, f)	( 0 )

/*
 * Every MUTEX_IS_OWNED() caller expects to own it. When there is no mutex
 * support, act as if we have ownership.
 */
#define	MUTEX_IS_OWNED(env, mutex)	1
#endif

/*
 * Bulk initialization of mutexes in regions.
 */

#define	MUTEX_BULK_INIT(env, region, start, howmany) do {		\
	DB_MUTEX *__mutexp;						\
	db_mutex_t __i = start;						\
	u_int32_t __n = howmany;					\
	for (__mutexp = MUTEXP_SET(env, __i);				\
	    --__n > 0;							\
	    __mutexp = MUTEXP_SET(env, __i)) {				\
		__mutexp->flags = 0;					\
		__i = (F_ISSET(env, ENV_PRIVATE)) ?			\
		    ((uintptr_t)__mutexp + region->mutex_size) : __i + 1; \
		__mutexp->mutex_next_link = __i;			\
	}								\
	__mutexp->flags = 0;						\
	__mutexp->mutex_next_link = MUTEX_INVALID;			\
} while (0)

/*
 * Berkeley DB ports may require single-threading at places in the code.
 */
#ifdef HAVE_MUTEX_VXWORKS
#include "taskLib.h"
/*
 * Use the taskLock() mutex to eliminate a race where two tasks are
 * trying to initialize the global lock at the same time.
 */
#define	DB_BEGIN_SINGLE_THREAD do {					\
	if (DB_GLOBAL(db_global_init))					\
		(void)semTake(DB_GLOBAL(db_global_lock), WAIT_FOREVER);	\
	else {								\
		taskLock();						\
		if (DB_GLOBAL(db_global_init)) {			\
			taskUnlock();					\
			(void)semTake(DB_GLOBAL(db_global_lock),	\
			    WAIT_FOREVER);				\
			continue;					\
		}							\
		DB_GLOBAL(db_global_lock) =				\
		    semBCreate(SEM_Q_FIFO, SEM_EMPTY);			\
		if (DB_GLOBAL(db_global_lock) != NULL)			\
			DB_GLOBAL(db_global_init) = 1;			\
		taskUnlock();						\
	}								\
} while (DB_GLOBAL(db_global_init) == 0)
#define	DB_END_SINGLE_THREAD	(void)semGive(DB_GLOBAL(db_global_lock))
#endif

/*
 * Single-threading defaults to a no-op.
 */
#ifndef DB_BEGIN_SINGLE_THREAD
#define	DB_BEGIN_SINGLE_THREAD
#endif
#ifndef DB_END_SINGLE_THREAD
#define	DB_END_SINGLE_THREAD
#endif

#if defined(__cplusplus)
}
#endif

#include "dbinc_auto/mutex_ext.h"
#endif /* !_DB_MUTEX_H_ */
