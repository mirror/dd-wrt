/* src/vm/jit/patcher-common.cpp - architecture independent code patching stuff

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
#include <inttypes.h>

#include <algorithm>
#include <functional>

#include "codegen.hpp"                   /* for PATCHER_NOPS */
#include "md.hpp"
#include "trap.hpp"

#include "native/native.hpp"

#include "toolbox/list.hpp"
#include "toolbox/logging.hpp"           /* XXX remove me! */

#include "vm/breakpoint.hpp"
#include "vm/exceptions.hpp"
#include "vm/hook.hpp"
#include "vm/initialize.hpp"
#include "vm/options.hpp"
#include "vm/os.hpp"
#include "vm/resolve.hpp"
#include "vm/vm.hpp"

#include "vm/jit/code.hpp"
#include "vm/jit/codegen-common.hpp"
#include "vm/jit/disass.hpp"
#include "vm/jit/jit.hpp"
#include "vm/jit/patcher-common.hpp"

STAT_DECLARE_VAR(int,size_patchref,0)

/* patcher_function_list *******************************************************

   This is a list which maps patcher function pointers to the according
   names of the patcher functions. It is only usefull for debugging
   purposes.

*******************************************************************************/

#if !defined(NDEBUG)
typedef struct patcher_function_list_t {
	functionptr patcher;
	const char* name;
} patcher_function_list_t;

static patcher_function_list_t patcher_function_list[] = {
	{ PATCHER_initialize_class,              "initialize_class" },
#ifdef ENABLE_VERIFIER
	{ PATCHER_resolve_class,                 "resolve_class" },
#endif /* ENABLE_VERIFIER */
	{ PATCHER_resolve_native_function,       "resolve_native_function" },
	{ PATCHER_invokestatic_special,          "invokestatic_special" },
	{ PATCHER_invokevirtual,                 "invokevirtual" },
	{ PATCHER_invokeinterface,               "invokeinterface" },
	{ PATCHER_breakpoint,                    "breakpoint" },
	{ NULL,                                  "-UNKNOWN PATCHER FUNCTION-" }
};
#endif


/* patcher_list_create *********************************************************

   Creates an empty patcher list for the given codeinfo.

*******************************************************************************/

void patcher_list_create(codeinfo *code)
{
	code->patchers = new LockedList<patchref_t>();
}


/* patcher_list_reset **********************************************************

   Resets the patcher list inside a codeinfo. This is usefull when
   resetting a codeinfo for recompiling.

*******************************************************************************/

void patcher_list_reset(codeinfo *code)
{
	STATISTICS(size_patchref -= sizeof(patchref_t) * code->patchers->size());

	// Free all elements of the list.
	code->patchers->clear();
}

/* patcher_list_free ***********************************************************

   Frees the patcher list and all its entries for the given codeinfo.

*******************************************************************************/

void patcher_list_free(codeinfo *code)
{
	// Free all elements of the list.
	patcher_list_reset(code);

	// Free the list itself.
	delete code->patchers;
}


/**
 * Find an entry inside the patcher list for the given codeinfo by
 * specifying the program counter of the patcher position.
 *
 * NOTE: Caller should hold the patcher list lock or maintain
 * exclusive access otherwise.
 *
 * @param pc Program counter to find.
 *
 * @return Pointer to patcher.
 */

struct foo : public std::binary_function<patchref_t, void*, bool> {
	bool operator() (const patchref_t& pr, const void* pc) const
	{
		return (pr.mpc == (uintptr_t) pc);
	}
};

static patchref_t* patcher_list_find(codeinfo* code, void* pc)
{
	// Search for a patcher with the given PC.
	List<patchref_t>::iterator it = std::find_if(code->patchers->begin(), code->patchers->end(), std::bind2nd(foo(), pc));

	if (it == code->patchers->end())
		return NULL;

	return &(*it);
}


/**
 * Show the content of the whole patcher reference list for
 * debugging purposes.
 *
 * @param code The codeinfo containing the patcher list.
 */
#if !defined(NDEBUG)
void patcher_list_show(codeinfo *code)
{
	for (List<patchref_t>::iterator it = code->patchers->begin(); it != code->patchers->end(); it++) {
		patchref_t& pr = *it;

		// Lookup name in patcher function list.
		patcher_function_list_t* l;
		for (l = patcher_function_list; l->patcher != NULL; l++)
			if (l->patcher == pr.patcher)
				break;

		// Display information about patcher.
		printf("\tpatcher");
		printf(" pc:0x%016"PRIxPTR,    pr.mpc);
		printf(" datap:0x%016"PRIxPTR, pr.datap);
		printf(" ref:0x%016"PRIxPTR,   (uintptr_t) pr.ref);
#if PATCHER_CALL_SIZE == 4
		printf(" mcode:%08"PRIx32, (uint32_t) pr.mcode);
#elif PATCHER_CALL_SIZE == 2
		printf(" mcode:%04"PRIx16, (uint16_t) pr.mcode);
#else
# error Unknown PATCHER_CALL_SIZE
#endif
		printf(" type:%s\n", l->name);

		// Display machine code of patched position.
#if 0 && defined(ENABLE_DISASSEMBLER)
		printf("\t\tcurrent -> ");
		disassinstr((uint8_t*) pr.mpc);
		printf("\t\tapplied -> ");
		disassinstr((uint8_t*) &(pr.mcode));
#endif
	}
}
#endif


/* patcher_add_patch_ref *******************************************************

   Appends a new patcher reference to the list of patching positions.

   Returns a pointer to the newly created patchref_t.

*******************************************************************************/

patchref_t *patcher_add_patch_ref(jitdata *jd, functionptr patcher, void* ref, s4 disp)
{
	codegendata *cd   = jd->cd;
	codeinfo    *code = jd->code;

#if defined(ALIGN_PATCHER_TRAP)
	emit_patcher_alignment(cd);
#endif

	int32_t patchmpc = cd->mcodeptr - cd->mcodebase;

#if !defined(NDEBUG)
	if (patcher_list_find(code, (void*) (intptr_t) patchmpc) != NULL)
		os::abort("patcher_add_patch_ref: different patchers at same position.");
#endif

#if defined(USES_PATCHABLE_MEMORY_BARRIER)
	PATCHER_NOPS;
#endif

	// Set patcher information (mpc is resolved later).
	patchref_t pr;

	pr.mpc         = patchmpc;
	pr.datap       = 0;
	pr.disp        = disp;
	pr.disp_mb     = 0;
	pr.patch_align = 0;
	pr.patcher     = patcher;
	pr.ref         = ref;
	pr.mcode       = 0;
	pr.done        = false;

	// Store patcher in the list (NOTE: structure is copied).
	code->patchers->push_back(pr);

	STATISTICS(size_patchref += sizeof(patchref_t));

#if defined(ENABLE_JIT) && (defined(__I386__) || defined(__SPARC_64__) || defined(__X86_64__))

	/* XXX We can remove that when we don't use UD2 anymore on i386
	   and x86_64. */

	/* On some architectures the patcher stub call instruction might
	   be longer than the actual instruction generated.  On this
	   architectures we store the last patcher call position and after
	   the basic block code generation is completed, we check the
	   range and maybe generate some nop's. */
	/* The nops are generated in codegen_emit in each codegen */

	cd->lastmcodeptr = cd->mcodeptr + PATCHER_CALL_SIZE;
#endif

	return &code->patchers->back();
}


/**
 * Resolve all patchers in the current JIT run.
 *
 * @param jd JIT data-structure
 */
void patcher_resolve(jitdata* jd)
{
	// Get required compiler data.
	codeinfo* code = jd->code;

	for (List<patchref_t>::iterator it = code->patchers->begin(); it != code->patchers->end(); it++) {
		patchref_t& pr = *it;

		pr.mpc   += (intptr_t) code->entrypoint;
		pr.datap  = (intptr_t) (pr.disp + code->entrypoint);
	}
}


/**
 * Check if the patcher is already patched.  This is done by comparing
 * the machine instruction.
 *
 * @param pr Patcher structure.
 *
 * @return true if patched, false otherwise.
 */
bool patcher_is_patched(patchref_t* pr)
{
	// Validate the instruction at the patching position is the same
	// instruction as the patcher structure contains.
	uint32_t mcode = *((uint32_t*) pr->mpc);

#if PATCHER_CALL_SIZE == 4
	if (mcode != pr->mcode) {
#elif PATCHER_CALL_SIZE == 2
	if ((uint16_t) mcode != (uint16_t) pr->mcode) {
#else
#error Unknown PATCHER_CALL_SIZE
#endif
		// The code differs.
		return false;
	}

	return true;
}


/**
 *
 */
bool patcher_is_patched_at(void* pc)
{
	codeinfo* code = code_find_codeinfo_for_pc(pc);

	// Get the patcher for the given PC.
	patchref_t* pr = patcher_list_find(code, pc);

	if (pr == NULL) {
		// The given PC is not a patcher position.
		return false;
	}

	// Validate the instruction.
	return patcher_is_patched(pr);
}


/* patcher_handler *************************************************************

   Handles the request to patch JIT code at the given patching
   position. This function is normally called by the signal
   handler.

   NOTE: The patcher list lock is used to maintain exclusive
   access of the patched position (in fact of the whole code).
   After patching has suceeded, the patcher reference should be
   removed from the patcher list to avoid double patching.

*******************************************************************************/

#if !defined(NDEBUG)
/* XXX this indent is not thread safe! */
/* XXX if you want it thread safe, place patcher_depth in threadobject! */
static int patcher_depth = 0;
#define TRACE_PATCHER_INDENT for (i=0; i<patcher_depth; i++) printf("\t")
#endif /* !defined(NDEBUG) */

bool patcher_handler(u1 *pc)
{
	codeinfo      *code;
	patchref_t    *pr;
	bool           result;
#if !defined(NDEBUG)
	patcher_function_list_t *l;
	int                      i;
#endif

	/* define the patcher function */

	bool (*patcher_function)(patchref_t *);

	/* search the codeinfo for the given PC */

	code = code_find_codeinfo_for_pc(pc);
	assert(code);

	// Enter a mutex on the patcher list.
	code->patchers->lock();

	/* search the patcher information for the given PC */

	pr = patcher_list_find(code, pc);

	if (pr == NULL)
		os::abort("patcher_handler: Unable to find patcher reference.");

	if (pr->done) {
#if !defined(NDEBUG)
		if (opt_DebugPatcher) {
			log_println("patcher_handler: double-patching detected!");
		}
#endif
		code->patchers->unlock();
		return true;
	}

#if !defined(NDEBUG)
	if (opt_DebugPatcher) {
		for (l = patcher_function_list; l->patcher != NULL; l++)
			if (l->patcher == pr->patcher)
				break;

		TRACE_PATCHER_INDENT; printf("patching in "); method_print(code->m); printf(" at %p\n", (void *) pr->mpc);
		TRACE_PATCHER_INDENT; printf("\tpatcher function = %s <%p>\n", l->name, (void *) (intptr_t) pr->patcher);

		TRACE_PATCHER_INDENT;
		printf("\tmachine code before = ");

# if defined(ENABLE_DISASSEMBLER)
		disassinstr((u1*) (void*) pr->mpc);
# else
		printf("%x at %p (disassembler disabled)\n", *((uint32_t*) pr->mpc), (void*) pr->mpc);
# endif

		patcher_depth++;
		assert(patcher_depth > 0);
	}
#endif

	/* cast the passed function to a patcher function */

	patcher_function = (bool (*)(patchref_t *)) (ptrint) pr->patcher;

	/* call the proper patcher function */

	result = (patcher_function)(pr);

#if !defined(NDEBUG)
	if (opt_DebugPatcher) {
		assert(patcher_depth > 0);
		patcher_depth--;

		TRACE_PATCHER_INDENT;
		printf("\tmachine code after  = ");

# if defined(ENABLE_DISASSEMBLER)
		disassinstr((u1*) (void*) pr->mpc);
# else
		printf("%x at %p (disassembler disabled)\n", *((uint32_t*) pr->mpc), (void*) pr->mpc);
# endif

		if (result == false) {
			TRACE_PATCHER_INDENT; printf("\tPATCHER EXCEPTION!\n");
		}
	}
#endif

	// Check return value and mangle the pending exception.
	if (result == false)
		resolve_handle_pending_exception(true);

	// XXX This is only preliminary to prevent double-patching.
	else
		pr->done = true;

	code->patchers->unlock();

	return result;
}


/* patcher_initialize_class ****************************************************

   Initalizes a given classinfo pointer.
   This function does not patch any data.

*******************************************************************************/

bool patcher_initialize_class(patchref_t *pr)
{
	classinfo *c;

	/* get stuff from the patcher reference */

	c = (classinfo *) pr->ref;

	/* check if the class is initialized */

	if (!(c->state & CLASS_INITIALIZED))
		if (!initialize_class(c))
			return false;

	/* patch back original code */

	patcher_patch_code(pr);

	return true;
}


/* patcher_resolve_class *******************************************************

   Resolves a given unresolved class reference.
   This function does not patch any data.

*******************************************************************************/

#ifdef ENABLE_VERIFIER
bool patcher_resolve_class(patchref_t *pr)
{
	unresolved_class *uc;

	/* get stuff from the patcher reference */

	uc = (unresolved_class *) pr->ref;

	/* resolve the class and check subtype constraints */

	if (!resolve_class_eager_no_access_check(uc))
		return false;

	/* patch back original code */

	patcher_patch_code(pr);

	return true;
}
#endif /* ENABLE_VERIFIER */


/* patcher_resolve_native_function *********************************************

   Resolves the native function for a given methodinfo.
   This function patches one data segment word.

*******************************************************************************/

bool patcher_resolve_native_function(patchref_t *pr)
{
	methodinfo  *m;
	uint8_t     *datap;

	/* get stuff from the patcher reference */

	m     = (methodinfo *) pr->ref;
	datap = (uint8_t *)    pr->datap;

	/* resolve native function */

	NativeMethods& nm = VM::get_current()->get_nativemethods();
	void* f = nm.resolve_method(m);

	if (f == NULL)
		return false;

	/* patch native function pointer */

	*((intptr_t*) datap) = (intptr_t) f;

	/* synchronize data cache */

	md_dcacheflush(datap, SIZEOF_VOID_P);

	/* patch back original code */

	patcher_patch_code(pr);

	return true;
}


/**
 * Deals with breakpoint instructions (ICMD_BREAKPOINT) compiled
 * into a JIT method. This patcher might never patch back the
 * original machine code because breakpoints are kept active.
 */
bool patcher_breakpoint(patchref_t *pr)
{
	// Get stuff from the patcher reference.
	Breakpoint* breakp = (Breakpoint*) pr->ref;

	// Hook point when a breakpoint was triggered.
	Hook::breakpoint(breakp);

	// In case the breakpoint wants to be kept active, we simply
	// fail to "patch" at this point.
	if (!breakp->is_oneshot)
		return false;

	// Patch back original code.
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

