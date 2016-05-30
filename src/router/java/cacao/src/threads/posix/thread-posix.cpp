/* src/threads/posix/thread-posix.cpp - POSIX thread functions

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

#define __STDC_LIMIT_MACROS

#include "threads/thread.hpp"           // for threadobject, etc
#include <assert.h>                     // for assert
#include <errno.h>                      // for errno, EINTR
#include <pthread.h>                    // for pthread_attr_init, etc
#include <sched.h>                      // for sched_param, sched_yield, etc
#if !defined(__DARWIN__)
#include <semaphore.h>                  // for sem_t, sem_destroy, etc
#endif
#include <signal.h>                     // for pthread_kill, SIGUSR1
#include <stdint.h>                     // for INT32_MAX
#include <stdlib.h>                     // for NULL
#include <string.h>                     // for strerror
#include <sys/time.h>                   // for timeval, gettimeofday
#include <sys/types.h>                  // for int32_t, int64_t
#include <time.h>                       // for timespec
#include "config.h"                     // for ENABLE_GC_BOEHM, etc
#include "native/llni.hpp"              // for LLNI_class_get, LLNI_WRAP
#include "threads/condition.hpp"        // for Condition
#include "threads/lock.hpp"             // for lock_monitor_enter, etc
#include "threads/mutex.hpp"            // for Mutex
#include "threads/thread.hpp"           // for DEBUGTHREADS, etc
#include "threads/threadlist.hpp"       // for ThreadList
#include "threads/ThreadRuntime.hpp"    // for ThreadRuntime
#include "vm/class.hpp"                 // for class_resolveclassmethod, etc
#include "vm/exceptions.hpp"            // for exceptions_get_exception, etc
#include "vm/global.hpp"                // for java_handle_t, functionptr
#include "vm/hook.hpp"                  // for thread_end, thread_start
#include "vm/javaobjects.hpp"           // for java_lang_Thread
#include "vm/options.hpp"               // for opt_stacksize
#include "vm/os.hpp"                    // for os
#include "vm/signallocal.hpp"
#include "vm/types.hpp"                 // for s4, s8, u2
#include "vm/utf8.hpp"
#include "vm/vm.hpp"                    // for vm_abort, vm_call_method

#ifndef __SIGRTMIN /* if arch has not defined it in bits/signum.h... */
# define __SIGRTMIN 32
#endif
#ifndef __SIGRTMAX
#define __SIGRTMAX (_NSIG - 1)
#endif

#if defined(ENABLE_GC_CACAO)
# include "mm/cacao-gc/gc.h"
#endif

#if defined(__DARWIN__)

typedef struct {
	Mutex* mutex;
	Condition* cond;
	int value;
} sem_t;

#else
# include <semaphore.h>
#endif

#include "mm/gc-boehm.hpp"

struct methodinfo;

using namespace cacao;

#if defined(__DARWIN__)
/* Darwin has no working semaphore implementation.  This one is taken
   from Boehm-GC. */

/*
   This is a very simple semaphore implementation for Darwin. It
   is implemented in terms of pthreads calls so it isn't async signal
   safe. This isn't a problem because signals aren't used to
   suspend threads on Darwin.
*/
   
static int sem_init(sem_t *sem, int pshared, int value)
{
	if (pshared)
		assert(0);

	sem->mutex = new Mutex();
	sem->cond  = new Condition();
	sem->value = value;

	return 0;
}

static int sem_post(sem_t *sem)
{
	sem->mutex->lock();
	sem->value++;
	sem->cond->signal();
	sem->mutex->unlock();

	return 0;
}

static int sem_wait(sem_t *sem)
{
	sem->mutex->lock();

	while (sem->value == 0) {
		sem->cond->wait(sem->mutex);
	}

	sem->value--;
	sem->mutex->unlock();

	return 0;
}

static int sem_destroy(sem_t *sem)
{
	delete sem->cond;
	delete sem->mutex;

	return 0;
}
#endif /* defined(__DARWIN__) */


/* startupinfo *****************************************************************

   Struct used to pass info from threads_start_thread to 
   threads_startup_thread.

******************************************************************************/

typedef struct {
	threadobject *thread;      /* threadobject for this thread             */
	functionptr   function;    /* function to run in the new thread        */
	sem_t        *psem;        /* signals when thread has been entered     */
	                           /* in the thread list                       */
	sem_t        *psem_first;  /* signals when pthread_create has returned */
} startupinfo;


/* prototypes *****************************************************************/

static void threads_calc_absolute_time(struct timespec *tm, s8 millis, s4 nanos);
static void threads_set_thread_priority(pthread_t tid, int priority);


/******************************************************************************/
/* GLOBAL VARIABLES                                                           */
/******************************************************************************/

/* the thread object of the current thread                                    */
/* This is either a thread-local variable defined with __thread, or           */
/* a thread-specific value stored with key threads_current_threadobject_key.  */
#if defined(HAVE___THREAD)
__thread threadobject *thread_current;
#else
pthread_key_t thread_current_key;
#endif

/* global mutex for stop-the-world                                            */
static Mutex* stopworldlock;

#if defined(ENABLE_GC_CACAO)
/* global mutex for the GC */
static Mutex* mutex_gc;
#endif

/* global mutex and condition for joining threads on exit */
static Condition* cond_join;

#if defined(ENABLE_GC_CACAO)
/* semaphore used for acknowleding thread suspension                          */
static sem_t suspend_ack;
#endif


/* threads_sem_init ************************************************************
 
   Initialize a semaphore. Checks against errors and interruptions.

   IN:
       sem..............the semaphore to initialize
	   shared...........true if this semaphore will be shared between processes
	   value............the initial value for the semaphore
   
*******************************************************************************/

void threads_sem_init(sem_t *sem, bool shared, int value)
{
	int r;

	assert(sem);

	do {
		r = sem_init(sem, shared, value);
		if (r == 0)
			return;
	} while (errno == EINTR);

	vm_abort("sem_init failed: %s", strerror(errno));
}


/* threads_sem_wait ************************************************************
 
   Wait for a semaphore, non-interruptible.

   IMPORTANT: Always use this function instead of `sem_wait` directly, as
              `sem_wait` may be interrupted by signals!
  
   IN:
       sem..............the semaphore to wait on
   
*******************************************************************************/

void threads_sem_wait(sem_t *sem)
{
	int r;

	assert(sem);

	do {
		r = sem_wait(sem);
		if (r == 0)
			return;
	} while (errno == EINTR);

	vm_abort("sem_wait failed: %s", strerror(errno));
}


/* threads_sem_post ************************************************************
 
   Increase the count of a semaphore. Checks for errors.

   IN:
       sem..............the semaphore to increase the count of
   
*******************************************************************************/

void threads_sem_post(sem_t *sem)
{
	int r;

	assert(sem);

	/* unlike sem_wait, sem_post is not interruptible */

	r = sem_post(sem);
	if (r == 0)
		return;

	vm_abort("sem_post failed: %s", strerror(errno));
}


/* threads_stopworld ***********************************************************

   Stops the world from turning. All threads except the calling one
   are suspended. The function returns as soon as all threads have
   acknowledged their suspension.

*******************************************************************************/

#if defined(ENABLE_GC_CACAO)
void threads_stopworld(void)
{
#if !defined(__DARWIN__) && !defined(__CYGWIN__)
	threadobject *t;
	threadobject *self;
	bool result;
	s4 count, i;
#endif

	stopworldlock->lock();

	/* lock the threads lists */

	threadlist_lock();

#if defined(__DARWIN__)
	/*threads_cast_darwinstop();*/
	assert(0);
#elif defined(__CYGWIN__)
	/* TODO */
	assert(0);
#else
	self = THREADOBJECT;

	DEBUGTHREADS("stops World", self);

	count = 0;

	/* suspend all running threads */
	for (t = threadlist_first(); t != NULL; t = threadlist_next(t)) {
		/* don't send the signal to ourself */

		if (t == self)
			continue;

		/* don't send the signal to NEW threads (because they are not
		   completely initialized) */

		if (t->state == THREAD_STATE_NEW)
			continue;

		/* send the signal */

		result = threads_suspend_thread(t, SUSPEND_REASON_STOPWORLD);
		assert(result);

		/* increase threads count */

		count++;
	}

	/* wait for all threads signaled to suspend */
	for (i = 0; i < count; i++)
		threads_sem_wait(&suspend_ack);
#endif

	/* ATTENTION: Don't unlock the threads-lists here so that
	   non-signaled NEW threads can't change their state and execute
	   code. */
}
#endif


/* threads_startworld **********************************************************

   Starts the world again after it has previously been stopped. 

*******************************************************************************/

#if defined(ENABLE_GC_CACAO)
void threads_startworld(void)
{
#if !defined(__DARWIN__) && !defined(__CYGWIN__)
	threadobject *t;
	threadobject *self;
	bool result;
	s4 count, i;
#endif

#if defined(__DARWIN__)
	/*threads_cast_darwinresume();*/
	assert(0);
#elif defined(__IRIX__)
	threads_cast_irixresume();
#elif defined(__CYGWIN__)
	/* TODO */
	assert(0);
#else
	self = THREADOBJECT;

	DEBUGTHREADS("starts World", self);

	count = 0;

	/* resume all thread we haltet */
	for (t = threadlist_first(); t != NULL; t = threadlist_next(t)) {
		/* don't send the signal to ourself */

		if (t == self)
			continue;

		/* don't send the signal to NEW threads (because they are not
		   completely initialized) */

		if (t->state == THREAD_STATE_NEW)
			continue;

		/* send the signal */

		result = threads_resume_thread(t);
		assert(result);

		/* increase threads count */

		count++;
	}

	/* wait for all threads signaled to suspend */
	for (i = 0; i < count; i++)
		threads_sem_wait(&suspend_ack);

#endif

	/* unlock the threads lists */

	threadlist_unlock();

	stopworldlock->unlock();
}
#endif


/* threads_impl_thread_clear ***************************************************

   Clears all fields in threadobject the way an MZERO would have
   done. MZERO cannot be used anymore because it would mess up the
   pthread_* bits.

   IN:
      t....the threadobject

*******************************************************************************/

void threads_impl_thread_clear(threadobject *t)
{
	t->object = NULL;

	t->thinlock = 0;

	t->index             = 0;
	t->flags             = 0;
	t->state             = THREAD_STATE_NEW;
	t->is_in_active_list = false;

	t->impl.tid = 0;

#if defined(__DARWIN__)
	t->impl.mach_thread = 0;
#endif

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

/* threads_impl_thread_reuse ***************************************************

   Resets some implementation fields in threadobject. This was
   previously done in threads_impl_thread_new.

   IN:
      t....the threadobject

*******************************************************************************/

void threads_impl_thread_reuse(threadobject *t)
{
	/* get the pthread id */

	t->impl.tid = pthread_self();
}

void threads_impl_clear_heap_pointers(threadobject *t)
{
	t->object     = 0;
	t->flc_object = 0;
}

/* threads_impl_preinit ********************************************************

   Do some early initialization of stuff required.

   ATTENTION: Do NOT use any Java heap allocation here, as gc_init()
   is called AFTER this function!

*******************************************************************************/

void threads_impl_preinit(void)
{
	stopworldlock = new Mutex();

	/* initialize exit mutex and condition (on exit we join all
	   threads) */

	cond_join = new Condition();

#if defined(ENABLE_GC_CACAO)
	/* initialize the GC mutex & suspend semaphore */

	mutex_gc = new Mutex();
 	threads_sem_init(&suspend_ack, 0, 0);
#endif

#if !defined(HAVE___THREAD)
	int result = pthread_key_create(&thread_current_key, NULL);
	if (result != 0)
		os::abort_errnum(result, "threads_impl_preinit: pthread_key_create failed");
#endif
}


/* threads_mutex_gc_lock *******************************************************

   Enter the global GC mutex.

*******************************************************************************/

#if defined(ENABLE_GC_CACAO)
void threads_mutex_gc_lock(void)
{
	mutex_gc->lock();
}
#endif


/* threads_mutex_gc_unlock *****************************************************

   Leave the global GC mutex.

*******************************************************************************/

#if defined(ENABLE_GC_CACAO)
void threads_mutex_gc_unlock(void)
{
	mutex_gc->unlock();
}
#endif

/* threads_impl_init ***********************************************************

   Initializes the implementation specific bits.

*******************************************************************************/

void threads_impl_init(void)
{
	pthread_attr_t attr;
	int            result;

	threads_set_thread_priority(pthread_self(), NORM_PRIORITY);

	/* Initialize the thread attribute object. */

	result = pthread_attr_init(&attr);

	if (result != 0)
		os::abort_errnum(result, "threads_impl_init: pthread_attr_init failed");

	result = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	if (result != 0)
		os::abort_errnum(result, "threads_impl_init: pthread_attr_setdetachstate failed");
}


/* threads_startup_thread ******************************************************

   Thread startup function called by pthread_create.

   Thread which have a startup.function != NULL are marked as internal
   threads. All other threads are threated as normal Java threads.

   NOTE: This function is not called directly by pthread_create. The Boehm GC
         inserts its own GC_start_routine in between, which then calls
		 threads_startup.

   IN:
      arg..........the argument passed to pthread_create, ie. a pointer to
	               a startupinfo struct. CAUTION: When the `psem` semaphore
				   is posted, the startupinfo struct becomes invalid! (It
				   is allocated on the stack of threads_start_thread.)

******************************************************************************/

static void *threads_startup_thread(void *arg)
{
	startupinfo  *startup;
	threadobject *t;
	sem_t        *psem;
	classinfo    *c;
	methodinfo   *m;
	functionptr   function;

#if defined(ENABLE_GC_BOEHM)
# if !defined(__DARWIN__)
	struct GC_stack_base sb;
	int result;
# endif
#endif

#if defined(ENABLE_INTRP)
	u1 *intrp_thread_stack;
#endif

#if defined(ENABLE_INTRP)
	/* create interpreter stack */

	if (opt_intrp) {
		intrp_thread_stack = GCMNEW(u1, opt_stacksize);
		MSET(intrp_thread_stack, 0, u1, opt_stacksize);
	}
	else
		intrp_thread_stack = NULL;
#endif

	/* get passed startupinfo structure and the values in there */

	startup = (startupinfo*) arg;

	t        = startup->thread;
	function = startup->function;
	psem     = startup->psem;

	/* Seems like we've encountered a situation where thread->tid was
	   not set by pthread_create. We alleviate this problem by waiting
	   for pthread_create to return. */

	threads_sem_wait(startup->psem_first);

#if defined(__DARWIN__)
	t->impl.mach_thread = mach_thread_self();
#endif

	/* Now that we are in the new thread, we can store the internal
	   thread data-structure in the TSD. */

	thread_set_current(t);

#if defined(ENABLE_GC_BOEHM)
# if defined(__DARWIN__)
	// This is currently not implemented in Boehm-GC.  Just fail silently.
# else
	/* Register the thread with Boehm-GC.  This must happen before the
	   thread allocates any memory from the GC heap.*/

	result = GC_get_stack_base(&sb);

	if (result != 0)
		vm_abort("threads_startup_thread: GC_get_stack_base failed: result=%d", result);

	GC_register_my_thread(&sb);
# endif
#endif

	// Get the java.lang.Thread object for this thread.
	java_handle_t* object = LLNI_WRAP(t->object);
	java_lang_Thread jlt(object);

	/* set our priority */

	threads_set_thread_priority(t->impl.tid, jlt.get_priority());

	/* tell threads_startup_thread that we registered ourselves */
	/* CAUTION: *startup becomes invalid with this!             */

	startup = NULL;
	threads_sem_post(psem);

#if defined(ENABLE_INTRP)
	/* set interpreter stack */

	if (opt_intrp)
		thread->_global_sp = (Cell *) (intrp_thread_stack + opt_stacksize);
#endif

	// Hook point just before the threads initial method is executed.
	Hook::thread_start(t);

	DEBUGTHREADS("starting", t);

	/* find and run the Thread.run()V method if no other function was passed */

	if (function == NULL) {
		c = ThreadRuntime::get_thread_class_from_object(object);

		m = class_resolveclassmethod(c, utf8::run, utf8::void__void, c, true);

		if (m == NULL)
			vm_abort("threads_startup_thread: run() method not found in class");

		java_handle_t *h = ThreadRuntime::get_vmthread_handle(jlt);

		/* Run the thread. */

		(void) vm_call_method(m, h);
	}
	else {
		/* call passed function, e.g. finalizer_thread */

		(function)();
	}

	DEBUGTHREADS("stopping", t);

	// Hook point just after the threads initial method returned.
	Hook::thread_end(t);

	/* We ignore the return value. */

	(void) thread_detach_current_thread();

	return NULL;
}


/* threads_impl_thread_start ***************************************************

   Start a thread in the JVM.  Both (vm internal and java) thread
   objects exist.

   IN:
      thread....the thread object
	  f.........function to run in the new thread. NULL means that the
	            "run" method of the object `t` should be called

******************************************************************************/

void threads_impl_thread_start(threadobject *thread, functionptr f)
{
	sem_t          sem;
	sem_t          sem_first;
	pthread_attr_t attr;
	startupinfo    startup;
	int            result;

	/* fill startupinfo structure passed by pthread_create to
	 * threads_startup_thread */

	startup.thread     = thread;
	startup.function   = f;              /* maybe we don't call Thread.run()V */
	startup.psem       = &sem;
	startup.psem_first = &sem_first;

	threads_sem_init(&sem, 0, 0);
	threads_sem_init(&sem_first, 0, 0);

	/* Initialize thread attributes. */

	result = pthread_attr_init(&attr);

	if (result != 0)
		os::abort_errnum(result, "threads_impl_thread_start: pthread_attr_init failed");

    result = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    if (result != 0)
		os::abort_errnum(result, "threads_impl_thread_start: pthread_attr_setdetachstate failed");

	/* initialize thread stacksize */

	result = pthread_attr_setstacksize(&attr, opt_stacksize);

	if (result != 0)
		os::abort_errnum(result, "threads_impl_thread_start: pthread_attr_setstacksize failed");

	/* create the thread */

	result = pthread_create(&(thread->impl.tid), &attr, threads_startup_thread, &startup);

	if (result != 0)
		os::abort_errnum(result, "threads_impl_thread_start: pthread_create failed");

	/* destroy the thread attributes */

	result = pthread_attr_destroy(&attr);

	if (result != 0)
		os::abort_errnum(result, "threads_impl_thread_start: pthread_attr_destroy failed");

	/* signal that pthread_create has returned, so thread->tid is valid */

	threads_sem_post(&sem_first);

	/* wait here until the thread has entered itself into the thread list */

	threads_sem_wait(&sem);

	/* cleanup */

	sem_destroy(&sem);
	sem_destroy(&sem_first);
}


/* threads_set_thread_priority *************************************************

   Set the priority of the given thread.

   IN:
      tid..........thread id
	  priority.....priority to set

******************************************************************************/

void threads_set_thread_priority(threadobject *t, int priority)
{
	threads_set_thread_priority(t->impl.tid, priority);
}


static void threads_set_thread_priority(pthread_t tid, int priority)
{
	struct sched_param schedp;
	int policy;

	pthread_getschedparam(tid, &policy, &schedp);
	schedp.sched_priority = priority;
	pthread_setschedparam(tid, policy, &schedp);
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

	/* XXX Care about exceptions? */
	(void) lock_monitor_enter(jlt.get_handle());
	
	lock_notify_all_object(jlt.get_handle());

	/* XXX Care about exceptions? */
	(void) lock_monitor_exit(jlt.get_handle());

	t->waitmutex->lock();
	t->impl.tid = 0;
	t->waitmutex->unlock();

	{
		MutexLocker lock(ThreadList::get()->mutex());

		/* Free the internal thread data-structure. */

		thread_free(t);

		/* Signal that this thread has finished and leave the mutex. */

		cond_join->signal();
	}

	t->suspendmutex->lock();
	t->suspendmutex->unlock();

	return true;
}


/**
 * Internal helper function which suspends the current thread. This is
 * the core method of the suspension mechanism actually blocking the
 * execution until the suspension reason is cleared again. Note that
 * the current thread needs to hold the suspension mutex while calling
 * this function.
 */
static void threads_suspend_self()
{
	threadobject* thread = THREADOBJECT;

	DEBUGTHREADS("suspending", thread);

	// Mark thread as suspended.
	assert(!thread->suspended);
	assert(thread->suspend_reason != SUSPEND_REASON_NONE);
	thread->suspended = true;

	// Acknowledge the suspension.
	thread->suspendcond->broadcast();

#if defined(ENABLE_GC_CACAO)
	// If we are stopping the world, we should send a global ack.
	if (thread->suspend_reason == SUSPEND_REASON_STOPWORLD)
		threads_sem_post(&suspend_ack);
#endif

	// Release the suspension mutex and wait till we are resumed.
	while (thread->suspend_reason != SUSPEND_REASON_NONE)
		thread->suspendcond->wait(thread->suspendmutex);

#if defined(ENABLE_GC_CACAO)
	// XXX This is propably not ok!
	// If we are starting the world, we should send a global ack.
	if (thread->suspend_reason == SUSPEND_REASON_STOPWORLD)
		threads_sem_post(&suspend_ack);
#endif

	// Mark thread as not suspended.
	assert(thread->suspended);
	assert(thread->suspend_reason == SUSPEND_REASON_NONE);
	thread->suspended = false;

	DEBUGTHREADS("resuming", thread);
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

	// Guard this with the suspension mutex.
	MutexLocker ml(*thread->suspendmutex);

	// Check if thread is already suspended.
	if (thread->suspended)
		return false;

	// Check if thread is in the process of suspending.
	if (thread->suspend_reason != SUSPEND_REASON_NONE)
		return false;

	// Set the reason for suspending the thread.
	thread->suspend_reason = reason;

	if (thread == THREADOBJECT) {
		// We already hold the suspension mutex and can suspend ourselves
		// immediately without using signals at all.
		threads_suspend_self();
	}
	else {
		// Send the suspend signal to the other thread.
		if (!thread->impl.tid)
			return false;
		if (pthread_kill(thread->impl.tid, SIGUSR1) != 0)
			os::abort_errno("threads_suspend_thread: pthread_kill failed");

		// Wait for the thread to acknowledge the suspension.
		while (!thread->suspended)
			thread->suspendcond->wait(thread->suspendmutex);
	}

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

	// Guard this with the suspension mutex.
	MutexLocker ml(*thread->suspendmutex);

	// Check if thread really is suspended.
	if (!thread->suspended)
		return false;

	// Threads can only be resumed for the same reason they were suspended.
	if (thread->suspend_reason != reason)
		return false;

	// Clear the reason for suspending the thread.
	thread->suspend_reason = SUSPEND_REASON_NONE;

	// Tell everyone that the thread should resume.
	thread->suspendcond->broadcast();

	return true;
}


/**
 * Acknowledges the suspension of the current thread.
 */
void threads_suspend_ack()
{
	threadobject* thread = THREADOBJECT;

	// Guard this with the suspension mutex.
	MutexLocker ml(*thread->suspendmutex);

	// Suspend ourselves while holding the suspension mutex.
	threads_suspend_self();
}


/* threads_join_all_threads ****************************************************

   Join all non-daemon threads.

*******************************************************************************/

void threads_join_all_threads(void)
{
	threadobject *t;

	/* get current thread */

	t = THREADOBJECT;

	/* This thread is waiting for all non-daemon threads to exit. */

	thread_set_state_waiting(t);

	/* enter join mutex */

	MutexLocker lock(ThreadList::get()->mutex());

	/* Wait for condition as long as we have non-daemon threads.  We
	   compare against 1 because the current (main thread) is also a
	   non-daemon thread. */

	while (ThreadList::get()->get_number_of_non_daemon_threads() > 1)
		ThreadList::get()->wait_cond(cond_join);
}


/* threads_timespec_earlier ****************************************************

   Return true if timespec tv1 is earlier than timespec tv2.

   IN:
      tv1..........first timespec
	  tv2..........second timespec

   RETURN VALUE:
      true, if the first timespec is earlier

*******************************************************************************/

static inline bool threads_timespec_earlier(const struct timespec *tv1,
											const struct timespec *tv2)
{
	return (tv1->tv_sec < tv2->tv_sec)
				||
		(tv1->tv_sec == tv2->tv_sec && tv1->tv_nsec < tv2->tv_nsec);
}


/* threads_current_time_is_earlier_than ****************************************

   Check if the current time is earlier than the given timespec.

   IN:
      tv...........the timespec to compare against

   RETURN VALUE:
      true, if the current time is earlier

*******************************************************************************/

static bool threads_current_time_is_earlier_than(const struct timespec *tv)
{
	struct timeval tvnow;
	struct timespec tsnow;

	/* get current time */

	if (gettimeofday(&tvnow, NULL) != 0)
		vm_abort("gettimeofday failed: %s\n", strerror(errno));

	/* convert it to a timespec */

	tsnow.tv_sec = tvnow.tv_sec;
	tsnow.tv_nsec = tvnow.tv_usec * 1000;

	/* compare current time with the given timespec */

	return threads_timespec_earlier(&tsnow, tv);
}


/* threads_wait_with_timeout ***************************************************

   Wait until the given point in time on a monitor until either
   we are notified, we are interrupted, or the time is up.

   IN:
      t............the current thread
	  wakeupTime...absolute (latest) wakeup time
	                   If both tv_sec and tv_nsec are zero, this function
					   waits for an unlimited amount of time.

*******************************************************************************/

static void threads_wait_with_timeout(threadobject *t, struct timespec *wakeupTime, bool parking)
{
	// Acquire the waitmutex.
	t->waitmutex->lock();

	/* wait on waitcond */

	if (wakeupTime->tv_sec || wakeupTime->tv_nsec) {
		/* with timeout */
		while (!t->interrupted && !(parking ? t->park_permit : t->signaled)
			   && threads_current_time_is_earlier_than(wakeupTime))
		{
			if (parking)
				thread_set_state_timed_parked(t);
			else
				thread_set_state_timed_waiting(t);

			t->waitcond->timedwait(t->waitmutex, wakeupTime);

			thread_set_state_runnable(t);
		}
	}
	else {
		/* no timeout */
		while (!t->interrupted && !(parking ? t->park_permit : t->signaled)) {
			if (parking)
				thread_set_state_parked(t);
			else
				thread_set_state_waiting(t);

			t->waitcond->wait(t->waitmutex);

			thread_set_state_runnable(t);
		}
	}

	if (parking)
		t->park_permit = false;

	// Release the waitmutex.
	t->waitmutex->unlock();
}


/* threads_wait_with_timeout_relative ******************************************

   Wait for the given maximum amount of time on a monitor until either
   we are notified, we are interrupted, or the time is up.

   IN:
      t............the current thread
	  millis.......milliseconds to wait
	  nanos........nanoseconds to wait

*******************************************************************************/

void threads_wait_with_timeout_relative(threadobject *thread, s8 millis,
										s4 nanos)
{
	struct timespec wakeupTime;

	/* calculate the the (latest) wakeup time */

	threads_calc_absolute_time(&wakeupTime, millis, nanos);

	/* wait */

	threads_wait_with_timeout(thread, &wakeupTime, false);
}


/* threads_calc_absolute_time **************************************************

   Calculate the absolute point in time a given number of ms and ns from now.

   IN:
      millis............milliseconds from now
	  nanos.............nanoseconds from now

   OUT:
      *tm...............receives the timespec of the absolute point in time

*******************************************************************************/

static void threads_calc_absolute_time(struct timespec *tm, s8 millis, s4 nanos)
{
	// (at least with GNU classpath) we know that 0 <= nanos <= 999999
	do {
		if (!millis && !nanos)
			break;
		struct timeval tv;
		gettimeofday(&tv, NULL);
		s8 secs = tv.tv_sec + millis / 1000;
		if (secs > INT32_MAX)	// integer overflow
			break;
		tv.tv_sec = secs;
		millis %= 1000;
		long nsec = tv.tv_usec * 1000 + (s4) millis * 1000000 + nanos;
		tm->tv_sec = tv.tv_sec + nsec / 1000000000;
		if (tm->tv_sec < tv.tv_sec) // integer overflow
			break;
		tm->tv_nsec = nsec % 1000000000;
		return;
	} while (0);
	tm->tv_sec = 0;
	tm->tv_nsec = 0;
}


/* threads_thread_interrupt ****************************************************

   Interrupt the given thread.

   The thread gets the "waitcond" signal and 
   its interrupted flag is set to true.

   IN:
      thread............the thread to interrupt

*******************************************************************************/

void threads_thread_interrupt(threadobject *t)
{
	/* Signal the thread a "waitcond" and tell it that it has been
	   interrupted. */

	t->waitmutex->lock();

	DEBUGTHREADS("interrupted", t);

	/* Interrupt blocking system call using a signal. */

	if (t->impl.tid)
		pthread_kill(t->impl.tid, Signal_INTERRUPT_SYSTEM_CALL);

	t->waitcond->signal();

	t->interrupted = true;

	t->waitmutex->unlock();
}


/**
 * Sleep the current thread for the specified amount of time.
 *
 * @param millis Milliseconds to sleep.
 * @param nanos  Nanoseconds to sleep.
 */
void threads_sleep(int64_t millis, int32_t nanos)
{
	threadobject    *t;
	struct timespec  wakeupTime;
	bool             interrupted;

	if (millis < 0) {
/* 		exceptions_throw_illegalargumentexception(); */
		exceptions_throw_illegalargumentexception();
		return;
	}

	t = thread_get_current();

	if (thread_is_interrupted(t) && !exceptions_get_exception()) {
		/* Clear interrupted flag (Mauve test:
		   gnu/testlet/java/lang/Thread/interrupt). */

		thread_set_interrupted(t, false);

/* 		exceptions_throw_interruptedexception("sleep interrupted"); */
		exceptions_throw_interruptedexception();
		return;
	}

	// (Note taken from classpath/vm/reference/java/lang/VMThread.java (sleep))
	// Note: JDK treats a zero length sleep is like Thread.yield(),
	// without checking the interrupted status of the thread.  It's
	// unclear if this is a bug in the implementation or the spec.
	// See http://bugs.sun.com/bugdatabase/view_bug.do?bug_id=6213203 */
	if (millis == 0 && nanos == 0) {
		threads_yield();
	}
	else {
		threads_calc_absolute_time(&wakeupTime, millis, nanos);

		threads_wait_with_timeout(t, &wakeupTime, false);

		interrupted = thread_is_interrupted(t);

		if (interrupted) {
			thread_set_interrupted(t, false);

			// An other exception could have been thrown
			// (e.g. ThreadDeathException).
			if (!exceptions_get_exception())
				exceptions_throw_interruptedexception();
		}
	}
}

/**
 * Park the current thread for the specified amount of time or until a
 * specified deadline.
 *
 * @param absolute Is the time in nanos a deadline or a duration?
 * @param nanos    Nanoseconds to park (absolute=false)
 *                 or deadline in milliseconds (absolute=true)
 */
void threads_park(bool absolute, int64_t nanos)
{
	threadobject    *t;
	struct timespec  wakeupTime;

	t = thread_get_current();

	if (absolute) {
		wakeupTime.tv_nsec = 0;
		wakeupTime.tv_sec = nanos / 1000; /* milliseconds */
	}
	else
		threads_calc_absolute_time(&wakeupTime, nanos / 1000000, nanos % 1000000);

	threads_wait_with_timeout(t, &wakeupTime, true);
}

/**
 * Unpark the specified thread.
 *
 * @param t The thread to unpark.
 */
void threads_unpark(threadobject *t)
{
	t->waitmutex->lock();

	t->waitcond->signal();

	t->park_permit = true;

	t->waitmutex->unlock();
}


/* threads_yield ***************************************************************

   Yield to the scheduler.

*******************************************************************************/

void threads_yield(void)
{
	sched_yield();
}


/***
 * Return the tid of a thread.
 */
intptr_t threads_get_tid(threadobject *t) {
	// pthread_t is a pointer type in OS X 
	// and a unsigned integer on linux.
	// Thus we need different casts
#ifdef __DARWIN__
	return reinterpret_cast<intptr_t>(t->impl.tid);
#else
	return static_cast<intptr_t>(t->impl.tid);
#endif
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
