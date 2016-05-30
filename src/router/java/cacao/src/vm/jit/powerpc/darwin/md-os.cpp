/* src/vm/jit/powerpc/darwin/md-os.cpp - machine dependent PowerPC Darwin functions

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
#include <signal.h>
#include <stdint.h>
#include <ucontext.h>

#include "vm/types.hpp"

#include "vm/jit/powerpc/codegen.hpp"
#include "vm/jit/powerpc/darwin/md-abi.hpp"

#include "threads/thread.hpp"

#include "vm/signallocal.hpp"

#include "vm/jit/trap.hpp"

#if !__DARWIN_UNIX03
#define __srr0 srr0
#define __r0   r0
#define __r1   r1
#define __r13  r13
#define __lr   lr
#define __ss   ss
#endif

/**
 * Signal handler for hardware-exceptions.
 */
void md_signal_handler_sigsegv(int sig, siginfo_t *siginfo, void *_p)
{
	ucontext_t*         _uc = (ucontext_t *) _p;
	mcontext_t          _mc = _uc->uc_mcontext;
	ppc_thread_state_t* _ss = &(_mc->__ss);

	void* xpc = (void*) _ss->__srr0;

	// Handle the trap.
	trap_handle(TRAP_SIGSEGV, xpc, _p);
}


/**
 * Signal handler for hardware-traps.
 */
void md_signal_handler_sigtrap(int sig, siginfo_t *siginfo, void *_p)
{
	ucontext_t*         _uc = (ucontext_t *) _p;
	mcontext_t          _mc = _uc->uc_mcontext;
	ppc_thread_state_t* _ss = &(_mc->__ss);

	void* xpc = (void*) _ss->__srr0;

	// Handle the trap.
	trap_handle(TRAP_SIGTRAP, xpc, _p);
}


/**
 * Signal handler for hardware-patchers.
 */
void md_signal_handler_sigill(int sig, siginfo_t *siginfo, void *_p)
{
	ucontext_t*         _uc = (ucontext_t *) _p;
	mcontext_t          _mc = _uc->uc_mcontext;
	ppc_thread_state_t* _ss = &(_mc->__ss);

	void* xpc = (void*) _ss->__srr0;

	// Handle the trap.
	trap_handle(TRAP_SIGILL, xpc, _p);
}


/* md_signal_handler_sigusr2 ***************************************************

   Signal handler for profiling sampling.

*******************************************************************************/

void md_signal_handler_sigusr2(int sig, siginfo_t *siginfo, void *_p)
{
	threadobject       *t;
	ucontext_t         *_uc;
	mcontext_t          _mc;
	ppc_thread_state_t *_ss;
	u1                 *pc;

	t = THREADOBJECT;

	_uc = (ucontext_t *) _p;
	_mc = _uc->uc_mcontext;
	_ss = &_mc->__ss;

	pc = (u1 *) _ss->__srr0;

	t->pc = pc;
}


/* md_executionstate_read ******************************************************

   Read the given context into an executionstate.

*******************************************************************************/

void md_executionstate_read(executionstate_t *es, void *context)
{
	ucontext_t*         _uc = (ucontext_t *) context;
	mcontext_t          _mc = _uc->uc_mcontext;
	ppc_thread_state_t* _ss = &(_mc->__ss);

	/* read special registers */
	es->pc = (uint8_t*) _ss->__srr0;
	es->sp = (uint8_t*) _ss->__r1;
	es->pv = (uint8_t*) _ss->__r13;
	es->ra = (uint8_t*) _ss->__lr;

	/* read integer registers */
	unsigned int* regs = &(_ss->__r0);
	for (int i = 0; i < INT_REG_CNT; i++)
		es->intregs[i] = regs[i];

	/* read float registers */
	for (int i = 0; i < FLT_REG_CNT; i++)
		es->fltregs[i] = 0xdeadbeefdeadbeefULL;
}


/* md_executionstate_write *****************************************************

   Write the given executionstate back to the context.

*******************************************************************************/

void md_executionstate_write(executionstate_t *es, void *context)
{
	ucontext_t*         _uc = (ucontext_t *) context;
	mcontext_t          _mc = _uc->uc_mcontext;
	ppc_thread_state_t* _ss = &(_mc->__ss);

	/* write integer registers */
	unsigned int* regs = &(_ss->__r0);
	for (int i = 0; i < INT_REG_CNT; i++)
		regs[i] = es->intregs[i];

	/* write special registers */
	_ss->__srr0 = (intptr_t) es->pc;
	_ss->__r1   = (intptr_t) es->sp;
	_ss->__r13  = (intptr_t) es->pv;
	_ss->__lr   = (intptr_t) es->ra;
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
