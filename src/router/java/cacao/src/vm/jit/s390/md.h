/* src/vm/jit/s390/md.h - machine dependent s390 Linux functions

   Copyright (C) 2006-2013
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


#ifndef _VM_JIT_S390_MD_H
#define _VM_JIT_S390_MD_H

#include "config.h"

#include <assert.h>
#include <stdint.h>

#include "vm/jit/codegen-common.hpp"
#include "vm/jit/methodtree.hpp"


/**
 * Returns the size (in bytes) of the current stackframe, specified by
 * the passed codeinfo structure.
 */
inline static int32_t md_stacktrace_get_framesize(codeinfo* code)
{
	// Check for the asm_vm_call_method special case.
	if (code == NULL)
		return 0;

	// On S390 we use 8-byte stackslots.
	return code->stackframesize * 8;
}


/* md_stacktrace_get_returnaddress *********************************************

   Returns the return address of the current stackframe, specified by
   the passed stack pointer and the stack frame size.

*******************************************************************************/

inline static void *md_stacktrace_get_returnaddress(void *sp, int32_t stackframesize)
{
	void *ra;

	/* On S390 the return address is located on the top of the
	   stackframe. */

	ra = *((void **) (((uintptr_t) sp) + stackframesize - 8));

	return ra;
}


/* md_codegen_get_pv_from_pc ***************************************************

   On this architecture just a wrapper function to methodtree_find.

*******************************************************************************/

inline static void *md_codegen_get_pv_from_pc(void *ra)
{
	void *pv;

	/* Get the start address of the function which contains this
       address from the method table. */

	pv = methodtree_find(ra);

	return pv;
}


/* md_cacheflush ***************************************************************

   Calls the system's function to flush the instruction and data
   cache.

*******************************************************************************/

inline static void md_cacheflush(void *addr, int nbytes)
{
	/* do nothing */
}


/* md_icacheflush **************************************************************

   Calls the system's function to flush the instruction cache.

*******************************************************************************/

inline static void md_icacheflush(void *addr, int nbytes)
{
	/* do nothing */
}


/* md_dcacheflush **************************************************************

   Calls the system's function to flush the data cache.

*******************************************************************************/

inline static void md_dcacheflush(void *addr, int nbytes)
{
	/* do nothing */
}

#endif /* _VM_JIT_S390_MD_H */


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
