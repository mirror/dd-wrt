/* src/vm/jit/mips/md.cpp - machine dependent MIPS functions

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


#include "config.h"

#include <stdint.h>
#include <ucontext.h>

#include "vm/jit/mips/codegen.hpp"
#include "vm/jit/mips/md.hpp"

#include "vm/jit/asmpart.hpp"
#include "vm/jit/jit.hpp"
#include "vm/jit/trap.hpp"


/* md_jit_method_patch_address *************************************************

   Gets the patch address of the currently compiled method. The offset
   is extracted from the load instruction(s) before the jump and added
   to the right base address (PV or REG_METHODPTR).

   INVOKESTATIC/SPECIAL:

   dfdeffb8    ld       s8,-72(s8)
   03c0f809    jalr     s8
   00000000    nop

   INVOKEVIRTUAL:

   dc990000    ld       t9,0(a0)
   df3e0000    ld       s8,0(t9)
   03c0f809    jalr     s8
   00000000    nop

   INVOKEINTERFACE:

   dc990000    ld       t9,0(a0)
   df39ff90    ld       t9,-112(t9)
   df3e0018    ld       s8,24(t9)
   03c0f809    jalr     s8
   00000000    nop

*******************************************************************************/

void *md_jit_method_patch_address(void *pv, void *ra, void *mptr)
{
	uint32_t *pc;
	uint32_t  mcode;
	int32_t   offset;
	void     *pa;

	/* go back to the actual load instruction (3 instructions on MIPS) */

	pc = ((uint32_t *) ra) - 3;

	/* get first instruction word on current PC */

	mcode = pc[0];

	/* check if we have 2 instructions (lui) */

	if ((mcode >> 16) == 0x3c19) {
		/* XXX write a regression for this */
		assert(0);

		/* get displacement of first instruction (lui) */

		offset = (int32_t) (mcode << 16);

		/* get displacement of second instruction (daddiu) */

		mcode = pc[1];

		assert((mcode >> 16) != 0x6739);

		offset += (int16_t) (mcode & 0x0000ffff);

		pa = NULL;
	}
	else {
		/* get the offset from the instruction */

		offset = (int16_t) (mcode & 0x0000ffff);

		/* check for call with REG_METHODPTR: ld s8,x(t9) */

#if SIZEOF_VOID_P == 8
		if ((mcode >> 16) == 0xdf3e) {
#else
		if ((mcode >> 16) == 0x8f3e) {
#endif
			/* in this case we use the passed method pointer */

			/* return NULL if no mptr was specified (used for replacement) */

			if (mptr == NULL)
				return NULL;

			pa = ((uint8_t *) mptr) + offset;
		}
		else {
			/* in the normal case we check for a `ld s8,x(s8)' instruction */

#if SIZEOF_VOID_P == 8
			assert((mcode >> 16) == 0xdfde);
#else
			assert((mcode >> 16) == 0x8fde);
#endif

			/* and get the final data segment address */

			pa = ((uint8_t *) pv) + offset;
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
        // TODO Check the whole instruction.
        if (mcode == 0xec000000) {
            trp->type  = TRAP_PATCHER;
            trp->value = 0;
            return true;
        }
        return false;

    case TRAP_SIGSEGV:
    {
        // Retrieve base address of instruction.
        int32_t   s1   = M_ITYPE_GET_RS(mcode);
        uintptr_t addr = es->intregs[s1];

        // Check for special-load.
        if (s1 == REG_ZERO) {
            int32_t d    = M_ITYPE_GET_RT(mcode);
            int32_t disp = M_ITYPE_GET_IMM(mcode);

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
	union {
		u8 both;
		u4 words[2];
	} mcode;

	if (revert) {
		/* restore the patched-over instruction */
		*(u8*)(pc) = *(u8*)(savedmcode);
	}
	else {
		/* save the current machine code */
		*(u8*)(savedmcode) = *(u8*)(pc);

		/* build the machine code for the patch */
		assert(0); /* XXX build trap instruction below */
		mcode.both = 0;

		/* write the new machine code */
		*(u8*)(pc) = mcode.both;
	}

	/* flush instruction cache */
    md_icacheflush(pc,2*4);
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
