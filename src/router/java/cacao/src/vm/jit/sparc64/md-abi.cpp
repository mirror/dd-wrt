/* src/vm/jit/sparc64/md-abi.cpp - functions for Sparc ABI

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

#include "vm/jit/sparc64/md-abi.hpp"

#include "vm/descriptor.hpp"
#include "vm/global.hpp"
#include "vm/method.hpp"

#include "vm/jit/abi.hpp"
#include "vm/jit/code.hpp"
#include "vm/jit/stack.hpp"

/* temp */
#include "mm/memory.hpp"
#include <assert.h>


/* register descripton array **************************************************/

/* callee point-of-view, after SAVE has been called. */
s4 nregdescint[] = {
	/* zero  itmp1/g1 itmp2/g2 itmp3/g3 temp/g4  temp/g5  sys/g6   sys/g7 */  
	REG_RES, REG_RES, REG_RES, REG_RES, REG_RES, REG_RES, REG_RES, REG_RES,
	
	/* o0    o1       o2       o3       o4       pv/o5    sp/o6    ra/o7  */
	REG_ARG, REG_ARG, REG_ARG, REG_ARG, REG_ARG, REG_RES, REG_RES, REG_RES,
	
	/* l0    l1       l2       l3       l4       l5       l6       l7     */
	REG_SAV, REG_SAV, REG_SAV, REG_SAV, REG_SAV, REG_SAV, REG_SAV, REG_SAV,
	
	/* i0    i1       i2       i3       i4       pv/i5    fp/i6    ra/i7  */
	REG_SAV, REG_SAV, REG_SAV, REG_SAV, REG_SAV, REG_RES, REG_RES, REG_RES,
	REG_END
};

const char *abi_registers_integer_name[] = {
		"zero",  "g1",  "g2",  "g3",  "g4",  "g5",  "g6",  "g7",
		"o0",    "o1",  "o2",  "o3",  "o4",  "o5",  "sp",  "o7",
		"l0",    "l1",  "l2",  "l3",  "l4",  "l5",  "l6",  "l7",
		"i0",    "i1",  "i2",  "i3",  "i4",  "i5",  "fp",  "i7"
};

const s4 abi_registers_integer_argument[] = {
	8,  /* o0  */
	9,  /* o1  */
	10, /* o2  */
	11, /* o3  */
	12, /* o4  */
};

const s4 abi_registers_integer_saved[] = {
	16, /* l0  */
	17, /* l1  */
	18, /* l2  */
	19, /* l3  */
	20, /* l4  */
	21, /* l5  */
	22, /* l6  */
	23, /* l7  */
	24, /* i0  */
	25, /* i1  */
	26, /* i2  */
	27, /* i3  */
	28, /* i4  */
};



s4 nregdescfloat[] = {
	REG_RET, REG_RES, REG_RES, REG_RES, REG_TMP, REG_TMP, REG_TMP, REG_TMP,
	REG_ARG, REG_ARG, REG_ARG, REG_ARG, REG_ARG, REG_TMP, REG_TMP, REG_TMP,
	REG_END
};


const s4 abi_registers_float_argument[] = {
	8,  /* f16  */
	9,  /* f18  */
	10, /* f20  */
	11, /* f22  */
	12, /* f24  */
};

const s4 abi_registers_float_temporary[] = {
	4,  /* f8   */
	5,  /* f10  */
	6,  /* f12  */
	7,  /* f14  */
	13, /* f26  */
	14, /* f28  */
	15, /* f30  */
};

s4 nat_argintregs[] = {
	REG_OUT0, REG_OUT1, REG_OUT2, REG_OUT3, REG_OUT4, REG_OUT5
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
	s4         reguse;
	s4         stacksize;

	/* set default values */

	reguse = 0;
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
				pd->index = reguse;
				pd->regoff   = abi_registers_integer_argument[reguse];
				reguse++;
				md->argintreguse = reguse;
			}
			else {
				pd->inmemory = true;
				pd->index = stacksize;
				pd->regoff = stacksize * 8;
				stacksize++;
			}
			break;

		case TYPE_FLT:
		case TYPE_DBL:
			if (i < FLT_ARG_CNT) {
				pd->inmemory = false;
				pd->index = reguse;
				pd->regoff   = abi_registers_float_argument[reguse];
				reguse++;
				md->argfltreguse = reguse;
			}
			else {
				pd->inmemory = true;
				pd->index = stacksize;
				pd->regoff = stacksize * 8;
				stacksize++;
			}
			break;
		}
		assert(pd->regoff != 26);
	}
	
	/* Since O0 is used for passing return values, this */
	/* argument register usage has to be regarded, too                        */
	if (IS_INT_LNG_TYPE(md->returntype.type)) {
		if (reguse < 1)
			md->argintreguse = 1;
	}

	/* fill register and stack usage */

	md->memuse = stacksize;
}


/* md_param_alloc_native *******************************************************
 *
 *    Pre-allocate arguments according to the native ABI.
 *
 *    *******************************************************************************/

void md_param_alloc_native(methoddesc *md)
{
	paramdesc *pd;
	s4         i;
	s4         reguse;
	s4         stacksize;
	s4         min_nat_regs;

	/* Note: regoff will be set relative to a stack base of $sp+16 */
	 
	/* set default values */
	reguse = 0;
	stacksize = 6; /* abi params: allocated, but not used */

	/* when we are above this, we have to increase the stacksize with every */
	/* single argument to create the proper argument array                  */
	min_nat_regs = MIN(INT_NATARG_CNT, FLT_NATARG_CNT);

	/* get params field of methoddesc */

	pd = md->params;

	for (i = 0; i < md->paramcount; i++, pd++) {
		switch (md->paramtypes[i].type) {
		case TYPE_INT:
		case TYPE_ADR:
		case TYPE_LNG:
			if (i < INT_NATARG_CNT) {
				pd->inmemory = false;
				pd->regoff = nat_argintregs[reguse];
				reguse++;
				md->argintreguse = reguse;

			} else {
				pd->inmemory = true;
				pd->regoff = reguse * 8;
				reguse++;
			}
			break;
		case TYPE_FLT:
		case TYPE_DBL:
			if (i < FLT_NATARG_CNT) {
				pd->inmemory = false;
				pd->regoff = reguse;
				reguse++;
				md->argfltreguse = reguse;
			} else {
				pd->inmemory = true;
				pd->regoff = reguse * 8;
				reguse++;
			}

			break;
		}

		if (i >= min_nat_regs)
			stacksize++;
	}
	
	/* Since O0 is used for passing return values, this */
	/* argument register usage has to be regarded, too                        */
	if (IS_INT_LNG_TYPE(md->returntype.type)) {
		if (reguse < 1)
			md->argintreguse = 1;
	}

	/* fill register and stack usage */

	md->memuse = stacksize;
}

/* md_return_alloc *************************************************************

  XXX

*******************************************************************************/

void md_return_alloc(jitdata *jd, stackelement_t* stackslot)
{
	/* XXX */
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
