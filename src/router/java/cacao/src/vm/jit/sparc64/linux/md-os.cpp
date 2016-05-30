/* src/vm/jit/sparc64/linux/md-os.cpp - machine dependent SPARC Linux functions

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
#include <stdint.h>
#include <signal.h>

#include "vm/types.hpp"

#include "vm/jit/sparc64/codegen.hpp"
#include "vm/jit/sparc64/md-abi.hpp"

#include "vm/signallocal.hpp"

#include "vm/jit/asmpart.hpp"
#include "vm/jit/stacktrace.hpp"
#include "vm/jit/trap.hpp"


typedef struct sigcontext sigcontext;

ptrint md_get_reg_from_context(sigcontext *ctx, u4 rindex)
{
	ptrint val;	
	s8     *window;
	
	
	/* return 0 for REG_ZERO */
	
	if (rindex == 0)
		return 0;
		
	
	if (rindex <= 15) {
		
		/* register is in global or out range, available in context */
		
		val = ctx->sigc_regs.u_regs[rindex];
	}
	else {
		assert(rindex <= 31);
		
		/* register is local or in, need to fetch from regsave area on stack */
		
		window = ctx->sigc_regs.u_regs[REG_SP] + BIAS;
		val = window[rindex - 16];
	}
	
	return val;
}
	
	

/* md_signal_handler_sigsegv ***************************************************

   NullPointerException signal handler for hardware null pointer
   check.

*******************************************************************************/

void md_signal_handler_sigsegv(int sig, siginfo_t *info , void *_p)
{
	/*
	ucontext_t  *_uc;
	mcontext_t  *_mc;
	*/
	sigcontext *ctx;
	u1          *pv;
	u1          *sp;
	u1          *ra;
	u1          *xpc;
	u4          mcode;
	int         d;
	int         s1;
	int16_t     disp;
	intptr_t    val;
	intptr_t    addr;
	int         type;
	void       *p;

	ctx = (sigcontext *) info;


	pv  = (u1 *) md_get_reg_from_context(ctx, REG_PV_CALLEE);
	sp  = (u1 *) md_get_reg_from_context(ctx, REG_SP);
	ra  = (u1 *) md_get_reg_from_context(ctx, REG_RA_CALLEE);  /* this is correct for leafs */
	xpc = (u1 *) ctx->sigc_regs.tpc;

	/* get exception-throwing instruction */	

	mcode = *((u4 *) xpc);

	d    = M_OP3_GET_RD(mcode);
	s1   = M_OP3_GET_RS(mcode);
	disp = M_OP3_GET_IMM(mcode);

	/* flush register windows? */
	
	val  = md_get_reg_from_context(ctx, d);

	/* check for special-load */

	if (s1 == REG_ZERO) {
		/* we use the exception type as load displacement */

		type = disp;
	}
	else {
		/* This is a normal NPE: addr must be NULL and the NPE-type
		   define is 0. */

		addr = md_get_reg_from_context(ctx, s1);
		type = (int) addr;
	}

	/* Handle the trap. */

	p = trap_handle(type, val, pv, sp, ra, xpc, _p);

	/* set registers */

	ctx->sigc_regs.u_regs[REG_ITMP2_XPTR] = (uintptr_t) p;
	ctx->sigc_regs.u_regs[REG_ITMP3_XPC]  = (uintptr_t) xpc;
	ctx->sigc_regs.tpc                    = (uintptr_t) asm_handle_exception;
	ctx->sigc_regs.tnpc                   = (uintptr_t) asm_handle_exception + 4;
}


/* md_icacheflush **************************************************************

   Calls the system's function to flush the instruction cache.

*******************************************************************************/

void md_icacheflush(u1 *addr, s4 nbytes)
{
	u1* end;
	
	end = addr + nbytes;
	
	/* zero the least significant 3 bits to align on a 64-bit boundary */
	addr = (u1 *) (((ptrint) addr) & -8l);
	
	while (addr < end) {
		__asm__ (
			"flush %0"
			:
			: "r"(addr)
			);
		addr += 8;
	}
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
