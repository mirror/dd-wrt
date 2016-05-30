/* src/vm/jit/sparc64/md.cpp - machine dependent SPARC64 functions

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

#include <cassert>
#include <stdint.h>

#include "vm/types.hpp"

#include "vm/jit/sparc64/md-abi.hpp"

#include "vm/jit/asmpart.hpp"
#include "vm/jit/codegen-common.hpp"
#include "vm/jit/jit.hpp"


/* assembler function prototypes **********************************************/
void asm_store_fp_state_reg(u8 *mem);
void asm_load_fp_state_reg(u8 *mem);


/* NOP is defined as a SETHI instruction with rd and imm. set to zero */
/* therefore we check if the 22-bit immediate is zero */
#define IS_SETHI(instr) \
	(((instr & 0xc1c00000)  == 0x01000000) \
	&& ((instr & 0x3fffff) != 0x0))
	
#define IS_LDX_IMM(instr) \
	(((instr >> 13) & 0x60fc1) == 0x602c1)
	
#define IS_SUB(instr) \
	(((instr >> 13) & 0x60fc0) == 0x40100)

inline s2 decode_13bit_imm(u4 instr) {
	s2 imm;

	/* mask everything else in the instruction */
	imm = instr & 0x00001fff;

	/* sign extend 13-bit to 16-bit */
	imm <<= 3;
	imm >>= 3;

	return imm;
}

/* md_init *********************************************************************

   Do some machine dependent initialization.

*******************************************************************************/

void md_init(void)
{
	/* do nothing */
}


/* md_jit_method_patch_address *************************************************

   Gets the patch address of the currently compiled method. The offset
   is extracted from the load instruction(s) before the jump and added
   to the right base address (PV or REG_METHODPTR).

   INVOKESTATIC/SPECIAL:

   ????????    ldx      [i5 - 72],o5
   ????????    jmp      o5             <-- ra
   ????????    nop

   w/ sethi (mptr in dseg out of 13-bit simm range)

   ????????    sethi    hi(0x2000),o5
   ????????    sub      i5,o5,o5
   ????????    ldx      [o5 - 72],o5
   ????????    jmp      o5             <-- ra
   ????????    nop

   INVOKEVIRTUAL:

   ????????    ldx      [o0 + 0},g2
   ????????    ldx      [g2 + 0],o5
   ????????    jmp      o5             <-- ra
   ????????    nop

   INVOKEINTERFACE:

   ????????    ldx      [o0 + 0},g2
   ????????    ldx      [g2 - 112],g2
   ????????    ldx      [g2 + 24],o5
   ????????    jmp      o5             <-- ra
   ????????    nop

*******************************************************************************/

void *md_jit_method_patch_address(void *pv, void *ra, void *mptr)
{
	uint32_t *pc;
	uint32_t  mcode, mcode_sethi, mcode_masked;
	int32_t   disp;
	uint8_t  *iptr;
	void     *pa;

	/* Go back to the location of a possible sethi (3 instruction
	   before jump). */

	pc = ((uint32_t *) ra) - 3;

	/* note: ra is the address of the jump instruction on SPARC */

	mcode_sethi = pc[0];

	/* check for sethi instruction */

	if (IS_SETHI(mcode_sethi)) {
		u4 mcode_sub, mcode_ldx;

		mcode_sub = pc[1];
		mcode_ldx = pc[2];

		/* make sure the sequence of instructions is a loadhi */
		if ((IS_SUB(mcode_sub)) && (IS_LDX_IMM(mcode_ldx)))
		{


		/* get 22-bit immediate of sethi instruction */

		disp = (int32_t) (mcode_sethi & 0x3fffff);
		disp = disp << 10;
		
		/* goto next instruction */
		
		/* make sure it's a sub instruction (pv - big_disp) */
		assert(IS_SUB(mcode_sub));
		disp = -disp;

		/* get displacement of load instruction */

		assert(IS_LDX_IMM(mcode_ldx));

		disp += decode_13bit_imm(mcode_ldx);
		
		pa = ((uint8_t *) pv) + disp;

		return pa;
		}
	}

	/* we didn't find a sethi, or it didn't belong to a loadhi */
	/* check for simple (one-instruction) load */

	mcode = pc[2];

	/* shift and mask rd */

	mcode_masked = (mcode >> 13) & 0x060fff;
	
	/* get the offset from the instruction */

	disp = decode_13bit_imm(mcode);

	/* check for call with rs1 == REG_METHODPTR: ldx [g2+x],pv_caller */

	if (mcode_masked == 0x0602c5) {
		/* in this case we use the passed method pointer */

		/* return NULL if no mptr was specified (used for replacement) */

		if (mptr == NULL)
			return NULL;

		pa = ((uint8_t *) mptr) + disp;

	} else {
		/* in the normal case we check for a `ldx [i5+x],pv_caller' instruction */

		assert(mcode_masked  == 0x0602fb);

		/* and get the final data segment address */

		pa = ((uint8_t *) pv) + disp;
	}

	return pa;
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

		/* build the machine code for the patch */
		assert(0); /* XXX build trap instruction below */
		mcode = 0;

		/* write the new machine code */
		*(u4*)(pc) = (u4) mcode;
	}

	/* flush instruction cache */
    /* md_icacheflush(pc,4); */
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
