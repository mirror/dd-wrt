/* src/vm/jit/s390/patcher.c - s390 code patching functions

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


#include "config.h"

#include <assert.h>
#include <stdint.h>

#include "mm/memory.hpp"
#include "native/native.hpp"

#include "vm/jit/builtin.hpp"
#include "vm/class.hpp"
#include "vm/field.hpp"
#include "vm/initialize.hpp"
#include "vm/options.hpp"
#include "vm/references.hpp"
#include "vm/resolve.hpp"
#include "vm/types.hpp"

#include "vm/jit/patcher-common.hpp"
#include "vm/jit/s390/codegen.h"
#include "vm/jit/s390/md-abi.h"


#define PATCH_BACK_ORIGINAL_MCODE \
	*((u2 *) pr->mpc) = (u2) pr->mcode;

#define PATCHER_TRACE 


/* patcher_patch_code **********************************************************

   Just patches back the original machine code.

*******************************************************************************/

void patcher_patch_code(patchref_t *pr)
{
	PATCH_BACK_ORIGINAL_MCODE;
}


/* patcher_get_putstatic *******************************************************

   Machine code:

*******************************************************************************/

bool patcher_get_putstatic(patchref_t *pr)
{
	unresolved_field *uf;
	u1               *datap;
	fieldinfo        *fi;

	PATCHER_TRACE;

	/* get stuff from the stack */

	uf    = (unresolved_field *) pr->ref;
	datap = (u1 *)               pr->datap;

	/* get the fieldinfo */

	if (!(fi = resolve_field_eager(uf)))
		return false;

	/* check if the field's class is initialized */

	if (!(fi->clazz->state & CLASS_INITIALIZED))
		if (!initialize_class(fi->clazz))
			return false;

	PATCH_BACK_ORIGINAL_MCODE;

	/* patch the field value's address */

	*((intptr_t *) datap) = (intptr_t) fi->value;

	return true;
}


/* patcher_get_putfield ********************************************************

   Machine code:

*******************************************************************************/

bool patcher_get_putfield(patchref_t *pr)
{
	u1               *ra;
	unresolved_field *uf;
	fieldinfo        *fi;
	s4                disp;

	PATCHER_TRACE;

	/* get stuff from the stack */

	ra    = (u1 *)               pr->mpc;
	uf    = (unresolved_field *) pr->ref;
	disp  =                      pr->disp;

	/* get the fieldinfo */

	if (!(fi = resolve_field_eager(uf)))
		return false;

	PATCH_BACK_ORIGINAL_MCODE;

	/* If there is an operand load before, skip the load size passed in disp (see ICMD_PUTFIELD) */

	ra += disp;

	/* patch correct offset */

	if (fi->type == TYPE_LNG) {
		ASSERT_VALID_DISP(fi->offset + 4);
		/* 2 RX operations, for 2 words; each already contains a 0 or 4 offset. */
		N_RX_SET_DISP(ra, fi->offset + N_RX_GET_DISP(ra));
		ra += SZ_RX;
		N_RX_SET_DISP(ra, fi->offset + N_RX_GET_DISP(ra));
	} else {
		ASSERT_VALID_DISP(fi->offset);
		/* 1 RX operation */
		N_RX_SET_DISP(ra, fi->offset);
	}

	return true;
}

/* patcher_invokestatic_special ************************************************

   Machine code:

*******************************************************************************/

bool patcher_invokestatic_special(patchref_t *pr)
{
	unresolved_method *um;
	u1                *datap;
	methodinfo        *m;

	PATCHER_TRACE;

	/* get stuff from the stack */

	um    = (unresolved_method *) pr->ref;
	datap = (u1 *)                pr->datap;

	/* get the fieldinfo */

	if (!(m = resolve_method_eager(um)))
		return false;

	PATCH_BACK_ORIGINAL_MCODE;

	/* patch stubroutine */

	*((ptrint *) datap) = (ptrint) m->stubroutine;

	return true;
}

/* patcher_invokevirtual *******************************************************

   Machine code:

*******************************************************************************/

bool patcher_invokevirtual(patchref_t *pr)
{
	u1                *ra;
	unresolved_method *um;
	methodinfo        *m;
	s4                 off;

	PATCHER_TRACE;

	/* get stuff from the stack */

	ra    = (u1 *)                pr->mpc;
	um    = (unresolved_method *) pr->ref;

	/* get the fieldinfo */

	if (!(m = resolve_method_eager(um)))
		return false;

	/* patch back original code */

	PATCH_BACK_ORIGINAL_MCODE;

	/* patch vftbl index */


	off = (s4) (OFFSET(vftbl_t, table[0]) +
								   sizeof(methodptr) * m->vftblindex);

	ASSERT_VALID_DISP(off);

	N_RX_SET_DISP(ra + SZ_RX, off);

	return true;
}


/* patcher_invokeinterface *****************************************************

   Machine code:

*******************************************************************************/

bool patcher_invokeinterface(patchref_t *pr)
{
	u1                *ra;
	unresolved_method *um;
	methodinfo        *m;
	s4                 idx, off;

	PATCHER_TRACE;

	/* get stuff from the stack */

	ra    = (u1 *)                pr->mpc;
	um    = (unresolved_method *) pr->ref;

	/* get the fieldinfo */

	if (!(m = resolve_method_eager(um)))
		return false;

	/* patch back original code */

	PATCH_BACK_ORIGINAL_MCODE;

	/* get interfacetable index */

	idx = (s4) (OFFSET(vftbl_t, interfacetable[0]) -
		sizeof(methodptr) * m->clazz->index);

	ASSERT_VALID_IMM(idx);

	/* get method offset */

	off =
		(s4) (sizeof(methodptr) * (m - m->clazz->methods));

	ASSERT_VALID_DISP(off);

	/* patch them */

	N_RI_SET_IMM(ra + SZ_L, idx);
	N_RX_SET_DISP(ra + SZ_L + SZ_LHI + SZ_L, off);

	return true;
}


/* patcher_resolve_classref_to_flags *******************************************

   CHECKCAST/INSTANCEOF:

   <patched call position>

*******************************************************************************/

bool patcher_resolve_classref_to_flags(patchref_t *pr)
{
	constant_classref *cr;
	u1                *datap;
	classinfo         *c;

	PATCHER_TRACE;

	/* get stuff from the stack */

	cr    = (constant_classref *) pr->ref;
	datap = (u1 *)                pr->datap;

	/* get the fieldinfo */

	if (!(c = resolve_classref_eager(cr)))
		return false;

	PATCH_BACK_ORIGINAL_MCODE;

	/* patch class flags */

	*((s4 *) datap) = (s4) c->flags;

	return true;
}

/* patcher_resolve_classref_to_classinfo ***************************************

   ACONST:
   MULTIANEWARRAY:
   ARRAYCHECKCAST:

*******************************************************************************/

bool patcher_resolve_classref_to_classinfo(patchref_t *pr)
{
	constant_classref *cr;
	u1                *datap;
	classinfo         *c;

	PATCHER_TRACE;

	/* get stuff from the stack */

	cr    = (constant_classref *) pr->ref;
	datap = (u1 *)                pr->datap;

	/* get the classinfo */

	if (!(c = resolve_classref_eager(cr)))
		return false;

	PATCH_BACK_ORIGINAL_MCODE;

	/* patch the classinfo pointer */

	*((ptrint *) datap) = (ptrint) c;

	return true;
}

/* patcher_resolve_classref_to_vftbl *******************************************

   CHECKCAST (class):
   INSTANCEOF (class):

*******************************************************************************/

bool patcher_resolve_classref_to_vftbl(patchref_t *pr)
{
	constant_classref *cr;
	u1                *datap;
	classinfo         *c;

	PATCHER_TRACE;

	/* get stuff from the stack */

	cr    = (constant_classref *) pr->ref;
	datap = (u1 *)                pr->datap;

	/* get the fieldinfo */

	if (!(c = resolve_classref_eager(cr)))
		return false;

	PATCH_BACK_ORIGINAL_MCODE;

	/* patch super class' vftbl */

	*((ptrint *) datap) = (ptrint) c->vftbl;

	return true;
}

/* patcher_checkcast_instanceof_interface **************************************

   Machine code:

*******************************************************************************/

bool patcher_checkcast_instanceof_interface(patchref_t *pr)
{

	u1                *ra;
	constant_classref *cr;
	classinfo         *c;

	PATCHER_TRACE;

	/* get stuff from the stack */

	ra    = (u1 *)                pr->mpc;
	cr    = (constant_classref *) pr->ref;

	/* get the fieldinfo */

	if (!(c = resolve_classref_eager(cr)))
		return false;

	/* patch back original code */

	PATCH_BACK_ORIGINAL_MCODE;

	/* patch super class index */

	/* From here, split your editor and open codegen.c */

	switch (N_RX_GET_REG(ra)) {
		case REG_ITMP1: 
			/* First M_ALD is into ITMP1 */
			/* INSTANCEOF code */

			N_RI_SET_IMM(ra + SZ_L + SZ_L, - c->index);
			N_RI_SET_IMM(
				ra + SZ_L + SZ_L + SZ_AHI + SZ_BRC,
				(int16_t)(OFFSET(vftbl_t, interfacetable[0]) - c->index * sizeof(methodptr*))
			);
			break;

		case REG_ITMP2:
			/* First M_ALD is into ITMP2 */
			/* CHECKCAST code */

			N_RI_SET_IMM(ra + SZ_L + SZ_L, - c->index);
			N_RI_SET_IMM(
				ra + SZ_L + SZ_L + SZ_AHI + SZ_BRC + SZ_ILL,
				(int16_t)(OFFSET(vftbl_t, interfacetable[0]) - c->index * sizeof(methodptr*))
			);
			break;

		default:
			assert(0);
			break;
	}

	return true;
}


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
