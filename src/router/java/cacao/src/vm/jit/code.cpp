/* src/vm/jit/code.cpp - codeinfo struct for representing compiled code

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

#include "vm/jit/code.hpp"
#include "mm/codememory.hpp"            // for CFREE
#include "mm/memory.hpp"                // for OFFSET, FREE, NEW
#include "vm/jit/linenumbertable.hpp"   // for LinenumberTable
#include "vm/jit/methodtree.hpp"        // for methodtree_find, etc
#include "vm/jit/patcher-common.hpp"    // for patcher_list_create, etc
#include "vm/jit/replace.hpp"           // for replace_free_replacement_points
#include "vm/options.hpp"               // for checksync
#include "vm/vm.hpp"                    // for vm_abort

STAT_DECLARE_GROUP(info_struct_stat)
STAT_REGISTER_GROUP_VAR(int,size_codeinfo,0,"size codeinfo","codeinfo",info_struct_stat) // sizeof(codeinfo)?

struct methodinfo;

/* code_init *******************************************************************

   Initialize the code-subsystem.

*******************************************************************************/

void code_init(void)
{
	/* Check if offset of codeinfo.m == 0 (see comment in code.h). */

	if (OFFSET(codeinfo, m) != 0)
		vm_abort("code_init: offset of codeinfo.m != 0: %d != 0", OFFSET(codeinfo, m));
}


/* code_codeinfo_new ***********************************************************

   Create a new codeinfo for the given method.
   
   IN:
       m................method to create a new codeinfo for

   The following fields are set in codeinfo:
       m
       patchers

   RETURN VALUE:
       a new, initialized codeinfo, or
	   NULL if an exception occurred.
  
*******************************************************************************/

codeinfo *code_codeinfo_new(methodinfo *m)
{
	codeinfo *code;

	code = NEW(codeinfo);

	code->m = m;

	patcher_list_create(code);

	STATISTICS(size_codeinfo += sizeof(codeinfo));

	return code;
}


/* code_find_codeinfo_for_pc ***************************************************

   Return the codeinfo for the compilation unit that contains the
   given PC.

   ARGUMENTS:
       pc...............machine code position

   RETURN VALUE:
       the codeinfo * for the given PC

*******************************************************************************/

codeinfo *code_find_codeinfo_for_pc(void *pc)
{
	void *pv;

	pv = methodtree_find(pc);

	return code_get_codeinfo_for_pv(pv);
}


/* code_find_codeinfo_for_pc ***************************************************

   Return the codeinfo for the compilation unit that contains the
   given PC. This method does not check the return value and is used
   by the GC.

   IN:
       pc...............machine code position

   RETURN VALUE:
       the codeinfo * for the given PC, or NULL

*******************************************************************************/

codeinfo *code_find_codeinfo_for_pc_nocheck(void *pc)
{
	void *pv;

	pv = methodtree_find_nocheck(pc);

	if (pv == NULL)
		return NULL;

	return code_get_codeinfo_for_pv(pv);
}


/* code_get_methodinfo_for_pv **************************************************

   Return the methodinfo for the given PV.

   IN:
       pv...............PV

   RETURN VALUE:
       the methodinfo *

*******************************************************************************/

methodinfo *code_get_methodinfo_for_pv(void *pv)
{
	codeinfo *code;

	code = code_get_codeinfo_for_pv(pv);

	/* This is the case for asm_vm_call_method. */

	if (code == NULL)
		return NULL;

	return code->m;
}


/* code_get_sync_slot_count ****************************************************

   Return the number of stack slots used for storing the synchronized object
   (and the return value around lock_monitor_exit calls) by the given code.
   
   IN:
       code.............the codeinfo of the code in question
	                    (must be != NULL)

   RETURN VALUE:
       the number of stack slots used for synchronization
  
*******************************************************************************/

#if defined(ENABLE_REPLACEMENT)
int code_get_sync_slot_count(codeinfo *code)
{
	int count;
	
	assert(code);

	if (!checksync)
		return 0;

	if (!code_is_synchronized(code))
		return 0;

	count = 1;

#if defined(__POWERPC__)
	/* powerpc needs an extra slot */
	count++;
#endif

	return count;
}
#endif /* defined(ENABLE_REPLACEMENT) */


/* code_codeinfo_free **********************************************************

   Free the memory used by a codeinfo.
   
   IN:
       code.............the codeinfo to free

*******************************************************************************/

void code_codeinfo_free(codeinfo *code)
{
	if (code == NULL)
		return;

	if (code->mcode != NULL)
		CFREE((void *) (ptrint) code->mcode, code->mcodelength);

	patcher_list_free(code);

#if defined(ENABLE_REPLACEMENT)
	replace_free_replacement_points(code);
#endif

#if defined(ENABLE_PROFILING)
	/* Release memory for basic block profiling information. */

	if (code->bbfrequency != NULL)
		MFREE(code->bbfrequency, u4, code->basicblockcount);
#endif

	FREE(code, codeinfo);

	STATISTICS(size_codeinfo -= sizeof(codeinfo));
}


/* code_free_code_of_method ****************************************************

   Free all codeinfos of the given method
   
   IN:
       m................the method of which the codeinfos are to be freed

*******************************************************************************/

void code_free_code_of_method(methodinfo *m)
{
	codeinfo *nextcode;
	codeinfo *code;

	if (!m)
		return;
	
	nextcode = m->code;
	while (nextcode) {
		code = nextcode;
		nextcode = code->prev;
		code_codeinfo_free(code);
	}

	m->code = NULL;
}

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
