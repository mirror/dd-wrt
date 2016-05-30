/* src/threads/none/thread-none.cpp - fake threads

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

#include "threads/thread.hpp"
#include <cassert>
#include "threads/condition.hpp"        // for Condition
#include "threads/lock.hpp"             // for lock_monitor_enter, etc
#include "threads/mutex.hpp"            // for Mutex
#include "threads/thread.hpp"           // for DEBUGTHREADS, etc
#include "threads/threadlist.hpp"       // for ThreadList
#include "threads/ThreadRuntime.hpp"    // for ThreadRuntime
#include "vm/exceptions.hpp"
#include "vm/javaobjects.hpp"           // for java_lang_Thread
#include "vm/vm.hpp"                    // for vm_call_method

using namespace cacao;

/* global variables ***********************************************************/

threadobject *thread_current;


/* functions ******************************************************************/

/***
 * Do some early initialization of stuff required.
 *
 * ATTENTION: Do NOT use any Java heap allocation here, as gc_init()
 *            is called AFTER this function!
 */
void threads_impl_preinit() {}


/***
 * Initializes the implementation specific bits.
 */
void threads_impl_init() {
}

/***
 * Should start a thread, nop
 */
void threads_impl_thread_start(threadobject *thread, functionptr f) {}


/* threads_set_thread_priority *************************************************

   Set the priority of the given thread.

   IN:
      tid..........thread id
	  priority.....priority to set

******************************************************************************/

void threads_set_thread_priority(threadobject *t, int priority)
{
}

/**
 * Detaches the current thread from the VM.
 *
 * @return true on success, false otherwise
 */
bool thread_detach_current_thread(void)
{
	threadobject* t = thread_get_current();

	/* Sanity check. */

	assert(t != NULL);

    /* If the given thread has already been detached, this operation
	   is a no-op. */

	if (thread_is_attached(t) == false)
		return true;

	DEBUGTHREADS("detaching", t);

	java_handle_t* object = LLNI_WRAP(t->object);
	java_lang_Thread jlt(object);

#if defined(ENABLE_JAVASE)
	java_handle_t* group = jlt.get_group();

    /* If there's an uncaught exception, call uncaughtException on the
       thread's exception handler, or the thread's group if this is
       unset. */

	java_handle_t* e = exceptions_get_and_clear_exception();

    if (e != NULL) {
		/* We use the type void* for handler here, as it's not trivial
		   to build the java_lang_Thread_UncaughtExceptionHandler
		   header file with cacaoh. */

		java_handle_t *handler = ThreadRuntime::get_thread_exception_handler(jlt);

		classinfo*     c;
		java_handle_t* h;

		if (handler != NULL) {
			LLNI_class_get(handler, c);
			h = (java_handle_t *) handler;
		}
		else {
			LLNI_class_get(group, c);
			h = (java_handle_t *) group;
		}

		methodinfo* m = class_resolveclassmethod(c,
												 utf8::uncaughtException,
												 utf8::java_lang_Thread_java_lang_Throwable__V,
												 NULL,
												 true);

		if (m == NULL)
			return false;

		(void) vm_call_method(m, h, object, e);

		if (exceptions_get_exception())
			return false;
    }

	/* XXX TWISTI: should all threads be in a ThreadGroup? */

	/* Remove thread from the thread group. */

	if (group != NULL) {
		classinfo* c;
		LLNI_class_get(group, c);

		methodinfo *m = ThreadRuntime::get_threadgroup_remove_method(c);

		if (m == NULL)
			return false;

		(void) vm_call_method(m, group, object);

		if (exceptions_get_exception())
			return false;

		// Clear the ThreadGroup in the Java thread object (Mauve
		// test: gnu/testlet/java/lang/Thread/getThreadGroup).
		jlt.set_group(NULL);
	}
#endif

	/* Thread has terminated. */

	thread_set_state_terminated(t);

	/* Notify all threads waiting on this thread.  These are joining
	   this thread. */

	/* Free the internal thread data-structure. */

	thread_free(t);

	return true;
}

/**
 * Suspend the passed thread. Execution of that thread stops until the thread
 * is explicitly resumed again.
 *
 * @param thread The thread to be suspended.
 * @param reason Reason for suspending the given thread.
 * @return True of operation was successful, false otherwise.
 */
bool threads_suspend_thread(threadobject *thread, SuspendReason reason)
{
	// Sanity check.
	assert(reason != SUSPEND_REASON_NONE);

	// Check if thread is already suspended.
	if (thread->suspended)
		return false;

	// Check if thread is in the process of suspending.
	if (thread->suspend_reason != SUSPEND_REASON_NONE)
		return false;

	// Set the reason for suspending the thread.
	thread->suspend_reason = reason;
	return true;
}


/**
 * Resumes execution of the passed thread.
 *
 * @param thread The thread to be resumed.
 * @param reason Reason for suspending the given thread.
 * @return True of operation was successful, false otherwise.
 */
bool threads_resume_thread(threadobject *thread, SuspendReason reason)
{
	// Sanity check.
	assert(thread != THREADOBJECT);
	assert(reason != SUSPEND_REASON_NONE);

	// Check if thread really is suspended.
	if (!thread->suspended)
		return false;

	// Threads can only be resumed for the same reason they were suspended.
	if (thread->suspend_reason != reason)
		return false;

	// Clear the reason for suspending the thread.
	thread->suspend_reason = SUSPEND_REASON_NONE;
	return true;
}

void threads_impl_clear_heap_pointers(threadobject *t)
{
	t->object     = 0;
	t->flc_object = 0;
}

/***
 * Acknowledges the suspension of the current thread.
 */
void threads_suspend_ack() {}

/***
 * Join all non-daemon threads.
 */
void threads_join_all_threads() {}

/***
 * Clears all fields in threadobject.
 */
void threads_impl_thread_clear(threadobject *t) {
	t->object = NULL;

	t->thinlock = 0;

	t->index             = 0;
	t->flags             = 0;
	t->state             = THREAD_STATE_NEW;
	t->is_in_active_list = false;

	t->interrupted = false;
	t->signaled    = false;

	t->suspended      = false;
	t->suspend_reason = SUSPEND_REASON_NONE;

	t->pc = NULL;

	t->_exceptionptr   = NULL;
	t->_stackframeinfo = NULL;
	t->_localref_table = NULL;

#if defined(ENABLE_INTRP)
	t->_global_sp = NULL;
#endif

#if defined(ENABLE_GC_CACAO)
	t->gc_critical = false;

	t->ss = NULL;
	t->es = NULL;
#endif

	// Simply reuse the existing dump memory.
}


/***
 * Resets some implementation fields in threadobject.
 */
void threads_impl_thread_reuse(threadobject *t) {}

/***
 * Interrupt the given thread.
 *
 * The thread gets its interrupted flag is set to true.
 */
void threads_thread_interrupt(threadobject *t) {
	// Signal the thread a "waitcond" and tell it that it has been
	// interrupted.

	DEBUGTHREADS("interrupted", t);

	t->interrupted = true;
}

/***
 * Sleep the current thread for the specified amount of time.
 *
 * @param millis Milliseconds to sleep.
 * @param nanos  Nanoseconds to sleep.
 */
void threads_sleep(int64_t millis, int32_t nanos) {
	if (millis < 0) {
		exceptions_throw_illegalargumentexception(/*"timeout value is negative"*/);
		return;
	}

	threadobject *t = thread_get_current();

	if (thread_is_interrupted(t) && !exceptions_get_exception()) {
		// Clear interrupted flag
		// (Mauve test: gnu/testlet/java/lang/Thread/interrupt).

		thread_set_interrupted(t, false);

		exceptions_throw_interruptedexception(/*"sleep interrupted"*/);
		return;
   }

	// (Note taken from classpath/vm/reference/java/lang/VMThread.java (sleep))
	// Note: JDK treats a zero length sleep is like Thread.yield(),
	// without checking the interrupted status of the thread.  It's
	// unclear if this is a bug in the implementation or the spec.
	// See http://bugs.sun.com/bugdatabase/view_bug.do?bug_id=6213203 */
	if (millis == 0 && nanos == 0) {
	  threads_yield();
	} else {
		if (thread_is_interrupted(t)) {
			thread_set_interrupted(t, false);

			// An other exception could have been thrown
			// (e.g. ThreadDeathException).
			if (!exceptions_get_exception())
				exceptions_throw_interruptedexception();
		}
	}
}


/***
 * Wait for the given maximum amount of time on a monitor until either
 * we are notified, we are interrupted, or the time is up.
 */
void threads_wait_with_timeout_relative(threadobject *thread, s8 millis, s4 nanos) {}


/**
 * Park the current thread for the specified amount of time or until a
 * specified deadline.
 *
 * @param absolute Is the time in nanos a deadline or a duration?
 * @param nanos    Nanoseconds to park (absolute=false)
 *                 or deadline in milliseconds (absolute=true)
 */
void threads_park(bool absolute, int64_t nanos) {}

/**
 * Unpark the specified thread.
 *
 * @param t The thread to unpark.
 */
void threads_unpark(threadobject *t) {}


/***
 * Yield to the scheduler.
 */
void threads_yield(void) {}


/***
 * Return the tid of a thread.
 */
intptr_t threads_get_tid(threadobject *t) {
	return 0;
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
