/* src/threads/lock.cpp - lock implementation

   Copyright (C) 1996-2013
   CACAOVM - Verein zur Foerderung der freien virtuellen Maschine CACAO

   This file is part of CACAO.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.

*/

#include "threads/lock.hpp"
#include <assert.h>                     // for assert
#include <stdint.h>                     // for uintptr_t, int32_t
#include <stdio.h>                      // for NULL
#include <list>                         // for _List_iterator, etc
#include "arch.hpp"                     // for CAS_PROVIDES_FULL_BARRIER
#include "config.h"                     // for ENABLE_GC_BOEHM
#include "lockword.hpp"                 // for Lockword
#include "mm/gc.hpp"                    // for heap_hashcode, etc
#include "mm/memory.hpp"                // for MNEW, MZERO, FREE, MFREE, etc
#include "native/llni.hpp"              // for LLNI_DIRECT, LLNI_class_get
#include "threads/condition.hpp"        // for Condition
#include "threads/mutex.hpp"            // for Mutex
#include "threads/thread.hpp"           // for threadobject, etc
#include "threads/atomic.hpp"           // for memory_barrier, etc
#include "threads/threadlist.hpp"       // for ThreadList
#include "toolbox/OStream.hpp"          // for OStream, nl
#include "toolbox/list.hpp"             // for List
#include "toolbox/logging.hpp"          // for log_println, LOG
#include "vm/class.hpp"                 // for operator<<, etc
#include "vm/exceptions.hpp"
#include "vm/finalizer.hpp"             // for Finalizer
#include "vm/global.hpp"                // for java_handle_t
#include "vm/options.hpp"               // for opt_DebugLocks
#include "vm/types.hpp"                 // for u4, s4, s8

#if defined(ENABLE_JVMTI)
#include "native/jvmti/cacaodbg.h"
#endif

#if defined(ENABLE_GC_BOEHM)
# include "mm/boehm-gc/include/gc.h"
#endif

/* debug **********************************************************************/

#if !defined(NDEBUG)
# define DEBUGLOCKS(format) \
    do { \
        if (opt_DebugLocks) { \
            log_println format; \
        } \
    } while (0)
#else
# define DEBUGLOCKS(format)
#endif

STAT_DECLARE_VAR(int,size_lock_record,0)
STAT_DECLARE_VAR(int,size_lock_hashtable,0)
STAT_DECLARE_VAR(int,size_lock_waiter,0)

/******************************************************************************/
/* MACROS                                                                     */
/******************************************************************************/

/* number of lock records in the first pool allocated for a thread */
#define LOCK_INITIAL_LOCK_RECORDS 8

#define LOCK_INITIAL_HASHTABLE_SIZE  1613  /* a prime in the middle between 1024 and 2048 */


/******************************************************************************/
/* MACROS FOR THIN/FAT LOCKS                                                  */
/******************************************************************************/

/* We use a variant of the tasuki locks described in the paper
 *
 *     Tamiya Onodera, Kiyokuni Kawachiya
 *     A Study of Locking Objects with Bimodal Fields
 *     Proceedings of the ACM OOPSLA '99, pp. 223-237
 *     1999
 *
 * The underlying thin locks are a variant of the thin locks described in
 * 
 *     Bacon, Konuru, Murthy, Serrano
 *     Thin Locks: Featherweight Synchronization for Java
 *	   Proceedings of the ACM Conference on Programming Language Design and 
 *	   Implementation (Montreal, Canada), SIGPLAN Notices volume 33, number 6,
 *	   June 1998
 *
 * In thin lock mode the lockword looks like this:
 *
 *     ,----------------------,-----------,---,
 *     |      thread ID       |   count   | 0 |
 *     `----------------------'-----------'---'
 *
 *     thread ID......the 'index' of the owning thread, or 0
 *     count..........number of times the lock has been entered	minus 1
 *     0..............the shape bit is 0 in thin lock mode
 *
 * In fat lock mode it is basically a lock_record_t *:
 *
 *     ,----------------------------------,---,
 *     |    lock_record_t * (without LSB) | 1 |
 *     `----------------------------------'---'
 *
 *     1..............the shape bit is 1 in fat lock mode
 */

/* global variables ***********************************************************/

/* hashtable mapping objects to lock records */
static lock_hashtable_t lock_hashtable;


/******************************************************************************/
/* PROTOTYPES                                                                 */
/******************************************************************************/

static void lock_hashtable_init(void);

static inline uintptr_t* lock_lockword_get(java_handle_t* o);
static void lock_record_enter(threadobject *t, lock_record_t *lr);
static void lock_record_exit(threadobject *t, lock_record_t *lr);
static bool lock_record_wait(threadobject *t, lock_record_t *lr, s8 millis, s4 nanos);
static void lock_record_notify(threadobject *t, lock_record_t *lr, bool one);


/*============================================================================*/
/* INITIALIZATION OF DATA STRUCTURES                                          */
/*============================================================================*/


/* lock_init *******************************************************************

   Initialize global data for locking.

*******************************************************************************/

void lock_init(void)
{
	/* initialize lock hashtable */

	lock_hashtable_init();
}


/* lock_record_new *************************************************************

   Allocate a lock record.

*******************************************************************************/

static lock_record_t *lock_record_new(void)
{
	lock_record_t *lr;

	/* allocate the data structure on the C heap */

	lr = NEW(lock_record_t);

	STATISTICS(size_lock_record += sizeof(lock_record_t));

	/* initialize the members */

	lr->object  = NULL;
	lr->owner   = NULL;
	lr->count   = 0;
	lr->waiters = new List<threadobject*>();

#if defined(ENABLE_GC_CACAO)
	/* register the lock object as weak reference with the GC */

	gc_weakreference_register(&(lr->object), GC_REFTYPE_LOCKRECORD);
#endif

	// Initialize the mutex.
	lr->mutex = new Mutex();

	DEBUGLOCKS(("[lock_record_new   : lr=%p]", (void *) lr));

	return lr;
}


/* lock_record_free ************************************************************

   Free a lock record.

   IN:
       lr....lock record to free

*******************************************************************************/

static void lock_record_free(lock_record_t *lr)
{
	DEBUGLOCKS(("[lock_record_free  : lr=%p]", (void *) lr));

	// Destroy the mutex.
	delete lr->mutex;

#if defined(ENABLE_GC_CACAO)
	/* unregister the lock object reference with the GC */

	gc_weakreference_unregister(&(lr->object));
#endif

	// Free the waiters list.
	delete lr->waiters;

	/* Free the data structure. */

	FREE(lr, lock_record_t);

	STATISTICS(size_lock_record -= sizeof(lock_record_t));
}


/*============================================================================*/
/* HASHTABLE MAPPING OBJECTS TO LOCK RECORDS                                  */
/*============================================================================*/

/* lock_hashtable_init *********************************************************

   Initialize the global hashtable mapping objects to lock records.

*******************************************************************************/

static void lock_hashtable_init(void)
{
	lock_hashtable.mutex   = new Mutex();

	lock_hashtable.size    = LOCK_INITIAL_HASHTABLE_SIZE;
	lock_hashtable.entries = 0;
	lock_hashtable.ptr     = MNEW(lock_record_t *, lock_hashtable.size);

	STATISTICS(size_lock_hashtable += sizeof(lock_record_t *) * lock_hashtable.size);

	MZERO(lock_hashtable.ptr, lock_record_t *, lock_hashtable.size);
}


/* lock_hashtable_grow *********************************************************

   Grow the lock record hashtable to about twice its current size and
   rehash the entries.

*******************************************************************************/

/* must be called with hashtable mutex locked */
static void lock_hashtable_grow(void)
{
	u4 oldsize;
	u4 newsize;
	lock_record_t **oldtable;
	lock_record_t **newtable;
	lock_record_t *lr;
	lock_record_t *next;
	u4 i;
	u4 h;
	u4 newslot;

	/* allocate a new table */

	oldsize = lock_hashtable.size;
	newsize = oldsize*2 + 1; /* XXX should use prime numbers */

	DEBUGLOCKS(("growing lock hashtable to size %d", newsize));

	oldtable = lock_hashtable.ptr;
	newtable = MNEW(lock_record_t *, newsize);

	STATISTICS(size_lock_hashtable += sizeof(lock_record_t *) * newsize);

	MZERO(newtable, lock_record_t *, newsize);

	/* rehash the entries */

	for (i = 0; i < oldsize; i++) {
		lr = oldtable[i];
		while (lr) {
			next = lr->hashlink;

			h = heap_hashcode(lr->object);
			newslot = h % newsize;

			lr->hashlink = newtable[newslot];
			newtable[newslot] = lr;

			lr = next;
		}
	}

	/* replace the old table */

	lock_hashtable.ptr  = newtable;
	lock_hashtable.size = newsize;

	MFREE(oldtable, lock_record_t *, oldsize);

	STATISTICS(size_lock_hashtable -= sizeof(lock_record_t *) * oldsize);
}


/* lock_hashtable_cleanup ******************************************************

   Removes (and frees) lock records which have a cleared object reference
   from the hashtable. The locked object was reclaimed by the GC.

*******************************************************************************/

#if defined(ENABLE_GC_CACAO)
void lock_hashtable_cleanup(void)
{
	threadobject  *t;
	lock_record_t *lr;
	lock_record_t *prev;
	lock_record_t *next;
	int i;

	t = THREADOBJECT;

	/* lock the hashtable */

	Mutex_lock(lock_hashtable.mutex);

	/* search the hashtable for cleared references */

	for (i = 0; i < lock_hashtable.size; i++) {
		lr = lock_hashtable.ptr[i];
		prev = NULL;

		while (lr) {
			next = lr->hashlink;

			/* remove lock records with cleared references */

			if (lr->object == NULL) {

				/* unlink the lock record from the hashtable */

				if (prev == NULL)
					lock_hashtable.ptr[i] = next;
				else
					prev->hashlink = next;

				/* free the lock record */

				lock_record_free(lr);

			} else {
				prev = lr;
			}

			lr = next;
		}
	}

	/* unlock the hashtable */

	Mutex_unlock(lock_hashtable.mutex);
}
#endif


/* lock_hashtable_get **********************************************************

   Find the lock record for the given object.  If it does not exists,
   yet, create it and enter it in the hashtable.

   IN:
	  o....the object to look up

   RETURN VALUE:
      the lock record to use for this object

*******************************************************************************/

#if defined(ENABLE_GC_BOEHM)
static void lock_record_finalizer(java_handle_t *object, void *p);
#endif

static lock_record_t *lock_hashtable_get(java_handle_t* o)
{
	// This function is inside a critical section.
	GCCriticalSection cs;

	u4             slot;
	lock_record_t *lr;

	// lw_cache is used throughout this file because the lockword can change at
	// any time, unless it is absolutely certain that we are holding the lock.
	// We don't do deflation, so we would also not expect a fat lockword to
	// change, but for the sake of uniformity, lw_cache is used even in this
	// case.
	uintptr_t lw_cache = *lock_lockword_get(o);
	Lockword lockword(lw_cache);

	if (lockword.is_fat_lock())
		return lockword.get_fat_lock();

	// Lock the hashtable.
	lock_hashtable.mutex->lock();

	/* lookup the lock record in the hashtable */

	slot = heap_hashcode(LLNI_DIRECT(o)) % lock_hashtable.size;
	lr   = lock_hashtable.ptr[slot];

	for (; lr != NULL; lr = lr->hashlink) {
		if (lr->object == LLNI_DIRECT(o))
			break;
	}

	if (lr == NULL) {
		/* not found, we must create a new one */

		lr = lock_record_new();

		lr->object = LLNI_DIRECT(o);

#if defined(ENABLE_GC_BOEHM)
		/* register new finalizer to clean up the lock record */

		Finalizer::attach_custom_finalizer(o, lock_record_finalizer, 0);
#endif

		/* enter it in the hashtable */

		lr->hashlink             = lock_hashtable.ptr[slot];
		lock_hashtable.ptr[slot] = lr;
		lock_hashtable.entries++;

		/* check whether the hash should grow */

		if (lock_hashtable.entries * 3 > lock_hashtable.size * 4) {
			lock_hashtable_grow();
		}
	}

	// Unlock the hashtable.
	lock_hashtable.mutex->unlock();

	/* return the new lock record */

	return lr;
}

/* lock_hashtable_remove *******************************************************

   Remove the lock record for the given object from the hashtable
   and free it afterwards.

   IN:
       t....the current thread
       o....the object to look up

*******************************************************************************/

static void lock_hashtable_remove(threadobject *t, java_handle_t *o)
{
	lock_record_t *lr;
	u4             slot;
	lock_record_t *tmplr;

	// Lock the hashtable.
	lock_hashtable.mutex->lock();

	/* get lock record */

	uintptr_t lw_cache = *lock_lockword_get(o);
	Lockword lockword(lw_cache);

	// Sanity check.
	assert(lockword.is_fat_lock());

	lr = lockword.get_fat_lock();

	/* remove the lock-record from the hashtable */

	slot  = heap_hashcode(LLNI_DIRECT(o)) % lock_hashtable.size;
	tmplr = lock_hashtable.ptr[slot];

	if (tmplr == lr) {
		/* special handling if it's the first in the chain */

		lock_hashtable.ptr[slot] = lr->hashlink;
	}
	else {
		for (; tmplr != NULL; tmplr = tmplr->hashlink) {
			if (tmplr->hashlink == lr) {
				tmplr->hashlink = lr->hashlink;
				break;
			}
		}

		assert(tmplr != NULL);
	}

	/* decrease entry count */

	lock_hashtable.entries--;

	// Unlock the hashtable.
	lock_hashtable.mutex->unlock();

	/* free the lock record */

	lock_record_free(lr);
}


/* lock_record_finalizer *******************************************************

   XXX Remove me for exact GC.

*******************************************************************************/

#define DEBUG_NAME "finalizer"

static void lock_record_finalizer(java_handle_t *o, void *p)
{
#if defined(ENABLE_LOGGING)
	classinfo     *c;

	LLNI_class_get(o, c);

	LOG("[finalizer lockrecord:"
	    << " o=" << o
	    << " p=" << p
	    << " class=" << c
		<< "]" << cacao::nl);
#endif

	/* remove the lock-record entry from the hashtable and free it */

	lock_hashtable_remove(THREADOBJECT, o);
}

#undef DEBUG_NAME

/*============================================================================*/
/* LOCKING ALGORITHM                                                          */
/*============================================================================*/


/* lock_lockword_get ***********************************************************

   Get the lockword for the given object.

   IN:
      o............the object

*******************************************************************************/

static inline uintptr_t* lock_lockword_get(java_handle_t* o)
{
#if defined(ENABLE_GC_CACAO)
	// Sanity check.
	assert(GCCriticalSection::inside() == true);
#endif

	return &(LLNI_DIRECT(o)->lockword);
}


/* lock_record_enter ***********************************************************

   Enter the lock represented by the given lock record.

   IN:
      t.................the current thread
	  lr................the lock record

*******************************************************************************/

static inline void lock_record_enter(threadobject *t, lock_record_t *lr)
{
	lr->mutex->lock();
	lr->owner = t;
}


/* lock_record_exit ************************************************************

   Release the lock represented by the given lock record.

   IN:
      t.................the current thread
	  lr................the lock record

   PRE-CONDITION:
      The current thread must own the lock represented by this lock record.
	  This is NOT checked by this function!

*******************************************************************************/

static inline void lock_record_exit(threadobject *t, lock_record_t *lr)
{
	lr->owner = NULL;
	lr->mutex->unlock();
}


/* lock_inflate ****************************************************************

   Inflate the lock of the given object. This may only be called by the
   owner of the monitor of the object.

   IN:
	  o............the object of which to inflate the lock
	  lr...........the lock record to install. The current thread must
	               own the lock of this lock record!

   PRE-CONDITION:
      The current thread must be the owner of this object's monitor AND
	  of the lock record's lock!

*******************************************************************************/

static void lock_inflate(java_handle_t *o, lock_record_t *lr)
{
	Lockword lockword(*lock_lockword_get(o));
	lockword.inflate(lr);
}


/* sable_flc_waiting ***********************************************************

   Enqueue the current thread on another thread's FLC list. The function
   blocks until the lock has been inflated by the owning thread.

   The algorithm used to be an almost literal copy from SableVM. The
   superfluous list traversal in the waiting loop has been removed since,
   though.

   IN:
	  lockword.....the object's lockword as seen at the first locking attempt
	  t............the current thread
	  o............the object of which to enter the monitor

*******************************************************************************/

static void sable_flc_waiting(uintptr_t lw_cache, threadobject *t, java_handle_t *o)
{
	int32_t index;
	threadobject *t_other;
	int old_flc;

	Lockword lockword(lw_cache);
	index = lockword.get_thin_lock_thread_index();
	t_other = ThreadList::get()->get_thread_by_index(index);

	// The lockword could have changed during our way here.  If the
	// thread index is zero, the lock got unlocked and we simply
	// return.
	if (t_other == NULL)
/* 		failure, TODO: add statistics */
		return;

	t_other->flc_lock->lock();
	old_flc = t_other->flc_bit;
	t_other->flc_bit = true;

	DEBUGLOCKS(("thread %d set flc bit for lock-holding thread %d", t->index, t_other->index));

	// Set FLC bit first, then read the lockword again.
	Atomic::memory_barrier();

	lw_cache = *lock_lockword_get(o);

	/* Lockword is still the way it was seen before */
	if (lockword.is_thin_lock() && (lockword.get_thin_lock_thread_index() == index))
	{
		//threadobject *f;
		/* Add tuple (t, o) to the other thread's FLC list */
		t->flc_object = o;
		t->flc_next = t_other->flc_list;
		t_other->flc_list = t;
		if (t->flc_next == 0)
			t_other->flc_tail = t;
		//f = t_other->flc_tail;

		// The other thread will clear flc_object.
		while (t->flc_object)
		{
			// We are not cleared yet -- the other thread cannot have seen
			// the FLC bit yet.
			assert(t_other->flc_bit);

			// Wait until another thread sees the flc bit and notifies
			// us of unlocking.
			t->flc_cond->wait(t_other->flc_lock);
		}

		t->flc_next = NULL;
	}
	else
		t_other->flc_bit = old_flc;

	t_other->flc_lock->unlock();
}

/* notify_flc_waiters **********************************************************

   Traverse the thread's FLC list and inflate all corresponding locks. Notify
   the associated threads as well.

   IN:
	  t............the current thread
	  o............the object currently being unlocked

*******************************************************************************/

static void notify_flc_waiters(threadobject *t, java_handle_t *o)
{
	threadobject *current;

	t->flc_lock->lock();

	current = t->flc_list;
	while (current)
	{
		if (current->flc_object != o)
		{
			/* The object has to be inflated so the other threads can properly
			   block on it. */

			// Only if not already inflated.
			Lockword lockword(*lock_lockword_get(current->flc_object));
			if (lockword.is_thin_lock()) {
				lock_record_t *lr = lock_hashtable_get(current->flc_object);
				lock_record_enter(t, lr);

				DEBUGLOCKS(("thread %d inflating lock of %p to lr %p",
							t->index, (void*) current->flc_object, (void*) lr));

				lock_inflate(current->flc_object, lr);
			}
		}

		// Wake the waiting threads.
		current->flc_cond->broadcast();
		current->flc_object = NULL;

		current = current->flc_next;
	}

	t->flc_list = NULL;
	t->flc_bit = false;

	t->flc_lock->unlock();
}

/* lock_monitor_enter **********************************************************

   Acquire the monitor of the given object. If the current thread already
   owns the monitor, the lock counter is simply increased.

   This function blocks until it can acquire the monitor.

   IN:
      t............the current thread
	  o............the object of which to enter the monitor

   RETURN VALUE:
      true.........the lock has been successfully acquired
	  false........an exception has been thrown

*******************************************************************************/

bool lock_monitor_enter(java_handle_t *o)
{
	// This function is inside a critical section.
	GCCriticalSection cs;

	if (o == NULL) {
		exceptions_throw_nullpointerexception();
		return false;
	}

	threadobject* t = thread_get_current();

	uintptr_t thinlock = t->thinlock;

retry:
	// Most common case: try to thin-lock an unlocked object.
	uintptr_t *lw_ptr = lock_lockword_get(o);
	uintptr_t lw_cache = *lw_ptr;
	Lockword lockword(lw_cache);
	bool result = Lockword(*lw_ptr).lock(thinlock);

	if (result == true) {
		// Success, we locked it.
		// NOTE: The Java Memory Model requires a memory barrier here.
#if defined(CAS_PROVIDES_FULL_BARRIER) && CAS_PROVIDES_FULL_BARRIER
		// On some architectures, the CAS (hidden in the
		// lockword.lock call above), already provides this barrier,
		// so we only need to inform the compiler.
		Atomic::instruction_barrier();
#else
		Atomic::memory_barrier();
#endif
		return true;
	}

	// Next common case: recursive lock with small recursion count.
	// NOTE: We don't have to worry about stale values here, as any
	// stale value will indicate another thread holding the lock (or
	// an inflated lock).
	if (lockword.get_thin_lock_without_count() == thinlock) {
		// We own this monitor.  Check the current recursion count.
		if (lockword.is_max_thin_lock_count() == false) {
			// The recursion count is low enough.
			Lockword(*lw_ptr).increase_thin_lock_count();

			// Success, we locked it.
			return true;
		}
		else {
			// Recursion count overflow.
			lock_record_t* lr = lock_hashtable_get(o);
			lock_record_enter(t, lr);
			lock_inflate(o, lr);
			lr->count++;

			notify_flc_waiters(t, o);

			return true;
		}
	}

	// The lock is either contented or fat.
	if (lockword.is_fat_lock()) {
		lock_record_t* lr = lockword.get_fat_lock();

		// Check for recursive entering.
		if (lr->owner == t) {
			lr->count++;
			return true;
		}

		// Acquire the mutex of the lock record.
		lock_record_enter(t, lr);

		// Sanity check.
		assert(lr->count == 0);
		return true;
	}

	/****** inflation path ******/

#if defined(ENABLE_JVMTI)
	/* Monitor Contended Enter */
	jvmti_MonitorContendedEntering(false, o);
#endif

	sable_flc_waiting(lw_cache, t, o);

#if defined(ENABLE_JVMTI)
	/* Monitor Contended Entered */
	jvmti_MonitorContendedEntering(true, o);
#endif
	goto retry;
}


/* lock_monitor_exit ***********************************************************

   Decrement the counter of a (currently owned) monitor. If the counter
   reaches zero, release the monitor.

   If the current thread is not the owner of the monitor, an 
   IllegalMonitorState exception is thrown.

   IN:
      t............the current thread
	  o............the object of which to exit the monitor

   RETURN VALUE:
      true.........everything ok,
	  false........an exception has been thrown

*******************************************************************************/

bool lock_monitor_exit(java_handle_t* o)
{
	// This function is inside a critical section.
	GCCriticalSection cs;

	if (o == NULL) {
		exceptions_throw_nullpointerexception();
		return false;
	}

	threadobject* t = thread_get_current();

	uintptr_t thinlock = t->thinlock;

	// We don't have to worry about stale values here, as any stale
	// value will indicate that we don't own the lock.
	uintptr_t *lw_ptr = lock_lockword_get(o);
	uintptr_t lw_cache = *lw_ptr;
	Lockword lockword(lw_cache);

	// Most common case: we release a thin lock that we hold once.
	if (lockword.get_thin_lock() == thinlock) {
		// Memory barrier for Java Memory Model.
		Atomic::write_memory_barrier();
		Lockword(*lw_ptr).unlock();
		// Memory barrier for FLC bit testing.
		Atomic::memory_barrier();

		/* check if there has been a flat lock contention on this object */

		if (t->flc_bit) {
			DEBUGLOCKS(("thread %d saw flc bit", t->index));

			/* there has been a contention on this thin lock */
			notify_flc_waiters(t, o);
		}

		return true;
	}

	// Next common case: we release a recursive lock, count > 0.
	if (lockword.get_thin_lock_without_count() == thinlock) {
		Lockword(*lw_ptr).decrease_thin_lock_count();
		return true;
	}

	// Either the lock is fat, or we don't hold it at all.
	if (lockword.is_fat_lock()) {
		lock_record_t* lr = lockword.get_fat_lock();

		// Check if we own this monitor.
		// NOTE: We don't have to worry about stale values here, as
		// any stale value will be != t and thus fail this check.
		if (lr->owner != t) {
			exceptions_throw_illegalmonitorstateexception();
			return false;
		}

		/* { the current thread `t` owns the lock record `lr` on object `o` } */

		if (lr->count != 0) {
			// We had locked this one recursively.  Just decrement, it
			// will still be locked.
			lr->count--;
			return true;
		}

		// Unlock this lock record.
		lock_record_exit(t, lr);
		return true;
	}

	// Legal thin lock cases have been handled above, so this is an
	// error.
	exceptions_throw_illegalmonitorstateexception();

	return false;
}


/* lock_record_add_waiter ******************************************************

   Add a thread to the list of waiting threads of a lock record.

   IN:
      lr...........the lock record
      thread.......the thread to add

*******************************************************************************/

static void lock_record_add_waiter(lock_record_t *lr, threadobject* t)
{
	// Add the thread as last entry to waiters list.
	lr->waiters->push_back(t);

	STATISTICS(size_lock_waiter += sizeof(threadobject*));
}


/* lock_record_remove_waiter ***************************************************

   Remove a thread from the list of waiting threads of a lock record.

   IN:
      lr...........the lock record
      t............the current thread

   PRE-CONDITION:
      The current thread must be the owner of the lock record.

*******************************************************************************/

static void lock_record_remove_waiter(lock_record_t *lr, threadobject* t)
{
	// Remove the thread from the waiters.
	lr->waiters->remove(t);

	STATISTICS(size_lock_waiter -= sizeof(threadobject*));
}


/* lock_record_wait ************************************************************

   Wait on a lock record for a given (maximum) amount of time.

   IN:
      t............the current thread
	  lr...........the lock record
	  millis.......milliseconds of timeout
	  nanos........nanoseconds of timeout

   RETURN VALUE:
      true.........we have been interrupted,
      false........everything ok

   PRE-CONDITION:
      The current thread must be the owner of the lock record.
	  This is NOT checked by this function!
   
*******************************************************************************/

static bool lock_record_wait(threadobject *thread, lock_record_t *lr, s8 millis, s4 nanos)
{
	s4   lockcount;
	bool wasinterrupted = false;

	DEBUGLOCKS(("[lock_record_wait  : lr=%p, t=%p, millis=%lld, nanos=%d]",
				lr, thread, millis, nanos));

	/* { the thread t owns the fat lock record lr on the object o } */

	/* register us as waiter for this object */

	lock_record_add_waiter(lr, thread);

	/* remember the old lock count */

	lockcount = lr->count;

	/* unlock this record */

	lr->count = 0;
	lock_record_exit(thread, lr);

	/* wait until notified/interrupted/timed out */

	threads_wait_with_timeout_relative(thread, millis, nanos);

	/* re-enter the monitor */

	lock_record_enter(thread, lr);

	/* remove us from the list of waiting threads */

	lock_record_remove_waiter(lr, thread);

	/* restore the old lock count */

	lr->count = lockcount;

	/* We can only be signaled OR interrupted, not both. If both flags
	   are set, reset only signaled and leave the thread in
	   interrupted state. Otherwise, clear both. */

	if (!thread->signaled) {
		wasinterrupted = thread->interrupted;
		thread->interrupted = false;
	}

	thread->signaled = false;

	/* return if we have been interrupted */

	return wasinterrupted;
}


/* lock_monitor_wait ***********************************************************

   Wait on an object for a given (maximum) amount of time.

   IN:
      t............the current thread
	  o............the object
	  millis.......milliseconds of timeout
	  nanos........nanoseconds of timeout

   PRE-CONDITION:
      The current thread must be the owner of the object's monitor.
   
*******************************************************************************/

static void lock_monitor_wait(threadobject *t, java_handle_t *o, s8 millis, s4 nanos)
{
	lock_record_t *lr;

	uintptr_t *lw_ptr = lock_lockword_get(o);
	uintptr_t lw_cache = *lw_ptr;
	Lockword lockword(lw_cache);

	// Check if we own this monitor.
	// NOTE: We don't have to worry about stale values here, as any
	// stale value will fail this check.
	if (lockword.is_fat_lock()) {
		lr = lockword.get_fat_lock();

		if (lr->owner != t) {
			exceptions_throw_illegalmonitorstateexception();
			return;
		}
	}
	else {
		// It's a thin lock.
		if (lockword.get_thin_lock_without_count() != t->thinlock) {
			exceptions_throw_illegalmonitorstateexception();
			return;
		}

		// Get the lock-record.
		lr = lock_hashtable_get(o);
		lock_record_enter(t, lr);

		// Inflate this lock.
		Lockword(*lw_ptr).inflate(lr);

		notify_flc_waiters(t, o);
	}

	/* { the thread t owns the fat lock record lr on the object o } */

	if (lock_record_wait(t, lr, millis, nanos))
		exceptions_throw_interruptedexception();
}


/* lock_record_notify **********************************************************

   Notify one thread or all threads waiting on the given lock record.

   IN:
      t............the current thread
	  lr...........the lock record
	  one..........if true, only notify one thread

   PRE-CONDITION:
      The current thread must be the owner of the lock record.
	  This is NOT checked by this function!
   
*******************************************************************************/

static void lock_record_notify(threadobject *t, lock_record_t *lr, bool one)
{
#if defined(ENABLE_GC_CACAO)
	// Sanity check.
	assert(GCCriticalSection::inside() == false);
#endif

	// { The thread t owns the fat lock record lr on the object o }

	for (List<threadobject*>::iterator it = lr->waiters->begin(); it != lr->waiters->end(); it++) {
		threadobject* waiter = *it;

		// We must skip threads which have already been notified. They
		// will remove themselves from the list.
		if (waiter->signaled)
			continue;

		// Enter the wait-mutex.
		waiter->waitmutex->lock();

		DEBUGLOCKS(("[lock_record_notify: lr=%p, t=%p, waitingthread=%p, one=%d]", lr, t, waiter, one));

		// Signal the waiter.
		waiter->waitcond->signal();

		// Mark the thread as signaled.
		waiter->signaled = true;

		// Leave the wait-mutex.
		waiter->waitmutex->unlock();

		// If we should only wake one thread, we are done.
		if (one)
			break;
	}
}


/* lock_monitor_notify *********************************************************

   Notify one thread or all threads waiting on the given object.

   IN:
      t............the current thread
	  o............the object
	  one..........if true, only notify one thread

   PRE-CONDITION:
      The current thread must be the owner of the object's monitor.
   
*******************************************************************************/

static void lock_monitor_notify(threadobject *t, java_handle_t *o, bool one)
{
	lock_record_t* lr = NULL;

	{
		// This scope is inside a critical section.
		GCCriticalSection cs;

		uintptr_t lw_cache = *lock_lockword_get(o);
		Lockword lockword(lw_cache);

		// Check if we own this monitor.
		// NOTE: We don't have to worry about stale values here, as any
		// stale value will fail this check.

		if (lockword.is_fat_lock()) {
			lr = lockword.get_fat_lock();

			if (lr->owner != t) {
				exceptions_throw_illegalmonitorstateexception();
				return;
			}
		}
		else {
			// It's a thin lock.
			if (lockword.get_thin_lock_without_count() != t->thinlock) {
				exceptions_throw_illegalmonitorstateexception();
				return;
			}

			// No thread can wait on a thin lock, so there's nothing to do.
			return;
		}
	}

	// { The thread t owns the fat lock record lr on the object o }
	lock_record_notify(t, lr, one);
}



/*============================================================================*/
/* INQUIRY FUNCIONS                                                           */
/*============================================================================*/


/* lock_is_held_by_current_thread **********************************************

   Return true if the current thread owns the monitor of the given object.

   IN:
	  o............the object

   RETURN VALUE:
      true, if the current thread holds the lock of this object.
   
*******************************************************************************/

bool lock_is_held_by_current_thread(java_handle_t *o)
{
	// This function is inside a critical section.
	GCCriticalSection cs;

	// Check if we own this monitor.
	// NOTE: We don't have to worry about stale values here, as any
	// stale value will fail this check.
	threadobject* t = thread_get_current();
	uintptr_t lw_cache = *lock_lockword_get(o);
	Lockword lockword(lw_cache);

	if (lockword.is_fat_lock()) {
		// It's a fat lock.
		lock_record_t* lr = lockword.get_fat_lock();
		return (lr->owner == t);
	}
	else {
		// It's a thin lock.
		return (lockword.get_thin_lock_without_count() == t->thinlock);
	}
}



/*============================================================================*/
/* WRAPPERS FOR OPERATIONS ON THE CURRENT THREAD                              */
/*============================================================================*/


/* lock_wait_for_object ********************************************************

   Wait for the given object.

   IN:
	  o............the object
	  millis.......milliseconds to wait
	  nanos........nanoseconds to wait
   
*******************************************************************************/

void lock_wait_for_object(java_handle_t *o, s8 millis, s4 nanos)
{
	threadobject *thread;

	thread = THREADOBJECT;

	lock_monitor_wait(thread, o, millis, nanos);
}


/* lock_notify_object **********************************************************

   Notify one thread waiting on the given object.

   IN:
	  o............the object
   
*******************************************************************************/

void lock_notify_object(java_handle_t *o)
{
	threadobject *thread;

	thread = THREADOBJECT;

	lock_monitor_notify(thread, o, true);
}


/* lock_notify_all_object ******************************************************

   Notify all threads waiting on the given object.

   IN:
	  o............the object
   
*******************************************************************************/

void lock_notify_all_object(java_handle_t *o)
{
	threadobject *thread;

	thread = THREADOBJECT;

	lock_monitor_notify(thread, o, false);
}


/*
 * These are local overrides for various environment variables in Emacs.
 * Please do not remove this and leave it at the end of the file, where
 * Emacs will automagically detect them.
 * ---------------------------------------------------------------------
 * Local variables:
 * mode: c++
 * indent-tabs-mode: t
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim:noexpandtab:sw=4:ts=4:
 */
