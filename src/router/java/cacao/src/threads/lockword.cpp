/* src/threads/lockword.cpp - lockword implementation

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


#include "threads/lockword.hpp"

#include <assert.h>

#include "threads/lock.hpp"
#include "threads/thread.hpp"


/**
 * Inflate the lock of the given object. This may only be called by
 * the owner of the monitor of the object.
 * 
 * PRE-CONDITION: The current thread must be the owner of this
 * object's monitor AND of the lock record's lock!
 *
 * @param lr The lock-record to install.  The current thread must own
 *	         the lock of this lock record!
 */
void Lockword::inflate(lock_record_t* lr)
{
	if (is_fat_lock()) {
		// Sanity check.
		assert(get_fat_lock() == lr);
		return;
	}

	// Sanity check.
	assert(get_thin_lock_without_count() == thread_get_current()->thinlock);

	// Copy the count from the thinlock.
	lr->count = get_thin_lock_count();

// 	DEBUGLOCKS(("[lock_inflate      : lr=%p, t=%p, o=%p, o->lockword=%lx, count=%d]",
// 				lr, thread_get_current(), o, get_thin_lock(), lr->count));

	// Install the lock-record in the lockword.
	set(lr);
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
