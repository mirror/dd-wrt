/* src/threads/posix/condition-posix.hpp - POSIX condition variable

   Copyright (C) 2008
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


#ifndef _CONDITION_POSIX_HPP
#define _CONDITION_POSIX_HPP

#include "config.h"

#include <pthread.h>
#include <time.h>
#include "threads/mutex.hpp"


/**
 * POSIX condition variable.
 */
class Condition {
private:
	// POSIX condition structure.
	pthread_cond_t _cond;

public:
	Condition();
	~Condition();

	void broadcast();
	void signal();
	void timedwait(Mutex* mutex, const struct timespec* abstime);
	void wait(Mutex* mutex);
	void wait(Mutex& mutex);
};


// Includes.
#include "vm/vm.hpp"


// Includes.
#include "vm/os.hpp"


/**
 * Initialize a POSIX condition variable.
 */
inline Condition::Condition()
{
	int result = pthread_cond_init(&_cond, NULL);

	if (result != 0) {
		os::abort_errnum(result, "Condition::Condition(): pthread_cond_init failed");
	}
}


/**
 * Destroys a POSIX condition variable.
 */
inline Condition::~Condition()
{
	// Restart all threads waiting on this condition.
	broadcast();

	int result = pthread_cond_destroy(&_cond);

	if (result != 0) {
		os::abort_errnum(result, "Condition::~Condition(): pthread_cond_destroy failed");
	}
}


/**
 * Restarts all the threads that are waiting on the condition
 * variable.
 */
inline void Condition::broadcast()
{
	int result = pthread_cond_broadcast(&_cond);

	if (result != 0) {
		os::abort_errnum(result, "Condition::broadcast(): pthread_cond_broadcast failed");
	}
}


/**
 * Restarts one of the threads that are waiting on this condition
 * variable.
 */
inline void Condition::signal()
{
	int result = pthread_cond_signal(&_cond);

	if (result != 0) {
		os::abort_errnum(result, "Condition::signal(): pthread_cond_signal failed");
	}
}


/**
 * Waits on the condition variable, as wait() does, but it also bounds
 * the duration of the wait.
 */
inline void Condition::timedwait(Mutex* mutex, const struct timespec* abstime)
{
	// This function can return return values which are valid.
	(void) pthread_cond_timedwait(&_cond, &(mutex->_mutex), abstime);
}


/**
 * Waits for the condition variable.
 */
inline void Condition::wait(Mutex* mutex)
{
	wait(*mutex);
}


/**
 * Waits for the condition variable.
 */
inline void Condition::wait(Mutex& mutex)
{
	int result = pthread_cond_wait(&_cond, &(mutex._mutex));

	if (result != 0) {
		os::abort_errnum(result, "Condition::wait(): pthread_cond_wait failed");
	}
}

#endif /* _CONDITION_POSIX_HPP */


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
