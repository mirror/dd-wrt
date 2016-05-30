/* src/vm/jit/executionstate.cpp - execution-state handling

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

#include "vm/jit/executionstate.hpp"
#include <stdint.h>                     // for uintptr_t, int32_t, uint8_t, etc
#include <string.h>                     // for memcmp
#include <cstdio>                       // for printf
#include "config.h"                     // for SIZEOF_VOID_P
#include "arch.hpp"
#include "md-abi.hpp"                   // for FLT_REG_CNT, INT_REG_CNT
#include "md.hpp"                       // for md_codegen_get_pv_from_pc, etc
#include "vm/descriptor.hpp"            // for methoddesc
#include "vm/exceptions.hpp"            // for exceptions_handle_exception
#include "vm/jit/abi.hpp"               // for nregdescfloat, nregdescint
#include "vm/jit/code.hpp"              // for codeinfo, etc
#include "vm/method.hpp"                // for method_print, methodinfo
#include "vm/os.hpp"                    // for os
#include "vm/types.hpp"                 // for s4, ptrint, u1, u8

/**
 * Restore callee-saved registers (including the RA register),
 * set the stack pointer to the next stackframe,
 * set the PC to the return address of the popped frame.
 *
 * *** This function imitates the effects of the method epilog ***
 * *** and returning from the method call.                     ***
 *
 * @param es Execution state to be modified.
 *        NOTE: es->code and es->pv are NOT updated.
 */
void executionstate_pop_stackframe(executionstate_t *es)
{
	int32_t reg;
	int32_t i;

	// Sanity checks.
	assert(es->code != NULL);

	// Calculate the size of the stackframe.
	int32_t framesize = md_stacktrace_get_framesize(es->code);

	// Read the return address.
	uint8_t* ra;
#if STACKFRAME_LEAFMETHODS_RA_REGISTER
	if (code_is_leafmethod(es->code))
		ra = es->ra;
	else
#endif
		ra = (u1*) md_stacktrace_get_returnaddress(es->sp, framesize);

	// Calculate the base of the stack frame.
	uintptr_t sp     = (uintptr_t) es->sp;
	uintptr_t basesp = sp + es->code->stackframesize * SIZE_OF_STACKSLOT;

	// Restore return address, if part of frame.
#if STACKFRAME_RA_TOP_OF_FRAME
# if STACKFRAME_LEAFMETHODS_RA_REGISTER
	if (!code_is_leafmethod(es->code)) {
# endif
		basesp -= 1 * SIZE_OF_STACKSLOT;
		es->ra = *((uint8_t**) basesp);
# if STACKFRAME_LEAFMETHODS_RA_REGISTER
	}
# endif
#endif /* STACKFRAME_RA_TOP_OF_FRAME */

	// Restore return address, if inside linkage area.
#if STACKFRAME_RA_LINKAGE_AREA
# if STACKFRAME_LEAFMETHODS_RA_REGISTER
	if (!code_is_leafmethod(es->code))
# endif
		es->ra = *((uint8_t**) (basesp + LA_LR_OFFSET));
#endif /* STACKFRAME_RA_LINKAGE_AREA */

	// Restore saved int registers.
	reg = INT_REG_CNT;
	for (i=0; i<es->code->savedintcount; ++i) {
		while (nregdescint[--reg] != REG_SAV)
			;
		basesp -= 1 * SIZE_OF_STACKSLOT;
		es->intregs[reg] = *((uintptr_t*) basesp);
	}

	// Restore saved flt registers.
	// XXX align?
	reg = FLT_REG_CNT;
	for (i=0; i<es->code->savedfltcount; ++i) {
		while (nregdescfloat[--reg] != REG_SAV)
			;
		basesp -= STACK_SLOTS_PER_FLOAT * SIZE_OF_STACKSLOT;
		es->fltregs[reg] = *((double*) basesp);
	}

	// Adjust the stackpointer.
	es->sp += framesize;
#if STACKFRMAE_RA_BETWEEN_FRAMES
	es->sp += SIZEOF_VOID_P; /* skip return address */
#endif

	// Set the program counter to the return address.
	es->pc = ra;

	// In debugging mode clobber non-saved registers.
#if !defined(NDEBUG)
	for (i=0; i<INT_REG_CNT; ++i)
		if (nregdescint[i] != REG_SAV)
			es->intregs[i] = (ptrint) 0x33dead3333dead33ULL;
	for (i=0; i<FLT_REG_CNT; ++i)
		if (nregdescfloat[i] != REG_SAV)
			*(u8*)&(es->fltregs[i]) = 0x33dead3333dead33ULL;
#endif /* !defined(NDEBUG) */
}

/**
 * Performs stack unwinding in case of an exception. This is done by
 * popping frames off the given execution state until a frame is reached
 * for which there is a handler. Execution will continue at the handler
 * site once the execution state is written back to the machine.
 *
 * @param es Execution state to be modified.
 * @param e The thrown exception object.
 *
 * This is specified in:
 *    The Java(TM) Virtual Machine Specification, Second Edition
 *    Section 3.6.5: Abrupt Method Invocation Completion
 */
void executionstate_unwind_exception(executionstate_t* es, java_handle_t* e)
{
	void* handler = NULL;

	// Iterate until we find an exception handler.
	while (handler == NULL) {

		// Search an exception handler in the current frame.
		handler = exceptions_handle_exception(e, es->pc, es->pv, es->sp);

		// Jump directly into the handler in case we found one.
		if (handler != NULL)
			break;

		// Find the codeinfo structure for the current frame.
		es->code = code_get_codeinfo_for_pv(es->pv);

		// Pop one frame off the stack.
		executionstate_pop_stackframe(es);

		// Get the PV for the parent Java method.
		es->pv = (uint8_t*) md_codegen_get_pv_from_pc(es->pc);

		// After popping the frame the PC points to the instruction just after
		// the invocation. To get the XPC we need to correct the PC to point
		// just before the invocation. But we do not know how big the
		// invocation site actually is, so we subtract one, which should be
		// sufficient for our purposes.
		es->pc -= 1;
	}

	// Update the execution state to continue at the handler site.
	es->pc = (uint8_t*) handler;
}


/* executionstate_sanity_check *************************************************

   Perform some sanity checks for the md_executionstate_read and
   md_executionstate_write functions.

*******************************************************************************/

#if !defined(NDEBUG)
void executionstate_sanity_check(void *context)
{
    /* estimate a minimum for the context size */

#define MINIMUM_CONTEXT_SIZE  (SIZEOF_VOID_P    * INT_REG_CNT \
		                       + sizeof(double) * FLT_REG_CNT)

	executionstate_t es1;
	executionstate_t es2;
	executionstate_t es3;
	unsigned int i;
	unsigned char reference[MINIMUM_CONTEXT_SIZE];

	/* keep a copy of (a prefix of) the context for reference */

	os::memcpy(&reference, context, MINIMUM_CONTEXT_SIZE);

	/* different poisons */

	os::memset(&es1, 0xc9, sizeof(executionstate_t));
	os::memset(&es2, 0xb5, sizeof(executionstate_t));
	os::memset(&es3, 0x6f, sizeof(executionstate_t));

	md_executionstate_read(&es1, context);

	/* verify that item-by-item copying preserves the state */

	es2.pc = es1.pc;
	es2.sp = es1.sp;
	es2.pv = es1.pv;
	es2.ra = es1.ra;
	es2.code = es1.code;
	for (i = 0; i < INT_REG_CNT; ++i)
		es2.intregs[i] = es1.intregs[i];
	for (i = 0; i < FLT_REG_CNT; ++i)
		es2.fltregs[i] = es1.fltregs[i];

	/* write it back - this should not change the context */
	/* We cannot check that completely, unfortunately, as we don't know */
	/* the size of the (OS-dependent) context. */

	md_executionstate_write(&es2, context);

	/* Read it again, Sam! */

	md_executionstate_read(&es3, context);

	/* Compare. Note: Because of the NAN madness, we cannot compare
	 * doubles using '=='. */

	assert(es3.pc == es1.pc);
	assert(es3.sp == es1.sp);
	assert(es3.pv == es1.pv);
	for (i = 0; i < INT_REG_CNT; ++i)
		assert(es3.intregs[i] == es1.intregs[i]);
	for (i = 0; i < FLT_REG_CNT; ++i)
		assert(memcmp(es3.fltregs+i, es1.fltregs+i, sizeof(double)) == 0);

	/* i386 and x86_64 do not have an RA register */

#if defined(__I386__) || defined(__X86_64__)
	assert(es3.ra != es1.ra);
#else
	assert(es3.ra == es1.ra);
#endif

	/* "code" is not set by the md_* functions */

	assert(es3.code != es1.code);

	/* assert that we have not messed up the context */

	assert(memcmp(&reference, context, MINIMUM_CONTEXT_SIZE) == 0);
}
#endif


/* executionstate_println ******************************************************

   Print execution state

   IN:
       es...............the execution state to print

*******************************************************************************/

#if !defined(NDEBUG)
void executionstate_println(executionstate_t *es)
{
	uint64_t *sp;
	int       slots;
	int       extraslots;
	int       i;

	if (!es) {
		printf("(executionstate_t *)NULL\n");
		return;
	}

	printf("executionstate_t:\n");
	printf("\tpc = %p", es->pc);
	printf("  sp = %p", es->sp);
	printf("  pv = %p", es->pv);
	printf("  ra = %p\n", es->ra);

#if defined(ENABLE_DISASSEMBLER)
	for (i=0; i<INT_REG_CNT; ++i) {
		if (i%4 == 0)
			printf("\t");
		else
			printf(" ");
# if SIZEOF_VOID_P == 8
		printf("%-3s = %016lx", abi_registers_integer_name[i], es->intregs[i]);
# else
		printf("%-3s = %08x", abi_registers_integer_name[i], (unsigned) es->intregs[i]);
# endif
		if (i%4 == 3)
			printf("\n");
	}

	for (i=0; i<FLT_REG_CNT; ++i) {
		if (i%4 == 0)
			printf("\t");
		else
			printf(" ");
		printf("F%02d = %016llx",i,(unsigned long long)es->fltregs[i]);
		if (i%4 == 3)
			printf("\n");
	}
#endif

	sp = (uint64_t *) es->sp;

	extraslots = 2;

	if (es->code) {
		methoddesc *md = es->code->m->parseddesc;
		slots = es->code->stackframesize;
		extraslots = 1 + md->memuse;
	}
	else
		slots = 0;


	if (slots) {
		printf("\tstack slots(+%d) at sp:", extraslots);
		for (i=0; i<slots+extraslots; ++i) {
			if (i%4 == 0)
				printf("\n\t\t");
			printf("M%02d%c", i, (i >= slots) ? '(' : ' ');
			printf("%016llx",(unsigned long long)*sp++);
			printf("%c", (i >= slots) ? ')' : ' ');
		}
		printf("\n");
	}

	printf("\tcode: %p", (void*)es->code);
	if (es->code != NULL) {
		printf(" stackframesize=%d ", es->code->stackframesize);
		method_print(es->code->m);
	}
	printf("\n");

	printf("\n");
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
