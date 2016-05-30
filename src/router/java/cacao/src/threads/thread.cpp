/* src/threads/thread.cpp - machine independent thread functions

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
#include <assert.h>                     // for assert
#include <inttypes.h> 
#include <stddef.h>                     // for NULL, size_t
#include <stdint.h>                     // for int32_t, intptr_t
#include <string.h>                     // for strstr
#include <unistd.h>                     // for _CS_GNU_LIBPTHREAD_VERSION, etc
#include "config.h"                     // for ENABLE_GC_BOEHM, etc
#include "lockword.hpp"                 // for Lockword
#include "mm/dumpmemory.hpp"            // for DumpMemory
#include "mm/gc.hpp"                    // for gc_register_current_thread, etc
#include "mm/memory.hpp"                // for GCNEW_UNCOLLECTABLE, MNEW, etc
#include "native/llni.hpp"              // for LLNI_WRAP, LLNI_DIRECT
#include "threads/atomic.hpp"           // for write_memory_barrier
#include "threads/condition.hpp"        // for Condition
#include "threads/mutex.hpp"            // for Mutex
#include "threads/threadlist.hpp"       // for ThreadList
#include "threads/ThreadRuntime.hpp"    // for ThreadRuntime
#include "toolbox/logging.hpp"
#include "vm/finalizer.hpp"             // for Finalizer
#include "vm/globals.hpp"               // for class_java_lang_Thread
#include "vm/javaobjects.hpp"           // for java_lang_Thread
#include "vm/jit/builtin.hpp"           // for builtin_new
#include "vm/options.hpp"
#include "vm/string.hpp"                // for JavaString
#include "vm/types.hpp"                 // for ptrint, u4
#include "vm/utf8.hpp"                  // for Utf8String
#include "vm/vm.hpp"                    // for vm_abort

STAT_DECLARE_VAR(int,size_threadobject,0)

struct methodinfo;

using namespace cacao;

/* global variables ***********************************************************/

static methodinfo    *thread_method_init;
static java_handle_t *threadgroup_system;
static java_handle_t *threadgroup_main;

#if defined(__LINUX__)
/* XXX Remove for exact-GC. */
bool threads_pthreads_implementation_nptl;
#endif


/* static functions ***********************************************************/

static void          thread_create_initial_thread(void);
static threadobject *thread_new(int32_t flags);


/* threads_preinit *************************************************************

   Do some early initialization of stuff required.

*******************************************************************************/

void threads_preinit(void)
{
	threadobject *mainthread;
#if defined(__LINUX__) && defined(_CS_GNU_LIBPTHREAD_VERSION)
	char         *pathbuf;
	size_t        len;
#endif

	TRACESUBSYSTEMINITIALIZATION("threads_preinit");

#if defined(__LINUX__)
	/* XXX Remove for exact-GC. */

	/* On Linux we need to check the pthread implementation. */

	/* _CS_GNU_LIBPTHREAD_VERSION (GNU C library only; since glibc 2.3.2) */
	/* If the glibc is a pre-2.3.2 version, we fall back to
	   linuxthreads. */

# if defined(_CS_GNU_LIBPTHREAD_VERSION)
	len = confstr(_CS_GNU_LIBPTHREAD_VERSION, NULL, (size_t) 0);

	/* Some systems return as length 0 (maybe cross-compilation
	   related).  In this case we also fall back to linuxthreads. */

	if (len > 0) {
		pathbuf = MNEW(char, len);

		(void) confstr(_CS_GNU_LIBPTHREAD_VERSION, pathbuf, len);

		if (strstr(pathbuf, "NPTL") != NULL)
			threads_pthreads_implementation_nptl = true;
		else
			threads_pthreads_implementation_nptl = false;
	}
	else
		threads_pthreads_implementation_nptl = false;
# else
	threads_pthreads_implementation_nptl = false;
# endif
#endif

	/* Create the single threadlist instance. */
	ThreadList::create_object();

	/* Initialize the threads implementation (sets the thinlock on the
	   main thread). */

	threads_impl_preinit();

	/* Create internal thread data-structure for the main thread. */

	mainthread = thread_new(THREAD_FLAG_JAVA);

	/* Add the thread to the thread list. */

	ThreadList::get()->add_to_active_thread_list(mainthread);

	/* The main thread should always have index 1. */

	if (mainthread->index != 1)
		vm_abort("threads_preinit: main thread index not 1: %d != 1",
				 mainthread->index);

	/* Thread is already running. */

	mainthread->state = THREAD_STATE_RUNNABLE;

	/* Store the internal thread data-structure in the TSD. */

	thread_set_current(mainthread);
}


/* threads_init ****************************************************************

   Initialize the main thread.

*******************************************************************************/

void threads_init(void)
{
	TRACESUBSYSTEMINITIALIZATION("threads_init");

	/* Create the system and main thread groups. */

	ThreadRuntime::thread_create_initial_threadgroups(&threadgroup_main, &threadgroup_system);

	/* Cache the java.lang.Thread initialization method. */

	thread_method_init = ThreadRuntime::get_thread_init_method();

	if (thread_method_init == NULL)
		vm_abort("threads_init: failed to resolve thread init method");

	thread_create_initial_thread();
}


/* thread_create_object ********************************************************

   Create a Java thread object for the given thread data-structure,
   initializes it and adds the thread to the threadgroup.

   ARGUMENTS:

       t ....... thread
       name .... thread name
       group ... threadgroup

   RETURN:

*******************************************************************************/

static bool thread_create_object(threadobject *t, java_handle_t *name, java_handle_t *group)
{
	/* Create a java.lang.Thread Java object. */

	java_handle_t* h = builtin_new(class_java_lang_Thread);

	if (h == NULL)
		return false;

	java_lang_Thread jlt(h);

	// Set the Java object in the thread data-structure.  This
	// indicates that the thread is attached to the VM.
	t->object = LLNI_DIRECT(jlt.get_handle());

	return ThreadRuntime::invoke_thread_initializer(jlt, t, thread_method_init, name, group);
}


/* thread_create_initial_thread ***********************************************

   Create the initial thread: main

*******************************************************************************/

static void thread_create_initial_thread(void)
{
	threadobject  *t;
	java_handle_t *name;

	/* Get the main-thread (NOTE: The main thread is always the first
	   thread in the list). */

	t = ThreadList::get()->get_main_thread();

	/* The thread name. */

	name = JavaString::from_utf8(utf8::main);

#if defined(ENABLE_INTRP)
	/* create interpreter stack */

	if (opt_intrp) {
		MSET(intrp_main_stack, 0, u1, opt_stacksize);
		mainthread->_global_sp = (Cell*) (intrp_main_stack + opt_stacksize);
	}
#endif

	/* Create the Java thread object. */

	if (!thread_create_object(t, name, threadgroup_main))
		vm_abort("thread_create_initial_thread: failed to create Java object");

	/* Initialize the implementation specific bits. */

	threads_impl_init();

	DEBUGTHREADS("starting (main)", t);
}


/* thread_new ******************************************************************

   Allocates and initializes an internal thread data-structure and
   adds it to the threads list.

*******************************************************************************/

static threadobject *thread_new(int32_t flags)
{
	threadobject *t;
	int32_t       index;

	/* Allocate a thread data structure. */

	/* First, try to get one from the free-list. */
	ThreadList::get()->get_free_thread(&t, &index);

	if (t != NULL) {
		/* Equivalent of MZERO on the else path */

		threads_impl_thread_clear(t);
	} else {
#ifdef ENABLE_GC_BOEHM
		t = GCNEW_UNCOLLECTABLE(threadobject, 1);
#else
		t = NEW(threadobject);
#endif

		STATISTICS(size_threadobject += sizeof(threadobject));

		/* Clear memory. */

		MZERO(t, threadobject, 1);

		// Initialize the mutex and the condition.
		t->flc_lock = new Mutex();
		t->flc_cond = new Condition();

		t->waitmutex = new Mutex();
		t->waitcond = new Condition();

		t->suspendmutex = new Mutex();
		t->suspendcond = new Condition();

#ifdef ENABLE_TLH
		tlh_init(&(t->tlh));
#endif

#ifdef ENABLE_GC_CACAO
		/* Register reference to java.lang.Thread with the GC. */
		/* FIXME is it ok to do this only once? */

		gc_reference_register(&(t->object), GC_REFTYPE_THREADOBJECT);
		gc_reference_register(&(t->_exceptionptr), GC_REFTYPE_THREADOBJECT);
#endif

		t->_dumpmemory = new DumpMemory();
	}

	/* Pre-compute the thinlock-word. */

	assert(index != 0);

	t->index     = index;
	t->thinlock  = Lockword::pre_compute_thinlock(t->index);
	t->flags     = flags;
	t->state     = THREAD_STATE_NEW;

#ifdef ENABLE_GC_CACAO
	t->flags    |= THREAD_FLAG_IN_NATIVE; 
#endif

#ifdef ENABLE_DEBUG_FILTER
	// Initialize filter counters
	t->filterverbosecallctr[0] = 0;
	t->filterverbosecallctr[1] = 0;
#endif

#ifndef NDEBUG
	t->tracejavacallindent = 0;
	t->tracejavacallcount = 0;
#endif

	t->flc_bit    = false;
	t->flc_next   = NULL;
	t->flc_list   = NULL;
	t->flc_object = NULL; // not really needed

#ifdef ENABLE_TLH
	tlh_destroy(&(t->tlh));
	tlh_init(&(t->tlh));
#endif

	/* Initialize the implementation-specific bits. */

	threads_impl_thread_reuse(t);

	return t;
}


/* thread_free *****************************************************************

   Remove the thread from the threads-list and free the internal
   thread data structure.  The thread index is added to the
   thread-index free-list.

   IN:
       t ... thread data structure

*******************************************************************************/

void thread_free(threadobject *t)
{
	java_handle_t *h = LLNI_WRAP(t->object);
	java_lang_Thread jlt(h);
	ThreadRuntime::clear_heap_reference(jlt);

	/* Set the reference to the Java object to NULL. */

	t->object = 0;

	ThreadList::get()->deactivate_thread(t);
}


/* threads_thread_start_internal ***********************************************

   Start an internal thread in the JVM.  No Java thread objects exists
   so far.

   IN:
      name.......UTF-8 name of the thread
      f..........function pointer to C function to start

*******************************************************************************/

static void thread_cleanup_finalizer(java_handle_t *h, void *data)
{
	threadobject *t = reinterpret_cast<threadobject*>(data);
	ThreadList::get()->release_thread(t, false);
}

bool threads_thread_start_internal(Utf8String name, functionptr f)
{
	threadobject *t;

	/* Create internal thread data-structure. */

	t = thread_new(THREAD_FLAG_INTERNAL | THREAD_FLAG_DAEMON);

	/* Add the thread to the thread list. */

	ThreadList::get()->add_to_active_thread_list(t);

	/* Create the Java thread object. */

	if (!thread_create_object(t, JavaString::from_utf8(name), threadgroup_system)) {
		ThreadList::get()->release_thread(t, true);
		return false;
	}

#if defined(ENABLE_GC_BOEHM)
	Finalizer::attach_custom_finalizer(LLNI_WRAP(t->object), thread_cleanup_finalizer, t);
#endif

	/* Start the thread. */

	threads_impl_thread_start(t, f);

	/* everything's ok */

	return true;
}


/* threads_thread_start ********************************************************

   Start a Java thread in the JVM.  Only the java thread object exists
   so far.

   IN:
      object.....the java thread object java.lang.Thread

*******************************************************************************/

void threads_thread_start(java_handle_t *object)
{
	java_lang_Thread jlt(object);

	/* Create internal thread data-structure. */

	u4 flags = THREAD_FLAG_JAVA;
#if defined(ENABLE_JAVASE)
	/* Is this a daemon thread? */

	if (jlt.get_daemon())
		flags |= THREAD_FLAG_DAEMON;
#endif

	threadobject* t = thread_new(flags);

	/* Link the two objects together. */

	t->object = LLNI_DIRECT(object);

	/* Add the thread to the thread list. */

	ThreadList::get()->add_to_active_thread_list(t);

	Atomic::write_memory_barrier();

	ThreadRuntime::setup_thread_vmdata(jlt, t);

#if defined(ENABLE_GC_BOEHM)
	Finalizer::attach_custom_finalizer(LLNI_WRAP(t->object), thread_cleanup_finalizer, t);
#endif

	thread_set_state_runnable(t);

	/* Start the thread.  Don't pass a function pointer (NULL) since
	   we want Thread.run()V here. */

	threads_impl_thread_start(t, NULL);
}


/**
 * Attaches the current thread to the VM.
 *
 * @param vm_aargs Attach arguments.
 * @param isdaemon true if the attached thread should be a daemon
 *                 thread.
 *
 * @return true on success, false otherwise.
 */
bool thread_attach_current_thread(JavaVMAttachArgs *vm_aargs, bool isdaemon)
{
	bool           result;
	threadobject  *t;
	java_handle_t *name;
	java_handle_t *group;

    /* If the current thread has already been attached, this operation
	   is a no-op. */

	result = thread_current_is_attached();

	if (result == true)
		return true;

	/* Create internal thread data structure. */

	u4 flags = THREAD_FLAG_JAVA;
	if (isdaemon)
		flags |= THREAD_FLAG_DAEMON;

	t = thread_new(flags);

	/* Store the internal thread data-structure in the TSD. */

	thread_set_current(t);

	/* The thread is flagged and (non-)daemon thread, we can leave the
	   mutex. */

	/* Add the thread to the thread list. */

	ThreadList::get()->add_to_active_thread_list(t);

	DEBUGTHREADS("attaching", t);

	/* Get the thread name. */

	name = vm_aargs ? JavaString::from_utf8(vm_aargs->name)
	                : JavaString::from_utf8(utf8::null);

#if defined(ENABLE_JAVASE)
	/* Get the threadgroup. */

	if (vm_aargs != NULL)
		group = (java_handle_t *) vm_aargs->group;
	else
		group = NULL;

	/* If no threadgroup was given, use the main threadgroup. */

	if (group == NULL)
		group = threadgroup_main;
#endif

#if defined(ENABLE_INTRP)
	/* create interpreter stack */

	if (opt_intrp) {
		MSET(intrp_main_stack, 0, u1, opt_stacksize);
		thread->_global_sp = (Cell *) (intrp_main_stack + opt_stacksize);
	}
#endif

	/* Create the Java thread object. */

	if (!thread_create_object(t, name, group)) {
		ThreadList::get()->release_thread(t, true);
		return false;
	}

	/* The thread is completely initialized. */

	thread_set_state_runnable(t);

	return true;
}


/**
 * Attaches the current external thread to the VM.  This function is
 * called by JNI's AttachCurrentThread.
 *
 * @param vm_aargs Attach arguments.
 * @param isdaemon true if the attached thread should be a daemon
 *                 thread.
 *
 * @return true on success, false otherwise.
 */
bool thread_attach_current_external_thread(JavaVMAttachArgs *vm_aargs, bool isdaemon)
{
	gc_register_current_thread();

	if (thread_attach_current_thread(vm_aargs, isdaemon) == false) {
		gc_unregister_current_thread();
		return false;
	}

	return true;
}


/**
 * Detaches the current external thread from the VM.  This function is
 * called by JNI's DetachCurrentThread.
 *
 * @return true on success, false otherwise.
 */
bool thread_detach_current_external_thread(void)
{
	if (thread_detach_current_thread() == false)
		return false;

	// Unregister the thread with GC.
	// This must happen after the thread allocates any memory from the GC heap.
	// NOTE: Don't detach the main thread.  
	//       This is a workaround for OpenJDK's java launcher.

	if (thread_get_current()->index != 1)
		gc_unregister_current_thread();

	return true;
}


/* thread_fprint_name **********************************************************

   Print the name of the given thread to the given stream.

   ARGUMENTS:
       t ........ thread data-structure
       stream ... stream to print to

*******************************************************************************/

void thread_fprint_name(threadobject *t, FILE *stream)
{
	if (LLNI_WRAP(t->object) == NULL)
		vm_abort("");

	java_lang_Thread jlt(LLNI_WRAP(t->object));

	ThreadRuntime::print_thread_name(jlt, stream);
}


/* thread_print_info ***********************************************************

   Print information of the passed thread.

   ARGUMENTS:
       t ... thread data-structure.

*******************************************************************************/

void thread_print_info(threadobject *t)
{
	java_lang_Thread jlt(LLNI_WRAP(t->object));

	/* Print as much as we can when we are in state NEW. */

	if (jlt.get_handle() != NULL) {
		/* Print thread name. */

		printf("\"");
		thread_fprint_name(t, stdout);
		printf("\"");
	}
	else {
	}

	if (thread_is_daemon(t))
		printf(" daemon");

	if (jlt.get_handle() != NULL) {
		printf(" prio=%d", jlt.get_priority());
	}

	ptrint tid = threads_get_tid(t);

	printf(" t=0x%"PRIxPTR" tid=0x%"PRIxPTR" (%"PRIdPTR")", (uintptr_t) t, tid, tid);
	printf(" index=%d", t->index);

	/* Print thread state. */

	int state = cacaothread_get_state(t);

	switch (state) {
	case THREAD_STATE_NEW:
		printf(" new");
		break;
	case THREAD_STATE_RUNNABLE:
		printf(" runnable");
		break;
	case THREAD_STATE_BLOCKED:
		printf(" blocked");
		break;
	case THREAD_STATE_WAITING:
		printf(" waiting");
		break;
	case THREAD_STATE_TIMED_WAITING:
		printf(" waiting on condition");
		break;
	case THREAD_STATE_PARKED:
		printf(" parked");
		break;
	case THREAD_STATE_TIMED_PARKED:
		printf(" timed parked");
		break;
	case THREAD_STATE_TERMINATED:
		printf(" terminated");
		break;
	default:
		vm_abort("thread_print_info: unknown thread state %d", state);
	}
}


/* threads_get_current_tid *****************************************************

   Return the tid of the current thread.
   
   RETURN VALUE:
       the current tid

*******************************************************************************/

intptr_t threads_get_current_tid(void)
{
	threadobject *t = thread_get_current();

	return t ? threads_get_tid(t) : 0;
}


/**
 * Set the current state of the given thread. This method should only
 * be called while holding the threadlist-lock and after checking that
 * the new state is valid. It is best to not call this method directly
 * but call the specific setter methods below.
 */
static inline void thread_set_state(threadobject *t, ThreadState state)
{
	// Set the state of our internal threadobject.
	t->state = state;

	ThreadRuntime::set_javathread_state(t, state);
}


/* thread_set_state_runnable ***************************************************

   Set the current state of the given thread to THREAD_STATE_RUNNABLE.

   NOTE: If the thread has already terminated, don't set the state.
         This is important for threads_detach_thread.

*******************************************************************************/

void thread_set_state_runnable(threadobject *t)
{
	if (t->state != THREAD_STATE_TERMINATED) {
		thread_set_state(t, THREAD_STATE_RUNNABLE);

		DEBUGTHREADS("is RUNNABLE", t);
	}
}


/* thread_set_state_waiting ****************************************************

   Set the current state of the given thread to THREAD_STATE_WAITING.

   NOTE: If the thread has already terminated, don't set the state.
         This is important for threads_detach_thread.

*******************************************************************************/

void thread_set_state_waiting(threadobject *t)
{
	if (t->state != THREAD_STATE_TERMINATED) {
		thread_set_state(t, THREAD_STATE_WAITING);

		DEBUGTHREADS("is WAITING", t);
	}
}


/* thread_set_state_timed_waiting **********************************************

   Set the current state of the given thread to
   THREAD_STATE_TIMED_WAITING.

   NOTE: If the thread has already terminated, don't set the state.
         This is important for threads_detach_thread.

*******************************************************************************/

void thread_set_state_timed_waiting(threadobject *t)
{
	if (t->state != THREAD_STATE_TERMINATED) {
		thread_set_state(t, THREAD_STATE_TIMED_WAITING);

		DEBUGTHREADS("is TIMED_WAITING", t);
	}
}


/* thread_set_state_parked *****************************************************

   Set the current state of the given thread to THREAD_STATE_PARKED.

   NOTE: If the thread has already terminated, don't set the state.
         This is important for threads_detach_thread.

*******************************************************************************/

void thread_set_state_parked(threadobject *t)
{
	if (t->state != THREAD_STATE_TERMINATED) {
		thread_set_state(t, THREAD_STATE_PARKED);

		DEBUGTHREADS("is PARKED", t);
	}
}


/* thread_set_state_timed_parked ***********************************************

   Set the current state of the given thread to THREAD_STATE_TIMED_PARKED.

   NOTE: If the thread has already terminated, don't set the state.
         This is important for threads_detach_thread.

*******************************************************************************/

void thread_set_state_timed_parked(threadobject *t)
{
	if (t->state != THREAD_STATE_TERMINATED) {
		thread_set_state(t, THREAD_STATE_TIMED_PARKED);

		DEBUGTHREADS("is TIMED_PARKED", t);
	}
}


/* thread_set_state_terminated *************************************************

   Set the current state of the given thread to
   THREAD_STATE_TERMINATED.

*******************************************************************************/

void thread_set_state_terminated(threadobject *t)
{
	/* Set the state inside a lock. */

	thread_set_state(t, THREAD_STATE_TERMINATED);

	DEBUGTHREADS("is TERMINATED", t);
}


/* thread_get_thread **********************************************************

   Return the thread data structure of the given Java thread object.

   ARGUMENTS:
       h ... java.lang.{VM}Thread object

   RETURN VALUE:
       the thread object

   NOTE:
       Usage of this function without the thread list lock held is
       almost certainly a bug.

*******************************************************************************/

threadobject *thread_get_thread(java_handle_t *h)
{
	return ThreadRuntime::get_thread_from_object(h);
}


/* threads_thread_is_alive *****************************************************

   Returns if the give thread is alive.

*******************************************************************************/

bool threads_thread_is_alive(threadobject *t)
{
	int state;

	state = cacaothread_get_state(t);

	switch (state) {
	case THREAD_STATE_NEW:
	case THREAD_STATE_TERMINATED:
		return false;

	case THREAD_STATE_RUNNABLE:
	case THREAD_STATE_BLOCKED:
	case THREAD_STATE_WAITING:
	case THREAD_STATE_TIMED_WAITING:
	case THREAD_STATE_PARKED:
	case THREAD_STATE_TIMED_PARKED:
		return true;

	default:
		vm_abort("threads_thread_is_alive: unknown thread state %d", state);
	}

	/* keep compiler happy */

	return false;
}

/* thread_is_interrupted *******************************************************

   Check if the given thread has been interrupted.

   ARGUMENTS:
       t ... the thread to check

   RETURN VALUE:
      true, if the given thread had been interrupted

*******************************************************************************/

bool thread_is_interrupted(threadobject *t)
{
	/* We need the mutex because classpath will call this function when
	   a blocking system call is interrupted. The mutex ensures that it will
	   see the correct value for the interrupted flag. */

	t->waitmutex->lock();
	bool interrupted = t->interrupted;
	t->waitmutex->unlock();

	return interrupted;
}


/* thread_set_interrupted ******************************************************

   Set the interrupted flag to the given value.

   ARGUMENTS:
       interrupted ... value to set

*******************************************************************************/

void thread_set_interrupted(threadobject *t, bool interrupted)
{
	t->waitmutex->lock();
	t->interrupted = interrupted;
	t->waitmutex->unlock();
}

/* thread_handle_set_priority **************************************************

   Calls threads_set_thread_priority for the threadobject associated
   with the thread indicated by handle th, while holding the thread
   list lock.

*******************************************************************************/

void thread_handle_set_priority(java_handle_t *th, int priority)
{
	threadobject *t = thread_get_thread(th);
	/* For GNU classpath, this should not happen, because both
	   setPriority() and start() are synchronized. */
	assert(t != 0);
	threads_set_thread_priority(t, priority);
}

/* thread_handle_is_interrupted ************************************************

   Calls thread_is_interrupted for the threadobject associated with
   the thread indicated by handle th, while holding the thread list
   lock.

*******************************************************************************/

bool thread_handle_is_interrupted(java_handle_t *th)
{
	threadobject *t = thread_get_thread(th);
	return t ? thread_is_interrupted(t) : false;
}

/* thread_handle_interrupt *****************************************************

   Calls threads_thread_interrupt for the threadobject associated with
   the thread indicated by handle th, while holding the thread list
   lock.

*******************************************************************************/

void thread_handle_interrupt(java_handle_t *th)
{
	threadobject *t = thread_get_thread(th);
	/* For GNU classpath, this should not happen, because both
	   interrupt() and start() are synchronized. */
	assert(t != 0);
	threads_thread_interrupt(t);
}

/* thread_handle_get_state *****************************************************

   Calls cacaothread_get_state for the threadobject associated with
   the thread indicated by handle th, while holding the thread list
   lock.

*******************************************************************************/

int thread_handle_get_state(java_handle_t *th)
{
	threadobject *t = thread_get_thread(th);
	return t ? cacaothread_get_state(t) : THREAD_STATE_NEW;
}


#if defined(ENABLE_TLH)

void threads_tlh_add_frame() {
	tlh_add_frame(&(THREADOBJECT->tlh));
}

void threads_tlh_remove_frame() {
	tlh_remove_frame(&(THREADOBJECT->tlh));
}

#endif


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
