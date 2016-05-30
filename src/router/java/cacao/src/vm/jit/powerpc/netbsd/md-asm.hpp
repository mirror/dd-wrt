/* src/vm/jit/powerpc/netbsd/md-asm.hpp - assembler defines for PowerPC NetBSD ABI

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

   Changes:

*/


#ifndef MD_ASM_HPP_
#define MD_ASM_HPP_ 1

#include <asm/ppc_asm.h>


/* register defines ***********************************************************/

#define zero  r0
#define sp    r1

/* #define XXX   r2  -  system reserved register */

#define a0    r3
#define a1    r4
#define a2    r5
#define a3    r6
#define a4    r7
#define a5    r8
#define a6    r9
#define a7    r10

#define itmp1 r11
#define itmp2 r12
#define pv    r13

#define s0    r14
#define s1    r15

#define itmp3 r16
#define t0    r17
#define t1    r18
#define t2    r19
#define t3    r20
#define t4    r21
#define t5    r22
#define t6    r23

#define s2    r24
#define s3    r25
#define s4    r26
#define s5    r27
#define s6    r28
#define s7    r29
#define s8    r30
#define s9    r31

#define v0    a0
#define v1    a1

#define xptr  itmp1
#define xpc   itmp2

#define mptr  r12
#define mptrn 12


#define ftmp3 fr0

#define fa0   fr1
#define fa1   fr2
#define fa2   fr3
#define fa3   fr4
#define fa4   fr5
#define fa5   fr6
#define fa6   fr7
#define fa7   fr8

#define fa8   fr9
#define fa9   fr10
#define fa10  fr11
#define fa11  fr12
#define fa12  fr13

#define fs0   fr14
#define fs1   fr15

#define ftmp1 fr16
#define ftmp2 fr17

#define ft0   fr18
#define ft1   fr19
#define ft2   fr20
#define ft3   fr21
#define ft4   fr22
#define ft5   fr23

#define fs2   fr24
#define fs3   fr25
#define fs4   fr26
#define fs5   fr27
#define fs6   fr28
#define fs7   fr29
#define fs8   fr30
#define fs9   fr31

#define fv0   fa0


/* save and restore macros ****************************************************/

#define SAVE_ARGUMENT_REGISTERS(off) \
	stw     a0,(0+(off))*4(sp); \
	stw     a1,(1+(off))*4(sp); \
	stw     a2,(2+(off))*4(sp); \
	stw     a3,(3+(off))*4(sp); \
	stw     a4,(4+(off))*4(sp); \
	stw     a5,(5+(off))*4(sp); \
	stw     a6,(6+(off))*4(sp); \
	stw     a7,(7+(off))*4(sp); \
	\
	stfd    fa0,(8+(off))*4(sp); \
	stfd    fa1,(10+(off))*4(sp); \
	stfd    fa2,(12+(off))*4(sp); \
	stfd    fa3,(14+(off))*4(sp); \
	stfd    fa4,(16+(off))*4(sp); \
	stfd    fa5,(18+(off))*4(sp); \
	stfd    fa6,(20+(off))*4(sp); \
	stfd    fa7,(22+(off))*4(sp);

#define RESTORE_ARGUMENT_REGISTERS(off) \
	lwz     a0,(0+(off))*4(sp); \
	lwz     a1,(1+(off))*4(sp); \
	lwz     a2,(2+(off))*4(sp); \
	lwz     a3,(3+(off))*4(sp); \
	lwz     a4,(4+(off))*4(sp); \
	lwz     a5,(5+(off))*4(sp); \
	lwz     a6,(6+(off))*4(sp); \
	lwz     a7,(7+(off))*4(sp); \
	\
	lfd     fa0,(8+(off))*4(sp); \
	lfd     fa1,(10+(off))*4(sp); \
	lfd     fa2,(12+(off))*4(sp); \
	lfd     fa3,(14+(off))*4(sp); \
	lfd     fa4,(16+(off))*4(sp); \
	lfd     fa5,(18+(off))*4(sp); \
	lfd     fa6,(20+(off))*4(sp); \
	lfd     fa7,(22+(off))*4(sp);


#define SAVE_TEMPORARY_REGISTERS(off) \
	stw     t0,(0+(off))*4(sp); \
	stw     t1,(1+(off))*4(sp); \
	stw     t2,(2+(off))*4(sp); \
	stw     t3,(3+(off))*4(sp); \
	stw     t4,(4+(off))*4(sp); \
	stw     t5,(5+(off))*4(sp); \
	stw     t6,(6+(off))*4(sp); \
	\
	stfd    ft0,(8+(off))*4(sp); \
	stfd    ft1,(10+(off))*4(sp); \
	stfd    ft2,(12+(off))*4(sp); \
	stfd    ft3,(14+(off))*4(sp); \
	stfd    ft4,(16+(off))*4(sp); \
	stfd    ft5,(18+(off))*4(sp);

#define RESTORE_TEMPORARY_REGISTERS(off) \
	lwz     t0,(0+(off))*4(sp); \
	lwz     t1,(1+(off))*4(sp); \
	lwz     t2,(2+(off))*4(sp); \
	lwz     t3,(3+(off))*4(sp); \
	lwz     t4,(4+(off))*4(sp); \
	lwz     t5,(5+(off))*4(sp); \
	lwz     t6,(6+(off))*4(sp); \
	\
	lfd     ft0,(8+(off))*4(sp); \
	lfd     ft1,(10+(off))*4(sp); \
	lfd     ft2,(12+(off))*4(sp); \
	lfd     ft3,(14+(off))*4(sp); \
	lfd     ft4,(16+(off))*4(sp); \
	lfd     ft5,(18+(off))*4(sp);

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
 */
