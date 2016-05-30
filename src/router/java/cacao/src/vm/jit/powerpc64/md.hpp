/* src/vm/jit/powerpc64/md.hpp - machine dependent PowerPC functions

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


#ifndef VM_JIT_POWERPC64_MD_HPP_
#define VM_JIT_POWERPC64_MD_HPP_ 1

#include "config.h"

#include <cassert>
#include <stdint.h>

#include "vm/jit/powerpc64/codegen.hpp"

#include "vm/global.hpp"
#include "vm/vm.hpp"

#include "vm/jit/asmpart.hpp"
#include "vm/jit/code.hpp"
#include "vm/jit/jit.hpp"


/**
 * Returns the size (in bytes) of the current stackframe, specified by
 * the passed codeinfo structure.
 */
inline static int32_t md_stacktrace_get_framesize(codeinfo* code)
{
	// Check for the asm_vm_call_method special case.
	if (code == NULL)
		return 0;

	// On PowerPC64 we use 8-byte stackslots.
	return code->stackframesize * 8;
}


/* md_stacktrace_get_returnaddress *********************************************

   Returns the return address of the current stackframe, specified by
   the passed stack pointer and the stack frame size.

*******************************************************************************/

inline static void *md_stacktrace_get_returnaddress(void *sp, int32_t stackframesize)
{
	void *ra;

	/* On PowerPC64 the return address is located in the linkage
	   area. */

	ra = *((void **) (((uintptr_t) sp) + stackframesize + LA_LR_OFFSET));

	return ra;
}


/* md_codegen_get_pv_from_pc ***************************************************

   Machine code:

   7d6802a6    mflr    r11
   39cbffe0    addi    r14,r11,-32

   or

   7d6802a6    mflr    r11
   3dcbffff    addis   r14,r11,-1
   39ce68b0    addi    r14,r13,26800

*******************************************************************************/

inline static void* md_codegen_get_pv_from_pc(void* ra)
{
	int32_t offset;

	uint32_t* pc = (uint32_t*) ra;

	/* get first instruction word after jump */

	uint32_t mcode = pc[1];

	/* check if we have 2 instructions (addis, addi) */

	if ((mcode >> 16) == 0x3dcb) {
		/* get displacement of first instruction (addis) */

		offset = (int32_t) (mcode << 16);

		/* get displacement of second instruction (addi) */

		mcode = pc[2];

		/* check for addi instruction */

		assert((mcode >> 16) == 0x39ce);

		offset += (int16_t) (mcode & 0x0000ffff);
	}
	else if ((mcode >> 16) == 0x39cb) {
		/* get offset of first instruction (addi) */

		offset = (int16_t) (mcode & 0x0000ffff);
	}
	else {
		vm_abort("md_codegen_get_pv_from_pc: unknown instruction %x", mcode);

		/* keep compiler happy */

		offset = 0;
	}

	/* calculate PV via RA + offset */

	void* pv = (void*) (((uintptr_t) ra) + offset);

	return pv;
}


/* md_cacheflush ***************************************************************

   Calls the system's function to flush the instruction and data
   cache.

*******************************************************************************/

inline static void md_cacheflush(void *addr, int nbytes)
{
	asm_cacheflush(addr, nbytes);
}


/* md_icacheflush **************************************************************

   Calls the system's function to flush the instruction cache.

*******************************************************************************/

inline static void md_icacheflush(void *addr, int nbytes)
{
	asm_cacheflush(addr, nbytes);
}


/* md_dcacheflush **************************************************************

   Calls the system's function to flush the data cache.

*******************************************************************************/

inline static void md_dcacheflush(void *addr, int nbytes)
{
	asm_cacheflush(addr, nbytes);
}

#endif // VM_JIT_POWERPC64_MD_HPP_


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
