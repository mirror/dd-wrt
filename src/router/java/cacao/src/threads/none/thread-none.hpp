/* src/threads/none/thread-none.hpp - fake threads header

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


#ifndef THREAD_NONE_HPP_
#define THREAD_NONE_HPP_ 1

#ifndef THREAD_HPP_
# error "Do not directly include this header, include threads/thread.hpp instead"
#endif

#include "config.h"

#include <stdint.h>

#include "vm/types.hpp"

#include "vm/jit/builtin.hpp"
#include "vm/globals.hpp"

#include "vm/jit/stacktrace.hpp"


/* global variables ***********************************************************/

#define THREADOBJECT thread_current

extern threadobject *thread_current;

/* inline functions ***********************************************************/

/**
 * Return the threadobject for the current thread.
 */
inline static threadobject* thread_get_current() {
   return thread_current;
}

/**
 * Set the threadobject for the current thread.
 */
inline static void thread_set_current(threadobject* t)
{
   thread_current = t;
}

#endif // THREAD_NONE_HPP_


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
 */
