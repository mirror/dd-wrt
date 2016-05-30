/* src/vm/jit/i386/darwin/md-os.cpp - machine dependent i386 Darwin functions

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

#include <assert.h>
#include <signal.h>
#include <stdint.h>
#include <ucontext.h>

#include "vm/types.hpp"

#include "vm/jit/i386/codegen.hpp"
#include "vm/jit/i386/md.hpp"

#include "threads/thread.hpp"

#include "vm/global.hpp"
#include "vm/signallocal.hpp"

#include "vm/jit/executionstate.hpp"
#include "vm/jit/trap.hpp"

#include "vm/jit/i386/codegen.hpp"

#if !__DARWIN_UNIX03
#define __eax eax
#define __ebx ebx
#define __ecx ecx
#define __edx edx
#define __esi esi
#define __edi edi
#define __ebp ebp
#define __esp esp
#define __eip eip
#define __ss ss
#endif

/**
 * Signal handler for hardware exceptions.
 */
void md_signal_handler_sigsegv(int sig, siginfo_t *siginfo, void *_p)
{
	ucontext_t*          _uc = (ucontext_t *) _p;
	mcontext_t           _mc = _uc->uc_mcontext;
	i386_thread_state_t* _ss = &_mc->__ss;

	void* xpc = (void*) _ss->__eip;

	// Handle the trap.
	trap_handle(TRAP_SIGSEGV, xpc, _p);
}


/**
 * Signal handler for hardware divide by zero (ArithmeticException)
 * check.
 */
void md_signal_handler_sigfpe(int sig, siginfo_t *siginfo, void *_p)
{
	ucontext_t*          _uc = (ucontext_t *) _p;
	mcontext_t           _mc = _uc->uc_mcontext;
	i386_thread_state_t* _ss = &_mc->__ss;

	void* xpc = (void*) _ss->__eip;

	// Handle the trap.
	trap_handle(TRAP_SIGFPE, xpc, _p);
}


/* md_signal_handler_sigusr2 ***************************************************

   Signal handler for profiling sampling.

*******************************************************************************/

void md_signal_handler_sigusr2(int sig, siginfo_t *siginfo, void *_p)
{
	threadobject        *t;
	ucontext_t          *_uc;
	mcontext_t           _mc;
	i386_thread_state_t *_ss;
	u1                  *pc;

	t = THREADOBJECT;

	_uc = (ucontext_t *) _p;
	_mc = _uc->uc_mcontext;
	_ss = &_mc->__ss;

	pc = (u1 *) _ss->__eip;

	t->pc = pc;
}


/**
 * Signal handler for hardware patcher traps (ud2).
 */
void md_signal_handler_sigill(int sig, siginfo_t *siginfo, void *_p)
{
	ucontext_t*          _uc = (ucontext_t *) _p;
	mcontext_t           _mc = _uc->uc_mcontext;
	i386_thread_state_t* _ss = &_mc->__ss;

	void* xpc = (void*) _ss->__eip;

	// Handle the trap.
	trap_handle(TRAP_SIGILL, xpc, _p);
}

/* md_executionstate_read ******************************************************

   Read the given context into an executionstate.

*******************************************************************************/

void md_executionstate_read(executionstate_t *es, void *context)
{
	ucontext_t          *_uc;
	mcontext_t           _mc; 
	i386_thread_state_t *_ss;
	int                  i;

	_uc = (ucontext_t *) context;
	_mc = _uc->uc_mcontext;
	_ss = &_mc->__ss;

	/* read special registers */
	es->pc = (u1 *) _ss->__eip;
	es->sp = (u1 *) _ss->__esp;
	es->pv = NULL;                   /* pv must be looked up via AVL tree */

	/* read integer registers */
	for (i = 0; i < INT_REG_CNT; i++)
		es->intregs[i] = (i == 0) ? _ss->__eax :
			((i == 1) ? _ss->__ecx :
			((i == 2) ? _ss->__edx :
			((i == 3) ? _ss->__ebx :
			((i == 4) ? _ss->__esp :
			((i == 5) ? _ss->__ebp :
			((i == 6) ? _ss->__esi : _ss->__edi))))));

	/* read float registers */
	for (i = 0; i < FLT_REG_CNT; i++)
		es->fltregs[i] = 0xdeadbeefdeadbeefULL;
}


/* md_executionstate_write *****************************************************

   Write the given executionstate back to the context.

*******************************************************************************/

void md_executionstate_write(executionstate_t *es, void *context)
{
	ucontext_t*          _uc;
	mcontext_t           _mc;
	i386_thread_state_t* _ss;
	int                  i;

	_uc = (ucontext_t *) context;
	_mc = _uc->uc_mcontext;
	_ss = &_mc->__ss;

	/* write integer registers */
	for (i = 0; i < INT_REG_CNT; i++)
		*((i == 0) ? &_ss->__eax :
		 ((i == 1) ? &_ss->__ecx :
		 ((i == 2) ? &_ss->__edx :
		 ((i == 3) ? &_ss->__ebx :
		 ((i == 4) ? &_ss->__esp :
		 ((i == 5) ? &_ss->__ebp :
		 ((i == 6) ? &_ss->__esi : &_ss->__edi))))))) = es->intregs[i];

	/* write special registers */
	_ss->__eip = (ptrint) es->pc;
	_ss->__esp = (ptrint) es->sp;
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
