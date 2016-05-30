/* src/vm/jit/stacktrace.cpp - machine independent stacktrace system

   Copyright (C) 1996-2013
   CACAOVM - Verein zur Foerderung der freien virtuellen Maschine CACAO
   Copyright (C) 2009 Theobroma Systems Ltd.

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
#include <cstdlib>
#include <cstring>
#include <stdint.h>

#include "vm/types.hpp"

#include "arch.hpp"
#include "md.hpp"

#include "mm/gc.hpp"

#include "vm/jit/stacktrace.hpp"

#include "native/llni.hpp"

#include "threads/thread.hpp"

#include "toolbox/logging.hpp"

#include "vm/array.hpp"
#include "vm/jit/builtin.hpp"
#include "vm/class.hpp"
#include "vm/cycles-stats.hpp"
#include "vm/exceptions.hpp"
#include "vm/globals.hpp"
#include "vm/javaobjects.hpp"
#include "vm/loader.hpp"
#include "vm/method.hpp"
#include "vm/options.hpp"
#include "vm/string.hpp"
#include "vm/vm.hpp"

#include "vm/jit/codegen-common.hpp"
#include "vm/jit/linenumbertable.hpp"
#include "vm/jit/methodheader.hpp"
#include "vm/jit/methodtree.hpp"

/* global variables ***********************************************************/

CYCLES_STATS_DECLARE(stacktrace_overhead        , 100, 1)
CYCLES_STATS_DECLARE(stacktrace_fillInStackTrace, 40,  5000)
CYCLES_STATS_DECLARE(stacktrace_get,              40,  5000)
CYCLES_STATS_DECLARE(stacktrace_getClassContext , 40,  5000)
CYCLES_STATS_DECLARE(stacktrace_getCurrentClass , 40,  5000)
CYCLES_STATS_DECLARE(stacktrace_get_stack       , 40,  10000)


/* stacktrace_stackframeinfo_add ***********************************************

   Fills a stackframe info structure with the given or calculated
   values and adds it to the chain.

*******************************************************************************/

void stacktrace_stackframeinfo_add(stackframeinfo_t* sfi, void* pv, void* sp, void* ra, void* xpc)
{
	stackframeinfo_t *currentsfi;
	codeinfo         *code;
#if defined(ENABLE_JIT)
	s4                 framesize;
#endif

	/* Get current stackframe info. */

	currentsfi = threads_get_current_stackframeinfo();

	/* sometimes we don't have pv handy (e.g. in asmpart.S:
       L_asm_call_jit_compiler_exception or in the interpreter). */

	if (pv == NULL) {
#if defined(ENABLE_INTRP)
		if (opt_intrp)
			pv = methodtree_find(ra);
		else
#endif
			{
#if defined(ENABLE_JIT)
# if defined(__SPARC_64__)
				pv = md_get_pv_from_stackframe(sp);
# else
				pv = md_codegen_get_pv_from_pc(ra);
# endif
#endif
			}
	}

	/* Get codeinfo pointer for the parent Java method. */

	code = code_get_codeinfo_for_pv(pv);

	/* XXX */
	/* 	assert(m != NULL); */

#if defined(ENABLE_JIT)
# if defined(ENABLE_INTRP)
	/* When using the interpreter, we pass RA to the function. */

	if (!opt_intrp) {
# endif
# if defined(__I386__) || defined(__X86_64__) || defined(__S390__)
		/* On i386 and x86_64 we always have to get the return address
		   from the stack. */
		/* On S390 we use REG_RA as REG_ITMP3, so we have always to get
		   the RA from stack. */

		framesize = md_stacktrace_get_framesize(code);

		ra = md_stacktrace_get_returnaddress(sp, framesize);
# else
		/* If the method is a non-leaf function, we need to get the
		   return address from the stack.  For leaf functions the
		   return address is set correctly.  This makes the assembler
		   and the signal handler code simpler.  The code is NULL is
		   the asm_vm_call_method special case. */

		if ((code == NULL) || !code_is_leafmethod(code)) {
			framesize = md_stacktrace_get_framesize(code);

			ra = md_stacktrace_get_returnaddress(sp, framesize);
		}
# endif
# if defined(ENABLE_INTRP)
	}
# endif
#endif

	/* Calculate XPC when not given.  The XPC is then the return
	   address of the current method minus 1 because the RA points to
	   the instruction after the call instruction.  This is required
	   e.g. for method stubs. */

	if (xpc == NULL) {
		xpc = (void *) (((intptr_t) ra) - 1);
	}

	/* Fill new stackframeinfo structure. */

	sfi->prev = currentsfi;
	sfi->code = code;
	sfi->pv   = pv;
	sfi->sp   = sp;
	sfi->ra   = ra;
	sfi->xpc  = xpc;

#if !defined(NDEBUG)
	if (opt_DebugStackFrameInfo) {
		log_start();
		log_print("[stackframeinfo add   : sfi=%p, method=%p, pv=%p, sp=%p, ra=%p, xpc=%p, method=",
				  sfi, sfi->code->m, sfi->pv, sfi->sp, sfi->ra, sfi->xpc);
		method_print(sfi->code->m);
		log_print("]");
		log_finish();
	}
#endif

	/* Store new stackframeinfo pointer. */

	threads_set_current_stackframeinfo(sfi);

	/* set the native world flag for the current thread */
	/* ATTENTION: This flag tells the GC how to treat this thread in case of
	   a collection. Set this flag _after_ a valid stackframe info was set. */

	THREAD_NATIVEWORLD_ENTER;
}


/* stacktrace_stackframeinfo_remove ********************************************

   Remove the given stackframeinfo from the chain in the current
   thread.

*******************************************************************************/

void stacktrace_stackframeinfo_remove(stackframeinfo_t *sfi)
{
	/* Clear the native world flag for the current thread. */
	/* ATTENTION: Clear this flag _before_ removing the stackframe info. */

	THREAD_NATIVEWORLD_EXIT;

#if !defined(NDEBUG)
	if (opt_DebugStackFrameInfo) {
		log_start();
		log_print("[stackframeinfo remove: sfi=%p, method=%p, pv=%p, sp=%p, ra=%p, xpc=%p, method=",
				  sfi, sfi->code->m, sfi->pv, sfi->sp, sfi->ra, sfi->xpc);
		method_print(sfi->code->m);
		log_print("]");
		log_finish();
	}
#endif

	/* Set previous stackframe info. */

	threads_set_current_stackframeinfo(sfi->prev);
}


/* stacktrace_stackframeinfo_fill **********************************************

   Fill the temporary stackframeinfo structure with the values given
   in sfi.

   IN:
       tmpsfi ... temporary stackframeinfo
       sfi ...... stackframeinfo to be used in the next iteration

*******************************************************************************/

static inline void stacktrace_stackframeinfo_fill(stackframeinfo_t *tmpsfi, stackframeinfo_t *sfi)
{
	/* Sanity checks. */

	assert(tmpsfi != NULL);
	assert(sfi != NULL);

	/* Fill the temporary stackframeinfo. */

	tmpsfi->code = sfi->code;
	tmpsfi->pv   = sfi->pv;
	tmpsfi->sp   = sfi->sp;
	tmpsfi->ra   = sfi->ra;
	tmpsfi->xpc  = sfi->xpc;

	/* Set the previous stackframe info of the temporary one to the
	   next in the chain. */

	tmpsfi->prev = sfi->prev;

#if !defined(NDEBUG)
	if (opt_DebugStackTrace)
		log_println("[stacktrace fill]");
#endif
}


/* stacktrace_stackframeinfo_next **********************************************

   Walk the stack (or the stackframeinfo-chain) to the next method and
   return the new stackframe values in the temporary stackframeinfo
   passed.

   ATTENTION: This function does NOT skip builtin methods!

   IN:
       tmpsfi ... temporary stackframeinfo of current method

*******************************************************************************/

static inline void stacktrace_stackframeinfo_next(stackframeinfo_t *tmpsfi)
{
	codeinfo         *code;
	void             *pv;
	void             *sp;
	void             *ra;
	//void             *xpc;
	uint32_t          framesize;
	stackframeinfo_t *prevsfi;

	/* Sanity check. */

	assert(tmpsfi != NULL);

	/* Get values from the stackframeinfo. */

	code = tmpsfi->code;
	pv   = tmpsfi->pv;
	sp   = tmpsfi->sp;
	ra   = tmpsfi->ra;
	//xpc  = tmpsfi->xpc;

	/* Get the current stack frame size. */

	framesize = md_stacktrace_get_framesize(code);

	/* Get the RA of the current stack frame (RA to the parent Java
	   method) if the current method is a non-leaf method.  Otherwise
	   the value in the stackframeinfo is correct (from the signal
	   handler). */

#if defined(ENABLE_JIT)
# if defined(ENABLE_INTRP)
	if (opt_intrp)
		ra = intrp_md_stacktrace_get_returnaddress(sp, framesize);
	else
# endif
		{
			if (!code_is_leafmethod(code))
				ra = md_stacktrace_get_returnaddress(sp, framesize);
		}
#else
	ra = intrp_md_stacktrace_get_returnaddress(sp, framesize);
#endif

	/* Get the PV for the parent Java method. */

#if defined(ENABLE_INTRP)
	if (opt_intrp)
		pv = methodtree_find(ra);
	else
#endif
		{
#if defined(ENABLE_JIT)
# if defined(__SPARC_64__)
			sp = md_get_framepointer(sp);
			pv = md_get_pv_from_stackframe(sp);
# else
			pv = md_codegen_get_pv_from_pc(ra);
# endif
#endif
		}

	/* Get the codeinfo pointer for the parent Java method. */

	code = code_get_codeinfo_for_pv(pv);

	/* Calculate the SP for the parent Java method. */

#if defined(ENABLE_INTRP)
	if (opt_intrp)
		sp = *(u1 **) (sp - framesize);
	else
#endif
		{
#if STACKFRMAE_RA_BETWEEN_FRAMES
			sp = (void *) (((intptr_t) sp) + framesize + SIZEOF_VOID_P);
#elif defined(__SPARC_64__)
			/* already has the new sp */
#else
			sp = (void *) (((intptr_t) sp) + framesize);
#endif
		}

	/* If the new codeinfo pointer is NULL we reached a
	   asm_vm_call_method function.  In this case we get the next
	   values from the previous stackframeinfo in the chain.
	   Otherwise the new values have been calculated before. */

	if (code == NULL) {
		prevsfi = tmpsfi->prev;

		/* If the previous stackframeinfo in the chain is NULL we
		   reached the top of the stacktrace.  We set code and prev to
		   NULL to mark the end, which is checked in
		   stacktrace_stackframeinfo_end_check. */

		if (prevsfi == NULL) {
			tmpsfi->code = NULL;
			tmpsfi->prev = NULL;
			return;
		}

		/* Fill the temporary stackframeinfo with the new values. */

		stacktrace_stackframeinfo_fill(tmpsfi, prevsfi);
	}
	else {
		/* Store the new values in the stackframeinfo.  NOTE: We
		   subtract 1 from the RA to get the XPC, because the RA
		   points to the instruction after the call instruction. */

		tmpsfi->code = code;
		tmpsfi->pv   = pv;
		tmpsfi->sp   = sp;
		tmpsfi->ra   = ra;
		tmpsfi->xpc  = (void *) (((intptr_t) ra) - 1);
	}

#if !defined(NDEBUG)
	/* Print current method information. */

	if (opt_DebugStackTrace) {
		log_start();
		log_print("[stacktrace: method=%p, pv=%p, sp=%p, ra=%p, xpc=%p, method=",
				  tmpsfi->code->m, tmpsfi->pv, tmpsfi->sp, tmpsfi->ra,
				  tmpsfi->xpc);
		method_print(tmpsfi->code->m);
		log_print("]");
		log_finish();
	}
#endif
}


/* stacktrace_stackframeinfo_end_check *****************************************

   Check if we reached the end of the stacktrace.

   IN:
       tmpsfi ... temporary stackframeinfo of current method

   RETURN:
       true .... the end is reached
	   false ... the end is not reached

*******************************************************************************/

static inline bool stacktrace_stackframeinfo_end_check(stackframeinfo_t *tmpsfi)
{
	/* Sanity check. */

	assert(tmpsfi != NULL);

	if ((tmpsfi->code == NULL) && (tmpsfi->prev == NULL)) {
#if !defined(NDEBUG)
		if (opt_DebugStackTrace)
			log_println("[stacktrace stop]");
#endif

		return true;
	}

	return false;
}


/* stacktrace_depth ************************************************************

   Calculates and returns the depth of the current stacktrace.

   IN:
       sfi ... stackframeinfo where to start the stacktrace

   RETURN:
       depth of the stacktrace

*******************************************************************************/

static int stacktrace_depth(stackframeinfo_t *sfi)
{
	stackframeinfo_t  tmpsfi;
	int               depth;
	methodinfo       *m;

#if !defined(NDEBUG)
	if (opt_DebugStackTrace)
		log_println("[stacktrace_depth]");
#endif

	/* XXX This is not correct, but a workaround for threads-dump for
	   now. */
/* 	assert(sfi != NULL); */
	if (sfi == NULL)
		return 0;

	/* Iterate over all stackframes. */

	depth = 0;

	for (stacktrace_stackframeinfo_fill(&tmpsfi, sfi);
		 stacktrace_stackframeinfo_end_check(&tmpsfi) == false;
		 stacktrace_stackframeinfo_next(&tmpsfi)) {
		/* Get methodinfo. */

		m = tmpsfi.code->m;

		/* Skip builtin methods. */

		if (m->flags & ACC_METHOD_BUILTIN)
			continue;

		depth++;
	}

	return depth;
}


/* stacktrace_get **************************************************************

   Builds and returns a stacktrace starting from the given stackframe
   info and returns the stacktrace structure wrapped in a Java
   byte-array to not confuse the GC.

   IN:
       sfi ... stackframe info to start stacktrace from

   RETURN:
       stacktrace as Java byte-array

*******************************************************************************/

java_handle_bytearray_t *stacktrace_get(stackframeinfo_t *sfi)
{
	stacktrace_t       *st;
	stacktrace_entry_t *ste;

	CYCLES_STATS_DECLARE_AND_START_WITH_OVERHEAD

#if !defined(NDEBUG)
	if (opt_DebugStackTrace)
		log_println("[stacktrace_get]");
#endif

	bool skip_fillInStackTrace = true;
	bool skip_init             = true;

	int depth = stacktrace_depth(sfi);

	if (depth == 0)
		return NULL;

	// Allocate memory from the GC heap and copy the stacktrace buffer.
	// ATTENTION: Use a Java byte-array for this to not confuse the GC.
	// FIXME: We waste some memory here as we skip some entries later.

	int32_t ba_size = sizeof(stacktrace_t) + sizeof(stacktrace_entry_t) * depth;

	ByteArray ba(ba_size);

	if (ba.is_null())
		goto return_NULL;

	// Get a stacktrace entry pointer.
	// ATTENTION: We need a critical section here because we use the
	// byte-array data pointer directly.

	LLNI_CRITICAL_START;

	st  = (stacktrace_t *) ba.get_raw_data_ptr();
	ste = st->entries;

	// Iterate over the whole stack.
	stackframeinfo_t tmpsfi;

	for (stacktrace_stackframeinfo_fill(&tmpsfi, sfi);
		 stacktrace_stackframeinfo_end_check(&tmpsfi) == false;
		 stacktrace_stackframeinfo_next(&tmpsfi)) {
		// Get the methodinfo

		methodinfo *m = tmpsfi.code->m;

		// Skip builtin methods

		if (m->flags & ACC_METHOD_BUILTIN)
		continue;

		// This logic is taken from
		// hotspot/src/share/vm/classfile/javaClasses.cpp
		// (java_lang_Throwable::fill_in_stack_trace)

		// the current stack contains the following frames:
		// * one  or more fillInStackTrace frames for the exception class, we skip these
		// * zero or more <init>           frames for the exception class, we skip these
		// * rest of the stack

		// skip Throwable.fillInStackTrace

		if (skip_fillInStackTrace) {
			if (m->name == utf8::fillInStackTrace) {
				continue;
			} else {
				// we saw all fillInStackTrace frames, stop skipping
				skip_fillInStackTrace = false;
			}
		}

		// Skip <init> methods of any classes deriving from java.lang.Throwable

		if (skip_init == true) {
			if (m->name == utf8::init && class_issubclass(m->clazz, class_java_lang_Throwable)) {
				continue;
			} else {
				// we saw all <init> frames, stop skipping
				skip_init = false;
			}
		}

		// Store the stacktrace entry and increment the pointer.

		ste->code = tmpsfi.code;
		ste->pc   = tmpsfi.xpc;

		ste++;
	}

	// Store the number of entries in the stacktrace structure

	st->length = ste - st->entries;

	LLNI_CRITICAL_END;

	CYCLES_STATS_END_WITH_OVERHEAD(stacktrace_fillInStackTrace,
								   stacktrace_overhead)
	return ba.get_handle();

return_NULL:
	CYCLES_STATS_END_WITH_OVERHEAD(stacktrace_fillInStackTrace,
								   stacktrace_overhead)

	return NULL;
}


/* stacktrace_get_current ******************************************************

   Builds and returns a stacktrace from the current thread and returns
   the stacktrace structure wrapped in a Java byte-array to not
   confuse the GC.

   RETURN:
       stacktrace as Java byte-array

*******************************************************************************/

java_handle_bytearray_t *stacktrace_get_current(void)
{
	stackframeinfo_t        *sfi;
	java_handle_bytearray_t *ba;

	sfi = threads_get_current_stackframeinfo();
	ba  = stacktrace_get(sfi);

	return ba;
}


/**
 * Creates a java.lang.StackTraceElement for one element of the given
 * stacktrace.
 *
 * @param st Given stacktrace.
 * @param index Index of element inside stacktrace.
 * @return The filled StackTraceElement object.
 */
#if defined(ENABLE_JAVASE)
java_handle_t* stacktrace_get_StackTraceElement(stacktrace_t* st, int32_t index)
{
	assert(st != NULL);

	if ((index < 0) || (index >= st->length)) {
		/* XXX This should be an IndexOutOfBoundsException (check this
		   again). */
		exceptions_throw_arrayindexoutofboundsexception();
		return NULL;
	}

	// Get the stacktrace entry.
	stacktrace_entry_t* ste = &(st->entries[index]);

	// Get the codeinfo, methodinfo and classinfo.
	codeinfo*   code = ste->code;
	methodinfo* m    = code->m;
	classinfo*  c    = m->clazz;

	// Get filename.
	java_handle_t* filename;

	if (!(m->flags & ACC_NATIVE)) {
		if (c->sourcefile != NULL)
			filename = JavaString::literal(c->sourcefile);
		else
			filename = NULL;
	}
	else
		filename = NULL;

	// Get line number.
	int32_t linenumber;

	if (m->flags & ACC_NATIVE) {
#if defined(WITH_JAVA_RUNTIME_LIBRARY_GNU_CLASSPATH)
		linenumber = -1;
#elif defined(WITH_JAVA_RUNTIME_LIBRARY_OPENJDK)
		linenumber = -2;
#else
# error unknown classpath configuration
#endif
	}
	else {
		// FIXME linenumbertable->find could change the methodinfo
		// pointer when hitting an inlined method.
		linenumber = code->linenumbertable->find(&m, ste->pc);
		linenumber = (linenumber == 0) ? -1 : linenumber;
	}

	// Get declaring class name.
	java_handle_t* declaringclass = JavaString(class_get_classname(c)).intern();

#if defined(WITH_JAVA_RUNTIME_LIBRARY_GNU_CLASSPATH)
	// Allocate a new StackTraceElement object.
	java_handle_t* h = builtin_new(class_java_lang_StackTraceElement);

	if (h == NULL)
			return NULL;

	java_lang_StackTraceElement jlste(h, filename, linenumber, declaringclass, JavaString::from_utf8(m->name), ((m->flags & ACC_NATIVE) ? 1 : 0));
#elif defined(WITH_JAVA_RUNTIME_LIBRARY_OPENJDK)
	// Allocate a new StackTraceElement object.
	java_lang_StackTraceElement jlste(declaringclass, JavaString::literal(m->name), filename, linenumber);

	if (jlste.is_null())
		return NULL;
#else
# error unknown classpath configuration
#endif

	return jlste.get_handle();
}
#endif


/**
 * Creates a complete array of java.lang.StackTraceElement objects
 * for the given stacktrace.
 *
 * @param st Given stacktrace.
 * @return Array of filled StackTraceElement objects.
 */
#if defined(ENABLE_JAVASE)
java_handle_objectarray_t* stacktrace_get_StackTraceElements(stacktrace_t* st)
{
	// Get length of stacktrace. If stacktrace is not available
	// an empty array should be returned.
	int32_t length = (st != NULL) ? st->length : 0;

	// Create the stacktrace element array.
	ObjectArray oa(length, class_java_lang_StackTraceElement);

	if (oa.is_null())
		return NULL;

	// Iterate over all stacktrace elements.
	for (int i = 0; i < length; i++) {

		// Get stacktrace element at current index.
		java_handle_t* h = stacktrace_get_StackTraceElement(st, i);

		if (h == NULL)
			return NULL;

		// Store stacktrace element in array.
		oa.set_element(i, h);
	}

	return oa.get_handle();
}
#endif


/* stacktrace_get_caller_class *************************************************

   Get the class on the stack at the given depth.  This function skips
   various special classes or methods.

   ARGUMENTS:
       depth ... depth to get caller class of

   RETURN:
       caller class

*******************************************************************************/

#if defined(ENABLE_JAVASE)
classinfo *stacktrace_get_caller_class(int depth)
{
	stackframeinfo_t *sfi;
	stackframeinfo_t  tmpsfi;
	methodinfo       *m;
	classinfo        *c;
	int               i;

#if !defined(NDEBUG)
	if (opt_DebugStackTrace)
		log_println("[stacktrace_get_caller_class]");
#endif

	/* Get the stackframeinfo of the current thread. */

	sfi = threads_get_current_stackframeinfo();

	/* Iterate over the whole stack until we reached the requested
	   depth. */

	i = 0;

	for (stacktrace_stackframeinfo_fill(&tmpsfi, sfi);
		 stacktrace_stackframeinfo_end_check(&tmpsfi) == false;
		 stacktrace_stackframeinfo_next(&tmpsfi)) {

		m = tmpsfi.code->m;
		c = m->clazz;

		/* Skip builtin methods. */

		if (m->flags & ACC_METHOD_BUILTIN)
			continue;

#if defined(WITH_JAVA_RUNTIME_LIBRARY_OPENJDK)
		/* NOTE: See hotspot/src/share/vm/runtime/vframe.cpp
		   (vframeStreamCommon::security_get_caller_frame). */

		/* This is java.lang.reflect.Method.invoke(), skip it. */

		if (m == method_java_lang_reflect_Method_invoke)
			continue;

		/* This is an auxiliary frame, skip it. */

		if (class_issubclass(c, class_sun_reflect_MagicAccessorImpl))
			continue;
#endif

		/* We reached the requested depth. */

		if (i >= depth)
			return c;

		i++;
	}

	return NULL;
}
#endif


/**
 * Returns the first non-null (user-defined) classloader on the stack.
 *
 * @return The first non-null classloader or NULL if none is found.
 */
classloader_t* stacktrace_first_nonnull_classloader(void)
{
	stackframeinfo_t *sfi;
	stackframeinfo_t  tmpsfi;
	methodinfo       *m;
	classloader_t    *cl;

#if !defined(NDEBUG)
	if (opt_DebugStackTrace)
		log_println("[stacktrace_first_nonnull_classloader]");
#endif

	/* Get the stackframeinfo of the current thread. */

	sfi = threads_get_current_stackframeinfo();

	/* Iterate over the whole stack. */

	for (stacktrace_stackframeinfo_fill(&tmpsfi, sfi);
		 stacktrace_stackframeinfo_end_check(&tmpsfi) == false;
		 stacktrace_stackframeinfo_next(&tmpsfi)) {

		m  = tmpsfi.code->m;
		cl = class_get_classloader(m->clazz);

#if defined(WITH_JAVA_RUNTIME_LIBRARY_OPENJDK)
		/* NOTE: See hotspot/src/share/vm/runtime/vframe.cpp
		   (vframeStreamCommon::skip_reflection_related_frames). */
		if (class_issubclass(m->clazz, class_sun_reflect_MethodAccessorImpl) ||
			class_issubclass(m->clazz, class_sun_reflect_ConstructorAccessorImpl))
			continue;
#endif

		if (cl != NULL)
			return cl;
	}

	return NULL;
}


/**
 * Checks if a given classloader is equal to the the second classloader
 * or one of its ancestors (parents).
 *
 * XXX: This helper method should be moved to java_lang_Classloader.
 */
#if defined(ENABLE_JAVASE)
static bool is_ancestor_of(classloader_t* loader, classloader_t* parent)
{
	// Iterate over chain of possible parents.
	while (parent != NULL) {

		// Check if given loader is parent.
		if (loader == parent)
			return true;

		// Jump to next parent.
		java_lang_ClassLoader jlcl(parent);
		parent = jlcl.get_parent();
	}

	return false;
}
#endif /* defined(ENABLE_JAVASE) */


/**
 * Returns the first non-system (user-defined) classloader on the stack.
 * A non-system classloader is a non-null classloader being not equal to
 * the system classloader (or one of its ancestors).
 *
 * @return The first non-system classloader or NULL if none is found.
 */
#if defined(ENABLE_JAVASE)
classloader_t* stacktrace_first_nonsystem_classloader(void)
{
	stackframeinfo_t *sfi;
	stackframeinfo_t  tmpsfi;
	methodinfo       *m;
	classloader_t    *cl;
	classloader_t    *syscl;

#if !defined(NDEBUG)
	if (opt_DebugStackTrace)
		log_println("[stacktrace_first_nonsystem_classloader]");
#endif

	// Get the stackframeinfo of the current thread.
	sfi = threads_get_current_stackframeinfo();

	// Get the system class class loader.
	syscl = java_lang_ClassLoader::invoke_getSystemClassLoader();

	// Iterate over the whole stack.
	for (stacktrace_stackframeinfo_fill(&tmpsfi, sfi);
		 stacktrace_stackframeinfo_end_check(&tmpsfi) == false;
		 stacktrace_stackframeinfo_next(&tmpsfi)) {

		m  = tmpsfi.code->m;
		cl = class_get_classloader(m->clazz);

		if (cl == NULL)
			continue;

		// XXX if a method in a class in a trusted loader is in a
		// doPrivileged, return NULL (or break) here.

		if (!is_ancestor_of(cl, syscl))
			return cl;
	}

	return NULL;
}
#endif /* defined(ENABLE_JAVASE) */


/* stacktrace_getClassContext **************************************************

   Creates a Class context array.

   RETURN VALUE:
      the array of java.lang.Class objects, or
	  NULL if an exception has been thrown

*******************************************************************************/

java_handle_objectarray_t *stacktrace_getClassContext(void)
{
	stackframeinfo_t           *sfi;
	stackframeinfo_t            tmpsfi;
	int                         depth;
	int                         i;
	methodinfo                 *m;

	CYCLES_STATS_DECLARE_AND_START

#if !defined(NDEBUG)
	if (opt_DebugStackTrace)
		log_println("[stacktrace_getClassContext]");
#endif

	sfi = threads_get_current_stackframeinfo();

	/* Get the depth of the current stack. */

	depth = stacktrace_depth(sfi);

	/* The first stackframe corresponds to the method whose
	   implementation calls this native function.  We remove that
	   entry. */

	depth--;
	stacktrace_stackframeinfo_fill(&tmpsfi, sfi);
	stacktrace_stackframeinfo_next(&tmpsfi);

	/* Allocate the Class array. */

	ClassArray ca(depth);

	if (ca.is_null()) {
		CYCLES_STATS_END(stacktrace_getClassContext);

		return NULL;
	}

	/* Fill the Class array from the stacktrace list. */
	/* Iterate over the whole stack. */

	i = 0;

	for (;
		 stacktrace_stackframeinfo_end_check(&tmpsfi) == false;
		 stacktrace_stackframeinfo_next(&tmpsfi)) {
		/* Get methodinfo. */

		m = tmpsfi.code->m;

		/* Skip builtin methods. */

		if (m->flags & ACC_METHOD_BUILTIN)
			continue;

		/* Store the class in the array. */

		ca.set_element(i, m->clazz);

		i++;
	}

	LLNI_CRITICAL_END;

	CYCLES_STATS_END(stacktrace_getClassContext)

	return ca.get_handle();
}


/* stacktrace_getCurrentClass **************************************************

   Find the current class by walking the stack trace.

   Quote from the JNI documentation:
	 
   In the Java 2 Platform, FindClass locates the class loader
   associated with the current native method.  If the native code
   belongs to a system class, no class loader will be
   involved. Otherwise, the proper class loader will be invoked to
   load and link the named class. When FindClass is called through the
   Invocation Interface, there is no current native method or its
   associated class loader. In that case, the result of
   ClassLoader.getBaseClassLoader is used."

*******************************************************************************/

#if defined(ENABLE_JAVASE)
classinfo *stacktrace_get_current_class(void)
{
	stackframeinfo_t *sfi;
	stackframeinfo_t  tmpsfi;
	methodinfo       *m;

	CYCLES_STATS_DECLARE_AND_START;

#if !defined(NDEBUG)
	if (opt_DebugStackTrace)
		log_println("[stacktrace_get_current_class]");
#endif

	/* Get the stackframeinfo of the current thread. */

	sfi = threads_get_current_stackframeinfo();

	/* If the stackframeinfo is NULL then FindClass is called through
	   the Invocation Interface and we return NULL */

	if (sfi == NULL) {
		CYCLES_STATS_END(stacktrace_getCurrentClass);

		return NULL;
	}

	/* Iterate over the whole stack. */

	for (stacktrace_stackframeinfo_fill(&tmpsfi, sfi);
		 stacktrace_stackframeinfo_end_check(&tmpsfi) == false;
		 stacktrace_stackframeinfo_next(&tmpsfi)) {
		/* Get the methodinfo. */

		m = tmpsfi.code->m;

		if (m->clazz == class_java_security_PrivilegedAction) {
			CYCLES_STATS_END(stacktrace_getCurrentClass);

			return NULL;
		}

		if (m->clazz != NULL) {
			CYCLES_STATS_END(stacktrace_getCurrentClass);

			return m->clazz;
		}
	}

	/* No Java method found on the stack. */

	CYCLES_STATS_END(stacktrace_getCurrentClass);

	return NULL;
}
#endif /* ENABLE_JAVASE */


/* stacktrace_get_stack ********************************************************

   Create a 2-dimensional array for java.security.VMAccessControler.

   RETURN VALUE:
      the arrary, or
         NULL if an exception has been thrown

*******************************************************************************/

#if defined(ENABLE_JAVASE) && defined(WITH_JAVA_RUNTIME_LIBRARY_GNU_CLASSPATH)
java_handle_objectarray_t *stacktrace_get_stack(void)
{
	stackframeinfo_t *sfi;
	stackframeinfo_t  tmpsfi;
	int               depth;
	methodinfo       *m;
	java_handle_t    *string;
	int               i;

	CYCLES_STATS_DECLARE_AND_START

#if !defined(NDEBUG)
	if (opt_DebugStackTrace)
		log_println("[stacktrace_get_stack]");
#endif

	/* Get the stackframeinfo of the current thread. */

	sfi = threads_get_current_stackframeinfo();

	/* Get the depth of the current stack. */

	depth = stacktrace_depth(sfi);

	if (depth == 0)
		return NULL;

	/* Allocate the required arrays. */

	ObjectArray oa(2, arrayclass_java_lang_Object);
	ClassArray  classes(depth);
	ObjectArray methodnames(depth, class_java_lang_String);

	if (oa.is_null())
		goto return_NULL;

	if (classes.is_null())
		goto return_NULL;

	if (methodnames.is_null())
		goto return_NULL;

	/* Set up the 2-dimensional array. */

	oa.set_element(0, (java_handle_t *) classes.get_handle());
	oa.set_element(1, (java_handle_t *) methodnames.get_handle());

	/* Iterate over the whole stack. */
	/* TODO We should use a critical section here to speed things
	   up. */

	i = 0;

	for (stacktrace_stackframeinfo_fill(&tmpsfi, sfi);
		 stacktrace_stackframeinfo_end_check(&tmpsfi) == false;
		 stacktrace_stackframeinfo_next(&tmpsfi)) {
		/* Get the methodinfo. */

		m = tmpsfi.code->m;

		/* Skip builtin methods. */

		if (m->flags & ACC_METHOD_BUILTIN)
			continue;

		/* Store the class in the array. */

		classes.set_element(i, m->clazz);

		/* Store the name in the array. */

		string = JavaString::from_utf8(m->name);

		if (string == NULL)
			goto return_NULL;

		methodnames.set_element(i, string);

		i++;
	}

	CYCLES_STATS_END(stacktrace_get_stack)

	return oa.get_handle();

return_NULL:
	CYCLES_STATS_END(stacktrace_get_stack)

	return NULL;
}
#endif


/* stacktrace_print_entry ****************************************************

   Print line for a stacktrace entry.

   ARGUMENTS:
       m ............ methodinfo of the entry
       linenumber ... linenumber of the entry

*******************************************************************************/

static void stacktrace_print_entry(methodinfo *m, int32_t linenumber)
{
	/* Sanity check. */

	assert(m != NULL);

	printf("\tat ");

	if (m->flags & ACC_METHOD_BUILTIN)
		printf("NULL");
	else
		utf_display_printable_ascii_classname(m->clazz->name);

	printf(".");
	utf_display_printable_ascii(m->name);
	utf_display_printable_ascii(m->descriptor);

	if (m->flags & ACC_NATIVE) {
		puts("(Native Method)");
	}
	else {
		if (m->flags & ACC_METHOD_BUILTIN) {
			puts("(builtin)");
		}
		else {
			printf("(");
			utf_display_printable_ascii(m->clazz->sourcefile);
			printf(":%d)\n", linenumber);
		}
	}

	fflush(stdout);
}


/* stacktrace_print ************************************************************

   Print the given stacktrace with CACAO intern methods only (no Java
   code required).

   This method is used by stacktrace_dump_trace and
   builtin_trace_exception.

   IN:
       st ... stacktrace to print

*******************************************************************************/

void stacktrace_print(stacktrace_t *st)
{
	stacktrace_entry_t *ste;
	methodinfo         *m;
	int32_t             linenumber;
	int                 i;

	ste = &(st->entries[0]);

	for (i = 0; i < st->length; i++, ste++) {
		m = ste->code->m;

		/* Get the line number. */

		linenumber = ste->code->linenumbertable->find(&m, ste->pc);

		stacktrace_print_entry(m, linenumber);
	}
}


/* stacktrace_print_current ****************************************************

   Print the current stacktrace of the current thread.

   NOTE: This function prints all frames of the stacktrace and does
   not skip frames like stacktrace_get.

*******************************************************************************/

void stacktrace_print_current(void)
{
	stackframeinfo_t *sfi;
	stackframeinfo_t  tmpsfi;
	codeinfo         *code;
	methodinfo       *m;
	int32_t           linenumber;

	sfi = threads_get_current_stackframeinfo();

	if (sfi == NULL) {
		puts("\t<<No stacktrace available>>");
		fflush(stdout);
		return;
	}

	for (stacktrace_stackframeinfo_fill(&tmpsfi, sfi);
		 stacktrace_stackframeinfo_end_check(&tmpsfi) == false;
		 stacktrace_stackframeinfo_next(&tmpsfi)) {
		/* Get the methodinfo. */

		code = tmpsfi.code;
		m    = code->m;

		// Get the line number.
		linenumber = code->linenumbertable->find(&m, tmpsfi.xpc);

		stacktrace_print_entry(m, linenumber);
	}
}


/**
 * Creates a stacktrace for the given thread.
 *
 * @param t Given thread.
 * @return Current stacktrace of the given thread.
 *
 * XXX: Creation of the stacktrace starts at the most recent
 * stackframeinfo block. If the thread is not inside the native
 * world, the created stacktrace is not complete!
 */
stacktrace_t* stacktrace_get_of_thread(threadobject* t)
{
	stackframeinfo_t*        sfi;
	java_handle_bytearray_t* stba;
	stacktrace_t*            st;

	sfi  = t->_stackframeinfo;
	stba = stacktrace_get(sfi);

	ByteArray ba(stba);

	if (ba.is_null())
		return NULL;

	st  = (stacktrace_t*) ba.get_raw_data_ptr();

	return st;
}


/* stacktrace_print_of_thread **************************************************

   Print the current stacktrace of the given thread. It will only work
   for suspended threads.

   ARGUMENTS:
       t ... thread

*******************************************************************************/

void stacktrace_print_of_thread(threadobject *t)
{
	stackframeinfo_t *sfi;
	stackframeinfo_t  tmpsfi;
	codeinfo         *code;
	methodinfo       *m;
	int32_t           linenumber;

	/* Build a stacktrace for the passed thread. */

	sfi = t->_stackframeinfo;
	
	if (!t->suspended || sfi == NULL) {
		puts("\t<<No stacktrace available>>");
		fflush(stdout);
		return;
	}

	for (stacktrace_stackframeinfo_fill(&tmpsfi, sfi);
		 stacktrace_stackframeinfo_end_check(&tmpsfi) == false;
		 stacktrace_stackframeinfo_next(&tmpsfi)) {
		/* Get the methodinfo. */

		code = tmpsfi.code;
		m    = code->m;

		// Get the line number.
		linenumber = code->linenumbertable->find(&m, tmpsfi.xpc);

		stacktrace_print_entry(m, linenumber);
	}
}


/* stacktrace_print_exception **************************************************

   Print the stacktrace of a given exception (more or less a wrapper
   to stacktrace_print).

   IN:
       h ... handle of exception to print

*******************************************************************************/

void stacktrace_print_exception(java_handle_t *h)
{
	if (h == NULL)
		return;

	java_lang_Throwable t(h);

	/* now print the stacktrace */

#if defined(WITH_JAVA_RUNTIME_LIBRARY_GNU_CLASSPATH)

	java_lang_VMThrowable vmt(t.get_vmState());
	ByteArray backtrace(vmt.get_vmdata());

#elif defined(WITH_JAVA_RUNTIME_LIBRARY_OPENJDK) || defined(WITH_JAVA_RUNTIME_LIBRARY_CLDC1_1)

	ByteArray backtrace(t.get_backtrace());

#else
# error unknown classpath configuration
#endif

	// Sanity check.

	assert(backtrace.is_non_null());

	/* We need a critical section here as we use the byte-array data
	   pointer directly. */

	LLNI_CRITICAL_START;
	
	stacktrace_t* st = (stacktrace_t*) backtrace.get_raw_data_ptr();

	stacktrace_print(st);

	LLNI_CRITICAL_END;
}


#if defined(ENABLE_CYCLES_STATS)
void stacktrace_print_cycles_stats(FILE *file)
{
	CYCLES_STATS_PRINT_OVERHEAD(stacktrace_overhead, file);
	CYCLES_STATS_PRINT(stacktrace_get,               file);
	CYCLES_STATS_PRINT(stacktrace_getClassContext ,  file);
	CYCLES_STATS_PRINT(stacktrace_getCurrentClass ,  file);
	CYCLES_STATS_PRINT(stacktrace_get_stack,         file);
}
#endif

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
