/* src/threads/posix/mutex-posix.hpp - POSIX mutual exclusion functions

   Copyright (C) 2008
   CACAOVM - Verein zur Foerderung der freien virtuellen Maschine CACAO
   Copyright (C) 2008 Theobroma Systems Ltd.

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


#ifndef _MUTEX_POSIX_HPP
#define _MUTEX_POSIX_HPP

#include "config.h"

#include <pthread.h>

/**
 * POSIX implementation of a mutex.
 */
class Mutex {
private:
	// POSIX mutex structure.
	pthread_mutex_t     _mutex;
	pthread_mutexattr_t _attr;

	// Condition class needs to access _mutex for wait() and
	// timedwait().
	friend class Condition;
	
public:
	inline Mutex();
	inline ~Mutex();

	inline void lock();
	inline void unlock();
};

// Includes.
#include "vm/os.hpp"

/**
 * Initializes the given mutex object and checks for errors.
 */
inline Mutex::Mutex()
{
	int result = pthread_mutexattr_init(&_attr);

	if (result != 0) {
		os::abort_errnum(result, "Mutex::Mutex(): pthread_mutexattr_init failed");
	}

	result = pthread_mutexattr_settype(&_attr, PTHREAD_MUTEX_RECURSIVE);

	if (result != 0) {
		os::abort_errnum(result, "Mutex::Mutex(): pthread_mutexattr_settype failed");
	}

	result = pthread_mutex_init(&_mutex, &_attr);

	if (result != 0) {
		os::abort_errnum(result, "Mutex::Mutex(): pthread_mutex_init failed");
	}
}


/**
 * Destroys the given mutex object and checks for errors.
 */
inline Mutex::~Mutex()
{
	int result = pthread_mutexattr_destroy(&_attr);

	if (result != 0) {
		os::abort_errnum(result, "Mutex::~Mutex(): pthread_mutexattr_destroy failed");
	}

	result = pthread_mutex_destroy(&_mutex);

	if (result != 0) {
		os::abort_errnum(result, "Mutex::~Mutex(): pthread_mutex_destroy failed");
	}
}


/**
 * Locks the given mutex object and checks for errors. If the mutex is
 * already locked by another thread, the calling thread is suspended until
 * the mutex is unlocked.
 *
 * If the mutex is already locked by the calling thread, the same applies,
 * thus effectively causing the calling thread to deadlock. (This is because
 * we use "fast" pthread mutexes without error checking.)
 */
inline void Mutex::lock()
{
	int result = pthread_mutex_lock(&_mutex);

	if (result != 0) {
		os::abort_errnum(result, "Mutex::lock(): pthread_mutex_lock failed");
	}
}


/**
 * Unlocks the given mutex object and checks for errors. The mutex is
 * assumed to be locked and owned by the calling thread.
 */
inline void Mutex::unlock()
{
	int result = pthread_mutex_unlock(&_mutex);

	if (result != 0) {
		os::abort_errnum(result, "Mutex::unlock: pthread_mutex_unlock failed");
	}
}

#endif /* _MUTEX_POSIX_HPP */


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
