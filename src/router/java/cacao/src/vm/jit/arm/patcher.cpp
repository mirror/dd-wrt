/* src/vm/jit/arm/patcher.cpp - ARM code patching functions

   Copyright (C) 1996-2013
   CACAOVM - Verein zur Foerderung der freien virtuellen Maschine CACAO
   Copyright (C) 2008 Theobroma Systems Ltd.

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

#include "vm/types.hpp"

#include "vm/jit/arm/md.hpp"

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
    md_icacheflush((void*) pr->mpc, 1 * 4);
}


/**
 * Check if the trap instruction at the given PC is valid.
 *
 * @param pc Program counter.
 *
 * @return true if valid, false otherwise.
 */
bool patcher_is_valid_trap_instruction_at(void* pc)
{
	uint32_t mcode = *((uint32_t*) pc);

	// Check for the undefined instruction we use.
	if ((mcode & 0x0ff000f0) != 0x07f000f0) {
		return false;
	}

	return true;
}


/* patcher_get_putstatic *******************************************************

   Machine code:

   <patched call position>
   e51c103c    ldr   r1, [ip, #-60]

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

	// Patch back the original code.
	patcher_patch_code(pr);

	return true;
}


/* patcher_get_putfield ********************************************************

   Machine code:

   <patched call position>
   e58a8000    str   r8, [sl, #__]

*******************************************************************************/

bool patcher_get_putfield(patchref_t *pr)
{
	uint32_t*         pc = (uint32_t*)         pr->mpc;
	unresolved_field* uf = (unresolved_field*) pr->ref;

	// Resolve the field.
	fieldinfo* fi = resolve_field_eager(uf);

	if (fi == NULL)
		return false;

	// Patch the field's offset into the instruction.

	switch(fi->type) {
	case TYPE_ADR:
	case TYPE_INT:
#if defined(ENABLE_SOFTFLOAT)
	case TYPE_FLT:
#endif
		assert(fi->offset <= 0x0fff);
		pr->mcode |= (fi->offset & 0x0fff);
		break;

	case TYPE_LNG:
#if defined(ENABLE_SOFTFLOAT)
	case TYPE_DBL:
#endif
		assert((fi->offset + 4) <= 0x0fff);
		pr->mcode |= ((fi->offset + 0) & 0x0fff);
		pc[1] &= 0xfffff000;
		pc[1] |= ((fi->offset + 4) & 0x0fff);
		break;

#if !defined(ENABLE_SOFTFLOAT)
	case TYPE_FLT:
	case TYPE_DBL:
		assert(fi->offset <= 0x03ff);
		pr->mcode |= ((fi->offset >> 2) & 0x00ff);
		break;
#endif
	}

	// Synchronize instruction cache.
	md_icacheflush(pc, 2 * 4);

	// Patch back the original code.
	patcher_patch_code(pr);

	return true;
}


/* patcher_resolve_classref_to_classinfo ***************************************

   ACONST - Machine code:

   <patched call postition>
   e51cc030    ldr   r0, [ip, #-48]

   MULTIANEWARRAY - Machine code:
    
   <patched call position>
   e3a00002    mov   r0, #2  ; 0x2
   e51c1064    ldr   r1, [ip, #-100]
   e1a0200d    mov   r2, sp
   e1a0e00f    mov   lr, pc
   e51cf068    ldr   pc, [ip, #-104]

   ARRAYCHECKCAST - Machine code:

   <patched call position>
   e51c1120    ldr   r1, [ip, #-288]
   e1a0e00f    mov   lr, pc
   e51cf124    ldr   pc, [ip, #-292]

*******************************************************************************/

bool patcher_resolve_classref_to_classinfo(patchref_t *pr)
{
	constant_classref* cr    = (constant_classref *) pr->ref;
	uintptr_t*         datap = (uintptr_t*)          pr->datap;

	// Resolve the class.
	classinfo* c = resolve_classref_eager(cr);

	if (c == NULL)
		return false;

	// Patch the classinfo pointer.
	*datap = (uintptr_t) c;

	// Patch back the original code.
	patcher_patch_code(pr);

	return true;
}


/* patcher_invokestatic_special ************************************************

   Machine code:

   <patched call position>
   e51cc02c    ldr   ip, [ip, #-44]
   e1a0e00f    mov   lr, pc
   e1a0f00c    mov   pc, ip

******************************************************************************/

bool patcher_invokestatic_special(patchref_t *pr)
{
	unresolved_method* um    = (unresolved_method*) pr->ref;
	uintptr_t*         datap = (uintptr_t*)         pr->datap;

	// Reolve the method.
	methodinfo* m = resolve_method_eager(um);

	if (m == NULL)
		return false;

	// Patch stubroutine.
	*datap = (uintptr_t) m->stubroutine;

	// Patch back the original code.
	patcher_patch_code(pr);

	return true;
}


/* patcher_invokevirtual *******************************************************

   Machine code:

   <patched call position>
   e590b000    ldr   fp, [r0]
   e59bc000    ldr   ip, [fp, #__]
   e1a0e00f    mov   lr, pc
   e1a0f00c    mov   pc, ip

*******************************************************************************/

bool patcher_invokevirtual(patchref_t *pr)
{
	unresolved_method* um    = (unresolved_method*) pr->ref;
	int32_t*           datap = (int32_t*)           pr->datap;

	// Resolve the method.
	methodinfo* m = resolve_method_eager(um);

	if (m == NULL)
		return false;

	// Patch vftbl index.
	int32_t disp = OFFSET(vftbl_t, table[0]) + sizeof(methodptr) * m->vftblindex;
	*datap = disp;

	// Patch back the original code.
	patcher_patch_code(pr);

	return true;
}


/* patcher_invokeinterface *****************************************************

   Machine code:

   <patched call position>
   e590b000    ldr   fp, [r0]
   e59bb000    ldr   fp, [fp, #__]
   e59bc000    ldr   ip, [fp, #__]
   e1a0e00f    mov   lr, pc
   e1a0f00c    mov   pc, ip

*******************************************************************************/

bool patcher_invokeinterface(patchref_t *pr)
{
	unresolved_method* um    = (unresolved_method*) pr->ref;
	int32_t*           datap = (int32_t*)           pr->datap;

	// Resolve the method.
	methodinfo* m = resolve_method_eager(um);

	if (m == NULL)
		return false;

	// Patch interfacetable index.
	int32_t disp = OFFSET(vftbl_t, interfacetable[0]) - sizeof(methodptr*) * m->clazz->index;
	*datap = disp;

	// HACK The next data segment entry is one below.
	datap--;

	// Patch method offset.
	disp = sizeof(methodptr) * (m - m->clazz->methods);
	*datap = disp;

	// Patch back the original code.
	patcher_patch_code(pr);

	return true;
}


/* patcher_checkcast_instanceof_flags ******************************************

   Machine code:

   <patched call position>

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

	// Patch back the original code.
	patcher_patch_code(pr);

	return true;
}


/* patcher_resolve_classref_to_index *******************************************

   Machine code:

   <patched call position>

*******************************************************************************/

bool patcher_resolve_classref_to_index(patchref_t *pr)
{
	constant_classref* cr    = (constant_classref*) pr->ref;
	int32_t*           datap = (int32_t*)           pr->datap;

	// Resolve the class.
	classinfo* c = resolve_classref_eager(cr);

	if (c == NULL)
		return false;

	// Patch super class index.
	*datap = (int32_t) c->index;

	// Patch back the original code.
	patcher_patch_code(pr);

	return true;
}


/* patcher_resolve_classref_to_vftbl *******************************************

   Machine code:

   <patched call position>

*******************************************************************************/

bool patcher_resolve_classref_to_vftbl(patchref_t *pr)
{
	constant_classref* cr    = (constant_classref*) pr->ref;
	uintptr_t*         datap = (uintptr_t*)         pr->datap;

	// Resolve the class.

	classinfo* c = resolve_classref_eager(cr);

	if (c == NULL)
		return false;

	// Patch super class' vftbl.
	*datap = (uintptr_t) c->vftbl;

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
