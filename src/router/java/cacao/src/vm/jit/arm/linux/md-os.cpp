/* src/vm/jit/arm/linux/md-os.cpp - machine dependent ARM Linux functions

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

#include <stdint.h>
#include "mm/memory.hpp"

#define ucontext broken_glibc_ucontext
#define ucontext_t broken_glibc_ucontext_t
#include <ucontext.h>
#undef ucontext
#undef ucontext_t

typedef struct ucontext {
   unsigned long     uc_flags;
   struct ucontext  *uc_link;
   stack_t           uc_stack;
   struct sigcontext uc_mcontext;
   sigset_t          uc_sigmask;
} ucontext_t;

#define scontext_t struct sigcontext

#include "vm/types.hpp"

#include "vm/jit/arm/md.hpp"
#include "vm/jit/arm/md-abi.hpp"

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
	ucontext_t* _uc = (ucontext_t*) _p;
	scontext_t* _sc = &_uc->uc_mcontext;

	/* ATTENTION: glibc included messed up kernel headers we needed a
	   workaround for the ucontext structure. */

	void* xpc = (u1 *) _sc->arm_pc;

	// Handle the trap.
	trap_handle(TRAP_SIGSEGV, xpc, _p);
}


/**
 * Illegal instruction signal handler for hardware exception checks.
 */
void md_signal_handler_sigill(int sig, siginfo_t *siginfo, void *_p)
{
	ucontext_t* _uc = (ucontext_t*) _p;
	scontext_t* _sc = &_uc->uc_mcontext;

	/* ATTENTION: glibc included messed up kernel headers we needed a
	   workaround for the ucontext structure. */

	void* xpc = (void*) _sc->arm_pc;

	// Handle the trap.
	trap_handle(TRAP_SIGILL, xpc, _p);
}


/* md_signal_handler_sigusr2 ***************************************************

   Signal handler for profiling sampling.

*******************************************************************************/

void md_signal_handler_sigusr2(int sig, siginfo_t *siginfo, void *_p)
{
	threadobject *thread;
	ucontext_t   *_uc;
	scontext_t   *_sc;
	u1           *pc;

	thread = THREADOBJECT;

	_uc = (ucontext_t*) _p;
	_sc = &_uc->uc_mcontext;

	pc = (u1 *) _sc->arm_pc;

	thread->pc = pc;
}


/**
 * Read the given context into an executionstate.
 *
 * @param es      execution state
 * @param context machine context
 */
void md_executionstate_read(executionstate_t *es, void *context)
{
	ucontext_t *_uc;
	scontext_t *_sc;
	int         i;

	_uc = (ucontext_t *) context;
	_sc = &_uc->uc_mcontext;

	/* ATTENTION: glibc included messed up kernel headers we needed a
	   workaround for the ucontext structure. */

	/* read special registers */

	es->pc = (u1 *) _sc->arm_pc;
	es->sp = (u1 *) _sc->arm_sp;
	es->pv = (u1 *) _sc->arm_ip;
	es->ra = (u1 *) _sc->arm_lr;

	/* read integer registers */

	for (i = 0; i < INT_REG_CNT; i++)
		es->intregs[i] = *((int32_t*) _sc + OFFSET(scontext_t, arm_r0)/4 + i);

	/* read float registers */

	for (i = 0; i < FLT_REG_CNT; i++)
		es->fltregs[i] = 0xdeadbeefdeadbeefULL;
}


/**
 * Write the given executionstate back to the context.
 *
 * @param es      execution state
 * @param context machine context
 */
void md_executionstate_write(executionstate_t *es, void *context)
{
	ucontext_t *_uc;
	scontext_t *_sc;
	int         i;

	_uc = (ucontext_t *) context;
	_sc = &_uc->uc_mcontext;

	/* ATTENTION: glibc included messed up kernel headers we needed a
	   workaround for the ucontext structure. */

	/* write integer registers */

	for (i = 0; i < INT_REG_CNT; i++)
		*((int32_t*) _sc + OFFSET(scontext_t, arm_r0)/4 + i) = es->intregs[i];

	/* write special registers */

	_sc->arm_pc = (ptrint) es->pc;
	_sc->arm_sp = (ptrint) es->sp;
	_sc->arm_ip = (ptrint) es->pv;
	_sc->arm_lr = (ptrint) es->ra;
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

