/* src/vm/jit/alpha/md-abi.cpp - functions for Alpha ABI

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

#include "vm/jit/alpha/md-abi.hpp"

#include "vm/descriptor.hpp"
#include "vm/global.hpp"
#include "vm/method.hpp"

#include "vm/jit/abi.hpp"
#include "vm/jit/code.hpp"


/* register descripton array **************************************************/

s4 nregdescint[] = {
	/*   v0,      t0,      t1,      t2,      t3,      t4,      t5,      t6,   */
	REG_RET, REG_TMP, REG_TMP, REG_TMP, REG_TMP, REG_TMP, REG_TMP, REG_TMP, 

	/*   t7,      s0,      s1,      s2,      s3,      s4,      s5,      s6,   */
	REG_TMP, REG_SAV, REG_SAV, REG_SAV, REG_SAV, REG_SAV, REG_SAV, REG_SAV, 

	/*   a0,      a1,      a2,      a3,      a4,      a5,      t8,      t9,   */
	REG_ARG, REG_ARG, REG_ARG, REG_ARG, REG_ARG, REG_ARG, REG_TMP, REG_TMP,

	/*  t10,   itmp1,      ra,      pv,      at,   itmp3,      sp,    zero,   */
	REG_TMP, REG_RES, REG_RES, REG_RES, REG_RES, REG_RES, REG_RES, REG_RES,

	REG_END
};

const char *abi_registers_integer_name[] = {
	"v0",  "t0",  "t1",  "t2",  "t3",  "t4",  "t5",  "t6",
	"t7",  "s0",  "s1",  "s2",  "s3",  "s4",  "s5",  "s6",
	"a0",  "a1",  "a2",  "a3",  "a4",  "a5",  "t8",  "t9",
	"t10", "t11", "ra",  "pv",  "at",  "gp",  "sp",  "zero"
};

const s4 abi_registers_integer_argument[] = {
	16, /* a0  */
	17, /* a1  */
	18, /* a2  */
	19, /* a3  */
	20, /* a4  */
	21, /* a5  */
};

const s4 abi_registers_integer_saved[] = {
	9,  /* s0  */
	10, /* s1  */
	11, /* s2  */
	12, /* s3  */
	13, /* s4  */
	14, /* s5  */
	15, /* s6  */
};

const s4 abi_registers_integer_temporary[] = {
	1,  /* t0  */
	2,  /* t1  */
	3,  /* t2  */
	4,  /* t3  */
	5,  /* t4  */
	6,  /* t5  */
	7,  /* t6  */
	8,  /* t7  */
	22, /* t8  */
	23, /* t9  */
	24, /* t10 */
};


s4 nregdescfloat[] = {
	REG_RET, REG_TMP, REG_SAV, REG_SAV, REG_SAV, REG_SAV, REG_SAV, REG_SAV,
	REG_SAV, REG_SAV, REG_TMP, REG_TMP, REG_TMP, REG_TMP, REG_TMP, REG_TMP, 
	REG_ARG, REG_ARG, REG_ARG, REG_ARG, REG_ARG, REG_ARG, REG_TMP, REG_TMP,
	REG_TMP, REG_TMP, REG_TMP, REG_TMP, REG_RES, REG_RES, REG_RES, REG_RES,
	REG_END
};


const s4 abi_registers_float_argument[] = {
	16, /* fa0  */
	17, /* fa1  */
	18, /* fa2  */
	19, /* fa3  */
	20, /* fa4  */
	21, /* fa5  */
};

const s4 abi_registers_float_saved[] = {
	2,  /* fs0  */
	3,  /* fs1  */
	4,  /* fs2  */
	5,  /* fs3  */
	6,  /* fs4  */
	7,  /* fs5  */
	8,  /* fs6  */
	9,  /* fs7  */
};

const s4 abi_registers_float_temporary[] = {
	1,  /* ft0  */
	10, /* ft1  */
	11, /* ft2  */
	12, /* ft3  */
	13, /* ft4  */
	14, /* ft5  */
	15, /* ft6  */
	22, /* ft7  */
	23, /* ft8  */
	24, /* ft9  */
	25, /* ft10 */
	26, /* ft11 */
	27, /* ft12 */
};


/* md_param_alloc **************************************************************

   Allocate the parameters of the given method descriptor according to the
   calling convention of the platform.

*******************************************************************************/

void md_param_alloc(methoddesc *md)
{
	paramdesc *pd;
	s4         i;
	s4         reguse;
	s4         stacksize;

	/* set default values */

	reguse    = 0;
	stacksize = 0;

	/* get params field of methoddesc */

	pd = md->params;

	for (i = 0; i < md->paramcount; i++, pd++) {
		switch (md->paramtypes[i].type) {
		case TYPE_INT:
		case TYPE_ADR:
		case TYPE_LNG:
			if (i < INT_ARG_CNT) {
				pd->inmemory = false;
				pd->index    = reguse;
				pd->regoff   = abi_registers_integer_argument[reguse];
				reguse++;
				md->argintreguse = reguse;
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
			if (i < FLT_ARG_CNT) {
				pd->inmemory = false;
				pd->index    = reguse;
				pd->regoff   = abi_registers_float_argument[reguse];
				reguse++;
				md->argfltreguse = reguse;
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

	/* fill register and stack usage */

	md->memuse = stacksize;
}


/* md_param_alloc_native *******************************************************

   Pre-allocate arguments according to the native ABI.

*******************************************************************************/

void md_param_alloc_native(methoddesc *md)
{
	/* On Alpha we use the same ABI for JIT method calls as for native
	   method calls. */

	md_param_alloc(md);
}


/* md_return_alloc *************************************************************

   Precolor the Java Stackelement containing the Return Value. Since
   alpha has a dedicated return register (not an reused arg or
   reserved reg), this is striaghtforward possible, as long, as this
   stackelement does not have to survive a method invokation
   (SAVEDVAR)

   --- in
   jd:                      jitdata of the current method
   stackslot:               Java Stackslot to contain the Return Value
   
   --- out
   if precoloring was possible:
   VAR(stackslot->varnum)->flags       = PREALLOC
   			             ->vv.regoff   = [REG_RESULT|REG_FRESULT]
   rd->arg[flt|int]reguse   set to a value according the register usage

   NOTE: Do not pass a LOCALVAR in stackslot->varnum.
*******************************************************************************/

void md_return_alloc(jitdata *jd, stackelement_t *stackslot)
{
	methodinfo *m;
	methoddesc *md;

	/* get required compiler data */

	m = jd->m;

	md = m->parseddesc;

	/* Only precolor the stackslot, if it is not a SAVEDVAR <-> has
	   not to survive method invokations. */

	if (!(stackslot->flags & SAVEDVAR)) {

		VAR(stackslot->varnum)->flags = PREALLOC;

		if (IS_INT_LNG_TYPE(md->returntype.type))
			VAR(stackslot->varnum)->vv.regoff = REG_RESULT;
		else
			VAR(stackslot->varnum)->vv.regoff = REG_FRESULT;
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
