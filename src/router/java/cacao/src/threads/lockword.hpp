/* src/threads/lockword.hpp - lockword implementation

   Copyright (C) 2008-2010
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


#ifndef _LOCKWORD_HPP
#define _LOCKWORD_HPP

#include "config.h"
#include <stdint.h>
#include <assert.h>
#include "threads/atomic.hpp"

/**
 * Lockword.
 */
class Lockword {
private:
	static const int       THIN_LOCK_WORD_SIZE   = SIZEOF_VOID_P * 8; // Pointer size multiplied by 8-bit.
	static const int       THIN_LOCK_SHAPE_BIT   = 0x01;

	static const uintptr_t THIN_UNLOCKED         = 0;

	static const int       THIN_LOCK_COUNT_SHIFT = 1;
	static const int       THIN_LOCK_COUNT_SIZE  = 8;
	static const int       THIN_LOCK_COUNT_INCR  = (1 << THIN_LOCK_COUNT_SHIFT);
	static const int       THIN_LOCK_COUNT_MAX   = ((1 << THIN_LOCK_COUNT_SIZE) - 1);

	static const int       THIN_LOCK_COUNT_MASK  = (THIN_LOCK_COUNT_MAX << THIN_LOCK_COUNT_SHIFT);

	static const int       THIN_LOCK_TID_SHIFT   = (THIN_LOCK_COUNT_SIZE + THIN_LOCK_COUNT_SHIFT);
	static const int       THIN_LOCK_TID_SIZE    = (THIN_LOCK_WORD_SIZE - THIN_LOCK_TID_SHIFT);

private:
	// The actual lockword.
	uintptr_t& _lockword;

public:
	Lockword(uintptr_t& lockword) : _lockword(lockword) {}

	void init() { _lockword = THIN_UNLOCKED; } // REMOVEME

	static inline uintptr_t pre_compute_thinlock(int32_t index);

	inline bool is_thin_lock         () const;
	inline bool is_fat_lock          () const;

	inline bool is_unlocked() const;
	inline bool lock       (uintptr_t thinlock);
	inline void unlock     ();

	inline uintptr_t             get_thin_lock              () const;
	inline uintptr_t             get_thin_lock_without_count() const;
	inline int32_t               get_thin_lock_count        () const;
	inline int32_t               get_thin_lock_thread_index () const;
	inline struct lock_record_t* get_fat_lock               () const;

	inline void set(uintptr_t lockword);
	inline void set(struct lock_record_t* lr);

	inline bool is_max_thin_lock_count  () const;
	inline void increase_thin_lock_count();
	inline void decrease_thin_lock_count();

	void inflate(struct lock_record_t* lr);
};


/**
 * Pre-compute the thin lock value for a thread index.
 *
 * @param index The thead index (>= 1).
 *
 * @return The thin lock value for this thread index.
 */
uintptr_t Lockword::pre_compute_thinlock(int32_t index)
{
	return (index << THIN_LOCK_TID_SHIFT) | THIN_UNLOCKED;
}


bool Lockword::is_thin_lock() const
{
	return ((_lockword & THIN_LOCK_SHAPE_BIT) == 0);
}


bool Lockword::is_fat_lock() const
{
	return ((_lockword & THIN_LOCK_SHAPE_BIT) != 0);
}


/**
 * Check if the lockword is an unlocked thin-lock.
 *
 * @return true if unlocked, false otherwise.
 */
bool Lockword::is_unlocked() const
{
	return (_lockword == THIN_UNLOCKED);
}


/**
 * Try to lock the lockword with the given thin-lock value.
 *
 * @param thinlock Thin-lock value to store in the Lockword.
 *
 * @return true if lock was successful, false otherwise.
 */
bool Lockword::lock(uintptr_t thinlock)
{
	// Atomically exchange the lockword value.
	uintptr_t oldlockword = Atomic::compare_and_swap(&_lockword, THIN_UNLOCKED, thinlock);

	return Lockword(oldlockword).is_unlocked();
}


/**
 * Set the lockword to THIN_UNLOCKED.
 */
void Lockword::unlock()
{
	_lockword = THIN_UNLOCKED;
}


uintptr_t Lockword::get_thin_lock() const
{
	return _lockword;
}


uintptr_t Lockword::get_thin_lock_without_count() const
{
	return (_lockword & ~THIN_LOCK_COUNT_MASK);
}


int32_t Lockword::get_thin_lock_count() const
{
	return (int32_t) (_lockword & THIN_LOCK_COUNT_MASK) >> THIN_LOCK_COUNT_SHIFT;
}


int32_t Lockword::get_thin_lock_thread_index() const
{
	return (int32_t) (_lockword >> THIN_LOCK_TID_SHIFT);
}


struct lock_record_t* Lockword::get_fat_lock() const
{
	return (struct lock_record_t*) (_lockword & ~THIN_LOCK_SHAPE_BIT);
}


void Lockword::set(uintptr_t lockword)
{
	_lockword = lockword;
}


void Lockword::set(struct lock_record_t* lr)
{
	_lockword = ((uintptr_t) lr) | THIN_LOCK_SHAPE_BIT;
}


bool Lockword::is_max_thin_lock_count() const
{
	return (get_thin_lock_count() >= THIN_LOCK_COUNT_MAX);
}


void Lockword::increase_thin_lock_count()
{
	// Sanity check.
	assert(get_thin_lock_count() < THIN_LOCK_COUNT_MAX);

	_lockword += (1 << THIN_LOCK_COUNT_SHIFT);
}


void Lockword::decrease_thin_lock_count()
{
	// Sanity check.
	assert(get_thin_lock_count() > 0);

	_lockword -= (1 << THIN_LOCK_COUNT_SHIFT);
}

#endif // _LOCKWORD_HPP


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
