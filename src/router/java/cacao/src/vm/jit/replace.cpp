/* src/vm/jit/replace.cpp - on-stack replacement of methods

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

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "arch.hpp"
#include "md.hpp"

#if defined(ENABLE_GC_CACAO)
# include "mm/cacao-gc/gc.h"
#endif

#include "mm/dumpmemory.hpp"
#include "mm/memory.hpp"

#include "threads/thread.hpp"

#include "toolbox/logging.hpp"

#include "vm/classcache.hpp"
#include "vm/descriptor.hpp"
#include "vm/globals.hpp"
#include "vm/options.hpp"
#include "vm/string.hpp"

#if defined(ENABLE_RT_TIMING)
# include "vm/rt-timing.hpp"
#endif

#include "vm/jit/abi.hpp"
#include "vm/jit/asmpart.hpp"
#include "vm/jit/disass.hpp"
#include "vm/jit/executionstate.hpp"
#include "vm/jit/jit.hpp"
#include "vm/jit/methodheader.hpp"
#include "vm/jit/replace.hpp"
#include "vm/jit/show.hpp"
#include "vm/jit/stack.hpp"
#include "vm/jit/stacktrace.hpp"

#include "vm/jit/ir/instruction.hpp"


#define REPLACE_PATCH_DYNAMIC_CALL
/*#define REPLACE_PATCH_ALL*/


/*** debugging ****************************************************************/

#if !defined(NDEBUG)
static void java_value_print(s4 type, replace_val_t value);
static void replace_stackframeinfo_println(stackframeinfo_t *sfi);
#endif

#if !defined(NDEBUG)
#define DOLOG(code)        do{ if (opt_TraceReplacement > 1) { code; } } while(0)
#define DOLOG_SHORT(code)  do{ if (opt_TraceReplacement > 0) { code; } } while(0)
#else
#define DOLOG(code)
#define DOLOG_SHORT(code)
#endif


/*** statistics ***************************************************************/

#define REPLACE_STATISTICS

#if defined(REPLACE_STATISTICS)

static int stat_replacements = 0;
static int stat_frames = 0;
static int stat_recompile = 0;
static int stat_staticpatch = 0;
static int stat_unroll_inline = 0;
static int stat_unroll_call = 0;
static int stat_dist_frames[20] = { 0 };
static int stat_dist_locals[20] = { 0 };
static int stat_dist_locals_adr[10] = { 0 };
static int stat_dist_locals_prim[10] = { 0 };
static int stat_dist_locals_ret[10] = { 0 };
static int stat_dist_locals_void[10] = { 0 };
static int stat_dist_stack[10] = { 0 };
static int stat_dist_stack_adr[10] = { 0 };
static int stat_dist_stack_prim[10] = { 0 };
static int stat_dist_stack_ret[10] = { 0 };
static int stat_methods = 0;
static int stat_rploints = 0;
static int stat_regallocs = 0;
static int stat_dist_method_rplpoints[20] = { 0 };

#define REPLACE_COUNT(cnt)  (cnt)++
#define REPLACE_COUNT_IF(cnt, cond)  do{ if(cond) (cnt)++; } while(0)
#define REPLACE_COUNT_INC(cnt, inc)  ((cnt) += (inc))

#define REPLACE_COUNT_DIST(array, val)                               \
    do {                                                             \
        int limit = (sizeof(array) / sizeof(int)) - 1;               \
        if ((val) < (limit)) (array)[val]++;                         \
        else (array)[limit]++;                                       \
    } while (0)

static void replace_statistics_source_frame(sourceframe_t *frame);

#else

#define REPLACE_COUNT(cnt)
#define REPLACE_COUNT_IF(cnt, cond)
#define REPLACE_COUNT_INC(cnt, inc)
#define REPLACE_COUNT_DIST(array, val)

#endif /* defined(REPLACE_STATISTICS) */


/*** constants used internally ************************************************/

#define TOP_IS_NORMAL    0
#define TOP_IS_ON_STACK  1
#define TOP_IS_IN_ITMP1  2
#define TOP_IS_VOID      3


/******************************************************************************/
/* PART I: Creating / freeing replacement points                              */
/******************************************************************************/


/* replace_create_replacement_point ********************************************

   Create a replacement point.

   IN:
       jd...............current jitdata
	   iinfo............inlining info for the current position
	   rp...............pre-allocated (uninitialized) rplpoint
	   type.............rplpoint::TYPE constant
	   iptr.............current instruction
	   *pra.............current rplalloc pointer
	   javalocals.......the javalocals at the current point
	   stackvars........the stack variables at the current point
	   stackdepth.......the stack depth at the current point
	   paramcount.......number of parameters at the start of stackvars

   OUT:
       *rpa.............points to the next free rplalloc

*******************************************************************************/

static void replace_create_replacement_point(jitdata *jd,
											 insinfo_inline *iinfo,
											 rplpoint *rp,
											 rplpoint::Type type,
											 instruction *iptr,
											 rplalloc **pra,
											 s4 *javalocals,
											 s4 *stackvars,
											 s4 stackdepth,
											 s4 paramcount)
{
	rplalloc *ra;
	s4        i;
	varinfo  *v;
	s4        index;

	ra = *pra;

	REPLACE_COUNT(stat_rploints);

	rp->method   = (iinfo) ? iinfo->method : jd->m;
	rp->pc       = NULL;        /* set by codegen */
	rp->callsize = 0;     /* set by codegen */
	rp->regalloc = ra;
	rp->flags    = 0;
	rp->type     = type;
	rp->id       = iptr->flags.bits >> INS_FLAG_ID_SHIFT;

	/* XXX unify these two fields */
	rp->parent = (iinfo) ? iinfo->rp : NULL;

	/* store local allocation info of javalocals */

	if (javalocals) {
		for (i = 0; i < rp->method->maxlocals; ++i) {
			index = javalocals[i];
			if (index == jitdata::UNUSED)
				continue;

			ra->index = i;
			if (index >= 0) {
				v = VAR(index);
				ra->flags  = v->flags & (INMEMORY);
				ra->regoff = v->vv.regoff;
				ra->type   = v->type;
			}
			else {
				ra->regoff = RETADDR_FROM_JAVALOCAL(index);
				ra->type = TYPE_RET;
				ra->flags = 0;
			}
			ra++;
		}
	}

	/* store allocation info of java stack vars */

	for (i = 0; i < stackdepth; ++i) {
		v = VAR(stackvars[i]);
		ra->flags = v->flags & (INMEMORY);
		ra->index = (i < paramcount) ? RPLALLOC_PARAM : RPLALLOC_STACK;
		ra->type  = v->type;
		/* XXX how to handle locals on the stack containing returnAddresses? */
		if (v->type == TYPE_RET) {
			assert(stackvars[i] >= jd->localcount);
			ra->regoff = v->vv.retaddr->nr;
		}
		else
			ra->regoff = v->vv.regoff;
		ra++;
	}

	/* total number of allocations */

	rp->regalloccount = ra - rp->regalloc;

	*pra = ra;
}


/* replace_create_inline_start_replacement_point *******************************

   Create an INLINE_START replacement point.

   IN:
       jd...............current jitdata
	   rp...............pre-allocated (uninitialized) rplpoint
	   iptr.............current instruction
	   *pra.............current rplalloc pointer
	   javalocals.......the javalocals at the current point

   OUT:
       *rpa.............points to the next free rplalloc

   RETURN VALUE:
       the insinfo_inline * for the following inlined body

*******************************************************************************/

static insinfo_inline * replace_create_inline_start_replacement_point(
											 jitdata *jd,
											 rplpoint *rp,
											 instruction *iptr,
											 rplalloc **pra,
											 s4 *javalocals)
{
	insinfo_inline *calleeinfo;
	rplalloc       *ra;

	calleeinfo = iptr->sx.s23.s3.inlineinfo;

	calleeinfo->rp = rp;

	replace_create_replacement_point(jd, calleeinfo->parent, rp,
			rplpoint::TYPE_INLINE, iptr, pra,
			javalocals,
			calleeinfo->stackvars, calleeinfo->stackvarscount,
			calleeinfo->paramcount);

	if (calleeinfo->synclocal != jitdata::UNUSED) {
		ra = (*pra)++;
		ra->index  = RPLALLOC_SYNC;
		ra->regoff = jd->var[calleeinfo->synclocal].vv.regoff;
		ra->flags  = jd->var[calleeinfo->synclocal].flags & INMEMORY;
		ra->type   = TYPE_ADR;

		rp->regalloccount++;
	}

	return calleeinfo;
}


/* replace_create_replacement_points *******************************************

   Create the replacement points for the given code.

   IN:
       jd...............current jitdata, must not have any replacement points

   OUT:
       code->rplpoints.......set to the list of replacement points
	   code->rplpointcount...number of replacement points
	   code->regalloc........list of allocation info
	   code->regalloccount...total length of allocation info list
	   code->globalcount.....number of global allocations at the
	                         start of code->regalloc

   RETURN VALUE:
       true.............everything ok
       false............an exception has been thrown

*******************************************************************************/

#define CLEAR_javalocals(array, method)                              \
    do {                                                             \
        for (i=0; i<(method)->maxlocals; ++i)                        \
            (array)[i] = jitdata::UNUSED;                                     \
    } while (0)

#define COPY_OR_CLEAR_javalocals(dest, array, method)                \
    do {                                                             \
        if ((array) != NULL)                                         \
            MCOPY((dest), (array), s4, (method)->maxlocals);         \
        else                                                         \
            CLEAR_javalocals((dest), (method));                      \
    } while (0)

#define COUNT_javalocals(array, method, counter)                     \
    do {                                                             \
        for (i=0; i<(method)->maxlocals; ++i)                        \
            if ((array)[i] != jitdata::UNUSED)                                \
				(counter)++;                                         \
    } while (0)

bool replace_create_replacement_points(jitdata *jd)
{
	codeinfo        *code;
	registerdata    *rd;
	basicblock      *bptr;
	int              count;
	methodinfo      *m;
	rplpoint        *rplpoints;
	rplpoint        *rp;
	int              alloccount;
	rplalloc        *regalloc;
	rplalloc        *ra;
	int              i;
	instruction     *iptr;
	instruction     *iend;
	s4              *javalocals;
	s4              *jl;
	methoddesc      *md;
	insinfo_inline  *iinfo;
	s4               startcount;
	s4               firstcount;
#if defined(REPLACE_PATCH_DYNAMIC_CALL)
	bool             needentry;
#endif

	REPLACE_COUNT(stat_methods);

	/* get required compiler data */

	code = jd->code;
	rd   = jd->rd;

	/* assert that we wont overwrite already allocated data */

	assert(code);
	assert(code->m);
	assert(code->rplpoints == NULL);
	assert(code->rplpointcount == 0);
	assert(code->regalloc == NULL);
	assert(code->regalloccount == 0);
	assert(code->globalcount == 0);

	m = code->m;

	/* in instance methods, we may need a rplpoint at the method entry */

#if defined(REPLACE_PATCH_DYNAMIC_CALL)
	if (!(m->flags & ACC_STATIC)) {
		jd->basicblocks[0].bitflags |= BBFLAG_REPLACEMENT;
		needentry = true;
	}
	else {
		needentry = false;
	}
#endif /* defined(REPLACE_PATCH_DYNAMIC_CALL) */

	/* iterate over the basic block list to find replacement points */

	count = 0;
	alloccount = 0;

	javalocals = (s4*) DumpMemory::allocate(sizeof(s4) * jd->maxlocals);

	for (bptr = jd->basicblocks; bptr; bptr = bptr->next) {

		/* skip dead code */

		if (bptr->state < basicblock::FINISHED)
			continue;

		/* get info about this block */

		m = bptr->method;
		iinfo = bptr->inlineinfo;

		/* initialize javalocals at the start of this block */

		COPY_OR_CLEAR_javalocals(javalocals, bptr->javalocals, m);

		/* iterate over the instructions */

		iptr = bptr->iinstr;
		iend = iptr + bptr->icount;
		startcount = count;
		firstcount = count;

		for (; iptr != iend; ++iptr) {
			switch (iptr->opc) {
#if defined(ENABLE_GC_CACAO)
				case ICMD_BUILTIN:
					md = iptr->sx.s23.s3.bte->md;
					count++;
					COUNT_javalocals(javalocals, m, alloccount);
					alloccount += iptr->s1.argcount;
					if (iinfo)
						alloccount -= iinfo->throughcount;
					break;
#endif

				case ICMD_INVOKESTATIC:
				case ICMD_INVOKESPECIAL:
				case ICMD_INVOKEVIRTUAL:
				case ICMD_INVOKEINTERFACE:
					INSTRUCTION_GET_METHODDESC(iptr, md);
					count++;
					COUNT_javalocals(javalocals, m, alloccount);
					alloccount += iptr->s1.argcount;
					if (iinfo)
						alloccount -= iinfo->throughcount;
					break;

				case ICMD_ISTORE:
				case ICMD_LSTORE:
				case ICMD_FSTORE:
				case ICMD_DSTORE:
				case ICMD_ASTORE:
					stack_javalocals_store(iptr, javalocals);
					break;

				case ICMD_IRETURN:
				case ICMD_LRETURN:
				case ICMD_FRETURN:
				case ICMD_DRETURN:
				case ICMD_ARETURN:
					alloccount += 1;
					/* FALLTHROUGH! */
				case ICMD_RETURN:
					count++;
					break;

				case ICMD_INLINE_START:
					iinfo = iptr->sx.s23.s3.inlineinfo;

					count++;
					COUNT_javalocals(javalocals, m, alloccount);
					alloccount += iinfo->stackvarscount;
					if (iinfo->synclocal != jitdata::UNUSED)
						alloccount++;

					m = iinfo->method;
					/* javalocals may be set at next block start, or now */
					COPY_OR_CLEAR_javalocals(javalocals, iinfo->javalocals_start, m);
					break;

				case ICMD_INLINE_BODY:
					assert(iinfo == iptr->sx.s23.s3.inlineinfo);

					jl = iinfo->javalocals_start;
					if (jl == NULL) {
						/* get the javalocals from the following block start */
						assert(bptr->next);
						jl = bptr->next->javalocals;
					}
					count++;
					COUNT_javalocals(jl, m, alloccount);
					break;

				case ICMD_INLINE_END:
					assert(iinfo == iptr->sx.s23.s3.inlineinfo ||
						   iinfo == iptr->sx.s23.s3.inlineinfo->parent);
					iinfo = iptr->sx.s23.s3.inlineinfo;
					m = iinfo->outer;
					if (iinfo->javalocals_end)
						MCOPY(javalocals, iinfo->javalocals_end, s4, m->maxlocals);
					iinfo = iinfo->parent;
					break;
				default:
					break;
			}

			if (iptr == bptr->iinstr)
				firstcount = count;
		} /* end instruction loop */

		/* create replacement points at targets of backward branches */
		/* We only need the replacement point there, if there is no  */
		/* replacement point inside the block.                       */

		if (bptr->bitflags & BBFLAG_REPLACEMENT) {
#if defined(REPLACE_PATCH_DYNAMIC_CALL)
			int test = (needentry && bptr == jd->basicblocks) ? firstcount : count;
#else
			int test = count;
#endif
			if (test > startcount) {
				/* we don't need an extra rplpoint */
				bptr->bitflags &= ~BBFLAG_REPLACEMENT;
			}
			else {
				count++;
				alloccount += bptr->indepth;
				if (bptr->inlineinfo)
					alloccount -= bptr->inlineinfo->throughcount;

				COUNT_javalocals(bptr->javalocals, bptr->method, alloccount);
			}
		}

	} /* end basicblock loop */

	/* if no points were found, there's nothing to do */

	if (!count)
		return true;

	/* allocate replacement point array and allocation array */

	rplpoints = MNEW(rplpoint, count);
	regalloc = MNEW(rplalloc, alloccount);
	ra = regalloc;

	/* initialize replacement point structs */

	rp = rplpoints;

	/* XXX try to share code with the counting loop! */

	for (bptr = jd->basicblocks; bptr; bptr = bptr->next) {
		/* skip dead code */

		if (bptr->state < basicblock::FINISHED)
			continue;

		/* get info about this block */

		m = bptr->method;
		iinfo = bptr->inlineinfo;

		/* initialize javalocals at the start of this block */

		COPY_OR_CLEAR_javalocals(javalocals, bptr->javalocals, m);

		/* create replacement points at targets of backward branches */

		if (bptr->bitflags & BBFLAG_REPLACEMENT) {

			i = (iinfo) ? iinfo->throughcount : 0;
			replace_create_replacement_point(jd, iinfo, rp++,
					(rplpoint::Type) bptr->type, bptr->iinstr, &ra,
					bptr->javalocals, bptr->invars + i, bptr->indepth - i, 0);

			if (JITDATA_HAS_FLAG_COUNTDOWN(jd))
				rp[-1].flags |= rplpoint::FLAG_COUNTDOWN;
		}

		/* iterate over the instructions */

		iptr = bptr->iinstr;
		iend = iptr + bptr->icount;

		for (; iptr != iend; ++iptr) {
			switch (iptr->opc) {
#if defined(ENABLE_GC_CACAO)
				case ICMD_BUILTIN:
					md = iptr->sx.s23.s3.bte->md;

					i = (iinfo) ? iinfo->throughcount : 0;
					replace_create_replacement_point(jd, iinfo, rp++,
							rplpoint::TYPE_CALL, iptr, &ra,
							javalocals, iptr->sx.s23.s2.args,
							iptr->s1.argcount - i,
							md->paramcount);
					break;
#endif

				case ICMD_INVOKESTATIC:
				case ICMD_INVOKESPECIAL:
				case ICMD_INVOKEVIRTUAL:
				case ICMD_INVOKEINTERFACE:
					INSTRUCTION_GET_METHODDESC(iptr, md);

					i = (iinfo) ? iinfo->throughcount : 0;
					replace_create_replacement_point(jd, iinfo, rp++,
							rplpoint::TYPE_CALL, iptr, &ra,
							javalocals, iptr->sx.s23.s2.args,
							iptr->s1.argcount - i,
							md->paramcount);
					break;

				case ICMD_ISTORE:
				case ICMD_LSTORE:
				case ICMD_FSTORE:
				case ICMD_DSTORE:
				case ICMD_ASTORE:
					stack_javalocals_store(iptr, javalocals);
					break;

				case ICMD_IRETURN:
				case ICMD_LRETURN:
				case ICMD_FRETURN:
				case ICMD_DRETURN:
				case ICMD_ARETURN:
					replace_create_replacement_point(jd, iinfo, rp++,
							rplpoint::TYPE_RETURN, iptr, &ra,
							NULL, &(iptr->s1.varindex), 1, 0);
					break;

				case ICMD_RETURN:
					replace_create_replacement_point(jd, iinfo, rp++,
							rplpoint::TYPE_RETURN, iptr, &ra,
							NULL, NULL, 0, 0);
					break;

				case ICMD_INLINE_START:
					iinfo = replace_create_inline_start_replacement_point(
								jd, rp++, iptr, &ra, javalocals);
					m = iinfo->method;
					/* javalocals may be set at next block start, or now */
					COPY_OR_CLEAR_javalocals(javalocals, iinfo->javalocals_start, m);
					break;

				case ICMD_INLINE_BODY:
					assert(iinfo == iptr->sx.s23.s3.inlineinfo);

					jl = iinfo->javalocals_start;
					if (jl == NULL) {
						/* get the javalocals from the following block start */
						assert(bptr->next);
						jl = bptr->next->javalocals;
					}
					/* create a non-trappable rplpoint */
					replace_create_replacement_point(jd, iinfo, rp++,
							rplpoint::TYPE_BODY, iptr, &ra,
							jl, NULL, 0, 0);
					rp[-1].flags |= rplpoint::FLAG_NOTRAP;
					break;

				case ICMD_INLINE_END:
					assert(iinfo == iptr->sx.s23.s3.inlineinfo ||
						   iinfo == iptr->sx.s23.s3.inlineinfo->parent);
					iinfo = iptr->sx.s23.s3.inlineinfo;
					m = iinfo->outer;
					if (iinfo->javalocals_end)
						MCOPY(javalocals, iinfo->javalocals_end, s4, m->maxlocals);
					iinfo = iinfo->parent;
					break;
				default:
					break;
			}
		} /* end instruction loop */
	} /* end basicblock loop */

	assert((rp - rplpoints) == count);
	assert((ra - regalloc) == alloccount);

	/* store the data in the codeinfo */

	code->rplpoints     = rplpoints;
	code->rplpointcount = count;
	code->regalloc      = regalloc;
	code->regalloccount = alloccount;
	code->globalcount   = 0;
	code->memuse        = rd->memuse;

	REPLACE_COUNT_DIST(stat_dist_method_rplpoints, count);
	REPLACE_COUNT_INC(stat_regallocs, alloccount);

	/* everything alright */

	return true;
}


/* replace_free_replacement_points *********************************************

   Free memory used by replacement points.

   IN:
       code.............codeinfo whose replacement points should be freed.

*******************************************************************************/

void replace_free_replacement_points(codeinfo *code)
{
	assert(code);

	if (code->rplpoints)
		MFREE(code->rplpoints,rplpoint,code->rplpointcount);

	if (code->regalloc)
		MFREE(code->regalloc,rplalloc,code->regalloccount);

	code->rplpoints = NULL;
	code->rplpointcount = 0;
	code->regalloc = NULL;
	code->regalloccount = 0;
	code->globalcount = 0;
}


/******************************************************************************/
/* PART II: Activating / deactivating replacement points                      */
/******************************************************************************/


/* replace_activate_replacement_points *****************************************

   Activate the replacement points of the given compilation unit. When this
   function returns, the replacement points are "armed", so each thread
   reaching one of the points will enter the replacement mechanism.

   IN:
       code.............codeinfo of which replacement points should be
						activated
	   mappable.........if true, only mappable replacement points are
						activated

*******************************************************************************/

void replace_activate_replacement_points(codeinfo *code, bool mappable)
{
	rplpoint *rp;
	s4        i;
	s4        count;
	u1       *savedmcode;

	assert(code->savedmcode == NULL);

	/* count trappable replacement points */

	count = 0;
	i = code->rplpointcount;
	rp = code->rplpoints;
	for (; i--; rp++) {
		if (rp->flags & rplpoint::FLAG_NOTRAP)
			continue;

		if (mappable && (rp->type == rplpoint::TYPE_RETURN))
			continue;

		count++;
	}

	/* allocate buffer for saved machine code */

	savedmcode = MNEW(u1, count * REPLACEMENT_PATCH_SIZE);
	code->savedmcode = savedmcode;
	savedmcode += count * REPLACEMENT_PATCH_SIZE;

	/* activate trappable replacement points */
	/* (in reverse order to handle overlapping points within basic blocks) */

	i = code->rplpointcount;
	rp = code->rplpoints + i;
	while (rp--, i--) {
		assert(!(rp->flags & rplpoint::FLAG_ACTIVE));

		if (rp->flags & rplpoint::FLAG_NOTRAP)
			continue;

		if (mappable && (rp->type == rplpoint::TYPE_RETURN))
			continue;

		DOLOG( printf("activate replacement point:\n");
			   replace_replacement_point_println(rp, 1); fflush(stdout); );

		savedmcode -= REPLACEMENT_PATCH_SIZE;

#if defined(ENABLE_JIT)
# if defined(ENABLE_DISASSEMBLER)
		DOLOG( printf("\tinstruction before: ");
			   disassinstr(rp->pc); fflush(stdout); );
# endif

		md_patch_replacement_point(rp->pc, savedmcode, false);

# if defined(ENABLE_DISASSEMBLER)
		DOLOG( printf("\tinstruction after : ");
			   disassinstr(rp->pc); fflush(stdout); );
# endif
#endif

		rp->flags |= rplpoint::FLAG_ACTIVE;
	}

	assert(savedmcode == code->savedmcode);
}


/* replace_deactivate_replacement_points ***************************************

   Deactivate a replacement points in the given compilation unit.
   When this function returns, the replacement points will be "un-armed",
   that is a each thread reaching a point will just continue normally.

   IN:
       code.............the compilation unit

*******************************************************************************/

void replace_deactivate_replacement_points(codeinfo *code)
{
	rplpoint *rp;
	s4        i;
	s4        count;
	u1       *savedmcode;

	if (code->savedmcode == NULL) {
		/* disarm countdown points by patching the branches */

		i = code->rplpointcount;
		rp = code->rplpoints;
		for (; i--; rp++) {
			if ((rp->flags & (rplpoint::FLAG_ACTIVE | rplpoint::FLAG_COUNTDOWN))
					== rplpoint::FLAG_COUNTDOWN)
			{
#if 0
				*(s4*) (rp->pc + 9) = 0; /* XXX machine dependent! */
#endif
			}
		}
		return;
	}

	assert(code->savedmcode != NULL);
	savedmcode = code->savedmcode;

	/* de-activate each trappable replacement point */

	i = code->rplpointcount;
	rp = code->rplpoints;
	count = 0;
	for (; i--; rp++) {
		if (!(rp->flags & rplpoint::FLAG_ACTIVE))
			continue;

		count++;

		DOLOG( printf("deactivate replacement point:\n");
			   replace_replacement_point_println(rp, 1); fflush(stdout); );

#if defined(ENABLE_JIT)
# if defined(ENABLE_DISASSEMBLER)
		DOLOG( printf("\tinstruction before: ");
			   disassinstr(rp->pc); fflush(stdout); );
# endif

		md_patch_replacement_point(rp->pc, savedmcode, true);

# if defined(ENABLE_DISASSEMBLER)
		DOLOG( printf("\tinstruction before: ");
			   disassinstr(rp->pc); fflush(stdout); );
# endif
#endif

		rp->flags &= ~rplpoint::FLAG_ACTIVE;

		savedmcode += REPLACEMENT_PATCH_SIZE;
	}

	assert(savedmcode == code->savedmcode + count * REPLACEMENT_PATCH_SIZE);

	/* free saved machine code */

	MFREE(code->savedmcode, u1, count * REPLACEMENT_PATCH_SIZE);
	code->savedmcode = NULL;
}


/******************************************************************************/
/* PART III: The replacement mechanism                                        */
/******************************************************************************/


/* replace_read_value **********************************************************

   Read a value with the given allocation from the execution state.

   IN:
	   es...............execution state
	   ra...............allocation
	   javaval..........where to put the value

   OUT:
       *javaval.........the value

*******************************************************************************/

static void replace_read_value(executionstate_t *es,
							   rplalloc *ra,
							   replace_val_t *javaval)
{
	if (ra->flags & INMEMORY) {
		/* XXX HAS_4BYTE_STACKSLOT may not be the right discriminant here */
#ifdef HAS_4BYTE_STACKSLOT
		if (IS_2_WORD_TYPE(ra->type)) {
			javaval->l = *(u8*)(es->sp + ra->regoff);
		}
		else {
#endif
			javaval->p = *(ptrint*)(es->sp + ra->regoff);
#ifdef HAS_4BYTE_STACKSLOT
		}
#endif
	}
	else {
		/* allocated register */
		if (IS_FLT_DBL_TYPE(ra->type)) {
			javaval->d = es->fltregs[ra->regoff];

			if (ra->type == TYPE_FLT)
				javaval->f = javaval->d;
		}
		else {
#if defined(SUPPORT_COMBINE_INTEGER_REGISTERS)
			if (ra->type == TYPE_LNG) {
				javaval->words.lo = es->intregs[GET_LOW_REG(ra->regoff)];
				javaval->words.hi = es->intregs[GET_HIGH_REG(ra->regoff)];
			}
			else
#endif /* defined(SUPPORT_COMBINE_INTEGER_REGISTERS) */
				javaval->p = es->intregs[ra->regoff];
		}
	}
}


/* replace_write_value *********************************************************

   Write a value to the given allocation in the execution state.

   IN:
	   es...............execution state
	   ra...............allocation
	   *javaval.........the value

*******************************************************************************/

static void replace_write_value(executionstate_t *es,
							    rplalloc *ra,
							    replace_val_t *javaval)
{
	if (ra->flags & INMEMORY) {
		/* XXX HAS_4BYTE_STACKSLOT may not be the right discriminant here */
#ifdef HAS_4BYTE_STACKSLOT
		if (IS_2_WORD_TYPE(ra->type)) {
			*(u8*)(es->sp + ra->regoff) = javaval->l;
		}
		else {
#endif
			*(ptrint*)(es->sp + ra->regoff) = javaval->p;
#ifdef HAS_4BYTE_STACKSLOT
		}
#endif
	}
	else {
		/* allocated register */
		switch (ra->type) {
			case TYPE_FLT:
				es->fltregs[ra->regoff] = (double) javaval->f;
				break;
			case TYPE_DBL:
				es->fltregs[ra->regoff] = javaval->d;
				break;
#if defined(SUPPORT_COMBINE_INTEGER_REGISTERS)
			case TYPE_LNG:
				es->intregs[GET_LOW_REG(ra->regoff)] = javaval->words.lo;
				es->intregs[GET_HIGH_REG(ra->regoff)] = javaval->words.hi;
				break;
#endif
			default:
				es->intregs[ra->regoff] = javaval->p;
		}
	}
}


/* replace_new_sourceframe *****************************************************

   Allocate a new source frame and insert it at the front of the frame list.

   IN:
	   ss...............the source state

   OUT:
	   ss->frames.......set to new frame (the new head of the frame list).

   RETURN VALUE:
       returns the new frame

*******************************************************************************/

static sourceframe_t *replace_new_sourceframe(sourcestate_t *ss)
{
	sourceframe_t *frame;

	frame = (sourceframe_t*) DumpMemory::allocate(sizeof(sourceframe_t));
	MZERO(frame, sourceframe_t, 1);

	frame->down = ss->frames;
	ss->frames = frame;

	return frame;
}


/* replace_read_executionstate *************************************************

   Read a source frame from the given executions state.
   The new source frame is pushed to the front of the frame list of the
   source state.

   IN:
       rp...............replacement point at which `es` was taken
	   es...............execution state
	   ss...............the source state to add the source frame to
	   topframe.........true, if the first (top-most) source frame on the
	                    stack is to be read

   OUT:
       *ss..............the source state with the newly created source frame
	                    added

*******************************************************************************/

static s4 replace_normalize_type_map[] = {
/* rplpoint::TYPE_STD    |--> */ rplpoint::TYPE_STD,
/* rplpoint::TYPE_EXH    |--> */ rplpoint::TYPE_STD,
/* rplpoint::TYPE_SBR    |--> */ rplpoint::TYPE_STD,
/* rplpoint::TYPE_CALL   |--> */ rplpoint::TYPE_CALL,
/* rplpoint::TYPE_INLINE |--> */ rplpoint::TYPE_CALL,
/* rplpoint::TYPE_RETURN |--> */ rplpoint::TYPE_RETURN,
/* rplpoint::TYPE_BODY   |--> */ rplpoint::TYPE_STD
};


static void replace_read_executionstate(rplpoint *rp,
										executionstate_t *es,
										sourcestate_t *ss,
										bool topframe)
{
	methodinfo    *m;
	codeinfo      *code;
	int            count;
	int            i;
	rplalloc      *ra;
	sourceframe_t *frame;
	int            topslot;
	stackslot_t   *sp;
#if defined(__I386__)
	stackslot_t   *basesp;
#endif

	code = code_find_codeinfo_for_pc(rp->pc);
	m = rp->method;
	topslot = TOP_IS_NORMAL;

	/* stack pointer */

	sp = (stackslot_t *) es->sp;

	/* in some cases the top stack slot is passed in REG_ITMP1 */

	if (rp->type == rplpoint::TYPE_EXH) {
		topslot = TOP_IS_IN_ITMP1;
	}

	/* calculate base stack pointer */

#if defined(__I386__)
	basesp = sp + code->stackframesize;
#endif

	/* create the source frame */

	frame = replace_new_sourceframe(ss);
	frame->method = rp->method;
	frame->id = rp->id;
	assert(rp->type >= 0 && rp->type < sizeof(replace_normalize_type_map)/sizeof(s4));
	frame->type = replace_normalize_type_map[rp->type];
	frame->fromrp = rp;
	frame->fromcode = code;

	/* read local variables */

	count = m->maxlocals;
	frame->javalocalcount = count;
	frame->javalocals = (replace_val_t*) DumpMemory::allocate(sizeof(replace_val_t) * count);
	frame->javalocaltype = (u1*) DumpMemory::allocate(sizeof(u1) * count);

	/* mark values as undefined */
	for (i=0; i<count; ++i) {
#if !defined(NDEBUG)
		frame->javalocals[i].l = (u8) 0x00dead0000dead00ULL;
#endif
		frame->javalocaltype[i] = TYPE_VOID;
	}

	/* some entries in the intregs array are not meaningful */
	/*es->intregs[REG_ITMP3] = (u8) 0x11dead1111dead11ULL;*/
#if !defined(NDEBUG)
	es->intregs[REG_SP   ] = (ptrint) 0x11dead1111dead11ULL;
#ifdef REG_PV
	es->intregs[REG_PV   ] = (ptrint) 0x11dead1111dead11ULL;
#endif
#endif /* !defined(NDEBUG) */

	/* read javalocals */

	count = rp->regalloccount;
	ra = rp->regalloc;

	while (count && (i = ra->index) >= 0) {
		assert(i < m->maxlocals);
		frame->javalocaltype[i] = ra->type;
		if (ra->type == TYPE_RET)
			frame->javalocals[i].i = ra->regoff;
		else
			replace_read_value(es, ra, frame->javalocals + i);
		ra++;
		count--;
	}

	/* read instance, if this is the first rplpoint */

#if defined(REPLACE_PATCH_DYNAMIC_CALL)
	if (topframe && !(rp->method->flags & ACC_STATIC) && rp == code->rplpoints) {
#if 1
		/* we are at the start of the method body, so if local 0 is set, */
		/* it is the instance.                                           */
		if (frame->javalocaltype[0] == TYPE_ADR)
			frame->instance = frame->javalocals[0];
#else
		rplalloc instra;
		methoddesc *md;

		md = rp->method->parseddesc;
		assert(md->params);
		assert(md->paramcount >= 1);
		instra.type = TYPE_ADR;
		instra.regoff = md->params[0].regoff;
		if (md->params[0].inmemory) {
			instra.flags = INMEMORY;
			instra.regoff += (1 + code->stackframesize) * SIZE_OF_STACKSLOT;
		}
		else {
			instra.flags = 0;
		}
		replace_read_value(es, &instra, &(frame->instance));
#endif
	}
#if defined(__I386__)
	else if (!(rp->method->flags & ACC_STATIC)) {
		/* On i386 we always pass the first argument on stack. */
		frame->instance.a = *(java_object_t **)(basesp + 1);
	}
#endif
#endif /* defined(REPLACE_PATCH_DYNAMIC_CALL) */

	/* read stack slots */

	frame->javastackdepth = count;
	frame->javastack = (replace_val_t*) DumpMemory::allocate(sizeof(replace_val_t) * count);
	frame->javastacktype = (u1*) DumpMemory::allocate(sizeof(u1) * count);

#if !defined(NDEBUG)
	/* mark values as undefined */
	for (i=0; i<count; ++i) {
		frame->javastack[i].l = (u8) 0x00dead0000dead00ULL;
		frame->javastacktype[i] = TYPE_VOID;
	}
#endif /* !defined(NDEBUG) */

	i = 0;

	/* the first stack slot is special in SBR and EXH blocks */

	if (topslot == TOP_IS_ON_STACK) {
		assert(count);

		assert(ra->index == RPLALLOC_STACK);
		assert(ra->type == TYPE_ADR);
		frame->javastack[i].p = sp[-1];
		frame->javastacktype[i] = TYPE_ADR; /* XXX RET */
		count--;
		i++;
		ra++;
	}
	else if (topslot == TOP_IS_IN_ITMP1) {
		assert(count);

		assert(ra->index == RPLALLOC_STACK);
		assert(ra->type == TYPE_ADR);
		frame->javastack[i].p = es->intregs[REG_ITMP1];
		frame->javastacktype[i] = TYPE_ADR; /* XXX RET */
		count--;
		i++;
		ra++;
	}
	else if (topslot == TOP_IS_VOID) {
		assert(count);

		assert(ra->index == RPLALLOC_STACK);
		frame->javastack[i].l = 0;
		frame->javastacktype[i] = TYPE_VOID;
		count--;
		i++;
		ra++;
	}

	/* read remaining stack slots */

	for (; count--; ra++) {
		if (ra->index == RPLALLOC_SYNC) {
			assert(rp->type == rplpoint::TYPE_INLINE);

			/* only read synchronization slots when traversing an inline point */

			if (!topframe) {
				sourceframe_t *calleeframe = frame->down;
				assert(calleeframe);
				assert(calleeframe->syncslotcount == 0);
				assert(calleeframe->syncslots == NULL);

				calleeframe->syncslotcount = 1;
				calleeframe->syncslots = (replace_val_t*) DumpMemory::allocate(sizeof(replace_val_t));
				replace_read_value(es,ra,calleeframe->syncslots);
			}

			frame->javastackdepth--;
			continue;
		}

		assert(ra->index == RPLALLOC_STACK || ra->index == RPLALLOC_PARAM);

		/* do not read parameters of calls down the call chain */

		if (!topframe && ra->index == RPLALLOC_PARAM) {
			frame->javastackdepth--;
		}
		else {
			if (ra->type == TYPE_RET)
				frame->javastack[i].i = ra->regoff;
			else
				replace_read_value(es,ra,frame->javastack + i);
			frame->javastacktype[i] = ra->type;
			i++;
		}
	}
}


/* replace_write_executionstate ************************************************

   Pop a source frame from the front of the frame list of the given source state
   and write its values into the execution state.

   IN:
       rp...............replacement point for which execution state should be
	                    created
	   es...............the execution state to modify
	   ss...............the given source state
	   topframe.........true, if this is the last (top-most) source frame to be
	                    translated

   OUT:
       *es..............the execution state derived from the source state

*******************************************************************************/

static void replace_write_executionstate(rplpoint *rp,
										 executionstate_t *es,
										 sourcestate_t *ss,
										 bool topframe)
{
	methodinfo     *m;
	//codeinfo       *code;
	int             count;
	int             i;
	rplalloc       *ra;
	sourceframe_t  *frame;
	int             topslot;
	stackslot_t    *sp;
	//stackslot_t    *basesp;

	//code = code_find_codeinfo_for_pc(rp->pc);
	m = rp->method;
	topslot = TOP_IS_NORMAL;

	/* pop a source frame */

	frame = ss->frames;
	assert(frame);
	ss->frames = frame->down;

	/* calculate stack pointer */

	sp = (stackslot_t *) es->sp;

	//basesp = sp + code->stackframesize;

	/* in some cases the top stack slot is passed in REG_ITMP1 */

	if (rp->type == rplpoint::TYPE_EXH) {
		topslot = TOP_IS_IN_ITMP1;
	}

	/* write javalocals */

	ra = rp->regalloc;
	count = rp->regalloccount;

	while (count && (i = ra->index) >= 0) {
		assert(i < m->maxlocals);
		assert(i < frame->javalocalcount);
		assert(ra->type == frame->javalocaltype[i]);
		if (ra->type == TYPE_RET) {
			/* XXX assert that it matches this rplpoint */
		}
		else
			replace_write_value(es, ra, frame->javalocals + i);
		count--;
		ra++;
	}

	/* write stack slots */

	i = 0;

	/* the first stack slot is special in SBR and EXH blocks */

	if (topslot == TOP_IS_ON_STACK) {
		assert(count);

		assert(ra->index == RPLALLOC_STACK);
		assert(i < frame->javastackdepth);
		assert(frame->javastacktype[i] == TYPE_ADR);
		sp[-1] = frame->javastack[i].p;
		count--;
		i++;
		ra++;
	}
	else if (topslot == TOP_IS_IN_ITMP1) {
		assert(count);

		assert(ra->index == RPLALLOC_STACK);
		assert(i < frame->javastackdepth);
		assert(frame->javastacktype[i] == TYPE_ADR);
		es->intregs[REG_ITMP1] = frame->javastack[i].p;
		count--;
		i++;
		ra++;
	}
	else if (topslot == TOP_IS_VOID) {
		assert(count);

		assert(ra->index == RPLALLOC_STACK);
		assert(i < frame->javastackdepth);
		assert(frame->javastacktype[i] == TYPE_VOID);
		count--;
		i++;
		ra++;
	}

	/* write remaining stack slots */

	for (; count--; ra++) {
		if (ra->index == RPLALLOC_SYNC) {
			assert(rp->type == rplpoint::TYPE_INLINE);

			/* only write synchronization slots when traversing an inline point */

			if (!topframe) {
				assert(frame->down);
				assert(frame->down->syncslotcount == 1); /* XXX need to understand more cases */
				assert(frame->down->syncslots != NULL);

				replace_write_value(es,ra,frame->down->syncslots);
			}
			continue;
		}

		assert(ra->index == RPLALLOC_STACK || ra->index == RPLALLOC_PARAM);

		/* do not write parameters of calls down the call chain */

		if (!topframe && ra->index == RPLALLOC_PARAM) {
			/* skip it */
			/*
			ra->index = RPLALLOC_PARAM;
			replace_val_t v;
			v.l = 0;
			replace_write_value(es,ra,&v);
			*/
		}
		else {
			assert(i < frame->javastackdepth);
			assert(ra->type == frame->javastacktype[i]);
			if (ra->type == TYPE_RET) {
				/* XXX assert that it matches this rplpoint */
			}
			else {
				replace_write_value(es,ra,frame->javastack + i);
			}
			i++;
		}
	}

	/* set new pc */

	es->pc = rp->pc;
}


/* md_push_stackframe **********************************************************

   Save the given return address, build the new stackframe,
   and store callee-saved registers.

   *** This function imitates the effects of a call and the ***
   *** method prolog of the callee.                         ***

   IN:
       es...............execution state
       calleecode.......the code we are "calling"
       ra...............the return address to save

   OUT:
       *es..............the execution state after pushing the stack frame
                        NOTE: es->pc, es->code, and es->pv are NOT updated.

*******************************************************************************/

void md_push_stackframe(executionstate_t *es, codeinfo *calleecode, u1 *ra)
{
	s4           reg;
	s4           i;
	stackslot_t *basesp;
	stackslot_t *sp;

	assert(es);
	assert(calleecode);

	/* write the return address */

#if STACKFRMAE_RA_BETWEEN_FRAMES
	es->sp -= SIZEOF_VOID_P;
	*((void **)es->sp) = (void *) ra;
	if (calleecode->stackframesize)
		es->sp -= (SIZE_OF_STACKSLOT - SIZEOF_VOID_P);
#endif /* STACKFRAME_RA_BETWEEN_FRAMES */

	es->ra = (u1*) (ptrint) ra;

	/* build the stackframe */

	DOLOG( printf("building stackframe of %d words at %p\n",
				  calleecode->stackframesize, (void*)es->sp); );

	sp = (stackslot_t *) es->sp;
	basesp = sp;

	sp -= calleecode->stackframesize;
	es->sp = (u1*) sp;

	/* in debug mode, invalidate stack frame first */

	/* XXX may not invalidate linkage area used by native code! */

#if !defined(NDEBUG) && 0
	for (i=0; i< (basesp - sp) && i < 1; ++i) {
		sp[i] = 0xdeaddeadU;
	}
#endif

#if defined(__I386__)
	/* Stackslot 0 may contain the object instance for vftbl patching.
	   Destroy it, so there's no undefined value used. */
	if ((basesp - sp) > 0) {
		sp[0] = 0;
	}
#endif

	/* save the return address register */

#if STACKFRAME_RA_TOP_OF_FRAME
# if STACKFRAME_LEAFMETHODS_RA_REGISTER
	if (!code_is_leafmethod(calleecode))
# endif
		*--basesp = (ptrint) ra;
#endif /* STACKFRAME_RA_TOP_OF_FRAME */

#if STACKFRAME_RA_LINKAGE_AREA
# if STACKFRAME_LEAFMETHODS_RA_REGISTER
	if (!code_is_leafmethod(calleecode))
# endif
		*((uint8_t**) ((intptr_t) basesp + LA_LR_OFFSET)) = ra;
#endif /* STACKFRAME_RA_LINKAGE_AREA */

	/* save int registers */

	reg = INT_REG_CNT;
	for (i=0; i<calleecode->savedintcount; ++i) {
		while (nregdescint[--reg] != REG_SAV)
			;
		basesp -= 1;
		*((uintptr_t*) basesp) = es->intregs[reg];

		/* XXX may not clobber saved regs used by native code! */
#if !defined(NDEBUG) && 0
		es->intregs[reg] = (ptrint) 0x44dead4444dead44ULL;
#endif
	}

	/* save flt registers */

	/* XXX align? */
	reg = FLT_REG_CNT;
	for (i=0; i<calleecode->savedfltcount; ++i) {
		while (nregdescfloat[--reg] != REG_SAV)
			;
		basesp -= STACK_SLOTS_PER_FLOAT;
		*((double*) basesp) = es->fltregs[reg];

		/* XXX may not clobber saved regs used by native code! */
#if !defined(NDEBUG) && 0
		*(u8*)&(es->fltregs[reg]) = 0x44dead4444dead44ULL;
#endif
	}
}


/* replace_pop_activation_record ***********************************************

   Peel a stack frame from the execution state.

   *** This function imitates the effects of the method epilog ***
   *** and returning from the method call.                     ***

   IN:
       es...............execution state
       frame............source frame, receives synchronization slots

   OUT:
       *es..............the execution state after popping the stack frame

   RETURN VALUE:
       the return address of the poped activation record

*******************************************************************************/

u1* replace_pop_activation_record(executionstate_t *es,
								  sourceframe_t *frame)
{
	u1 *ra;
	s4 i;
	s4 count;
	codeinfo *code;
	stackslot_t *sp;

	assert(es->code);
	assert(frame);

	/* calculate the base of the stack frame */

	sp = (stackslot_t *) es->sp;
	assert(frame->syncslotcount == 0);
	assert(frame->syncslots == NULL);
	count = code_get_sync_slot_count(es->code);
	frame->syncslotcount = count;
	frame->syncslots = (replace_val_t*) DumpMemory::allocate(sizeof(replace_val_t) * count);
	for (i=0; i<count; ++i) {
		frame->syncslots[i].p = *((intptr_t*) (sp + es->code->memuse + i)); /* XXX md_ function */
	}

	/* pop the stackframe */

	executionstate_pop_stackframe(es);

	ra = es->pc;

	DOLOG( printf("RA = %p\n", (void*)ra); );

	/* Subtract one from the PC so we do not hit the replacement point */
	/* of the instruction following the call, if there is one.         */

	es->pc--;

	/* find the new codeinfo */

	void* pv = md_codegen_get_pv_from_pc(ra);
	DOLOG( printf("PV = %p\n", pv); );

	code = code_get_codeinfo_for_pv(pv);
	DOLOG( printf("CODE = %p\n", (void*) code); );

	/* return NULL if we reached native code */

	es->pv   = (uint8_t*) pv;
	es->code = code;

	return (code) ? ra : NULL;
}


/* replace_patch_method_pointer ************************************************

   Patch a method pointer (may be in code, data segment, vftbl, or interface
   table).

   IN:
	   mpp..............address of the method pointer to patch
	   entrypoint.......the new entrypoint of the method
	   kind.............kind of call to patch, used only for debugging

*******************************************************************************/

static void replace_patch_method_pointer(methodptr *mpp,
										 methodptr entrypoint,
										 const char *kind)
{
#if !defined(NDEBUG)
	codeinfo       *oldcode;
	codeinfo       *newcode;
#endif

	DOLOG( printf("patch method pointer from: %p to %p\n",
				  (void*) *mpp, (void*)entrypoint); );

#if !defined(NDEBUG)
	oldcode = code_get_codeinfo_for_pv(*mpp);
	newcode = code_get_codeinfo_for_pv(entrypoint);

	DOLOG_SHORT( printf("\tpatch %s %p ", kind, (void*) oldcode);
				 method_println(oldcode->m);
				 printf("\t      with      %p ", (void*) newcode);
				 method_println(newcode->m); );

	assert(oldcode->m == newcode->m);
#endif

	/* write the new entrypoint */

	*mpp = (methodptr) entrypoint;
}


/* replace_patch_class *********************************************************

   Patch a method in the given class.

   IN:
	   vftbl............vftbl of the class
	   m................the method to patch
	   oldentrypoint....the old entrypoint to replace
	   entrypoint.......the new entrypoint

*******************************************************************************/

void replace_patch_class(vftbl_t *vftbl,
						 methodinfo *m,
						 u1 *oldentrypoint,
						 u1 *entrypoint)
{
	s4                 i;
	methodptr         *mpp;
	methodptr         *mppend;

	/* patch the vftbl of the class */

	replace_patch_method_pointer(vftbl->table + m->vftblindex,
								 entrypoint,
								 "virtual  ");

	/* patch the interface tables */

	assert(oldentrypoint);

	for (i=0; i < vftbl->interfacetablelength; ++i) {
		mpp = vftbl->interfacetable[-i];
		mppend = mpp + vftbl->interfacevftbllength[i];
		for (; mpp != mppend; ++mpp)
			if (*mpp == oldentrypoint) {
				replace_patch_method_pointer(mpp, entrypoint, "interface");
			}
	}
}


/* replace_patch_class_hierarchy ***********************************************

   Patch a method in all loaded classes.

   IN:
	   m................the method to patch
	   oldentrypoint....the old entrypoint to replace
	   entrypoint.......the new entrypoint

*******************************************************************************/

struct replace_patch_data_t {
	methodinfo *m;
	u1         *oldentrypoint;
	u1         *entrypoint;
};

void replace_patch_callback(classinfo *c, struct replace_patch_data_t *pd)
{
	vftbl_t *vftbl = c->vftbl;

	if (vftbl != NULL
		&& vftbl->vftbllength > pd->m->vftblindex
		&& (void*) vftbl->table[pd->m->vftblindex] != (void*) (uintptr_t) &asm_abstractmethoderror
		&& code_get_methodinfo_for_pv(vftbl->table[pd->m->vftblindex]) == pd->m)
	{
		replace_patch_class(c->vftbl, pd->m, pd->oldentrypoint, pd->entrypoint);
	}
}

void replace_patch_class_hierarchy(methodinfo *m,
								   u1 *oldentrypoint,
								   u1 *entrypoint)
{
	struct replace_patch_data_t pd;

	pd.m = m;
	pd.oldentrypoint = oldentrypoint;
	pd.entrypoint = entrypoint;

	DOLOG_SHORT( printf("patching class hierarchy: ");
			     method_println(m); );

	classcache_foreach_loaded_class(
			(classcache_foreach_functionptr_t) &replace_patch_callback,
			(void*) &pd);
}


/* replace_patch_future_calls **************************************************

   Analyse a call site and depending on the kind of call patch the call, the
   virtual function table, or the interface table.

   IN:
	   ra...............return address pointing after the call site
	   callerframe......source frame of the caller
	   calleeframe......source frame of the callee, must have been mapped

*******************************************************************************/

void replace_patch_future_calls(u1 *ra,
								sourceframe_t *callerframe,
								sourceframe_t *calleeframe)
{
	u1            *patchpos;
	methodptr      entrypoint;
	methodptr      oldentrypoint;
#if !defined(__I386__)
	bool           atentry;
#endif
	void          *pv;
	codeinfo      *calleecode;
	methodinfo    *calleem;
	java_object_t *obj;
	vftbl_t       *vftbl;

	assert(ra);
	assert(callerframe->down == calleeframe);

	/* get the new codeinfo and the method that shall be entered */

	calleecode = calleeframe->tocode;
	assert(calleecode);

	calleem = calleeframe->method;
	assert(calleem == calleecode->m);

	entrypoint = (methodptr) calleecode->entrypoint;

	/* check if we are at an method entry rplpoint at the innermost frame */

#if !defined(__I386__)
	atentry = (calleeframe->down == NULL)
			&& !(calleem->flags & ACC_STATIC)
			&& (calleeframe->fromrp->id == 0); /* XXX */
#endif

	/* get the position to patch, in case it was a statically bound call   */

	pv = callerframe->fromcode->entrypoint;
	patchpos = (u1*) md_jit_method_patch_address(pv, ra, NULL);

	if (patchpos == NULL) {
		/* the call was dispatched dynamically */

		/* we can only patch such calls if we are at the entry point */

#if !defined(__I386__)
		/* On i386 we always know the instance argument. */
		if (!atentry)
			return;
#endif

		assert((calleem->flags & ACC_STATIC) == 0);

		oldentrypoint = calleeframe->fromcode->entrypoint;

		/* we need to know the instance */

		if (!calleeframe->instance.a) {
			DOLOG_SHORT( printf("WARNING: object instance unknown!\n"); );
			replace_patch_class_hierarchy(calleem, oldentrypoint, entrypoint);
			return;
		}

		/* get the vftbl */

		obj = calleeframe->instance.a;
		vftbl = obj->vftbl;

		assert(vftbl->clazz->vftbl == vftbl);

		DOLOG_SHORT( printf("\tclass: "); class_println(vftbl->clazz); );

		replace_patch_class(vftbl, calleem, oldentrypoint, entrypoint);
	}
	else {
		/* the call was statically bound */

#if defined(__I386__)
		/* It happens that there is a patcher trap. (pm) */
		if (*(u2 *)(patchpos - 1) == 0x0b0f) {
		} else
#endif
		replace_patch_method_pointer((methodptr *) patchpos, entrypoint, "static   ");
	}
}


/* replace_push_activation_record **********************************************

   Push a stack frame onto the execution state.

   *** This function imitates the effects of a call and the ***
   *** method prolog of the callee.                         ***

   IN:
	   es...............execution state
	   rpcall...........the replacement point at the call site
	   callerframe......source frame of the caller, or NULL for creating the
	                    first frame
	   calleeframe......source frame of the callee, must have been mapped

   OUT:
       *es..............the execution state after pushing the stack frame

*******************************************************************************/

void replace_push_activation_record(executionstate_t *es,
									rplpoint *rpcall,
									sourceframe_t *callerframe,
									sourceframe_t *calleeframe)
{
	s4           i;
	s4           count;
	stackslot_t *sp;
	u1          *ra;
	codeinfo    *calleecode;

	assert(es);
	assert(!rpcall || callerframe);
    assert(!rpcall || rpcall->type == rplpoint::TYPE_CALL);
	assert(!rpcall || rpcall == callerframe->torp);
	assert(calleeframe);
	assert(!callerframe || calleeframe == callerframe->down);

	/* the compilation unit we are entering */

	calleecode = calleeframe->tocode;
	assert(calleecode);

	/* calculate the return address */

	if (rpcall)
		ra = rpcall->pc + rpcall->callsize;
	else
		ra = es->pc + 1 /* XXX this is ugly */;

	/* push the stackframe */

	md_push_stackframe(es, calleecode, ra);

	/* we move into a new code unit, set code, PC, PV */

	es->code = calleecode;
	es->pc = calleecode->entrypoint; /* XXX not needed? */
	es->pv = calleecode->entrypoint;

	/* write slots used for synchronization */

	sp = (stackslot_t *) es->sp;
	count = code_get_sync_slot_count(calleecode);
	assert(count == calleeframe->syncslotcount);
	for (i=0; i<count; ++i) {
		*((intptr_t*) (sp + calleecode->memuse + i)) = calleeframe->syncslots[i].p;
	}

	/* redirect future invocations */

	if (callerframe && rpcall) {
#if defined(REPLACE_PATCH_ALL)
		if (rpcall->type == callerframe->fromrp->type)
#else
		if (rpcall == callerframe->fromrp)
#endif
			replace_patch_future_calls(ra, callerframe, calleeframe);
	}
}


/* replace_find_replacement_point **********************************************

   Find the replacement point in the given code corresponding to the
   position given in the source frame.

   IN:
	   code.............the codeinfo in which to search the rplpoint
	   frame............the source frame defining the position to look for
	   parent...........parent replacement point to match

   RETURN VALUE:
       the replacement point

*******************************************************************************/

rplpoint * replace_find_replacement_point(codeinfo *code,
										  sourceframe_t *frame,
										  rplpoint *parent)
{
	//methodinfo *m;
	rplpoint *rp;
	s4        i;
	s4        j;
	s4        stacki;
	rplalloc *ra;

	assert(code);
	assert(frame);

	DOLOG( printf("searching replacement point for:\n");
		   replace_source_frame_println(frame); );

	//m = frame->method;

	DOLOG( printf("code = %p\n", (void*)code); );

	rp = code->rplpoints;
	i = code->rplpointcount;
	while (i--) {
		if (rp->id == frame->id && rp->method == frame->method
				&& rp->parent == parent
				&& replace_normalize_type_map[rp->type] == frame->type)
		{
			/* check if returnAddresses match */
			/* XXX optimize: only do this if JSRs in method */
			DOLOG( printf("checking match for:");
				   replace_replacement_point_println(rp, 1); fflush(stdout); );
			ra = rp->regalloc;
			stacki = 0;
			for (j = rp->regalloccount; j--; ++ra) {
				if (ra->type == TYPE_RET) {
					if (ra->index == RPLALLOC_STACK) {
						assert(stacki < frame->javastackdepth);
						if (frame->javastack[stacki].i != ra->regoff)
							goto no_match;
						stacki++;
					}
					else {
						assert(ra->index >= 0 && ra->index < frame->javalocalcount);
						if (frame->javalocals[ra->index].i != ra->regoff)
							goto no_match;
					}
				}
			}

			/* found */
			return rp;
		}
no_match:
		rp++;
	}

#if !defined(NDEBUG)
	printf("candidate replacement points were:\n");
	rp = code->rplpoints;
	i = code->rplpointcount;
	for (; i--; ++rp) {
		replace_replacement_point_println(rp, 1);
	}
#endif

	vm_abort("no matching replacement point found");
	return NULL; /* NOT REACHED */
}


/* replace_find_replacement_point_for_pc ***************************************

   Find the nearest replacement point at or before the given PC. The
   given PC has to be between (rp->pc) and (rp->pc+rp->callsize) for
   the replacement point to be found.

   IN:
       code.............compilation unit the PC is in
	   pc...............the machine code PC

   RETURN VALUE:
       the replacement point found, or
	   NULL if no replacement point was found

*******************************************************************************/

rplpoint *replace_find_replacement_point_for_pc(codeinfo *code, u1 *pc, unsigned desired_flags)
{
	rplpoint *found;
	rplpoint *rp;
	s4        i;

	DOLOG( printf("searching for rp at pc:%p in %p ", (void*)pc, (void*)code);
		   method_println(code->m); );

	found = NULL;

	rp = code->rplpoints;
	for (i=0; i<code->rplpointcount; ++i, ++rp) {
		DOLOG( replace_replacement_point_println(rp, 2); );
		if (rp->pc <= pc && rp->pc + rp->callsize >= pc) {
			if (desired_flags) {
				if (rp->flags & desired_flags) {
					found = rp;
				}
			} else {
				found = rp;
			}
		}
	}

	return found;
}

/* replace_pop_native_frame ****************************************************

   Unroll a native frame in the execution state and create a source frame
   for it.

   IN:
	   es...............current execution state
	   ss...............the current source state
	   sfi..............stackframeinfo for the native frame

   OUT:
       es...............execution state after unrolling the native frame
	   ss...............gets the added native source frame

*******************************************************************************/

static void replace_pop_native_frame(executionstate_t *es,
									 sourcestate_t *ss,
									 stackframeinfo_t *sfi)
{
	sourceframe_t *frame;
	codeinfo      *code;
	s4             i,j;

	assert(sfi);

	frame = replace_new_sourceframe(ss);

	frame->sfi = sfi;

	/* remember pc and size of native frame */

	frame->nativepc = es->pc;
	frame->nativeframesize = (es->sp != 0) ? (((uintptr_t) sfi->sp) - ((uintptr_t) es->sp)) : 0;
	assert(frame->nativeframesize >= 0);

	/* remember values of saved registers */

	j = 0;
	for (i=0; i<INT_REG_CNT; ++i) {
		if (nregdescint[i] == REG_SAV)
			frame->nativesavint[j++] = es->intregs[i];
	}

	j = 0;
	for (i=0; i<FLT_REG_CNT; ++i) {
		if (nregdescfloat[i] == REG_SAV)
			frame->nativesavflt[j++] = es->fltregs[i];
	}

	/* restore saved registers */

#if defined(ENABLE_GC_CACAO)
	j = 0;
	for (i=0; i<INT_REG_CNT; ++i) {
		if (nregdescint[i] == REG_SAV)
			es->intregs[i] = sfi->intregs[j++];
	}
#else
	/* XXX we don't have them, yet, in the sfi, so clear them */

	for (i=0; i<INT_REG_CNT; ++i) {
		if (nregdescint[i] == REG_SAV)
			es->intregs[i] = 0;
	}
#endif

	/* XXX we don't have float registers in the sfi, so clear them */

	for (i=0; i<FLT_REG_CNT; ++i) {
		if (nregdescfloat[i] == REG_SAV)
			es->fltregs[i] = 0.0;
	}

	/* restore codeinfo of the native stub */

	code = code_get_codeinfo_for_pv(sfi->pv);

	/* restore sp, pv, pc and codeinfo of the parent method */

	es->sp   = (uint8_t*) (((uintptr_t) sfi->sp) + md_stacktrace_get_framesize(code));
#if STACKFRMAE_RA_BETWEEN_FRAMES
	es->sp  += SIZEOF_VOID_P; /* skip return address */
#endif
	es->pv   = (uint8_t*) md_codegen_get_pv_from_pc(sfi->ra);
	es->pc   = (uint8_t*) (((uintptr_t) ((sfi->xpc) ? sfi->xpc : sfi->ra)) - 1);
	es->code = code_get_codeinfo_for_pv(es->pv);
}


/* replace_push_native_frame ***************************************************

   Rebuild a native frame onto the execution state and remove its source frame.

   Note: The native frame is "rebuild" by setting fields like PC and stack
         pointer in the execution state accordingly. Values in the
		 stackframeinfo may be modified, but the actual stack frame of the
		 native code is not touched.

   IN:
	   es...............current execution state
	   ss...............the current source state

   OUT:
       es...............execution state after re-rolling the native frame
	   ss...............the native source frame is removed

*******************************************************************************/

static void replace_push_native_frame(executionstate_t *es, sourcestate_t *ss)
{
	sourceframe_t *frame;
	s4             i,j;

	assert(es);
	assert(ss);

	DOLOG( printf("pushing native frame\n"); );

	/* remove the frame from the source state */

	frame = ss->frames;
	assert(frame);
	assert(REPLACE_IS_NATIVE_FRAME(frame));

	ss->frames = frame->down;

	/* skip sp for the native stub */

	es->sp -= md_stacktrace_get_framesize(frame->sfi->code);
#if STACKFRMAE_RA_BETWEEN_FRAMES
	es->sp -= SIZEOF_VOID_P; /* skip return address */
#endif

	/* assert that the native frame has not moved */

	assert(es->sp == frame->sfi->sp);

	/* update saved registers in the stackframeinfo */

#if defined(ENABLE_GC_CACAO)
	j = 0;
	for (i=0; i<INT_REG_CNT; ++i) {
		if (nregdescint[i] == REG_SAV)
			frame->sfi->intregs[j++] = es->intregs[i];
	}

	/* XXX leave float registers untouched here */
#endif

	/* restore saved registers */

	j = 0;
	for (i=0; i<INT_REG_CNT; ++i) {
		if (nregdescint[i] == REG_SAV)
			es->intregs[i] = frame->nativesavint[j++];
	}

	j = 0;
	for (i=0; i<FLT_REG_CNT; ++i) {
		if (nregdescfloat[i] == REG_SAV)
			es->fltregs[i] = frame->nativesavflt[j++];
	}

	/* skip the native frame on the machine stack */

	es->sp -= frame->nativeframesize;

	/* set the pc the next frame must return to */

	es->pc = frame->nativepc;
}


/* replace_recover_source_state ************************************************

   Recover the source state from the given replacement point and execution
   state.

   IN:
       rp...............replacement point that has been reached, if any
	   sfi..............stackframeinfo, if called from native code
	   es...............execution state at the replacement point rp

   RETURN VALUE:
       the source state

*******************************************************************************/

sourcestate_t *replace_recover_source_state(rplpoint *rp,
											stackframeinfo_t *sfi,
										    executionstate_t *es)
{
	sourcestate_t *ss;
	u1            *ra;
	bool           locked;
#if defined(REPLACE_STATISTICS)
	s4             depth;
#endif

	/* create the source frame structure in dump memory */

	ss = (sourcestate_t*) DumpMemory::allocate(sizeof(sourcestate_t));
	ss->frames = NULL;

	/* each iteration of the loop recovers one source frame */

	depth = 0;
	locked = false;

	while (rp || sfi) {

		DOLOG( executionstate_println(es); );

		/* if we are not at a replacement point, it is a native frame */

		if (rp == NULL) {
			DOLOG( printf("native frame: sfi: "); replace_stackframeinfo_println(sfi); );

			locked = true;
			replace_pop_native_frame(es, ss, sfi);
			sfi = sfi->prev;

			if (es->code == NULL)
				continue;

			goto after_machine_frame;
		}

		/* read the values for this source frame from the execution state */

		DOLOG( printf("recovering source state for%s:\n",
					(ss->frames == NULL) ? " TOPFRAME" : "");
			   replace_replacement_point_println(rp, 1); );

		replace_read_executionstate(rp, es, ss, ss->frames == NULL);

#if defined(REPLACE_STATISTICS)
		REPLACE_COUNT(stat_frames);
		depth++;
		replace_statistics_source_frame(ss->frames);
#endif

		/* in locked areas (below native frames), identity map the frame */

		if (locked) {
			ss->frames->torp = ss->frames->fromrp;
			ss->frames->tocode = ss->frames->fromcode;
		}

		/* unroll to the next (outer) frame */

		if (rp->parent) {
			/* this frame is in inlined code */

			DOLOG( printf("INLINED!\n"); );

			rp = rp->parent;

			assert(rp->type == rplpoint::TYPE_INLINE);
			REPLACE_COUNT(stat_unroll_inline);
		}
		else {
			/* this frame had been called at machine-level. pop it. */

			DOLOG( printf("UNWIND\n"); );

			ra = replace_pop_activation_record(es, ss->frames);
			if (ra == NULL) {
				DOLOG( printf("REACHED NATIVE CODE\n"); );

				rp = NULL;

#if !defined(ENABLE_GC_CACAO)
				break; /* XXX remove to activate native frames */
#endif
				continue;
			}

			/* find the replacement point at the call site */

after_machine_frame:
			rp = replace_find_replacement_point_for_pc(es->code, es->pc, 0);

			if (rp == NULL)
				vm_abort("could not find replacement point while unrolling call");

			DOLOG( printf("found replacement point.\n");
					replace_replacement_point_println(rp, 1); );

			assert(rp->type == rplpoint::TYPE_CALL);
			REPLACE_COUNT(stat_unroll_call);
		}
	} /* end loop over source frames */

	REPLACE_COUNT_DIST(stat_dist_frames, depth);

	return ss;
}


/* replace_map_source_state ****************************************************

   Map each source frame in the given source state to a target replacement
   point and compilation unit. If no valid code is available for a source
   frame, it is (re)compiled.

   IN:
       ss...............the source state

   OUT:
       ss...............the source state, modified: The `torp` and `tocode`
	                    fields of each source frame are set.

   RETURN VALUE:
       true.............everything went ok
	   false............an exception has been thrown

*******************************************************************************/

static bool replace_map_source_state(sourcestate_t *ss)
{
	sourceframe_t *frame;
	codeinfo      *code;
	rplpoint      *rp;
	rplpoint      *parent; /* parent of inlined rplpoint */
#if defined(REPLACE_STATISTICS)
	codeinfo      *oldcode;
#endif

	parent = NULL;
	code = NULL;

	/* iterate over the source frames from outermost to innermost */

	for (frame = ss->frames; frame != NULL; frame = frame->down) {

		/* XXX skip native frames */

		if (REPLACE_IS_NATIVE_FRAME(frame)) {
			parent = NULL;
			continue;
		}

		/* map frames which are not already mapped */

		if (frame->tocode) {
			code = frame->tocode;
			rp = frame->torp;
			assert(rp);
		}
		else {
			assert(frame->torp == NULL);

			if (parent == NULL) {
				/* find code for this frame */

#if defined(REPLACE_STATISTICS)
				oldcode = frame->method->code;
#endif
				/* request optimization of hot methods and their callers */

				if (frame->method->hitcountdown < 0
					|| (frame->down && frame->down->method->hitcountdown < 0))
					jit_request_optimization(frame->method);

				code = jit_get_current_code(frame->method);

				if (code == NULL)
					return false; /* exception */

				REPLACE_COUNT_IF(stat_recompile, code != oldcode);
			}

			assert(code);

			/* map this frame */

			rp = replace_find_replacement_point(code, frame, parent);

			frame->tocode = code;
			frame->torp = rp;
		}

		if (rp->type == rplpoint::TYPE_CALL) {
			parent = NULL;
		}
		else {
			/* inlining */
			parent = rp;
		}
	}

	return true;
}


/* replace_map_source_state_identity *******************************************

   Map each source frame in the given source state to the same replacement
   point and compilation unit it was derived from. This is mainly used for
   garbage collection.

   IN:
       ss...............the source state

   OUT:
       ss...............the source state, modified: The `torp` and `tocode`
	                    fields of each source frame are set.

*******************************************************************************/

#if defined(ENABLE_GC_CACAO)
static void replace_map_source_state_identity(sourcestate_t *ss)
{
	sourceframe_t *frame;

	/* iterate over the source frames from outermost to innermost */

	for (frame = ss->frames; frame != NULL; frame = frame->down) {

		/* skip native frames */

		if (REPLACE_IS_NATIVE_FRAME(frame)) {
			continue;
		}

		/* map frames using the identity mapping */

		if (frame->tocode) {
			assert(frame->tocode == frame->fromcode);
			assert(frame->torp   == frame->fromrp);
		} else {
			assert(frame->tocode == NULL);
			assert(frame->torp   == NULL);
			frame->tocode = frame->fromcode;
			frame->torp   = frame->fromrp;
		}
	}
}
#endif


/* replace_build_execution_state ***********************************************

   Build an execution state for the given (mapped) source state.

   !!! CAUTION: This function rewrites the machine stack !!!

   THIS FUNCTION MUST BE CALLED USING A SAFE STACK AREA!

   IN:
       ss...............the source state. Must have been mapped by
						replace_map_source_state before.
	   es...............the base execution state on which to build

   OUT:
       *es..............the new execution state

*******************************************************************************/

static void replace_build_execution_state(sourcestate_t *ss,
										  executionstate_t *es)
{
	rplpoint      *rp;
	sourceframe_t *prevframe;
	rplpoint      *parent;

	parent = NULL;
	prevframe = NULL;
	rp = NULL;

	while (ss->frames) {

		if (REPLACE_IS_NATIVE_FRAME(ss->frames)) {
			prevframe = ss->frames;
			replace_push_native_frame(es, ss);
			parent = NULL;
			rp = NULL;
			continue;
		}

		if (parent == NULL) {
			/* create a machine-level stack frame */

			DOLOG( printf("pushing activation record for:\n");
				   if (rp) replace_replacement_point_println(rp, 1);
				   else printf("\tfirst frame\n"); );

			replace_push_activation_record(es, rp, prevframe, ss->frames);

			DOLOG( executionstate_println(es); );
		}

		rp = ss->frames->torp;
		assert(rp);

		DOLOG( printf("creating execution state for%s:\n",
				(ss->frames->down == NULL) ? " TOPFRAME" : "");
			   replace_replacement_point_println(ss->frames->fromrp, 1);
			   replace_replacement_point_println(rp, 1); );

		es->code = ss->frames->tocode;
		prevframe = ss->frames;

		replace_write_executionstate(rp, es, ss, ss->frames->down == NULL);

		DOLOG( executionstate_println(es); );

		if (rp->type == rplpoint::TYPE_CALL) {
			parent = NULL;
		}
		else {
			/* inlining */
			parent = rp;
		}
	}
}


/* replace_me ******************************************************************

   This function is called by the signal handler when a thread reaches
   a replacement point. `replace_me` must map the execution state to the
   target replacement point and let execution continue there.

   THIS FUNCTION MUST BE CALLED USING A SAFE STACK AREA!

   IN:
       rp...............replacement point that has been reached
       es...............execution state read by signal handler

*******************************************************************************/

static void replace_me(rplpoint *rp, executionstate_t *es)
{
	stackframeinfo_t    *sfi;
	sourcestate_t       *ss;
	sourceframe_t       *frame;
	codeinfo            *origcode;
	rplpoint            *origrp;
#if defined(ENABLE_THREADS) && defined(ENABLE_GC_CACAO)
	threadobject        *thread;
#endif

	origcode = es->code;
	origrp   = rp;

#if defined(ENABLE_TLH)
	/*printf("Replacing in %s/%s\n", UTF_TEXT(rp->method->clazz->name), UTF_TEXT(rp->method->name));*/
#endif

	/*if (strcmp(UTF_TEXT(rp->method->clazz->name), "antlr/AlternativeElement") == 0 && strcmp(UTF_TEXT(rp->method->name), "getAutoGenType") ==0) opt_TraceReplacement = 2; else opt_TraceReplacement = 0;*/

	DOLOG_SHORT( printf("REPLACING(%d %p): (id %d %p) ",
				 stat_replacements, (void*)THREADOBJECT,
				 rp->id, (void*)rp);
				 method_println(es->code->m); );

	DOLOG( replace_replacement_point_println(rp, 1); );

	REPLACE_COUNT(stat_replacements);

	// Create new dump memory area.
	DumpMemoryArea dma;

	/* Get the stackframeinfo for the current thread. */

	sfi = threads_get_current_stackframeinfo();

	/* recover source state */

	ss = replace_recover_source_state(rp, sfi, es);

#if defined(ENABLE_THREADS) && defined(ENABLE_GC_CACAO)
	/* if there is a collection pending, we assume the replacement point should
	   suspend this thread */

	if (gc_pending) {

		thread = THREADOBJECT;

		DOLOG_SHORT( printf("REPLACEMENT: Suspending thread for GC now!\n"); );

		/* map the sourcestate using the identity mapping */
		replace_map_source_state_identity(ss);

		/* since we enter the same method again, we turn off rps now */
		/* XXX michi: can we really do this? what if the rp was active before
		   we activated it for the gc? */
		replace_deactivate_replacement_points(origcode);

		/* remember executionstate and sourcestate for this thread */
		GC_EXECUTIONSTATE = es;
		GC_SOURCESTATE    = ss;

		/* really suspend this thread now (PC = 0) */
		threads_suspend_ack(NULL, NULL);

		DOLOG_SHORT( printf("REPLACEMENT: Resuming thread after GC now!\n"); );

	} else {
#endif /*defined(ENABLE_THREADS) && defined(ENABLE_GC_CACAO)*/

	/* map the source state */

	if (!replace_map_source_state(ss))
		vm_abort("exception during method replacement");

	DOLOG( replace_sourcestate_println(ss); );

	DOLOG_SHORT( replace_sourcestate_println_short(ss); );

#if !defined(NDEBUG)
	/* avoid infinite loops by self-replacement, only if not in testing mode */

	if (!opt_TestReplacement) {
		frame = ss->frames;
		while (frame->down)
			frame = frame->down;

		if (frame->torp == origrp) {
			DOLOG_SHORT(
				printf("WARNING: identity replacement, turning off rps to avoid infinite loop\n");
			);
			replace_deactivate_replacement_points(origcode);
		}
	}
#endif

#if defined(ENABLE_THREADS) && defined(ENABLE_GC_CACAO)
	}
#endif

	/* build the new execution state */

	replace_build_execution_state(ss, es);

#if !defined(NDEBUG)
	/* continue execution after patched machine code, if testing mode enabled */

	if (opt_TestReplacement)
		es->pc += REPLACEMENT_PATCH_SIZE;
#endif
}


/* replace_handler *************************************************************

   This function is called by the signal handler. It determines if there
   is an active replacement point pending at the given PC and returns
   accordingly.

   THIS FUNCTION MUST BE CALLED USING A SAFE STACK AREA!

   IN:
       pc...............the program counter that triggered the replacement.
       es...............the execution state (machine state) to which the
	                    replacement should be applied.

   OUT:
       es...............the execution state after replacement finished.

   RETURN VALUE:
       true.............replacement done, everything went ok
       false............no replacement done, execution state unchanged

*******************************************************************************/

bool replace_handler(u1 *pc, executionstate_t *es)
{
	codeinfo         *code;
	rplpoint         *rp;

	/* search the codeinfo for the given PC */

	code = code_find_codeinfo_for_pc(pc);
	assert(code);

	/* search for a replacement point at the given PC */

	rp = replace_find_replacement_point_for_pc(code, pc, (rplpoint::FLAG_ACTIVE | rplpoint::FLAG_COUNTDOWN));

	/* check if the replacement point belongs to given PC and is active */

	if ((rp != NULL) && (rp->pc == pc)
	    && (rp->flags & (rplpoint::FLAG_ACTIVE | rplpoint::FLAG_COUNTDOWN))) {

		DOLOG( printf("valid replacement point\n"); );

		/* set codeinfo pointer in execution state */

		es->code = code;

		/* do the actual replacement */

		replace_me(rp, es);

		/* new code is entered after returning */

		DOLOG( printf("JUMPING IN!\n"); fflush(stdout); );
		return true;
	}
	else
		return false;
}


/******************************************************************************/
/* NOTE: Stuff specific to the exact GC is below.                             */
/******************************************************************************/

#if defined(ENABLE_GC_CACAO)
void replace_gc_from_native(threadobject *thread, u1 *pc, u1 *sp)
{
	stackframeinfo_t *sfi;
	executionstate_t *es;
	sourcestate_t    *ss;

	/* Get the stackframeinfo of this thread. */

	assert(thread == THREADOBJECT);

	sfi = threads_get_current_stackframeinfo();

	/* create the execution state */
	es = (executionstate_t*) DumpMemory::allocate(sizeof(executionstate_t));
	es->pc = pc;
	es->sp = sp;
	es->pv = 0;      /* since we are in a native, PV is invalid! */
	es->code = NULL; /* since we are in a native, we do not have a codeinfo */

	/* we assume we are in a native (no replacement point)! */
	ss = replace_recover_source_state(NULL, sfi, es);

	/* map the sourcestate using the identity mapping */
	replace_map_source_state_identity(ss);

	/* remember executionstate and sourcestate for this thread */
	GC_EXECUTIONSTATE = es;
	GC_SOURCESTATE    = ss;
}
#endif

#if defined(ENABLE_GC_CACAO)
void replace_gc_into_native(threadobject *thread)
{
	executionstate_t *es;
	sourcestate_t    *ss;

	/* get the executionstate and sourcestate for the given thread */
	es = GC_EXECUTIONSTATE;
	ss = GC_SOURCESTATE;

	/* rebuild the stack of the given thread */
	replace_build_execution_state(ss, es);
}
#endif


/******************************************************************************/
/* NOTE: No important code below.                                             */
/******************************************************************************/


/* statistics *****************************************************************/

#if defined(REPLACE_STATISTICS)
static void print_freq(FILE *file,int *array,int limit)
{
	int i;
	int sum = 0;
	int cum = 0;
	for (i=0; i<limit; ++i)
		sum += array[i];
	sum += array[limit];
	for (i=0; i<limit; ++i) {
		cum += array[i];
		fprintf(file,"      %3d: %8d (cum %3d%%)\n",
				i, array[i], (sum) ? ((100*cum)/sum) : 0);
	}
	fprintf(file,"    >=%3d: %8d\n",limit,array[limit]);
}
#endif /* defined(REPLACE_STATISTICS) */


#if defined(REPLACE_STATISTICS)

#define REPLACE_PRINT_DIST(name, array)                              \
    printf("    " name " distribution:\n");                          \
    print_freq(stdout, (array), sizeof(array)/sizeof(int) - 1);

void replace_print_statistics(void)
{
	printf("replacement statistics:\n");
	printf("    # of replacements:   %d\n", stat_replacements);
	printf("    # of frames:         %d\n", stat_frames);
	printf("    # of recompilations: %d\n", stat_recompile);
	printf("    patched static calls:%d\n", stat_staticpatch);
	printf("    unrolled inlines:    %d\n", stat_unroll_inline);
	printf("    unrolled calls:      %d\n", stat_unroll_call);
	REPLACE_PRINT_DIST("frame depth", stat_dist_frames);
	REPLACE_PRINT_DIST("locals per frame", stat_dist_locals);
	REPLACE_PRINT_DIST("ADR locals per frame", stat_dist_locals_adr);
	REPLACE_PRINT_DIST("primitive locals per frame", stat_dist_locals_prim);
	REPLACE_PRINT_DIST("RET locals per frame", stat_dist_locals_ret);
	REPLACE_PRINT_DIST("void locals per frame", stat_dist_locals_void);
	REPLACE_PRINT_DIST("stack slots per frame", stat_dist_stack);
	REPLACE_PRINT_DIST("ADR stack slots per frame", stat_dist_stack_adr);
	REPLACE_PRINT_DIST("primitive stack slots per frame", stat_dist_stack_prim);
	REPLACE_PRINT_DIST("RET stack slots per frame", stat_dist_stack_ret);
	printf("\n");
	printf("    # of methods:            %d\n", stat_methods);
	printf("    # of replacement points: %d\n", stat_rploints);
	printf("    # of regallocs:          %d\n", stat_regallocs);
	printf("        per rplpoint:        %f\n", (double)stat_regallocs / stat_rploints);
	printf("        per method:          %f\n", (double)stat_regallocs / stat_methods);
	REPLACE_PRINT_DIST("replacement points per method", stat_dist_method_rplpoints);
	printf("\n");

}
#endif /* defined(REPLACE_STATISTICS) */


#if defined(REPLACE_STATISTICS)
static void replace_statistics_source_frame(sourceframe_t *frame)
{
	int adr = 0;
	int ret = 0;
	int prim = 0;
	int vd = 0;
	int n = 0;
	int i;

	for (i=0; i<frame->javalocalcount; ++i) {
		switch (frame->javalocaltype[i]) {
			case TYPE_ADR: adr++; break;
			case TYPE_RET: ret++; break;
			case TYPE_INT: case TYPE_LNG: case TYPE_FLT: case TYPE_DBL: prim++; break;
			case TYPE_VOID: vd++; break;
			default: assert(0);
		}
		n++;
	}
	REPLACE_COUNT_DIST(stat_dist_locals, n);
	REPLACE_COUNT_DIST(stat_dist_locals_adr, adr);
	REPLACE_COUNT_DIST(stat_dist_locals_void, vd);
	REPLACE_COUNT_DIST(stat_dist_locals_ret, ret);
	REPLACE_COUNT_DIST(stat_dist_locals_prim, prim);
	adr = ret = prim = n = 0;
	for (i=0; i<frame->javastackdepth; ++i) {
		switch (frame->javastacktype[i]) {
			case TYPE_ADR: adr++; break;
			case TYPE_RET: ret++; break;
			case TYPE_INT: case TYPE_LNG: case TYPE_FLT: case TYPE_DBL: prim++; break;
		}
		n++;
	}
	REPLACE_COUNT_DIST(stat_dist_stack, n);
	REPLACE_COUNT_DIST(stat_dist_stack_adr, adr);
	REPLACE_COUNT_DIST(stat_dist_stack_ret, ret);
	REPLACE_COUNT_DIST(stat_dist_stack_prim, prim);
}
#endif /* defined(REPLACE_STATISTICS) */


/* debugging helpers **********************************************************/

/* replace_replacement_point_println *******************************************

   Print replacement point info.

   IN:
       rp...............the replacement point to print

*******************************************************************************/

#if !defined(NDEBUG)

#define TYPECHAR(t)  (((t) >= 0 && (t) <= TYPE_RET) ? show_jit_type_letters[t] : '?')

static const char *replace_type_str[] = {
	"STD",
	"EXH",
	"SBR",
	"CALL",
	"INLINE",
	"RETURN",
	"BODY"
};

void replace_replacement_point_println(rplpoint *rp, int depth)
{
	int j;
	int index;

	if (!rp) {
		printf("(rplpoint *)NULL\n");
		return;
	}

	for (j=0; j<depth; ++j)
		putchar('\t');

	printf("rplpoint (id %d) %p pc:%p+%d type:%s",
			rp->id, (void*)rp,rp->pc,rp->callsize,
			replace_type_str[rp->type]);
	if (rp->flags & rplpoint::FLAG_NOTRAP)
		printf(" NOTRAP");
	if (rp->flags & rplpoint::FLAG_COUNTDOWN)
		printf(" COUNTDOWN");
	if (rp->flags & rplpoint::FLAG_ACTIVE)
		printf(" ACTIVE");
	printf(" parent:%p\n", (void*)rp->parent);
	for (j=0; j<depth; ++j)
		putchar('\t');
	printf("ra:%d = [",	rp->regalloccount);

	for (j=0; j<rp->regalloccount; ++j) {
		if (j)
			putchar(' ');
		index = rp->regalloc[j].index;
		switch (index) {
			case RPLALLOC_STACK: printf("S"); break;
			case RPLALLOC_PARAM: printf("P"); break;
			case RPLALLOC_SYNC : printf("Y"); break;
			default: printf("%d", index);
		}
		printf(":%1c:", TYPECHAR(rp->regalloc[j].type));
		if (rp->regalloc[j].type == TYPE_RET) {
			printf("ret(L%03d)", rp->regalloc[j].regoff);
		}
		else {
			show_allocation(rp->regalloc[j].type, rp->regalloc[j].flags, rp->regalloc[j].regoff);
		}
	}

	printf("]\n");
	for (j=0; j<depth; ++j)
		putchar('\t');
	printf("method: ");
	method_print(rp->method);

	printf("\n");
}
#endif /* !defined(NDEBUG) */


/* replace_show_replacement_points *********************************************

   Print replacement point info.

   IN:
       code.............codeinfo whose replacement points should be printed.

*******************************************************************************/

#if !defined(NDEBUG)
void replace_show_replacement_points(codeinfo *code)
{
	int i;
	int depth;
	rplpoint *rp;
	rplpoint *parent;

	if (!code) {
		printf("(codeinfo *)NULL\n");
		return;
	}

	printf("\treplacement points: %d\n",code->rplpointcount);

	printf("\ttotal allocations : %d\n",code->regalloccount);
	printf("\tsaved int regs    : %d\n",code->savedintcount);
	printf("\tsaved flt regs    : %d\n",code->savedfltcount);
	printf("\tmemuse            : %d\n",code->memuse);

	printf("\n");

	for (i=0; i<code->rplpointcount; ++i) {
		rp = code->rplpoints + i;

		depth = 1;
		parent = rp->parent;
		while (parent) {
			depth++;
			parent = parent->parent;
		}
		replace_replacement_point_println(rp, depth);
	}
}
#endif


#if !defined(NDEBUG)
static void java_value_print(s4 type, replace_val_t value)
{
	java_object_t *obj;
	Utf8String     u;

	printf("%016llx",(unsigned long long) value.l);

	if (type < 0 || type > TYPE_RET)
		printf(" <INVALID TYPE:%d>", type);
	else
		printf(" %s", show_jit_type_names[type]);

	if (type == TYPE_ADR && value.a != NULL) {
		obj = value.a;
		putchar(' ');
		utf_display_printable_ascii_classname(obj->vftbl->clazz->name);

		if (obj->vftbl->clazz == class_java_lang_String) {
			printf(" \"");
			u = JavaString(obj).to_utf8();
			utf_display_printable_ascii(u);
			printf("\"");
		}
	}
	else if (type == TYPE_INT) {
		printf(" %ld", (long) value.i);
	}
	else if (type == TYPE_LNG) {
		printf(" %lld", (long long) value.l);
	}
	else if (type == TYPE_FLT) {
		printf(" %f", value.f);
	}
	else if (type == TYPE_DBL) {
		printf(" %f", value.d);
	}
}
#endif /* !defined(NDEBUG) */


#if !defined(NDEBUG)
void replace_source_frame_println(sourceframe_t *frame)
{
	s4 i,j;
	s4 t;

	if (REPLACE_IS_NATIVE_FRAME(frame)) {
		printf("\tNATIVE\n");
		printf("\tsfi: "); replace_stackframeinfo_println(frame->sfi);
		printf("\tnativepc: %p\n", frame->nativepc);
		printf("\tframesize: %d\n", frame->nativeframesize);

		j = 0;
		for (i=0; i<INT_REG_CNT; ++i) {
			if (nregdescint[i] == REG_SAV)
				printf("\t%s = %p\n", abi_registers_integer_name[i], (void*)frame->nativesavint[j++]);
		}

		j = 0;
		for (i=0; i<FLT_REG_CNT; ++i) {
			if (nregdescfloat[i] == REG_SAV)
				printf("\tF%02d = %f\n", i, frame->nativesavflt[j++]);
		}

		printf("\n");
		return;
	}

	printf("\t");
	method_println(frame->method);
	printf("\tid: %d\n", frame->id);
	printf("\ttype: %s\n", replace_type_str[frame->type]);
	printf("\n");

	if (frame->instance.a) {
		printf("\tinstance: ");
		java_value_print(TYPE_ADR, frame->instance);
		printf("\n");
	}

	if (frame->javalocalcount) {
		printf("\tlocals (%d):\n",frame->javalocalcount);
		for (i=0; i<frame->javalocalcount; ++i) {
			t = frame->javalocaltype[i];
			if (t == TYPE_VOID) {
				printf("\tlocal[ %2d] = void\n",i);
			}
			else {
				printf("\tlocal[%c%2d] = ",TYPECHAR(t),i);
				java_value_print(t, frame->javalocals[i]);
				printf("\n");
			}
		}
		printf("\n");
	}

	if (frame->javastackdepth) {
		printf("\tstack (depth %d):\n",frame->javastackdepth);
		for (i=0; i<frame->javastackdepth; ++i) {
			t = frame->javastacktype[i];
			if (t == TYPE_VOID) {
				printf("\tstack[%2d] = void", i);
			}
			else {
				printf("\tstack[%2d] = ",i);
				java_value_print(frame->javastacktype[i], frame->javastack[i]);
				printf("\n");
			}
		}
		printf("\n");
	}

	if (frame->syncslotcount) {
		printf("\tsynchronization slots (%d):\n",frame->syncslotcount);
		for (i=0; i<frame->syncslotcount; ++i) {
			printf("\tslot[%2d] = %016llx\n",i,(unsigned long long) frame->syncslots[i].p);
		}
		printf("\n");
	}

	if (frame->fromcode) {
		printf("\tfrom %p ", (void*)frame->fromcode);
		method_println(frame->fromcode->m);
	}
	if (frame->tocode) {
		printf("\tto %p ", (void*)frame->tocode);
		method_println(frame->tocode->m);
	}

	if (frame->fromrp) {
		printf("\tfrom replacement point:\n");
		replace_replacement_point_println(frame->fromrp, 2);
	}
	if (frame->torp) {
		printf("\tto replacement point:\n");
		replace_replacement_point_println(frame->torp, 2);
	}

	printf("\n");
}
#endif /* !defined(NDEBUG) */


/* replace_sourcestate_println *************************************************

   Print source state

   IN:
       ss...............the source state to print

*******************************************************************************/

#if !defined(NDEBUG)
void replace_sourcestate_println(sourcestate_t *ss)
{
	int i;
	sourceframe_t *frame;

	if (!ss) {
		printf("(sourcestate_t *)NULL\n");
		return;
	}

	printf("sourcestate_t:\n");

	for (i=0, frame = ss->frames; frame != NULL; frame = frame->down, ++i) {
		printf("    frame %d:\n", i);
		replace_source_frame_println(frame);
	}
}
#endif


/* replace_sourcestate_println_short *******************************************

   Print a compact representation of the given source state.

   IN:
       ss...............the source state to print

*******************************************************************************/

#if !defined(NDEBUG)
void replace_sourcestate_println_short(sourcestate_t *ss)
{
	sourceframe_t *frame;

	for (frame = ss->frames; frame != NULL; frame = frame->down) {
		printf("\t");

		if (REPLACE_IS_NATIVE_FRAME(frame)) {
			printf("NATIVE (pc %p size %d) ",
				   (void*)frame->nativepc, frame->nativeframesize);
			replace_stackframeinfo_println(frame->sfi);
			continue;
		}

		if (frame->torp) {
			printf("%c", (frame->torp == frame->fromrp) ? '=' : '+');
		}

		printf("%s", replace_type_str[frame->fromrp->type]);

		if (frame->torp && frame->torp->type != frame->fromrp->type)
			printf("->%s", replace_type_str[frame->torp->type]);

		if (frame->tocode != frame->fromcode)
			printf(" (%p->%p/%d) ",
				   (void*) frame->fromcode, (void*) frame->tocode,
				   frame->fromrp->id);
		else
			printf(" (%p/%d) ", (void*) frame->fromcode, frame->fromrp->id);

		method_println(frame->method);
	}
}
#endif

#if !defined(NDEBUG)
static void replace_stackframeinfo_println(stackframeinfo_t *sfi)
{
	printf("prev=%p pv=%p sp=%p ra=%p xpc=%p method=",
			(void*)sfi->prev, (void*)sfi->pv, (void*)sfi->sp,
			(void*)sfi->ra, (void*)sfi->xpc);

	if (sfi->code)
		method_println(sfi->code->m);
	else
		printf("(nil)\n");
}
#endif


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
