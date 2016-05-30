/* src/vm/jit/powerpc64/linux/md-os.cpp - machine dependent PowerPC64 Linux functions

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
#include <ucontext.h>

#include "vm/types.hpp"

#include "vm/jit/powerpc64/codegen.hpp"
#include "vm/jit/powerpc64/md.hpp"
#include "vm/jit/powerpc64/linux/md-abi.hpp"

#include "threads/thread.hpp"

#include "vm/signallocal.hpp"

#include "vm/jit/asmpart.hpp"
#include "vm/jit/executionstate.hpp"
#include "vm/jit/trap.hpp"


/**
 * Signal handler for hardware-exceptions.
 */
void md_signal_handler_sigsegv(int sig, siginfo_t *siginfo, void *_p)
{
 	ucontext_t* _uc = (ucontext_t *) _p;
 	mcontext_t* _mc = &(_uc->uc_mcontext);

	void* xpc = (void*) _mc->gp_regs[PT_NIP];

	// Handle the trap.
	trap_handle(TRAP_SIGSEGV, xpc, _p);
}


/**
 * Signal handler for patcher calls.
 */
void md_signal_handler_sigill(int sig, siginfo_t* siginfo, void* _p)
{
	ucontext_t* _uc = (ucontext_t*) _p;
	mcontext_t* _mc = &(_uc->uc_mcontext);

	void* xpc =(void*) _mc->gp_regs[PT_NIP];

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
 	_mc = &(_uc->uc_mcontext);

	pc = (u1 *) _mc->gp_regs[PT_NIP];

	tobj->pc = pc;
}


/* md_executionstate_read ******************************************************

   Read the given context into an executionstate.

*******************************************************************************/

void md_executionstate_read(executionstate_t *es, void *context)
{
	ucontext_t    *_uc;
	mcontext_t    *_mc;
	s4              i;

	_uc = (ucontext_t *) context;
	_mc = &(_uc->uc_mcontext);

	/* read special registers */
	es->pc = (u1 *) _mc->gp_regs[PT_NIP];
	es->sp = (u1 *) _mc->gp_regs[REG_SP];
	es->pv = (u1 *) _mc->gp_regs[REG_PV];
	es->ra = (u1 *) _mc->gp_regs[PT_LNK];

	/* read integer registers */
	for (i = 0; i < INT_REG_CNT; i++)
		es->intregs[i] = _mc->gp_regs[i];

	/* read float registers */
	/* Do not use the assignment operator '=', as the type of
	 * the _mc->fpregs[i] can cause invalid conversions. */

	// The assertion below will fail because _mc->fp_regs[] also
	// contains the "fpscr" register.
	//assert(sizeof(_mc->fp_regs) == sizeof(es->fltregs));
	os::memcpy(&es->fltregs, &_mc->fp_regs, sizeof(es->fltregs));
}


/* md_executionstate_write *****************************************************

   Write the given executionstate back to the context.

*******************************************************************************/

void md_executionstate_write(executionstate_t *es, void *context)
{
	ucontext_t    *_uc;
	mcontext_t    *_mc;
	s4              i;

	_uc = (ucontext_t *) context;
	_mc = &(_uc->uc_mcontext);

	/* write integer registers */
	for (i = 0; i < INT_REG_CNT; i++)
		_mc->gp_regs[i] = es->intregs[i];

	/* write float registers */
	/* Do not use the assignment operator '=', as the type of
	 * the _mc->fpregs[i] can cause invalid conversions. */

	// The assertion below will fail because _mc->fp_regs[] also
	// contains the "fpscr" register.
	//assert(sizeof(_mc->fp_regs) == sizeof(es->fltregs));
	os::memcpy(&_mc->fp_regs, &es->fltregs, sizeof(es->fltregs));

	/* write special registers */
	_mc->gp_regs[PT_NIP] = (ptrint) es->pc;
	_mc->gp_regs[REG_SP] = (ptrint) es->sp;
	_mc->gp_regs[REG_PV] = (ptrint) es->pv;
	_mc->gp_regs[PT_LNK] = (ptrint) es->ra;
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
