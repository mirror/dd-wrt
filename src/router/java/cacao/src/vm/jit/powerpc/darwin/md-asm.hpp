/* src/vm/jit/powerpc/darwin/md-asm.hpp - assembler defines for PowerPC Darwin ABI

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

/* define register names compatible with Linux names **************************/

#define fr0     f0
#define fr1     f1
#define fr2     f2
#define fr3     f3
#define fr4     f4
#define fr5     f5
#define fr6     f6
#define fr7     f7
#define fr8     f8
#define fr9     f9
#define fr10    f10
#define fr11    f11
#define fr12    f12
#define fr13    f13
#define fr14    f14
#define fr15    f15
#define fr16    f16
#define fr17    f17
#define fr18    f18
#define fr19    f19
#define fr20    f20
#define fr21    f21
#define fr22    f22
#define fr23    f23
#define fr24    f24
#define fr25    f25
#define fr26    f26
#define fr27    f27
#define fr28    f28
#define fr29    f29
#define fr30    f30
#define fr31    f31


/* register defines ***********************************************************/

#define zero  r0
#define sp    r1

#define t0    r2

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
#define t1    r17
#define t2    r18
#define t3    r19
#define t4    r20
#define t5    r21
#define t6    r22
#define t7    r23

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
#define mptrn 12


#define ftmp3 f0

#define fa0   f1
#define fa1   f2
#define fa2   f3
#define fa3   f4
#define fa4   f5
#define fa5   f6
#define fa6   f7

#define fa7   f8
#define fa8   f9
#define fa9   f10
#define fa10  f11
#define fa11  f12
#define fa12  f13

#define fs0   f14
#define fs1   f15

#define ftmp1 f16
#define ftmp2 f17

#define ft0   f18
#define ft1   f19
#define ft2   f20
#define ft3   f21
#define ft4   f22
#define ft5   f23

#define fs2   f24
#define fs3   f25
#define fs4   f26
#define fs5   f27
#define fs6   f28
#define fs7   f29
#define fs8   f30
#define fs9   f31

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
	stfd    fa0,(8+(off))*4)(sp); \
	stfd    fa1,(10+(off))*4)(sp); \
	stfd    fa2,(12+(off))*4)(sp); \
	stfd    fa3,(14+(off))*4)(sp); \
	stfd    fa4,(16+(off))*4)(sp); \
	stfd    fa5,(18+(off))*4)(sp); \
	stfd    fa6,(20+(off))*4)(sp); \
	stfd    fa7,(22+(off))*4)(sp); \
	stfd    fa8,(24+(off))*4)(sp); \
	stfd    fa9,(26+(off))*4)(sp); \
	stfd    fa10,(28+(off))*4)(sp); \
	stfd    fa11,(30+(off))*4)(sp); \
	stfd    fa12,(32+(off))*4)(sp);

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
	lfd     fa0,(8+(off))*4)(sp); \
	lfd     fa1,(10+(off))*4)(sp); \
	lfd     fa2,(12+(off))*4)(sp); \
	lfd     fa3,(14+(off))*4)(sp); \
	lfd     fa4,(16+(off))*4)(sp); \
	lfd     fa5,(18+(off))*4)(sp); \
	lfd     fa6,(20+(off))*4)(sp); \
	lfd     fa7,(22+(off))*4)(sp); \
	lfd     fa8,(24+(off))*4)(sp); \
	lfd     fa9,(26+(off))*4)(sp); \
	lfd     fa10,(28+(off))*4)(sp); \
	lfd     fa11,(30+(off))*4)(sp); \
	lfd     fa12,(32+(off))*4)(sp);


/* Defines for darwin's old gnu assembler *************************************/

/* internal defines ***********************************************************/

#define asm_vm_call_method                    _asm_vm_call_method
#define asm_vm_call_method_int                _asm_vm_call_method_int
#define asm_vm_call_method_long               _asm_vm_call_method_long
#define asm_vm_call_method_float              _asm_vm_call_method_float
#define asm_vm_call_method_double             _asm_vm_call_method_double
#define asm_vm_call_method_end                _asm_vm_call_method_end

#define asm_vm_call_method_exception_handler  _asm_vm_call_method_exception_handler

#define asm_handle_nat_exception              _asm_handle_nat_exception
#define asm_handle_exception                  _asm_handle_exception

#define asm_abstractmethoderror               _asm_abstractmethoderror

#define asm_cacheflush                        _asm_cacheflush


/* external defines ***********************************************************/

#define builtin_throw_exception               L_builtin_throw_exception$stub

#define md_asm_codegen_get_pv_from_pc         L_md_asm_codegen_get_pv_from_pc$stub
#define exceptions_handle_exception           L_exceptions_handle_exception$stub

#define exceptions_asm_new_abstractmethoderror \
    L_exceptions_asm_new_abstractmethoderror$stub

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
