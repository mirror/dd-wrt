/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2005, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

#include "db_config.h"

#include "db_int.h"
#ifndef HAVE_SIMPLE_THREAD_TYPE
#include "dbinc/db_page.h"
#include "dbinc/hash.h"			/* Needed for call to __ham_func5. */
#endif
#include "dbinc/lock.h"
#include "dbinc/log.h"
#include "dbinc/mp.h"
#include "dbinc/txn.h"

static int __env_acquiring_mutex __P((ENV *, DB_THREAD_INFO *));
static int __env_holds_mutex __P((ENV *));
static int __env_in_failchk __P((ENV *, int *));
static int __env_clear_latches __P((ENV *));
static void __env_clear_state __P((ENV *));

/*
 * When failchk broadcast is enabled continue after the first error, to try to
 * find all of them; without broadcasting stop at the first failure.
 */
#ifdef HAVE_FAILCHK_BROADCAST
#define	FAILCHK_PROCESS_ERROR(t_ret, ret)	\
	if ((t_ret) != 0 && (ret) == 0)	(ret) = (t_ret)
#else
#define	FAILCHK_PROCESS_ERROR(t_ret, ret)	\
	if (((ret) = (t_ret)) != 0) goto err
#endif

/*
 * Set the default number of buckets to be 1/8th the number of
 * thread control blocks.  This is rather arbitrary.
 */
#define	DB_THREAD_INFOS_PER_BUCKET	8

/*
 * __env_failchk_pp --
 *	ENV->failchk pre/post processing.
 *
 *	Single process failchk continues after recoverable failures but stops as
 *	soon as recovery is required. Broadcast failchks continue even after
 *	DB_RUNRECOVERY failures are detected, to maximize the possibility to
 *	wake up processes blocked on dead resources, e.g. mutexes.
 *
 * PUBLIC: int __env_failchk_pp __P((DB_ENV *, u_int32_t));
 */
int
__env_failchk_pp(dbenv, flags)
	DB_ENV *dbenv;
	u_int32_t flags;
{
	DB_THREAD_INFO *ip;
	ENV *env;
	int in_failchk, ret;

	env = dbenv->env;
	in_failchk = ret = 0;

	ENV_ILLEGAL_BEFORE_OPEN(env, "DB_ENV->failchk");

	/*
	 * ENV->failchk requires self and is-alive functions.  We
	 * have a default self function, but no is-alive function.
	 */
	if (!ALIVE_ON(env)) {
		__db_errx(env, DB_STR("1503",
		    "DB_ENV->failchk requires DB_ENV->is_alive be configured"));
		return (EINVAL);
	}

	if (flags != 0)
		return (__db_ferr(env, "DB_ENV->failchk", 0));

	ENV_ENTER(env, ip);
        /*
         * Only run failchk if no other threads are currently running
         * failchk.  Note that since this operation is not mutex
         * protected it is possible for two threads to race and end
         * up both running failchk.  This is not considered an issue since
         * failchk will still run correctly even if multiple threads are
         * executing it at the same time.
         */
        ret = __env_in_failchk(env, &in_failchk);
        if (ret == 0 && !in_failchk) {
		FAILCHK_THREAD(env, ip);	/* mark as failchk thread */
		DB_TEST_CRASH(env->test_abort, DB_TEST_FAILCHK);
		ret = __env_failchk_int(dbenv);
        }
	ENV_LEAVE(env, ip);
	return (ret);
}

/*
 * __env_failchk_int --
 *	Process the subsystem failchk routines
 *
 *	The FAILCHK_PROCESS_ERROR macro (defined at the top of this file)
 *	differs between the broadcast and single process versions of failchk.
 *
 * PUBLIC: int __env_failchk_int __P((DB_ENV *));
 */
int
__env_failchk_int(dbenv)
	DB_ENV *dbenv;
{
	ENV *env;
	int ret, t_ret;

	env = dbenv->env;
	ret = 0;
	F_SET(dbenv, DB_ENV_FAILCHK);

	/*
	 * We check for dead threads that hold mutexes first as this would
	 * hang other things we try later, like locks and transactions.
	 */
	if ((ret = __env_holds_mutex(env)) != 0) {
		__db_err(env, ret, "__env_holds_mutex");
		goto err;
	}

        /*
         * Release any shared latches are held by a dead thread; they could
	 * interfere with failchk.
         */
        if ((t_ret = __env_clear_latches(env)) != 0)
		FAILCHK_PROCESS_ERROR(t_ret, ret);

	if (LOCKING_ON(env) && (t_ret = __lock_failchk(env)) != 0)
		FAILCHK_PROCESS_ERROR(t_ret, ret);

	if (TXN_ON(env) && ret == 0 && ((t_ret = __txn_failchk(env)) != 0 ||
	    (t_ret = __dbreg_failchk(env)) != 0))
		FAILCHK_PROCESS_ERROR(t_ret, ret);

	if ((t_ret = __memp_failchk(env)) != 0)
		FAILCHK_PROCESS_ERROR(t_ret, ret);

#ifdef HAVE_REPLICATION_THREADS
	if (REP_ON(env) && (t_ret = __repmgr_failchk(env)) != 0)
		FAILCHK_PROCESS_ERROR(t_ret, ret);
#endif

err:

#ifdef HAVE_MUTEX_SUPPORT
	/* Clean up mutexes only if failchk cleaned up without recovery. */
	if (ret == 0 && (t_ret = __mutex_failchk(env)) != 0)
	    ret = t_ret;
#endif

	/* Any dead blocked thread slots are no longer needed; allow reuse. */
	if (ret == 0)
		__env_clear_state(env);
	if (ret == DB_RUNRECOVERY) {
		/* Announce a panic; avoid __env_panic()'s diag core dump. */
		__env_panic_set(env, 1);
		__env_panic_event(env, ret);
	}
	F_CLR(dbenv, DB_ENV_FAILCHK);
	return (ret);
}

/*
 * __env_thread_size --
 *	Calculate the initial amount of memory for thread info blocks.
 *
 * PUBLIC: size_t __env_thread_size __P((ENV *, size_t));
 */
size_t
__env_thread_size(env, other_alloc)
	ENV *env;
	size_t other_alloc;
{
	DB_ENV *dbenv;
	size_t size;
	u_int32_t max;

	dbenv = env->dbenv;
	if ((max = dbenv->thr_max) < dbenv->thr_init)
		max = dbenv->thr_init;
	else if (max == 0 && ALIVE_ON(env) && (max = dbenv->tx_init) == 0) {
		/*
		 * They want thread tracking, but don't say how much.
		 * Arbitrarily assume 1/10 of the remaining memory or at
		 * least 100.
		 */
		if (dbenv->memory_max != 0)
			max = (u_int32_t)
			    (((dbenv->memory_max - other_alloc) / 10) /
				sizeof(DB_THREAD_INFO));
		if (max < 100)
			max = 100;
	}
	dbenv->thr_max = max;
	env->thr_nbucket = __db_tablesize(max / DB_THREAD_INFOS_PER_BUCKET);
	/*
	 * Include space for thread control block hash table and enough
	 * entries in each hash bucket for the initial number of threads.
	 */
	size = __env_alloc_size(env->thr_nbucket * sizeof(DB_HASHTAB));
	size += dbenv->thr_init * __env_alloc_size(sizeof(DB_THREAD_INFO));
	return (size);
}

/*
 * __env_thread_max --
 *	Return how much additional memory to reserve for threads.
 *
 * PUBLIC: size_t __env_thread_max __P((ENV *));
 */
size_t
__env_thread_max(env)
	ENV *env;
{
	DB_ENV *dbenv;
	size_t count;

	dbenv = env->dbenv;

	/*
	 * Allow for the worst case number of configured thread control blocks,
	 * plus 25%; then subtract the number of threads already allowed for.
	 */
	count = env->thr_nbucket * dbenv->thr_max;
	if (count < dbenv->thr_init)
		count = dbenv->thr_init;
	count += count / 4;
	return ((count - dbenv->thr_init) *
	    __env_alloc_size(sizeof(DB_THREAD_INFO)));
}

/*
 * __env_thread_init --
 *	Initialize the thread control block table.
 *
 * PUBLIC: int __env_thread_init __P((ENV *, int));
 */
int
__env_thread_init(env, during_creation)
	ENV *env;
	int during_creation;
{
	DB_ENV *dbenv;
	DB_HASHTAB *htab;
	REGENV *renv;
	REGINFO *infop;
	THREAD_INFO *thread;
	int ret;

	dbenv = env->dbenv;
	infop = env->reginfo;
	renv = infop->primary;

	if (renv->thread_off == INVALID_ROFF) {
		if (dbenv->thr_max == 0) {
			env->thr_hashtab = NULL;
			if (ALIVE_ON(env)) {
				__db_errx(env, DB_STR("1504",
		"is_alive method specified but no thread region allocated"));
				return (EINVAL);
			}
			return (0);
		}

		if (!during_creation) {
			__db_errx(env, DB_STR("1505",
"thread table must be allocated when the database environment is created"));
			return (EINVAL);
		}

		if ((ret =
		    __env_alloc(infop, sizeof(THREAD_INFO), &thread)) != 0) {
			__db_err(env, ret, DB_STR("1506",
			    "unable to allocate a thread status block"));
			return (ret);
		}
		memset(thread, 0, sizeof(*thread));
		renv->thread_off = R_OFFSET(infop, thread);
		thread->thr_nbucket =
		    __db_tablesize(dbenv->thr_max / DB_THREAD_INFOS_PER_BUCKET);
		if ((ret = __env_alloc(infop,
		     thread->thr_nbucket * sizeof(DB_HASHTAB), &htab)) != 0)
			return (ret);
		thread->thr_hashoff = R_OFFSET(infop, htab);
		__db_hashinit(htab, thread->thr_nbucket);
		thread->thr_max = dbenv->thr_max;
		thread->thr_init = dbenv->thr_init;
	} else {
		thread = R_ADDR(infop, renv->thread_off);
		htab = R_ADDR(infop, thread->thr_hashoff);
	}

	env->thr_hashtab = htab;
	env->thr_nbucket = thread->thr_nbucket;
	dbenv->thr_max = thread->thr_max;
	dbenv->thr_init = thread->thr_init;
	return (0);
}

/*
 * __env_thread_destroy --
 *	Destroy the thread control block table.
 *
 * PUBLIC: void __env_thread_destroy __P((ENV *));
 */
void
__env_thread_destroy(env)
	ENV *env;
{
	DB_HASHTAB *htab;
	DB_THREAD_INFO *ip, *np;
	REGENV *renv;
	REGINFO *infop;
	THREAD_INFO *thread;
	u_int32_t i;

	infop = env->reginfo;
	renv = infop->primary;
	if (renv->thread_off == INVALID_ROFF)
		return;

	thread = R_ADDR(infop, renv->thread_off);
	if ((htab = env->thr_hashtab) != NULL) {
		for (i = 0; i < env->thr_nbucket; i++) {
			ip = SH_TAILQ_FIRST(&htab[i], __db_thread_info);
			for (; ip != NULL; ip = np) {
				np = SH_TAILQ_NEXT(ip,
				    dbth_links, __db_thread_info);
				__env_alloc_free(infop, ip);
			}
		}
		__env_alloc_free(infop, htab);
	}

	__env_alloc_free(infop, thread);
	return;
}

/*
 *  __env_acquiring_mutex --
 *	Determine whether a (dead) thread was in the middle of a mutex lock or
 *	unlock call.
 */
static int
__env_acquiring_mutex(env, ip)
	ENV *env;
	DB_THREAD_INFO *ip;
{
	int i;

	for (i = 0; i != MUTEX_STATE_MAX; i++)
		if (ip->dbth_latches[i].action == MUTEX_ACTION_INTEND_SHARE)
			return (1);
	COMPQUIET(env, NULL);
	return (0);
}

/*
 * __env_holds_mutex --
 *	Look for threads which died in the api while either
 *	- holding an exclusive mutex, or
 *	- trying to acquire a shared latch.
 *
 *	If no threads died but there are blocked threads unpin
 *	any buffers they may have locked.
 */
static int
__env_holds_mutex(env)
	ENV *env;
{
	DB_ENV *dbenv;
	DB_HASHTAB *htab;
	DB_THREAD_INFO *ip;
	REGENV *renv;
	REGINFO *infop;
	THREAD_INFO *thread;
	u_int32_t i;
	pid_t pid;
	int unpin, ret, t_ret;

	if ((htab = env->thr_hashtab) == NULL)
		return (EINVAL);

	dbenv = env->dbenv;
	infop = env->reginfo;
	renv = infop->primary;
	thread = R_ADDR(infop, renv->thread_off);
	unpin = 0;
	ret = 0;

	for (i = 0; i < env->thr_nbucket; i++)
		SH_TAILQ_FOREACH(ip, &htab[i], dbth_links, __db_thread_info) {
			pid = ip->dbth_pid;
			if (ip->dbth_state == THREAD_SLOT_NOT_IN_USE ||
			    ip->dbth_state == THREAD_BLOCKED_DEAD ||
			    (ip->dbth_state == THREAD_OUT &&
			    thread->thr_count <  thread->thr_max))
				continue;
			if (dbenv->is_alive(
			    dbenv, ip->dbth_pid, ip->dbth_tid, 0))
				continue;
                        /*
                         * If a thread died in the API, and neither held any
			 * exclusive mutexes nor was it interrupted in the
			 * middle of a mutex call then it is safe to continue.
                         */
                        if (ip->dbth_state == THREAD_ACTIVE &&
                            ip->mtx_ctr == 0 &&
			    !__env_acquiring_mutex(env, ip)) {
				ip->dbth_state = THREAD_BLOCKED_DEAD;
				unpin = 1;
				continue;
                        }
			if (ip->dbth_state == THREAD_BLOCKED) {
				ip->dbth_state = THREAD_BLOCKED_DEAD;
				unpin = 1;
				continue;
			}
			if (ip->dbth_state == THREAD_OUT) {
				ip->dbth_state = THREAD_SLOT_NOT_IN_USE;
				continue;
			}
			/*
			 * The above tests are not atomic, so it is possible
			 * that the process pointed by ip has changed during
			 * the tests.  In particular, if the process pointed
			 * by ip when is_alive was executed terminated
			 * normally, a new process may reuse the same ip
			 * structure and change its dbth_state before the next
			 * two tests were performed. Therefore, we need to test
			 * here that all four tests above are done on the same
			 * process.  If the process pointed by ip changed, all
			 * tests are invalid and can be ignored.
			 * Similarly, it's also possible for two processes
			 * racing to change the dbth_state of the same ip
			 * structure. For example, both process A and B reach
			 * the above test for the same terminated process C
			 * where C's dbth_state is THREAD_OUT.  If A goes into
			 * the 'if' block and changes C's dbth_state to
			 * THREAD_SLOT_NOT_IN_USE before B checks the
			 * condition, B would incorrectly fail the test and
			 * run into this line.  Therefore, we need to check
			 * C's dbth_state again and fail the db only if C's
			 * dbth_state is indeed THREAD_ACTIVE.
			 */
			if (ip->dbth_state != THREAD_ACTIVE ||
			    ip->dbth_pid != pid)
				continue;
			__os_gettime(env, &ip->dbth_failtime, 0);
			t_ret = __db_failed(env, DB_STR("1507",
			    "Thread died in Berkeley DB library"),
			    ip->dbth_pid, ip->dbth_tid);
			if (ret == 0)
				ret = t_ret;
			/*
			 * Classic failchk stop after one dead thread in the
			 * api, but broadcasting looks for all.
			 */
#ifndef HAVE_FAILCHK_BROADCAST
			return (ret);
#endif
		}

	if (unpin && ret == 0) {
		for (i = 0; i < env->thr_nbucket; i++)
			SH_TAILQ_FOREACH(ip,
			    &htab[i], dbth_links, __db_thread_info)
				if (ip->dbth_state == THREAD_BLOCKED_DEAD &&
				    (t_ret =
					__memp_unpin_buffers(env, ip)) != 0) {
					if (ret == 0)
						ret = t_ret;
#ifndef HAVE_FAILCHK_BROADCAST
					return (ret);
#endif
				}
    	}

	return (ret);
}

/*
 * __env_in_failchk --
 *	Check if any other threads are running failchk.
 */
static int
__env_in_failchk(env, failchk)
	ENV *env;
        int *failchk;
{
	DB_ENV *dbenv;
	DB_HASHTAB *htab;
	DB_THREAD_INFO *ip;
	u_int32_t i;

	*failchk = 0;
	if ((htab = env->thr_hashtab) == NULL)
		return (0);

	dbenv = env->dbenv;

	for (i = 0; i < env->thr_nbucket; i++) {
		SH_TAILQ_FOREACH(ip, &htab[i], dbth_links, __db_thread_info) {
                        if (ip->dbth_state == THREAD_FAILCHK) {
                                // If a thread died in failchk, panic.
                                if (!dbenv->is_alive(
			            dbenv, ip->dbth_pid, ip->dbth_tid, 0)) {
				        __env_panic_set(env, 1);
		                        __env_panic_event(env, DB_RUNRECOVERY);
					return (DB_RUNRECOVERY);
                                }
                                *failchk = 1;
                                return (0);
                        }
                }
        }
        return (0);
}

/*
 * __env_clear_latches --
 *	Unlock shared latches held by dead processes, so that they won't block
 *	failchk.  Some of the latches which would otherwise cause problems
 *	include the ones for:
 *		mpool file hash buckets
 *		mpool buffer hash buckets
 *	Note that mpool buffer header mutexes have already been released by
 *	__memp_unpin_buffers(), so they are no longer present.
 */
static int
__env_clear_latches(env)
	ENV *env;
{
	DB_HASHTAB *htab;
	DB_THREAD_INFO *ip;
	u_int32_t i;
        int j, ret, t_ret;

        ret = t_ret = 0;
	htab = env->thr_hashtab;

	for (i = 0; i < env->thr_nbucket; i++)
		SH_TAILQ_FOREACH(ip, &htab[i], dbth_links, __db_thread_info) {
                        if (ip->dbth_state != THREAD_BLOCKED_DEAD)
				continue;
			for (j = 0; j < MUTEX_STATE_MAX; j++)
				if (ip->dbth_latches[j].action ==
				    MUTEX_ACTION_SHARED) {
					t_ret = __mutex_unlock(env,
					    ip->dbth_latches[j].mutex, ip, 0);
					if (t_ret != 0 && ret == 0)
						ret = t_ret;
				}
		}

	return (ret);
}

/*
 * __env_clear_state --
 *	Look for threads which died while blocked or in the API and clear them.
 */
static void
__env_clear_state(env)
	ENV *env;
{
	DB_HASHTAB *htab;
	DB_THREAD_INFO *ip;
	u_int32_t i;

	htab = env->thr_hashtab;
	for (i = 0; i < env->thr_nbucket; i++)
		SH_TAILQ_FOREACH(ip, &htab[i], dbth_links, __db_thread_info)
			if (ip->dbth_state == THREAD_BLOCKED_DEAD)
				ip->dbth_state = THREAD_SLOT_NOT_IN_USE;
}

struct __db_threadid {
	pid_t pid;
	db_threadid_t tid;
};

/*
 * __env_set_state --
 *	Set the state of the current thread's entry in the thread information
 *	table, alocating an entry if necessary, or look for its location without
 *	modifying the table. Do nothing if locking has been turned off.
 *
 * PUBLIC: int __env_set_state __P((ENV *, DB_THREAD_INFO **, DB_THREAD_STATE));
 */
int
__env_set_state(env, ipp, state)
	ENV *env;
	DB_THREAD_INFO **ipp;
	DB_THREAD_STATE state;
{
	struct __db_threadid id;
	DB_ENV *dbenv;
	DB_HASHTAB *htab;
	DB_THREAD_INFO *ip;
	REGENV *renv;
	REGINFO *infop;
	THREAD_INFO *thread;
	u_int32_t indx;
	int ret;

	dbenv = env->dbenv;
	htab = env->thr_hashtab;

	if (F_ISSET(dbenv, DB_ENV_NOLOCKING)) {
		*ipp = NULL;
		return (0);
	}
	dbenv->thread_id(dbenv, &id.pid, &id.tid);

	/*
	 * Hashing of thread ids.  This is simple but could be replaced with
	 * something more expensive if needed.
	 */
#ifdef HAVE_SIMPLE_THREAD_TYPE
	/*
	 * A thread ID may be a pointer, so explicitly cast to a pointer of
	 * the appropriate size before doing the bitwise XOR.
	 */
	indx = (u_int32_t)((uintptr_t)id.pid ^ (uintptr_t)id.tid);
#else
	indx = __ham_func5(NULL, &id.tid, sizeof(id.tid));
#endif
	indx %= env->thr_nbucket;
	SH_TAILQ_FOREACH(ip, &htab[indx], dbth_links, __db_thread_info) {
#ifdef HAVE_SIMPLE_THREAD_TYPE
		if (id.pid == ip->dbth_pid && id.tid == ip->dbth_tid)
			break;
#else
		if (memcmp(&id.pid, &ip->dbth_pid, sizeof(id.pid)) != 0)
			continue;
#ifdef HAVE_MUTEX_PTHREADS
		if (pthread_equal(id.tid, ip->dbth_tid) == 0)
#else
		if (memcmp(&id.tid, &ip->dbth_tid, sizeof(id.tid)) != 0)
#endif
			continue;
		break;
#endif
	}

	/*
	 * THREAD_VERIFY does not add the pid+thread to the table. The caller
	 * expects to find itself in the table already. If an ipp was passed in
	 * store the ip location there. Return whether or not it was found.
	 * In diagnostic mode a missing entry triggers the assert.
	 * THREAD_CTR_VERIFY is just like THREAD_VERIFY, except that it
	 * can be used when the thread state is THREAD_OUT.  It is used by the
	 * mutex counter code.
	 */
	if (state == THREAD_VERIFY || state == THREAD_CTR_VERIFY) {
		DB_ASSERT(env, ip != NULL
		    && (ip->dbth_state != THREAD_OUT
		    || state == THREAD_CTR_VERIFY));
		if (ipp != NULL)
			*ipp = ip;
		if (ip == NULL
		    || (ip->dbth_state == THREAD_OUT && state == THREAD_VERIFY))
			return (USR_ERR(env, EINVAL));
		else
			return (0);
	}

	*ipp = NULL;
	ret = 0;
	if (ip != NULL) {
		/*
		 * __env_set_state() gets called in many places, but a thread
		 * doing failchk is only supposed to be called to set the state
		 * to THREAD_OUT. Complain for any other state change.
		 */
		if (state != THREAD_OUT)
			DB_ASSERT(env, ip->dbth_state != THREAD_FAILCHK);
		ip->dbth_state = state;
	} else {
		infop = env->reginfo;
		renv = infop->primary;
		thread = R_ADDR(infop, renv->thread_off);
		/*
		 * Do not increment the counter until after the thread
		 * information is allocated.  Otherwise it kicks off an
		 * infinite loop of trying to allocate the thread info
		 * so it can increment the counter.
		 */
		MUTEX_LOCK_NO_CTR(env, renv->mtx_regenv);

		/*
		 * If we are passed the specified max, try to reclaim one from
		 * our bucket.  If failcheck has marked the slot not in use, we
		 * can take it, otherwise we must call is_alive before freeing
		 * it. We do not reclaim any entries from other buckets -- doing
		 * so safely would require lookups to lock mtx_regenv.
		 */
		if (thread->thr_count >= thread->thr_max) {
			SH_TAILQ_FOREACH(
			    ip, &htab[indx], dbth_links, __db_thread_info)
				if (ip->dbth_state == THREAD_SLOT_NOT_IN_USE ||
				    (ip->dbth_state == THREAD_OUT &&
				    ALIVE_ON(env) && !dbenv->is_alive(
				    dbenv, ip->dbth_pid, ip->dbth_tid, 0)))
					break;

			if (ip != NULL) {
				DB_ASSERT(env, ip->dbth_pincount == 0);
				goto init;
			}
		}

		thread->thr_count++;
		if ((ret = __env_alloc(infop,
		     sizeof(DB_THREAD_INFO), &ip)) == 0) {
			memset(ip, 0, sizeof(*ip));
			/*
			 * As long as inserting at the head can be 'committed'
			 * with a single memory write, this entry can be added
			 * without requiring any concurrent searches to
			 * do any locking.
			 */
			SH_TAILQ_INSERT_HEAD(
			    &htab[indx], ip, dbth_links, __db_thread_info);
			ip->dbth_pincount = 0;
			ip->dbth_pinmax = PINMAX;
			ip->dbth_pinlist = R_OFFSET(infop, ip->dbth_pinarray);

init:			ip->dbth_pid = id.pid;
			ip->dbth_tid = id.tid;
			if (renv->mtx_regenv != MUTEX_INVALID)
                                ip->mtx_ctr++;
			ip->dbth_state = state;
			for (indx = 0; indx != MUTEX_STATE_MAX; indx++)
				ip->dbth_latches[indx].mutex = MUTEX_INVALID;
			SH_TAILQ_INIT(&ip->dbth_xatxn);
		}
		MUTEX_UNLOCK(env, renv->mtx_regenv);
	}

	*ipp = ip;

	DB_ASSERT(env, ret == 0);
	if (ret != 0)
		__db_errx(env, DB_STR("1508",
		    "Unable to allocate thread control block"));
	return (ret);
}

/*
 * __env_thread_id_string --
 *	Convert a thread id to a string.
 *
 * PUBLIC: char *__env_thread_id_string
 * PUBLIC:     __P((DB_ENV *, pid_t, db_threadid_t, char *));
 */
char *
__env_thread_id_string(dbenv, pid, tid, buf)
	DB_ENV *dbenv;
	pid_t pid;
	db_threadid_t tid;
	char *buf;
{
#ifdef HAVE_SIMPLE_THREAD_TYPE
#ifdef UINT64_FMT
	char fmt[20];

	snprintf(fmt, sizeof(fmt), "%s/%s", INT64_FMT, UINT64_FMT);
	snprintf(buf,
	    DB_THREADID_STRLEN, fmt, (u_int64_t)pid, (u_int64_t)(uintptr_t)tid);
#else
	snprintf(buf, DB_THREADID_STRLEN, "%lu/%lu", (u_long)pid, (u_long)tid);
#endif
#else
#ifdef UINT64_FMT
	char fmt[20];

	snprintf(fmt, sizeof(fmt), "%s/TID", UINT64_FMT);
	snprintf(buf, DB_THREADID_STRLEN, fmt, (u_int64_t)pid);
#else
	snprintf(buf, DB_THREADID_STRLEN, "%lu/TID", (u_long)pid);
#endif
#endif
	COMPQUIET(dbenv, NULL);
	COMPQUIET(*(u_int8_t *)&tid, 0);

	return (buf);
}
