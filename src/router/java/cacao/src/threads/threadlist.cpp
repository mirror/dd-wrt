/* src/threads/threadlist.cpp - thread list maintenance

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


#include "config.h"

#include <stdint.h>
#include <algorithm>
#include <functional>

#include "threads/mutex.hpp"
#include "threads/threadlist.hpp"
#include "threads/thread.hpp"

#include "toolbox/list.hpp"
#include "toolbox/logging.hpp"

#include "vm/jit/stacktrace.hpp"

ThreadList *ThreadList::the_threadlist = 0;

/**
 * Dumps info for all threads running in the VM.  This function is
 * called when SIGQUIT (<ctrl>-\) is sent to the VM.
 */
void ThreadList::dump_threads()
{
	// XXX we should stop the world here and remove explicit
	//     thread suspension from the loop below.
	// Lock the thread lists.
	MutexLocker lock(_mutex);

	printf("Full thread dump CACAO "VERSION_FULL":\n");

	// Iterate over all started threads.
	threadobject* self = THREADOBJECT;
	for (List<threadobject*>::iterator it = _active_thread_list.begin(); it != _active_thread_list.end(); it++) {
		threadobject* t = *it;

		// Ignore threads which are in state NEW.
		if (t->state == THREAD_STATE_NEW)
			continue;

		/* Suspend the thread (and ignore return value). */

		if (t != self)
			(void) threads_suspend_thread(t, SUSPEND_REASON_DUMP);

		/* Print thread info. */

		printf("\n");
		thread_print_info(t);
		printf("\n");

		/* Print trace of thread. */

		stacktrace_print_of_thread(t);

		/* Resume the thread (and ignore return value). */

		if (t != self)
			(void) threads_resume_thread(t, SUSPEND_REASON_DUMP);
	}
}


/**
 * Fills the passed list with all currently active threads. Creating a copy
 * of the thread list here, is the only way to ensure we do not end up in a
 * dead-lock when iterating over the list.
 *
 * @param list list class to be filled
 */
void ThreadList::get_active_threads(List<threadobject*> &list)
{
	MutexLocker lock(_mutex);

	// Use the assignment operator to create a copy of the thread list.
	list = _active_thread_list;
}


/**
 * Fills the passed list with all currently active threads which should be
 * visible to Java. Creating a copy of the thread list here, is the only way
 * to ensure we do not end up in a dead-lock when iterating over the list.
 *
 * @param list list class to be filled
 */
void ThreadList::get_active_java_threads(List<threadobject*> &list)
{
	MutexLocker lock(_mutex);

	// Iterate over all active threads.
	for (List<threadobject*>::iterator it = _active_thread_list.begin(); it != _active_thread_list.end(); it++) {
		threadobject* t = *it;

		// We skip internal threads.
		if (t->flags & THREAD_FLAG_INTERNAL)
			continue;

		list.push_back(t);
	}
}


/**
 * Get the next free thread object.
 *
 * Gets the next free thread object and a thread index for it.
 * The results are stored into the passed pointers.
 *
 * If no free thread is available `*thread' will contain NULL.
 * `*index' will always, even if no free thread is available,
 * contain a valid index you can use for a new thread.
 */
void ThreadList::get_free_thread(threadobject **thread, int32_t *index) {
	MutexLocker lock(_mutex);

	// Do we have free threads in the free-list?
	if (_free_thread_list.empty() == false) {
		// Yes, get the index and remove it from the free list.
		threadobject* t = _free_thread_list.front();
		_free_thread_list.pop_front();

		*thread = t;
		*index  = t->index;
	} else {
		*thread = NULL;
		*index  = ++_last_index;
	}
}


/**
 * Return the number of daemon threads visible to Java.
 *
 * NOTE: This function does a linear-search over the threads list,
 *       because it is only used by the management interface.
 *
 * @return number of daemon threads
 */
int32_t ThreadList::get_number_of_daemon_java_threads(void)
{
	int number = 0;

	// Lock the thread lists.
	MutexLocker lock(_mutex);

	// Iterate over all active threads.
	for (List<threadobject*>::iterator it = _active_thread_list.begin(); it != _active_thread_list.end(); it++) {
		threadobject* t = *it;

		// We skip internal threads.
		if (t->flags & THREAD_FLAG_INTERNAL)
			continue;

		if (thread_is_daemon(t))
			number++;
	}

	return number;
}


/**
 * Return the number of non-daemon threads.
 *
 * NOTE: This function does a linear-search over the threads list,
 *       because it is only used for joining the threads.
 *
 * @return number of non daemon threads
 */
int32_t ThreadList::get_number_of_non_daemon_threads(void)
{
	MutexLocker lock(_mutex);

	int nondaemons = 0;

	for (List<threadobject*>::iterator it = _active_thread_list.begin(); it != _active_thread_list.end(); it++) {
		threadobject* t = *it;

		if (!thread_is_daemon(t))
			nondaemons++;
	}

	return nondaemons;
}

// Comparator class.
class comparator : public std::binary_function<threadobject*, int32_t, bool> {
public:
	bool operator() (const threadobject* t, const int32_t index) const {
		return (t->index == index);
	}
};

/**
 * Return the thread object with the given index.
 *
 * @return thread object
 */
threadobject* ThreadList::get_thread_by_index(int32_t index)
{
	MutexLocker lock(_mutex);

	List<threadobject*>::iterator it = find_if(_active_thread_list.begin(), _active_thread_list.end(), std::bind2nd(comparator(), index));

	// No thread found.
	if (it == _active_thread_list.end()) {
		return NULL;
	}

	threadobject* t = *it;

	// The thread found is in state new.
	if (t->state == THREAD_STATE_NEW) {
		return NULL;
	}

	return t;
}


/**
 * Return the Java thread object from the given thread object.
 *
 * @return Java thread object
 */
threadobject* ThreadList::get_thread_from_java_object(java_handle_t* h)
{
	MutexLocker lock(_mutex);

	for (List<threadobject*>::iterator it = _active_thread_list.begin(); it != _active_thread_list.end(); it++) {
		threadobject* t = *it;

		bool equal;
		LLNI_equals(t->object, h, equal);

		if (equal == true) {
			return t;
		}
	}

	return NULL;
}

void ThreadList::deactivate_thread(threadobject *t)
{
	MutexLocker lock(_mutex);

	remove_from_active_thread_list(t);
	threads_impl_clear_heap_pointers(t); // allow it to be garbage collected
}

/**
 * Release the thread.
 *
 * @return free thread index
 */
void ThreadList::release_thread(threadobject* t, bool needs_deactivate)
{
	MutexLocker lock(_mutex);

	if (needs_deactivate)
		// Move thread from active thread list to free thread list.
		remove_from_active_thread_list(t);
	else
		assert(!t->is_in_active_list);

	_free_thread_list.push_back(t);
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
