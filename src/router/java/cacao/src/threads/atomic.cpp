/* src/threads/atomic.cpp - atomic instructions

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


#include "config.h"

#include <stdint.h>

#include "threads/atomic.hpp"
#include "threads/mutex.hpp"

// Gobal mutex for generic atomic instructions.
static Mutex lock;

namespace Atomic {

/**
 * A generic atomic compare and swap for 32-bit integer values.  This
 * function is using a mutex to provide atomicity.
 *
 * @param p      Pointer to memory address.
 * @param oldval Old value to be expected.
 * @param newval New value to be stored.
 *
 * @return value of the memory location before the store
 */
uint32_t generic_compare_and_swap(volatile uint32_t *p, uint32_t oldval, uint32_t newval)
{
	uint32_t result;

	lock.lock();

	// Do the compare-and-swap.

	result = *p;

	if (oldval == result)
		*p = newval;

	lock.unlock();

	return result;
}


/**
 * A generic atomic compare and swap for 64-bit integer values.  This
 * function is using a mutex to provide atomicity.
 *
 * @param p      Pointer to memory address.
 * @param oldval Old value to be expected.
 * @param newval New value to be stored.
 *
 * @return value of the memory location before the store
 */
uint64_t generic_compare_and_swap(volatile uint64_t *p, uint64_t oldval, uint64_t newval)
{
	uint64_t result;

	lock.lock();

	// Do the compare-and-swap.

	result = *p;

	if (oldval == result)
		*p = newval;

	lock.unlock();

	return result;
}


/**
 * A generic atomic compare and swap for pointer values.  This
 * function is using a mutex to provide atomicity.
 *
 * @param p      Pointer to memory address.
 * @param oldval Old value to be expected.
 * @param newval New value to be stored.
 *
 * @return value of the memory location before the store
 */
void* generic_compare_and_swap(volatile void** p, void* oldval, void* newval)
{
	void* result;

	lock.lock();

	// Do the compare-and-swap.

	result = (void*) *p;

	if (oldval == result)
		*p = newval;

	lock.unlock();

	return result;
}


/**
 * A generic memory barrier.  This function is using a mutex to
 * provide atomicity.
 */
void generic_memory_barrier(void)
{
	lock.lock();
	lock.unlock();
}

}

// Legacy C interface.

extern "C" {
uint32_t Atomic_compare_and_swap_32(uint32_t *p, uint32_t oldval, uint32_t newval) { return Atomic::compare_and_swap(p, oldval, newval); }
uint64_t Atomic_compare_and_swap_64(uint64_t *p, uint64_t oldval, uint64_t newval) { return Atomic::compare_and_swap(p, oldval, newval); }
void*    Atomic_compare_and_swap_ptr(void** p, void* oldval, void* newval) { return Atomic::compare_and_swap(p, oldval, newval); }
void     Atomic_memory_barrier(void) { Atomic::memory_barrier(); }
void     Atomic_write_memory_barrier(void) { Atomic::write_memory_barrier(); }
void     Atomic_instruction_barrier(void) { Atomic::instruction_barrier(); }
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
