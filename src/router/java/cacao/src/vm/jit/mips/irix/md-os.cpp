/* src/vm/jit/mips/irix/md-os.cpp - machine dependent MIPS IRIX functions

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
#include <sys/fpu.h>

#include "vm/types.hpp"

#include "vm/jit/mips/codegen.hpp"
#include "vm/jit/mips/md-abi.hpp"

#include "mm/gc.hpp"

#include "vm/global.hpp"
#include "vm/signallocal.hpp"

#include "vm/jit/asmpart.hpp"
#include "vm/jit/codegen-common.hpp"


/* md_init *********************************************************************

   Do some machine dependent initialization.

*******************************************************************************/

void md_init(void)
{
	/* The Boehm GC initialization blocks the SIGSEGV signal. So we do a      */
	/* dummy allocation here to ensure that the GC is initialized.            */

#if defined(ENABLE_GC_BOEHM)
	(void) GCNEW(u1);
#endif


	/* Turn off flush-to-zero */

	{
		union fpc_csr n;
		n.fc_word = get_fpc_csr();
		n.fc_struct.flush = 0;
		set_fpc_csr(n.fc_word);
	}
}


/* md_signal_handler_sigsegv ***************************************************

   Signal handler for hardware-exceptions.

*******************************************************************************/

void md_signal_handler_sigsegv(int sig, siginfo_t *siginfo, void *_p)
{
	ucontext_t     *_uc;
	mcontext_t     *_mc;
	u1             *pv;
	u1             *sp;
	u1             *ra;
	u1             *xpc;
	u4              mcode;
	int             d;
	int             s1;
	int16_t         disp;
	intptr_t        val;
	intptr_t        addr;
	int              type;
	void           *p;

	_uc = (struct ucontext *) _p;
	_mc = &_uc->uc_mcontext;

	pv  = (u1 *) _mc->gregs[REG_PV];
	sp  = (u1 *) _mc->gregs[REG_SP];
	ra  = (u1 *) _mc->gregs[REG_RA];             /* this is correct for leafs */
	xpc = (u1 *) _mc->gregs[CTX_EPC];

	/* get exception-throwing instruction */

	mcode = *((u4 *) xpc);

	d    = M_ITYPE_GET_RT(mcode);
	s1   = M_ITYPE_GET_RS(mcode);
	disp = M_ITYPE_GET_IMM(mcode);

	/* check for special-load */

	if (s1 == REG_ZERO) {
		/* we use the exception type as load displacement */

		type = disp;
		val  = _mc->gregs[d];
	}
	else {
		/* This is a normal NPE: addr must be NULL and the NPE-type
		   define is 0. */

		addr = _mc->gregs[s1];
		type = (int) addr;
		val  = 0;
	}

	/* Handle the type. */

	p = signal_handle(type, val, pv, sp, ra, xpc, _p);

	/* set registers (only if exception object ready) */

	if (p != NULL) {
		_mc->gregs[REG_ITMP1_XPTR] = (intptr_t) p;
		_mc->gregs[REG_ITMP2_XPC]  = (intptr_t) xpc;
		_mc->gregs[CTX_EPC]        = (intptr_t) asm_handle_exception;
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
