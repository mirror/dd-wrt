/* src/vm/jit/s390/md.c - machine dependent s390 Linux functions

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


#define _GNU_SOURCE

#include "config.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <ucontext.h>

#include "vm/jit/s390/md-abi.h"

#include "threads/thread.hpp"

#include "vm/exceptions.hpp"
#include "vm/signallocal.hpp"

#include "vm/jit/abi.hpp"
#include "vm/jit/executionstate.hpp"
#include "vm/jit/methodheader.hpp"
#include "vm/jit/methodtree.hpp"
#include "vm/jit/stacktrace.hpp"
#include "vm/jit/trap.hpp"

#if !defined(NDEBUG) && defined(ENABLE_DISASSEMBLER)
#include "vm/options.hpp" /* XXX debug */
#endif

#include "vm/jit/codegen-common.hpp"
#include "vm/jit/s390/codegen.h"
#include "vm/jit/s390/md.h"


/* prototypes *****************************************************************/

u1 *exceptions_handle_exception(java_object_t *xptro, u1 *xpc, u1 *pv, u1 *sp);

void md_signal_handler_sigill(int sig, siginfo_t *siginfo, void *_p);

void md_dump_context(u1 *pc, mcontext_t *mc);

/* md_init *********************************************************************

   Do some machine dependent initialization.

*******************************************************************************/

void md_init(void)
{
}

/* md_dump_context ************************************************************
 
   Logs the machine context
  
*******************************************************************************/

void md_dump_context(u1 *pc, mcontext_t *mc) {
	int i;
	u1 *pv;
	methodinfo *m;

	union {
		u8 l;
		fpreg_t fr;
	} freg;

	log_println("Dumping context.");

	log_println("Program counter: 0x%08X", pc);

	pv = methodtree_find_nocheck(pc);

	if (pv == NULL) {
		log_println("No java method found at location.");
	} else {
		m = (*(codeinfo **)(pv + CodeinfoPointer))->m;
		log_println(
			"Java method: class %s, method %s, descriptor %s.",
			UTF_TEXT(m->clazz->name), UTF_TEXT(m->name), UTF_TEXT(m->descriptor)
		);
	}

#if defined(ENABLE_DISASSEMBLER)
	log_println("Printing instruction at program counter:");
	disassinstr(pc);
#endif

	log_println("General purpose registers:");

	for (i = 0; i < 16; i++) {
		log_println("\tr%d:\t0x%08X\t%d", i, mc->gregs[i], mc->gregs[i]);
	}

	log_println("Floating point registers:");

	for (i = 0; i < 16; i++) {
		freg.fr.d = mc->fpregs.fprs[i].d;
		log_println("\tf%d\t0x%016llX\t(double)%e\t(float)%f", i, freg.l, freg.fr.d, freg.fr.f);
	}

	log_println("Dumping the current stacktrace:");
	stacktrace_print_current();
}

/**
 * NullPointerException signal handler for hardware null pointer check.
 */
void md_signal_handler_sigsegv(int sig, siginfo_t *siginfo, void *_p)
{
	ucontext_t* _uc = (ucontext_t *) _p;
	mcontext_t* _mc = &_uc->uc_mcontext;

	void* xpc = (u1 *) _mc->psw.addr;

	// Handle the trap.
	trap_handle(TRAP_SIGSEGV, xpc, _p);
}

/**
  * Illegal Instruction signal handler for hardware exception checks.
  */
void md_signal_handler_sigill(int sig, siginfo_t *siginfo, void *_p)
{
	ucontext_t* _uc = (ucontext_t *) _p;
	mcontext_t* _mc = &_uc->uc_mcontext;

	void* xpc = siginfo->si_addr;

	// Handle the trap.
	trap_handle(TRAP_SIGILL, xpc, _p);
}

/* md_signal_handler_sigfpe ****************************************************

   ArithmeticException signal handler for hardware divide by zero
   check.

*******************************************************************************/

void md_signal_handler_sigfpe(int sig, siginfo_t *siginfo, void *_p)
{
	ucontext_t* _uc = (ucontext_t *) _p;
	mcontext_t* _mc = &_uc->uc_mcontext;

	void* xpc = siginfo->si_addr;

	if (N_RR_GET_OPC(xpc) == OPC_DR) { /* DR */

		int r1 = N_RR_GET_REG1(xpc);
		int r2 = N_RR_GET_REG2(xpc);

		if (
			(_mc->gregs[r1] == 0xFFFFFFFF) &&
			(_mc->gregs[r1 + 1] == 0x80000000) && 
			(_mc->gregs[r2] == 0xFFFFFFFF)
		) {
			/* handle special case 0x80000000 / 0xFFFFFFFF that fails on hardware */
			/* next instruction */
			u1 *pc = (u1 *)_mc->psw.addr;
			/* remainder */
			_mc->gregs[r1] = 0;
			/* quotient */
			_mc->gregs[r1 + 1] = 0x80000000;
			/* continue at next instruction */
			_mc->psw.addr = (ptrint) pc;

			return;
		}
	}

	// Handle the trap.
	trap_handle(TRAP_SIGFPE, xpc, _p);
}


/* md_signal_handler_sigusr2 ***************************************************

   Signal handler for profiling sampling.

*******************************************************************************/

#if defined(ENABLE_THREADS)
void md_signal_handler_sigusr2(int sig, siginfo_t *siginfo, void *_p)
{
	threadobject *t;
	ucontext_t   *_uc;
	mcontext_t   *_mc;
	u1           *pc;

	t = THREADOBJECT;

	_uc = (ucontext_t *) _p;
	_mc = &_uc->uc_mcontext;

	/* ATTENTION: Don't use CACAO's internal REG_* defines as they are
	   different to the ones in <ucontext.h>. */

	pc = (u1 *) _mc->psw.addr;

	t->pc = pc;
}
#endif


/**
 * Read the given context into an executionstate.
 *
 * @param es      execution state
 * @param context machine context
 */
void md_executionstate_read(executionstate_t* es, void* context)
{
	ucontext_t *_uc;
	mcontext_t *_mc;
	int         i;

	_uc = (ucontext_t *) context;
	_mc = &_uc->uc_mcontext;

	/* read special registers */
	es->pc = (u1 *) _mc->psw.addr;
	es->sp = (u1 *) _mc->gregs[REG_SP];
	es->pv = (u1 *) _mc->gregs[REG_PV] - N_PV_OFFSET;
	es->ra = (u1 *) _mc->gregs[REG_RA];

	/* read integer registers */
	for (i = 0; i < INT_REG_CNT; i++)
		es->intregs[i] = _mc->gregs[i];

	/* read float registers */
	/* Do not use the assignment operator '=', as the type of
	 * the _mc->sc_fpregs[i] can cause invalid conversions. */

	assert(sizeof(_mc->fpregs.fprs) == sizeof(es->fltregs));
	os::memcpy(&es->fltregs, &_mc->fpregs.fprs, sizeof(_mc->fpregs.fprs));
}


/**
 * Write the given executionstate back to the context.
 *
 * @param es      execution state
 * @param context machine context
 */
void md_executionstate_write(executionstate_t* es, void* context)
{
	ucontext_t *_uc;
	mcontext_t *_mc;
	int         i;

	_uc = (ucontext_t *) context;
	_mc = &_uc->uc_mcontext;

	/* write integer registers */
	for (i = 0; i < INT_REG_CNT; i++)
		_mc->gregs[i] = es->intregs[i];

	/* write float registers */
	/* Do not use the assignment operator '=', as the type of
	 * the _mc->sc_fpregs[i] can cause invalid conversions. */

	assert(sizeof(_mc->fpregs.fprs) == sizeof(es->fltregs));
	os::memcpy(&_mc->fpregs.fprs, &es->fltregs, sizeof(_mc->fpregs.fprs));

	/* write special registers */
	_mc->psw.addr      = (ptrint) es->pc;
	_mc->gregs[REG_SP] = (ptrint) es->sp;
	_mc->gregs[REG_PV] = (ptrint) es->pv + N_PV_OFFSET;
	_mc->gregs[REG_RA] = (ptrint) es->ra;
}


/* md_jit_method_patch_address *************************************************

   Gets the patch address of the currently compiled method. The offset
   is extracted from the load instruction(s) before the jump and added
   to the right base address (PV or REG_METHODPTR).

   INVOKESTATIC/SPECIAL:

0x7748d7b2:   a7 18 ff d4                      lhi      %r1,-44  
(load dseg offset)
0x7748d7b6:   58 d1 d0 00                      l        %r13,0(%r1,%r13)
(load pv)
0x7748d7ba:   0d ed                            basr     %r14,%r13
(jump to pv)

   INVOKEVIRTUAL:

0x7748d82a:   58 c0 20 00                      l        %r12,0(%r2)
(load mptr)
0x7748d82e:   58 d0 c0 00                      l        %r13,0(%r12)
(load pv from mptr)
0x7748d832:   0d ed                            basr     %r14,%r13
(jump to pv)


   INVOKEINTERFACE:

last 2 instructions the same as in invokevirtual

*******************************************************************************/

void *md_jit_method_patch_address(void* pv, void *ra, void *mptr)
{
	uint8_t *pc;
	uint8_t  base, index;
	int32_t  offset;
	void    *pa;                        /* patch address                      */

	/* go back to the load before the call instruction */

	pc = ((uint8_t *) ra) - SZ_BCR - SZ_L;

	/* get the base register of the load */

	base  = N_RX_GET_BASE(pc);
	index = N_RX_GET_INDEX(pc);

	/* check for the different calls */

	switch (base) {
		case REG_PV:
			/* INVOKESTATIC/SPECIAL */

			switch (index) {
				case R0:
					/* the offset is in the load instruction */
					offset = N_RX_GET_DISP(pc) + N_PV_OFFSET;
					break;
				case REG_ITMP1:
					/* the offset is in the immediate load before the load */
					offset = N_RI_GET_IMM(pc - SZ_L);
					break;
				default:
					assert(0);
			}

			/* add the offset to the procedure vector */

			pa = ((uint8_t *) pv) + offset;
			break;

		case REG_METHODPTR:
			/* mptr relative */
			/* INVOKEVIRTUAL/INTERFACE */

			offset = N_RX_GET_DISP(pc);

			/* return NULL if no mptr was specified (used for replacement) */

			if (mptr == NULL)
				return NULL;

			/* add offset to method pointer */
			
			pa = (uint8_t *)mptr + offset;
			break;

		default:
			/* catch any problems */
			vm_abort("md_jit_method_patch_address");
			break;
	}

	return pa;
}


/**
 * Decode the trap instruction at the given PC.
 *
 * @param trp information about trap to be filled
 * @param sig signal number
 * @param xpc exception PC
 * @param es execution state of the machine
 * @return true if trap was decoded successfully, false otherwise.
 */
bool md_trap_decode(trapinfo_t* trp, int sig, void* xpc, executionstate_t* es)
{
	switch (sig) {
	case TRAP_SIGILL:
		if (N_RR_GET_OPC(xpc) == OPC_ILL) {
			int32_t reg = N_ILL_GET_REG(xpc);
			trp->type  = N_ILL_GET_TYPE(xpc);
			trp->value = es->intregs[reg];
			return true;
		}
		return false;

	case TRAP_SIGSEGV:
	{
		int is_null;
		int32_t base;
		switch (N_RX_GET_OPC(xpc)) {
			case OPC_L:
			case OPC_ST:
			case OPC_CL: /* array size check on NULL array */
				base = N_RX_GET_BASE(xpc);
				if (base == 0) {
					is_null = 1;
				} else if (es->intregs[base] == 0) {
					is_null = 1;
				} else {
					is_null = 0;
				}
				break;
			default:
				is_null = 0;
				break;
		}

		// Check for implicit NullPointerException.
		if (is_null) {
			trp->type  = TRAP_NullPointerException;
			trp->value = 0;
			return true;
		}

		return false;
	}

	case TRAP_SIGFPE:
	{
		if (N_RR_GET_OPC(xpc) == OPC_DR) {
			int r2 = N_RR_GET_REG2(xpc);
			if (es->intregs[r2] == 0) {
				trp->type = TRAP_ArithmeticException;
				trp->value = 0;
				return true;
			}
		}

		return false;
	}

	default:
		return false;
	}
}



/* md_patch_replacement_point **************************************************

   Patch the given replacement point.

*******************************************************************************/
#if defined(ENABLE_REPLACEMENT)
void md_patch_replacement_point(u1 *pc, u1 *savedmcode, bool revert)
{
	assert(0);
}
#endif

void md_handle_exception(int32_t *regs, int64_t *fregs, int32_t *out) {

	uint8_t *xptr;
	uint8_t *xpc;
	uint8_t *sp;
	uint8_t *pv;
	uint8_t *ra;
	uint8_t *handler;
	int32_t framesize;
	int32_t intsave;
	int32_t fltsave;
	int64_t *savearea;
	int i;
	int reg;
	int loops = 0;

	/* get registers */

	xptr = *(uint8_t **)(regs + REG_ITMP1_XPTR);
	xpc = *(uint8_t **)(regs + REG_ITMP2_XPC);
	sp = *(uint8_t **)(regs + REG_SP);


	/* initialize number of calle saved int regs to restore to 0 */
	out[0] = 0;

	/* initialize number of calle saved flt regs to restore to 0 */
	out[1] = 0;

	do {

		++loops;

		pv = methodtree_find(xpc);

		handler = exceptions_handle_exception((java_object_t *)xptr, xpc, pv, sp);

		if (handler == NULL) {

			/* exception was not handled
			 * get values of calee saved registers and remove stack frame 
			 */

			/* read stuff from data segment */

			framesize = *(int32_t *)(pv + FrameSize);

			intsave = *(int32_t *)(pv + IntSave);
			if (intsave > out[0]) {
				out[0] = intsave;
			}

			fltsave = *(int32_t *)(pv + FltSave);
			if (fltsave > out[1]) {
				out[1] = fltsave;
			}

			/* pointer to register save area */

			savearea = (int64_t *)(sp + framesize - 8);

			/* return address */

			ra = *(uint8_t **)(sp + framesize - 8);

			/* restore saved registers */

			for (i = 0; i < intsave; ++i) {
				--savearea;
				reg = abi_registers_integer_saved[INT_SAV_CNT - 1 - i];
				regs[reg] = *(int32_t *)(savearea);
			}

			for (i = 0; i < fltsave; ++i) {
				--savearea;
				reg = abi_registers_float_saved[FLT_SAV_CNT - 1 - i];
				fregs[reg] = *savearea;
			}

			/* remove stack frame */

			sp += framesize;

			/* new xpc is call before return address */

			xpc = ra - 2;

		} else {
			xpc = handler;
		}
	} while (handler == NULL);

	/* write new values for registers */

	*(uint8_t **)(regs + REG_ITMP1_XPTR) = xptr;
	*(uint8_t **)(regs + REG_ITMP2_XPC) = xpc;
	*(uint8_t **)(regs + REG_SP) = sp;
	*(uint8_t **)(regs + REG_PV) = pv - 0XFFC;

	/* maybe leaf flag */

	out[2] = (loops == 1);
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
