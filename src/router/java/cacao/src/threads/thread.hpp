/* src/threads/thread.hpp - machine independent thread functions

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


#ifndef THREAD_HPP_
#define THREAD_HPP_ 1

#include "config.h"

#include "native/llni.hpp"
#include "vm/global.hpp"
#include "vm/options.hpp"    // for opt_DebugThreads
#include "vm/os.hpp"
#include "vm/types.hpp"
#include "vm/utf8.hpp"       // for Utf8String

class  Condition;
class  DumpMemory;
struct localref_table;
class  Mutex;
struct stackframeinfo_t;
struct JavaVMAttachArgs;

/* thread states **************************************************************/

enum ThreadState {
	THREAD_STATE_NEW           = 0,
	THREAD_STATE_RUNNABLE      = 1,
	THREAD_STATE_BLOCKED       = 2,
	THREAD_STATE_WAITING       = 3,
	THREAD_STATE_TIMED_WAITING = 4,
	THREAD_STATE_TERMINATED    = 5,
	THREAD_STATE_PARKED        = 6,
	THREAD_STATE_TIMED_PARKED  = 7
};

enum ThreadFlag {
  THREAD_FLAG_JAVA      = 0x01,   // a normal Java thread
  THREAD_FLAG_INTERNAL  = 0x02,   // CACAO internal thread
  THREAD_FLAG_DAEMON    = 0x04,   // daemon thread
  THREAD_FLAG_IN_NATIVE = 0x08    // currently executing native code
};

enum SuspendReason {
  SUSPEND_REASON_NONE      = 0,   // no reason to suspend
  SUSPEND_REASON_JAVA      = 1,   // suspended from java.lang.Thread
  SUSPEND_REASON_STOPWORLD = 2,   // suspended from stop-the-world
  SUSPEND_REASON_DUMP      = 3,   // suspended from threadlist dumping
  SUSPEND_REASON_JVMTI     = 4    // suspended from JVMTI agent
};

/* thread priorities **********************************************************/

enum ThreadPriority {
	MIN_PRIORITY  =  1,
	NORM_PRIORITY =  5,
	MAX_PRIORITY  = 10
};

/* threadobject ***************************************************************/

#if defined(ENABLE_THREADS)
# include "threads/posix/threadobject.hpp"
#else
# include "threads/none/threadobject.hpp"
#endif

struct threadobject {
	//*****  platform specific thread data
	cacao::detail::threadobject impl;

	java_object_t        *object;       /* link to java.lang.Thread object    */

	ptrint                thinlock;     /* pre-computed thin lock value       */
	u4                    flags;        /* flag field                         */
	ThreadState           state;        /* state field                        */
	
	//***** for ThreadList
	bool                  is_in_active_list; /* for debugging only            */
	s4                    index;        /* thread index, starting with 1      */

	//***** for the sable tasuki lock extension */
	bool                  flc_bit;
	struct threadobject  *flc_list;     /* FLC list head for this thread      */
	struct threadobject  *flc_tail;     /* tail pointer for FLC list          */
	struct threadobject  *flc_next;     /* next pointer for FLC list          */
	java_handle_t        *flc_object;
	Mutex*                flc_lock;     /* controlling access to these fields */
	Condition*            flc_cond;

	//***** these are used for the wait/notify implementation
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
	stackframeinfo_t     *_stackframeinfo;   /* current native stackframeinfo */
	localref_table       *_localref_table;   /* JNI local references          */

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

/* debug **********************************************************************/

#if !defined(NDEBUG)
# define DEBUGTHREADS(message, thread) \
	do { \
		if (opt_DebugThreads) { \
			printf("[Thread %-16s: ", message); \
			thread_print_info(thread); \
			printf("]\n"); \
		} \
	} while (0)
#else
# define DEBUGTHREADS(message, thread)
#endif


/* global variables ***********************************************************/

#if defined(__LINUX__)
/* XXX Remove for exact-GC. */
extern bool threads_pthreads_implementation_nptl;
#endif


/* state for trace java call **************************************************/

#if !defined(NDEBUG)
#	define TRACEJAVACALLINDENT (THREADOBJECT->tracejavacallindent)
#	define TRACEJAVACALLCOUNT  (THREADOBJECT->tracejavacallcount)
#endif


/* counter for verbose call filter ********************************************/

#if defined(ENABLE_DEBUG_FILTER)
#	define FILTERVERBOSECALLCTR (THREADOBJECT->filterverbosecallctr)
#endif


/* native-world flags *********************************************************/

#if defined(ENABLE_GC_CACAO)
# define THREAD_NATIVEWORLD_ENTER THREADOBJECT->flags |=  THREAD_FLAG_IN_NATIVE
# define THREAD_NATIVEWORLD_EXIT  THREADOBJECT->flags &= ~THREAD_FLAG_IN_NATIVE
#else
# define THREAD_NATIVEWORLD_ENTER /*nop*/
# define THREAD_NATIVEWORLD_EXIT  /*nop*/
#endif


/* inline functions ***********************************************************/

inline static threadobject* thread_get_current(void);

#if defined(ENABLE_THREADS)
# include "threads/posix/thread-posix.hpp"
#else
# include "threads/none/thread-none.hpp"
#endif

/***
 * Return the java.lang.Thread object for the current thread.
 */
inline static java_handle_t *thread_get_current_object(void) {
	return LLNI_WRAP(thread_get_current()->object);
}


/* cacaothread_get_state *******************************************************

   Returns the current state of the given thread.

   ARGUMENTS:
       t ... the thread to check

   RETURN:
       thread state

*******************************************************************************/

inline static int cacaothread_get_state(threadobject *t)
{
	return t->state;
}


/* thread_is_attached **********************************************************

   Returns if the given thread is attached to the VM.

   ARGUMENTS:
       t ... the thread to check

   RETURN:
       true .... the thread is attached to the VM
       false ... the thread is not

*******************************************************************************/

inline static bool thread_is_attached(threadobject *t)
{
	java_handle_t *o;

	o = LLNI_WRAP(t->object);

	return o != NULL;
}


/* thread_is_daemon ************************************************************

   Returns if the given thread is a daemon thread.

   ARGUMENTS:
       t ... the thread to check

   RETURN:
       true .... the thread is a daemon thread
       false ... the thread is not

*******************************************************************************/

inline static bool thread_is_daemon(threadobject *t)
{
	return (t->flags & THREAD_FLAG_DAEMON) != 0;
}


/* thread_current_is_attached **************************************************

   Returns if the current thread is attached to the VM.

   RETURN:
       true .... the thread is attached to the VM
       false ... the thread is not

*******************************************************************************/

inline static bool thread_current_is_attached(void)
{
	threadobject  *t;

	t = thread_get_current();

	if (t == NULL)
		return false;

	return thread_is_attached(t);
}

inline static struct stackframeinfo_t* threads_get_current_stackframeinfo(void)
{
	return THREADOBJECT->_stackframeinfo;
}

inline static void threads_set_current_stackframeinfo(struct stackframeinfo_t* sfi)
{
	THREADOBJECT->_stackframeinfo = sfi;
}


/* function prototypes ********************************************************/

void          threads_preinit(void);
void          threads_init(void);

void          thread_free(threadobject *t);

bool          threads_thread_start_internal(Utf8String name, functionptr f);
void          threads_thread_start(java_handle_t *object);

bool          thread_attach_current_thread(JavaVMAttachArgs *vm_aargs, bool isdaemon);
bool          thread_attach_current_external_thread(JavaVMAttachArgs *vm_aargs, bool isdaemon);
bool          thread_detach_current_thread(void);

bool          thread_detach_current_external_thread(void);

void          thread_fprint_name(threadobject *t, FILE *stream);
void          thread_print_info(threadobject *t);

intptr_t      threads_get_current_tid(void);
intptr_t      threads_get_tid(threadobject*);

void          thread_set_state_runnable(threadobject *t);
void          thread_set_state_waiting(threadobject *t);
void          thread_set_state_timed_waiting(threadobject *t);
void          thread_set_state_parked(threadobject *t);
void          thread_set_state_timed_parked(threadobject *t);
void          thread_set_state_terminated(threadobject *t);

threadobject *thread_get_thread(java_handle_t *h);

bool          threads_thread_is_alive(threadobject *t);
bool          thread_is_interrupted(threadobject *t);
void          thread_set_interrupted(threadobject *t, bool interrupted);
void          threads_thread_interrupt(threadobject *thread);


void threads_start_thread(threadobject *thread, functionptr function);

void threads_set_thread_priority(threadobject *t, int priority);

bool threads_suspend_thread(threadobject *thread, SuspendReason reason);
bool threads_resume_thread(threadobject *thread, SuspendReason reason);
void threads_suspend_ack();

void threads_join_all_threads(void);

void threads_sleep(int64_t millis, int32_t nanos);

void threads_wait_with_timeout_relative(threadobject *t, s8 millis, s4 nanos);

void threads_park(bool absolute, int64_t nanos);
void threads_unpark(threadobject *thread);

#if defined(ENABLE_TLH)
void threads_tlh_add_frame();
void threads_tlh_remove_frame();
#endif

/* implementation specific functions */

void          threads_impl_preinit(void);
void          threads_impl_init(void);

#if defined(ENABLE_GC_CACAO)
void          threads_mutex_gc_lock(void);
void          threads_mutex_gc_unlock(void);
#endif

void          threads_impl_thread_clear(threadobject *t);
void          threads_impl_thread_reuse(threadobject *t);
void          threads_impl_clear_heap_pointers(threadobject *t);
void          threads_impl_thread_start(threadobject *thread, functionptr f);

void          threads_yield(void);

void          thread_handle_set_priority(java_handle_t *th, int);
bool          thread_handle_is_interrupted(java_handle_t *th);
void          thread_handle_interrupt(java_handle_t *th);
int           thread_handle_get_state(java_handle_t *th);

#endif // THREAD_HPP_


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
