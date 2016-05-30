/* src/vm/jit/x86_64/linux/md-asm.hpp - assembler defines for x86_64 Linux ABI

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

#define v0       %rax
#define v0l      %eax
#define itmp1    v0

#define a3       %rcx
#define a2       %rdx

#define t0       %rbx
#define t0l      %ebx

#define sp       %rsp
#define s0       %rbp

#define a1       %rsi
#define a0       %rdi
#define a0l      %edi

#define a4       %r8
#define a5       %r9

#define itmp2    %r10
#define itmp3    %r11

#define s1       %r12
#define s2       %r13
#define s3       %r14
#define s4       %r15


#define bp       s0

#define itmp1l   %eax
#define itmp2l   %r10d
#define itmp3l   %r11d

#define xptr     itmp1
#define xpc      itmp2
#define mptr     itmp2


#define fa0      %xmm0
#define fa1      %xmm1
#define fa2      %xmm2
#define fa3      %xmm3
#define fa4      %xmm4
#define fa5      %xmm5
#define fa6      %xmm6
#define fa7      %xmm7

#define ftmp1    %xmm8
#define ftmp2    %xmm9
#define ftmp3    %xmm10

#define ft0      %xmm11
#define ft1      %xmm12
#define ft2      %xmm13
#define ft3      %xmm14
#define ft4      %xmm15


/* save and restore macros ****************************************************/

#define SAVE_ARGUMENT_REGISTERS(off) \
	mov     a0,(0+(off))*8(sp)   ; \
	mov     a1,(1+(off))*8(sp)   ; \
	mov     a2,(2+(off))*8(sp)   ; \
	mov     a3,(3+(off))*8(sp)   ; \
	mov     a4,(4+(off))*8(sp)   ; \
	mov     a5,(5+(off))*8(sp)   ; \
	\
	movq    fa0,(6+(off))*8(sp)  ; \
	movq    fa1,(7+(off))*8(sp)  ; \
	movq    fa2,(8+(off))*8(sp)  ; \
	movq    fa3,(9+(off))*8(sp)  ; \
	movq    fa4,(10+(off))*8(sp) ; \
	movq    fa5,(11+(off))*8(sp) ; \
	movq    fa6,(12+(off))*8(sp) ; \
	movq    fa7,(13+(off))*8(sp) ;


#define RESTORE_ARGUMENT_REGISTERS(off) \
	mov     (0+(off))*8(sp),a0   ; \
	mov     (1+(off))*8(sp),a1   ; \
	mov     (2+(off))*8(sp),a2   ; \
	mov     (3+(off))*8(sp),a3   ; \
	mov     (4+(off))*8(sp),a4   ; \
	mov     (5+(off))*8(sp),a5   ; \
	\
	movq    (6+(off))*8(sp),fa0  ; \
	movq    (7+(off))*8(sp),fa1  ; \
	movq    (8+(off))*8(sp),fa2  ; \
	movq    (9+(off))*8(sp),fa3  ; \
	movq    (10+(off))*8(sp),fa4 ; \
	movq    (11+(off))*8(sp),fa5 ; \
	movq    (12+(off))*8(sp),fa6 ; \
	movq    (13+(off))*8(sp),fa7 ;


#define SAVE_TEMPORARY_REGISTERS(off) \
	mov     t0,(0+(off))*8(sp)   ; \
	movq    ft0,(1+(off))*8(sp)  ; \
	movq    ft1,(2+(off))*8(sp)  ; \
	movq    ft2,(3+(off))*8(sp)  ; \
	movq    ft3,(4+(off))*8(sp)  ;


#define RESTORE_TEMPORARY_REGISTERS(off) \
	mov     (0+(off))*8(sp),t0   ; \
	movq    (1+(off))*8(sp),ft0  ; \
	movq    (2+(off))*8(sp),ft1  ; \
	movq    (3+(off))*8(sp),ft2  ; \
	movq    (4+(off))*8(sp),ft3  ;

#define PIC_SYMBOL(f) f##@PLT

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
