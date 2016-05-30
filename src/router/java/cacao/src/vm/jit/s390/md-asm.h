/* src/vm/jit/x86_64/md-asm.h - assembler defines for x86_64 Linux ABI

   Copyright (C) 1996-2005, 2006 R. Grafl, A. Krall, C. Kruegel,
   C. Oates, R. Obermaisser, M. Platter, M. Probst, S. Ring,
   E. Steiner, C. Thalinger, D. Thuernbeck, P. Tomsich, C. Ullrich,
   J. Wenninger, Institut f. Computersprachen - TU Wien

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

   Authors: Christian Thalinger

   Changes:

*/


#ifndef _MD_ASM_H
#define _MD_ASM_H

/* register defines ***********************************************************/

#define a0       %r2
#define a1       %r3
#define a2       %r4
#define a3       %r5
#define a4       %r6

#define sp       %r15
#define itmp1    %r1
#define itmp2    %r14
#define itmp3    %r0
#define v0       %r2
#define pv       %r13
#define ra       %r14

#define mptr     itmp1
#define xptr     itmp3
#define xpc      itmp1

#define s0 %r7
#define s1 %r8
#define s2 %r9
#define s3 %r10
#define s4 %r11
#define s5 %r12

#define fa0 %f0
#define fa1 %f2
#define ftmp1 %r4
#define ftmp2 %r6

/* save and restore macros ****************************************************/

/* Volatile float registers (all volatile in terms of C abi) */

#define LOAD_STORE_VOLATILE_FLOAT_REGISTERS(inst, off) \
	inst    %f0, ((0 * 8) + (off))(sp); \
	inst    %f2, ((1 * 8) + (off))(sp); \
	inst    %f1, ((2 * 8) + (off))(sp); \
	inst    %f3, ((3 * 8) + (off))(sp); \
	inst    %f5, ((4 * 8) + (off))(sp); \
	inst    %f7, ((5 * 8) + (off))(sp); \
	inst    %f8, ((6 * 8) + (off))(sp); \
	inst    %f9, ((7 * 8) + (off))(sp); \
	inst    %f10, ((8 * 8) + (off))(sp); \
	inst    %f11, ((9 * 8) + (off))(sp); \
	inst    %f12, ((10 * 8) + (off))(sp); \
	inst    %f13, ((11 * 8) + (off))(sp); \
	inst    %f14, ((12 * 8) + (off))(sp); \
	inst    %f15, ((13 * 8) + (off))(sp); 

#define VOLATILE_FLOAT_REGISTERS_SIZE (14 * 8)

#define LOAD_VOLATILE_FLOAT_REGISTERS(off) LOAD_STORE_VOLATILE_FLOAT_REGISTERS(ld, off)
#define STORE_VOLATILE_FLOAT_REGISTERS(off) LOAD_STORE_VOLATILE_FLOAT_REGISTERS(std, off)

/* Volatile integer registers (all volatile in terms of C abi) */

#define LOAD_STORE_VOLATILE_INTEGER_REGISTERS(instm, inst, off) \
	instm   %r0, %r5, ((0 * 4) + (off))(sp); \
	inst    %r14, ((6 * 4) + (off))(sp);

#define VOLATILE_INTEGER_REGISTERS_SIZE (7 * 4)

#define LOAD_VOLATILE_INTEGER_REGISTERS(off) LOAD_STORE_VOLATILE_INTEGER_REGISTERS(lm, l, off)
#define STORE_VOLATILE_INTEGER_REGISTERS(off) LOAD_STORE_VOLATILE_INTEGER_REGISTERS(stm, st, off)

/* Argument registers (in terms of JAVA an C abi) */

#define ARGUMENT_REGISTERS_SIZE ((5 * 4) + (2 * 8))

#define LOAD_STORE_ARGUMENT_REGISTERS(iinst, finst, off) \
	iinst %r2, %r6, (off)(sp) ; \
	finst %f0, (off +  (5 * 4))(sp) ; \
	finst %f2, (off + (5 * 4) + 8)(sp)

#define STORE_ARGUMENT_REGISTERS(off) LOAD_STORE_ARGUMENT_REGISTERS(stm, std, off)
#define LOAD_ARGUMENT_REGISTERS(off) LOAD_STORE_ARGUMENT_REGISTERS(lm, ld, off)

/* Temporary registers (in terms of JAVA abi) */

#define TEMPORARY_REGISTERS_SIZE ((1 * 4) + (12 * 8))

#define LOAD_STORE_TEMPORARY_REGISTERS(iinst, finst, off) \
	finst    %f1, ((0 * 8) + (off))(sp); \
	finst    %f3, ((1 * 8) + (off))(sp); \
	finst    %f5, ((2 * 8) + (off))(sp); \
	finst    %f7, ((3 * 8) + (off))(sp); \
	finst    %f8, ((4 * 8) + (off))(sp); \
	finst    %f9, ((5 * 8) + (off))(sp); \
	finst    %f10, ((6 * 8) + (off))(sp); \
	finst    %f11, ((7 * 8) + (off))(sp); \
	finst    %f12, ((8 * 8) + (off))(sp); \
	finst    %f13, ((9 * 8) + (off))(sp); \
	finst    %f14, ((10 * 8) + (off))(sp); \
	finst    %f15, ((11 * 8) + (off))(sp); \
	iinst    %r0, ((12 * 8) + (off))(sp);

#define LOAD_TEMPORARY_REGISTERS(off) LOAD_STORE_TEMPORARY_REGISTERS(l, ld, off)
#define STORE_TEMPORARY_REGISTERS(off) LOAD_STORE_TEMPORARY_REGISTERS(st, std, off)

#endif /* _MD_ASM_H */


/*
 * These are local overrides for various environment variables in Emacs.
 * Please do not remove this and leave it at the end of the file, where
 * Emacs will automagically detect them.
 * ---------------------------------------------------------------------
 * Local variables:
 * mode: c
 * indent-tabs-mode: t
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 */
