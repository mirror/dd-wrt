/* src/threads/threadobject.hpp - Thread data structure

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


#ifndef THREADOBJECT_HPP_
#define THREADOBJECT_HPP_ 1

#ifndef THREAD_HPP_
# error "Do not directly include this header, include threads/thread.hpp instead"
#endif

#include "config.h"
#include "vm/types.hpp"

#if defined(ENABLE_TLH)
# include "mm/tlh.hpp"
#endif

#include "threads/condition.hpp"
#include "threads/mutex.hpp"

#if defined(ENABLE_THREADS)
# include "threads/posix/threadobject-posix.hpp"
#else
# include "threads/none/threadobject-none.hpp"
#endif

/* forward declarations */
class DumpMemory;

/* threadobject ****************************************************************

   Struct holding thread local variables.

*******************************************************************************/

struct threadobject {
	detail::threadobject  impl;         // platform specific thread data

	java_object_t        *object;       /* link to java.lang.Thread object    */

	ptrint                thinlock;     /* pre-computed thin lock value       */

	s4                    index;        /* thread index, starting with 1      */
	u4                    flags;        /* flag field                         */
	ThreadState           state;        /* state field                        */
	bool                  is_in_active_list; /* for debugging only            */

	/* for the sable tasuki lock extension */
	bool                  flc_bit;
	struct threadobject  *flc_list;     /* FLC list head for this thread      */
	struct threadobject  *flc_tail;     /* tail pointer for FLC list          */
	struct threadobject  *flc_next;     /* next pointer for FLC list          */
	java_handle_t        *flc_object;
	Mutex*                flc_lock;     /* controlling access to these fields */
	Condition*            flc_cond;

	/* these are used for the wait/notify implementation                      */
	Mutex*                waitmutex;
	Condition*            waitcond;

	Mutex*                suspendmutex; /* lock before suspending this thread */
	Condition*            suspendcond;  /* notify to resume this thread       */

	bool                  interrupted;
	bool                  signaled;
	bool                  park_permit;

	bool                  suspended;    /* is this thread suspended?          */
	SuspendReason         suspend_reason; /* reason for suspending            */

	u1                   *pc;           /* current PC (used for profiling)    */

	java_object_t        *_exceptionptr;     /* current exception             */
	struct stackframeinfo_t     *_stackframeinfo;   /* current native stackframeinfo */
	struct localref_table       *_localref_table;   /* JNI local references          */

#if defined(ENABLE_INTRP)
	Cell                 *_global_sp;        /* stack pointer for interpreter */
#endif

#if defined(ENABLE_GC_CACAO)
	bool                  gc_critical;  /* indicates a critical section       */

	sourcestate_t        *ss;
	executionstate_t     *es;
#endif

	DumpMemory*          _dumpmemory;     ///< Dump memory structure.

#if defined(ENABLE_DEBUG_FILTER)
	u2                    filterverbosecallctr[2]; /* counters for verbose call filter */
#endif

#if !defined(NDEBUG)
	s4                    tracejavacallindent;
	u4                    tracejavacallcount;
#endif

#if defined(ENABLE_TLH)
	tlh_t                 tlh;
#endif

#if defined(ENABLE_ESCAPE_REASON)
	void *escape_reasons;
#endif
};


/* current threadobject *******************************************************/

#if defined(HAVE___THREAD)

#define THREADOBJECT      thread_current

extern __thread threadobject *thread_current;

#else /* defined(HAVE___THREAD) */

#define THREADOBJECT \
	((threadobject *) pthread_getspecific(thread_current_key))

extern pthread_key_t thread_current_key;

#endif /* defined(HAVE___THREAD) */


/* native-world flags *********************************************************/

#if defined(ENABLE_GC_CACAO)
# define THREAD_NATIVEWORLD_ENTER THREADOBJECT->flags |=  THREAD_FLAG_IN_NATIVE
# define THREAD_NATIVEWORLD_EXIT  THREADOBJECT->flags &= ~THREAD_FLAG_IN_NATIVE
#else
# define THREAD_NATIVEWORLD_ENTER /*nop*/
# define THREAD_NATIVEWORLD_EXIT  /*nop*/
#endif


/* counter for verbose call filter ********************************************/

#if defined(ENABLE_DEBUG_FILTER)
#	define FILTERVERBOSECALLCTR (THREADOBJECT->filterverbosecallctr)
#endif

/* state for trace java call **************************************************/

#if !defined(NDEBUG)
#	define TRACEJAVACALLINDENT (THREADOBJECT->tracejavacallindent)
#	define TRACEJAVACALLCOUNT (THREADOBJECT->tracejavacallcount)
#endif

inline static threadobject* thread_get_current(void);


// Includes.
#include "native/localref.hpp"

#include "threads/lock.hpp"

#include "vm/global.hpp"
#include "vm/vm.hpp"

#if defined(ENABLE_GC_CACAO)
# include "vm/jit/executionstate.hpp"
# include "vm/jit/replace.hpp"
#endif

#if defined(ENABLE_INTRP)
#include "vm/jit/intrp/intrp.h"
#endif


/* inline functions ***********************************************************/

/**
 * Return the Thread object of the current thread.
 *
 * @return The current Thread object.
 */
inline static threadobject* thread_get_current(void)
{
	threadobject *t;

#if defined(HAVE___THREAD)
	t = thread_current;
#else
	t = (threadobject *) pthread_getspecific(thread_current_key);
#endif

	return t;
}


/**
 * Set the current Thread object.
 *
 * @param t The thread object to set.
 */
inline static void thread_set_current(threadobject* t)
{
#if defined(HAVE___THREAD)
	thread_current = t;
#else
	int result;

	result = pthread_setspecific(thread_current_key, t);

	if (result != 0)
		//os::abort_errnum(result, "thread_set_current: pthread_setspecific failed");
		vm_abort("thread_set_current: pthread_setspecific failed");
#endif
}


inline static struct stackframeinfo_t* threads_get_current_stackframeinfo(void)
{
	return THREADOBJECT->_stackframeinfo;
}

inline static void threads_set_current_stackframeinfo(struct stackframeinfo_t* sfi)
{
	THREADOBJECT->_stackframeinfo = sfi;
}


/* functions ******************************************************************/

void threads_start_thread(threadobject *thread, functionptr function);

void threads_set_thread_priority(pthread_t tid, int priority);

bool threads_suspend_thread(threadobject *thread, SuspendReason reason);
bool threads_resume_thread(threadobject *thread, SuspendReason reason);
void threads_suspend_ack();

void threads_join_all_threads(void);

void threads_sleep(int64_t millis, int32_t nanos);

void threads_wait_with_timeout_relative(threadobject *t, s8 millis, s4 nanos);

void threads_thread_interrupt(threadobject *thread);

void threads_park(bool absolute, int64_t nanos);
void threads_unpark(threadobject *thread);

#if defined(ENABLE_TLH)
void threads_tlh_add_frame();
void threads_tlh_remove_frame();
#endif

#endif // THREAD_POSIX_HPP_

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
