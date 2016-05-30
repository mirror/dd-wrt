/* src/vm/jit/intrp/asmpart.c - Java-C interface functions for Interpreter

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

#include <assert.h>

#include "vm/types.hpp"

#include "arch.hpp"

#include "threads/thread.hpp"

#include "vm/jit/builtin.hpp"
#include "vm/class.hpp"
#include "vm/linker.hpp"
#include "vm/loader.hpp"
#include "vm/options.hpp"

#include "vm/jit/asmpart.hpp"
#include "vm/jit/methodheader.hpp"
#include "vm/jit/methodtree.hpp"
#include "vm/jit/dseg.hpp"

#include "vm/jit/intrp/intrp.h"


static bool intrp_asm_vm_call_method_intern(methodinfo *m, s4 vmargscount,
											vm_arg *vmargs)
{
	java_objectheader *retval;
	Cell              *sp;
	s4                 i;
	u1                *entrypoint;

	sp = global_sp;
	CLEAR_global_sp;

	assert(sp != NULL);

	for (i = 0; i < vmargscount; i++) {
		switch (vmargs[i].type) {
		case TYPE_INT:
		case TYPE_FLT:
		case TYPE_ADR:
			*(--sp) = (Cell) vmargs[i].data.l;
			break;
		case TYPE_LNG:
		case TYPE_DBL:
			sp -= 2;
			*((u8 *) sp) = vmargs[i].data.l;
			break;
		}
	}

	entrypoint = createcalljavafunction(m);

	retval = engine((Inst *) entrypoint, sp, NULL);

	/* XXX remove the method from the method table */

	if (retval != NULL) {
		(void)builtin_throw_exception(retval);
		return false;
	}
	else
		return true;
}


java_objectheader *intrp_asm_vm_call_method(methodinfo *m, s4 vmargscount,
											vm_arg *vmargs)
{
	java_objectheader *retval = NULL;

	if (intrp_asm_vm_call_method_intern(m, vmargscount, vmargs)) {
		if (m->parseddesc->returntype.type == TYPE_ADR)
			retval = (java_objectheader *)*global_sp++;
		else
			assert(m->parseddesc->returntype.type == TYPE_VOID);
		return retval;
	} else
		return NULL;
}


s4 intrp_asm_vm_call_method_int(methodinfo *m, s4 vmargscount, vm_arg *vmargs)
{
	s4 retval = 0;

	if (intrp_asm_vm_call_method_intern(m, vmargscount, vmargs)) {
		if (m->parseddesc->returntype.type == TYPE_INT)
			retval = *global_sp++;
		else
			assert(m->parseddesc->returntype.type == TYPE_VOID);
		return retval;
	} else
		return 0;
}


s8 intrp_asm_vm_call_method_long(methodinfo *m, s4 vmargscount, vm_arg *vmargs)
{
	s8 retval;

	assert(m->parseddesc->returntype.type == TYPE_LNG);

	if (intrp_asm_vm_call_method_intern(m, vmargscount, vmargs)) {
		retval = *(s8 *)global_sp;
		global_sp += 2;
		return retval;
	} else
		return 0;
}


float intrp_asm_vm_call_method_float(methodinfo *m, s4 vmargscount,
									 vm_arg *vmargs)
{
	float retval;

	assert(m->parseddesc->returntype.type == TYPE_FLT);

	if (intrp_asm_vm_call_method_intern(m, vmargscount, vmargs)) {
		retval = *(float *)global_sp;
		global_sp += 1;
		return retval;
	} else
		return 0.0;
}


double intrp_asm_vm_call_method_double(methodinfo *m, s4 vmargscount,
									   vm_arg *vmargs)
{
	double retval;

	assert(m->parseddesc->returntype.type == TYPE_DBL);

	if (intrp_asm_vm_call_method_intern(m, vmargscount, vmargs)) {
		retval = *(double *)global_sp;
		global_sp += 2;
		return retval;
	} else
		return 0.0;
}


Inst *intrp_asm_handle_exception(Inst *ip, java_objectheader *o, Cell *fp, Cell **new_spp, Cell **new_fpp)
{
	classinfo            *c;
	classref_or_classinfo cr;
	s4                    framesize;
	s4                    issync;
	dseg_exception_entry *ex;
	s4                    exceptiontablelength;
	s4                    i;

  /* for a description of the stack see IRETURN in java.vmg */

  for (; fp != NULL; ) {
	  u1 *f = methodtree_find((u1 *) (ip - 1));

	  /* get methodinfo pointer from method header */

	  codeinfo *code = *((codeinfo **) ((u1 *)f + CodeinfoPointer));
	  methodinfo *m = code->m;

	  framesize            = *((s4 *)             (((u1 *) f) + FrameSize));
	  issync               = *((s4 *)             (((u1 *) f) + IsSync));
	  ex                   =   (dseg_exception_entry *) 
												  (((u1 *) f) + ExTableStart);
	  exceptiontablelength = *((s4 *)             (((u1 *) f) + ExTableSize));

#if !defined(NDEBUG)
	  if (opt_verbose || opt_verbosecall || opt_verboseexception)
		  builtin_trace_exception(o, m, ip, 1);
#endif

	  for (i = 0; i < exceptiontablelength; i++) {
		  ex--;

		  cr = ex->catchtype;

		  if (cr.any == NULL) {
			  /* catch all */
			  c = NULL;
		  }
		  else {
			  if (cr.is_classref()) {
				  /* The exception class reference is unresolved. */
				  /* We have to do _eager_ resolving here. While the class of */
				  /* the exception object is guaranteed to be loaded, it may  */
				  /* well have been loaded by a different loader than the     */
				  /* defining loader of m's class, which is the one we must   */
				  /* use to resolve the catch class. Thus lazy resolving      */
				  /* might fail, even if the result of the resolution would   */
				  /* be an already loaded class.                              */

				  /* The resolving may involve Java code, so we need a usable */
				  /* global_sp. XXX is this a correct value for global_sp?    */

				  global_sp = (Cell *)(((u1 *)fp) - framesize - SIZEOF_VOID_P);

				  c = resolve_classref_eager(cr.ref);

				  CLEAR_global_sp;

				  if (c == NULL) {
					  /* Exception resolving the exception class, argh! */
					  /* XXX how to report that error? */
					  assert(0);
				  }

				  /* Ok, we resolved it. Enter it in the table, so we don't */
				  /* have to do this again.                                 */
				  /* XXX this write should be atomic. Is it?                */

				  ex->catchtype.cls = c;
			  }
			  else {
				  c = cr.cls;
				
				  /* If the class is not linked, the exception object cannot */
				  /* be an instance of it.                                   */
				  if (!(c->state & CLASS_LINKED))
					  continue;
			  }
		  }

		  if (ip-1 >= (Inst *) ex->startpc && ip-1 < (Inst *) ex->endpc &&
			  (c == NULL || builtin_instanceof(o, c))) 
		  {
			  *new_spp = (Cell *)(((u1 *)fp) - framesize - SIZEOF_VOID_P);
			  *new_fpp = fp;
			  return (Inst *) (ex->handlerpc);
		  }
	  }

#if defined(ENABLE_THREADS)
	  /* is this method synchronized? */

	  if (issync) {
		  java_objectheader *syncobj;

		  /* get synchronization object */

		  if (m->flags & ACC_STATIC) {
			  syncobj = (java_objectheader *) m->clazz;
		  }
		  else {
			  syncobj = (java_objectheader *) access_local_cell(-framesize + SIZEOF_VOID_P);
		  }

		  assert(syncobj != NULL);

		  lock_monitor_exit(syncobj);
	  }
#endif /* defined(ENABLE_THREADS) */

	  /* unwind stack frame */
	  ip = (Inst *)access_local_cell(-framesize - SIZEOF_VOID_P);
	  fp = (Cell *)access_local_cell(-framesize);
  }

  return NULL; 
}


void intrp_asm_abstractmethoderror(void)
{
	vm_abort("intrp_asm_abstractmethoderror: IMPLEMENT ME!");
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
