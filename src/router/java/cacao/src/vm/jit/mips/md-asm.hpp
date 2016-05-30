/* src/vm/jit/mips/md-asm.hpp - assembler defines for MIPS ABI

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


#ifndef MD_ASM_HPP_
#define MD_ASM_HPP_ 1

#include "config.h"


/* register defines ***********************************************************/

#if SIZEOF_VOID_P == 8

#define zero    $0
#define itmp1   $1
#define v0      $2
#define itmp2   $3
#define a0      $4
#define a1      $5
#define a2      $6
#define a3      $7

#define a4      $8
#define	a5      $9
#define	a6      $10
#define	a7      $11
#define	t0      $12
#define t1      $13
#define t2      $14
#define t3      $15

#define s0      $16
#define s1      $17
#define s2      $18
#define s3      $19
#define s4      $20
#define s5      $21
#define s6      $22
#define s7      $23

#define t4      $24
#define itmp3   $25
#define k0      $26
#define k1      $27

#define gp      $28
#define sp      $29
#define s8      $30
#define ra      $31

#define pv      s8

#define xptr    itmp1
#define xpc     itmp2
#define mptr    itmp3
#define mptrreg 25


#define fv0     $f0
#define ft0     $f1
#define ft1     $f2
#define ft2     $f3
#define ft3     $f4
#define ft4     $f5
#define ft5     $f6
#define ft6     $f7

#define ft7     $f8
#define ft8     $f9
#define ft9     $f10
#define ft10    $f11
#define fa0     $f12
#define fa1     $f13
#define fa2     $f14
#define fa3     $f15

#define fa4     $f16
#define fa5     $f17
#define fa6     $f18
#define fa7     $f19
#define ft11    $f20
#define ft12    $f21
#define ft13    $f22
#define ft14    $f23

#define fs0     $f24
#define ft15    $f25
#define fs1     $f26
#define ft16    $f27
#define fs2     $f28
#define ft17    $f29
#define fs3     $f30
#define ft18    $f31

#define fss0    $f20
#define fss1    $f22
#define fss2    $f25
#define fss3    $f27
#define fss4    $f29
#define fss5    $f31

#else /* SIZEOF_VOID_P == 8 */

#define zero    $0
#define itmp1   $1
#define v0      $2
#define v1      $3
#define a0      $4
#define a1      $5
#define a2      $6
#define a3      $7

#define t0      $8
#define t1      $9
#define t2      $10
#define t3      $11
#define t4      $12
#define t5      $13
#define t6      $14
#define t7      $15

#define s0      $16
#define s1      $17
#define s2      $18
#define s3      $19
#define s4      $20
#define s5      $21
#define s6      $22
#define s7      $23

#define itmp2   $24
#define itmp3   $25
#define k0      $26
#define k1      $27

#define gp      $28
#define sp      $29
#define s8      $30
#define ra      $31


#define pv      s8
#define xptr    itmp1
#define xpc     itmp2
#define mptr    itmp3
#define mptrreg 25


#define fv0     $f0
#define ftmp1   $f2
#define ftmp2   $f4
#define ftmp3   $f6

#define ft0     $f8
#define ft1     $f10
#define fa0     $f12
#define fa1     $f14

#define ft2     $f16
#define ft3     $f18
#define fs0     $f20
#define fs1     $f22

#define fs2     $f24
#define fs3     $f26
#define fs4     $f28
#define fs5     $f30

#endif /* SIZEOF_VOID_P == 8 */

#if SIZEOF_VOID_P == 8

#define aaddu   daddu
#define asubu   dsubu
#define aaddi   daddi
#define aaddiu  daddiu
#define ald     ld
#define ast     sd
#define ala     dla
#define asll    dsll

#define all     lld
#define asc     scd

#else

#define aaddu   addu
#define asubu   subu
#define aaddi   addi
#define aaddiu  addiu
#define ald     lw
#define ast     sw
#define ala     la
#define asll    sll

#define all     ll
#define asc     sc

#endif


/* save and restore macros ****************************************************/

#if SIZEOF_VOID_P == 8

#define SAVE_ARGUMENT_REGISTERS(off) \
	sd      a0,(0+(off))*8(sp)	; \
	sd      a1,(1+(off))*8(sp)	; \
	sd      a2,(2+(off))*8(sp)	; \
	sd      a3,(3+(off))*8(sp)	; \
	sd      a4,(4+(off))*8(sp)	; \
	sd      a5,(5+(off))*8(sp)	; \
	sd      a6,(6+(off))*8(sp)	; \
	sd      a7,(7+(off))*8(sp)	; \
	\
	s.d     fa0,(8+(off))*8(sp)	; \
	s.d     fa1,(9+(off))*8(sp)	; \
	s.d     fa2,(10+(off))*8(sp); \
	s.d     fa3,(11+(off))*8(sp); \
	s.d     fa4,(12+(off))*8(sp); \
	s.d     fa5,(13+(off))*8(sp); \
	s.d     fa6,(14+(off))*8(sp); \
	s.d     fa7,(15+(off))*8(sp); 

#define RESTORE_ARGUMENT_REGISTERS(off) \
	ld      a0,(0+(off))*8(sp)	; \
	ld      a1,(1+(off))*8(sp)	; \
	ld      a2,(2+(off))*8(sp)	; \
	ld      a3,(3+(off))*8(sp)	; \
	ld      a4,(4+(off))*8(sp)	; \
	ld      a5,(5+(off))*8(sp)	; \
	ld      a6,(6+(off))*8(sp)	; \
	ld      a7,(7+(off))*8(sp)	; \
	\
	l.d     fa0,(8+(off))*8(sp); \
	l.d     fa1,(9+(off))*8(sp); \
	l.d     fa2,(10+(off))*8(sp); \
	l.d     fa3,(11+(off))*8(sp); \
	l.d     fa4,(12+(off))*8(sp); \
	l.d     fa5,(13+(off))*8(sp); \
	l.d     fa6,(14+(off))*8(sp); \
	l.d     fa7,(15+(off))*8(sp); 


#define SAVE_TEMPORARY_REGISTERS(off) \
	sd      t0,(0+(off))*8(sp)	; \
	sd      t1,(1+(off))*8(sp)	; \
	sd      t2,(2+(off))*8(sp)	; \
	sd      t3,(3+(off))*8(sp)	; \
	sd      t4,(4+(off))*8(sp)	; \
	\
	s.d     ft3,(5+(off))*8(sp)	; \
	s.d     ft4,(6+(off))*8(sp)	; \
	s.d     ft5,(7+(off))*8(sp)	; \
	s.d     ft6,(8+(off))*8(sp)	; \
	s.d     ft7,(9+(off))*8(sp)	; \
	s.d     ft8,(10+(off))*8(sp)	; \
	s.d     ft9,(11+(off))*8(sp)	; \
	s.d     ft10,(12+(off))*8(sp)	; \
	s.d     ft11,(13+(off))*8(sp)	; \
	s.d     ft12,(14+(off))*8(sp)	; \
	s.d     ft13,(15+(off))*8(sp)	; \
	s.d     ft14,(16+(off))*8(sp)	; \
	s.d     ft15,(17+(off))*8(sp)	; \
	s.d     ft16,(18+(off))*8(sp)	; \
	s.d     ft17,(19+(off))*8(sp)	; \
	s.d     ft18,(20+(off))*8(sp)	;

#define RESTORE_TEMPORARY_REGISTERS(off) \
	ld      t0,(0+(off))*8(sp)	; \
	ld      t1,(1+(off))*8(sp)	; \
	ld      t2,(2+(off))*8(sp)	; \
	ld      t3,(3+(off))*8(sp)	; \
	ld      t4,(4+(off))*8(sp)	; \
	\
	l.d     ft3,(5+(off))*8(sp)	; \
	l.d     ft4,(6+(off))*8(sp)	; \
	l.d     ft5,(7+(off))*8(sp)	; \
	l.d     ft6,(8+(off))*8(sp)	; \
	l.d     ft7,(9+(off))*8(sp)	; \
	l.d     ft8,(10+(off))*8(sp)	; \
	l.d     ft9,(11+(off))*8(sp)	; \
	l.d     ft10,(12+(off))*8(sp)	; \
	l.d     ft11,(13+(off))*8(sp)	; \
	l.d     ft12,(14+(off))*8(sp)	; \
	l.d     ft13,(15+(off))*8(sp)	; \
	l.d     ft14,(16+(off))*8(sp)	; \
	l.d     ft15,(17+(off))*8(sp)	; \
	l.d     ft16,(18+(off))*8(sp)	; \
	l.d     ft17,(19+(off))*8(sp)	; \
	l.d     ft18,(20+(off))*8(sp)	;

#else /* SIZEOF_VOID_P == 8 */

#define SAVE_ARGUMENT_REGISTERS(off) \
	sw      a0,(0+(off))*4(sp)	; \
	sw      a1,(1+(off))*4(sp)	; \
	sw      a2,(2+(off))*4(sp)	; \
	sw      a3,(3+(off))*4(sp)	; \
	s.d     fa0,(4+(off))*4(sp) ; \
	s.d     fa1,(6+(off))*4(sp) ;

#define RESTORE_ARGUMENT_REGISTERS(off) \
	lw      a0,(0+(off))*4(sp)	; \
	lw      a1,(1+(off))*4(sp)	; \
	lw      a2,(2+(off))*4(sp)	; \
	lw      a3,(3+(off))*4(sp)	; \
	l.d     fa0,(4+(off))*4(sp) ; \
	l.d     fa1,(6+(off))*4(sp) ;


#define SAVE_TEMPORARY_REGISTERS(off) \
	sw      t0,(0+(off))*4(sp)	; \
	sw      t1,(1+(off))*4(sp)	; \
	sw      t2,(2+(off))*4(sp)	; \
	sw      t3,(3+(off))*4(sp)	; \
	sw      t4,(4+(off))*4(sp)	; \
	sw      t5,(5+(off))*4(sp)	; \
	sw      t6,(6+(off))*4(sp)	; \
	sw      t7,(7+(off))*4(sp)	; \
	s.d     ft0,(8+(off))*4(sp) ; \
	s.d     ft1,(10+(off))*4(sp) ; \
	s.d     ft2,(12+(off))*4(sp) ; \
	s.d     ft3,(14+(off))*4(sp) ;

#define RESTORE_TEMPORARY_REGISTERS(off) \
	lw      t0,(0+(off))*4(sp)	; \
	lw      t1,(1+(off))*4(sp)	; \
	lw      t2,(2+(off))*4(sp)	; \
	lw      t3,(3+(off))*4(sp)	; \
	lw      t4,(4+(off))*4(sp)	; \
	lw      t5,(5+(off))*4(sp)	; \
	lw      t6,(6+(off))*4(sp)	; \
	lw      t7,(7+(off))*4(sp)	; \
	l.d     ft0,(8+(off))*4(sp) ; \
	l.d     ft1,(10+(off))*4(sp) ; \
	l.d     ft2,(12+(off))*4(sp) ; \
	l.d     ft3,(14+(off))*4(sp) ;

#endif /* SIZEOF_VOID_P == 8 */

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
