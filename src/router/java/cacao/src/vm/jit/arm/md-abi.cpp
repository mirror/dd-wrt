/* src/vm/jit/arm/md-abi.cpp - functions for arm ABI

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

#include "vm/jit/arm/md-abi.hpp"

#include "vm/descriptor.hpp"
#include "vm/method.hpp"
#include "vm/global.hpp"

#include "vm/jit/abi.hpp"
#include "vm/jit/code.hpp"
#include "vm/jit/codegen-common.hpp" // for PACK_REGS

/* register descripton array **************************************************/

s4 nregdescint[] = {
	REG_ARG, REG_ARG, REG_ARG, REG_ARG, REG_SAV, REG_SAV, REG_SAV, REG_SAV,
	REG_SAV, REG_RES, REG_RES, REG_RES, REG_RES, REG_RES, REG_RES, REG_RES,
	REG_END
};

const char *abi_registers_integer_name[] = {
	"a1", "a2", "a3", "a4", "v1", "v2", "v3", "v4",
	"v5", "t3", "t1", "t2", "ip", "sp", "lr", "pc",
};

const s4 abi_registers_integer_argument[] = {
	0,  /* a0 */
	1,  /* a1 */
	2,  /* a2 */
	3,  /* a3 */
	REG_SPLIT,
};

const s4 abi_registers_integer_saved[] = {
	4,  /* s0 */
	5,  /* s1 */
	6,  /* s2 */
	7,  /* s3 */
	8,  /* s4 */
};

const s4 abi_registers_integer_temporary[] = {
	-1,
};


#if defined(ENABLE_SOFTFLOAT)
s4 nregdescfloat[] = {
	REG_RES, REG_RES, REG_RES, REG_RES,
	REG_RES, REG_RES, REG_RES, REG_RES,
	REG_END
};
#else
#if !defined(__ARMHF__)
s4 nregdescfloat[] = {
	REG_TMP, REG_TMP, REG_TMP, REG_TMP,
	REG_TMP, REG_TMP, REG_RES, REG_RES,
	REG_END
};
#else
s4 nregdescfloat[] = {
	REG_ARG, REG_ARG, REG_ARG, REG_ARG, REG_ARG, REG_ARG, REG_RES, REG_RES,
	REG_SAV, REG_SAV, REG_SAV, REG_SAV, REG_SAV, REG_SAV, REG_SAV, REG_SAV,
	REG_END
};
#endif
#endif /* defined(ENABLE_SOFTFLOAT) */

const s4 abi_registers_float_argument[] = {
#if defined(__ARMHF__)
	0, 1, 2, 3, 4, 5, 6, 7,
#endif
	-1,
};

const s4 abi_registers_float_saved[] = {
#if defined(__ARMHF__)
	8, 9, 10, 11, 12, 13, 14, 15,
#endif
	-1,
};

const s4 abi_registers_float_temporary[] = {
#if defined(__ARMHF__) || defined(ENABLE_SOFTFLOAT)
	-1,
#else
	0,  /* ft0 */
	1,  /* ft1 */
	2,  /* ft2 */
	3,  /* ft3 */
	4,  /* ft4 */
	5,  /* ft5 */
#endif
};


/* md_param_alloc **************************************************************

   Allocate Arguments to Stackslots according the Calling Conventions

   --- in:
   md->paramcount:           Number of arguments for this method
   md->paramtypes[].type:    Argument types

   --- out:
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
	s4         reguse, freguse;
	s4         stacksize;

	/* set default values */

	reguse    = 0;
	freguse   = 0;
	stacksize = 0;

	/* get params field of methoddesc */

	pd = md->params;

	for (i = 0; i < md->paramcount; i++, pd++) {
		switch (md->paramtypes[i].type) {
		case TYPE_INT:
		case TYPE_ADR:
#if !defined(__ARMHF__)
		case TYPE_FLT:
#endif
			if (reguse < INT_ARG_CNT) {
				pd->inmemory = false;
				pd->index    = reguse;
				pd->regoff   = abi_registers_integer_argument[reguse];
				reguse++;
			}
			else {
				pd->inmemory = true;
				pd->index    = stacksize;
				pd->regoff   = stacksize * 8;
				stacksize++;
			}
			break;

		case TYPE_LNG:
#if !defined(__ARMHF__)
		case TYPE_DBL:
#endif
			/* interally we use the EABI */

			ALIGN_2(reguse);

			if (reguse < INT_ARG_CNT) {
				pd->inmemory = false;
#if defined(__ARMEL__)
				pd->index    = PACK_REGS(reguse, reguse + 1);
				pd->regoff   =
					PACK_REGS(abi_registers_integer_argument[reguse],
							  abi_registers_integer_argument[reguse + 1]);
#else
				pd->index    = PACK_REGS(reguse + 1, reguse);
				pd->regoff   =
					PACK_REGS(abi_registers_integer_argument[reguse + 1],
							  abi_registers_integer_argument[reguse]);
#endif
				reguse += 2;
			}
			else {

				/*ALIGN_2(stacksize);*/

				pd->inmemory  = true;
				pd->index     = stacksize;
				pd->regoff    = stacksize * 8;
				/*stacksize    += 2;*/
				stacksize++;
			}
			break;

#if defined(__ARMHF__)
		case TYPE_FLT:
		case TYPE_DBL:
			if (freguse < FLT_ARG_CNT) {
				pd->inmemory = false;
				pd->index    = freguse;
				pd->regoff   = abi_registers_float_argument[freguse];
				freguse++;
			}
			else {
				pd->inmemory = true;
				pd->index    = stacksize;
				pd->regoff   = stacksize * 8;
				stacksize++;
			}
			break;
#endif
		}
	}

	/* Since R0/R1 (==A0/A1) are used for passing return values, this
	   argument register usage has to be regarded, too. */

	if (md->returntype.type != TYPE_VOID
#if defined(__ARMHF__)
			&& !IS_FLT_DBL_TYPE(md->returntype.type)
#endif
	)
	{
		if (!IS_2_WORD_TYPE(md->returntype.type)) {
			if (reguse < 1)
				reguse = 1;
		}
		else {
			if (reguse < 2)
				reguse = 2;
		}
	}
#if defined(__ARMHF__)
	else if (IS_FLT_DBL_TYPE(md->returntype.type))
		if (freguse < 1)
			freguse = 1;
#endif

	/* fill register and stack usage */

	md->argintreguse = reguse;
	md->argfltreguse = freguse;
	md->memuse       = stacksize;
}


/* md_param_alloc_native *******************************************************

   Pre-allocate arguments according the native ABI.

*******************************************************************************/

void md_param_alloc_native(methoddesc *md)
{
	paramdesc *pd;
	s4         i;
	s4         reguse, freguse;
	s4         stacksize;
	s1 sregused[8]; /* 1 = full, 0 = half free */
	bool backfill;

	/* set default values */

	reguse    = 0;
	freguse   = 0;
	stacksize = 0;
	backfill  = true;

	/* get params field of methoddesc */

	pd = md->params;

	for (i = 0; i < md->paramcount; i++, pd++) {
#if defined(__ARMHF__)
		if (IS_FLT_DBL_TYPE(md->paramtypes[i].type)) {
			bool found = false;
			do {
				/* Try to find free VFP register first. */
				/* If it fails, use the normal path below. */
				if (IS_2_WORD_TYPE(md->paramtypes[i].type)) {
					if (freguse >= 8)
						break;
					pd->inmemory = false;
					pd->index    = -1;
					pd->regoff   = abi_registers_float_argument[freguse];
					sregused[freguse] = 1;
					freguse++;
				}
				else {
					int i;
					if (backfill) {
						for (i=0; i<freguse; i++)
							if (!sregused[i])
								break;
					}
					else
						i = freguse;
					if (i<freguse) {
						pd->regoff   = abi_registers_float_argument[i] + BACKFILL_OFFSET;
						sregused[i] = 1;
					}
					else {
						if (freguse >= 8)
							break;
						/* no backfill space found */
						pd->regoff   = abi_registers_float_argument[freguse];
						sregused[freguse] = 0;
						freguse++;
					}
					pd->inmemory = false;
					pd->index    = -1;
				}
				found = true;
			} while (0);
			if (found)
				continue;
			backfill = false;
		}
#endif

		switch (md->paramtypes[i].type) {
		case TYPE_INT:
		case TYPE_ADR:
		case TYPE_FLT:
			if (reguse < INT_ARG_CNT
#if defined(__ARMHF__)
				&& md->paramtypes[i].type != TYPE_FLT
#endif
			   ) {
				pd->inmemory = false;
				pd->index    = -1;
				pd->regoff   = abi_registers_integer_argument[reguse];
				reguse++;
			}
			else {
				pd->inmemory = true;
				pd->index    = -1;
				pd->regoff   = stacksize * 4;
				stacksize++;
			}
			break;

		case TYPE_LNG:
		case TYPE_DBL:
			if (reguse < (INT_ARG_CNT - 1)
#if defined(__ARMHF__)
				&& md->paramtypes[i].type != TYPE_DBL
#endif
				) {
#if defined(__ARM_EABI__)
				ALIGN_2(reguse);
#endif
				pd->inmemory = false;
#if defined(__ARMEL__)
				pd->index    = -1;
				pd->regoff   =
					PACK_REGS(abi_registers_integer_argument[reguse],
							  abi_registers_integer_argument[reguse + 1]);
#else
				pd->index    = -1;
				pd->regoff   =
					PACK_REGS(abi_registers_integer_argument[reguse + 1],
							  abi_registers_integer_argument[reguse]);
#endif
				reguse += 2;
			}
#if !defined(__ARM_EABI__)
			else if (reguse < INT_ARG_CNT) {
				pd->inmemory = false;
# if defined(__ARMEL__)
				pd->index    = -1;
				pd->regoff   =
					PACK_REGS(abi_registers_integer_argument[reguse],
							  abi_registers_integer_argument[INT_ARG_CNT]);
# else
				pd->index    = -1;
				pd->regoff   =
					PACK_REGS(abi_registers_integer_argument[INT_ARG_CNT],
							  abi_registers_integer_argument[reguse]);
# endif
				reguse++;
				stacksize++;
			}
#endif
			else {
#if defined(__ARM_EABI__)
				ALIGN_2(stacksize);
#endif
				pd->inmemory  = true;
				pd->index     = -1;
				pd->regoff    = stacksize * 4;
#if defined(__ARMHF__)
				if (md->paramtypes[i].type != TYPE_DBL)
#endif
					reguse        = INT_ARG_CNT;
				stacksize    += 2;
			}
			break;
		}
	}

	/* Since R0/R1 (==A0/A1) are used for passing return values, this
	   argument register usage has to be regarded, too. */

	if (md->returntype.type != TYPE_VOID
#if defined(__ARMHF__)
			&& !IS_FLT_DBL_TYPE(md->returntype.type)
#endif
	)
	{
		if (!IS_2_WORD_TYPE(md->returntype.type)) {
			if (reguse < 1)
				reguse = 1;
		}
		else {
			if (reguse < 2)
				reguse = 2;
		}
	}
#if defined(__ARMHF__)
	else if (IS_FLT_DBL_TYPE(md->returntype.type))
		if (freguse < 1)
			freguse = 1;
#endif

	/* fill register and stack usage */

	md->argintreguse = reguse;
	md->argfltreguse = freguse;
	md->memuse       = stacksize;
}


/* md_return_alloc *************************************************************

   Precolor the Java Stackelement containing the Return Value, if possible.

   --- in
   m:                       Methodinfo of current method
   return_type:             Return Type of the Method (TYPE_INT.. TYPE_ADR)
                            TYPE_VOID is not allowed!
   stackslot:               Java Stackslot to contain the Return Value

   --- out
   if precoloring was possible:
   VAR(stackslot->varnum)->flags = PREALLOC
   VAR(stackslot->varnum)->vv.regoff = [REG_RESULT, (REG_RESULT2/REG_RESULT), REG_FRESULT]
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
	   could already use R0 == a00! */

	if (!code_is_leafmethod(code) || (md->paramcount == 0)) {
		/* Only precolor the stackslot, if it is not a SAVEDVAR <->
		   has not to survive method invokations. */

		if (!(stackslot->flags & SAVEDVAR)) {
#if !defined(ENABLE_SOFTFLOAT)
			/* Stackelements containing float or double values
			   (TYPE_FLT | TYPE_DBL) cannot be precolored, because we
			   use integer register to pass return values. (floats:
			   R0, doubles: R0/R1) */

			if (!IS_FLT_DBL_TYPE(md->returntype.type)) {
#endif

				VAR(stackslot->varnum)->flags = PREALLOC;

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

#if !defined(ENABLE_SOFTFLOAT)
			}
#if defined(__ARMHF__)
			else {
				VAR(stackslot->varnum)->flags = PREALLOC;

				if (rd->argfltreguse < 1)
					rd->argfltreguse = 1;

				VAR(stackslot->varnum)->vv.regoff = REG_FRESULT;
			}
#endif
#endif
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
