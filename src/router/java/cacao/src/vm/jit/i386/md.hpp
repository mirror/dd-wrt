/* src/vm/jit/i386/md.hpp - machine dependent i386 functions

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


#ifndef VM_JIT_I386_MD_HPP_
#define VM_JIT_I386_MD_HPP_ 1

#include "config.h"

#include <cassert>
#include <stdint.h>

#include "vm/jit/code.hpp"
#include "vm/jit/codegen-common.hpp"
#include "vm/jit/methodtree.hpp"


/* inline functions ***********************************************************/

/**
 * Returns the size (in bytes) of the current stackframe, specified by
 * the passed codeinfo structure.
 */
inline static int32_t md_stacktrace_get_framesize(codeinfo* code)
{
	int32_t stackframesize;

	// Check for the asm_vm_call_method special case.
	if (code == NULL)
		return 0;

	// On i386 we use 8-byte stackslots.
	stackframesize = code->stackframesize * 8;

	// If there is a stackframe present, we need to take the alignment
	// compensation for the stored return address into account.
	if (stackframesize != 0)
		stackframesize += 4;

	return stackframesize;
}


/* md_stacktrace_get_returnaddress *********************************************

   Returns the return address of the current stackframe, specified by
   the passed stack pointer and the stack frame size.

*******************************************************************************/

inline static void *md_stacktrace_get_returnaddress(void *sp, int32_t stackframesize)
{
	void *ra;

	/* On i386 the return address is above the current stack frame. */

	ra = *((void **) (((uintptr_t) sp) + stackframesize));

	return ra;
}


/* md_codegen_get_pv_from_pc ***************************************************

   On this architecture just a wrapper function to
   methodtree_find.

*******************************************************************************/

inline static void *md_codegen_get_pv_from_pc(void *ra)
{
	void *pv;

	/* Get the start address of the function which contains this
       address from the method table. */

	pv = methodtree_find(ra);

	return pv;
}


/* md_cacheflush ***************************************************************

   Calls the system's function to flush the instruction and data
   cache.

*******************************************************************************/

inline static void md_cacheflush(void *addr, int nbytes)
{
	// Compiler optimization barrier (see PR97).
	__asm__ __volatile__ ("" : : : "memory");
}


/* md_icacheflush **************************************************************

   Calls the system's function to flush the instruction cache.

*******************************************************************************/

inline static void md_icacheflush(void *addr, int nbytes)
{
	// Compiler optimization barrier (see PR97).
	__asm__ __volatile__ ("" : : : "memory");
}


/* md_dcacheflush **************************************************************

   Calls the system's function to flush the data cache.

*******************************************************************************/

inline static void md_dcacheflush(void *addr, int nbytes)
{
	// Compiler optimization barrier (see PR97).
	__asm__ __volatile__ ("" : : : "memory");
}


/* md_get_cycle_count **********************************************************

   Get the current time-stamp counter from the CPU.

*******************************************************************************/

inline static uint64_t md_get_cycle_count()
{
	uint64_t cycles;

	// Get current cycles count from the CPU.
	__asm__ __volatile__ ("rdtsc" : "=A" (cycles));

	return cycles;
}

#endif // VM_JIT_I386_MD_HPP_


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
