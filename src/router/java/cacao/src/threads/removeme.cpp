/* src/threads/removeme.cpp

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

// XXX Remove me as soon as all using files have been converted to C++!

#include "config.h"

#include "threads/condition.hpp"
#include "threads/mutex.hpp"

extern "C" {

Mutex* Mutex_new() { return new Mutex(); }
void Mutex_delete(Mutex* mutex) { delete mutex; }
void Mutex_lock(Mutex* mutex) { mutex->lock(); }
void Mutex_unlock(Mutex* mutex) { mutex->unlock(); }

Condition* Condition_new() { return new Condition(); }
void Condition_delete(Condition* cond) { delete cond; }
void Condition_broadcast(Condition* cond) { cond->broadcast(); }
void Condition_signal(Condition* cond) { cond->signal(); }
void Condition_timedwait(Condition* cond, Mutex* mutex, const struct timespec* abstime) { cond->timedwait(mutex, abstime); }
void Condition_wait(Condition* cond, Mutex* mutex) { cond->wait(mutex); }

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
