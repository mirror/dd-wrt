/* src/vm/jit/s390/md-abi.c - s390 Linux ABI

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

#include "vm/descriptor.hpp"
#include "vm/global.hpp"
#include "vm/types.hpp"

#include "vm/jit/jit.hpp"
#include "vm/jit/stack.hpp"

#include "vm/jit/s390/md-abi.h"

#include <assert.h>


/* register descripton array **************************************************/

s4 nregdescint[] = {
	/*itmp3,   itmp1,      a0,      a1,      a2,      a3,      a4,      s0, */
	REG_RES, REG_RES, REG_ARG, REG_ARG, REG_ARG, REG_ARG, REG_ARG, REG_SAV,
	/*   s1,      s2,      s3,      s4,      s5,    pv,  ra/itmp2,      sp */
  	REG_SAV, REG_SAV, REG_SAV, REG_SAV, REG_SAV, REG_RES, REG_RES, REG_RES,
    REG_END
};

const char *abi_registers_integer_name[] = {
	"r0", "r1", "r2", "r3",
	"r4", "r5", "r6", "r7",
	"r8", "r9", "r10", "r11",
	"r12", "r13", "r14", "r15"
};

const s4 abi_registers_integer_argument[] = {
	2, /* r2/a0 */
	3, /* r3/a1 */
	4, /* r4/a2 */
	5, /* r5/a3 */
	6  /* r6/a4 */
};

const s4 abi_registers_integer_saved[] = {
	7,  /* r7/s0 */
	8,  /* r8/s1 */
	9,  /* r9/s2 */
	10, /* r10/s3 */
	11, /* r11/s4 */
	12  /* r12/s5 */
};

const s4 abi_registers_integer_temporary[] = {
	-1 /* none */
};

s4 nregdescfloat[] = {
	REG_ARG, REG_TMP, REG_ARG, REG_TMP, REG_RES, REG_TMP, REG_RES, REG_TMP,
	REG_TMP, REG_TMP, REG_TMP, REG_TMP, REG_TMP, REG_TMP, REG_TMP, REG_TMP,
    REG_END
};

const s4 abi_registers_float_argument[] = {
	0, /* f0/fa0 */
	2  /* f2/fa2 */
};

const s4 abi_registers_float_saved[] = {
	-1 /* none */
};

const s4 abi_registers_float_temporary[] = {
	1,  /* f1/ft0 */
	3,  /* f3/ft1 */
	5,  /* f5/ft2 */
	7,  /* f7/ft3 */
	8,  /* f8/ft4 */
	9,  /* f9/ft5 */
	10, /* f10/ft6 */
	11, /* f11/ft7 */
	12, /* f12/ft8 */
	13, /* f13/ft9 */
	14, /* f14/ft10 */
	15  /* f15/ft11 */
};

/* md_param_alloc_intern *******************************************************

   Allocates parameters to registers or stackslots for both native and java
   methods.

   --- in:
   slot: size in bytes of a stack slot
   slots1w: number of stack slots used by a 1 word type parameter
   slots2w: number of stack slots used by a 2 word type parameter
   stackoff: offset on stack frame where to start placing arguments

*******************************************************************************/

static void md_param_alloc_intern(methoddesc *md, s4 slot, s4 slots1w, s4 slots2w, s4 stackoff)
{
	paramdesc *pd;
	s4         i;
	s4         iarg;
	s4         farg;
	s4         stacksize;

	/* set default values */

	iarg = 0;
	farg = 0;
	stacksize = 0;

	/* get params field of methoddesc */

	pd = md->params;

	for (i = 0; i < md->paramcount; i++, pd++) {
		switch (md->paramtypes[i].type) {
		case TYPE_INT:
		case TYPE_ADR:
			if (iarg < INT_ARG_CNT) {
				pd->inmemory  = false;
 				pd->regoff    = abi_registers_integer_argument[iarg]; 
				pd->index     = iarg;
				iarg++;
			}
			else {
				pd->inmemory  = true;
				pd->regoff    = (stacksize * slot) + stackoff;
				pd->index     = stacksize;
				stacksize += slots1w;
			}
			break;

		case TYPE_LNG:
			if (iarg < INT_ARG_CNT - 1) {
				/* _ALIGN(iarg); */
				pd->inmemory  = false;
 				pd->regoff    = 
 					PACK_REGS(abi_registers_integer_argument[iarg + 1], 
 							  abi_registers_integer_argument[iarg]); 
				pd->index     = PACK_REGS(iarg + 1, iarg);
				iarg += 2;
			}
			else {
				/* _ALIGN(stacksize); */
				pd->inmemory  = true;
				pd->regoff    = (stacksize * slot) + stackoff;
				pd->index     = stacksize;
				iarg          = INT_ARG_CNT;
				stacksize    += slots2w;
			}
			break;

		case TYPE_FLT:
			if (farg < FLT_ARG_CNT) {
				pd->inmemory  = false;
 				pd->regoff    = abi_registers_float_argument[farg]; 
				pd->index     = farg;
				farg++;
			}
			else {
				pd->inmemory  = true;
				pd->regoff    = (stacksize * slot) + stackoff;
				pd->index     = stacksize;
				stacksize += slots1w;
			}
			break;

		case TYPE_DBL:
			if (farg < FLT_ARG_CNT) {
				pd->inmemory  = false;
 				pd->regoff    = abi_registers_float_argument[farg]; 
				pd->index     = farg;
				farg++;
			}
			else {
				/* _ALIGN(stacksize); */
				pd->inmemory  = true;
				pd->regoff    = (stacksize * slot) + stackoff;
				pd->index     = stacksize;
				stacksize    += slots2w;
			}
			break;

		default:
			assert(0);
		}
	}

	/* Since A0+A1/FA0 are used for passing return
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
	md->memuse = stacksize;
}

void md_param_alloc(methoddesc *md)
{
	md_param_alloc_intern(md, 8, 1, 1, 0);
}

void md_param_alloc_native(methoddesc *md)
{
	md_param_alloc_intern(md, 4, 1, 2, 96);
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

void md_return_alloc(jitdata *jd, stackelement_t* stackslot)
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
				if (!IS_2_WORD_TYPE(md->returntype.type)) {
					if (rd->argintreguse < 1)
						rd->argintreguse = 1;

					VAR(stackslot->varnum)->vv.regoff = REG_RESULT;
				}
				else {
					if (rd->argintreguse < 2)
						rd->argintreguse = 2;

					VAR(stackslot->varnum)->vv.regoff = REG_RESULT_PACKED;
				}
			}
			else { /* float/double */
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
 * mode: c
 * indent-tabs-mode: t
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim:noexpandtab:sw=4:ts=4:
 */
