/* src/vm/jit/powerpc/linux/md-asm.hpp - assembler defines for PowerPC Linux ABI

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

/* register defines ***********************************************************/

#define r0      0
#define r1      1
#define r2      2
#define r3      3
#define r4      4
#define r5      5
#define r6      6
#define r7      7
#define r8      8
#define r9      9
#define r10     10
#define r11     11
#define r12     12
#define r13     13
#define r14     14
#define r15     15
#define r16     16
#define r17     17
#define r18     18
#define r19     19
#define r20     20
#define r21     21
#define r22     22
#define r23     23
#define r24     24
#define r25     25
#define r26     26
#define r27     27
#define r28     28
#define r29     29
#define r30     30
#define r31     31

#define fr0     0
#define fr1     1
#define fr2     2
#define fr3     3
#define fr4     4
#define fr5     5
#define fr6     6
#define fr7     7
#define fr8     8
#define fr9     9
#define fr10    10
#define fr11    11
#define fr12    12
#define fr13    13
#define fr14    14
#define fr15    15
#define fr16    16
#define fr17    17
#define fr18    18
#define fr19    19
#define fr20    20
#define fr21    21
#define fr22    22
#define fr23    23
#define fr24    24
#define fr25    25
#define fr26    26
#define fr27    27
#define fr28    28
#define fr29    29
#define fr30    30
#define fr31    31


/* register defines ***********************************************************/

#define zero  r0
#define sp    r1

/* #define foo   r2  -  system reserved register */

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

#define mptr  itmp2
#define mptrn itmp2


#define ftmp3 r0

#define fa0   r1
#define fa1   r2
#define fa2   r3
#define fa3   r4
#define fa4   r5
#define fa5   r6
#define fa6   r7
#define fa7   r8

#define fa8   r9
#define fa9   r10
#define fa10  r11
#define fa11  r12
#define fa12  r13

#define fs0   r14
#define fs1   r15

#define ftmp1 r16
#define ftmp2 r17

#define ft0   r18
#define ft1   r19
#define ft2   r20
#define ft3   r21
#define ft4   r22
#define ft5   r23

#define fs2   r24
#define fs3   r25
#define fs4   r26
#define fs5   r27
#define fs6   r28
#define fs7   r29
#define fs8   r30
#define fs9   r31

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
