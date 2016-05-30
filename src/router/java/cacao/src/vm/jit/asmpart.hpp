/* src/vm/jit/asmpart.hpp - prototypes for machine specfic functions

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


#ifndef ASMPART_HPP_
#define ASMPART_HPP_ 1

#include "config.h"

#include <stdint.h>

#include "vm/types.hpp"

#include "vm/global.hpp"
#include "vm/vm.hpp"


/* function prototypes ********************************************************/

extern "C" {

/* machine dependent initialization */
s4   asm_md_init(void);

#if !defined(JIT_COMPILER_VIA_SIGNAL)
/* 
   invokes the compiler for untranslated JavaVM methods.
   Register R0 contains a pointer to the method info structure
   (prepared by createcompilerstub).
*/
void asm_call_jit_compiler(void);
#endif

#if defined(ENABLE_JIT)
java_object_t *asm_vm_call_method(void *pv, uint64_t *array, int32_t stackargs);
int32_t        asm_vm_call_method_int(void *pv, uint64_t *array, int32_t stackargs);

int64_t        asm_vm_call_method_long(void *pv, uint64_t *array, int32_t stackargs);
float          asm_vm_call_method_float(void *pv, uint64_t *array, int32_t stackargs);
double         asm_vm_call_method_double(void *pv, uint64_t *array, int32_t stackargs);

void   asm_vm_call_method_exception_handler(void);
void   asm_vm_call_method_end(void);
#endif

#if defined(ENABLE_INTRP)
java_objectheader *intrp_asm_vm_call_method(methodinfo *m, s4 vmargscount,
											vm_arg *vmargs);

s4     intrp_asm_vm_call_method_int(methodinfo *m, s4 vmargscount,
									vm_arg *vmargs);
s8     intrp_asm_vm_call_method_long(methodinfo *m, s4 vmargscount,
									 vm_arg *vmargs);
float  intrp_asm_vm_call_method_float(methodinfo *m, s4 vmargscount,
									  vm_arg *vmargs);
double intrp_asm_vm_call_method_double(methodinfo *m, s4 vmargscount,
									   vm_arg *vmargs);
#endif

/* exception handling functions */

#if defined(ENABLE_JIT)
void asm_handle_exception(void);
void asm_handle_nat_exception(void);
#endif

/* stub for throwing AbstractMethodError's */
#if defined(ENABLE_JIT)
void asm_abstractmethoderror(void);
#endif

#if defined(ENABLE_INTRP)
void intrp_asm_abstractmethoderror(void);
#endif

/* wrapper for code patching functions */
void asm_patcher_wrapper(void);

/* cache flush function */
void asm_cacheflush(void* addr, int nbytes);

void *md_asm_codegen_get_pv_from_pc(void *ra);

#if defined(ENABLE_ESCAPE_CHECK)
void asm_escape_check(java_object_t *obj);
#endif

} // extern "C"

#endif // _ASMPART_HPP_


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
