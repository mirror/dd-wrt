/* src/vm/jit/intrp/patcher.c - Interpreter code patching functions

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
#include "vm/types.hpp"

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
#include "vm/jit/patcher.hpp"


/* patcher_get_putstatic *******************************************************

   Machine code:

*******************************************************************************/

bool intrp_patcher_get_putstatic(u1 *sp)
{
	ptrint            *ip;
	unresolved_field  *uf;
	fieldinfo         *fi;

	ip = (ptrint *) sp;
	uf = (unresolved_field *) ip[2];

	/* get the fieldinfo */

	if (!(fi = resolve_field_eager(uf)))
		return false;

	/* check if the field's class is initialized */

	if (!(fi->clazz->state & CLASS_INITIALIZED))
		if (!initialize_class(fi->clazz))
			return false;

	/* patch the field's address */

	ip[1] = (ptrint) &(fi->value);

	return true;
}


/* patcher_get_putstatic_clinit ************************************************

   This patcher is used if we already have the resolved fieldinfo but the
   class of the field has not been initialized, yet.
   
   Machine code:

*******************************************************************************/

bool intrp_patcher_get_putstatic_clinit(u1 *sp)
{
	ptrint            *ip;
	fieldinfo         *fi;

	/* get the fieldinfo */

	ip = (ptrint *) sp;
	fi = (fieldinfo *) ip[2];

	/* check if the field's class is initialized */

	if (!(fi->clazz->state & CLASS_INITIALIZED))
		if (!initialize_class(fi->clazz))
			return false;

	/* patch the field's address */

	ip[1] = (ptrint) &(fi->value);

	return true;
}


/* patcher_get_putfield ********************************************************

   Machine code:

*******************************************************************************/

bool intrp_patcher_get_putfield(u1 *sp)
{
	ptrint            *ip;
	unresolved_field  *uf;
	fieldinfo         *fi;

	ip = (ptrint *) sp;
	uf = (unresolved_field *) ip[2];

	/* get the fieldinfo */

	if (!(fi = resolve_field_eager(uf)))
		return false;

	/* patch the field's offset */

	ip[1] = fi->offset;

	return true;
}


/* patcher_aconst **************************************************************

   Machine code:

*******************************************************************************/

bool intrp_patcher_aconst(u1 *sp)
{
	ptrint            *ip;
	constant_classref *cr;
	classinfo         *c;

	ip = (ptrint *) sp;
	cr = (constant_classref *) ip[2];
	
	/* get the classinfo */

	if (!(c = resolve_classref_eager(cr)))
		return false;

	/* patch the classinfo pointer */

	ip[1] = (ptrint) c;

	return true;
}


/* patcher_builtin_multianewarray **********************************************

   Machine code:

*******************************************************************************/

bool intrp_patcher_builtin_multianewarray(u1 *sp)
{
	ptrint            *ip;
	constant_classref *cr;
	classinfo         *c;

	ip = (ptrint *) sp;
	cr = (constant_classref *) ip[3];

	/* get the classinfo */

	if (!(c = resolve_classref_eager(cr)))
		return false;

	/* patch the classinfo pointer */

	ip[1] = (ptrint) c;

	return true;
}


/* patcher_builtin_arraycheckcast **********************************************

   Machine code:

*******************************************************************************/

bool intrp_patcher_builtin_arraycheckcast(u1 *sp)
{
	ptrint            *ip;
	constant_classref *cr;
	classinfo         *c;

	ip = (ptrint *) sp;
	cr = (constant_classref *) ip[2];

	/* get the classinfo */

	if (!(c = resolve_classref_eager(cr)))
		return false;

	/* patch the classinfo pointer */

	ip[1] = (ptrint) c;

	return true;
}


/* patcher_invokestatic_special ************************************************

   Machine code:

******************************************************************************/

bool intrp_patcher_invokestatic_special(u1 *sp)
{
	ptrint            *ip;
	unresolved_method *um;
	methodinfo        *m;

	ip = (ptrint *) sp;
	um = (unresolved_method *) ip[3];

	/* get the fieldinfo */

	if (!(m = resolve_method_eager(um)))
		return false;

	/* patch methodinfo and stubroutine */

	ip[3] = (ptrint) m;
	ip[1] = (ptrint) m->stubroutine;

	return true;
}


/* patcher_invokevirtual *******************************************************

   Machine code:

*******************************************************************************/

bool intrp_patcher_invokevirtual(u1 *sp)
{
	ptrint            *ip;
	unresolved_method *um;
	methodinfo        *m;

	ip = (ptrint *) sp;
	um = (unresolved_method *) ip[3];

	/* get the fieldinfo */

	if (!(m = resolve_method_eager(um)))
		return false;

	/* patch methodinfo and vftbl index */

	ip[3] = (ptrint) m;
	ip[1] = (OFFSET(vftbl_t, table[0]) + sizeof(methodptr) * m->vftblindex);

	return true;
}


/* patcher_invokeinterface *****************************************************

   Machine code:

*******************************************************************************/

bool intrp_patcher_invokeinterface(u1 *sp)
{
	ptrint            *ip;
	unresolved_method *um;
	methodinfo        *m;

	ip = (ptrint *) sp;
	um = (unresolved_method *) ip[4];

	/* get the methodinfo */

	if (!(m = resolve_method_eager(um)))
		return false;

	/* patch interfacetable index */

	ip[1] = (OFFSET(vftbl_t, interfacetable[0]) -
			 sizeof(methodptr*) * m->clazz->index);

	/* patch methodinfo and method offset */

	ip[4] = (ptrint) m;
	ip[2] = (sizeof(methodptr) * (m - m->clazz->methods));

	return true;
}


/* patcher_checkcast_instanceof ************************************************

   Machine code:

*******************************************************************************/

bool intrp_patcher_checkcast_instanceof(u1 *sp)
{
	ptrint            *ip;
	constant_classref *cr;
	classinfo         *c;

	ip = (ptrint *) sp;
	cr = (constant_classref *) ip[2];

	/* get the classinfo */

	if (!(c = resolve_classref_eager(cr)))
		return false;

	/* patch super class pointer */

	ip[1] = (ptrint) c;

	return true;
}


/* patcher_resolve_native ******************************************************

   XXX

*******************************************************************************/

bool intrp_patcher_resolve_native(u1 *sp)
{
	ptrint      *ip;
	methodinfo  *m;
	functionptr  f;

	ip = (ptrint *) sp;
	m = (methodinfo *) ip[1];

	/* resolve native function */

	if (!(f = native_resolve_function(m)))
		return false;

	/* patch native function pointer */

	ip[2] = (ptrint) f;

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
