/* src/vm/jit/alpha/linux/md-os.cpp - machine dependent Alpha Linux functions

   Copyright (C) 1996-2013
   CACAOVM - Verein zur Foerderung der freien virtuellen Maschine CACAO
   Copyright (C) 2008, 2009 Theobroma Systems Ltd.

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
#include <ucontext.h>

#include "vm/types.hpp"

#include "vm/jit/alpha/codegen.hpp"
#include "vm/jit/alpha/md.hpp"
#include "vm/jit/alpha/md-abi.hpp"

#include "threads/thread.hpp"

#include "vm/signallocal.hpp"

#include "vm/jit/executionstate.hpp"
#include "vm/jit/trap.hpp"


/**
 * NullPointerException signal handler for hardware null pointer check.
 */
void md_signal_handler_sigsegv(int sig, siginfo_t *siginfo, void *_p)
{
	ucontext_t* _uc = (ucontext_t *) _p;
	mcontext_t* _mc = &_uc->uc_mcontext;

	void* xpc = (void*) _mc->sc_pc;

	// Handle the trap.
	trap_handle(TRAP_SIGSEGV, xpc, _p);
}


/**
 * Illegal Instruction signal handler for hardware exception checks.
 */
void md_signal_handler_sigill(int sig, siginfo_t *siginfo, void *_p)
{
	ucontext_t* _uc = (ucontext_t*) _p;
	mcontext_t* _mc = &_uc->uc_mcontext;

	void* xpc = (void*) _mc->sc_pc;

	// The PC points to the instruction after the illegal instruction.
	xpc = (void*) (((uintptr_t) xpc) - 4);

	// Handle the trap.
	trap_handle(TRAP_SIGILL, xpc, _p);
}


/* md_signal_handler_sigusr2 ***************************************************

   Signal handler for profiling sampling.

*******************************************************************************/

void md_signal_handler_sigusr2(int sig, siginfo_t *siginfo, void *_p)
{
	threadobject *tobj;
	ucontext_t   *_uc;
	mcontext_t   *_mc;
	u1           *pc;

	tobj = THREADOBJECT;

	_uc = (ucontext_t *) _p;
	_mc = &_uc->uc_mcontext;

	pc = (u1 *) _mc->sc_pc;

	tobj->pc = pc;
}


/* md_executionstate_read ******************************************************

   Read the given context into an executionstate.

*******************************************************************************/

void md_executionstate_read(executionstate_t *es, void *context)
{
	ucontext_t *_uc;
	mcontext_t *_mc;
	int         i;

	_uc = (ucontext_t *) context;
	_mc = &_uc->uc_mcontext;

	/* read special registers */
	es->pc = (u1 *) _mc->sc_pc;
	es->sp = (u1 *) _mc->sc_regs[REG_SP];
	es->pv = (u1 *) _mc->sc_regs[REG_PV];
	es->ra = (u1 *) _mc->sc_regs[REG_RA];

	/* read integer registers */
	for (i = 0; i < INT_REG_CNT; i++)
		es->intregs[i] = _mc->sc_regs[i];

	/* read float registers */
	/* Do not use the assignment operator '=', as the type of
	 * the _mc->sc_fpregs[i] can cause invalid conversions. */

	assert(sizeof(_mc->sc_fpregs) == sizeof(es->fltregs));
	os::memcpy(&es->fltregs, &_mc->sc_fpregs, sizeof(_mc->sc_fpregs));
}


/* md_executionstate_write *****************************************************

   Write the given executionstate back to the context.

*******************************************************************************/

void md_executionstate_write(executionstate_t *es, void *context)
{
	ucontext_t *_uc;
	mcontext_t *_mc;
	int         i;

	_uc = (ucontext_t *) context;
	_mc = &_uc->uc_mcontext;

	/* write integer registers */
	for (i = 0; i < INT_REG_CNT; i++)
		_mc->sc_regs[i] = es->intregs[i];

	/* write float registers */
	/* Do not use the assignment operator '=', as the type of
	 * the _mc->sc_fpregs[i] can cause invalid conversions. */

	assert(sizeof(_mc->sc_fpregs) == sizeof(es->fltregs));
	os::memcpy(&_mc->sc_fpregs, &es->fltregs, sizeof(_mc->sc_fpregs));

	/* write special registers */
	_mc->sc_pc           = (ptrint) es->pc;
	_mc->sc_regs[REG_SP] = (ptrint) es->sp;
	_mc->sc_regs[REG_PV] = (ptrint) es->pv;
	_mc->sc_regs[REG_RA] = (ptrint) es->ra;
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
 */
