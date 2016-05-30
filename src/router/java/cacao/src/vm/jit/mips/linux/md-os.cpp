/* src/vm/jit/mips/linux/md-os.cpp - machine dependent MIPS Linux functions

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
#include <sgidefs.h> /* required for _MIPS_SIM_ABI* defines (before signal.h) */
#include <signal.h>
#include <stdint.h>
#include <ucontext.h>

#include "vm/types.hpp"

#include "vm/jit/mips/codegen.hpp"
#include "vm/jit/mips/md.hpp"
#include "vm/jit/mips/md-abi.hpp"

#include "mm/gc.hpp"
#include "mm/memory.hpp"

#include "vm/signallocal.hpp"

#include "vm/jit/executionstate.hpp"
#include "vm/jit/trap.hpp"


/* md_init *********************************************************************

   Do some machine dependent initialization.

*******************************************************************************/

void md_init(void)
{
	/* The Boehm GC initialization blocks the SIGSEGV signal. So we do
	   a dummy allocation here to ensure that the GC is
	   initialized. */

#if defined(ENABLE_GC_BOEHM)
	(void) GCNEW(int);
#endif

#if 0
	/* Turn off flush-to-zero */

	{
		union fpc_csr n;
		n.fc_word = get_fpc_csr();
		n.fc_struct.flush = 0;
		set_fpc_csr(n.fc_word);
	}
#endif
}


/**
 * NullPointerException signal handler for hardware null pointer check.
 */
void md_signal_handler_sigsegv(int sig, siginfo_t *siginfo, void *_p)
{
	ucontext_t* _uc = (struct ucontext *) _p;
	mcontext_t* _mc = &_uc->uc_mcontext;
	void *xpc;

#if !defined(__UCLIBC__)
# if ((__GLIBC__ == 2) && (__GLIBC_MINOR__ < 5))
	/* NOTE: We only need this for pre glibc-2.5. */

	int disp = 0;
	int cause;

	/* get the cause of this exception */
	cause = _mc->cause;

	/* check the cause to find the faulting instruction */

	/* TODO: use defines for that stuff */

	switch (cause & 0x0000003c) {
	case 0x00000008:
		/* TLBL: XPC is ok */
		break;

	case 0x00000010:
		/* AdEL: XPC is of the following instruction */
		disp -= 4;
		break;
	}
	xpc = (void*) (_mc->pc + disp);
# else
	xpc = (void*) _mc->pc;
# endif
#else
	xpc = (void*) _gregs[CTX_EPC];
#endif

	// Handle the trap.
	trap_handle(TRAP_SIGSEGV, xpc, _p);
}


/**
 * Illegal Instruction signal handler for hardware exception checks.
 */
void md_signal_handler_sigill(int sig, siginfo_t* siginfo, void* _p)
{
	ucontext_t* _uc = (struct ucontext *) _p;
	mcontext_t* _mc = &_uc->uc_mcontext;

#if !defined(__UCLIBC__)
	void* xpc = (void*) _mc->pc;
#else
	void* xpc = (void*) _gregs[CTX_EPC];
#endif

	// Handle the trap.
	trap_handle(TRAP_SIGILL, xpc, _p);
}


/* md_signal_handler_sigusr2 ***************************************************

   DOCUMENT ME

*******************************************************************************/

void md_signal_handler_sigusr2(int sig, siginfo_t *siginfo, void *_p)
{
}


/**
 * Read the given context into an executionstate.
 *
 * @param es      execution state
 * @param context machine context
 */
void md_executionstate_read(executionstate_t* es, void* context)
{
	ucontext_t* _uc;
	mcontext_t* _mc;
	greg_t*     _gregs;
	int         i;

	_uc = (ucontext_t*) context;
	_mc = &_uc->uc_mcontext;

#if defined(__UCLIBC__)
	_gregs = _mc->gpregs;
#else	
	_gregs = _mc->gregs;
#endif

	/* Read special registers. */

	/* In glibc's ucontext.h the registers are defined as long long,
	   even for MIPS32, so we cast them.  This is not the case for
	   uClibc. */

#if defined(__UCLIBC__)
	es->pc = _gregs[CTX_EPC];
#else
	es->pc = (uint8_t*) (uintptr_t) _mc->pc;
#endif

	es->sp = (uint8_t*) (uintptr_t) _gregs[REG_SP];
	es->pv = (uint8_t*) (uintptr_t) _gregs[REG_PV];
	es->ra = (uint8_t*) (uintptr_t) _gregs[REG_RA];

	/* Read integer registers. */

	for (i = 0; i < INT_REG_CNT; i++)
		es->intregs[i] = _gregs[i];

	/* Read float registers. */

	/* Do not use the assignment operator '=', as the type of the
	   _mc->fpregs[i] can cause invalid conversions. */

	assert(sizeof(_mc->fpregs.fp_r) == sizeof(es->fltregs));
	os::memcpy(&es->fltregs, &_mc->fpregs.fp_r, sizeof(_mc->fpregs.fp_r));
}


/**
 * Write the given executionstate back to the context.
 *
 * @param es      execution state
 * @param context machine context
 */
void md_executionstate_write(executionstate_t* es, void* context)
{
	ucontext_t* _uc;
	mcontext_t* _mc;
	greg_t*     _gregs;
	int         i;

	_uc = (ucontext_t *) context;
	_mc = &_uc->uc_mcontext;

#if defined(__UCLIBC__)
	_gregs = _mc->gpregs;
#else	
	_gregs = _mc->gregs;
#endif

	/* Write integer registers. */

	for (i = 0; i < INT_REG_CNT; i++)
		_gregs[i] = es->intregs[i];

	/* Write float registers. */

	/* Do not use the assignment operator '=', as the type of the
	   _mc->fpregs[i] can cause invalid conversions. */

	assert(sizeof(_mc->fpregs.fp_r) == sizeof(es->fltregs));
	os::memcpy(&_mc->fpregs.fp_r, &es->fltregs, sizeof(_mc->fpregs.fp_r));

	/* Write special registers. */

#if defined(__UCLIBC__)
	_gregs[CTX_EPC] = es->pc;
#else
	_mc->pc         = (uintptr_t) es->pc;
#endif

	_gregs[REG_SP]  = (uintptr_t) es->sp;
	_gregs[REG_PV]  = (uintptr_t) es->pv;
	_gregs[REG_RA]  = (uintptr_t) es->ra;
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
