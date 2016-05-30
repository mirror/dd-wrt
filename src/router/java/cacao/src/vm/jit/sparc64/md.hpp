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


#ifndef VM_JIT_SPARC64_MD_HPP_
#define VM_JIT_SPARC64_MD_HPP_ 1

#include "config.h"

#include <cassert>
#include <stdint.h>

#include "vm/types.hpp"
#include "vm/jit/code.hpp"
#include "vm/jit/codegen-common.hpp"


/**
 * Returns the size (in bytes) of the current stackframe, specified by
 * the passed codeinfo structure.
 */
inline static int32_t md_stacktrace_get_framesize(codeinfo* code)
{
	// Check for the asm_vm_call_method special case.
	if (code == NULL)
		return 0;

	// On SPARC we use 8-byte stackslots.
#error Verify the below line, then remove this error!
	return code->stackframesize * 8;
}


/* md_stacktrace_get_returnaddress *********************************************

   Returns the return address of the current stackframe, specified by
   the passed stack pointer and the stack frame size.

*******************************************************************************/

inline static void *md_stacktrace_get_returnaddress(void *sp, int32_t stackframesize)
{
	void *ra;

	/* flush register windows to the stack */
	__asm__ ("flushw");

	/* the return address resides in register i7, the last register in the
	 * 16-extended-word save area
	 */
	ra = *((void **) (((uintptr_t) sp) + 120 + BIAS));
	
	/* NOTE: on SPARC ra is the address of the call instruction */

	return ra;
}

inline static void *md_get_framepointer(void *sp)
{
	void *fp;

	/* flush register windows to the stack */
	__asm__ ("flushw");

	fp = *((void **) (((uintptr_t) sp) + 112 + BIAS));

	return fp;
}

inline static void *md_get_pv_from_stackframe(void *sp)
{
	void *pv;

	/* flush register windows to the stack */
	__asm__ ("flushw");

	pv = *((void **) (((uintptr_t) sp) + 104 + BIAS));

	return pv;
}


/* md_codegen_get_pv_from_pc ***************************************************

   This reconstructs and returns the PV of a method given a return address
   pointer. (basically, same was as the generated code following the jump does)
   
   Machine code:

   6b5b4000    jmpl    (pv)
   10000000    nop
   277afffe    ldah    pv,-2(ra)
   237ba61c    lda     pv,-23012(pv)

*******************************************************************************/

/* TODO Move these macros into the code generator. */

/* shift away 13-bit immediate,  mask rd and rs1    */
#define SHIFT_AND_MASK(instr) \
	((instr >> 13) & 0x60fc1)

/* NOP is defined as a SETHI instruction with rd and imm. set to zero */
/* therefore we check if the 22-bit immediate is zero */
#define IS_SETHI(instr) \
	(((instr & 0xc1c00000)  == 0x01000000) \
	&& ((instr & 0x3fffff) != 0x0))

inline s2 decode_13bit_imm(u4 instr) {
	s2 imm;

	/* mask everything else in the instruction */
	imm = instr & 0x00001fff;

	/* sign extend 13-bit to 16-bit */
	imm <<= 3;
	imm >>= 3;

	return imm;
}

inline static void *md_codegen_get_pv_from_pc(void *ra)
{
	uint8_t *pv;
	u8  mcode;
	s4  offset;

	pv = ra;

	/* get the instruction word after jump and nop */
	mcode = *((u4 *) (ra+8) );

	/* check if we have a sethi insruction */
	if (IS_SETHI(mcode)) {
		s4 xor_imm;
				
		/* get 22-bit immediate of sethi instruction */
		offset = (s4) (mcode & 0x3fffff);
		offset = offset << 10;
		
		/* now the xor */
		mcode = *((u4 *) (ra+12) );
		xor_imm = decode_13bit_imm(mcode);
		
		offset ^= xor_imm;	 
	}
	else {
		u4 mcode_masked;
		
		mcode_masked = SHIFT_AND_MASK(mcode);

		assert(mcode_masked == 0x40001);

		/* mask and extend the negative sign for the 13 bit immediate */
		offset = decode_13bit_imm(mcode);
	}
	
	pv += offset;

	return pv;
}


/* md_cacheflush ***************************************************************

   Calls the system's function to flush the instruction and data
   cache.

*******************************************************************************/

inline static void md_cacheflush(void *addr, int nbytes)
{
	/* don't know yet */	
}


/* md_dcacheflush **************************************************************

   Calls the system's function to flush the data cache.

*******************************************************************************/

inline static void md_dcacheflush(void *addr, int nbytes)
{
	/* XXX don't know yet */	
	/* printf("md_dcacheflush\n"); */
	__asm__ __volatile__ ( "membar 0x7F" : : : "memory" );
}

#endif // VM_JIT_SPARC64_MD_HPP_


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
