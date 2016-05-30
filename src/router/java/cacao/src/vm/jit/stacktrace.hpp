/* src/vm/jit/stacktrace.hpp - header file for stacktrace generation

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


#ifndef STACKTRACE_HPP_
#define STACKTRACE_HPP_ 1

#include <stdint.h>                     // for int32_t
#include "config.h"                     // for ENABLE_JAVASE, ENABLE_JIT, etc
#include "vm/global.hpp"                // for java_handle_objectarray_t, etc
#include "vm/loader.hpp"                // for classloader_t

#include "vm/types.hpp"
#include "md-abi.hpp"

struct classinfo;
struct codeinfo;
struct threadobject;

/* stackframeinfo **************************************************************

   ATTENTION: Keep the number of elements of this structure even, to
   make sure that the stack keeps aligned (e.g. 16-bytes for x86_64).

*******************************************************************************/

struct stackframeinfo_t {
	stackframeinfo_t *prev;             /* pointer to prev stackframeinfo     */
	codeinfo         *code;             /* codeinfo of current method         */
	void             *pv;               /* PV of current function             */
	void             *sp;               /* SP of parent Java function         */
	void             *ra;               /* RA to parent Java function         */
	void             *xpc;              /* XPC (for inline stubs)             */
#if defined(ENABLE_GC_CACAO)
	/* 
	 * The exact GC needs to be able to recover saved registers, so the
	 * native-stub saves these registers here
	 */
	uintptr_t         intregs[INT_SAV_CNT];
#endif
};


/* stacktrace_entry_t *********************************************************/

struct stacktrace_entry_t {
	codeinfo *code;                     /* codeinfo pointer of this method    */
	void     *pc;                       /* PC in this method                  */
};


/* stacktrace_t ***************************************************************/

struct stacktrace_t {
	int32_t            length;          /* length of the entries array        */
	stacktrace_entry_t entries[1];      /* stacktrace entries                 */
};


/* function prototypes ********************************************************/

void                       stacktrace_stackframeinfo_add(stackframeinfo_t* sfi, void* pv, void* sp, void* ra, void* xpc);
void                       stacktrace_stackframeinfo_remove(stackframeinfo_t *sfi);

java_handle_bytearray_t   *stacktrace_get(stackframeinfo_t *sfi);
java_handle_bytearray_t   *stacktrace_get_current(void);

#if defined(ENABLE_JAVASE)
java_handle_t*             stacktrace_get_StackTraceElement(stacktrace_t *st, int32_t index);
java_handle_objectarray_t* stacktrace_get_StackTraceElements(stacktrace_t *st);
classinfo                 *stacktrace_get_caller_class(int depth);
classloader_t             *stacktrace_first_nonnull_classloader(void);
classloader_t             *stacktrace_first_nonsystem_classloader(void);
java_handle_objectarray_t *stacktrace_getClassContext(void);
classinfo                 *stacktrace_get_current_class(void);
java_handle_objectarray_t *stacktrace_get_stack(void);
#endif

void                       stacktrace_print(stacktrace_t *st);
void                       stacktrace_print_current(void);

stacktrace_t*              stacktrace_get_of_thread(threadobject *t);
void                       stacktrace_print_of_thread(threadobject *t);

void                       stacktrace_print_exception(java_handle_t *h);

/* machine dependent functions (code in ARCH_DIR/md.c) */

#if defined(ENABLE_JIT)
# if defined(__SPARC_64__)
u1 *md_get_framepointer(u1 *sp);
u1 *md_get_pv_from_stackframe(u1 *sp);
# endif
#endif

#if defined(ENABLE_INTRP)
void* intrp_md_stacktrace_get_returnaddress(void* sp, int32_t framesize);
#endif

#if defined(ENABLE_CYCLES_STATS)
void stacktrace_print_cycles_stats(FILE *file);
#endif

#endif // STACKTRACE_HPP_


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
