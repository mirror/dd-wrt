/* src/vm/finalizer.cpp - finalizer linked list and thread

   Copyright (C) 1996-2014
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

#include "finalizer.hpp"
#include "config.h"                     // for ENABLE_THREADS, etc
#include <assert.h>                     // for assert
#include <stdlib.h>                     // for NULL
#include <map>                          // for multimap, _Rb_tree_iterator, etc
#include <utility>                      // for pair, make_pair
#include "mm/gc.hpp"                    // for gc_invoke_finalizers
#include "native/llni.hpp"              // for LLNI_DIRECT, LLNI_class_get
#include "threads/condition.hpp"        // for Condition
#include "threads/mutex.hpp"            // for Mutex, MutexLocker
#include "threads/thread.hpp"
#include "toolbox/OStream.hpp"          // for OStream, nl
#include "toolbox/logging.hpp"          // for LOG
#include "utf8.hpp"                     // for Utf8String
#include "vm/class.hpp"                 // for operator<<, classinfo
#include "vm/exceptions.hpp"            // for exceptions_clear_exception, etc
#include "vm/global.hpp"                // for java_handle_t
#include "vm/options.hpp"
#include "vm/vm.hpp"                    // for vm_call_method

#if defined(ENABLE_GC_BOEHM)
# include "mm/boehm-gc/include/gc.h"
#endif

#define DEBUG_NAME "finalizer"

/* global variables ***********************************************************/

/***
 * Simple synchronization helper for coordinating sets of finalizer runs and
 * requests from concurrent threads. It assumes that there is only a single
 * finalizer thread.
 */
class FinalizerThreadCoordinator {
	Mutex mutex;
	Condition cond; // finalizer thread blocks on this cv
	Condition *shutdown; // thread requesting shutdown blocks on this one
	bool running;   // thread requested to run finalizers
	bool pending;   // thread should continue running finalizers
	bool disabled;  // thread has been requested to stop

public:
	FinalizerThreadCoordinator(): shutdown(0), running(false), pending(false), disabled(false) { }

	// Called by finalizer thread
	bool accept();
	void done();
	void shutdown_ack();

	// Called by requestor threads
	void request();
	void join();
};
static FinalizerThreadCoordinator *finalizer_thread_coord;

void FinalizerThreadCoordinator::request()
{
	MutexLocker lock(mutex);
	if (running)
		pending = true;
	else {
		running = true;
		cond.signal();
	}
}

bool FinalizerThreadCoordinator::accept()
{
	MutexLocker lock(mutex);
	while (!running)
		cond.wait(&mutex);
	return !shutdown;
}

void FinalizerThreadCoordinator::done()
{
	MutexLocker lock(mutex);
	running = pending;
	pending = false;
}

void FinalizerThreadCoordinator::shutdown_ack()
{
	MutexLocker lock(mutex);
	shutdown->signal();
	shutdown = 0;
}

void FinalizerThreadCoordinator::join()
{
	Condition cond_shutdown;
	MutexLocker lock(mutex);
	if (disabled)
		// deliberately hang forever
		for (;;)
			cond_shutdown.wait(&mutex);
	running = true;
	pending = true;
	disabled = true;
	shutdown = &cond_shutdown;
	cond.signal();
	while (shutdown)
		cond_shutdown.wait(&mutex);
}

/* finalizer_init **************************************************************

   Initializes the finalizer global lock and the linked list.

*******************************************************************************/

bool finalizer_init()
{
	TRACESUBSYSTEMINITIALIZATION("finalizer_init");

	finalizer_thread_coord = new FinalizerThreadCoordinator;

	/* everything's ok */

	return true;
}


/* finalizer_thread ************************************************************

   This thread waits on an object for a notification and the runs the
   finalizers (finalizer thread).  This is necessary because of a
   possible deadlock in the GC.

*******************************************************************************/

static void finalizer_thread()
{
	while (true) {
		/* get the lock on the finalizer mutex, so we can call wait */

		if (!finalizer_thread_coord->accept()) {
			finalizer_thread_coord->shutdown_ack();
			break;
		}
		LOG("[finalizer thread    : status=awake]" << cacao::nl);

		/* and call the finalizers */

		gc_invoke_finalizers();

		LOG("[finalizer thread    : status=sleeping]" << cacao::nl);
		finalizer_thread_coord->done();
	}
}


/* finalizer_start_thread ******************************************************

   Starts the finalizer thread.

*******************************************************************************/

bool finalizer_start_thread()
{
	Utf8String name = Utf8String::from_utf8("Finalizer");

	if (!threads_thread_start_internal(name, finalizer_thread))
		return false;

	/* everything's ok */

	return true;
}

void finalizer_join_thread()
{
	finalizer_thread_coord->join();
}

/* finalizer_notify ************************************************************

   Notifies the finalizer thread that it should run the
   gc_invoke_finalizers from the GC.

*******************************************************************************/

void finalizer_notify()
{
	LOG("[finalizer notified]" << cacao::nl);

#if defined(ENABLE_THREADS)
	/* get the lock on the finalizer lock object, so we can call wait */

	finalizer_thread_coord->request();

#else
	/* if we don't have threads, just run the finalizers */

	gc_invoke_finalizers();
#endif
}


/* finalizer_run ***************************************************************

   Actually run the finalizer functions.

*******************************************************************************/

void finalizer_run(void *o, void *p)
{
	java_handle_t *h;
	classinfo     *c;

	h = (java_handle_t *) o;

#if !defined(ENABLE_GC_CACAO) && defined(ENABLE_HANDLES)
	/* XXX this is only a dirty hack to make Boehm work with handles */

	h = LLNI_WRAP((java_object_t *) h);
#endif

	LLNI_class_get(h, c);

	LOG("[finalizer running   :"
	    << " o=" << o
	    << " p=" << p
	    << " class=" << c
		<< "]" << cacao::nl);

	/* call the finalizer function */

	(void) vm_call_method(c->finalizer, h);

	if (exceptions_get_exception() != NULL) {
		LOG("[finalizer exception]" << cacao::nl);
		DEBUG(exceptions_print_stacktrace());
	}

	/* if we had an exception in the finalizer, ignore it */

	exceptions_clear_exception();

#if defined(ENABLE_GC_BOEHM)
	Finalizer::reinstall_custom_finalizer(h);
#endif
}

#if defined(ENABLE_GC_BOEHM)

struct FinalizerData {
	Finalizer::FinalizerFunc f;
	void *data;
	FinalizerData(Finalizer::FinalizerFunc f, void *data): f(f), data(data) { }
};

Mutex final_mutex;
// final_map contains registered custom finalizers for a given Java
// object. Must be accessed with held final_mutex.
std::multimap<java_handle_t *, FinalizerData> final_map;

static void custom_finalizer_handler(void *object, void *data)
{
	typedef std::multimap<java_handle_t *, FinalizerData>::iterator MI;
	java_handle_t *hdl = (java_handle_t *) object;
	MutexLocker l(final_mutex);
	MI it_first = final_map.lower_bound(hdl), it = it_first;
	assert(it->first == hdl);
	for (; it->first == hdl; ++it) {
		final_mutex.unlock();
		it->second.f(hdl, it->second.data);
		final_mutex.lock();
	}
	final_map.erase(it_first, it);
}

/* attach_custom_finalizer *****************************************************

   Register a custom handler that is run when the object becomes
   unreachable. This is intended for internal cleanup actions. If the
   handler already exists, it is not registered again, and its data
   pointer is returned.

*******************************************************************************/

void *Finalizer::attach_custom_finalizer(java_handle_t *h, Finalizer::FinalizerFunc f, void *data)
{
	MutexLocker l(final_mutex);

	GC_finalization_proc ofinal = 0;
	void *odata = 0;

	GC_REGISTER_FINALIZER_UNREACHABLE(LLNI_DIRECT(h), custom_finalizer_handler, 0, &ofinal, &odata);

	/* There was a finalizer -- reinstall it. We do not want to
	   disrupt the normal finalizer operation. This is thread-safe
	   because the other finalizer is only installed at object
	   creation time. */
	if (ofinal && ofinal != custom_finalizer_handler)
		GC_REGISTER_FINALIZER_NO_ORDER(LLNI_DIRECT(h), ofinal, odata, 0, 0);

	typedef std::multimap<java_handle_t *, FinalizerData>::iterator MI;
	std::pair<MI, MI> r = final_map.equal_range(h);
	for (MI it = r.first; it != r.second; ++it)
		if (it->second.f == f)
			return it->second.data;
	final_map.insert(r.first, std::make_pair(h, FinalizerData(f, data)));
	return data;
}

/* reinstall_custom_finalizer **************************************************

   Arranges for the custom finalizers to be called after the Java
   finalizer, possibly much later, because the object needs to become
   unreachable.

*******************************************************************************/

void Finalizer::reinstall_custom_finalizer(java_handle_t *h)
{
	MutexLocker l(final_mutex);
	std::multimap<java_handle_t *, FinalizerData>::iterator it = final_map.find(h);
	if (it == final_map.end())
		return;

	GC_REGISTER_FINALIZER_UNREACHABLE(LLNI_DIRECT(h), custom_finalizer_handler, 0, 0, 0);
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
