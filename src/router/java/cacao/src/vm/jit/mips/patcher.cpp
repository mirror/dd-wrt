/* src/vm/jit/mips/patcher.cpp - MIPS code patching functions

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


#include "config.h"

#include <cassert>
#include <stdint.h>

#include "vm/jit/mips/codegen.hpp"
#include "vm/jit/mips/md.hpp"

#include "mm/memory.hpp"

#include "native/native.hpp"

#include "vm/jit/builtin.hpp"
#include "vm/class.hpp"
#include "vm/field.hpp"
#include "vm/initialize.hpp"
#include "vm/options.hpp"
#include "vm/references.hpp"
#include "vm/resolve.hpp"

#include "vm/jit/asmpart.hpp"
#include "vm/jit/patcher-common.hpp"


/* patcher_patch_code **********************************************************

   Just patches back the original machine code.

*******************************************************************************/

void patcher_patch_code(patchref_t *pr)
{
	*((uint32_t*) pr->mpc) = (uint32_t) pr->mcode;
	md_icacheflush((void*) pr->mpc, PATCHER_CALL_SIZE);
}


/* patcher_get_putstatic *******************************************************

   Machine code:

   <patched call position>
   dfc1ffb8    ld       at,-72(s8)
   fc250000    sd       a1,0(at)

*******************************************************************************/

bool patcher_get_putstatic(patchref_t *pr)
{
	unresolved_field* uf    = (unresolved_field *) pr->ref;
	uintptr_t*        datap = (uintptr_t*)         pr->datap;

	// Resolve the field.
	fieldinfo* fi = resolve_field_eager(uf);

	if (fi == NULL)
		return false;

	// Check if the field's class is initialized.
	if (!(fi->clazz->state & CLASS_INITIALIZED))
		if (!initialize_class(fi->clazz))
			return false;

	// Patch the field value's address.
	*datap = (uintptr_t) fi->value;

	// Synchronize data cache.
	md_dcacheflush(datap, SIZEOF_VOID_P);

	// Patch back the original code.
	patcher_patch_code(pr);

	return true;
}


/* patcher_get_putfield ********************************************************

   Machine code:

   <patched call position>
   8ee90020    lw       a5,32(s7)

*******************************************************************************/

bool patcher_get_putfield(patchref_t *pr)
{
	uint32_t*         pc = (uint32_t*)         pr->mpc;
	unresolved_field* uf = (unresolved_field*) pr->ref;

	// Resolve the field.
	fieldinfo* fi = resolve_field_eager(uf);

	if (fi == NULL)
		return false;

#if SIZEOF_VOID_P == 4
	if (IS_LNG_TYPE(fi->type)) {
# if WORDS_BIGENDIAN == 1
		// ATTENTION: order of these instructions depend on M_LLD_INTERN.
		// The first instruction is patched back later.
		pr->mcode |= (int16_t) ((fi->offset + 0) & 0x0000ffff);
		pc[1]     |= (int16_t) ((fi->offset + 4) & 0x0000ffff);
# else
		// ATTENTION: order of these instructions depend on M_LLD_INTERN.
		// The first instruction is patched back later.
		pr->mcode |= (int16_t) ((fi->offset + 4) & 0x0000ffff);
		pc[1]     |= (int16_t) ((fi->offset + 0) & 0x0000ffff);
# endif
	}
	else
#endif
	{
		// The instruction is patched back later.
		pr->mcode |= (int16_t) (fi->offset & 0x0000ffff);
	}

	// Synchronize instruction cache.
	md_icacheflush(pc, 2 * 4);

	// Patch back the original code.
	patcher_patch_code(pr);

	return true;
}


/* patcher_resolve_classref_to_classinfo ***************************************

   ACONST:

   <patched call postition>
   dfc4ff98    ld       a0,-104(s8)

   MULTIANEWARRAY:

   <patched call position>
   dfc5ff90    ld       a1,-112(s8)
   03a03025    move     a2,sp
   dfd9ff88    ld       t9,-120(s8)
   0320f809    jalr     t9
   00000000    nop

   ARRAYCHECKCAST:

   <patched call position>
   dfc5ffc0    ld       a1,-64(s8)
   dfd9ffb8    ld       t9,-72(s8)
   0320f809    jalr     t9
   00000000    nop

*******************************************************************************/

bool patcher_resolve_classref_to_classinfo(patchref_t *pr)
{
	constant_classref* cr    = (constant_classref*) pr->ref;
	uintptr_t*         datap = (uintptr_t*)         pr->datap;

	// Resolve the class.
	classinfo* c = resolve_classref_eager(cr);

	if (c == NULL)
		return false;

	// Patch the class.
	*datap = (uintptr_t) c;

	// Synchronize data cache.
	md_dcacheflush(datap, SIZEOF_VOID_P);

	// Patch back the original code.
	patcher_patch_code(pr);

	return true;
}


/* patcher_resolve_classref_to_vftbl *******************************************

   CHECKCAST (class):
   INSTANCEOF (class):

   <patched call position>
   dd030000    ld       v1,0(a4)
   dfd9ff18    ld       t9,-232(s8)

*******************************************************************************/

bool patcher_resolve_classref_to_vftbl(patchref_t *pr)
{
	constant_classref* cr    = (constant_classref *) pr->ref;
	uintptr_t*         datap = (uintptr_t*)          pr->datap;

	// Resolve the field.
	classinfo* c = resolve_classref_eager(cr);

	if (c == NULL)
		return false;

	// Patch super class' vftbl.
	*datap = (uintptr_t) c->vftbl;

	// Synchronize data cache.
	md_dcacheflush(datap, SIZEOF_VOID_P);

	// Patch back the original code.
	patcher_patch_code(pr);

	return true;
}


/* patcher_resolve_classref_to_flags *******************************************

   CHECKCAST/INSTANCEOF:

   <patched call position>
   8fc3ff24    lw       v1,-220(s8)
   30630200    andi     v1,v1,512
   1060000d    beq      v1,zero,0x000000001051824c
   00000000    nop

*******************************************************************************/

bool patcher_resolve_classref_to_flags(patchref_t *pr)
{
	constant_classref* cr    = (constant_classref*) pr->ref;
	int32_t*           datap = (int32_t*)           pr->datap;

	// Resolve the class.
	classinfo* c = resolve_classref_eager(cr);

	if (c == NULL)
		return false;

	// Patch class flags.
	*datap = (int32_t) c->flags;

	// Synchronize data cache.
	md_dcacheflush(datap, sizeof(int32_t));

	// Patch back the original code.
	patcher_patch_code(pr);

	return true;
}


/* patcher_invokestatic_special ************************************************

   Machine code:

   <patched call position>
   dfdeffc0    ld       s8,-64(s8)
   03c0f809    jalr     s8
   00000000    nop

******************************************************************************/

bool patcher_invokestatic_special(patchref_t *pr)
{
	unresolved_method* um    = (unresolved_method*) pr->ref;
	void**             datap = (void**)             pr->datap;

	// Resolve the method.
	methodinfo* m = resolve_method_eager(um);

	if (m == NULL)
		return false;

	// Patch stubroutine.
	*datap = (void*) m->stubroutine;

	// Synchronize data cache.
	md_dcacheflush(datap, SIZEOF_VOID_P);

	// Patch back the original code.
	patcher_patch_code(pr);

	return true;
}


/* patcher_invokevirtual *******************************************************

   Machine code:

   <patched call position>
   dc990000    ld       t9,0(a0)
   df3e0040    ld       s8,64(t9)
   03c0f809    jalr     s8
   00000000    nop

*******************************************************************************/

bool patcher_invokevirtual(patchref_t *pr)
{
	uint32_t*          pc = (uint32_t*)          pr->mpc;
	unresolved_method* um = (unresolved_method*) pr->ref;

	// Resolve the method.
	methodinfo* m = resolve_method_eager(um);

	if (m == NULL)
		return false;

	// Patch vftbl index.
	pc[1] |= (int32_t) ((OFFSET(vftbl_t, table[0]) + sizeof(methodptr) * m->vftblindex) & 0x0000ffff);

	// Synchronize instruction cache.
	md_icacheflush(pc + 1, 1 * 4);

	// Patch back the original code.
	patcher_patch_code(pr);

	return true;
}


/* patcher_invokeinterface *****************************************************

   Machine code:

   <patched call position>
   dc990000    ld       t9,0(a0)
   df39ffa0    ld       t9,-96(t9)
   df3e0018    ld       s8,24(t9)
   03c0f809    jalr     s8
   00000000    nop

*******************************************************************************/

bool patcher_invokeinterface(patchref_t *pr)
{
	uint32_t*          pc = (uint32_t*)          pr->mpc;
	unresolved_method* um = (unresolved_method*) pr->ref;

	// Resovlve the method.
	methodinfo* m = resolve_method_eager(um);

	if (m == NULL)
		return false;

	// Patch interfacetable index.
	pc[1] |= (int32_t) ((OFFSET(vftbl_t, interfacetable[0]) - sizeof(methodptr*) * m->clazz->index) & 0x0000ffff);

	// Patch method offset.
	pc[2] |= (int32_t) ((sizeof(methodptr) * (m - m->clazz->methods)) & 0x0000ffff);

	// Synchronize instruction cache.
	md_icacheflush(pc + 1, 2 * 4);

	// Patch back the original code.
	patcher_patch_code(pr);

	return true;
}


/* patcher_checkcast_interface *************************************************

   Machine code:

   <patched call position>
   dd030000    ld       v1,0(a4)
   8c79001c    lw       t9,28(v1)
   27390000    addiu    t9,t9,0
   1b200082    blez     t9,zero,0x000000001051843c
   00000000    nop
   dc790000    ld       t9,0(v1)

*******************************************************************************/

bool patcher_checkcast_interface(patchref_t *pr)
{
	uint32_t*          pc = (uint32_t*)          pr->mpc;
	constant_classref* cr = (constant_classref*) pr->ref;

	// Resolve the class.
	classinfo* c = resolve_classref_eager(cr);

	if (c == NULL)
		return false;

	// Patch super class index.
	pc[2] |= (int32_t) (-(c->index) & 0x0000ffff);
	pc[6] |= (int32_t) ((OFFSET(vftbl_t, interfacetable[0]) - c->index * sizeof(methodptr*)) & 0x0000ffff);

	// Synchronize instruction cache.
	md_icacheflush(pc + 2, 5 * 4);

	// Patch back the original code.
	patcher_patch_code(pr);

	return true;
}


/* patcher_instanceof_interface ************************************************

   Machine code:

   <patched call position>
   dd030000    ld       v1,0(a4)
   8c79001c    lw       t9,28(v1)
   27390000    addiu    t9,t9,0
   1b200082    blez     t9,zero,0x000000001051843c
   00000000    nop
   dc790000    ld       t9,0(v1)

*******************************************************************************/

bool patcher_instanceof_interface(patchref_t *pr)
{
	uint32_t*          pc = (uint32_t*)          pr->mpc;
	constant_classref* cr = (constant_classref*) pr->ref;

	// Resolve the method.
	classinfo* c = resolve_classref_eager(cr);

	if (c == NULL)
		return false;

	// Patch super class index.
	pc[2] |= (int32_t) (-(c->index) & 0x0000ffff);
	pc[5] |= (int32_t) ((OFFSET(vftbl_t, interfacetable[0]) - c->index * sizeof(methodptr*)) & 0x0000ffff);

	// Synchronize instruction cache.
	md_icacheflush(pc + 2, 4 * 4);

	// Patch back the original code.
	patcher_patch_code(pr);

	return true;
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
