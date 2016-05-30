/* src/vm/jit/abi.hpp - common ABI defines

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


#ifndef ABI_HPP_
#define ABI_HPP_ 1

#include "config.h"
#include "vm/types.hpp"

#include "arch.hpp"

#include "vm/jit/abi-asm.hpp"
#include "vm/jit/jit.hpp"
#include "vm/jit/stack.hpp"


/* ABI externs ****************************************************************/

extern s4 nregdescint[];
extern char *regs[];
extern s4 nregdescfloat[];

extern const char *abi_registers_integer_name[];
extern const s4    abi_registers_integer_argument[];
extern const s4    abi_registers_integer_saved[];
extern const s4    abi_registers_integer_temporary[];

extern const s4    abi_registers_float_argument[];
extern const s4    abi_registers_float_saved[];
extern const s4    abi_registers_float_temporary[];


/* function prototypes ********************************************************/

/* machine dependent descriptor function */
void md_param_alloc(methoddesc *md);
void md_param_alloc_native(methoddesc *md);

/* machine dependent return value handling function */
void md_return_alloc(jitdata *jd, stackelement_t *stackslot);

#endif // ABI_HPP_


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
