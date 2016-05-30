/* src/vm/jit/i386/linux/md-os.cpp - machine dependent i386 Linux functions

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


#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "config.h"

#include <stdint.h>
#include <ucontext.h>

#include "vm/types.hpp"

#include "vm/jit/i386/codegen.hpp"
#include "vm/jit/i386/md.hpp"

#include "threads/thread.hpp"

#include "vm/signallocal.hpp"

#include "vm/jit/executionstate.hpp"
#include "vm/jit/trap.hpp"


/**
 * Signal handler for hardware exceptions.
 */
void md_signal_handler_sigsegv(int sig, siginfo_t *siginfo, void *_p)
{
	ucontext_t* _uc = (ucontext_t *) _p;
	mcontext_t* _mc = &_uc->uc_mcontext;

	void* xpc = (void*) _mc->gregs[REG_EIP];

	// Handle the trap.
	trap_handle(TRAP_SIGSEGV, xpc, _p);
}


/**
 * Signal handler for hardware divide by zero (ArithmeticException)
 * check.
 */
void md_signal_handler_sigfpe(int sig, siginfo_t *siginfo, void *_p)
{
	ucontext_t* _uc = (ucontext_t *) _p;
	mcontext_t* _mc = &_uc->uc_mcontext;

	void* xpc = (void*) _mc->gregs[REG_EIP];

	// Handle the trap.
	trap_handle(TRAP_SIGFPE, xpc, _p);
}


/**
 * Signal handler for hardware patcher traps (ud2).
 */
void md_signal_handler_sigill(int sig, siginfo_t *siginfo, void *_p)
{
	ucontext_t* _uc = (ucontext_t *) _p;
	mcontext_t* _mc = &_uc->uc_mcontext;

	void* xpc = (void*) _mc->gregs[REG_EIP];

	// Handle the trap.
	trap_handle(TRAP_SIGILL, xpc, _p);
}


/* md_signal_handler_sigusr2 ***************************************************

   Signal handler for profiling sampling.

*******************************************************************************/

void md_signal_handler_sigusr2(int sig, siginfo_t *siginfo, void *_p)
{
	threadobject *t;
	ucontext_t   *_uc;
	mcontext_t   *_mc;
	u1           *pc;

	t = THREADOBJECT;

	_uc = (ucontext_t *) _p;
	_mc = &_uc->uc_mcontext;

	pc = (u1 *) _mc->gregs[REG_EIP];

	t->pc = pc;
}


/* md_executionstate_read ******************************************************

   Read the given context into an executionstate for Replacement.

*******************************************************************************/

void md_executionstate_read(executionstate_t *es, void *context)
{
	ucontext_t *_uc;
	mcontext_t *_mc;
	s4          i;

	_uc = (ucontext_t *) context;
	_mc = &_uc->uc_mcontext;

	/* read special registers */
	es->pc = (u1 *) _mc->gregs[REG_EIP];
	es->sp = (u1 *) _mc->gregs[REG_ESP];
	es->pv = NULL;                   /* pv must be looked up via AVL tree */

	/* read integer registers */
	for (i = 0; i < INT_REG_CNT; i++)
		es->intregs[i] = _mc->gregs[REG_EAX - i];

	/* read float registers */
	for (i = 0; i < FLT_REG_CNT; i++)
		es->fltregs[i] = 0xdeadbeefdeadbeefULL;
}


/* md_executionstate_write *****************************************************

   Write the given executionstate back to the context for Replacement.

*******************************************************************************/

void md_executionstate_write(executionstate_t *es, void *context)
{
	ucontext_t *_uc;
	mcontext_t *_mc;
	s4          i;

	_uc = (ucontext_t *) context;
	_mc = &_uc->uc_mcontext;

	/* write integer registers */
	for (i = 0; i < INT_REG_CNT; i++)
		_mc->gregs[REG_EAX - i] = es->intregs[i];

	/* write special registers */
	_mc->gregs[REG_EIP] = (ptrint) es->pc;
	_mc->gregs[REG_ESP] = (ptrint) es->sp;
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
