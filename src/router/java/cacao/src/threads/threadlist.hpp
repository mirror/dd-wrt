/* src/threads/threadlist.hpp - thread list maintenance

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


#ifndef _THREADLIST_HPP
#define _THREADLIST_HPP

#include "config.h"
#include <stdint.h>
#include "threads/condition.hpp"
#include "threads/thread.hpp"
#include "toolbox/list.hpp"
#include "vm/global.hpp"

/* ThreadList *****************************************************************/

class ThreadList {
private:
	static ThreadList   *the_threadlist;

	Mutex               _mutex;              // a mutex for all thread lists

	List<threadobject*> _active_thread_list; // list of active threads
	List<threadobject*> _free_thread_list;   // list of free threads

	// Thread counters visible to Java.
	int32_t             _number_of_started_java_threads;
	int32_t             _number_of_active_java_threads;
	int32_t             _peak_of_active_java_threads;

	// Thread counter for internal usage.
	int32_t             _last_index;

	void                 remove_from_active_thread_list(threadobject* t);
public:
	ThreadList();

	/// Supposed to be called exactly once, early during initialization.
	static void    create_object();
	/// Provides access to singleton.
	static ThreadList *get() { assert(the_threadlist); return the_threadlist; }

	Mutex&         mutex() { return _mutex; }

	void           wait_cond(Condition *cond) { cond->wait(_mutex); }

	void           add_to_active_thread_list(threadobject* t);

	// Thread management methods.
	threadobject*  get_main_thread();
	void           get_free_thread(threadobject **t, int32_t *index);
	threadobject*  get_thread_by_index(int32_t index);
	threadobject*  get_thread_from_java_object(java_handle_t* h);
	void           release_thread(threadobject* t, bool needs_deactivate);
	void           deactivate_thread(threadobject *t);

	// Thread listing methods.
	void           get_active_threads(List<threadobject*> &list);
	void           get_active_java_threads(List<threadobject*> &list);

	// Thread counting methods visible to Java.
	int32_t        get_number_of_started_java_threads();
	int32_t        get_number_of_active_java_threads();
	int32_t        get_number_of_daemon_java_threads();
	int32_t        get_peak_of_active_java_threads();
	void           reset_peak_of_active_java_threads();

	// Thread counting methods for internal use.
	int32_t        get_number_of_active_threads();
	int32_t        get_number_of_non_daemon_threads();

	// Debugging methods.
	void           dump_threads();
};

inline ThreadList::ThreadList():
	_number_of_started_java_threads(0),
	_number_of_active_java_threads(0),
	_peak_of_active_java_threads(0),
	_last_index(0)
{
}

inline void ThreadList::create_object()
{
	assert(!the_threadlist);
	the_threadlist = new ThreadList;
}

inline void ThreadList::add_to_active_thread_list(threadobject* t)
{
	MutexLocker(mutex());

	_active_thread_list.push_back(t);
	t->is_in_active_list = true;

	// Update counter variables.
	if ((t->flags & THREAD_FLAG_INTERNAL) == 0) {
		_number_of_started_java_threads++;
		_number_of_active_java_threads++;
		_peak_of_active_java_threads = MAX(_peak_of_active_java_threads, _number_of_active_java_threads);
	}
}

inline void ThreadList::remove_from_active_thread_list(threadobject* t)
{
	MutexLocker(mutex());

	_active_thread_list.remove(t);
	t->is_in_active_list = false;

	// Update counter variables.
	if ((t->flags & THREAD_FLAG_INTERNAL) == 0) {
		_number_of_active_java_threads--;
	}
}

inline threadobject* ThreadList::get_main_thread()
{
	MutexLocker(mutex());

	return _active_thread_list.front();
}

inline int32_t ThreadList::get_number_of_active_threads()
{
	MutexLocker(mutex());

	return _active_thread_list.size();
}

inline int32_t ThreadList::get_number_of_started_java_threads()
{
	MutexLocker(mutex());

	return _number_of_started_java_threads;
}

inline int32_t ThreadList::get_number_of_active_java_threads()
{
	MutexLocker(mutex());

	return _number_of_active_java_threads;
}

inline int32_t ThreadList::get_peak_of_active_java_threads()
{
	MutexLocker(mutex());

	return _peak_of_active_java_threads;
}

inline void ThreadList::reset_peak_of_active_java_threads()
{
	MutexLocker(mutex());

	_peak_of_active_java_threads = _number_of_active_java_threads;
}

#endif // _THREADLIST_HPP


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
