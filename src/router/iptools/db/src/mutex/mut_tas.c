/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

#include "db_config.h"

#include "db_int.h"
#include "dbinc/lock.h"

/*
 * __db_tas_mutex_init --
 *	Initialize a test-and-set mutex.
 *
 * PUBLIC: int __db_tas_mutex_init __P((ENV *, db_mutex_t, u_int32_t));
 */
int
__db_tas_mutex_init(env, mutex, flags)
	ENV *env;
	db_mutex_t mutex;
	u_int32_t flags;
{
	DB_ENV *dbenv;
	DB_MUTEX *mutexp;
	int ret;

#ifndef HAVE_MUTEX_HYBRID
	COMPQUIET(flags, 0);
#endif

	dbenv = env->dbenv;
	mutexp = MUTEXP_SET(env, mutex);

	/* Check alignment. */
	if (((uintptr_t)mutexp & (dbenv->mutex_align - 1)) != 0) {
		__db_errx(env, DB_STR("2028",
		    "TAS: mutex not appropriately aligned"));
		return (EINVAL);
	}

#ifdef HAVE_SHARED_LATCHES
	if (F_ISSET(mutexp, DB_MUTEX_SHARED))
		atomic_init(&mutexp->sharecount, 0);
	else
#endif
	if (MUTEX_INIT(&mutexp->tas)) {
		ret = __os_get_syserr();
		__db_syserr(env, ret, DB_STR("2029", "TAS: mutex initialize"));
		return (__os_posix_err(ret));
	}
#ifdef HAVE_MUTEX_HYBRID
	if ((ret = __db_pthread_mutex_init(env,
	     mutex, flags | DB_MUTEX_SELF_BLOCK)) != 0)
		return (ret);
#endif
	return (0);
}

/*
 * __db_tas_mutex_lock --
 *	Exclusive lock function for test-and-set mutexes/latches.

 *	On error, leave any mtx_ctr increment in place -- failchk must not
 *	attempt recovering from an incompletely acquired mutex.
 *
 * PUBLIC: int __db_tas_mutex_lock
 * PUBLIC:     __P((ENV *, db_mutex_t, db_timeout_t, u_int32_t));
 */
int
__db_tas_mutex_lock(env, mutex, timeout, flags)
	ENV *env;
	db_mutex_t mutex;
	db_timeout_t timeout;
	u_int32_t flags;
{
	DB_ENV *dbenv;
	DB_MUTEX *mutexp;
	DB_MUTEXMGR *mtxmgr;
	DB_MUTEXREGION *mtxregion;
	DB_THREAD_INFO *ip;
	db_timespec now, timeout_timespec;
	u_int32_t nspins;
	u_long micros;
	int ret;
	char buf[DB_THREADID_STRLEN];
#ifndef HAVE_MUTEX_HYBRID
	u_long max_micros;
	db_timeout_t time_left;
#endif

	dbenv = env->dbenv;

	if (!MUTEX_ON(env) || F_ISSET(dbenv, DB_ENV_NOLOCKING))
		return (0);

	PANIC_CHECK(env);

	mtxmgr = env->mutex_handle;
	mtxregion = mtxmgr->reginfo.primary;
	mutexp = MUTEXP_SET(env, mutex);

	if (F_ISSET(mutexp, DB_MUTEX_LOCKED))
		STAT_INC(env, mutex, set_wait, mutexp->mutex_set_wait, mutex);
	else
		STAT_INC(env,
		    mutex, set_nowait, mutexp->mutex_set_nowait, mutex);

#ifdef HAVE_MUTEX_HYBRID
	micros = 0;
#else
	/*
	 * Wait 1ms initially, up to 10ms for mutexes backing logical database
	 * locks, and up to 25 ms for mutual exclusion data structure mutexes.
	 * SR: #7675
	 */
	micros = 1000;
	max_micros = F_ISSET(mutexp, DB_MUTEX_LOGICAL_LOCK) ? 10000 : 25000;
#endif

	/* Clear the ending timespec so it'll be initialized upon first need. */
	if (timeout != 0)
		timespecclear(&timeout_timespec);

	 /*
	  * Fetch the current thread's state struct, if there is a thread hash
	  * table and we are keeping track of this mutex.  We increment the
	  * mtx_ctr before we know that this call succeeds, and decrement it on
	  * failure or in __db_pthread_mutex_unlock.  By incrementing it before
	  * the attempt, we detect crashing that occur during this function.
	  * A crash while holding a non-counted mutex will not be detected by
	  * failchk's mtx_ctr code: those have to be detected some other way.
	  */
	ip = NULL;
	if (env->thr_hashtab != NULL && LF_ISSET(MUTEX_CTR)) {
		if ((ret = __env_set_state(env, &ip, THREAD_CTR_VERIFY)) != 0)
			return (__env_panic(env, ret));
		if (ip != NULL) {
			DB_ASSERT(env, ip->mtx_ctr < 20);
			ip->mtx_ctr++;
		}
	}

loop:	/* Attempt to acquire the resource for N spins. */
	for (nspins =
	    mtxregion->stat.st_mutex_tas_spins; nspins > 0; --nspins) {
#ifdef HAVE_MUTEX_S390_CC_ASSEMBLY
		tsl_t zero;

		zero = 0;
#endif

#ifdef HAVE_MUTEX_HPPA_MSEM_INIT
	relock:
#endif
		/*
		 * Avoid interlocked instructions until they're likely to
		 * succeed by first checking whether it is held
		 */
		if (MUTEXP_IS_BUSY(mutexp) || !MUTEXP_ACQUIRE(mutexp)) {
			if (F_ISSET(dbenv, DB_ENV_FAILCHK) && ip != NULL &&
			    dbenv->is_alive(dbenv,
			    mutexp->pid, mutexp->tid, 0) == 0) {
				/*
				 * The process owing the mutex is "dead" now,
				 * but it may have already released the mutex.
				 * We need to check again by going back to the
				 * top of the loop if the mutex is still held
				 * by the "dead" process. We yield 10 us to
				 * increase the likelyhood of mutexp fields
				 * being up-to-date. Spin just one more time;
				 * there is no need for additional spins if
				 * the owner is dead.
				 */
				if (nspins > 1) {
					nspins = 2;
					__os_yield(env, 0, 10);
					continue;
				}
				if (ip->dbth_state == THREAD_FAILCHK) {
					ret = USR_ERR(env, DB_RUNRECOVERY);
					__db_err(env, ret,
			    "Failchk blocked by dead process %s on mutex %ld",
					    dbenv->thread_id_string(dbenv,
					    mutexp->pid, mutexp->tid, buf),
					    (u_long)mutex);
					return (ret);
				}
						
			}
			if (!LF_ISSET(MUTEX_WAIT)) {
				/*
				 * Since DB_LOCK_NOTGRANTED has not corrupted
				 * the mutex, failchk can still clean up without
				 * recovery.
				 */
				if (ip != NULL) {
					DB_ASSERT(env, ip->mtx_ctr > 0);
					ip->mtx_ctr--;
				}
				return (USR_ERR(env, DB_LOCK_NOTGRANTED));
			}
			/*
			 * Some systems (notably those with newer Intel CPUs)
			 * need a small pause here. [#6975]
			 */
			MUTEX_PAUSE
			continue;
		}

		MEMBAR_ENTER();

#ifdef HAVE_MUTEX_HPPA_MSEM_INIT
		/*
		 * HP semaphores are unlocked automatically when a holding
		 * process exits.  If the mutex appears to be locked
		 * (F_ISSET(DB_MUTEX_LOCKED)) but we got here, assume this
		 * has happened.  Set the pid and tid into the mutex and
		 * lock again.  (The default state of the mutexes used to
		 * block in __lock_get_internal is locked, so exiting with
		 * a locked mutex is reasonable behavior for a process that
		 * happened to initialize or use one of them.)
		 */
		if (F_ISSET(mutexp, DB_MUTEX_LOCKED)) {
			dbenv->thread_id(dbenv, &mutexp->pid, &mutexp->tid);
			goto relock;
		}
		/*
		 * If we make it here, the mutex isn't locked, the diagnostic
		 * won't fire, and we were really unlocked by someone calling
		 * the DB mutex unlock function.
		 */
#endif
#ifdef HAVE_FAILCHK_BROADCAST
		if (F_ISSET(mutexp, DB_MUTEX_OWNER_DEAD)) {
			MUTEX_UNSET(&mutexp->tas);
			return (__mutex_died(env, mutex));
		}
#endif
#ifdef DIAGNOSTIC
		if (F_ISSET(mutexp, DB_MUTEX_LOCKED)) {
			__db_errx(env, DB_STR_A("2030",
		    "TAS lock failed: lock %ld currently in use: ID: %s",
			    "%ld %s"), (long)mutex,
			    dbenv->thread_id_string(dbenv,
			    mutexp->pid, mutexp->tid, buf));
			return (__env_panic(env, EACCES));
		}
#endif
		F_SET(mutexp, DB_MUTEX_LOCKED);
		dbenv->thread_id(dbenv, &mutexp->pid, &mutexp->tid);
#if defined(MUTEX_DIAG)
		__os_gettime(env, &mutexp->mutex_history.when, 0);
		/* Why 3? Skip __os_stack_text, __db_tas_mutex_lock{_int,} */
		__os_stack_text(env, mutexp->mutex_history.stacktext,
		    sizeof(mutexp->mutex_history.stacktext), 12, 3);
#endif

#ifdef DIAGNOSTIC
		/*
		 * We want to switch threads as often as possible.  Yield
		 * every time we get a mutex to ensure contention.
		 */
		if (F_ISSET(dbenv, DB_ENV_YIELDCPU))
			__os_yield(env, 0, 0);
#endif
		/*
		 * On successful returns do not touch mtx_ctr; if it was
		 * incremented it stays that way.
		 */
		return (0);
	}

	/*
	 * We need to wait for the lock to become available.  Setup timeouts if
	 * this is the first wait, or the failchk timeout is smaller than the
	 * wait timeout. Check expiration times for subsequent waits.
	 */
	if (timeout != 0) {
		/* Set the expiration time if this is the first sleep . */
		if (!timespecisset(&timeout_timespec))
			__clock_set_expires(env, &timeout_timespec, timeout);
		else {
			timespecclear(&now);
			if (__clock_expired(env, &now, &timeout_timespec)) {
				if (ip != NULL) {
					DB_ASSERT(env, ip->mtx_ctr > 0);
					ip->mtx_ctr--;
				}
				return (USR_ERR(env, DB_TIMEOUT));
			}
#ifndef HAVE_MUTEX_HYBRID
			timespecsub(&now, &timeout_timespec);
			DB_TIMESPEC_TO_TIMEOUT(time_left, &now, 0);
			time_left = timeout - time_left;
			if (micros > time_left)
				micros = time_left;
#endif
		}
	}

	/*
	 * This yields for a while for tas mutexes, and just gives up the
	 * processor for hybrid mutexes.
	 * By yielding here we can get the other thread to give up the
	 * mutex before calling the more expensive library mutex call.
	 * Tests have shown this to be a big win when there is contention.
	 */
	PERFMON4(env, mutex, suspend, mutex, TRUE, mutexp->alloc_id, mutexp);
	__os_yield(env, 0, micros);
	PERFMON4(env, mutex, resume, mutex, TRUE, mutexp->alloc_id, mutexp);

#if defined(HAVE_MUTEX_HYBRID)
	if (!MUTEXP_IS_BUSY(mutexp))
		goto loop;
	/* Wait until the mutex can be obtained exclusively or it times out. */
	if ((ret = __db_hybrid_mutex_suspend(env, mutex,
	    timeout == 0 ? NULL : &timeout_timespec, ip, TRUE)) != 0) {
		DB_DEBUG_MSG(env,
		    "mutex_lock %ld suspend returned %d", (u_long)mutex, ret);
		if (ip != NULL) {
			DB_ASSERT(env, ip->mtx_ctr > 0);
			ip->mtx_ctr--;
		}
		return (ret);
	}
#else
	if ((micros <<= 1) > max_micros)
		micros = max_micros;
#endif

#ifdef HAVE_FAILCHK_BROADCAST
	if (F_ISSET(mutexp, DB_MUTEX_OWNER_DEAD) &&
	    dbenv->mutex_failchk_timeout != 0)
		return (__mutex_died(env, mutex));
#endif
	/*
	 * We're spinning.  The environment might be hung, and somebody else
	 * has already recovered it.  The first thing recovery does is panic
	 * the environment.  Check to see if we're never going to get this
	 * mutex.
	 */
	PANIC_CHECK(env);

	goto loop;

}

#if defined(HAVE_SHARED_LATCHES)
/*
 * __db_tas_mutex_readlock --
 *	Acquire a shared readlock on o latch, possibly waiting if necessary.
 *
 *	
 *
 * PUBLIC: int __db_tas_mutex_readlock __P((ENV *, db_mutex_t, u_int32_t));
 */
int
__db_tas_mutex_readlock(env, mutex, flags)
	ENV *env;
	db_mutex_t mutex;
	u_int32_t flags;
{
	DB_ENV *dbenv;
	DB_MUTEX *mutexp;
	DB_MUTEXMGR *mtxmgr;
	DB_MUTEXREGION *mtxregion;
	DB_THREAD_INFO *ip;
	MUTEX_STATE *state;
	int lock, ret;
	u_int32_t nspins;
#ifndef HAVE_MUTEX_HYBRID
	u_long micros, max_micros;
#endif
	dbenv = env->dbenv;

	if (!MUTEX_ON(env) || F_ISSET(dbenv, DB_ENV_NOLOCKING))
		return (0);

	mtxmgr = env->mutex_handle;
	mtxregion = mtxmgr->reginfo.primary;
	mutexp = MUTEXP_SET(env, mutex);

	ip = NULL;
	state = NULL;
	if (env->thr_hashtab != NULL) {
		if ((ret = __env_set_state(env, &ip, THREAD_VERIFY)) != 0)
			return (__env_panic(env, ret));
		if ((ret = __mutex_record_lock(env,
		    mutex, ip, MUTEX_ACTION_INTEND_SHARE, &state)) != 0)
			return (ret);
	}

	DB_ASSERT(env, F_ISSET(mutexp, DB_MUTEX_SHARED));
	if (F_ISSET(mutexp, DB_MUTEX_LOCKED))
		STAT_INC(env,
		    mutex, set_rd_wait, mutexp->mutex_set_rd_wait, mutex);
	else
		STAT_INC(env,
		    mutex, set_rd_nowait, mutexp->mutex_set_rd_nowait, mutex);

#ifndef HAVE_MUTEX_HYBRID
	/*
	 * Wait 1ms initially, up to 10ms for mutexes backing logical database
	 * locks, and up to 25 ms for mutual exclusion data structure mutexes.
	 * SR: #7675
	 */
	micros = 1000;
	max_micros = F_ISSET(mutexp, DB_MUTEX_LOGICAL_LOCK) ? 10000 : 25000;
#endif

loop:	/* Attempt to acquire the resource for N spins. */
	for (nspins =
	    mtxregion->stat.st_mutex_tas_spins; nspins > 0; --nspins) {
		lock = atomic_read(&mutexp->sharecount);
		if (lock == MUTEX_SHARE_ISEXCLUSIVE ||
		    !atomic_compare_exchange(env,
			&mutexp->sharecount, lock, lock + 1)) {
			/*
			 * Some systems (notably those with newer Intel CPUs)
			 * need a small pause here. [#6975]
			 */
			MUTEX_PAUSE
			continue;
		}
#ifdef HAVE_FAILCHK_BROADCAST
		if (F_ISSET(mutexp, DB_MUTEX_OWNER_DEAD) &&
		    !F_ISSET(dbenv, DB_ENV_FAILCHK)) {
			(void)atomic_compare_exchange(env,
			    &mutexp->sharecount, lock, lock - 1);
			if (state != NULL)
				state->action = MUTEX_ACTION_UNLOCKED;
		       return (__mutex_died(env, mutex));
	       }
#endif

		MEMBAR_ENTER();
#ifdef MUTEX_DIAG
		__os_gettime(env, &mutexp->mutex_history.when, 0);
		__os_stack_text(env, mutexp->mutex_history.stacktext,
		    sizeof(mutexp->mutex_history.stacktext), 12, 3);
#endif
		/* For shared latches the threadid is the last requestor's id.
		 */
		dbenv->thread_id(dbenv, &mutexp->pid, &mutexp->tid);
		if (state != NULL)
			state->action = MUTEX_ACTION_SHARED;

		return (0);
	}

	/* Waiting for the latch must be avoided if it could hang up failchk. */
	if (F_ISSET(dbenv, DB_ENV_FAILCHK) &&
	    dbenv->is_alive(dbenv, mutexp->pid, mutexp->tid, 0) == 0 &&
	    ip->dbth_state == THREAD_FAILCHK) {
		if (state != NULL)
			state->action = MUTEX_ACTION_UNLOCKED;
		return (USR_ERR(env, DB_RUNRECOVERY));
	}
#ifdef HAVE_FAILCHK_BROADCAST
       if (F_ISSET(mutexp, DB_MUTEX_OWNER_DEAD)) {
	       if (state != NULL)
		       state->action = MUTEX_ACTION_UNLOCKED;
	       return (__mutex_died(env, mutex));
       }
#endif

	/*
	 * It is possible to spin out when the latch is just shared, due to
	 * many threads or interrupts interfering with the compare&exchange.
	 * Avoid spurious DB_LOCK_NOTGRANTED returns by retrying.
	 */
	if (!LF_ISSET(MUTEX_WAIT)) {
		if (atomic_read(&mutexp->sharecount) != MUTEX_SHARE_ISEXCLUSIVE)
			goto loop;
		if (state != NULL)
			state->action = MUTEX_ACTION_UNLOCKED;
		return (DB_LOCK_NOTGRANTED);
	}

	/* Wait for the lock to become available. */
#ifdef HAVE_MUTEX_HYBRID
	/*
	 * By yielding here we can get the other thread to give up the
	 * mutex before calling the more expensive library mutex call.
	 * Tests have shown this to be a big win when there is contention.
	 */
	PERFMON4(env, mutex, suspend, mutex, FALSE, mutexp->alloc_id, mutexp);
	__os_yield(env, 0, 0);
	PERFMON4(env, mutex, resume, mutex, FALSE, mutexp->alloc_id, mutexp);
	if (atomic_read(&mutexp->sharecount) != MUTEX_SHARE_ISEXCLUSIVE)
		goto loop;
	/* Wait until the mutex is no longer exclusively locked. */
	if ((ret = __db_hybrid_mutex_suspend(env, mutex, NULL, ip, FALSE)) != 0) {
		if (state != NULL)
			state->action = MUTEX_ACTION_UNLOCKED;
		return (ret);
	}
#else
	PERFMON4(env, mutex, suspend, mutex, FALSE, mutexp->alloc_id, mutexp);
	__os_yield(env, 0, micros);
	PERFMON4(env, mutex, resume, mutex, FALSE, mutexp->alloc_id, mutexp);
	if ((micros <<= 1) > max_micros)
		micros = max_micros;
#endif

	/*
	 * We're spinning.  The environment might be hung, and somebody else
	 * has already recovered it.  The first thing recovery does is panic
	 * the environment.  Check to see if we're never going to get this
	 * mutex.
	 */
	PANIC_CHECK(env);

	goto loop;
}
#endif

/*
 * __db_tas_mutex_unlock --
 *	Release a test-and-set mutex/latch.
 *	The DB_THREAD_INFO allows one to unlock on behalf of another thread.  
 *
 * PUBLIC: int __db_tas_mutex_unlock
 * PUBLIC:     __P((ENV *, db_mutex_t, DB_THREAD_INFO *, u_int32_t));
 *
 * Hybrid shared latch wakeup
 *	When an exclusive requester waits for the last shared holder to
 *	release, it increments mutexp->wait and pthread_cond_wait()'s. The
 *	last shared unlock calls __db_pthread_mutex_unlock() to wake it.
 */
int
__db_tas_mutex_unlock(env, mutex, ip, flags)
	ENV *env;
	db_mutex_t mutex;
	DB_THREAD_INFO *ip;
	u_int32_t flags;
{
	DB_ENV *dbenv;
	DB_MUTEX *mutexp;
	int ret, was_exclusive;
	char description[DB_MUTEX_DESCRIBE_STRLEN];
#ifdef HAVE_SHARED_LATCHES
	int sharecount;
#endif
	dbenv = env->dbenv;

	if (!MUTEX_ON(env) || F_ISSET(dbenv, DB_ENV_NOLOCKING))
		return (0);

	if (env->thr_hashtab != NULL && ip == NULL &&
	    (ret = __env_set_state(env, &ip, THREAD_CTR_VERIFY)) != 0)
		return (__env_panic(env, ret));

	mutexp = MUTEXP_SET(env, mutex);
	was_exclusive = F_ISSET(mutexp, DB_MUTEX_LOCKED);

	/*
	 * Bump the mutex counter when releasing a shared latch.  This lets us
	 * detect a crash crashes during this unlock call, and treats it as a
	 * hard (DB_RUNRECOVERY) failchk case.
	 */
	if (!was_exclusive && ip != NULL)
		ip->mtx_ctr++;

#if defined(DIAGNOSTIC)
#if defined(HAVE_SHARED_LATCHES)
	if (F_ISSET(mutexp, DB_MUTEX_SHARED)) {
		if (atomic_read(&mutexp->sharecount) == 0) {
			ret = USR_ERR(env, DB_RUNRECOVERY);
			if (PANIC_ISSET(env))
				return (__env_panic(env, ret));
			__db_errx(env, DB_STR_A("2031",
			    "shared unlock %ld already unlocked", "%ld"),
			    (long)mutex);
			return (__env_panic(env, EACCES));
		}
	} else
#endif
	if (!F_ISSET(mutexp, DB_MUTEX_LOCKED)) {
		if (PANIC_ISSET(env))
			return (__env_panic(env,
			    USR_ERR(env, DB_RUNRECOVERY)));
		__db_errx(env, DB_STR_A("2032",
		    "unlock %ld already unlocked", "%ld"), (long)mutex);
		return (__env_panic(env, EACCES));
	}
#endif
#ifdef MUTEX_DIAG
	timespecclear(&mutexp->mutex_history.when);
#endif

#ifdef HAVE_SHARED_LATCHES
	if (F_ISSET(mutexp, DB_MUTEX_SHARED)) {
		sharecount = atomic_read(&mutexp->sharecount);
		/*
		 * Many code paths contain sequence of the form
		 *	MUTEX_LOCK(); ret = function(); MUTEX_UNLOCK();
		 * If function() sees or causes a panic while it had temporarily
		 * unlocked the mutex it won't be locked anymore. Don't confuse
		 * the error by generating spurious follow-on messages.
		 */
		if (sharecount == 0) {
was_not_locked:
			if (!PANIC_ISSET(env)) {
				ret = USR_ERR(env, DB_RUNRECOVERY);
				__db_errx(env, DB_STR_A("2070",
				    "Shared unlock %s: already unlocked", "%s"),
				    __mutex_describe(env, mutex, description));
				return (__env_panic(env, ret));
			}
			return (__env_panic(env, EACCES));
		}
		if (sharecount == MUTEX_SHARE_ISEXCLUSIVE) {
			F_CLR(mutexp, DB_MUTEX_LOCKED);
			/* Flush flag update before zeroing count */
			MEMBAR_EXIT();
			atomic_init(&mutexp->sharecount, 0);
		} else {
			DB_ASSERT(env, sharecount > 0);
			MEMBAR_EXIT();
			sharecount = atomic_dec(env, &mutexp->sharecount);
			DB_ASSERT(env, sharecount >= 0);
			if (sharecount > 0)
				goto finish;
		}
	} else
#endif
	{
		if (!F_ISSET(mutexp, DB_MUTEX_LOCKED))
			goto was_not_locked;
		F_CLR(mutexp, DB_MUTEX_LOCKED);
		MEMBAR_EXIT();
		MUTEX_UNSET(&mutexp->tas);
	}

#ifdef HAVE_MUTEX_HYBRID
#ifdef DIAGNOSTIC
	if (F_ISSET(dbenv, DB_ENV_YIELDCPU))
		__os_yield(env, 0, 0);
#endif

	/* Prevent the load of wait from being hoisted before MUTEX_UNSET. */
	(void)MUTEX_MEMBAR(mutexp->flags);
	if (mutexp->wait &&
	    (ret = __db_pthread_mutex_unlock(env, mutex, ip, 0)) != 0)
		    return (ret);
#endif
	if (was_exclusive) {
		if (ip != NULL && LF_ISSET(MUTEX_CTR)) {
			DB_ASSERT(env, ip->mtx_ctr > 0);
			ip->mtx_ctr--;
		}
	} else {
finish:
		if (ip != NULL) {
			if ((ret = __mutex_record_unlock(env, mutex, ip)) != 0)
				return (ret);
			/*
			 * Now that the shared unlock is complete, reduce the
			 * counter that failchk uses to detect 'fatal' cases.
			 */
			ip->mtx_ctr--;
		}
	}

	return (0);
}

/*
 * __db_tas_mutex_destroy --
 *	Destroy a mutex.
 *
 * PUBLIC: int __db_tas_mutex_destroy __P((ENV *, db_mutex_t));
 */
int
__db_tas_mutex_destroy(env, mutex)
	ENV *env;
	db_mutex_t mutex;
{
	DB_MUTEX *mutexp;
#ifdef HAVE_MUTEX_HYBRID
	int ret;
#endif

	if (!MUTEX_ON(env))
		return (0);

	mutexp = MUTEXP_SET(env, mutex);

	MUTEX_DESTROY(&mutexp->tas);

#ifdef HAVE_MUTEX_HYBRID
	if ((ret = __db_pthread_mutex_destroy(env, mutex)) != 0)
		return (ret);
#endif

	COMPQUIET(mutexp, NULL);	/* MUTEX_DESTROY may not be defined. */
	return (0);
}
