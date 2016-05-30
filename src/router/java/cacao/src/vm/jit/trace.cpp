/* src/vm/jit/trace.cpp - Functions for tracing from java code.

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

#include "vm/jit/trace.hpp"
#include <cstdio>
#include "config.h"                     // for ENABLE_DEBUG_FILTER, etc
#include "md-abi.hpp"
#include "native/llni.hpp"
#include "threads/thread.hpp"           // for threadobject
#include "toolbox/logging.hpp"
#include "toolbox/buffer.hpp"           // for Buffer
#include "vm/array.hpp"
#include "vm/class.hpp"                 // for classinfo
#include "vm/descriptor.hpp"            // for methoddesc, typedesc
#include "vm/global.hpp"                // for imm_union, java_object_t, etc
#include "vm/globals.hpp"               // for class_java_lang_Class, etc
#include "vm/hook.hpp"                  // for method_enter, method_exit
#include "vm/javaobjects.hpp"           // for java_lang_Throwable, etc
#include "vm/jit/argument.hpp"          // for argument_jitarray_load, etc
#include "vm/jit/code.hpp"              // for codeinfo
#include "vm/jit/codegen-common.hpp"
#include "vm/jit/show.hpp"
#include "vm/method.hpp"                // for methodinfo, etc
#include "vm/options.hpp"               // for opt_TraceBuiltinCalls, etc
#include "vm/string.hpp"                // for JavaString
#include "vm/types.hpp"                 // for s4
#include "vm/utf8.hpp"                  // for Utf8String
#include "vm/vftbl.hpp"                 // for vftbl_t

#if !defined(NDEBUG)

/* trace_java_call_print_argument **********************************************

   XXX: Document me!

*******************************************************************************/

static void trace_java_call_print_argument(Buffer<>& logtext, methodinfo *m, typedesc *paramtype, imm_union imu)
{
	java_object_t *o;
	classinfo     *c;
	Utf8String     u;

	switch (paramtype->type) {
	case TYPE_INT:
		logtext.write_dec(imu.i).write(" (").write_hex(imu.i).write(')');
		break;
	case TYPE_LNG:
		logtext.write_dec(imu.l).write(" (").write_hex(imu.l).write(')');
		break;
	case TYPE_FLT:
		logtext.write_dec(imu.f).write(" (").write_hex(imu.f).write(')');
		break;
	case TYPE_DBL:
		logtext.write_dec(imu.d).write(" (").write_hex(imu.d).write(')');
		break;

	case TYPE_ADR:
		logtext.write_ptr(imu.a);

		/* Workaround for sun.misc.Unsafe methods.  In the future
		   (exact GC) we should check if the address is on the GC
		   heap. */

		if ((m->clazz       != NULL) &&
			(m->clazz->name == Utf8String::from_utf8("sun/misc/Unsafe")))
			break;

		/* Cast to java.lang.Object. */

		o = (java_handle_t*) (uintptr_t) imu.l;

		/* Check return argument for java.lang.Class or
		   java.lang.String. */

		if (o != NULL) {
			if (o->vftbl->clazz == class_java_lang_String) {
				/* convert java.lang.String object to utf8 string and strcat it to the logtext */

				logtext.write(" (String = \"")
				       .write(o)
				       .write("\")");
			}
			else {
				if (o->vftbl->clazz == class_java_lang_Class) {
					/* if the object returned is a java.lang.Class
					   cast it to classinfo structure and get the name
					   of the class */

					c = (classinfo *) o;

					u = c->name;
				}
				else {
					/* if the object returned is not a java.lang.String or
					   a java.lang.Class just print the name of the class */

					u = o->vftbl->clazz->name;
				}

				/* strcat to the logtext */

				logtext.write(" (Class = \"")
				       .write_slash_to_dot(u)
				       .write("\")");
			}
		}
		default:
			assert(false);
			break;
	}
}

/* trace_java_call_enter ******************************************************
 
   Traces an entry into a java method.

   arg_regs: Array containing all argument registers as int64_t values in
   the same order as listed in m->methoddesc. The array is usually allocated
   on the stack and used for restoring the argument registers later.

   stack: Pointer to first on stack argument in the same format passed to 
   asm_vm_call_method.

*******************************************************************************/

void trace_java_call_enter(methodinfo *m, uint64_t *arg_regs, uint64_t *stack)
{
	methoddesc *md;
	imm_union   arg;
	s4          i;

	/* We can only trace "slow" builtin functions (those with a stub)
	 * here, because the argument passing of "fast" ones happens via
	 * the native ABI and does not fit these functions. */

	if (method_is_builtin(m)) {
		if (!opt_TraceBuiltinCalls)
			return;
	}
	else {
		if (!opt_TraceJavaCalls)
			return;
#if defined(ENABLE_DEBUG_FILTER)
		if (!show_filters_test_verbosecall_enter(m))
			return;
#endif
	}

	// Hook point on entry into Java method.
	Hook::method_enter(m);

	md = m->parseddesc;

	// Create new dump memory area.
//	DumpMemoryArea dma;

	Buffer<> logtext(128);

	TRACEJAVACALLCOUNT++;

	logtext.writef("%10d ", TRACEJAVACALLCOUNT);
	logtext.writef("-%d-", TRACEJAVACALLINDENT);

	for (i = 0; i < TRACEJAVACALLINDENT; i++)
		logtext.write("\t");

	logtext.write("called: ");

	if (m->clazz != NULL)
		logtext.write_slash_to_dot(m->clazz->name);
	else
		logtext.write("NULL");
	logtext.write(".");
	logtext.write(m->name);
	logtext.write(m->descriptor);

	if (m->flags & ACC_PUBLIC)         logtext.write(" PUBLIC");
	if (m->flags & ACC_PRIVATE)        logtext.write(" PRIVATE");
	if (m->flags & ACC_PROTECTED)      logtext.write(" PROTECTED");
	if (m->flags & ACC_STATIC)         logtext.write(" STATIC");
	if (m->flags & ACC_FINAL)          logtext.write(" FINAL");
	if (m->flags & ACC_SYNCHRONIZED)   logtext.write(" SYNCHRONIZED");
	if (m->flags & ACC_VOLATILE)       logtext.write(" VOLATILE");
	if (m->flags & ACC_TRANSIENT)      logtext.write(" TRANSIENT");
	if (m->flags & ACC_NATIVE)         logtext.write(" NATIVE");
	if (m->flags & ACC_INTERFACE)      logtext.write(" INTERFACE");
	if (m->flags & ACC_ABSTRACT)       logtext.write(" ABSTRACT");

	logtext.write("(");

	for (i = 0; i < md->paramcount; ++i) {
		arg = argument_jitarray_load(md, i, arg_regs, stack);
		trace_java_call_print_argument(logtext, m, &md->paramtypes[i], arg);
		
		if (i != (md->paramcount - 1)) {
			logtext.write(", ");
		}
	}

	logtext.write(")");

	log_text(logtext.c_str());

	TRACEJAVACALLINDENT++;
}

/* trace_java_call_exit ********************************************************

   Traces an exit form a java method.

   return_regs: Array of size 1 containing return register.
   The array is usually allocated on the stack and used for restoring the
   registers later.

*******************************************************************************/

void trace_java_call_exit(methodinfo *m, uint64_t *return_regs)
{
	methoddesc *md;
	s4          i;
	imm_union   val;

	/* We can only trace "slow" builtin functions (those with a stub)
	 * here, because the argument passing of "fast" ones happens via
	 * the native ABI and does not fit these functions. */

	if (method_is_builtin(m)) {
		if (!opt_TraceBuiltinCalls)
			return;
	}
	else {
		if (!opt_TraceJavaCalls)
			return;
#if defined(ENABLE_DEBUG_FILTER)
		if (!show_filters_test_verbosecall_exit(m))
			return;
#endif
	}

	// Hook point upon exit from Java method.
	Hook::method_exit(m);

	md = m->parseddesc;

	/* outdent the log message */

	if (TRACEJAVACALLINDENT)
		TRACEJAVACALLINDENT--;
	else
		log_text("trace_java_call_exit: WARNING: unmatched unindent");

	// Create new dump memory area.
//	DumpMemoryArea dma;

	Buffer<> logtext(128);

	/* generate the message */

	logtext.write("           ");
	logtext.writef("-%d-", TRACEJAVACALLINDENT);

	for (i = 0; i < TRACEJAVACALLINDENT; i++)
		logtext.write("\t");

	logtext.write("finished: ");
	if (m->clazz != NULL)
		logtext.write_slash_to_dot(m->clazz->name);
	else
		logtext.write("NULL");
	logtext.write(".");
	logtext.write(m->name);
	logtext.write(m->descriptor);

	if (!IS_VOID_TYPE(md->returntype.type)) {
		logtext.write("->");
		val = argument_jitreturn_load(md, return_regs);

		trace_java_call_print_argument(logtext, m, &md->returntype, val);
	}

	log_text(logtext.c_str());
}


/* trace_exception *************************************************************

   Traces an exception which is handled by exceptions_handle_exception.

*******************************************************************************/

void trace_exception(java_object_t *xptr, methodinfo *m, void *pos)
{
	codeinfo *code;

	// Create new dump memory area.
//	DumpMemoryArea dma;

	Buffer<> logtext(128);

	if (xptr) {
		logtext.write("Exception ");
		logtext.write_slash_to_dot(xptr->vftbl->clazz->name);

	} else {
		logtext.write("Some Throwable");
	}

	logtext.write(" thrown in ");

	if (m) {
		logtext.write_slash_to_dot(m->clazz->name);
		logtext.write(".");
		logtext.write(m->name);
		logtext.write(m->descriptor);

		if (m->flags & ACC_SYNCHRONIZED)
			logtext.write("(SYNC");
		else
			logtext.write("(NOSYNC");

		if (m->flags & ACC_NATIVE) {
			logtext.write(",NATIVE");

			code = m->code;

			logtext.write(")(").write_ptr(code->entrypoint)
			       .write(") at position ").write_ptr(pos);
		} else {

			/* XXX preliminary: This should get the actual codeinfo */
			/* in which the exception happened.                     */
			code = m->code;
			
			logtext.write(")(").write_ptr(code->entrypoint)
			       .write(") at position ").write_ptr(pos);

			if (m->clazz->sourcefile == NULL)
				logtext.write("<NO CLASSFILE INFORMATION>");
			else
				logtext.write(m->clazz->sourcefile);

			logtext.writef(":%d)", 0);
		}

	} else
		logtext.write("call_java_method");

	log_text(logtext.c_str());
}


/* trace_exception_builtin *****************************************************

   Traces an exception which is thrown by builtin_throw_exception.

*******************************************************************************/

void trace_exception_builtin(java_handle_t* h)
{
	java_lang_Throwable jlt(h);

	// Get detail message.
	java_handle_t* s = NULL;

	if (jlt.get_handle() != NULL)
		s = jlt.get_detailMessage();

	java_lang_String jls(s);

	// Create new dump memory area.
//	DumpMemoryArea dma;

	Buffer<> logtext(128);

	logtext.write("Builtin exception thrown: ");

	if (jlt.get_handle()) {
		logtext.write_slash_to_dot(jlt.get_vftbl()->clazz->name);

		if (s) {
			logtext.write(": ");
			logtext.write(JavaString(jls.get_handle()));
		}

	} else {
		logtext.write("(nil)");
	}

	log_text(logtext.c_str());
}

#endif /* !defined(NDEBUG) */


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
