/* src/vm/jit/alpha/freebsd/md-os.cpp - machine dependent Alpha FreeBSD functions

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
#include <ucontext.h>
#include <sys/types.h>                         /* required by <machine/reg.h> */
#include <machine/reg.h>

#include "vm/types.hpp"

#include "vm/jit/alpha/md-abi.hpp"

#include "vm/global.hpp"
#include "vm/signallocal.hpp"
#include "vm/jit/asmpart.hpp"


/* md_signal_handler_sigsegv ***************************************************

   NullPointerException signal handler for hardware null pointer
   check.

*******************************************************************************/

void md_signal_handler_sigsegv(int sig, siginfo_t *siginfo, void *_p)
{
	ucontext_t  *_uc;
	mcontext_t  *_mc;
	u4           instr;
	ptrint       addr;
	u1          *pv;
	u1          *sp;
	u1          *ra;
	u1          *xpc;

	_uc = (ucontext_t *) _p;
	_mc = &_uc->uc_mcontext;

	instr = *((s4 *) (_mc->mc_regs[R_PC]));
	addr = _mc->mc_regs[(instr >> 16) & 0x1f];

	if (addr == 0) {
		pv  = (u1 *) _mc->mc_regs[REG_PV];
		sp  = (u1 *) _mc->mc_regs[REG_SP];
		ra  = (u1 *) _mc->mc_regs[REG_RA];       /* this is correct for leafs */
		xpc = (u1 *) _mc->mc_regs[R_PC];

		_mc->mc_regs[REG_ITMP1_XPTR] =
			(ptrint) stacktrace_hardware_nullpointerexception(pv, sp, ra, xpc);

		_mc->mc_regs[REG_ITMP2_XPC] = (ptrint) xpc;
		_mc->mc_regs[R_PC] = (ptrint) asm_handle_exception;

	} else {
		addr += (long) ((instr << 16) >> 16);

		throw_cacao_exception_exit(string_java_lang_InternalError,
								   "Segmentation fault: 0x%016lx at 0x%016lx\n",
								   addr, _mc->mc_regs[R_PC]);
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
 */
