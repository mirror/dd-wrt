/* src/vm/jit/s390/md-trap.h - s390 hardware traps

   Copyright (C) 2008, 2010
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


#ifndef _MD_TRAP_H
#define _MD_TRAP_H

#include "config.h"


/**
 * Trap number defines.
 *
 * On this architecture (s390) we use illegal instructions as trap
 * instructions.
 */

#define TRAP_INSTRUCTION_IS_LOAD    0

enum {
	TRAP_NullPointerException           = 0,
	TRAP_ArithmeticException            = 1,
	TRAP_ArrayIndexOutOfBoundsException = 2,
	TRAP_ArrayStoreException            = 3,
	TRAP_ClassCastException             = 4,
	TRAP_CHECK_EXCEPTION                = 5,
	TRAP_PATCHER                        = 6,
	TRAP_COMPILER                       = 7,
	TRAP_COUNTDOWN                      = 8
};


/**
 * Macro to fixup a compiler stub. The XPC is the RA minus 2,
 * because the RA points to the instruction after the call.
 */
#define MD_TRAP_COMPILER_FIXUP(xpc, ra, sp, pv) \
	do { \
		(pv) = (xpc); \
		(xpc) = (void*) (((uintptr_t) (ra)) - 2); \
	} while(0)


#endif /* _MD_TRAP_H */


/*
 * These are local overrides for various environment variables in Emacs.
 * Please do not remove this and leave it at the end of the file, where
 * Emacs will automagically detect them.
 * ---------------------------------------------------------------------
 * Local variables:
 * mode: c
 * indent-tabs-mode: t
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim:noexpandtab:sw=4:ts=4:
 */
