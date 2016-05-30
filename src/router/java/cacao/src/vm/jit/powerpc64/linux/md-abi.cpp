/* src/vm/jit/powerpc64/linux/md-abi.cpp - functions for PowerPC64 Linux ABI

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

#include "vm/types.hpp"

#include "vm/jit/powerpc64/linux/md-abi.hpp"

#include "vm/descriptor.hpp"
#include "vm/global.hpp"
#include "vm/method.hpp"

#include "vm/jit/abi.hpp"
#include "vm/jit/code.hpp"
#include "vm/jit/stack.hpp"


/* register descripton array **************************************************/

s4 nregdescint[] = {
	/* zero,      sp,     TOC,   a0/v0,   a1/v1,      a2,      a3,      a4,   */
	REG_RES, REG_RES, REG_RES, REG_ARG, REG_ARG, REG_ARG, REG_ARG, REG_ARG,

	/*   a5,      a6,      a7,   itmp1,   itmp2, NO(SYS),      pv,      s0,   */
	REG_ARG, REG_ARG, REG_ARG, REG_RES, REG_RES, REG_RES, REG_RES, REG_SAV,

	/*itmp3,      t0,      t1,      t2,      t3,      t4,      t5,      t6,   */
	REG_RES, REG_TMP, REG_TMP, REG_TMP, REG_TMP, REG_TMP, REG_TMP, REG_TMP,

	/*   s1,      s2,      s3,      s4,      s5,      s6,      s7,      s8,   */
	REG_SAV, REG_SAV, REG_SAV, REG_SAV, REG_SAV, REG_SAV, REG_SAV, REG_SAV,

	REG_END
};

const char *abi_registers_integer_name[] = {
	"r0",  "r1",  "r2",  "r3",  "r4",  "r5",  "r6",  "r7",
	"r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15",
	"r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23",
	"r24", "r25", "r26", "r27", "r28", "r29", "r30", "r31",
};

const s4 abi_registers_integer_argument[] = {
	3,  /* a0 */
	4,  /* a1 */
	5,  /* a2 */
	6,  /* a3 */
	7,  /* a4 */
	8,  /* a5 */
	9,  /* a6 */
	10, /* a7 */
};

const s4 abi_registers_integer_saved[] = {
	15, /* s0 */
	24, /* s1 */
	25, /* s2 */
	26, /* s3 */
	27, /* s4 */
	28, /* s5 */
	29, /* s6 */
	30, /* s7 */
	31, /* s8 */
};

const s4 abi_registers_integer_temporary[] = {
	17, /* t0 */
	18, /* t1 */
	19, /* t2 */
	20, /* t3 */
	21, /* t4 */
	22, /* t5 */
	23, /* t6 */
};


s4 nregdescfloat[] = {
	/*ftmp3,  fa0/v0,     fa1,     fa2,     fa3,     fa4,     fa5,     fa6,   */
	REG_RES, REG_ARG, REG_ARG, REG_ARG, REG_ARG, REG_ARG, REG_ARG, REG_ARG,

	/*  fa7,     fa8,     fa9,    fa10,    fa11,    fa12,     fs0,     fs1,   */
	REG_ARG, REG_ARG, REG_ARG, REG_ARG, REG_ARG, REG_ARG, REG_SAV, REG_SAV,

	/*  ftmp1, ftmp2,     fs2,     fs3,     fs4,     fs5,     fs6,     fs7    */
	REG_RES, REG_RES, REG_SAV, REG_SAV, REG_SAV, REG_SAV, REG_SAV, REG_SAV,

	/*  fs8,     fs9,    fs10,    fs11,    fs12,    fs13,    fs14,    fs15    */
	REG_SAV, REG_SAV, REG_SAV, REG_SAV, REG_SAV, REG_SAV, REG_SAV, REG_SAV,

	REG_END
};

const s4 abi_registers_float_argument[] = {
	1,  /* fa0  */
	2,  /* fa1  */
	3,  /* fa2  */
	4,  /* fa3  */
	5,  /* fa4  */
	6,  /* fa5  */
	7,  /* fa6  */
	8,  /* fa7  */
	9,  /* fa8  */
	10, /* fa9  */
	11, /* fa10 */
	12, /* fa11 */
	13, /* fa12 */
};

const s4 abi_registers_float_saved[] = {
	14, /* fs0  */
	15, /* fs1  */
	18, /* fs2  */
	19, /* fs3  */
	20, /* fs4  */
	21, /* fs5  */
	22, /* fs6  */
	23, /* fs7  */
	24, /* fs8  */
	25, /* fs9  */
	26, /* fs10 */
	27, /* fs11 */
	28, /* fs12 */
	29, /* fs13 */
	30, /* fs14 */
	31, /* fs15 */
};

const s4 abi_registers_float_temporary[] = {
	-1,
};


/* md_param_alloc **************************************************************

   Allocate Arguments to Stackslots according the Calling Conventions

   --- in
   md->paramcount:           Number of arguments for this method
   md->paramtypes[].type:    Argument types

   --- out
   md->params[].inmemory:    Argument spilled on stack
   md->params[].regoff:      Stack offset or rd->arg[int|flt]regs index
   md->memuse:               Stackslots needed for argument spilling
   md->argintreguse:         max number of integer arguments used
   md->argfltreguse:         max number of float arguments used

*******************************************************************************/

void md_param_alloc(methoddesc *md)
{
	paramdesc *pd;
	s4         i;
	s4         iarg;
	s4         farg;
	s4         arg;
	s4         stacksize, stackcount;


	/* set default values */

	iarg       = 0;
	farg       = 0;
	arg        = 0;
	stacksize  = LA_SIZE_IN_POINTERS;
	stackcount = 0;

	/* get params field of methoddesc */

	pd = md->params;

	for (i = 0; i < md->paramcount; i++, pd++) {
		switch (md->paramtypes[i].type) {
		case TYPE_LNG:
		case TYPE_INT:
		case TYPE_ADR:
			if (iarg < INT_ARG_CNT) {
				pd->inmemory = false;
				pd->index = iarg;
				pd->regoff   = abi_registers_integer_argument[iarg];
				iarg++;
			}
			else {
				pd->inmemory = true;
				pd->index = stacksize + stackcount;
				pd->regoff   = (stacksize + stackcount) * 8;
			}
			break;
		case TYPE_FLT:
		case TYPE_DBL:
			if (farg < FLT_ARG_CNT) {
				pd->inmemory = false;
				pd->index = farg;
				pd->regoff   = abi_registers_float_argument[farg];
				farg++;
				if (arg < INT_ARG_CNT) {
					iarg++;		/* yes, that is true, floating arguments take int register slots away */
				}
			}
			else {
				pd->inmemory = true;
				pd->index = stacksize + stackcount;
				pd->regoff   = (stacksize + stackcount) * 8;
			}
			break;
		default:
			assert(0);
		}
		arg++;
		stackcount++;
	}

	/* Since R3, F1 (==A0, A0) are used for passing return values, this */
	/* argument register usage has to be regarded, too                        */
	if (IS_INT_LNG_TYPE(md->returntype.type)) {
		if (iarg < 1)
			iarg = 1;
	}
	else if (IS_FLT_DBL_TYPE(md->returntype.type)) {
		if (farg < 1)
			farg = 1;
	}

	/* fill register and stack usage, parameter areas is at least PA_SIZE_IN_POINTERS */

	md->argintreguse = iarg;
	md->argfltreguse = farg;
	md->memuse = stacksize + (stackcount<PA_SIZE_IN_POINTERS? PA_SIZE_IN_POINTERS: stackcount);	
}


/* md_param_alloc_native *******************************************************

   Pre-allocate arguments according the native ABI.

*******************************************************************************/

void md_param_alloc_native(methoddesc *md)
{
	/* On PowerPC64 we use the same ABI for JIT method calls as for
	   native method calls. */

	md_param_alloc(md);
}


/* md_return_alloc *************************************************************

   Precolor the Java Stackelement containing the Return Value, if
   possible.  (R3==a00 for int/adr, R4/R3 == a01/a00 for long, F1==a00
   for float/double)

   --- in
   jd:                      jitdata of the current method
   stackslot:               Java Stackslot to contain the Return Value

   --- out
   if precoloring was possible:
   VAR(stackslot->varnum)->flags     = PREALLOC
   VAR(stackslot->varnum)->vv.regoff = [REG_RESULT, REG_FRESULT]
   rd->arg[flt|int]reguse   set to a value according the register usage

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
	
	/* In Leafmethods Local Vars holding parameters are precolored to
	   their argument register -> so leafmethods with paramcount > 0
	   could already use R3 == a00! */

	if (!code_is_leafmethod(code) || (md->paramcount == 0)) {
		/* Only precolor the stackslot, if it is not a SAVEDVAR <->
		   has not to survive method invokations. */

		if (!(stackslot->flags & SAVEDVAR)) {

			VAR(stackslot->varnum)->flags = PREALLOC;

			if (IS_INT_LNG_TYPE(md->returntype.type)) {
				if (rd->argintreguse < 1)
					rd->argintreguse = 1;

				VAR(stackslot->varnum)->vv.regoff = REG_RESULT;
			} else { /* float/double */
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
