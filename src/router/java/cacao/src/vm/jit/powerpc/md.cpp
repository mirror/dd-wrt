/* src/vm/jit/powerpc/md.cpp - machine dependent PowerPC functions

   Copyright (C) 1996-2013
   CACAOVM - Verein zur Foerderung der freien virtuellen Maschine CACAO
   Copyright (C) 2009 Theobroma Systems Ltd.

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

#include <cassert>
#include <stdint.h>

#include "vm/types.hpp"

#include "md-abi.hpp"
#include "vm/jit/powerpc/codegen.hpp"
#include "vm/jit/powerpc/md.hpp"

#include "vm/global.hpp"
#include "vm/vm.hpp"

#include "vm/jit/jit.hpp"
#include "vm/jit/patcher-common.hpp"
#include "vm/jit/trap.hpp"


/* md_init *********************************************************************

   Do some machine dependent initialization.

*******************************************************************************/

void md_init(void)
{
	/* nothing to do */
}


/* md_jit_method_patch_address *************************************************

   Gets the patch address of the currently compiled method. The offset
   is extracted from the load instruction(s) before the jump and added
   to the right base address (PV or REG_METHODPTR).

   INVOKESTATIC/SPECIAL:

   81adffd4    lwz     r13,-44(r13)
   7da903a6    mtctr   r13
   4e800421    bctrl

   INVOKEVIRTUAL:

   81830000    lwz     r12,0(r3)
   81ac0000    lwz     r13,0(r12)
   7da903a6    mtctr   r13
   4e800421    bctrl

   INVOKEINTERFACE:

   81830000    lwz     r12,0(r3)
   818c0000    lwz     r12,0(r12)
   81ac0000    lwz     r13,0(r12)
   7da903a6    mtctr   r13
   4e800421    bctrl

*******************************************************************************/

void *md_jit_method_patch_address(void *pv, void *ra, void *mptr)
{
	uint32_t *pc;
	uint32_t  mcode;
	int32_t   offset;
	void     *pa;

	/* go back to the actual load instruction (3 instructions) */

	pc = ((uint32_t *) ra) - 3;

	/* get first instruction word (lwz) */

	mcode = *pc;

	/* check if we have 2 instructions (addis, addi) */

	if ((mcode >> 16) == 0x3c19) {
		/* XXX write a regression for this */
		pa = NULL;
		assert(0);

		/* get displacement of first instruction (addis) */

		offset = (int32_t) (mcode << 16);

		/* get displacement of second instruction (addi) */

		mcode = *(pc + 1);

		assert((mcode >> 16) != 0x6739);

		offset += (int16_t) (mcode & 0x0000ffff);
	}
	else {
		/* get the offset from the instruction */

		offset = (int16_t) (mcode & 0x0000ffff);

		/* check for load from PV */

		if ((mcode >> 16) == 0x81ad) {
			/* get the final data segment address */

			pa = ((uint8_t *) pv) + offset;
		}
		else if ((mcode >> 16) == 0x81ac) {
			/* in this case we use the passed method pointer */

			/* return NULL if no mptr was specified (used for replacement) */

			if (mptr == NULL)
				return NULL;

			pa = ((uint8_t *) mptr) + offset;
		}
		else {
			/* catch any problems */

			vm_abort("md_jit_method_patch_address: unknown instruction %x",
					 mcode);

			/* keep compiler happy */

			pa = NULL;
		}
	}

	return pa;
}


/**
 * Decode the trap instruction at the given PC.
 *
 * @param trp information about trap to be filled
 * @param sig signal number
 * @param xpc exception PC
 * @param es execution state of the machine
 * @return true if trap was decoded successfully, false otherwise.
 */
bool md_trap_decode(trapinfo_t* trp, int sig, void* xpc, executionstate_t* es)
{
	// Get the exception-throwing instruction.
	uint32_t mcode = *((uint32_t*) xpc);

	switch (sig) {
	case TRAP_SIGILL:
		// Check for valid trap instruction.
		if (patcher_is_valid_trap_instruction_at(xpc)) {
			trp->type  = TRAP_PATCHER;
			trp->value = 0;
			return true;
		}
		return false;

	case TRAP_SIGTRAP:
	{
		int s1 = M_OP3_GET_A(mcode);

		// For now we only handle ArrayIndexOutOfBoundsException.
		trp->type  = TRAP_ArrayIndexOutOfBoundsException;
		trp->value = es->intregs[s1];
		return true;
	}

	case TRAP_SIGSEGV:
	{
		int       s1   = M_INSTR_OP2_IMM_A(mcode);
		uintptr_t addr = es->intregs[s1];

		// Check for special-load.
		if (s1 == REG_ZERO) {
			int16_t disp = M_INSTR_OP2_IMM_I(mcode);
			int     d    = M_INSTR_OP2_IMM_D(mcode);

			// We use the exception type as load displacement.
			trp->type  = disp;
			trp->value = es->intregs[d];
			return true;
		}

		// Check for implicit NullPointerException.
		if (addr == 0) {
			trp->type  = TRAP_NullPointerException;
			trp->value = 0;
			return true;
		}

		return false;
	}

	default:
		return false;
	}
}


/* md_patch_replacement_point **************************************************

   Patch the given replacement point.

*******************************************************************************/

#if defined(ENABLE_REPLACEMENT)
void md_patch_replacement_point(u1 *pc, u1 *savedmcode, bool revert)
{
	u4 mcode;

	if (revert) {
		/* restore the patched-over instruction */
		*(u4*)(pc) = *(u4*)(savedmcode);
	}
	else {
		/* save the current machine code */
		*(u4*)(savedmcode) = *(u4*)(pc);

		// Build the machine code for the patch. On PowerPC we use
		// an illegal instruction which really is just 0, believe me!
		mcode = 0;

		/* write the new machine code */
		*(u4*)(pc) = (u4) mcode;
	}
	
	/* flush instruction cache */
    md_icacheflush(pc,4);
}
#endif /* defined(ENABLE_REPLACEMENT) */


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
