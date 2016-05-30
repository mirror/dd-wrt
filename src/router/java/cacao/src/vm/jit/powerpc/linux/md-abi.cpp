/* src/vm/jit/powerpc/linux/md-abi.cpp - functions for PowerPC Linux ABI

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

#include "vm/jit/powerpc/linux/md-abi.hpp"

#include "vm/descriptor.hpp"
#include "vm/global.hpp"
#include "vm/method.hpp"

#include "vm/jit/abi.hpp"
#include "vm/jit/code.hpp"
#include "vm/jit/codegen-common.hpp" // for PACK_REGS
#include "vm/jit/stack.hpp"


/* register descripton arrays *************************************************/

int nregdescint[] = {
	/* zero,      sp, NO(sys),   a0/v0,   a1/v1,      a2,      a3,      a4,   */
	REG_RES, REG_RES, REG_RES, REG_ARG, REG_ARG, REG_ARG, REG_ARG, REG_ARG,

	/*   a5,      a6,      a7,   itmp1,   itmp2,      pv,      s0,      s1,   */
	REG_ARG, REG_ARG, REG_ARG, REG_RES, REG_RES, REG_RES, REG_SAV, REG_SAV,

	/*itmp3,      t0,      t1,      t2,      t3,      t4,      t5,      t6,   */
	REG_RES, REG_TMP, REG_TMP, REG_TMP, REG_TMP, REG_TMP, REG_TMP, REG_TMP,

	/*   s2,      s3,      s4,      s5,      s6,      s7,      s8,      s9,   */
	REG_SAV, REG_SAV, REG_SAV, REG_SAV, REG_SAV, REG_SAV, REG_SAV, REG_SAV,

	REG_END
};

const char *abi_registers_integer_name[] = {
	"r0",  "r1",  "r2",  "r3",  "r4",  "r5",  "r6",  "r7",
	"r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15",
	"r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23",
	"r24", "r25", "r26", "r27", "r28", "r29", "r30", "r31",
};

const int abi_registers_integer_argument[] = {
	3,  /* a0 */
	4,  /* a1 */
	5,  /* a2 */
	6,  /* a3 */
	7,  /* a4 */
	8,  /* a5 */
	9,  /* a6 */
	10, /* a7 */
};

const int abi_registers_integer_saved[] = {
	14, /* s0 */
	15, /* s1 */
	24, /* s2 */
	25, /* s3 */
	26, /* s4 */
	27, /* s5 */
	28, /* s6 */
	29, /* s7 */
	30, /* s8 */
	31, /* s9 */
};

const int abi_registers_integer_temporary[] = {
	17, /* t0 */
	18, /* t1 */
	19, /* t2 */
	20, /* t3 */
	21, /* t4 */
	22, /* t5 */
	23, /* t6 */
};


int nregdescfloat[] = {
	/*ftmp3,  fa0/v0,     fa1,     fa2,     fa3,     fa4,     fa5,     fa6,   */
	REG_RES, REG_ARG, REG_ARG, REG_ARG, REG_ARG, REG_ARG, REG_ARG, REG_ARG,

	/*  fa7,     ft0,     ft1,     ft2,     ft3,     ft4,     fs0,     fs1,   */
	REG_ARG, REG_TMP, REG_TMP, REG_TMP, REG_TMP, REG_TMP, REG_SAV, REG_SAV,

	/*ftmp1,   ftmp2,     ft5,     ft6,     ft7,     ft8,     ft9,    ft10,   */
	REG_RES, REG_RES, REG_TMP, REG_TMP, REG_TMP, REG_TMP, REG_TMP, REG_TMP,

	/*  fs2,     fs3,     fs4,     fs5,     fs6,     fs7,     fs8,     fs9    */
	REG_SAV, REG_SAV, REG_SAV, REG_SAV, REG_SAV, REG_SAV, REG_SAV, REG_SAV,

	REG_END
};


const int abi_registers_float_argument[] = {
	1,  /* fa0  */
	2,  /* fa1  */
	3,  /* fa2  */
	4,  /* fa3  */
	5,  /* fa4  */
	6,  /* fa5  */
	7,  /* fa6  */
	8,  /* fa7  */
};

const int abi_registers_float_saved[] = {
	14, /* fs0  */
	15, /* fs1  */
	24, /* fs2  */
	25, /* fs3  */
	26, /* fs4  */
	27, /* fs5  */
	28, /* fs6  */
	29, /* fs7  */
	30, /* fs8  */
	31, /* fs9  */
};

const int abi_registers_float_temporary[] = {
	9,  /* ft0  */
	10, /* ft1  */
	11, /* ft2  */
	12, /* ft3  */
	13, /* ft4  */
	18, /* ft5  */
	19, /* ft6  */
	20, /* ft7  */
	21, /* ft8  */
	22, /* ft9  */
	23, /* ft10 */
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
	int        i;
	int        iarg;
	int        farg;
	int        stacksize;

	/* set default values */

	iarg = 0;
	farg = 0;
	stacksize = LA_SIZE_IN_POINTERS;

	/* get params field of methoddesc */

	pd = md->params;

	for (i = 0; i < md->paramcount; i++, pd++) {
		switch (md->paramtypes[i].type) {
		case TYPE_INT:
		case TYPE_ADR:
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

		case TYPE_LNG:
			if (iarg < INT_ARG_CNT - 1) {
				ALIGN_2(iarg);
				pd->inmemory = false;
				pd->index    = PACK_REGS(iarg + 1, iarg);
				pd->regoff   =
					PACK_REGS(abi_registers_integer_argument[iarg + 1],
							  abi_registers_integer_argument[iarg]);
				iarg += 2;
			}
			else {
				pd->inmemory = true;
				pd->index    = stacksize;
				pd->regoff   = stacksize * 8;
				iarg         = INT_ARG_CNT;
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

	/* Since R3/R4, F1 (==A0/A1, A0) are used for passing return
	   values, this argument register usage has to be regarded,
	   too. */

	if (IS_INT_LNG_TYPE(md->returntype.type)) {
		if (iarg < (IS_2_WORD_TYPE(md->returntype.type) ? 2 : 1))
			iarg = IS_2_WORD_TYPE(md->returntype.type) ? 2 : 1;
	}
	else {
		if (IS_FLT_DBL_TYPE(md->returntype.type))
			if (farg < 1)
				farg = 1;
	}

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
	paramdesc *pd;
	int        i;
	int        iarg;
	int        farg;
	int        stacksize;

	/* set default values */

	iarg = 0;
	farg = 0;
	stacksize = LA_SIZE_IN_POINTERS;

	/* get params field of methoddesc */

	pd = md->params;

	for (i = 0; i < md->paramcount; i++, pd++) {
		switch (md->paramtypes[i].type) {
		case TYPE_INT:
		case TYPE_ADR:
			if (iarg < INT_ARG_CNT) {
				pd->inmemory  = false;
				pd->regoff    = abi_registers_integer_argument[iarg];
				iarg++;
			}
			else {
				pd->inmemory  = true;
				pd->regoff    = stacksize * 4;
				stacksize++;
			}
			break;

		case TYPE_LNG:
			if (iarg < INT_ARG_CNT - 1) {
				ALIGN_2(iarg);
				pd->inmemory  = false;
				pd->regoff    =
					PACK_REGS(abi_registers_integer_argument[iarg + 1],
							  abi_registers_integer_argument[iarg]);
				iarg += 2;
			}
			else {
				ALIGN_2(stacksize);
				pd->inmemory  = true;
				pd->regoff    = stacksize * 4;
				iarg          = INT_ARG_CNT;
				stacksize    += 2;
			}
			break;

		case TYPE_FLT:
			if (farg < FLT_ARG_CNT) {
				pd->inmemory  = false;
				pd->regoff    = abi_registers_float_argument[farg];
				farg++;
			}
			else {
				pd->inmemory  = true;
				pd->regoff    = stacksize * 4;
				stacksize++;
			}
			break;

		case TYPE_DBL:
			if (farg < FLT_ARG_CNT) {
				pd->inmemory  = false;
				pd->regoff    = abi_registers_float_argument[farg];
				farg++;
			}
			else {
				ALIGN_2(stacksize);
				pd->inmemory  = true;
				pd->regoff    = stacksize * 4;
				stacksize    += 2;
			}
			break;

		default:
			assert(false);
			break;
		}
	}

	/* Since R3/R4, F1 (==A0/A1, A0) are used for passing return
	   values, this argument register usage has to be regarded,
	   too. */

	if (IS_INT_LNG_TYPE(md->returntype.type)) {
		if (iarg < (IS_2_WORD_TYPE(md->returntype.type) ? 2 : 1))
			iarg = IS_2_WORD_TYPE(md->returntype.type) ? 2 : 1;
	}
	else {
		if (IS_FLT_DBL_TYPE(md->returntype.type))
			if (farg < 1)
				farg = 1;
	}

	/* fill register and stack usage */

	md->argintreguse = iarg;
	md->argfltreguse = farg;
	md->memuse       = stacksize;
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
   VAR(stackslot->varnum)->flags       = PREALLOC
   			             ->regoff      = [REG_RESULT|REG_FRESULT]
   rd->arg[flt|int]reguse   set to a value according the register usage

   NOTE: Do not pass a LOCALVAR in stackslot->varnum.

*******************************************************************************/

void md_return_alloc(jitdata *jd, stackelement_t *stackslot)
{
	methodinfo   *m;
	codeinfo     *code;
	registerdata *rd;
	methoddesc   *md;
	varinfo      *v;

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
			v = VAR(stackslot->varnum);
			v->flags = PREALLOC;

			switch (md->returntype.type) {
			case TYPE_INT:
			case TYPE_ADR:
				if (rd->argintreguse < 1)
					rd->argintreguse = 1;

				v->vv.regoff = REG_RESULT;
				break;

			case TYPE_LNG:
				if (rd->argintreguse < 2)
					rd->argintreguse = 2;

				v->vv.regoff = REG_RESULT_PACKED;
				break;

			case TYPE_FLT:
			case TYPE_DBL:
				if (rd->argfltreguse < 1)
					rd->argfltreguse = 1;
				
				v->vv.regoff = REG_FRESULT;
				break;

			default:
				break;
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
