/* src/vm/jit/mips/md-abi.cpp - functions for MIPS ABI

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

#include <stdarg.h>
#include <stdint.h>

#include "vm/types.hpp"

#include "vm/jit/mips/md-abi.hpp"

#include "mm/memory.hpp"

#include "vm/descriptor.hpp"
#include "vm/global.hpp"
#include "vm/method.hpp"

#include "vm/jit/abi.hpp"
#include "vm/jit/code.hpp"
#include "vm/jit/codegen-common.hpp" // for PACK_REGS
#include "vm/jit/stack.hpp"


/* register descripton array **************************************************/

#if SIZEOF_VOID_P == 8

/* MIPS64 */

s4 nregdescint[] = {
	REG_RES, REG_RES, REG_RET, REG_RES, REG_ARG, REG_ARG, REG_ARG, REG_ARG,
	REG_ARG, REG_ARG, REG_ARG, REG_ARG, REG_TMP, REG_TMP, REG_TMP, REG_TMP,
	REG_SAV, REG_SAV, REG_SAV, REG_SAV, REG_SAV, REG_SAV, REG_SAV, REG_SAV,
	REG_TMP, REG_RES, REG_RES, REG_RES, REG_RES, REG_RES, REG_RES, REG_RES,
	REG_END
};

const char *abi_registers_integer_name[] = {
	"zero",  "at",    "v0",    "v1",    "a0",    "a1",    "a2",    "a3",
	"a4",    "a5",    "a6",    "a7",    "t0",    "t1",    "t2",    "t3",
	"s0",    "s1",    "s2",    "s3",    "s4",    "s5",    "s6",    "s7",
	"t8",    "t9",    "k0",    "k1",    "gp",    "sp",    "s8",    "ra"
};

const s4 abi_registers_integer_argument[] = {
	4,  /* a0  */
	5,  /* a1  */
	6,  /* a2  */
	7,  /* a3  */
	8,  /* a4  */
	9,  /* a5  */
	10, /* a6  */
	11, /* a7  */
};

const s4 abi_registers_integer_saved[] = {
	16, /* s0  */
	17, /* s1  */
	18, /* s2  */
	19, /* s3  */
	20, /* s4  */
	21, /* s5  */
	22, /* s6  */
	23, /* s7  */
};

const s4 abi_registers_integer_temporary[] = {
	12, /* t0  */
	13, /* t1  */
	14, /* t2  */
	15, /* t3  */
	24, /* t4  */
};


s4 nregdescfloat[] = {
	/*  fv0,   ftmp1,   ftmp2,   ftmp3,     ft0,     ft1,     ft2,     ft3,   */
	REG_RET, REG_RES, REG_RES, REG_RES, REG_TMP, REG_TMP, REG_TMP, REG_TMP,

	/*  ft4,     ft5,     ft6,     ft7,     fa0,     fa1,     fa2,     fa3,   */
	REG_TMP, REG_TMP, REG_TMP, REG_TMP, REG_ARG, REG_ARG, REG_ARG, REG_ARG,

	/*  fa4,     fa5,     fa6,     fa7,     ft8,     ft9,    ft10,    ft11,   */
	REG_ARG, REG_ARG, REG_ARG, REG_ARG, REG_TMP, REG_TMP, REG_TMP, REG_TMP,

	/*  fs0,    ft12,     fs1,    ft13,     fs2,    ft14,     fs3     ft15    */
	REG_SAV, REG_TMP, REG_SAV, REG_TMP, REG_SAV, REG_TMP, REG_SAV, REG_TMP,

	REG_END
};

const s4 abi_registers_float_argument[] = {
	12, /* fa0  */
	13, /* fa1  */
	14, /* fa2  */
	15, /* fa3  */
	16, /* fa4  */
	17, /* fa5  */
	18, /* fa6  */
	19, /* fa7  */
};

const s4 abi_registers_float_saved[] = {
	24, /* fs0  */
	26, /* fs1  */
	28, /* fs2  */
	30, /* fs3  */
};

const s4 abi_registers_float_temporary[] = {
	4,  /* ft0  */
	5,  /* ft1  */
	6,  /* ft2  */
	7,  /* ft3  */
	8,  /* ft4  */
	9,  /* ft5  */
	10, /* ft6  */
	11, /* ft7  */
	20, /* ft8  */
	21, /* ft9  */
	22, /* ft10 */
	23, /* ft11 */
	25, /* ft12 */
	27, /* ft13 */
	29, /* ft14 */
	31, /* ft15 */
};

#else /* SIZEOF_VOID_P == 8 */

/* MIPS32 */

s4 nregdescint[] = {
	/* zero,   itmp1,      v0,      v1,      a0,      a1,      a2,      a3,   */
	REG_RES, REG_RES, REG_RET, REG_RES, REG_ARG, REG_ARG, REG_ARG, REG_ARG,

	/*   t0,      t1,      t2,      t3,      t4,      t5,      t6,      t7,   */
	REG_TMP, REG_TMP, REG_TMP, REG_TMP, REG_TMP, REG_TMP, REG_TMP, REG_TMP,

	/*   s0,      s1,      s2,      s3,      s4,      s5,      s6,      s7,   */
	REG_SAV, REG_SAV, REG_SAV, REG_SAV, REG_SAV, REG_SAV, REG_SAV, REG_SAV,

	/*itmp2,   itmp3, k0(sys), k1(sys),      gp,      sp,      pv,      ra    */
	REG_RES, REG_RES, REG_RES, REG_RES, REG_RES, REG_RES, REG_RES, REG_RES,

	REG_END
};

const char *abi_registers_integer_name[] = {
	"zero",  "at",    "v0",    "v1",    "a0",    "a1",    "a2",    "a3",
	"a4",    "a5",    "a6",    "a7",    "t0",    "t1",    "t2",    "t3",
	"s0",    "s1",    "s2",    "s3",    "s4",    "s5",    "s6",    "s7",
	"t8",    "t9",    "k0",    "k1",    "gp",    "sp",    "s8",    "ra"
};

const s4 abi_registers_integer_argument[] = {
	4,  /* a0  */
	5,  /* a1  */
	6,  /* a2  */
	7,  /* a3  */
};

const s4 abi_registers_integer_saved[] = {
	16, /* s0  */
	17, /* s1  */
	18, /* s2  */
	19, /* s3  */
	20, /* s4  */
	21, /* s5  */
	22, /* s6  */
	23, /* s7  */
};

const s4 abi_registers_integer_temporary[] = {
	8,  /* t0  */
	9,  /* t1  */
	10, /* t2  */
	11, /* t3  */
	12, /* t4  */
	13, /* t5  */
	14, /* t6  */
	15, /* t7  */
};


#if !defined(ENABLE_SOFT_FLOAT)

s4 nregdescfloat[] = {
	/*  fv0,            ftmp1,            ftmp2,            ftmp3,            */
	REG_RET, REG_RES, REG_RES, REG_RES, REG_RES, REG_RES, REG_RES, REG_RES,

	/*  ft0,              ft1,              fa0,              fa1,            */
	REG_TMP, REG_RES, REG_TMP, REG_RES, REG_ARG, REG_RES, REG_ARG, REG_RES,

	/*  ft2,              ft3,              fs0,              fs1,            */
	REG_TMP, REG_RES, REG_TMP, REG_RES, REG_SAV, REG_RES, REG_SAV, REG_RES,

	/*  fs2,              fs3,              fs4,              fs5             */
	REG_SAV, REG_RES, REG_SAV, REG_RES, REG_SAV, REG_RES, REG_SAV, REG_RES,

	REG_END
};

const s4 abi_registers_float_argument[] = {
	12, /* fa0  */
	14, /* fa1  */
};

const s4 abi_registers_float_saved[] = {
	20, /* fs0  */
	22, /* fs1  */
	24, /* fs2  */
	26, /* fs3  */
	28, /* fs4  */
	30, /* fs5  */
};

const s4 abi_registers_float_temporary[] = {
	8,  /* ft0  */
	10, /* ft1  */
	16, /* ft2  */
	18, /* ft3  */
};


#else /* !defined(ENABLE_SOFT_FLOAT) */

s4 nregdescfloat[] = {
	REG_END
};

#endif /* !defined(ENABLE_SOFT_FLOAT) */

#endif /* SIZEOF_VOID_P == 8 */


/* md_param_alloc **************************************************************

   Pre-allocate arguments according to the internal JIT ABI.

*******************************************************************************/

void md_param_alloc(methoddesc *md)
{
	paramdesc *pd;
	s4         i;
	s4         reguse;
	s4         stacksize;

	/* set default values */

	reguse      = 0;
	stacksize   = 0;

	/* get params field of methoddesc */

	pd = md->params;

#if SIZEOF_VOID_P == 8

	for (i = 0; i < md->paramcount; i++, pd++) {
		switch (md->paramtypes[i].type) {
		case TYPE_INT:
		case TYPE_ADR:
		case TYPE_LNG:
			if (reguse < INT_ARG_CNT) {
				pd->inmemory = false;
				pd->regoff   = abi_registers_integer_argument[reguse];
				reguse++;
				md->argintreguse = reguse;
			}
			else {
				pd->inmemory = true;
				pd->regoff   = stacksize * 8;
				stacksize++;
			}
			break;

		case TYPE_FLT:
		case TYPE_DBL:
			if (reguse < FLT_ARG_CNT) {
				pd->inmemory = false;
				pd->regoff   = abi_registers_float_argument[reguse];
				reguse++;
				md->argfltreguse = reguse;
			}
			else {
				pd->inmemory = true;
				pd->regoff   = stacksize * 8;
				stacksize++;
			}
			break;
		}

		/* register type is the same as java type */

		pd->type = md->paramtypes[i].type;
	}

#else /* SIZEOF_VOID_P == 8 */

# if !defined(ENABLE_SOFT_FLOAT)

	/* Set stacksize to 2, as 4 32-bit argument registers can be
	   stored. */
	/* XXX maybe this should be done in stack.c? */

	stacksize = 2;

	for (i = 0; i < md->paramcount; i++, pd++) {
		switch (md->paramtypes[i].type) {
		case TYPE_INT:
		case TYPE_ADR:
			if (reguse < INT_ARG_CNT) {
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

		case TYPE_LNG:
			ALIGN_2(reguse);

			if (reguse < INT_ARG_CNT) {
				pd->inmemory = false;
#  if WORDS_BIGENDIAN == 1
				pd->index    = PACK_REGS(reguse + 1, reguse);
				pd->regoff   =
					PACK_REGS(abi_registers_integer_argument[reguse + 1],
							  abi_registers_integer_argument[reguse]);
#  else
				pd->index    = PACK_REGS(reguse, reguse + 1);
				pd->regoff   =
					PACK_REGS(abi_registers_integer_argument[reguse],
							  abi_registers_integer_argument[reguse + 1]);
#  endif
				reguse += 2;
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
			if (reguse < FLT_ARG_CNT) {
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
		}

		/* register type is the same as java type */

		pd->type = md->paramtypes[i].type;
	}

# else /* !defined(ENABLE_SOFT_FLOAT) */
#  error never actually tested!

	for (i = 0; i < md->paramcount; i++, pd++) {
		switch (md->paramtypes[i].type) {
		case TYPE_INT:
		case TYPE_ADR:
		case TYPE_FLT:
			pd->type = TYPE_INT;

			if (reguse < INT_ARG_CNT) {
				pd->inmemory = false;
				pd->regoff   = abi_registers_integer_argument[reguse];
				reguse++;
				md->argintreguse = reguse;
			}
			else {
				pd->inmemory = true;
				pd->regoff   = stacksize * 8;
			}
			stacksize++;
			break;

		case TYPE_LNG:
		case TYPE_DBL:
			pd->type = TYPE_LNG;

			if (reguse < INT_ARG_CNT) {
				pd->inmemory = false;
#  if WORDS_BIGENDIAN == 1
				pd->regoff   =
					PACK_REGS(abi_registers_integer_argument[reguse + 1],
							  abi_registers_integer_argument[reguse]);
#  else
				pd->regoff   =
					PACK_REGS(abi_registers_integer_argument[reguse],
							  abi_registers_integer_argument[reguse + 1]);
#  endif
				reguse += 2;
				md->argintreguse = reguse;
			}
			else {
				pd->inmemory = true;
				pd->regoff   = stacksize * 8;
			}
			stacksize += 2;
			break;
		}
	}

# endif /* !defined(ENABLE_SOFT_FLOAT) */
#endif /* SIZEOF_VOID_P == 8 */

	/* fill register and stack usage */

	md->memuse = stacksize;
}


/* md_param_alloc_native *******************************************************

   Pre-allocate arguments according the native ABI.

*******************************************************************************/

void md_param_alloc_native(methoddesc *md)
{
#if SIZEOF_VOID_P == 8

	/* On MIPS n64 we use the same ABI for JIT method calls as for
	   native method calls. */

	md_param_alloc(md);

#else /* SIZEOF_VOID_P == 8 */

	paramdesc *pd;
	s4         i;
	s4         reguse;
	s4         stacksize;
# if !defined(ENABLE_SOFT_FLOAT)
	s4         t;
	bool       a0_is_float;
# endif

	/* set default values */

	reguse      = 0;
	stacksize   = 0;
# if !defined(ENABLE_SOFT_FLOAT)
	a0_is_float = false;
# endif

	/* get params field of methoddesc */

	pd = md->params;

# if !defined(ENABLE_SOFT_FLOAT)

	for (i = 0; i < md->paramcount; i++, pd++) {
		t = md->paramtypes[i].type;

		if (IS_FLT_DBL_TYPE(t) &&
			((i == 0) ||
			 ((i == 1) && IS_FLT_DBL_TYPE(md->paramtypes[0].type)))) {
			if (IS_2_WORD_TYPE(t)) {
				pd->type   = TYPE_DBL;
				pd->regoff = abi_registers_float_argument[reguse];
				reguse++;
				stacksize += 2;
			}
			else {
				pd->type   = TYPE_FLT;
				pd->regoff = abi_registers_float_argument[reguse];
				reguse++;
				stacksize++;
			}
			md->argfltreguse = reguse;
			a0_is_float = true;
		}
		else {
			if (IS_2_WORD_TYPE(t)) {
				ALIGN_2(reguse);
				pd->type = TYPE_LNG;

				if (reguse < INT_ARG_CNT) {
					pd->inmemory = false;
#  if WORDS_BIGENDIAN == 1
					pd->regoff   =
						PACK_REGS(abi_registers_integer_argument[reguse + 1],
								  abi_registers_integer_argument[reguse]);
#  else
					pd->regoff   =
						PACK_REGS(abi_registers_integer_argument[reguse],
								  abi_registers_integer_argument[reguse + 1]);
#  endif
					reguse += 2;
					md->argintreguse = reguse;
				}
				else {
					ALIGN_2(stacksize);

					pd->inmemory = true;
					pd->regoff   = stacksize * 4;
				}
				stacksize += 2;
			}
			else {
				pd->type = TYPE_INT;

				if (reguse < INT_ARG_CNT) {
					pd->inmemory = false;
					pd->regoff   = abi_registers_integer_argument[reguse];
					reguse++;
					md->argintreguse = reguse;
				}
				else {
					pd->inmemory = true;
					pd->regoff   = stacksize * 4;
				}
				stacksize++;
			}
		}
	}

# else /* !defined(ENABLE_SOFT_FLOAT) */
#  error never actually tested!

	for (i = 0; i < md->paramcount; i++, pd++) {
		switch (md->paramtypes[i].type) {
		case TYPE_INT:
		case TYPE_ADR:
		case TYPE_FLT:
			pd->type = TYPE_INT;

			if (i < INT_ARG_CNT) {
				pd->inmemory = false;
				pd->regoff   = abi_registers_integer_argument[reguse];
				reguse++;
				md->argintreguse = reguse;
			}
			else {
				pd->inmemory = true;
				pd->regoff   = stacksize * 4;
			}
			stacksize++;
			break;
		case TYPE_LNG:
		case TYPE_DBL:
			pd->type = TYPE_LNG;

			if (i < INT_ARG_CNT) {
				pd->inmemory = false;
#  if WORDS_BIGENDIAN == 1
				pd->regoff   =
					PACK_REGS(abi_registers_integer_argument[reguse + 1],
							  abi_registers_integer_argument[reguse]);
#  else
				pd->regoff   =
					PACK_REGS(abi_registers_integer_argument[reguse],
							  abi_registers_integer_argument[reguse + 1]);
#  endif
				reguse += 2;
				md->argintreguse = reguse;
			}
			else {
				pd->inmemory = true;
				pd->regoff   = stacksize * 4;
			}
			stacksize += 2;
			break;
		}
	}

# endif /* !defined(ENABLE_SOFT_FLOAT) */

	/* fill register and stack usage */

	md->memuse = stacksize;

#endif /* SIZEOF_VOID_P == 8 */
}


/* md_return_alloc *************************************************************

   Precolor the Java Stackelement containing the Return Value. Since
   mips has a dedicated return register (not an reused arg or reserved
   reg), this is striaghtforward possible, as long, as this
   stackelement does not have to survive a method invokation
   (SAVEDVAR)

   --- in
   jd:                      jitdata of the current method
   stackslot:               Java Stackslot to contain the Return Value
   
   --- out
   if precoloring was possible:
   VAR(stackslot->varnum)->flags       = PREALLOC
   			             ->regoff      = [REG_RESULT|REG_FRESULT]

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

		if (IS_INT_LNG_TYPE(md->returntype.type)) {
#if SIZEOF_VOID_P == 4
			if (IS_2_WORD_TYPE(md->returntype.type))
				VAR(stackslot->varnum)->vv.regoff = REG_RESULT_PACKED;
			else
#endif
				VAR(stackslot->varnum)->vv.regoff = REG_RESULT;
		}
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
