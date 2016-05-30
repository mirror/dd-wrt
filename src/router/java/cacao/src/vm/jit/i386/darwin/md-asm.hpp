/* src/vm/jit/i386/darwin/md-asm.hpp - assembler defines for i386 Darwin ABI

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

#define v0       %eax
#define itmp1    v0

#define itmp2    %ecx
#define itmp3    %edx

#define t0       %ebx

#define sp       %esp
#define s0       %ebp
#define s1       %esi
#define s2       %edi

#define bp       s0

#define itmp1b   %al

#define xptr     itmp1
#define xpc      itmp2
#define mptr     itmp2


/* save and restore macros ****************************************************/

#define SAVE_ARGUMENT_REGISTERS(off) \
    /* no argument registers */

#define SAVE_TEMPORARY_REGISTERS(off) \
	mov     t0,(0+(off))*4(sp) ;


#define RESTORE_ARGUMENT_REGISTERS(off) \
    /* no argument registers */

#define RESTORE_TEMPORARY_REGISTERS(off) \
	mov     (0+(off))*4(sp),t0 ;


/* defines for darwin's old gnu assembler *************************************/

#define asm_md_init                           _asm_md_init

#define asm_vm_call_method                    _asm_vm_call_method
#define asm_vm_call_method_int                _asm_vm_call_method_int
#define asm_vm_call_method_long               _asm_vm_call_method_long
#define asm_vm_call_method_float              _asm_vm_call_method_float
#define asm_vm_call_method_double             _asm_vm_call_method_double
#define asm_vm_call_method_end                _asm_vm_call_method_end

#define asm_vm_call_method_exception_handler  _asm_vm_call_method_exception_handler

#define asm_call_jit_compiler                 _asm_call_jit_compiler

#define asm_handle_nat_exception              _asm_handle_nat_exception
#define asm_handle_exception                  _asm_handle_exception

#define asm_abstractmethoderror               _asm_abstractmethoderror

#define asm_patcher_wrapper                   _asm_patcher_wrapper

#define asm_builtin_f2i                       _asm_builtin_f2i
#define asm_builtin_f2l                       _asm_builtin_f2l
#define asm_builtin_d2i                       _asm_builtin_d2i
#define asm_builtin_d2l                       _asm_builtin_d2l


/* external defines ***********************************************************/

#define exceptions_get_and_clear_exception    _exceptions_get_and_clear_exception

#define builtin_throw_exception               _builtin_throw_exception
#define methodtree_find                       _methodtree_find
#define exceptions_handle_exception           _exceptions_handle_exception
#define jit_asm_compile                       _jit_asm_compile

#define exceptions_asm_new_abstractmethoderror \
    _exceptions_asm_new_abstractmethoderror

#define patcher_wrapper                       _patcher_wrapper

#define replace_me                            _replace_me

#define abort                                 _abort

#define builtin_f2i                           _builtin_f2i
#define builtin_f2l                           _builtin_f2l
#define builtin_d2i                           _builtin_d2i
#define builtin_d2l                           _builtin_d2l

#endif // MD_ASM_H


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
