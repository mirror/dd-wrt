/* src/vm/jit/alpha/md-asm.hpp - assembler defines for Alpha ABI

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


/* register defines ***********************************************************/

#define v0      $0

#define t0      $1
#define t1      $2
#define t2      $3
#define t3      $4
#define t4      $5
#define t5      $6
#define t6      $7
#define t7      $8

#define	s0      $9
#define	s1      $10
#define	s2      $11
#define	s3      $12
#define	s4      $13
#define	s5      $14
#define	s6      $15

#define a0      $16
#define a1      $17
#define a2      $18
#define a3      $19
#define a4      $20
#define a5      $21

#define t8      $22
#define t9      $23
#define t10     $24
#define t11     $25
#define ra      $26
#define t12     $27

#define pv      t12
#define AT      $at
#define gp      $29
#define sp      $30
#define zero    $31

#define itmp1   t11
#define itmp2   $28
#define itmp3   gp

#define xptr    itmp1
#define xpc     itmp2
#define mptr    itmp2

#define fv0     $f0
#define ft0     $f1

#define fs0     $f2
#define fs1     $f3
#define fs2     $f4
#define fs3     $f5
#define fs4     $f6
#define fs5     $f7
#define fs6     $f8
#define fs7     $f9

#define ft1     $f10
#define ft2     $f11
#define ft3     $f12
#define ft4     $f13
#define ft5     $f14
#define ft6     $f15

#define fa0     $f16
#define fa1     $f17
#define fa2     $f18
#define fa3     $f19
#define fa4     $f20
#define fa5     $f21

#define ft7     $f22
#define ft8     $f23
#define ft9     $f24
#define ft10    $f25
#define ft11    $f26
#define ft12    $f27

#define ftmp1   $f28
#define ftmp2   $f29
#define ftmp3   $f30

#define fzero   $f31

#define PAL_imb 0x86


/* save and restore macros ****************************************************/

#define SAVE_ARGUMENT_REGISTERS(off) \
	stq     a0,(0+(off))*8(sp)    ; \
	stq     a1,(1+(off))*8(sp)    ; \
	stq     a2,(2+(off))*8(sp)    ; \
	stq     a3,(3+(off))*8(sp)    ; \
	stq     a4,(4+(off))*8(sp)    ; \
	stq     a5,(5+(off))*8(sp)    ; \
	\
	stt     fa0,(6+(off))*8(sp)   ; \
	stt     fa1,(7+(off))*8(sp)   ; \
	stt     fa2,(8+(off))*8(sp)   ; \
	stt     fa3,(9+(off))*8(sp)   ; \
	stt     fa4,(10+(off))*8(sp)  ; \
	stt     fa5,(11+(off))*8(sp)  ;

#define RESTORE_ARGUMENT_REGISTERS(off) \
	ldq     a0,(0+(off))*8(sp)    ; \
	ldq     a1,(1+(off))*8(sp)    ; \
	ldq     a2,(2+(off))*8(sp)    ; \
	ldq     a3,(3+(off))*8(sp)    ; \
	ldq     a4,(4+(off))*8(sp)    ; \
	ldq     a5,(5+(off))*8(sp)    ; \
	\
	ldt     fa0,(6+(off))*8(sp)   ; \
	ldt     fa1,(7+(off))*8(sp)   ; \
	ldt     fa2,(8+(off))*8(sp)   ; \
	ldt     fa3,(9+(off))*8(sp)   ; \
	ldt     fa4,(10+(off))*8(sp)  ; \
	ldt     fa5,(11+(off))*8(sp)  ;
		
#define SAVE_TEMPORARY_REGISTERS(off) \
	stq     t0,(0+(off))*8(sp)    ; \
	stq     t1,(1+(off))*8(sp)    ; \
	stq     t2,(2+(off))*8(sp)    ; \
	stq     t3,(3+(off))*8(sp)    ; \
	stq     t4,(4+(off))*8(sp)    ; \
	stq     t5,(5+(off))*8(sp)    ; \
	stq     t6,(6+(off))*8(sp)    ; \
	stq     t7,(7+(off))*8(sp)    ; \
	stq     t8,(8+(off))*8(sp)    ; \
	stq     t9,(9+(off))*8(sp)    ; \
	stq     t10,(10+(off))*8(sp)  ; \
	\
	stt     ft0,(11+(off))*8(sp)  ; \
	stt     ft1,(12+(off))*8(sp)  ; \
	stt     ft2,(13+(off))*8(sp)  ; \
	stt     ft3,(14+(off))*8(sp)  ; \
	stt     ft4,(15+(off))*8(sp)  ; \
	stt     ft5,(16+(off))*8(sp)  ; \
	stt     ft6,(17+(off))*8(sp)  ; \
	stt     ft7,(18+(off))*8(sp)  ; \
	stt     ft8,(19+(off))*8(sp)  ; \
	stt     ft9,(20+(off))*8(sp)  ; \
	stt     ft10,(21+(off))*8(sp) ; \
	stt     ft11,(22+(off))*8(sp) ; \
	stt     ft12,(23+(off))*8(sp) ;


#define RESTORE_TEMPORARY_REGISTERS(off) \
	ldq     t0,(0+(off))*8(sp)    ; \
	ldq     t1,(1+(off))*8(sp)    ; \
	ldq     t2,(2+(off))*8(sp)    ; \
	ldq     t3,(3+(off))*8(sp)    ; \
	ldq     t4,(4+(off))*8(sp)    ; \
	ldq     t5,(5+(off))*8(sp)    ; \
	ldq     t6,(6+(off))*8(sp)    ; \
	ldq     t7,(7+(off))*8(sp)    ; \
	ldq     t8,(8+(off))*8(sp)    ; \
	ldq     t9,(9+(off))*8(sp)    ; \
	ldq     t10,(10+(off))*8(sp)  ; \
	\
	ldt     ft0,(11+(off))*8(sp)  ; \
	ldt     ft1,(12+(off))*8(sp)  ; \
	ldt     ft2,(13+(off))*8(sp)  ; \
	ldt     ft3,(14+(off))*8(sp)  ; \
	ldt     ft4,(15+(off))*8(sp)  ; \
	ldt     ft5,(16+(off))*8(sp)  ; \
	ldt     ft6,(17+(off))*8(sp)  ; \
	ldt     ft7,(18+(off))*8(sp)  ; \
	ldt     ft8,(19+(off))*8(sp)  ; \
	ldt     ft9,(20+(off))*8(sp)  ; \
	ldt     ft10,(21+(off))*8(sp) ; \
	ldt     ft11,(22+(off))*8(sp) ; \
	ldt     ft12,(23+(off))*8(sp) ;

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
