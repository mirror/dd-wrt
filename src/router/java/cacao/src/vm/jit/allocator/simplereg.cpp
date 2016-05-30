/* src/vm/jit/allocator/simplereg.cpp - register allocator

   Copyright (C) 1996-2014
   CACAOVM - Verein zur Foerderung der freien virtuellen Maschine CACAO
   Copyright (C) 2009 Theobroma Systems Ltd.

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
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.

*/


#include "config.h"

#include <cassert>
#include <stdint.h>

#include "vm/types.hpp"

#include "arch.hpp"
#include "md-abi.hpp"

#include "mm/memory.hpp"
#include "mm/dumpmemory.hpp"

#include "vm/descriptor.hpp"            // for typedesc, methoddesc, etc
#include "vm/exceptions.hpp"
#include "vm/method.hpp"
#include "vm/options.hpp"
#include "vm/resolve.hpp"

#include "vm/jit/allocator/simplereg.hpp"
#include "vm/jit/abi.hpp"
#include "vm/jit/builtin.hpp"
#include "vm/jit/code.hpp"
#include "vm/jit/reg.hpp"
#include "vm/jit/show.hpp"

#include "vm/jit/ir/instruction.hpp"

#define DEBUG_NAME "simplereg"


STAT_DECLARE_VAR(int,count_locals_spilled,0)
STAT_DECLARE_VAR(int,count_locals_register,0)
// currently not used!
STAT_DECLARE_VAR(int,count_ss_spilled,0)
// currently not used!
STAT_DECLARE_VAR(int,count_ss_register,0)
STAT_DECLARE_VAR(int,count_interface_size,0)
// currently not used!
STAT_DECLARE_VAR(int,count_argument_reg_ss,0)
// currently not used!
STAT_DECLARE_VAR(int,count_argument_mem_ss,0)
STAT_REGISTER_VAR(int,count_method_in_register,0,"methods in register","Number of Methods kept in registers")

/* function prototypes for this file ******************************************/

static void simplereg_allocate_interfaces(jitdata *jd);
static void simplereg_allocate_locals(jitdata *jd);
static void simplereg_allocate_temporaries(jitdata *jd);


/* size of a stackslot used by the internal ABI */

#define SIZE_OF_STACKSLOT 8


/* total number of registers */

#define TOTAL_REG_CNT  (INT_REG_CNT + FLT_REG_CNT)


/* macros for handling register stacks ****************************************/

#if !defined(NDEBUG)
# define AVAIL_FRONT(cnt, limit)   (!opt_RegallocSpillAll && ((cnt) < (limit)))
# define AVAIL_BACK(cnt)           (!opt_RegallocSpillAll && ((cnt) > 0))
#else
# define AVAIL_FRONT(cnt, limit)   ((cnt) < (limit))
# define AVAIL_BACK(cnt)           ((cnt) > 0)
#endif

#if defined(SUPPORT_COMBINE_INTEGER_REGISTERS)
# if !defined(NDEBUG)
#  define AVAIL_FRONT_INT(cnt, limit)   (!opt_RegallocSpillAll && ((cnt) < (limit) - intregsneeded))
#  define AVAIL_BACK_INT(cnt)           (!opt_RegallocSpillAll && ((cnt) > intregsneeded))
# else
#  define AVAIL_FRONT_INT(cnt, limit)   ((cnt) < (limit) - intregsneeded)
#  define AVAIL_BACK_INT(cnt)           ((cnt) > intregsneeded)
# endif
#else
# define AVAIL_FRONT_INT(cnt, limit)   AVAIL_FRONT(cnt, limit)
# define AVAIL_BACK_INT(cnt)           AVAIL_BACK(cnt)
#endif

#define POP_FRONT(stk, cnt, reg)   do {  reg = stk[cnt++];    } while (0)
#define POP_BACK(stk, cnt, reg)    do {  reg = stk[--cnt];    } while (0)
#define PUSH_FRONT(stk, cnt, reg)  do {  stk[--cnt] = (reg);  } while (0)
#define PUSH_BACK(stk, cnt, reg)   do {  stk[cnt++] = (reg);  } while (0)

#if defined(SUPPORT_COMBINE_INTEGER_REGISTERS)
#define POP_FRONT_INT(stk, cnt, reg)                                 \
    do {                                                             \
        if (intregsneeded) {                                         \
            reg = PACK_REGS(stk[cnt], stk[cnt+1]);                   \
            cnt += 2;                                                \
        }                                                            \
        else                                                         \
            POP_FRONT(stk, cnt, reg);                                \
    } while (0)
#else
#define POP_FRONT_INT(stk, cnt, reg)  POP_FRONT(stk, cnt, reg)
#endif

#if defined(SUPPORT_COMBINE_INTEGER_REGISTERS)
#define POP_BACK_INT(stk, cnt, reg)                                  \
    do {                                                             \
        if (intregsneeded) {                                         \
            cnt -= 2;                                                \
            reg = PACK_REGS(stk[cnt], stk[cnt+1]);                   \
        }                                                            \
        else                                                         \
            POP_BACK(stk, cnt, reg);                                 \
    } while (0)
#else
#define POP_BACK_INT(stk, cnt, reg)  POP_BACK(stk, cnt, reg)
#endif

#if defined(SUPPORT_COMBINE_INTEGER_REGISTERS)
#define PUSH_BACK_INT(stk, cnt, reg)                                 \
    do {                                                             \
        if (intregsneeded) {                                         \
            stk[cnt] = GET_LOW_REG(reg);                             \
            stk[cnt + 1] = GET_HIGH_REG(reg);                        \
            cnt += 2;                                                \
        }                                                            \
        else                                                         \
            PUSH_BACK(stk, cnt, reg);                                \
    } while (0)
#else
#define PUSH_BACK_INT(stk, cnt, reg)  PUSH_BACK(stk, cnt, reg)
#endif

#define AVAIL_ARG_FLT  AVAIL_FRONT(rd->argfltreguse, FLT_ARG_CNT)
#define AVAIL_TMP_FLT  AVAIL_BACK(rd->tmpfltreguse)
#define AVAIL_SAV_FLT  AVAIL_BACK(rd->savfltreguse)

#define AVAIL_ARG_ADR  AVAIL_FRONT(rd->argadrreguse, ADR_ARG_CNT)
#define AVAIL_TMP_ADR  AVAIL_BACK(rd->tmpadrreguse)
#define AVAIL_SAV_ADR  AVAIL_BACK(rd->savadrreguse)

#define AVAIL_ARG_INT  AVAIL_FRONT_INT(rd->argintreguse, INT_ARG_CNT)
#define AVAIL_TMP_INT  AVAIL_BACK_INT(rd->tmpintreguse)
#define AVAIL_SAV_INT  AVAIL_BACK_INT(rd->savintreguse)

#define AVAIL_FREE_ARG_FLT  AVAIL_BACK(rd->freeargflttop)
#define AVAIL_FREE_TMP_FLT  AVAIL_BACK(rd->freetmpflttop)
#define AVAIL_FREE_SAV_FLT  AVAIL_BACK(rd->freesavflttop)

#define AVAIL_FREE_ARG_ADR  AVAIL_BACK(rd->freeargadrtop)
#define AVAIL_FREE_TMP_ADR  AVAIL_BACK(rd->freetmpadrtop)
#define AVAIL_FREE_SAV_ADR  AVAIL_BACK(rd->freesavadrtop)

#define AVAIL_FREE_ARG_INT  AVAIL_BACK_INT(rd->freearginttop)
#define AVAIL_FREE_TMP_INT  AVAIL_BACK_INT(rd->freetmpinttop)
#define AVAIL_FREE_SAV_INT  AVAIL_BACK_INT(rd->freesavinttop)

#define TAKE_ARG_FLT(r)  POP_FRONT(abi_registers_float_argument, rd->argfltreguse, r)
#define TAKE_TMP_FLT(r)  POP_BACK(rd->tmpfltregs, rd->tmpfltreguse, r)
#define TAKE_SAV_FLT(r)  POP_BACK(rd->savfltregs, rd->savfltreguse, r)

#define TAKE_ARG_ADR(r)  POP_FRONT(rd->argadrregs, rd->argadrreguse, r)
#define TAKE_TMP_ADR(r)  POP_BACK(rd->tmpadrregs, rd->tmpadrreguse, r)
#define TAKE_SAV_ADR(r)  POP_BACK(rd->savadrregs, rd->savadrreguse, r)

#define TAKE_ARG_INT(r)  POP_FRONT_INT(abi_registers_integer_argument, rd->argintreguse, r)
#define TAKE_TMP_INT(r)  POP_BACK_INT(rd->tmpintregs, rd->tmpintreguse, r)
#define TAKE_SAV_INT(r)  POP_BACK_INT(rd->savintregs, rd->savintreguse, r)

#define TAKE_FREE_ARG_FLT(r)  POP_BACK(rd->freeargfltregs, rd->freeargflttop, r)
#define TAKE_FREE_TMP_FLT(r)  POP_BACK(rd->freetmpfltregs, rd->freetmpflttop, r)
#define TAKE_FREE_SAV_FLT(r)  POP_BACK(rd->freesavfltregs, rd->freesavflttop, r)

#define TAKE_FREE_ARG_ADR(r)  POP_BACK(rd->freeargadrregs, rd->freeargadrtop, r)
#define TAKE_FREE_TMP_ADR(r)  POP_BACK(rd->freetmpadrregs, rd->freetmpadrtop, r)
#define TAKE_FREE_SAV_ADR(r)  POP_BACK(rd->freesavadrregs, rd->freesavadrtop, r)

#define TAKE_FREE_ARG_INT(r)  POP_BACK_INT(rd->freeargintregs, rd->freearginttop, r)
#define TAKE_FREE_TMP_INT(r)  POP_BACK_INT(rd->freetmpintregs, rd->freetmpinttop, r)
#define TAKE_FREE_SAV_INT(r)  POP_BACK_INT(rd->freesavintregs, rd->freesavinttop, r)

#define PUSH_FREE_ARG_FLT(r)  PUSH_BACK(rd->freeargfltregs, rd->freeargflttop, r)
#define PUSH_FREE_TMP_FLT(r)  PUSH_BACK(rd->freetmpfltregs, rd->freetmpflttop, r)
#define PUSH_FREE_SAV_FLT(r)  PUSH_BACK(rd->freesavfltregs, rd->freesavflttop, r)

#define PUSH_FREE_ARG_ADR(r)  PUSH_BACK(rd->freeargadrregs, rd->freeargadrtop, r)
#define PUSH_FREE_TMP_ADR(r)  PUSH_BACK(rd->freetmpadrregs, rd->freetmpadrtop, r)
#define PUSH_FREE_SAV_ADR(r)  PUSH_BACK(rd->freesavadrregs, rd->freesavadrtop, r)

#define PUSH_FREE_ARG_INT(r)  PUSH_BACK_INT(rd->freeargintregs, rd->freearginttop, r)
#define PUSH_FREE_TMP_INT(r)  PUSH_BACK_INT(rd->freetmpintregs, rd->freetmpinttop, r)
#define PUSH_FREE_SAV_INT(r)  PUSH_BACK_INT(rd->freesavintregs, rd->freesavinttop, r)


/* macros for allocating memory slots ****************************************/

#define NEW_MEM_SLOT(r)                                              \
    do {                                                             \
        (r) = rd->memuse * SIZE_OF_STACKSLOT;                        \
        rd->memuse += 1;                                             \
    } while (0)

#define NEW_MEM_SLOT_INT_LNG(r)  NEW_MEM_SLOT(r)
#define NEW_MEM_SLOT_FLT_DBL(r)  NEW_MEM_SLOT(r)
#define NEW_MEM_SLOT_REUSE_PADDING(r)  NEW_MEM_SLOT(r)


/* macros for creating/freeing temporary variables ***************************/

#define NEW_TEMP_REG(index)                                          \
    if ( ((index) >= jd->localcount)                                 \
         && (!(VAR(index)->flags & (INOUT | PREALLOC))) )            \
        simplereg_new_temp(jd, (index))


#define FREE_TEMP_REG(index)                                         \
    if (((index) > jd->localcount)                                   \
        && (!(VAR(index)->flags & (PREALLOC))))                      \
        simplereg_free_temp(jd, (index))


/* macro for getting a unique register index *********************************/

#if defined(SUPPORT_COMBINE_INTEGER_REGISTERS)
#define REG_INDEX(regoff, type) (IS_FLT_DBL_TYPE(type) ? (INT_REG_CNT + (regoff)) : (GET_LOW_REG(regoff)))
#else
#define REG_INDEX(regoff, type) (IS_FLT_DBL_TYPE(type) ? (INT_REG_CNT + (regoff)) : (regoff))
#endif


/* regalloc ********************************************************************

   Does a simple register allocation.

*******************************************************************************/

bool regalloc(jitdata *jd)
{
	/* There is a problem with the use of unused float argument
	   registers in leafmethods for stackslots on c7 (2 * Dual Core
	   AMD Opteron(tm) Processor 270) - runtime for the jvm98 _mtrt
	   benchmark is heaviliy increased. This could be prevented by
	   setting rd->argfltreguse to FLT_ARG_CNT before calling
	   simplereg_allocate_temporaries and setting it back to the original
	   value before calling simplereg_allocate_locals.  */

	simplereg_allocate_interfaces(jd);
	simplereg_allocate_temporaries(jd);
	simplereg_allocate_locals(jd);

	/* everthing's ok */

	return true;
}


/* simplereg_allocate_interfaces ***********************************************

   Allocates registers for all interface variables.

*******************************************************************************/

static void simplereg_allocate_interfaces(jitdata *jd)
{
	methodinfo   *m;
	codeinfo     *code;
	//codegendata  *cd;
	registerdata *rd;

	int     s, t, tt, saved;
	int     intalloc, fltalloc; /* Remember allocated Register/Memory offset */
	                /* in case more vars are packed into this interface slot */
	/* Allocate LNG and DBL types first to ensure 2 registers                */
	/* on some architectures.                                                */
	int     typeloop[] = { TYPE_LNG, TYPE_DBL, TYPE_INT, TYPE_FLT, TYPE_ADR };
	int     flags, regoff;
#if defined(SUPPORT_COMBINE_INTEGER_REGISTERS)
	int		intregsneeded;
#endif

	/* get required compiler data */

	m    = jd->m;
	code = jd->code;
	//cd   = jd->cd;
	rd   = jd->rd;

	/* rd->memuse was already set in stack.c to allocate stack space
	   for passing arguments to called methods. */

#if defined(__I386__)
	if (checksync && code_is_synchronized(code)) {
		/* reserve 0(%esp) for Monitorenter/exit Argument on i386 */
		if (rd->memuse < 1)
			rd->memuse = 1;
	}
#endif

	if (code_is_leafmethod(code)) {
		/* Reserve argument register, which will be used for Locals acting */
		/* as Parameters */
		if (rd->argintreguse < m->parseddesc->argintreguse)
			rd->argintreguse = m->parseddesc->argintreguse;
		if (rd->argfltreguse < m->parseddesc->argfltreguse)
			rd->argfltreguse = m->parseddesc->argfltreguse;
	}

	for (s = 0; s < jd->maxinterfaces; s++) {
		intalloc = -1; fltalloc = -1;

		/* check if the interface at this stack depth must be a SAVEDVAR */

		saved = 0;

		for (tt = 0; tt <=4; tt++) {
			if ((t = jd->interface_map[s * 5 + tt].flags) != jitdata::UNUSED) {
				saved |= t & SAVEDVAR;
			}
		}

		/* allocate reg/mem for each type the interface is used as */

		for (tt = 0; tt <= 4; tt++) {
			t = typeloop[tt];
			if (jd->interface_map[s * 5 + t].flags == jitdata::UNUSED)
				continue;

			flags = saved;
			regoff = -1; /* initialize to invalid value */

#if defined(SUPPORT_COMBINE_INTEGER_REGISTERS)
			intregsneeded = (IS_2_WORD_TYPE(t)) ? 1 : 0;
#endif

			if (!saved) {
				if (IS_FLT_DBL_TYPE(t)) {
					if (fltalloc >= 0) {
						/* Reuse memory slot(s)/register(s) for shared interface slots */
						flags |= jd->interface_map[fltalloc].flags & ~SAVEDVAR;
						regoff = jd->interface_map[fltalloc].regoff;
					}
					else if (AVAIL_ARG_FLT) {
						flags |= ARGREG;
						TAKE_ARG_FLT(regoff);
					}
					else if (AVAIL_TMP_FLT) {
						TAKE_TMP_FLT(regoff);
					}
					else if (AVAIL_SAV_FLT) {
						flags |= SAVREG;
						TAKE_SAV_FLT(regoff);
					}
					else {
						flags |= INMEMORY;
						NEW_MEM_SLOT_FLT_DBL(regoff);
					}
					fltalloc = s * 5 + t;
				}
				else { /* !IS_FLT_DBL_TYPE(t) */
#if (SIZEOF_VOID_P == 4) && !defined(SUPPORT_COMBINE_INTEGER_REGISTERS)
					/*
					 * for i386 put all longs in memory
					 */
					if (IS_2_WORD_TYPE(t)) {
						flags |= INMEMORY;
						NEW_MEM_SLOT_INT_LNG(regoff);
					}
					else
#endif
						if (intalloc >= 0) {
							/* Reuse memory slot(s)/register(s) for shared interface slots */
							flags |= jd->interface_map[intalloc].flags & ~SAVEDVAR;
							regoff = jd->interface_map[intalloc].regoff;
#if defined(SUPPORT_COMBINE_INTEGER_REGISTERS)
							/* reuse lower half */
							if (!(flags & INMEMORY) && IS_2_WORD_TYPE(intalloc % 5))
								regoff = GET_LOW_REG(regoff);
#endif
						}
						else {
							if (AVAIL_ARG_INT) {
								flags |= ARGREG;
								TAKE_ARG_INT(regoff);
							}
							else if (AVAIL_TMP_INT) {
								TAKE_TMP_INT(regoff);
							}
							else if (AVAIL_SAV_INT) {
								flags |= SAVREG;
								TAKE_SAV_INT(regoff);
							}
							else {
								flags |= INMEMORY;
								NEW_MEM_SLOT_INT_LNG(regoff);
							}
						}

					intalloc = s * 5 + t;
				} /* if (IS_FLT_DBL_TYPE(t)) */
			}
			else { /* (saved) */
				/* now the same like above, but without a chance to take a temporary register */
				if (IS_FLT_DBL_TYPE(t)) {
					if (fltalloc >= 0) {
						flags |= jd->interface_map[fltalloc].flags & ~SAVEDVAR;
						regoff = jd->interface_map[fltalloc].regoff;
					}
					else {
						if (AVAIL_SAV_FLT) {
							TAKE_SAV_FLT(regoff);
						}
						else {
							flags |= INMEMORY;
							NEW_MEM_SLOT_FLT_DBL(regoff);
						}
					}
					fltalloc = s * 5 + t;
				}
				else { /* IS_INT_LNG */
#if (SIZEOF_VOID_P == 4) && !defined(SUPPORT_COMBINE_INTEGER_REGISTERS)
					/*
					 * for i386 put all longs in memory
					 */
					if (IS_2_WORD_TYPE(t)) {
						flags |= INMEMORY;
						NEW_MEM_SLOT_INT_LNG(regoff);
					}
					else
#endif
					{
						if (intalloc >= 0) {
							flags |= jd->interface_map[intalloc].flags & ~SAVEDVAR;
							regoff = jd->interface_map[intalloc].regoff;
#if defined(SUPPORT_COMBINE_INTEGER_REGISTERS)
							/*  reuse lower half */
							if (!(flags & INMEMORY) && IS_2_WORD_TYPE(intalloc % 5))
								regoff = GET_LOW_REG(regoff);
#endif
						}
						else {
							if (AVAIL_SAV_INT) {
								TAKE_SAV_INT(regoff);
							}
							else {
								flags |= INMEMORY;
								NEW_MEM_SLOT_INT_LNG(regoff);
							}
						}

						intalloc = s*5 + t;
					} /* if (IS_FLT_DBL_TYPE(t) else */
				} /* if (IS_ADR_TYPE(t)) else */
			} /* if (saved) else */
			/* if (type >= 0) */

			assert(regoff >= 0);
			jd->interface_map[5*s + t].flags = flags | INOUT;
			jd->interface_map[5*s + t].regoff = regoff;
		} /* for t */
	} /* for s */
}


/* simplereg_allocate_locals_leafmethod ****************************************

   Allocates registers for all local variables of a leafmethod.

*******************************************************************************/

static void simplereg_allocate_locals_leafmethod(jitdata *jd)
{
	methodinfo   *m;
	//codegendata  *cd;
	registerdata *rd;
	methoddesc *md;

	int     p, s, t, tt, varindex;
	int     intalloc, fltalloc;
	varinfo *v;
	int     intregsneeded = 0;
	int     typeloop[] = { TYPE_LNG, TYPE_DBL, TYPE_INT, TYPE_FLT, TYPE_ADR };
	int     fargcnt, iargcnt;

	/* get required compiler data */

	m  = jd->m;
	//cd = jd->cd;
	rd = jd->rd;

	md = m->parseddesc;

	iargcnt = rd->argintreguse;
	fargcnt = rd->argfltreguse;

	for (p = 0, s = 0; s < jd->maxlocals; s++, p++) {
		intalloc = -1; fltalloc = -1;
		for (tt = 0; tt <= 4; tt++) {
			t = typeloop[tt];
			varindex = jd->local_map[s * 5 + t];
			if (varindex == jitdata::UNUSED)
				continue;

			v = VAR(varindex);

#if defined(SUPPORT_COMBINE_INTEGER_REGISTERS)
			intregsneeded = (IS_2_WORD_TYPE(t)) ? 1 : 0;
#endif
			if (IS_FLT_DBL_TYPE(t)) {
				if (fltalloc >= 0) {
					v->flags = VAR(fltalloc)->flags;
					v->vv.regoff = VAR(fltalloc)->vv.regoff;
				}
#if !defined(SUPPORT_PASS_FLOATARGS_IN_INTREGS)
				/* We can only use float arguments as local variables,
				 * if we do not pass them in integer registers. */
				else if ((p < md->paramcount) && !md->params[p].inmemory) {
					v->flags = 0;
					v->vv.regoff = md->params[p].regoff;
				}
#endif
				else if (AVAIL_TMP_FLT) {
					v->flags = 0;
					TAKE_TMP_FLT(v->vv.regoff);
				}
				/* use unused argument registers as local registers */
				else if ((p >= md->paramcount) && (fargcnt < FLT_ARG_CNT)) {
					v->flags = 0;
					POP_FRONT(abi_registers_float_argument,
							  fargcnt, v->vv.regoff);
				}
				else if (AVAIL_SAV_FLT) {
					v->flags = 0;
					TAKE_SAV_FLT(v->vv.regoff);
				}
				else {
					v->flags = INMEMORY;
					NEW_MEM_SLOT_FLT_DBL(v->vv.regoff);
				}
				fltalloc = jd->local_map[s * 5 + t];

			}
			else {
#if (SIZEOF_VOID_P == 4) && !defined(SUPPORT_COMBINE_INTEGER_REGISTERS)
				/*
				 * for i386 put all longs in memory
				 */
				if (IS_2_WORD_TYPE(t)) {
					v->flags = INMEMORY;
					NEW_MEM_SLOT_INT_LNG(v->vv.regoff);
				}
				else
#endif
				{
					if (intalloc >= 0) {
						v->flags = VAR(intalloc)->flags;
#if defined(SUPPORT_COMBINE_INTEGER_REGISTERS)
						if (!(v->flags & INMEMORY) && IS_2_WORD_TYPE(VAR(intalloc)->type))
							v->vv.regoff = GET_LOW_REG(VAR(intalloc)->vv.regoff);
						else
#endif
							v->vv.regoff = VAR(intalloc)->vv.regoff;
					}
					else if ((p < md->paramcount) &&
							 !md->params[p].inmemory)
					{
						v->flags = 0;
#if defined(SUPPORT_COMBINE_INTEGER_REGISTERS)
						if (IS_2_WORD_TYPE(t))
							v->vv.regoff =
								PACK_REGS(GET_LOW_REG(md->params[p].regoff),
										  GET_HIGH_REG(md->params[p].regoff));
							else
#endif
								v->vv.regoff = md->params[p].regoff;
					}
					else if (AVAIL_TMP_INT) {
						v->flags = 0;
						TAKE_TMP_INT(v->vv.regoff);
					}
					/*
					 * use unused argument registers as local registers
					 */
					else if ((p >= m->parseddesc->paramcount) &&
							 (iargcnt + intregsneeded < INT_ARG_CNT))
					{
						v->flags = 0;
						POP_FRONT_INT(abi_registers_integer_argument,
									  iargcnt, v->vv.regoff);
					}
					else if (AVAIL_SAV_INT) {
						v->flags = 0;
						TAKE_SAV_INT(v->vv.regoff);
					}
					else {
						v->flags = INMEMORY;
						NEW_MEM_SLOT_INT_LNG(v->vv.regoff);
					}
				}
				intalloc = jd->local_map[s * 5 + t];
			}
		} /* for (tt=0;...) */

		/* If the current parameter is a 2-word type, the next local slot */
		/* is skipped.                                                    */

		if (p < md->paramcount)
			if (IS_2_WORD_TYPE(md->paramtypes[p].type))
				s++;
	}
}

/* simplereg_allocate_locals ***************************************************

   Allocates registers for all local variables.

*******************************************************************************/

static void simplereg_allocate_locals(jitdata *jd)
{
	codeinfo     *code;
	//codegendata  *cd;
	registerdata *rd;

	int     s, t, tt, varindex;
	int     intalloc, fltalloc;
	varinfo *v;
	int     typeloop[] = { TYPE_LNG, TYPE_DBL, TYPE_INT, TYPE_FLT, TYPE_ADR };
#ifdef SUPPORT_COMBINE_INTEGER_REGISTERS
	s4 intregsneeded;
#endif

	/* get required compiler data */

	code = jd->code;
	//cd   = jd->cd;
	rd   = jd->rd;

	if (code_is_leafmethod(code)) {
		simplereg_allocate_locals_leafmethod(jd);
		return;
	}

	for (s = 0; s < jd->maxlocals; s++) {
		intalloc = -1; fltalloc = -1;
		for (tt=0; tt<=4; tt++) {
			t = typeloop[tt];

			varindex = jd->local_map[s * 5 + t];
			if (varindex == jitdata::UNUSED)
				continue;

			v = VAR(varindex);

#ifdef SUPPORT_COMBINE_INTEGER_REGISTERS
				intregsneeded = (IS_2_WORD_TYPE(t)) ? 1 : 0;
#endif

			if (IS_FLT_DBL_TYPE(t)) {
				if (fltalloc >= 0) {
					v->flags = VAR(fltalloc)->flags;
					v->vv.regoff = VAR(fltalloc)->vv.regoff;
				}
				else if (AVAIL_SAV_FLT) {
					v->flags = 0;
					TAKE_SAV_FLT(v->vv.regoff);
				}
				else {
					v->flags = INMEMORY;
					NEW_MEM_SLOT_FLT_DBL(v->vv.regoff);
				}
				fltalloc = jd->local_map[s * 5 + t];
			}
			else {
#if (SIZEOF_VOID_P == 4) && !defined(SUPPORT_COMBINE_INTEGER_REGISTERS)
				/*
				 * for i386 put all longs in memory
				 */
				if (IS_2_WORD_TYPE(t)) {
					v->flags = INMEMORY;
					NEW_MEM_SLOT_INT_LNG(v->vv.regoff);
				}
				else {
#endif
					if (intalloc >= 0) {
						v->flags = VAR(intalloc)->flags;
#if defined(SUPPORT_COMBINE_INTEGER_REGISTERS)
						if (!(v->flags & INMEMORY) && IS_2_WORD_TYPE(VAR(intalloc)->type))
							v->vv.regoff = GET_LOW_REG(VAR(intalloc)->vv.regoff);
						else
#endif
							v->vv.regoff = VAR(intalloc)->vv.regoff;
					}
					else if (AVAIL_SAV_INT) {
						v->flags = 0;
						TAKE_SAV_INT(v->vv.regoff);
					}
					else {
						v->flags = INMEMORY;
						NEW_MEM_SLOT_INT_LNG(v->vv.regoff);
					}
#if (SIZEOF_VOID_P == 4) && !defined(SUPPORT_COMBINE_INTEGER_REGISTERS)
				}
#endif
				intalloc = jd->local_map[s * 5 + t];
			}
		}
	}
}


static void simplereg_init(jitdata *jd, registerdata *rd)
{
	int i;

	rd->freememtop = 0;

	rd->freetmpinttop = 0;
	rd->freesavinttop = 0;
	rd->freetmpflttop = 0;
	rd->freesavflttop = 0;

	rd->freearginttop = 0;
	rd->freeargflttop = 0;

	rd->regisoutvar = DMNEW(int, TOTAL_REG_CNT);
	rd->regcopycount = DMNEW(int, TOTAL_REG_CNT);
	MZERO(rd->regcopycount, int, TOTAL_REG_CNT);

	/* memcopycount is dynamically allocated when needed */

	rd->memcopycount = NULL;
	rd->memcopycountsize = 0;

	rd->intusedinout = DMNEW(int, INT_REG_CNT);
	MZERO(rd->intusedinout, int, INT_REG_CNT);
	rd->fltusedinout = DMNEW(int, FLT_REG_CNT);
	MZERO(rd->fltusedinout, int, FLT_REG_CNT);

	/* record the interface registers as used */

	for (i=0; i<rd->argintreguse; ++i)
		rd->intusedinout[abi_registers_integer_argument[i]] = 1;
	for (i=rd->tmpintreguse; i<INT_TMP_CNT; ++i)
		rd->intusedinout[rd->tmpintregs[i]] = 1;
	for (i=rd->savintreguse; i<INT_SAV_CNT; ++i)
		rd->intusedinout[rd->savintregs[i]] = 1;

	for (i=0; i<rd->argfltreguse; ++i)
		rd->fltusedinout[abi_registers_float_argument[i]] = 1;
	for (i=rd->tmpfltreguse; i<FLT_TMP_CNT; ++i)
		rd->fltusedinout[rd->tmpfltregs[i]] = 1;
	for (i=rd->savfltreguse; i<FLT_SAV_CNT; ++i)
		rd->fltusedinout[rd->savfltregs[i]] = 1;
}


static void simplereg_init_block(registerdata *rd)
{
	int i;

	/* remove all interface registers from the free lists */

	for (i=0; i<rd->freearginttop; ++i)
		if (rd->intusedinout[rd->freeargintregs[i]]) {
			rd->freeargintregs[i--] = rd->freeargintregs[--rd->freearginttop];
		}
	for (i=0; i<rd->freetmpinttop; ++i)
		if (rd->intusedinout[rd->freetmpintregs[i]]) {
			rd->freetmpintregs[i--] = rd->freetmpintregs[--rd->freetmpinttop];
		}
	for (i=0; i<rd->freesavinttop; ++i)
		if (rd->intusedinout[rd->freesavintregs[i]]) {
			rd->freesavintregs[i--] = rd->freesavintregs[--rd->freesavinttop];
		}

	for (i=0; i<rd->freeargflttop; ++i)
		if (rd->fltusedinout[rd->freeargfltregs[i]]) {
			rd->freeargfltregs[i--] = rd->freeargfltregs[--rd->freeargflttop];
		}
	for (i=0; i<rd->freetmpflttop; ++i)
		if (rd->fltusedinout[rd->freetmpfltregs[i]]) {
			rd->freetmpfltregs[i--] = rd->freetmpfltregs[--rd->freetmpflttop];
		}
	for (i=0; i<rd->freesavflttop; ++i)
		if (rd->fltusedinout[rd->freesavfltregs[i]]) {
			rd->freesavfltregs[i--] = rd->freesavfltregs[--rd->freesavflttop];
		}
}


static void simplereg_new_temp(jitdata *jd, s4 index)
{
#ifdef SUPPORT_COMBINE_INTEGER_REGISTERS
	s4 intregsneeded;
#endif
	s4 tryagain;
	registerdata *rd;
	varinfo      *v;

	rd = jd->rd;
	v = VAR(index);

	/* assert that constants are not allocated */

	assert(v->type != TYPE_RET);

	/* Try to allocate a saved register if there is no temporary one          */
	/* available. This is what happens during the second run.                 */
	tryagain = (v->flags & SAVEDVAR) ? 1 : 2;

#ifdef SUPPORT_COMBINE_INTEGER_REGISTERS
	intregsneeded = (IS_2_WORD_TYPE(v->type)) ? 1 : 0;
#endif

	for(; tryagain; --tryagain) {
		if (tryagain == 1) {
			if (!(v->flags & SAVEDVAR))
				v->flags |= SAVREG;

			if (IS_FLT_DBL_TYPE(v->type)) {
				if (AVAIL_FREE_SAV_FLT) {
					TAKE_FREE_SAV_FLT(v->vv.regoff);
					return;
				}
				else if (AVAIL_SAV_FLT) {
					TAKE_SAV_FLT(v->vv.regoff);
					return;
				}
			}
			else {
#if (SIZEOF_VOID_P == 4) && !defined(SUPPORT_COMBINE_INTEGER_REGISTERS)
				/*
				 * for i386 put all longs in memory
				 */
				if (!IS_2_WORD_TYPE(v->type))
#endif
				{
					if (AVAIL_FREE_SAV_INT) {
						TAKE_FREE_SAV_INT(v->vv.regoff);
						return;
					}
					else if (AVAIL_SAV_INT) {
						TAKE_SAV_INT(v->vv.regoff);
						return;
					}
				}
			}
		}
		else { /* tryagain == 2 */
			if (IS_FLT_DBL_TYPE(v->type)) {
				if (AVAIL_FREE_ARG_FLT) {
					v->flags |= ARGREG;
					TAKE_FREE_ARG_FLT(v->vv.regoff);
					return;
				}
				else if (AVAIL_ARG_FLT) {
					v->flags |= ARGREG;
					TAKE_ARG_FLT(v->vv.regoff);
					return;
				}
				else if (AVAIL_FREE_TMP_FLT) {
					TAKE_FREE_TMP_FLT(v->vv.regoff);
					return;
				}
				else if (AVAIL_TMP_FLT) {
					TAKE_TMP_FLT(v->vv.regoff);
					return;
				}
			}
			else {
#if (SIZEOF_VOID_P == 4) && !defined(SUPPORT_COMBINE_INTEGER_REGISTERS)
				/*
				 * for i386 put all longs in memory
				 */
				if (!IS_2_WORD_TYPE(v->type))
#endif
				{
					if (AVAIL_FREE_ARG_INT) {
						v->flags |= ARGREG;
						TAKE_FREE_ARG_INT(v->vv.regoff);
						return;
					}
					else if (AVAIL_ARG_INT) {
						v->flags |= ARGREG;
						TAKE_ARG_INT(v->vv.regoff);
						return;
					}
					else if (AVAIL_FREE_TMP_INT) {
						TAKE_FREE_TMP_INT(v->vv.regoff);
						return;
					}
					else if (AVAIL_TMP_INT) {
						TAKE_TMP_INT(v->vv.regoff);
						return;
					}
				} /* if (!IS_2_WORD_TYPE(s->type)) */
			} /* if (IS_FLT_DBL_TYPE(s->type)) */
		} /* if (tryagain == 1) else */
	} /* for(; tryagain; --tryagain) */

	/* spill to memory */

	v->flags |= INMEMORY;

	if (rd->freememtop > 0)
		POP_BACK(rd->freemem, rd->freememtop, v->vv.regoff);
	else
		NEW_MEM_SLOT_REUSE_PADDING(v->vv.regoff);
}


static void simplereg_free(registerdata *rd, s4 flags, s4 regoff, s4 type)
{
	/* assert that constants are not freed */

	assert(type != TYPE_RET);

	/* if this is a copy of another variable, just decrement the copy counter */

	if (flags & INMEMORY) {
		int32_t memindex;

		if (flags & INOUT)
			return;

		memindex = regoff / SIZE_OF_STACKSLOT;

		if (memindex < rd->memcopycountsize && rd->memcopycount[memindex]) {
			rd->memcopycount[memindex]--;
			return;
		}
	}
	else {
		s4 regindex;

		regindex = REG_INDEX(regoff, type);

		/* do not free interface registers that are needed as outvars */

		if (flags & INOUT) {
			if (rd->regisoutvar[regindex]) {
				LOG("DONT FREE f=" << cacao::hex << cacao::setw(2)
				    << cacao::fillzero << flags << cacao::dec
					<< " r=" << regoff << " t=" << type << cacao::nl);
				return;
			}

			LOG("FREEING INVAR f=" << cacao::hex << cacao::setw(2)
			    << cacao::fillzero << flags << cacao::dec
			    << " r=" << regoff << " t=" << type << cacao::nl);
		}

		if (rd->regcopycount[regindex]) {
			rd->regcopycount[regindex]--;
			return;
		}
	}

	if (flags & INMEMORY) {
		PUSH_BACK(rd->freemem, rd->freememtop, regoff);
		return;
	}

	/* freeing a register */

	else if (IS_FLT_DBL_TYPE(type)) {
		if (flags & (SAVEDVAR | SAVREG))
			PUSH_FREE_SAV_FLT(regoff);
		else if (flags & ARGREG)
			PUSH_FREE_ARG_FLT(regoff);
		else
			PUSH_FREE_TMP_FLT(regoff);
	}
	else { /* IS_INT_LNG_TYPE */
#if defined(SUPPORT_COMBINE_INTEGER_REGISTERS)
		s4 intregsneeded = (IS_2_WORD_TYPE(type)) ? 1 : 0;
#endif

		if (flags & (SAVEDVAR | SAVREG))
			PUSH_FREE_SAV_INT(regoff);
		else if (flags & ARGREG)
			PUSH_FREE_ARG_INT(regoff);
		else
			PUSH_FREE_TMP_INT(regoff);
	}
}


static inline void simplereg_free_temp(jitdata *jd, s4 index)
{
	varinfo *v;

	v = VAR(index);

	simplereg_free(jd->rd, v->flags, v->vv.regoff, v->type);
}


static bool simplereg_alloc_dup(jitdata *jd, s4 srcindex, s4 dstindex)
{
	varinfo *sv;
	varinfo *dv;

	/* do not coalesce local variables here */

	if (srcindex <= jd->localcount || dstindex <= jd->localcount)
		return false;

	sv = VAR(srcindex);
	dv = VAR(dstindex);

	/* do not coalesce in/out vars or preallocated variables here */

	if ((sv->flags | dv->flags) & (INOUT | PREALLOC))
		return false;

	/* if the source is in memory, we can coalesce in any case */

	if (sv->flags & INMEMORY) {
		dv->flags |= INMEMORY;
		dv->vv.regoff = sv->vv.regoff;
		return true;
	}

	/* we do not allocate a REG_TMP to a REG_SAV variable */

	if ((sv->flags & SAVEDVAR) != (dv->flags & SAVEDVAR))
		return false;

	/* coalesce */
	dv->vv.regoff = sv->vv.regoff;
	dv->flags |= sv->flags & (SAVREG | ARGREG);

	return true;
}


/* simplereg_allocate_temporaries **********************************************

   Allocate temporary (non-interface, non-local) registers.

*******************************************************************************/

static void simplereg_allocate_temporaries(jitdata *jd)
{
	//methodinfo         *m;
	registerdata       *rd;
	s4                  i;
	s4                  len;
	instruction        *iptr;
	basicblock         *bptr;
	builtintable_entry *bte;
	methoddesc         *md;
	s4                 *argp;
	varinfo            *v;
	s4                  flags;
	s4                  regoff;
	s4                  type;
	s4                  regindex;

	/* get required compiler data */

	//m  = jd->m;
	rd = jd->rd;

	/* initialize temp registers */

	simplereg_init(jd, rd);

	bptr = jd->basicblocks;

	while (bptr != NULL) {
		if (bptr->state >= basicblock::REACHED) {

			LOG(cacao::nl << "allocating block L" << cacao::setw(3)
			              << cacao::fillzero << bptr->nr << cacao::nl);

			simplereg_init_block(rd);

			/* assert that all copy counts are zero */

#if !defined(NDEBUG) && !defined(ENABLE_SSA)
			for (i=0; i < TOTAL_REG_CNT; ++i)
				assert(rd->regcopycount[i] == 0);
#endif

			/* reset outvar flags */

			MZERO(rd->regisoutvar, int, TOTAL_REG_CNT);

			/* set allocation of invars */

			for (i=0; i<bptr->indepth; ++i)
			{
				v = VAR(bptr->invars[i]);
				if (v->type == TYPE_RET)
					continue;

				v->vv.regoff = jd->interface_map[5*i + v->type].regoff;
				v->flags  = jd->interface_map[5*i + v->type].flags;

				if (!(v->flags & INMEMORY))
					rd->regcopycount[REG_INDEX(v->vv.regoff, v->type)] = 1;
			}

			/* set allocation of outvars */

			for (i=0; i<bptr->outdepth; ++i)
			{
				v = VAR(bptr->outvars[i]);
				if (v->type == TYPE_RET)
					continue;

				v->vv.regoff = jd->interface_map[5*i + v->type].regoff;
				v->flags  = jd->interface_map[5*i + v->type].flags;

				if (!(v->flags & INMEMORY)) {
					regindex = REG_INDEX(v->vv.regoff, v->type);
					rd->regcopycount[regindex] = 1;
					rd->regisoutvar[regindex] = 1;
				}
			}

			/* free interface registers not used in this block */

			for (i=0; i < 5 * jd->maxinterfaces; ++i) {
				type = i%5;
				regoff = jd->interface_map[i].regoff;
				flags = jd->interface_map[i].flags;

				if (!(flags & INMEMORY)) {
					if (!rd->regcopycount[REG_INDEX(regoff, type)]) {
						LOG("MAY REUSE interface register f="
							<< cacao::hex << cacao::setw(2) << cacao::fillzero
							<< flags << cacao::dec
							<< " r=" << regoff << " t=" << type << cacao::nl);
						simplereg_free(rd, flags, regoff, type);

						/* mark it, so it is not freed again */
						rd->regcopycount[REG_INDEX(regoff, type)] = -1;
					}
				}
			}

			/* reset copy counts */

			MZERO(rd->regcopycount, int, TOTAL_REG_CNT);

			/* iterate over ICMDS to allocate temporary variables */

			iptr = bptr->iinstr;
			len = bptr->icount;

			while (--len >= 0)  {

				switch (iptr->opc) {

					/* pop 0 push 0 */

				case ICMD_JSR:
				case ICMD_NOP:
				case ICMD_CHECKNULL:
				case ICMD_IINC:
				case ICMD_RET:
				case ICMD_RETURN:
				case ICMD_GOTO:
				case ICMD_BREAKPOINT:
				case ICMD_PUTSTATICCONST:
				case ICMD_INLINE_START:
				case ICMD_INLINE_END:
				case ICMD_INLINE_BODY:
					break;

					/* pop 0 push 1 const */

				case ICMD_ICONST:
				case ICMD_LCONST:
				case ICMD_FCONST:
				case ICMD_DCONST:
				case ICMD_ACONST:
				case ICMD_GETEXCEPTION:

					/* pop 0 push 1 load */

				case ICMD_ILOAD:
				case ICMD_LLOAD:
				case ICMD_FLOAD:
				case ICMD_DLOAD:
				case ICMD_ALOAD:
					NEW_TEMP_REG(iptr->dst.varindex);
					break;

					/* pop 2 push 1 */

				case ICMD_IALOAD:
				case ICMD_LALOAD:
				case ICMD_FALOAD:
				case ICMD_DALOAD:
				case ICMD_AALOAD:

				case ICMD_BALOAD:
				case ICMD_CALOAD:
				case ICMD_SALOAD:
					FREE_TEMP_REG(iptr->sx.s23.s2.varindex);
					FREE_TEMP_REG(iptr->s1.varindex);
					NEW_TEMP_REG(iptr->dst.varindex);
					break;

					/* pop 3 push 0 */

				case ICMD_IASTORE:
				case ICMD_LASTORE:
				case ICMD_FASTORE:
				case ICMD_DASTORE:
				case ICMD_AASTORE:

				case ICMD_BASTORE:
				case ICMD_CASTORE:
				case ICMD_SASTORE:
					FREE_TEMP_REG(iptr->sx.s23.s3.varindex);
					FREE_TEMP_REG(iptr->sx.s23.s2.varindex);
					FREE_TEMP_REG(iptr->s1.varindex);
					break;

					/* pop 1 push 0 store */

				case ICMD_ISTORE:
				case ICMD_LSTORE:
				case ICMD_FSTORE:
				case ICMD_DSTORE:
				case ICMD_ASTORE:

					/* pop 1 push 0 */

				case ICMD_POP:

				case ICMD_IRETURN:
				case ICMD_LRETURN:
				case ICMD_FRETURN:
				case ICMD_DRETURN:
				case ICMD_ARETURN:

				case ICMD_ATHROW:

				case ICMD_PUTSTATIC:
				case ICMD_PUTFIELDCONST:

					/* pop 1 push 0 branch */

				case ICMD_IFNULL:
				case ICMD_IFNONNULL:

				case ICMD_IFEQ:
				case ICMD_IFNE:
				case ICMD_IFLT:
				case ICMD_IFGE:
				case ICMD_IFGT:
				case ICMD_IFLE:

				case ICMD_IF_LEQ:
				case ICMD_IF_LNE:
				case ICMD_IF_LLT:
				case ICMD_IF_LGE:
				case ICMD_IF_LGT:
				case ICMD_IF_LLE:

					/* pop 1 push 0 table branch */

				case ICMD_TABLESWITCH:
				case ICMD_LOOKUPSWITCH:

				case ICMD_MONITORENTER:
				case ICMD_MONITOREXIT:
					FREE_TEMP_REG(iptr->s1.varindex);
					break;

					/* pop 2 push 0 branch */

				case ICMD_IF_ICMPEQ:
				case ICMD_IF_ICMPNE:
				case ICMD_IF_ICMPLT:
				case ICMD_IF_ICMPGE:
				case ICMD_IF_ICMPGT:
				case ICMD_IF_ICMPLE:

				case ICMD_IF_LCMPEQ:
				case ICMD_IF_LCMPNE:
				case ICMD_IF_LCMPLT:
				case ICMD_IF_LCMPGE:
				case ICMD_IF_LCMPGT:
				case ICMD_IF_LCMPLE:

				case ICMD_IF_ACMPEQ:
				case ICMD_IF_ACMPNE:

					/* pop 2 push 0 */

				case ICMD_POP2:

				case ICMD_PUTFIELD:

				case ICMD_IASTORECONST:
				case ICMD_LASTORECONST:
				case ICMD_AASTORECONST:
				case ICMD_BASTORECONST:
				case ICMD_CASTORECONST:
				case ICMD_SASTORECONST:
					FREE_TEMP_REG(iptr->sx.s23.s2.varindex);
					FREE_TEMP_REG(iptr->s1.varindex);
					break;

					/* pop 0 push 1 copy */

				case ICMD_COPY:
					/* src === dst->prev (identical Stackslot Element)     */
					/* src --> dst       (copied value, take same reg/mem) */

 					if (!simplereg_alloc_dup(jd, iptr->s1.varindex, iptr->dst.varindex)) {
						NEW_TEMP_REG(iptr->dst.varindex);
 					}
					else {
						v = VAROP(iptr->dst);

						if (v->flags & INMEMORY) {
							int32_t memindex = v->vv.regoff / SIZE_OF_STACKSLOT;
							if (memindex >= rd->memcopycountsize) {
								int newsize = (memindex + 1) * 2;
								i = rd->memcopycountsize;
								rd->memcopycount = DMREALLOC(rd->memcopycount, int, i, newsize);
								MZERO(rd->memcopycount + i, int, newsize - i);
								rd->memcopycountsize = newsize;
							}
							rd->memcopycount[memindex]++;
						}
						else {
							/* XXX split reg/mem variables on arm may need special handling here */

							s4 regindex = REG_INDEX(v->vv.regoff, v->type);

							rd->regcopycount[regindex]++;
						}
 					}
					break;

					/* pop 1 push 1 move */

				case ICMD_MOVE:
 					if (!simplereg_alloc_dup(jd, iptr->s1.varindex, iptr->dst.varindex)) {
						NEW_TEMP_REG(iptr->dst.varindex);
						FREE_TEMP_REG(iptr->s1.varindex);
					}
					break;

					/* pop 2 push 1 */

				case ICMD_IADD:
				case ICMD_ISUB:
				case ICMD_IMUL:
				case ICMD_IDIV:
				case ICMD_IREM:

				case ICMD_ISHL:
				case ICMD_ISHR:
				case ICMD_IUSHR:
				case ICMD_IAND:
				case ICMD_IOR:
				case ICMD_IXOR:

				case ICMD_LADD:
				case ICMD_LSUB:
				case ICMD_LMUL:
				case ICMD_LDIV:
				case ICMD_LREM:

				case ICMD_LOR:
				case ICMD_LAND:
				case ICMD_LXOR:

				case ICMD_LSHL:
				case ICMD_LSHR:
				case ICMD_LUSHR:

				case ICMD_FADD:
				case ICMD_FSUB:
				case ICMD_FMUL:
				case ICMD_FDIV:
				case ICMD_FREM:

				case ICMD_DADD:
				case ICMD_DSUB:
				case ICMD_DMUL:
				case ICMD_DDIV:
				case ICMD_DREM:

				case ICMD_LCMP:
				case ICMD_FCMPL:
				case ICMD_FCMPG:
				case ICMD_DCMPL:
				case ICMD_DCMPG:
					FREE_TEMP_REG(iptr->sx.s23.s2.varindex);
					FREE_TEMP_REG(iptr->s1.varindex);
					NEW_TEMP_REG(iptr->dst.varindex);
					break;

					/* pop 1 push 1 */

				case ICMD_IADDCONST:
				case ICMD_ISUBCONST:
				case ICMD_IMULCONST:
				case ICMD_IMULPOW2:
				case ICMD_IDIVPOW2:
				case ICMD_IREMPOW2:
				case ICMD_IANDCONST:
				case ICMD_IORCONST:
				case ICMD_IXORCONST:
				case ICMD_ISHLCONST:
				case ICMD_ISHRCONST:
				case ICMD_IUSHRCONST:

				case ICMD_LADDCONST:
				case ICMD_LSUBCONST:
				case ICMD_LMULCONST:
				case ICMD_LMULPOW2:
				case ICMD_LDIVPOW2:
				case ICMD_LREMPOW2:
				case ICMD_LANDCONST:
				case ICMD_LORCONST:
				case ICMD_LXORCONST:
				case ICMD_LSHLCONST:
				case ICMD_LSHRCONST:
				case ICMD_LUSHRCONST:

				case ICMD_INEG:
				case ICMD_INT2BYTE:
				case ICMD_INT2CHAR:
				case ICMD_INT2SHORT:
				case ICMD_LNEG:
				case ICMD_FNEG:
				case ICMD_DNEG:

				case ICMD_I2L:
				case ICMD_I2F:
				case ICMD_I2D:
				case ICMD_L2I:
				case ICMD_L2F:
				case ICMD_L2D:
				case ICMD_F2I:
				case ICMD_F2L:
				case ICMD_F2D:
				case ICMD_D2I:
				case ICMD_D2L:
				case ICMD_D2F:

				case ICMD_CHECKCAST:

				case ICMD_ARRAYLENGTH:
				case ICMD_INSTANCEOF:

				case ICMD_NEWARRAY:
				case ICMD_ANEWARRAY:

				case ICMD_GETFIELD:
					FREE_TEMP_REG(iptr->s1.varindex);
					NEW_TEMP_REG(iptr->dst.varindex);
					break;

					/* pop 0 push 1 */

				case ICMD_GETSTATIC:

				case ICMD_NEW:
					NEW_TEMP_REG(iptr->dst.varindex);
					break;

					/* pop many push any */

				case ICMD_INVOKESTATIC:
				case ICMD_INVOKESPECIAL:
				case ICMD_INVOKEVIRTUAL:
				case ICMD_INVOKEINTERFACE:
					INSTRUCTION_GET_METHODDESC(iptr,md);
					i = md->paramcount;
					argp = iptr->sx.s23.s2.args;
					while (--i >= 0) {
						FREE_TEMP_REG(*argp);
						argp++;
					}
					if (md->returntype.type != TYPE_VOID)
						NEW_TEMP_REG(iptr->dst.varindex);
					break;

				case ICMD_BUILTIN:
					bte = iptr->sx.s23.s3.bte;
					md = bte->md;
					i = md->paramcount;
					argp = iptr->sx.s23.s2.args;
					while (--i >= 0) {
						FREE_TEMP_REG(*argp);
						argp++;
					}
					if (md->returntype.type != TYPE_VOID)
						NEW_TEMP_REG(iptr->dst.varindex);
					break;

				case ICMD_MULTIANEWARRAY:
					i = iptr->s1.argcount;
					argp = iptr->sx.s23.s2.args;
					while (--i >= 0) {
						FREE_TEMP_REG(*argp);
						argp++;
					}
					NEW_TEMP_REG(iptr->dst.varindex);
					break;

				default:
					exceptions_throw_internalerror("Unknown ICMD %d during register allocation",
												   iptr->opc);
					return;
				} /* switch */
				iptr++;
			} /* while instructions */
		} /* if */
		bptr = bptr->next;
	} /* while blocks */
}


#if defined(ENABLE_STATISTICS)
void simplereg_make_statistics(jitdata *jd)
{
	//methodinfo   *m;
	//codegendata  *cd;
	//registerdata *rd;
	int i;
	s4 len;
#if 0
	stackelement_t*    src, src_old;
	stackelement_t*    dst;
	instruction *iptr;
#endif
	basicblock  *bptr;
	int size_interface; /* == maximum size of in/out stack at basic block boundaries */
	bool in_register;
	//varinfo *var;

	/* get required compiler data */

	//m  = jd->m;
	//cd = jd->cd;
	//rd = jd->rd;

	in_register = true;

	size_interface = 0;

		/* count how many local variables are held in memory or register */
		for(i=0; i < jd->localcount; i++) {
			if (VAR(i)->flags & INMEMORY) {
				STATISTICS(count_locals_spilled++);
				in_register=false;
			}
			else {
				STATISTICS(count_locals_register++);
			}
		}

		/* count how many stack slots are held in memory or register */

		bptr = jd->basicblocks;

		while (bptr != NULL) {
			if (bptr->state >= basicblock::REACHED) {

#if defined(ENABLE_LSRA) || defined(ENABLE_SSA)
			if (!opt_lsra) {
#endif
				/* check for memory moves from interface to BB instack */
				len = bptr->indepth;

				if (len > size_interface) size_interface = len;

				while (len) {
					len--;
					//var = VAR(bptr->invars[len]);

					/* invars statistics (currently none) */
				}

				/* check for memory moves from BB outstack to interface */
				len = bptr->outdepth;
				if (len > size_interface) size_interface = len;

				while (len) {
					len--;
					//var = VAR(bptr->outvars[len]);

					/* outvars statistics (currently none) */
				}
#if defined(ENABLE_LSRA) || defined(ENABLE_SSA)
			}
#endif


#if 0
				dst = bptr->instack;
				iptr = bptr->iinstr;
				len = bptr->icount;
				src_old = NULL;

				while (--len >= 0)  {
					src = dst;
					dst = iptr->dst.var;

					if ((src!= NULL) && (src != src_old)) { /* new stackslot */
						switch (src->varkind) {
						case TEMPVAR:
						case STACKVAR:
							if (!(src->flags & INMEMORY))
								STATISTICS(count_ss_register++);
							else {
								STATISTICS(count_ss_spilled++);
								in_register=false;
							}
							break;
							/* 					case LOCALVAR: */
							/* 						if (!(rd->locals[src->varnum][src->type].flags & INMEMORY)) */
							/* 							STATISTICS(count_ss_register++); */
							/* 						else */
							/* 							STATISTICS(count_ss_spilled++); */
							/* 						break; */
						case ARGVAR:
							if (!(src->flags & INMEMORY))
								STATISTICS(count_argument_mem_ss++);
							else
								STATISTICS(count_argument_reg_ss++);
							break;


							/* 						if (IS_FLT_DBL_TYPE(src->type)) { */
							/* 							if (src->varnum < FLT_ARG_CNT) { */
							/* 								STATISTICS(count_ss_register++); */
							/* 								break; */
							/* 							} */
							/* 						} else { */
							/* #if defined(__POWERPC__) */
							/* 							if (src->varnum < INT_ARG_CNT - (IS_2_WORD_TYPE(src->type) != 0)) { */
							/* #else */
							/* 							if (src->varnum < INT_ARG_CNT) { */
							/* #endif */
							/*								STATISTICS(count_ss_register++); */
							/* 								break; */
							/* 							} */
							/* 						} */
							/* 						STATISTICS(count_ss_spilled++); */
							/* 						break; */
						}
					}
					src_old = src;

					iptr++;
				} /* while instructions */
#endif
			} /* if */

			bptr = bptr->next;
		} /* while blocks */

		STATISTICS(count_interface_size += size_interface); /* accummulate the size of the interface (between bb boundaries) */
		if (in_register) {
			STATISTICS(count_method_in_register++);
/* 			printf("INREGISTER: %s%s%s\n",UTF_TEXT(m->class->name), UTF_TEXT(m->name), UTF_TEXT(m->descriptor)); */
		}
}
#endif /* defined(ENABLE_STATISTICS) */


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
