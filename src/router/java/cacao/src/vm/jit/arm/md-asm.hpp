/* src/vm/jit/arm/md-asm.hpp - assembler defines for arm ABI

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

   Contact: cacao@cacaojvm.org

   Authors: Michael Starzinger
            Christian Thalinger

*/


#ifndef MD_ASM_HPP_
#define MD_ASM_HPP_ 1

#include "config.h"


/* register defines ***********************************************************/

#define res1    r0  /* result registers         */
#define res2    r1

#define a0      r0  /* argument registers       */
#define a1      r1
#define a2      r2
#define a3      r3

#define v1      r4  /* variable registers       */
#define v2      r5
#define v3      r6
#define v4      r7
#define v5      r8
#define v6      r9
#define v7      r10
#define v8      r11

#define pc      r15 /* program counter          */
#define lr      r14 /* return address           */
#define sp      r13 /* stack pointer            */
#define ip      r12 /* something like pv        */
#define fp      r11 /* frame pointer (not used) */

#define itmp1   v7  /* temporary scratch regs   */
#define itmp2   v8
#define itmp3   v6

#define mptr    v8

#define xptr  itmp1 /* exception registers      */
#define xpc   itmp2


/* save and restore macros ****************************************************/

#define SAVE_ARGUMENT_REGISTERS \
	stmfd   sp!, {a0,a1,a2,a3,lr}

#define SAVE_ARGUMENT_REGISTERS_IP \
	stmfd   sp!, {a0,a1,a2,a3,ip,lr}

#if defined(ENABLE_SOFTFLOAT)
# define SAVE_FLOAT_REGISTERS
#else
# define SAVE_FLOAT_REGISTERS \
	sfmfd   f0, 4, [sp]!; \
	sfmfd   f4, 4, [sp]!
#endif

#define SAVE_SCRATCH_REGISTERS \
	stmfd   sp!, {itmp3,itmp1,itmp2,lr}


#define RESTORE_ARGUMENT_REGISTERS \
	ldmfd   sp!, {a0,a1,a2,a3,lr}

#define RESTORE_ARGUMENT_REGISTERS_IP \
	ldmfd   sp!, {a0,a1,a2,a3,ip,lr}

#if defined(ENABLE_SOFTFLOAT)
# define RESTORE_FLOAT_REGISTERS
#else
# define RESTORE_FLOAT_REGISTERS \
	lfmfd   f4, 4, [sp]!; \
	lfmfd   f0, 4, [sp]!
#endif

#define RESTORE_SCRATCH_REGS_AND_RETURN \
	ldmfd   sp!, {itmp3,itmp1,itmp2,pc}

#endif // MD_ASM_HPP_


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
