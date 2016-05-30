/* src/vm/jit/sparc64/md-asm.hpp - assembler defines for Sparc ABI

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

   Authors: Christian Thalinger
            Alexander Jordan

   Changes:

*/

#ifndef MD_ASM_HPP_
#define MD_ASM_HPP_ 1


/* register defines
 * ***********************************************************/

#define zero	%g0
#define itmp1	%g1
#define itmp2	%g2
#define itmp3	%g3

/* PLT unsafe temp regs */
#define temp4 %g4
#define temp5 %g5

#define mptr_itmp2 itmp2

#define xptr_itmp2 itmp2
#define xpc_itmp3  itmp3

#define ra_caller  %o7
#define ra_callee  %i7

#define pv_caller  %o5
#define pv_callee  %i5


#define fv0     %f0
#define ft0     %f2
#define ft1     %f4
#define ft2     %f6
#define ft3     %f8
#define ft4     %f10
#define ft5     %f12
#define ft6     %f14

#define fa0     %f16
#define fa0f    %f17
#define fa1     %f18
#define fa1f    %f19
#define fa2     %f20
#define fa2f    %f21
#define fa3     %f22
#define fa3f    %f23
#define fa4     %f24
#define fa4f    %f25
#define ft7     %f26
#define ft8     %f28
#define ft9     %f30


#define bias    2047


/* save and restore macros ****************************************************/

#define SAVE_FLOAT_RETURN_REGISTER(off) \
	std     fv0,[%sp + bias + ((off)*8)] ;

#define RESTORE_FLOAT_RETURN_REGISTER(off) \
	ldd     [%sp + bias + ((off)*8)],fv0 ;


#define SAVE_FLOAT_ARGUMENT_REGISTERS(off) \
	std     fa0,[%sp + bias + ((0+(off))*8)] ; \
	std     fa1,[%sp + bias + ((1+(off))*8)] ; \
	std     fa2,[%sp + bias + ((2+(off))*8)] ; \
	std     fa3,[%sp + bias + ((3+(off))*8)] ; \
	std     fa4,[%sp + bias + ((4+(off))*8)] ;


#define RESTORE_FLOAT_ARGUMENT_REGISTERS(off) \
	ldd     [%sp + bias + ((0+(off))*8)],fa0 ; \
	ldd     [%sp + bias + ((1+(off))*8)],fa1 ; \
	ldd     [%sp + bias + ((2+(off))*8)],fa2 ; \
	ldd     [%sp + bias + ((3+(off))*8)],fa3 ; \
	ldd     [%sp + bias + ((4+(off))*8)],fa4 ;
	
#define SAVE_FLOAT_TEMPORARY_REGISTERS(off) \
	std     ft0,[%sp + bias + ((0+(off))*8)] ; \
	std     ft1,[%sp + bias + ((1+(off))*8)] ; \
	std     ft2,[%sp + bias + ((2+(off))*8)] ; \
	std     ft3,[%sp + bias + ((3+(off))*8)] ; \
	std     ft4,[%sp + bias + ((4+(off))*8)] ; \
	std     ft5,[%sp + bias + ((5+(off))*8)] ; \
	std     ft6,[%sp + bias + ((6+(off))*8)] ; \
	std     ft7,[%sp + bias + ((7+(off))*8)] ; \
	std     ft8,[%sp + bias + ((8+(off))*8)] ; \
	std     ft9,[%sp + bias + ((9+(off))*8)] ; \

#define RESTORE_FLOAT_TEMPORARY_REGISTERS(off) \
	ldd     [%sp + bias + ((0+(off))*8)],ft0 ; \
	ldd     [%sp + bias + ((1+(off))*8)],ft1 ; \
	ldd     [%sp + bias + ((2+(off))*8)],ft2 ; \
	ldd     [%sp + bias + ((3+(off))*8)],ft3 ; \
	ldd     [%sp + bias + ((4+(off))*8)],ft4 ; \
	ldd     [%sp + bias + ((5+(off))*8)],ft5 ; \
	ldd     [%sp + bias + ((6+(off))*8)],ft6 ; \
	ldd     [%sp + bias + ((7+(off))*8)],ft7 ; \
	ldd     [%sp + bias + ((8+(off))*8)],ft8 ; \
	ldd     [%sp + bias + ((9+(off))*8)],ft9 ; \
	
	
	
#endif // MD_ASM_HPP_


/*
 * These are local overrides for various environment variables in Emacs.
 * Please do not remove this and leave it at the end of the file, where
 * Emacs will automagically detect them.
 *
 * ---------------------------------------------------------------------
 * Local variables:
 * mode: c++
 * indent-tabs-mode: t
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 */

