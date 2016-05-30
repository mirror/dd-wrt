/* src/vm/jit/arm/md-trap.hpp - ARM hardware traps

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


#ifndef MD_TRAP_HPP_
#define MD_TRAP_HPP_ 1

#include "config.h"


/**
 * Trap number defines.
 *
 * On this architecture (arm) we use illegal instructions as trap
 * instructions.  Since the illegal instruction with the value 1 is
 * used by the kernel to generate a SIGTRAP, we skip this one.
 */

#define TRAP_INSTRUCTION_IS_LOAD    0

enum {
	TRAP_NullPointerException           = 0,

	/* Skip 1 because it's the SIGTRAP illegal instruction. */

	TRAP_ArithmeticException            = 2,
	TRAP_ArrayIndexOutOfBoundsException = 3,
	TRAP_ArrayStoreException            = 4,
	TRAP_ClassCastException             = 5,
	TRAP_CHECK_EXCEPTION                = 6,
	TRAP_PATCHER                        = 7,
	TRAP_COMPILER                       = 8,
	TRAP_COUNTDOWN                      = 9
};


/**
 * Macro to fixup a compiler stub. The XPC is the RA minus 4,
 * because the RA points to the instruction after the call.
 */
#define MD_TRAP_COMPILER_FIXUP(xpc, ra, sp, pv) \
	do { \
		(xpc) = (void*) (((uintptr_t) (ra)) - 4); \
	} while(0)


#endif // MD_TRAP_HPP_


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
