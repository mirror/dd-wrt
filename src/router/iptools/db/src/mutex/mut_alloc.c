/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1999, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

#include "db_config.h"

#include "db_int.h"
#include "dbinc/log.h"

static char *__mutex_action_string __P((MUTEX_ACTION));

/*
 * __mutex_alloc --
 *	Allocate a mutex from the mutex region.
 *
 * PUBLIC: int __mutex_alloc __P((ENV *, int, u_int32_t, db_mutex_t *));
 */
int
__mutex_alloc(env, alloc_id, flags, indxp)
	ENV *env;
	int alloc_id;
	u_int32_t flags;
	db_mutex_t *indxp;
{
	/* The caller may depend on us to initialize. */
	*indxp = MUTEX_INVALID;

	/*
	 * If this is not an application lock, and we've turned off locking,
	 * or the ENV handle isn't thread-safe, and this is a thread lock
	 * or the environment isn't multi-process by definition, there's no
	 * need to mutex at all.
	 */
	if (alloc_id != MTX_APPLICATION && alloc_id != MTX_MUTEX_TEST &&
	    (F_ISSET(env->dbenv, DB_ENV_NOLOCKING) ||
	    (!F_ISSET(env, ENV_THREAD) &&
	    (LF_ISSET(DB_MUTEX_PROCESS_ONLY) || F_ISSET(env, ENV_PRIVATE)))))
		return (0);

	/* Private environments never share mutexes. */
	if (F_ISSET(env, ENV_PRIVATE))
		LF_SET(DB_MUTEX_PROCESS_ONLY);

	/*
	 * If we have a region in which to allocate the mutexes, lock it and
	 * do the allocation.
	 */
	if (!MUTEX_ON(env)) {
		__db_errx(env, DB_STR("2033",
		    "Mutex allocated before mutex region."));
		return (__env_panic(env, EINVAL));
	}
	return (__mutex_alloc_int(env, 1, alloc_id, flags, indxp));
}

/*
 * __mutex_alloc_int --
 *	Internal routine to allocate a mutex.
 *
 * PUBLIC: int __mutex_alloc_int
 * PUBLIC:	__P((ENV *, int, int, u_int32_t, db_mutex_t *));
 */
int
__mutex_alloc_int(env, locksys, alloc_id, flags, indxp)
	ENV *env;
	int locksys, alloc_id;
	u_int32_t flags;
	db_mutex_t *indxp;
{
	DB_ENV *dbenv;
	DB_MUTEX *mutexp;
	DB_MUTEXMGR *mtxmgr;
	DB_MUTEXREGION *mtxregion;
	db_mutex_t i;
	size_t len;
	u_int32_t cnt;
	int ret;

	dbenv = env->dbenv;
	mtxmgr = env->mutex_handle;
	mtxregion = mtxmgr->reginfo.primary;
	ret = 0;

	/*
	 * If we're not initializing the mutex region, then lock the region to
	 * allocate new mutexes.  Drop the lock before initializing the mutex,
	 * mutex initialization may require a system call.
	 */
	if (locksys)
		MUTEX_SYSTEM_LOCK(env);

	if (mtxregion->mutex_next == MUTEX_INVALID) {
		if (mtxregion->stat.st_mutex_max != 0 &&
		    mtxregion->stat.st_mutex_cnt >=
		    mtxregion->stat.st_mutex_max) {
nomem:			__db_errx(env, DB_STR("2034",
	    "unable to allocate memory for mutex; resize mutex region"));
			if (locksys)
				MUTEX_SYSTEM_UNLOCK(env);
			return (ret == 0 ? ENOMEM : ret);
		}
		cnt = mtxregion->stat.st_mutex_cnt / 2;
		if (cnt < 8)
			cnt = 8;
		if (mtxregion->stat.st_mutex_max != 0 &&
		    mtxregion->stat.st_mutex_cnt + cnt >
		    mtxregion->stat.st_mutex_max)
			cnt = mtxregion->stat.st_mutex_max -
			    mtxregion->stat.st_mutex_cnt;

		/* Set i to the first newly created db_mutex_t. */
		if (F_ISSET(env, ENV_PRIVATE)) {
			F_SET(&mtxmgr->reginfo, REGION_TRACKED);
			while (__env_alloc(&mtxmgr->reginfo,
			    (cnt * mtxregion->mutex_size) +
			    mtxregion->stat.st_mutex_align, &i) != 0) {
				cnt >>= 1;
				if (cnt == 0)
					break;
			}
			F_CLR(&mtxmgr->reginfo, REGION_TRACKED);
			i = (db_mutex_t)ALIGNP_INC(i,
			    mtxregion->stat.st_mutex_align);
		} else {
			len = cnt * mtxregion->mutex_size;
			if ((ret = __env_alloc_extend(&mtxmgr->reginfo,
			    R_ADDR(&mtxmgr->reginfo,
			    mtxregion->mutex_off_alloc), &len)) != 0)
				goto nomem;
			cnt = (u_int32_t)(len / mtxregion->mutex_size);
			i = mtxregion->stat.st_mutex_cnt + 1;
		}
		if (cnt == 0)
			goto nomem;

		mtxregion->stat.st_mutex_free = cnt;
		mtxregion->mutex_next = i;
		mtxregion->stat.st_mutex_cnt += cnt;

		/*
		 * Now link the rest of the newly allocated db_mutex_t's into
		 * the free list.
		 */
		MUTEX_BULK_INIT(env, mtxregion, i, cnt);
	}

	*indxp = mtxregion->mutex_next;
	mutexp = MUTEXP_SET(env, *indxp);
	DB_ASSERT(env,
	    ((uintptr_t)mutexp & (dbenv->mutex_align - 1)) == 0);
	mtxregion->mutex_next = mutexp->mutex_next_link;

	--mtxregion->stat.st_mutex_free;
	++mtxregion->stat.st_mutex_inuse;
	if (mtxregion->stat.st_mutex_inuse > mtxregion->stat.st_mutex_inuse_max)
		mtxregion->stat.st_mutex_inuse_max =
		    mtxregion->stat.st_mutex_inuse;

	/* Initialize the mutex. */
	memset(mutexp, 0, sizeof(*mutexp));
	F_SET(mutexp, DB_MUTEX_ALLOCATED |
	    LF_ISSET(DB_MUTEX_LOGICAL_LOCK | DB_MUTEX_PROCESS_ONLY |
		DB_MUTEX_SELF_BLOCK | DB_MUTEX_SHARED));

	/*
	 * If the mutex is associated with a single process, set the process
	 * ID.  If the application ever calls DbEnv::failchk, we'll need the
	 * process ID to know if the mutex is still in use.
	 */
	if (LF_ISSET(DB_MUTEX_PROCESS_ONLY))
		dbenv->thread_id(dbenv, &mutexp->pid, NULL);

#ifdef HAVE_STATISTICS
	mutexp->alloc_id = alloc_id;
#else
	COMPQUIET(alloc_id, 0);
#endif

	if ((ret = __mutex_init(env, *indxp, flags)) != 0)
		(void)__mutex_free_int(env, 0, indxp);
	if (locksys)
		MUTEX_SYSTEM_UNLOCK(env);

	return (ret);
}

/*
 * __mutex_free --
 *	Free a mutex.
 *
 * PUBLIC: int __mutex_free __P((ENV *, db_mutex_t *));
 */
int
__mutex_free(env, indxp)
	ENV *env;
	db_mutex_t *indxp;
{
	/*
	 * There is no explicit ordering in how the regions are cleaned up
	 * up and/or discarded when an environment is destroyed (either a
	 * private environment is closed or a public environment is removed).
	 * The way we deal with mutexes is to clean up all remaining mutexes
	 * when we close the mutex environment (because we have to be able to
	 * do that anyway, after a crash), which means we don't have to deal
	 * with region cleanup ordering on normal environment destruction.
	 * All that said, what it really means is we can get here without a
	 * mpool region.  It's OK, the mutex has been, or will be, destroyed.
	 *
	 * If the mutex has never been configured, we're done.
	 */
	if (!MUTEX_ON(env) || *indxp == MUTEX_INVALID)
		return (0);

	return (__mutex_free_int(env, 1, indxp));
}

/*
 * __mutex_free_int --
 *	Internal routine to free a mutex.
 *
 * PUBLIC: int __mutex_free_int __P((ENV *, int, db_mutex_t *));
 */
int
__mutex_free_int(env, locksys, indxp)
	ENV *env;
	int locksys;
	db_mutex_t *indxp;
{
	DB_MUTEX *mutexp;
	DB_MUTEXMGR *mtxmgr;
	DB_MUTEXREGION *mtxregion;
	db_mutex_t mutex;
	int ret;

	mutex = *indxp;
	*indxp = MUTEX_INVALID;

	mtxmgr = env->mutex_handle;
	mtxregion = mtxmgr->reginfo.primary;
	mutexp = MUTEXP_SET(env, mutex);

	DB_ASSERT(env, F_ISSET(mutexp, DB_MUTEX_ALLOCATED));
	F_CLR(mutexp, DB_MUTEX_ALLOCATED);

	ret = __mutex_destroy(env, mutex);

	if (locksys)
		MUTEX_SYSTEM_LOCK(env);

	/* Link the mutex on the head of the free list. */
	mutexp->mutex_next_link = mtxregion->mutex_next;
	mtxregion->mutex_next = mutex;
	++mtxregion->stat.st_mutex_free;
	--mtxregion->stat.st_mutex_inuse;

	if (locksys)
		MUTEX_SYSTEM_UNLOCK(env);

	return (ret);
}

#ifdef HAVE_FAILCHK_BROADCAST
/*
 * __mutex_died --
 *	Announce that a mutex request couldn't been granted because the last
 *	thread to own it was killed by failchk. Sets ENV_DEAD_MUTEX in the
 *	possibly shared environment so that mutex unlock calls don't complain.
 *
 *
 * PUBLIC: int __mutex_died __P((ENV *, db_mutex_t));
 */
int
__mutex_died(env, mutex)
	ENV *env;
	db_mutex_t mutex;
{
	DB_ENV *dbenv;
	DB_EVENT_MUTEX_DIED_INFO info;
	DB_MUTEX *mutexp;
	char tidstr[DB_THREADID_STRLEN], failmsg[DB_FAILURE_SYMPTOM_SIZE];

	dbenv = env->dbenv;

	mutexp = MUTEXP_SET(env, mutex);
	info.mutex = mutex;
	info.pid = mutexp->pid;
	info.tid = mutexp->tid;
	(void)dbenv->thread_id_string(dbenv, mutexp->pid, mutexp->tid, tidstr);
	(void)__mutex_describe(env, mutex, info.desc);
	(void)snprintf(failmsg, sizeof(failmsg), DB_STR_A("2073",
	    "Mutex died: %s owned %s", "%s %s"), tidstr, info.desc);
	__db_errx(env, "%s", failmsg);
	/* If this is the first crashed process, save its description. */
	(void)__env_failure_remember(env, failmsg);
	DB_EVENT(env, DB_EVENT_MUTEX_DIED, &info);
	return (__env_panic(env, USR_ERR(env, DB_RUNRECOVERY)));
}
#endif

/*
 * __mutex_refresh --
 *	Reinitialize a mutex, if we are not sure of its state.
 *
 * PUBLIC: int __mutex_refresh __P((ENV *, db_mutex_t));
 */
int
__mutex_refresh(env, mutex)
	ENV *env;
	db_mutex_t mutex;
{
	DB_MUTEX *mutexp;
	u_int32_t flags;
	int ret;

	mutexp = MUTEXP_SET(env, mutex);
	flags = mutexp->flags;
	if ((ret = __mutex_destroy(env, mutex)) == 0) {
		memset(mutexp, 0, sizeof(*mutexp));
		F_SET(mutexp, DB_MUTEX_ALLOCATED |
		    LF_ISSET(DB_MUTEX_LOGICAL_LOCK |
			DB_MUTEX_PROCESS_ONLY | DB_MUTEX_SHARED));
		LF_CLR(DB_MUTEX_LOCKED);
		ret = __mutex_init(env, mutex, flags);
	}
	return (ret);
}

/*
 * __mutex_record_lock --
 *	Record that this thread is about to lock a latch.
 *	The last parameter is updated to point to this mutex's entry in the
 *	per-thread mutex state array, so that it can update it if it gets the
 *	mutex, or free it if the mutex is not acquired (e.g. it times out).
 *	Mutexes which can be unlocked by other threads are not placed in this
 *	list, because it would be too costly for that other thread to to find
 *	the right slot to clear. The caller has already checked that thread
 *	tracking is enabled.
 *
 * PUBLIC: int __mutex_record_lock __P((ENV *,
 * PUBLIC:     db_mutex_t, DB_THREAD_INFO *, MUTEX_ACTION, MUTEX_STATE **));
 */
int
__mutex_record_lock(env, mutex, ip, action, retp)
	ENV *env;
	db_mutex_t mutex;
	DB_THREAD_INFO *ip;
	MUTEX_ACTION action;
	MUTEX_STATE **retp;
{
	DB_MUTEX *mutexp;
	int i;

	*retp = NULL;
	mutexp = MUTEXP_SET(env, mutex);
	if (!F_ISSET(mutexp, DB_MUTEX_SHARED))
		return (0);
	for (i = 0; i != MUTEX_STATE_MAX; i++) {
		if (ip->dbth_latches[i].action == MUTEX_ACTION_UNLOCKED) {
			ip->dbth_latches[i].mutex = mutex;
			ip->dbth_latches[i].action = action;
#ifdef DIAGNOSTIC
			__os_gettime(env, &ip->dbth_latches[i].when, 0);
#endif
			*retp = &ip->dbth_latches[i];
			return (0);
		}
	}
	__db_errx(env, DB_STR_A("2074",
	    "No space available in latch table for %lu", "%lu"), (u_long)mutex);
	(void)__mutex_record_print(env, ip);
	return (__env_panic(env, USR_ERR(env, DB_RUNRECOVERY)));
}

/*
 * __mutex_record_unlock --
 *	Clear the record that ip's thread has a shared latch for shared access.
 *
 * PUBLIC: int __mutex_record_unlock
 * PUBLIC:     __P((ENV *, db_mutex_t, DB_THREAD_INFO *));
 */
int
__mutex_record_unlock(env, mutex, ip)
	ENV *env;
	db_mutex_t mutex;
	DB_THREAD_INFO *ip;
{
	int i, ret;

	for (i = 0; i != MUTEX_STATE_MAX; i++) {
		if (ip->dbth_latches[i].mutex == mutex &&
		    ip->dbth_latches[i].action != MUTEX_ACTION_UNLOCKED) {
			ip->dbth_latches[i].action = MUTEX_ACTION_UNLOCKED;
			return (0);
		}
	}
	/*
	 * It wasn't there. Panic unless this is the failchk thread.
	 * It is hard to tell whether this is a dead thread's lock. Fix?
	 */
	if (F_ISSET(env->dbenv, DB_ENV_FAILCHK) &&
	    ip->dbth_state == THREAD_FAILCHK) {
		DB_DEBUG_MSG(env, "mutex_record_unlock %lu by failchk thread",
		    (u_long)mutex);
		return (0);

	} else {
		ret = USR_ERR(env, DB_RUNRECOVERY);
		(void)__mutex_record_print(env, ip);
		__db_errx(env, DB_STR_A("2075",
		    "Latch %lu was not held", "%lu"), (u_long)mutex);
		return (__env_panic(env, ret));
	}
}


/*
 * __mutex_action_string --
 *	Map a MUTEX_ACTION_XXX enum to its string description.
 */
static char *
__mutex_action_string(action)
	MUTEX_ACTION action;
{
	switch (action) {
	case MUTEX_ACTION_UNLOCKED:
		return ("unlocked");
	case MUTEX_ACTION_INTEND_SHARE:
		return ("waiting to share");
	case MUTEX_ACTION_SHARED:
		return ("sharing");
	default:
		return ("unknown");
	}
	/* NOTREACHED */
}

/*
 * __mutex_record_print --
 *	Display the thread's mutex state via __db_msg(), including any
 *	information which would be relevant for db_stat or diagnostic messages.
 *
 * PUBLIC: int __mutex_record_print __P((ENV *, DB_THREAD_INFO *));
 */
int
__mutex_record_print(env, ip)
	ENV *env;
	DB_THREAD_INFO *ip;
{
	DB_MSGBUF mb, *mbp;
	MUTEX_STATE *state;
	db_mutex_t mutex;
	int i;
	char desc[DB_MUTEX_DESCRIBE_STRLEN];
	char time_buf[CTIME_BUFLEN];

	DB_MSGBUF_INIT(&mb);
	mbp = &mb;
	for (i = 0; i != MUTEX_STATE_MAX; i++) {
		state = &ip->dbth_latches[i];
		if (state->action == MUTEX_ACTION_UNLOCKED ||
		    (mutex = state->mutex) == MUTEX_INVALID)
			continue;
		time_buf[0] = '\0';
#ifdef DIAGNOSTIC
		if (timespecisset(&state->when))
			(void)__db_ctimespec(&state->when, time_buf);
#endif

		__db_msgadd(env, mbp, "%s %s %s ",
		    __mutex_describe(env, mutex, desc),
		    __mutex_action_string(state->action), time_buf);
#ifdef HAVE_STATISTICS
		__mutex_print_debug_stats(env, mbp, mutex, 0);
#endif
		DB_MSGBUF_FLUSH(env, mbp);
	}
	return (0);
}
