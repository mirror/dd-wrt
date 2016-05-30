/* src/vm/jit/x86_64/solaris/md-os.cpp - machine dependent x86_64 Solaris functions

   Copyright (C) 2008-2013
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
#include <cstdlib>
#include <stdint.h>
#include <ucontext.h>

#include "vm/types.hpp"

#include "vm/jit/x86_64/codegen.hpp"
#include "vm/jit/x86_64/md.hpp"

#include "threads/thread.hpp"

#include "vm/signallocal.hpp"

#include "vm/jit/asmpart.hpp"
#include "vm/jit/executionstate.hpp"
#include "vm/jit/trap.hpp"


/**
 * Signal handler for hardware exceptions.
 */
void md_signal_handler_sigsegv(int sig, siginfo_t *siginfo, void *_p)
{
	ucontext_t *_uc = (ucontext_t *) _p;
	mcontext_t *_mc = &_uc->uc_mcontext;

	/* ATTENTION: Don't use CACAO's internal REG_* defines as they are
	   different to the ones in <ucontext.h>. */

	void* xpc = (void *) _mc->gregs[REG_RIP];

    // Handle the trap.
    trap_handle(TRAP_SIGSEGV, xpc, _p);
}


/**
 * Signal handler for hardware divide by zero (ArithmeticException)
 * check.
 */
void md_signal_handler_sigfpe(int sig, siginfo_t *siginfo, void *_p)
{
	ucontext_t *_uc = (ucontext_t *) _p;
	mcontext_t *_mc = &_uc->uc_mcontext;

	/* ATTENTION: Don't use CACAO's internal REG_* defines as they are
	   different to the ones in <ucontext.h>. */

	void* xpc = (void *) _mc->gregs[REG_RIP];

    // Handle the trap.
    trap_handle(TRAP_SIGFPE, xpc, _p);
}


/**
 * Signal handler for hardware patcher traps (ud2).
 */
void md_signal_handler_sigill(int sig, siginfo_t *siginfo, void *_p)
{
	ucontext_t *_uc = (ucontext_t *) _p;
	mcontext_t *_mc = &_uc->uc_mcontext;

	/* ATTENTION: Don't use CACAO's internal REG_* defines as they are
	   different to the ones in <ucontext.h>. */

	void* xpc = (void *) _mc->gregs[REG_RIP];

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

	/* ATTENTION: Don't use CACAO's internal REG_* defines as they are
	   different to the ones in <ucontext.h>. */

	pc = (u1 *) _mc->gregs[REG_RIP];

	t->pc = pc;
}


/* md_executionstate_read ******************************************************

   Read the given context into an executionstate.

*******************************************************************************/

void md_executionstate_read(executionstate_t *es, void *context)
{
	ucontext_t *_uc;
	mcontext_t *_mc;
	s4          i;
	s4          d;

	_uc = (ucontext_t *) context;
	_mc = &_uc->uc_mcontext;

	/* read special registers */
	es->pc = (u1 *) _mc->gregs[REG_RIP];
	es->sp = (u1 *) _mc->gregs[REG_RSP];
	es->pv = NULL;

	/* read integer registers */
	for (i = 0; i < INT_REG_CNT; i++) {
		switch (i) {
		case 0:  /* REG_RAX */
			d = REG_RAX;
			break;
		case 1:  /* REG_RCX */
			d = REG_RCX;
			break;
		case 2:  /* REG_RDX */
			d = REG_RDX;
			break;
		case 3:  /* REG_RBX */
			d = REG_RBX;
			break;
		case 4:  /* REG_RSP */
			d = REG_RSP;
			break;
		case 5:  /* REG_RBP */
			d = REG_RBP;
			break;
		case 6:  /* REG_RSI */
			d = REG_RSI;
			break;
		case 7:  /* REG_RDI */
			d = REG_RDI;
			break;
		case 8:  /* REG_R8  == 7  */
		case 9:  /* REG_R9  == 6  */
		case 10: /* REG_R10 == 5  */
		case 11: /* REG_R11 == 4  */
		case 12: /* REG_R12 == 3  */
		case 13: /* REG_R13 == 2  */
		case 14: /* REG_R14 == 1  */
		case 15: /* REG_R15 == 0  */
			d = 15 - i;
			break;
		}

		es->intregs[i] = _mc->gregs[d];
	}

	/* read float registers */
	for (i = 0; i < FLT_REG_CNT; i++)
		es->fltregs[i] = 0xdeadbeefdeadbeefL;
}


/* md_executionstate_write *****************************************************

   Write the given executionstate back to the context.

*******************************************************************************/

void md_executionstate_write(executionstate_t *es, void *context)
{
	ucontext_t *_uc;
	mcontext_t *_mc;
	s4          i;
	s4          d;

	_uc = (ucontext_t *) context;
	_mc = &_uc->uc_mcontext;

	/* write integer registers */
	for (i = 0; i < INT_REG_CNT; i++) {
		switch (i) {
		case 0:  /* REG_RAX */
			d = REG_RAX;
			break;
		case 1:  /* REG_RCX */
			d = REG_RCX;
			break;
		case 2:  /* REG_RDX */
			d = REG_RDX;
			break;
		case 3:  /* REG_RBX */
			d = REG_RBX;
			break;
		case 4:  /* REG_RSP */
			d = REG_RSP;
			break;
		case 5:  /* REG_RBP */
			d = REG_RBP;
			break;
		case 6:  /* REG_RSI */
			d = REG_RSI;
			break;
		case 7:  /* REG_RDI */
			d = REG_RDI;
			break;
		case 8:  /* REG_R8  == 7  */
		case 9:  /* REG_R9  == 6  */
		case 10: /* REG_R10 == 5  */
		case 11: /* REG_R11 == 4  */
		case 12: /* REG_R12 == 3  */
		case 13: /* REG_R13 == 2  */
		case 14: /* REG_R14 == 1  */
		case 15: /* REG_R15 == 0  */
			d = 15 - i;
			break;
		}

		_mc->gregs[d] = es->intregs[i];
	}

	/* write special registers */
	_mc->gregs[REG_RIP] = (ptrint) es->pc;
	_mc->gregs[REG_RSP] = (ptrint) es->sp;
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
