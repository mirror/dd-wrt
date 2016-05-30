/* src/vm/jit/powerpc64/patcher.cpp - PowerPC64 code patching functions

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

#include "vm/jit/powerpc64/md.hpp"

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
#include "vm/jit/methodheader.hpp"
#include "vm/jit/patcher-common.hpp"


/* patcher_patch_code **********************************************************

   Just patches back the original machine code.

*******************************************************************************/

void patcher_patch_code(patchref_t *pr)
{
	// Patch back original code.
	*((uint32_t*) pr->mpc) = pr->mcode;

	// Synchronize instruction cache.
	md_icacheflush((void*) pr->mpc, 1 * 4);
}


/* patcher_get_putstatic *******************************************************

   Machine code:

   <patched call position>
   816dffc8    lwz   r11,-56(r13)
   80ab0000    lwz   r5,0(r11)

*******************************************************************************/

bool patcher_get_putstatic(patchref_t* pr)
{
	unresolved_field* uf    = (unresolved_field*) pr->ref;
	uintptr_t*        datap = (uintptr_t*)        pr->datap;

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
   811f0014    lwz   r8,20(r31)

*******************************************************************************/

bool patcher_get_putfield(patchref_t* pr)
{
	unresolved_field* uf = (unresolved_field*) pr->ref;

	// Resolve the field.
	fieldinfo* fi = resolve_field_eager(uf);

	if (fi == NULL)
		return false;

	// Patch the field offset in the patcher.  We also need this to
	// validate patchers.
	pr->mcode |= (int16_t) (fi->offset & 0x0000ffff);

	// Patch back the original code.
	patcher_patch_code(pr);

	return true;
}


/* patcher_invokestatic_special ************************************************

   Machine code:

   <patched call position>
   81adffd8    lwz   r13,-40(r13)
   7da903a6    mtctr r13
   4e800421    bctrl

******************************************************************************/

bool patcher_invokestatic_special(patchref_t* pr)
{
	unresolved_method* um    = (unresolved_method*) pr->ref;
	uintptr_t*         datap = (uintptr_t*)         pr->datap;

	// Resolve the method.
	methodinfo* m = resolve_method_eager(um);

	if (m == NULL)
		return false;

	// Patch stubroutine.
	*datap = (uintptr_t) m->stubroutine;

	// Synchronize data cache.
	md_dcacheflush(datap, SIZEOF_VOID_P);

	// Patch back the original code.
	patcher_patch_code(pr);

	return true;
}


/* patcher_invokevirtual *******************************************************

   Machine code:

   <patched call position>
   81830000    lwz   r12,0(r3)
   81ac0088    lwz   r13,136(r12)
   7da903a6    mtctr r13
   4e800421    bctrl

*******************************************************************************/

bool patcher_invokevirtual(patchref_t* pr)
{
	uint32_t*          pc = (uint32_t*)          pr->mpc;
	unresolved_method* um = (unresolved_method*) pr->ref;

	// Resolve the method.
	methodinfo* m = resolve_method_eager(um);

	if (m == NULL)
		return false;

	// Patch vftbl index.
	int32_t disp = (OFFSET(vftbl_t, table[0]) + sizeof(methodptr) * m->vftblindex);

	pc[1] |= (disp & 0x0000ffff);

	// Synchronize instruction cache.
	md_icacheflush(pc + 1, 1 * 4);

	// Patch back the original code.
	patcher_patch_code(pr);

	return true;
}


/* patcher_invokeinterface *****************************************************

   Machine code:

   <patched call position>
   81830000    lwz   r12,0(r3)
   818cffd0    lwz   r12,-48(r12)
   81ac000c    lwz   r13,12(r12)
   7da903a6    mtctr r13
   4e800421    bctrl

*******************************************************************************/

bool patcher_invokeinterface(patchref_t* pr)
{
	uint32_t*          pc = (uint32_t*)          pr->mpc;
	unresolved_method* um = (unresolved_method*) pr->ref;

	// Resolve the method.
	methodinfo* m = resolve_method_eager(um);

	if (m == NULL)
		return false;

	// Patch interfacetable index.
	int32_t disp = OFFSET(vftbl_t, interfacetable[0]) - sizeof(methodptr*) * m->clazz->index;

	// XXX TWISTI: check displacement
	pc[1] |= (disp & 0x0000ffff);

	// Patch method offset.
	disp = sizeof(methodptr) * (m - m->clazz->methods);

	// XXX TWISTI: check displacement
	pc[2] |= (disp & 0x0000ffff);

	// Synchronize instruction cache.
	md_icacheflush(pc + 1, 2 * 4);

	// Patch back the original code.
	patcher_patch_code(pr);

	return true;
}


/* patcher_checkcast_interface **************************************

   Machine code:

   <patched call position>
   81870000    lwz   r12,0(r7)
   800c0010    lwz   r0,16(r12)
   34000000    addic.        r0,r0,0
   408101fc    bgt-  0x3002e518		FIXME
   83c00003    lwz   r30,3(0)		FIXME
   800c0000    lwz   r0,0(r12)

*******************************************************************************/

bool patcher_checkcast_interface(patchref_t* pr)
{
	uint32_t*          pc = (uint32_t*)          pr->mpc;
	constant_classref* cr = (constant_classref*) pr->ref;

	// Resolve the class.
	classinfo* c = resolve_classref_eager(cr);

	if (c == NULL)
		return false;

	// Patch super class index.
	int32_t disp = -(c->index);
	pc[2] |= (disp & 0x0000ffff);

	disp = OFFSET(vftbl_t, interfacetable[0]) - c->index * sizeof(methodptr*);
	pc[5] |= (disp & 0x0000ffff);

	// Synchronize instruction cache.
	md_icacheflush(pc + 2, 4 * 4);

	// Patch back the original code.
	patcher_patch_code(pr);

	return true;
}


/* patcher_instanceof_interface **************************************

   Machine code:

   <patched call position>
   81870000    lwz   r12,0(r7)
   800c0010    lwz   r0,16(r12)
   34000000    addic.        r0,r0,0
   408101fc    ble-  0x3002e518
   800c0000    lwz   r0,0(r12)

*******************************************************************************/

bool patcher_instanceof_interface(patchref_t* pr)
{
	uint32_t*          pc = (uint32_t*)          pr->mpc;
	constant_classref* cr = (constant_classref*) pr->ref;

	// Resolve the class.
	classinfo* c = resolve_classref_eager(cr);

	if (c == NULL)
		return false;

	// Patch super class index.
	int32_t disp = -(c->index);
	pc[2] |= (disp & 0x0000ffff);

	disp = OFFSET(vftbl_t, interfacetable[0]) - c->index * sizeof(methodptr*);
	pc[4] |= (disp & 0x0000ffff);

	// Synchronize instruction cache.
	md_icacheflush(pc + 2, 3 * 4);

	// Patch back the original code.
	patcher_patch_code(pr);

	return true;
}


/* patcher_resolve_classref_to_classinfo ***************************************

   ACONST:

   <patched call postition>
   806dffc4    lwz   r3,-60(r13)
   81adffc0    lwz   r13,-64(r13)
   7da903a6    mtctr r13
   4e800421    bctrl


   MULTIANEWARRAY:

   <patched call position>
   808dffc0    lwz   r4,-64(r13)
   38a10038    addi  r5,r1,56
   81adffbc    lwz   r13,-68(r13)
   7da903a6    mtctr r13
   4e800421    bctrl


   ARRAYCHECKCAST:

   <patched call position>
   808dffd8    lwz   r4,-40(r13)
   81adffd4    lwz   r13,-44(r13)
   7da903a6    mtctr r13
   4e800421    bctrl

*******************************************************************************/

bool patcher_resolve_classref_to_classinfo(patchref_t* pr)
{
	constant_classref* cr    = (constant_classref*) pr->ref;
	uintptr_t*         datap = (uintptr_t*)         pr->datap;

	// Resolve the class.
	classinfo* c = resolve_classref_eager(cr);

	if (c == NULL)
		return false;

	// Patch the classinfo pointer.
	*datap = (uintptr_t) c;

	// Synchronize data cache.
	md_dcacheflush(datap, SIZEOF_VOID_P);

	// Patch back the original code.
	patcher_patch_code(pr);

	return true;
}


/* patcher_resolve_classref_to_vftbl *******************************************

   CHECKCAST (class):

   <patched call position>
   81870000    lwz   r12,0(r7)
   800c0014    lwz   r0,20(r12)
   818dff78    lwz   r12,-136(r13)


   INSTANCEOF (class):

   <patched call position>
   817d0000    lwz   r11,0(r29)
   818dff8c    lwz   r12,-116(r13)

*******************************************************************************/

bool patcher_resolve_classref_to_vftbl(patchref_t* pr)
{
	constant_classref* cr    = (constant_classref*) pr->ref;
	uintptr_t*         datap = (uintptr_t*)         pr->datap;

	// Resolve the class.
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
   818dff7c    lwz   r12,-132(r13)

*******************************************************************************/

bool patcher_resolve_classref_to_flags(patchref_t* pr)
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
	md_dcacheflush(datap, SIZEOF_VOID_P);

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
