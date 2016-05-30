/* src/vm/jit/x86_64/patcher.cpp - x86_64 code patching functions

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

#include <stdint.h>

#include "vm/types.hpp"

#include "vm/jit/x86_64/codegen.hpp"
#include "vm/jit/x86_64/md.hpp"

#include "mm/memory.hpp"

#include "native/native.hpp"

#include "vm/jit/builtin.hpp"
#include "vm/class.hpp"
#include "vm/field.hpp"
#include "vm/initialize.hpp"
#include "vm/options.hpp"
#include "vm/resolve.hpp"

#include "vm/jit/patcher-common.hpp"


/* patcher_patch_code **********************************************************

   Just patches back the original machine code.

*******************************************************************************/

void patcher_patch_code(patchref_t *pr)
{
	*((uint16_t*) pr->mpc) = (uint16_t) pr->mcode;
	md_icacheflush((void*) pr->mpc, PATCHER_CALL_SIZE);
}

static int32_t *patch_checked_location(int32_t *p, int32_t val)
{
	assert(*p == 0);
	// verify that it's aligned
	assert((((uintptr_t) p) & (4-1)) == 0);
	*p = val;
	return p;
}

static void checked_icache_flush(void *addr, int nbytes, int32_t *check_loc)
{
	assert((int8_t*) addr + nbytes - sizeof(int32_t) >= (int8_t*) check_loc);
	md_icacheflush(addr, nbytes);
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
	uint16_t mcode = *((uint16_t*) pc);

	// Check for the undefined instruction we use.
	return (mcode == 0x0b0f);
}

/**
 * Overwrites the MFENCE instruction at the indicated address with a 3-byte
 * NOP. The MFENCE instruction is not allowed to cross a (4-byte) word
 * boundary.
 *
 * @param pc Program counter.
 */
static void patch_out_mfence(void *pc)
{
	uint32_t *p = (uint32_t*) (((uintptr_t) pc) & ~3);

	assert((((uintptr_t) pc) & 3) < 2);
	if (((uintptr_t) pc) & 1)
		*p = (*p & 0x000000ff) | 0x001f0f00;
	else
		*p = (*p & 0xff000000) | 0x00001f0f;

	md_icacheflush(p, 4);
}

/* patcher_resolve_classref_to_classinfo ***************************************

   ACONST:

   <patched call position>
   48 bf a0 f0 92 00 00 00 00 00    mov    $0x92f0a0,%rdi

   MULTIANEWARRAY:

   <patched call position>
   48 be 30 40 b2 00 00 00 00 00    mov    $0xb24030,%rsi
   48 89 e2                         mov    %rsp,%rdx
   48 b8 7c 96 4b 00 00 00 00 00    mov    $0x4b967c,%rax
   48 ff d0                         callq  *%rax

   ARRAYCHECKCAST:

   <patched call position>
   48 be b8 3f b2 00 00 00 00 00    mov    $0xb23fb8,%rsi
   48 b8 00 00 00 00 00 00 00 00    mov    $0x0,%rax
   48 ff d0                         callq  *%rax

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
	md_dcacheflush((void*) pr->datap, SIZEOF_VOID_P);

	// Patch back the original code.
	patcher_patch_code(pr);

	return true;
}


/* patcher_resolve_classref_to_vftbl *******************************************

   CHECKCAST (class):
   INSTANCEOF (class):

   <patched call position>

*******************************************************************************/

bool patcher_resolve_classref_to_vftbl(patchref_t *pr)
{
	constant_classref* cr    = (constant_classref*) pr->ref;
	uintptr_t*         datap = (uintptr_t*)         pr->datap;

	// Resolve the field.
	classinfo* c = resolve_classref_eager(cr);

	if (c == NULL)
		return false;

	// Patch super class' vftbl.
	*datap = (uintptr_t) c->vftbl;

	// Synchronize data cache.
	md_dcacheflush((void*) pr->datap, SIZEOF_VOID_P);

	// Patch back the original code.
	patcher_patch_code(pr);

	return true;
}


/* patcher_resolve_classref_to_flags *******************************************

   CHECKCAST/INSTANCEOF:

   <patched call position>

*******************************************************************************/

bool patcher_resolve_classref_to_flags(patchref_t *pr)
{
	constant_classref* cr    = (constant_classref*) pr->ref;
/* 	int32_t*           datap = (int32_t*)           pr->datap; */
	uint8_t*           ra    = (uint8_t*)           pr->mpc;

	// Resolve the field.
	classinfo* c = resolve_classref_eager(cr);

	if (c == NULL)
		return false;

	ra += PATCHER_CALL_SIZE;
	ra += PATCH_ALIGNMENT((uintptr_t) ra, 2, sizeof(int32_t));

	// Patch class flags.
/* 	*datap = c->flags; */
	patch_checked_location((int32_t*) (ra + 2), c->flags);

	// Synchronize data cache.
/* 	md_dcacheflush(datap, sizeof(int32_t)); */
	md_icacheflush(ra + 2, sizeof(int32_t));

	// Patch back the original code.
	patcher_patch_code(pr);

	return true;
}


/* patcher_get_putstatic *******************************************************

   Machine code:

   <patched call position>
   4d 8b 15 86 fe ff ff             mov    -378(%rip),%r10
   49 8b 32                         mov    (%r10),%rsi

*******************************************************************************/

bool patcher_get_putstatic(patchref_t *pr)
{
	unresolved_field* uf    = (unresolved_field*) pr->ref;
	uintptr_t*        datap = (uintptr_t*)        pr->datap;
	uint8_t*          ra    = (uint8_t*)          pr->mpc;

	// Resolve the field.
	fieldinfo* fi = resolve_field_eager(uf);

	if (fi == NULL)
		return false;

	ra += PATCHER_CALL_SIZE;

	// Check if the field's class is initialized/
	if (!(fi->clazz->state & CLASS_INITIALIZED))
		if (!initialize_class(fi->clazz))
			return false;

	// Patch the field value's address.
	*datap = (uintptr_t) fi->value;

	if (pr->disp_mb && !(fi->flags & ACC_VOLATILE))
		patch_out_mfence(ra + pr->disp_mb - 2);

	// Synchronize data cache.
	md_dcacheflush((void*) pr->datap, SIZEOF_VOID_P);

	// Patch back the original code.
	patcher_patch_code(pr);

	return true;
}


/* patcher_get_putfield ********************************************************

   Machine code:

   <patched call position>
   45 8b 8f 00 00 00 00             mov    0x0(%r15),%r9d

*******************************************************************************/

bool patcher_get_putfield(patchref_t *pr)
{
	uint8_t*          pc = (uint8_t*)          pr->mpc;
	unresolved_field* uf = (unresolved_field*) pr->ref;

	// Resolve the field.
	fieldinfo* fi = resolve_field_eager(uf);

	if (fi == NULL)
		return false;

	pc += PATCHER_CALL_SIZE;

	int disp = -sizeof(int32_t) + pr->patch_align;
	patch_checked_location((int32_t*) (pc + disp), fi->offset);

	if (pr->disp_mb && !(fi->flags & ACC_VOLATILE))
		patch_out_mfence(pc + pr->disp_mb - 2);

	// Synchronize instruction cache.
	md_icacheflush(pc, disp + sizeof(int32_t));

	// Patch back the original code.
	patcher_patch_code(pr);

	return true;
}


/* patcher_putfieldconst *******************************************************

   Machine code:

   <patched call position>
   41 c7 85 00 00 00 00 7b 00 00 00    movl   $0x7b,0x0(%r13)

*******************************************************************************/

bool patcher_putfieldconst(patchref_t *pr)
{
	uint8_t*          pc = (uint8_t*)          pr->mpc;
	unresolved_field* uf = (unresolved_field*) pr->ref;

	// Resolve the field.
	fieldinfo* fi = resolve_field_eager(uf);

	if (fi == NULL)
		return false;

	pc += PATCHER_CALL_SIZE;

	int disp = -2*sizeof(int32_t) + pr->patch_align;
	patch_checked_location((int32_t*) (pc + disp), fi->offset);

	if (pr->disp_mb && !(fi->flags & ACC_VOLATILE))
		patch_out_mfence(pc + pr->disp_mb - 2);

	// Synchronize instruction cache.
	md_icacheflush(pc, disp + sizeof(int32_t));

	// Patch back the original code.
	patcher_patch_code(pr);

	return true;
}


/* patcher_invokestatic_special ************************************************

   Machine code:

   <patched call position>
   49 ba 00 00 00 00 00 00 00 00    mov    $0x0,%r10
   49 ff d2                         callq  *%r10

*******************************************************************************/

bool patcher_invokestatic_special(patchref_t *pr)
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
	md_dcacheflush((void*) pr->datap, SIZEOF_VOID_P);

	// Patch back the original code.
	patcher_patch_code(pr);

	return true;
}


/* patcher_invokevirtual *******************************************************

   Machine code:

   <patched call position>
   4c 8b 17                         mov    (%rdi),%r10
   49 8b 82 00 00 00 00             mov    0x0(%r10),%rax
   48 ff d0                         callq  *%rax

*******************************************************************************/

bool patcher_invokevirtual(patchref_t *pr)
{
	uint8_t*           pc = (uint8_t*)           pr->mpc;
	unresolved_method* um = (unresolved_method*) pr->ref;

	// Resovlve the method.
	methodinfo* m = resolve_method_eager(um);

	if (m == NULL)
		return false;

	pc += PATCHER_CALL_SIZE;
	pc += PATCH_ALIGNMENT((uintptr_t) pc, 6, sizeof(int32_t));

	// Patch vftbl index.
	patch_checked_location((int32_t*) (pc + 6), (int32_t) (OFFSET(vftbl_t, table[0]) + sizeof(methodptr) * m->vftblindex));

	// Synchronize instruction cache.
	md_icacheflush(pc + 3 + 3, SIZEOF_VOID_P);

	// Patch back the original code.
	patcher_patch_code(pr);

	return true;
}


/* patcher_invokeinterface *****************************************************

   Machine code:

   <patched call position>
   4c 8b 17                         mov    (%rdi),%r10
   4d 8b 92 00 00 00 00             mov    0x0(%r10),%r10
   49 8b 82 00 00 00 00             mov    0x0(%r10),%rax
   48 ff d0                         callq  *%rax

*******************************************************************************/

bool patcher_invokeinterface(patchref_t *pr)
{
	uint8_t*           pc = (uint8_t*)           pr->mpc;
	unresolved_method* um = (unresolved_method*) pr->ref;

	// Resolve the method.
	methodinfo* m = resolve_method_eager(um);

	if (m == NULL)
		return false;

	pc += PATCHER_CALL_SIZE;
	pc += PATCH_ALIGNMENT((uintptr_t) pc, 6, sizeof(int32_t));

	// Patch interfacetable index.
	patch_checked_location((int32_t*) (pc + 6), (int32_t) (OFFSET(vftbl_t, interfacetable[0]) - sizeof(methodptr) * m->clazz->index));

	int disp = PATCH_ALIGNMENT((uintptr_t) (pc + 3 + 7), 3, sizeof(int32_t));
	pc += disp;
	// Patch method offset.
	int32_t *loc = patch_checked_location((int32_t*) (pc + 3 + 7 + 3), (int32_t) (sizeof(methodptr) * (m - m->clazz->methods)));

	// Synchronize instruction cache.
	checked_icache_flush(pc + 6, SIZEOF_VOID_P + 3 + SIZEOF_VOID_P + disp, loc);

	// Patch back the original code.
	patcher_patch_code(pr);

	return true;
}


/* patcher_checkcast_interface *************************************************

   Machine code:

   <patched call position>
   45 8b 9a 1c 00 00 00             mov    0x1c(%r10),%r11d
   41 81 fb 00 00 00 00             cmp    $0x0,%r11d
   0f 8f 08 00 00 00                jg     0x00002aaaaae511d5
   48 8b 0c 25 03 00 00 00          mov    0x3,%rcx
   4d 8b 9a 00 00 00 00             mov    0x0(%r10),%r11

*******************************************************************************/

bool patcher_checkcast_interface(patchref_t *pr)
{
	uint8_t*           pc = (uint8_t*)           pr->mpc;
	constant_classref* cr = (constant_classref*) pr->ref;

	// Resolve the class.
	classinfo* c = resolve_classref_eager(cr);

	if (c == NULL)
		return false;

	pc += PATCHER_CALL_SIZE;
	pc += PATCH_ALIGNMENT((uintptr_t) pc, 10, sizeof(int32_t));

	// Patch super class index.
	patch_checked_location((int32_t*) (pc + 10), c->index);

	int disp = PATCH_ALIGNMENT((uintptr_t) (pc + 7 + 7 + 6 + 8), 3, sizeof(int32_t));
	pc += disp;
	int32_t *loc = patch_checked_location((int32_t*) (pc + 7 + 7 + 6 + 8 + 3), (int32_t) (OFFSET(vftbl_t, interfacetable[0]) - c->index * sizeof(methodptr*)));

	// Synchronize instruction cache.
	checked_icache_flush(pc + 10, sizeof(int32_t) + 6 + 8 + 3 + sizeof(int32_t) + disp, loc);

	// Patch back the original code.
	patcher_patch_code(pr);

	return true;
}


/* patcher_instanceof_interface ************************************************

   Machine code:

   <patched call position>
   45 8b 9a 1c 00 00 00             mov    0x1c(%r10),%r11d
   41 81 fb 00 00 00 00             cmp    $0x0,%r11d
   0f 8e 94 04 00 00                jle    0x00002aaaaab018f8
   4d 8b 9a 00 00 00 00             mov    0x0(%r10),%r11

*******************************************************************************/

bool patcher_instanceof_interface(patchref_t *pr)
{
	uint8_t*           pc = (uint8_t*)           pr->mpc;
	constant_classref* cr = (constant_classref*) pr->ref;

	// Resolve the class.
	classinfo* c = resolve_classref_eager(cr);

	if (c == NULL)
		return false;

	pc += PATCHER_CALL_SIZE;
	pc += PATCH_ALIGNMENT((uintptr_t) pc, 10, sizeof(int32_t));

	// Patch super class index.
	patch_checked_location((int32_t*) (pc + 10), c->index);

	int disp = PATCH_ALIGNMENT((uintptr_t) (pc + 7 + 7 + 6), 3, sizeof(int32_t));
	pc += disp;
	int32_t *loc = patch_checked_location((int32_t*) (pc + 7 + 7 + 6 + 3), (int32_t) (OFFSET(vftbl_t, interfacetable[0]) - c->index * sizeof(methodptr*)));

	// Synchronize instruction cache.
	checked_icache_flush(pc + 10, sizeof(int32_t) + 6 + 3 + sizeof(int32_t) + disp, loc);

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
