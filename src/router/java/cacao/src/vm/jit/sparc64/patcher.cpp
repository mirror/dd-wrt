/* src/vm/jit/mips/patcher.cpp - SPARC code patching functions

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

#include "vm/types.hpp"

#include "mm/memory.hpp"

#include "vm/jit/sparc64/codegen.hpp"
#include "vm/jit/sparc64/md.hpp"
#include "vm/jit/sparc64/md-abi.hpp"

#include "native/native.hpp"

#include "vm/jit/builtin.hpp"
#include "vm/class.hpp"
#include "vm/exceptions.hpp"
#include "vm/field.hpp"
#include "vm/initialize.hpp"
#include "vm/options.hpp"
#include "vm/references.hpp"
#include "vm/resolve.hpp"

#include "vm/jit/asmpart.hpp"
#include "vm/jit/patcher.hpp"
#include "vm/jit/methodheader.hpp"
#include "vm/jit/stacktrace.hpp"

#include "vm/jit/sparc64/solaris/macro_rename.hpp"


/* patcher_wrapper *************************************************************

   Wrapper for all patchers.  It also creates the stackframe info
   structure.

   If the return value of the patcher function is false, it gets the
   exception object, clears the exception pointer and returns the
   exception.

*******************************************************************************/

java_object_t *patcher_wrapper(u1 *sp, u1 *pv, u1 *ra)
{
	stackframeinfo_t   sfi;
	u1                *xpc;
	u1                *javasp;
	java_object_t *o;
#if SIZEOF_VOID_P == 8
	u8                mcode;
#else
	u4                mcode[2];
#endif
	functionptr        f;
	bool               result;
	java_handle_t *e;
	
	/* define the patcher function */

	bool (*patcher_function)(u1 *);

	assert(pv != NULL);

	/* get stuff from the stack */

	xpc = (u1 *)                *((ptrint *) (sp + 5 * 8));
	o   = (java_object_t *) *((ptrint *) (sp + 4 * 8));
	f   = (functionptr)         *((ptrint *) (sp + 0 * 8));

	/* store PV into the patcher function position */

	*((ptrint *) (sp + 0 * 8)) = (ptrint) pv;

	/* cast the passed function to a patcher function */

	patcher_function = (bool (*)(u1 *)) (ptrint) f;

	/* enter a monitor on the patching position */

	PATCHER_MONITORENTER;
	
	/* the (data) sp points directly to the patcher fields */
	/* calculate the real sp of the current java function considering the WINSAVE regs */
	
	javasp = sp - JITSTACK_CNT * 8 - BIAS;

	/* create the stackframeinfo */

	printf("patcher opening sfi for xpc=%p\n", xpc);

	stacktrace_stackframeinfo_add(&sfi, pv, javasp, ra, xpc);

	/* call the proper patcher function */

	result = (patcher_function)(sp);

	/* remove the stackframeinfo */

	stacktrace_stackframeinfo_remove(&sfi);
	printf("patcher closing sfi for xpc=%p\n", xpc);

	/* check for return value and exit accordingly */

	if (result == false) {
		e = exceptions_get_and_clear_exception();

		PATCHER_MONITOREXIT;

		return e;
	}

	/* patch back original (potentially patched) code */

#if SIZEOF_VOID_P == 8
	mcode    =                      *((u8 *)     (sp + 3 * 8));

	*((u4 *) (xpc + 0 * 4)) = mcode >> 32;
	*((u4 *) (xpc + 1 * 4)) = mcode;
#else
	mcode[0] =                      *((u4 *)     (sp + 3 * 8));
	mcode[1] =                      *((u4 *)     (sp + 3 * 8 + 4));

	*((u4 *) (xpc + 0 * 4)) = mcode[0];
	*((u4 *) (xpc + 1 * 4)) = mcode[1];
#endif


	/* synchronize instruction cache */

	md_icacheflush(xpc, PATCHER_CALL_SIZE);

	PATCHER_MARK_PATCHED_MONITOREXIT;

	return NULL;
}


/* patcher_get_putstatic *******************************************************

   Machine code:

   <patched call position>
   xxx         ldx      at,-72(pv)
   xxx         ld       a1,0(at)

*******************************************************************************/

bool patcher_get_putstatic(u1 *sp)
{
	unresolved_field *uf;
	s4                disp;
	u1               *pv;
	fieldinfo        *fi;

	/* get stuff from the stack */

	uf       = (unresolved_field *) *((ptrint *) (sp + 2 * 8));
	disp     =                      *((s4 *)     (sp + 1 * 8));
	pv       = (u1 *)               *((ptrint *) (sp + 0 * 8));

	/* get the fieldinfo */

	if (!(fi = resolve_field_eager(uf)))
		return false;

	/* check if the field's class is initialized */

	if (!(fi->clazz->state & CLASS_INITIALIZED))
		if (!initialize_class(fi->clazz))
			return false;

	/* patch the field value's address */

	*((intptr_t *) (pv + disp)) = (intptr_t) fi->value;

	/* synchronize data cache */

	md_dcacheflush(pv + disp, SIZEOF_VOID_P);

	return true;
}


/* patcher_get_putfield ********************************************************

   Machine code:

   <patched call position>
   8ee90020    lw       a5,32(s7)

*******************************************************************************/

bool patcher_get_putfield(u1 *sp)
{
	u1               *ra;
	unresolved_field *uf;
	fieldinfo        *fi;


	ra       = (u1 *)               *((ptrint *) (sp + 5 * 8));
	uf       = (unresolved_field *) *((ptrint *) (sp + 2 * 8));

	/* get the fieldinfo */

	if (!(fi = resolve_field_eager(uf)))
		return false;

	/* store the patched instruction on the stack */

	*((u4 *) (sp + 3 * 8)) |= (s2) (fi->offset & 0x00001fff);

	return true;
}


/* patcher_aconst **************************************************************

   Machine code:

   <patched call postition>
   xxx         ld       a0,-104(pv)

*******************************************************************************/

bool patcher_aconst(u1 *sp)
{
	constant_classref *cr;
	s4                 disp;
	u1                *pv;
	classinfo         *c;

	/* get stuff from the stack */

	cr       = (constant_classref *) *((ptrint *) (sp + 2 * 8));
	disp     =                       *((s4 *)     (sp + 1 * 8));
	pv       = (u1 *)                *((ptrint *) (sp + 0 * 8));

	/* get the classinfo */

	if (!(c = resolve_classref_eager(cr)))
		return false;

	/* patch the classinfo pointer */

	*((ptrint *) (pv + disp)) = (ptrint) c;

	/* synchronize data cache */

	md_dcacheflush(pv + disp, SIZEOF_VOID_P);

	return true;
}


/* patcher_builtin_multianewarray **********************************************

   Machine code:

   <patched call position>
   dfc5ff90    ld       a1,-112(s8)
   03a03025    move     a2,sp
   dfd9ff88    ld       t9,-120(s8)
   0320f809    jalr     t9
   00000000    nop

*******************************************************************************/

bool patcher_builtin_multianewarray(u1 *sp)
{
	constant_classref *cr;
	s4                 disp;
	u1                *pv;
	classinfo         *c;

	/* get stuff from the stack */

	cr       = (constant_classref *) *((ptrint *) (sp + 2 * 8));
	disp     =                       *((s4 *)     (sp + 1 * 8));
	pv       = (u1 *)                *((ptrint *) (sp + 0 * 8));

	/* get the classinfo */

	if (!(c = resolve_classref_eager(cr)))
		return false;

	/* patch the classinfo pointer */

	*((ptrint *) (pv + disp)) = (ptrint) c;

	/* synchronize data cache */

	md_dcacheflush(pv + disp, SIZEOF_VOID_P);

	return true;
}


/* patcher_builtin_arraycheckcast **********************************************

   Machine code:

   <patched call position>
   dfc5ffc0    ld       a1,-64(s8)
   dfd9ffb8    ld       t9,-72(s8)
   0320f809    jalr     t9
   00000000    nop

*******************************************************************************/

bool patcher_builtin_arraycheckcast(u1 *sp)
{
	constant_classref *cr;
	s4                 disp;
	u1                *pv;
	classinfo         *c;

	/* get stuff from the stack */

	cr       = (constant_classref *) *((ptrint *) (sp + 2 * 8));
	disp     =                       *((s4 *)     (sp + 1 * 8));
	pv       = (u1 *)                *((ptrint *) (sp + 0 * 8));

	/* get the classinfo */

	if (!(c = resolve_classref_eager(cr)))
		return false;

	/* patch the classinfo pointer */

	*((ptrint *) (pv + disp)) = (ptrint) c;

	/* synchronize data cache */

	md_dcacheflush(pv + disp, SIZEOF_VOID_P);

	return true;
}


/* patcher_invokestatic_special ************************************************

   Machine code:

   <patched call position>
   dfdeffc0    ld       s8,-64(s8)
   03c0f809    jalr     s8
   00000000    nop

******************************************************************************/

bool patcher_invokestatic_special(u1 *sp)
{
	unresolved_method *um;
	s4                 disp;
	u1                *pv;
	methodinfo        *m;

	/* get stuff from the stack */

	um       = (unresolved_method *) *((ptrint *) (sp + 2 * 8));
	disp     =                       *((s4 *)     (sp + 1 * 8));
	pv       = (u1 *)                *((ptrint *) (sp + 0 * 8));

	/* get the fieldinfo */

	if (!(m = resolve_method_eager(um)))
		return false;

	/* patch stubroutine */

	*((ptrint *) (pv + disp)) = (ptrint) m->stubroutine;

	/* synchronize data cache */

	md_dcacheflush(pv + disp, SIZEOF_VOID_P);

	return true;
}


/* patcher_invokevirtual *******************************************************

   Machine code:

   <patched call position>
   xxx         ldx      g2,0(o0)
   xxx         ldx      o5,64(g2)
   xxx         jmpl     o5
   xxx         nop

*******************************************************************************/

bool patcher_invokevirtual(u1 *sp)
{
	u1                *ra;
	unresolved_method *um;
	methodinfo        *m;

	/* get stuff from the stack */

	ra       = (u1 *)                *((ptrint *) (sp + 5 * 8));
	um       = (unresolved_method *) *((ptrint *) (sp + 2 * 8));

	/* get the fieldinfo */

	if (!(m = resolve_method_eager(um)))
		return false;

	/* patch vftbl index */

	*((s4 *) (sp + 3 * 8 + 4)) |=
		(s4) ((OFFSET(vftbl_t, table[0]) +
			   sizeof(methodptr) * m->vftblindex) & 0x00001fff);

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

bool patcher_invokeinterface(u1 *sp)
{
	u1                *ra;
	unresolved_method *um;
	methodinfo        *m;

	/* get stuff from the stack */

	ra       = (u1 *)                *((ptrint *) (sp + 5 * 8));
	um       = (unresolved_method *) *((ptrint *) (sp + 2 * 8));

	/* get the fieldinfo */

	if (!(m = resolve_method_eager(um)))
		return false;

	/* patch interfacetable index */

	*((s4 *) (sp + 3 * 8 + 4)) |=
		(s4) ((OFFSET(vftbl_t, interfacetable[0]) -
			   sizeof(methodptr*) * m->clazz->index) & 0x00001fff);

	/* patch method offset */

	*((s4 *) (ra + 2 * 4)) |=
		(s4) ((sizeof(methodptr) * (m - m->clazz->methods)) & 0x00001fff);

	/* synchronize instruction cache */

	md_icacheflush(ra + 2 * 4, 1 * 4);

	return true;
}


/* patcher_checkcast_instanceof_flags ******************************************

   Machine code:

   <patched call position>
   8fc3ff24    lw       v1,-220(s8)
   30630200    andi     v1,v1,512
   1060000d    beq      v1,zero,0x000000001051824c
   00000000    nop

*******************************************************************************/

bool patcher_checkcast_instanceof_flags(u1 *sp)
{
	constant_classref *cr;
	s4                 disp;
	u1                *pv;
	classinfo         *c;

	/* get stuff from the stack */

	cr       = (constant_classref *) *((ptrint *) (sp + 2 * 8));
	disp     =                       *((s4 *)     (sp + 1 * 8));
	pv       = (u1 *)                *((ptrint *) (sp + 0 * 8));

	/* get the fieldinfo */

	if (!(c = resolve_classref_eager(cr)))
		return false;

	/* patch class flags */

	*((s4 *) (pv + disp)) = (s4) c->flags;

	/* synchronize data cache */

	md_dcacheflush(pv + disp, sizeof(s4));

	return true;
}


/* patcher_checkcast_interface **************************************

   Machine code:

   <patched call position>
   dd030000    ld       v1,0(a4)
   8c79001c    lw       t9,28(v1)
   27390000    addiu    t9,t9,0
   1b200082    blez     t9,zero,0x000000001051843c
   00000000    nop
   dc790000    ld       t9,0(v1)

*******************************************************************************/

bool patcher_checkcast_interface(u1 *sp)
{
	u1                *ra;
	constant_classref *cr;
	classinfo         *c;

	/* get stuff from the stack */

	ra       = (u1 *)                *((ptrint *) (sp + 5 * 8));
	cr       = (constant_classref *) *((ptrint *) (sp + 2 * 8));

	/* get the fieldinfo */

	if (!(c = resolve_classref_eager(cr)))
		return false;

	/* patch super class index */

	*((s4 *) (ra + 2 * 4)) |= (s4) (-(c->index) & 0x00001fff);

	*((s4 *) (ra + (3 + EXCEPTION_CHECK_INSTRUCTIONS) * 4)) |= 
		(s4) ((OFFSET(vftbl_t, interfacetable[0])
		- c->index * sizeof(methodptr*)) & 0x00001fff);

	/* synchronize instruction cache */

	md_icacheflush(ra, (4 + EXCEPTION_CHECK_INSTRUCTIONS) * 4);

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

bool patcher_instanceof_interface(u1 *sp)
{
	u1                *ra;
	constant_classref *cr;
	classinfo         *c;

	/* get stuff from the stack */

	ra = (u1 *)                *((ptrint *) (sp + 5 * 8));
	cr = (constant_classref *) *((ptrint *) (sp + 2 * 8));

	/* get the fieldinfo */

	if (!(c = resolve_classref_eager(cr)))
		return false;

	/* patch super class index */

	*((s4 *) (ra + 2 * 4)) |= (s4) ((c->index) & 0x00001fff);
	*((s4 *) (ra + 5 * 4)) |=
		(s4) ((OFFSET(vftbl_t, interfacetable[0]) -
				 c->index * sizeof(methodptr*)) & 0x00001fff);

	/* synchronize instruction cache */

	md_icacheflush(ra, 6 * 4);

	return true;
}


/* patcher_checkcast_instanceof_class ******************************************

   Machine code:

   <patched call position>
   dd030000    ld       v1,0(a4)
   dfd9ff18    ld       t9,-232(s8)

*******************************************************************************/

bool patcher_checkcast_instanceof_class(u1 *sp)
{
	constant_classref *cr;
	s4                 disp;
	u1                *pv;
	classinfo         *c;

	/* get stuff from the stack */

	cr       = (constant_classref *) *((ptrint *) (sp + 2 * 8));
	disp     =                       *((s4 *)     (sp + 1 * 8));
	pv       = (u1 *)                *((ptrint *) (sp + 0 * 8));

	/* get the fieldinfo */

	if (!(c = resolve_classref_eager(cr)))
		return false;

	/* patch super class' vftbl */

	*((ptrint *) (pv + disp)) = (ptrint) c->vftbl;

	/* synchronize data cache */

	md_dcacheflush(pv + disp, SIZEOF_VOID_P);

	return true;
}


/* patcher_clinit **************************************************************

   No special machine code.

*******************************************************************************/

bool patcher_clinit(u1 *sp)
{
	classinfo *c;

	/* get stuff from the stack */

	c        = (classinfo *) *((ptrint *) (sp + 2 * 8));

	/* check if the class is initialized */

	if (!(c->state & CLASS_INITIALIZED))
		if (!initialize_class(c))
			return false;

	return true;
}


/* patcher_athrow_areturn ******************************************************

   Machine code:

   <patched call position>

*******************************************************************************/

#ifdef ENABLE_VERIFIER
bool patcher_athrow_areturn(u1 *sp)
{
	unresolved_class *uc;

	/* get stuff from the stack */

	uc       = (unresolved_class *) *((ptrint *) (sp + 2 * 8));

	/* resolve the class and check subtype constraints */

	if (!resolve_class_eager_no_access_check(uc))
		return false;

	return true;
}
#endif /* ENABLE_VERIFIER */


/* patcher_resolve_native ******************************************************

   XXX

*******************************************************************************/

bool patcher_resolve_native(u1 *sp)
{
	methodinfo  *m;
	s4           disp;
	u1          *pv;
	functionptr  f;

	/* get stuff from the stack */

	m        = (methodinfo *) *((ptrint *) (sp + 2 * 8));
	disp     =                *((s4 *)     (sp + 1 * 8));
	pv       = (u1 *)         *((ptrint *) (sp + 0 * 8));

	/* return address on SPARC is address of jump, therefore correct */

	/* resolve native function */

	if (!(f = native_resolve_function(m)))
		return false;

	/* patch native function pointer */

	*((ptrint *) (pv + disp)) = (ptrint) f;

	/* synchronize data cache */

	md_dcacheflush(pv + disp, SIZEOF_VOID_P);

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
