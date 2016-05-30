/* src/threads/threadobject.hpp - POSIX thread data structure

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


#ifndef THREAD_POSIX_HPP_
#define THREAD_POSIX_HPP_ 1

#ifndef THREAD_HPP_
# error "Do not directly include this header, include threads/thread.hpp instead"
#endif

#include "config.h"

#include <pthread.h>
#include <ucontext.h>

#include "vm/types.hpp"

#if defined(ENABLE_TLH)
# include "mm/tlh.hpp"
#endif

#include "threads/condition.hpp"
#include "threads/mutex.hpp"

/* current threadobject *******************************************************/

#if defined(HAVE___THREAD)

#define THREADOBJECT      thread_current

extern __thread threadobject *thread_current;

#else /* defined(HAVE___THREAD) */

#define THREADOBJECT \
	((threadobject *) pthread_getspecific(thread_current_key))

extern pthread_key_t thread_current_key;

#endif /* defined(HAVE___THREAD) */



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

#endif // THREADOBJECT_POSIX_HPP_

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
