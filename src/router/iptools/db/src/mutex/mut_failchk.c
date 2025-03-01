/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2005, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

#include "db_config.h"

#include "db_int.h"
#include "dbinc/lock.h"

static int __mutex_failchk_single __P((ENV *, db_mutex_t, DB_THREAD_INFO *));

/*
 * __mutex_failchk --
 *	Clean up after dead processes which left behind allocated per-process or
 *	locked mutexes.
 *
 * PUBLIC: int __mutex_failchk __P((ENV *));
 */
int
__mutex_failchk(env)
	ENV *env;
{
	DB_HASHTAB *htab;
	DB_MUTEXMGR *mtxmgr;
	DB_MUTEXREGION *mtxregion;
	DB_THREAD_INFO *ip;
	db_mutex_t mutex;
	unsigned i;
	int count;

	if (F_ISSET(env, ENV_PRIVATE) || (htab = env->thr_hashtab) == NULL)
		return (0);

	mtxmgr = env->mutex_handle;
	mtxregion = mtxmgr->reginfo.primary;
	count = 0;

	DB_ASSERT(env, F_ISSET(env->dbenv, DB_ENV_FAILCHK));
	MUTEX_SYSTEM_LOCK(env);

	/*
	 * The first loop does each thread's read-locked latches; the second
	 * does all locked mutexes.
	 */
	for (i = 0; i < env->thr_nbucket; i++)
		SH_TAILQ_FOREACH(ip, &htab[i], dbth_links, __db_thread_info) {
			if (ip->dbth_state == THREAD_SLOT_NOT_IN_USE)
				continue;
			count += __mutex_failchk_thread(env, ip);
		}

	for (mutex = 1; mutex <= mtxregion->stat.st_mutex_cnt; mutex++)
		if (__mutex_failchk_single(env, mutex, NULL) != 0)
			count++;

	MUTEX_SYSTEM_UNLOCK(env);

	if (count == 0)
		return (count);
	else
		return (USR_ERR(env, DB_RUNRECOVERY));
}

/*
 * __mutex_failchk_thread -
 *	Do the per-latch failchk work on each of this thread's shared latches.
 *
 * PUBLIC: int __mutex_failchk_thread __P((ENV *, DB_THREAD_INFO *));
 */
int
__mutex_failchk_thread(env, ip)
	ENV *env;
	DB_THREAD_INFO *ip;
{
	db_mutex_t mutex;
	int count, i;

	count = 0;
	for (i = 0; i != MUTEX_STATE_MAX; i++) {
		if (ip->dbth_latches[i].action == MUTEX_ACTION_UNLOCKED ||
		    (mutex = ip->dbth_latches[i].mutex) == MUTEX_INVALID)
			continue;
		if (__mutex_failchk_single(env, mutex, ip) != 0)
			count++;
	}
	return (count);
}

/*
 * __mutex_failchk_single --
 *	Determine whether this mutex is locked or shared by a potentially
 *	dead thread. If so, and the call to is_alive() finds that it is dead,
 *	clean up if possible (a process-only mutex); else wake up any waiters.
 */
static int
__mutex_failchk_single(env, mutex, ip)
	ENV *env;
	db_mutex_t mutex;
	DB_THREAD_INFO *ip;
{
	DB_ENV *dbenv;
	DB_MUTEX *mutexp;
	db_threadid_t threadid;
	pid_t pid;
	int already_dead, i, ret, unlock_shared;
	u_int32_t flags;
	char id_str[DB_THREADID_STRLEN];
	char mtx_desc[DB_MUTEX_DESCRIBE_STRLEN];

	dbenv = env->dbenv;
	mutexp = MUTEXP_SET(env, mutex);
	flags = mutexp->flags;
	unlock_shared = 0;
	/*
	 * Filter out mutexes which couldn't possibly be "interesting", in order
	 * to reduce the number of possibly costly is_alive() calls. Check that:
	 *	it is allocated
	 *	is it either locked, or a shared latch, or a per-process mutex
	 *	it is nether a logical lock, nor self-block, nor already dead.
	 * Self-blocking mutexes are skipped because it is expected that they
	 * can still be locked even though they are really 'idle', as with
	 * the wait case in __lock_get_internal(), LOG->free_commits, and
	 * __rep_waiter->mtx_repwait; or they were allocated by the application.
	 */
	if (!LF_ISSET(DB_MUTEX_ALLOCATED))
		return (0);
	if (!LF_ISSET(
	    DB_MUTEX_SHARED | DB_MUTEX_LOCKED | DB_MUTEX_PROCESS_ONLY))
		return (0);
	if (LF_ISSET(DB_MUTEX_LOGICAL_LOCK | DB_MUTEX_OWNER_DEAD))
		return (0);
	if (LF_ISSET(DB_MUTEX_SELF_BLOCK) && !LF_ISSET(DB_MUTEX_SHARED))
		return (0);

	already_dead = ip != NULL && timespecisset(&ip->dbth_failtime);
	/*
	 * The pid in the mutex is valid when for locked or per-process mutexes.
	 * The tid is correct only when exclusively locked. It's okay to look at
	 * the tid of an unlocked per-process mutex, we won't use it in the
	 * is_alive() call.
	 */
	if (LF_ISSET(DB_MUTEX_LOCKED | DB_MUTEX_PROCESS_ONLY)) {
		pid = mutexp->pid;
		threadid = mutexp->tid;
	} else {
		DB_ASSERT(env, LF_ISSET(DB_MUTEX_SHARED));
		/*
		 * If we get here with no thread, then this is an shared latch
		 * which is neither locked nor shared, we're done with it.
		 */
		if (ip == NULL)
			return (0);

		/*
		 * Check if the thread is actually sharing the mutex, and
		 * not just waiting for the oppertunity to share it.
		 */
		for (i = 0; i != MUTEX_STATE_MAX; i++) {
			if (ip->dbth_latches[i].action == MUTEX_ACTION_SHARED
			    && mutex == ip->dbth_latches[i].mutex)
				unlock_shared = 1;
		}
		pid = ip->dbth_pid;
		threadid = ip->dbth_tid;
	}
	if (!already_dead && dbenv->is_alive(dbenv,
	    pid, threadid, LF_ISSET(DB_MUTEX_PROCESS_ONLY)))
		return (0);

	/* The thread is dead; the mutex type indicates the kind of cleanup. */
	(void)dbenv->thread_id_string(dbenv, pid, threadid, id_str);
	(void)__mutex_describe(env, mutex, mtx_desc);

	if (LF_ISSET(DB_MUTEX_PROCESS_ONLY)) {
		if (already_dead)
			return (0);

		__db_msg(env, DB_STR_A("2065",
		    "Freeing %s for process: %s", "%s %s"), mtx_desc, id_str);

		/* Clear the mutex id if it is in a cached locker. */
		if ((ret = __lock_local_locker_invalidate(env, mutex)) != 0)
			return (ret);

		/* Unlock and free the mutex. */
		if (LF_ISSET(DB_MUTEX_LOCKED))
			__mutex_unlock(env, mutex, ip, MUTEX_CTR);

		return (__mutex_free_int(env, 0, &mutex));
	/*
	 * If the thread is sharing a mutex, unshare it.  Note that full
	 * recovery is called if the thread is marked as intending to share.
	 * This is because it is impossible to tell if the thread got the
	 * mutex, but died before it could be marked as having gotten it.
	 */
	} else if (LF_ISSET(DB_MUTEX_SHARED) && unlock_shared) {
		if (already_dead)
			return (0);

		__db_msg(env, DB_STR_A("2078",
		    "Unlocking shared mutex %s of thread %s", "%lu %s"),
		    mtx_desc, id_str);

		/*
		 * We already know the thread held the latch only in shared
		 * mode, not exclusively, so no need to reduce the counter.
		 */
		(void)__mutex_unlock(env, mutex, ip, 0);

		return (0);
	}
#ifdef HAVE_FAILCHK_BROADCAST
	else if (LF_ISSET(DB_MUTEX_LOCKED)) {
		__db_errx(env, DB_STR_A("2066",
		    "Marking %s as owned by dead thread %s", "%lu %s"),
		    mtx_desc, id_str);
		F_SET(mutexp, DB_MUTEX_OWNER_DEAD);
	} else if (LF_ISSET(DB_MUTEX_SHARED)) {
		__db_errx(env, DB_STR_A("2067",
		    "Marking %s as shared by dead thread %s", "%lu %s"),
		    mtx_desc, id_str);
		F_SET(mutexp, DB_MUTEX_OWNER_DEAD);
	} else {
		__db_errx(env, DB_STR_A("2068",
	"mutex_failchk: unknown state for %s with dead thread %s", "%lu %s"),
		    mtx_desc, id_str);
	}
#endif
	return (USR_ERR(env, DB_RUNRECOVERY));
}
