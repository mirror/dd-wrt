/* src/vm/jit/x86_64/md-abi.cpp - functions for x86_64 Linux ABI

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
#include "vm/types.hpp"

#include "vm/jit/x86_64/md-abi.hpp"

#include "vm/descriptor.hpp"
#include "vm/global.hpp"
#include "vm/method.hpp"

#include "vm/jit/abi.hpp"
#include "vm/jit/code.hpp"
#include "vm/jit/jit.hpp" /* for REG_* (maybe can be removed) */
#include "vm/jit/stack.hpp"


/* register descripton array **************************************************/

s4 nregdescint[] = {
    REG_RET, REG_ARG, REG_ARG, REG_TMP, REG_RES, REG_SAV, REG_ARG, REG_ARG,
    REG_ARG, REG_ARG, REG_RES, REG_RES, REG_SAV, REG_SAV, REG_SAV, REG_SAV,
    REG_END
};

const char *abi_registers_integer_name[] = {
	"rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi",
	"r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15"
};

const s4 abi_registers_integer_argument[] = {
	7,  /* a0 */
	6,  /* a1 */
	2,  /* a2 */
	1,  /* a3 */
	8,  /* a4 */
	9,  /* a5 */
};

const s4 abi_registers_integer_saved[] = {
	5,  /* s0 */
	12, /* s1 */
	13, /* s2 */
	14, /* s3 */
	15, /* s4 */
};

const s4 abi_registers_integer_temporary[] = {
	3,  /* t0 */
};


/* float registers *************************************************************

   xmm0,   xmm1,   xmm2,   xmm3,   xmm4,   xmm5,   xmm6,   xmm7,
   (fa0)   (fa1)   (fa2)   (fa3)   (fa4)   (fa5)   (fa6)   (fa7)

   xmm8,   xmm9,   xmm10,  xmm11,  xmm12,  xmm13,  xmm14,  xmm15
   (ftmp1) (ftmp2) (ftmp3) (ft0)   (ft1)   (ft2)   (ft3)   (ft4)

*******************************************************************************/

s4 nregdescfloat[] = {
    REG_ARG, REG_ARG, REG_ARG, REG_ARG, REG_ARG, REG_ARG, REG_ARG, REG_ARG,
    REG_RES, REG_RES, REG_RES, REG_TMP, REG_TMP, REG_TMP, REG_TMP, REG_TMP,
    REG_END
};


const s4 abi_registers_float_argument[] = {
	0,  /* fa0 */
	1,  /* fa1 */
	2,  /* fa2 */
	3,  /* fa3 */
	4,  /* fa4 */
	5,  /* fa5 */
	6,  /* fa6 */
	7,  /* fa7 */
};

const s4 abi_registers_float_saved[] = {
	-1,
};

const s4 abi_registers_float_temporary[] = {
	11, /* ft0 */
	12, /* ft1 */
	13, /* ft2 */
	14, /* ft3 */
	15, /* ft4 */
};


/* md_param_alloc **************************************************************

   XXX

*******************************************************************************/

void md_param_alloc(methoddesc *md)
{
	paramdesc *pd;
	s4         i;
	s4         iarg;
	s4         farg;
	s4         stacksize;

	/* set default values */

	iarg      = 0;
	farg      = 0;
	stacksize = 0;

	/* get params field of methoddesc */

	pd = md->params;

	for (i = 0; i < md->paramcount; i++, pd++) {
		switch (md->paramtypes[i].type) {
		case TYPE_INT:
		case TYPE_ADR:
		case TYPE_LNG:
			if (iarg < INT_ARG_CNT) {
				pd->inmemory = false;
				pd->index    = iarg;
				pd->regoff   = abi_registers_integer_argument[iarg];
				iarg++;
			}
			else {
				pd->inmemory = true;
				pd->index    = stacksize;
				pd->regoff   = stacksize * 8;
				stacksize++;
			}
			break;

		case TYPE_FLT:
		case TYPE_DBL:
			if (farg < FLT_ARG_CNT) {
				pd->inmemory = false;
				pd->index    = farg;
				pd->regoff   = abi_registers_float_argument[farg];
				farg++;
			}
			else {
				pd->inmemory = true;
				pd->index    = stacksize;
				pd->regoff   = stacksize * 8;
				stacksize++;
			}
			break;
		default:
			assert(false);
			break;
		}
	}

	/* Since XMM0 (==A0) is used for passing return values, this
	   argument register usage has to be regarded, too. */

	if (IS_FLT_DBL_TYPE(md->returntype.type))
		if (farg < 1)
			farg = 1;

	/* fill register and stack usage */

	md->argintreguse = iarg;
	md->argfltreguse = farg;
	md->memuse       = stacksize;
}


/* md_param_alloc_native *******************************************************

   Pre-allocate arguments according the native ABI.

*******************************************************************************/

void md_param_alloc_native(methoddesc *md)
{
	/* On x86_64 we use the same ABI for JIT method calls as for
	   native method calls. */

	md_param_alloc(md);
}


/* md_return_alloc *************************************************************

   Precolor the Java Stackelement containing the Return Value. Only
   for float/ double types straight forward possible, since INT_LNG
   types use "reserved" registers Float/Double values use a00 as
   return register.

   --- in
   jd:                      jitdata of the current method
   stackslot:               Java Stackslot to contain the Return Value

   --- out
   if precoloring was possible:
   VAR(stackslot->varnum)->flags     = PREALLOC
   			             ->vv.regoff = [REG_RESULT|REG_FRESULT]
   rd->arg[flt|int]reguse   set to a value according the register usage

   NOTE: Do not pass a LOCALVAR in stackslot->varnum.

*******************************************************************************/

void md_return_alloc(jitdata *jd, stackelement_t *stackslot)
{
	methodinfo   *m;
	codeinfo     *code;
	registerdata *rd;
	methoddesc   *md;

	/* get required compiler data */

	m    = jd->m;
	code = jd->code;
	rd   = jd->rd;

	md = m->parseddesc;

	/* precoloring only straightforward possible with flt/dbl types
	   For Address/Integer/Long REG_RESULT == rax == REG_ITMP1 and so
	   could be destroyed if the return value Stack Slot "lives too
	   long" */

	if (IS_FLT_DBL_TYPE(md->returntype.type)) {
		/* In Leafmethods Local Vars holding parameters are precolored
		   to their argument register -> so leafmethods with
		   paramcount > 0 could already use a00! */

		if (!code_is_leafmethod(code) || (md->paramcount == 0)) {
			/* Only precolor the stackslot, if it is not a SAVEDVAR
			   <-> has not to survive method invokations */

			if (!(stackslot->flags & SAVEDVAR)) {

				VAR(stackslot->varnum)->flags = PREALLOC;

			    /* float/double */
				if (rd->argfltreguse < 1)
					rd->argfltreguse = 1;

				VAR(stackslot->varnum)->vv.regoff = REG_FRESULT;
			}
		}
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
