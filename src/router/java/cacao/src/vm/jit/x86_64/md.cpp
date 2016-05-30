/* src/vm/jit/x86_64/md.cpp - machine dependent x86_64 functions

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
#include <cstdlib>
#include <stdint.h>

#include "vm/jit/x86_64/codegen.hpp"
#include "vm/jit/x86_64/md-abi.hpp"

#include "vm/vm.hpp"

#include "vm/jit/codegen-common.hpp"
#include "vm/jit/executionstate.hpp"
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

   4d 8b 15 e2 fe ff ff             mov    -286(%rip),%r10
   49 ff d2                         rex64Z callq  *%r10

   INVOKEVIRTUAL:

   4c 8b 17                         mov    (%rdi),%r10
   49 8b 82 00 00 00 00             mov    0x0(%r10),%rax
   48 ff d3                         rex64 callq  *%rax

   INVOKEINTERFACE:

   4c 8b 17                         mov    (%rdi),%r10
   4d 8b 92 00 00 00 00             mov    0x0(%r10),%r10
   49 8b 82 00 00 00 00             mov    0x0(%r10),%rax
   48 ff d3                         rex64 callq  *%r11

*******************************************************************************/

void *md_jit_method_patch_address(void *pv, void *ra, void *mptr)
{
	uint8_t *pc;
	uint8_t  mcode;
	int32_t  offset;
	void    *pa;                        /* patch address                      */

	/* go back to the actual call instruction (3-bytes) */

	pc = ((uint8_t *) ra) - 3;

	/* get the last byte of the call */

	mcode = pc[2];

	/* check for the different calls */

	if (mcode == 0xd2) {
		/* INVOKESTATIC/SPECIAL */

		/* Get the offset from the instruction (the offset address is
		   4-bytes before the call instruction). */

		offset = *((int32_t *) (pc - 4));

		/* add the offset to the return address (IP-relative addressing) */

		pa = pc + offset;
	}
	else if (mcode == 0xd3) {
		/* INVOKEVIRTUAL/INTERFACE */

		/* return NULL if no mptr was specified (used for replacement) */

		if (mptr == NULL)
			return NULL;

		/* Get the offset from the instruction (the offset address is
		   4-bytes before the call instruction). */

		offset = *((int32_t *) (pc - 4));

		/* add the offset to the method pointer */

		pa = ((uint8_t *) mptr) + offset;
	}
	else {
		/* catch any problems */

		vm_abort("md_jit_method_patch_address: unknown instruction %x", mcode);

		/* keep compiler happy */

		pa = NULL;
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
bool md_trap_decode(trapinfo_t* trp, int sig, void* _xpc, executionstate_t* es)
{
	uint8_t* xpc = (uint8_t*) _xpc;

#if 0
	// Check for StackOverflowException.
	threads_check_stackoverflow(sp);
#endif

	switch (sig) {
	case TRAP_SIGFPE:
		// Check for ArithmeticException.
		trp->type  = TRAP_ArithmeticException;
		trp->value = 0;
		return true;

	case TRAP_SIGILL:
		// Check for valid trap instruction.
		if (patcher_is_valid_trap_instruction_at(xpc)) {
			trp->type  = TRAP_PATCHER;
			trp->value = 0;
			return true;
		}
		return false;

	case TRAP_SIGSEGV:
	{
		// Get exception-throwing instruction.
		uint8_t opc = M_ALD_MEM_GET_OPC(xpc);
		uint8_t mod = M_ALD_MEM_GET_MOD(xpc);
		uint8_t rm  = M_ALD_MEM_GET_RM(xpc);

		// Check for hardware exception, for values
		// see emit_mov_mem_reg and emit_mem.
		if ((opc == 0x8b) && (mod == 0) && (rm == 4)) {
			int32_t d    = M_ALD_MEM_GET_REG(xpc);
			int32_t disp = M_ALD_MEM_GET_DISP(xpc);

			// We use the exception type as load displacement.
			trp->type  = disp;
			trp->value = es->intregs[d];
			return true;
		}

		// Default case is a normal NullPointerException.
		else {
			trp->type  = TRAP_NullPointerException;
			trp->value = 0;
			return true;
		}
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
	u2 mcode;

	if (revert) {
		/* write saved machine code */
		*(u2*)(pc) = *(u2*)(savedmcode);
	}
	else {
		/* save the current machine code */
		*(u2*)(savedmcode) = *(u2*)(pc);

		/* build the machine code for the patch */
		mcode = 0x0b0f;

		/* write new machine code */
		*(u2*)(pc) = (u2) mcode;
	}

    /* XXX if required asm_cacheflush(pc,8); */
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
