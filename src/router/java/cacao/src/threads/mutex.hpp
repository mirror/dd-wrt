/* src/threads/mutex.hpp - machine independent mutual exclusion functions

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


#ifndef _MUTEX_HPP
#define _MUTEX_HPP

#include "config.h"

#if defined(ENABLE_THREADS)
# include "threads/posix/mutex-posix.hpp"
#else
# include "threads/none/mutex-none.hpp"
#endif

/**
 * Helper class used to implicitly acquire and release a mutex
 * within a method scope.
 */
template<class T>
class AnyObjLocker
{
private:
	T& _mutex;

public:
	AnyObjLocker(T& mutex): _mutex(mutex) { _mutex.lock(); }
	~AnyObjLocker()                       { _mutex.unlock(); }
};

template<class T>
class AnyClassLocker
{
public:
	AnyClassLocker() { T::lock(); }
	~AnyClassLocker() { T::unlock(); }
};

typedef AnyObjLocker<Mutex> MutexLocker;

#endif /* _MUTEX_HPP */


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
