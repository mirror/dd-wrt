/* src/vm/jit/trace.hpp - Functions for tracing from java code.

   Copyright (C) 1996-2005, 2006, 2007, 2008
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


#ifndef _VM_JIT_TRACE_HPP
#define _VM_JIT_TRACE_HPP

#include <stdint.h>                     // for uint64_t
#include "config.h"

struct java_object_t;
struct methodinfo;


#if !defined(NDEBUG)

void trace_java_call_enter(methodinfo *m, uint64_t *arg_regs, uint64_t *stack);
void trace_java_call_exit(methodinfo *m, uint64_t *return_regs);

void trace_exception(java_object_t *xptr, methodinfo *m, void *pos);
void trace_exception_builtin(java_object_t *xptr);

#endif /* !defined(NDEBUG) */

#endif // _VM_JIT_TRACE_HPP


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
