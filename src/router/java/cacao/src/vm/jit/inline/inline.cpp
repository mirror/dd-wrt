/* src/vm/jit/inline/inline.cpp - method inlining

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

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "vm/types.hpp"

#include "mm/dumpmemory.hpp"

#include "threads/lock.hpp"
#include "threads/mutex.hpp"
#include "threads/thread.hpp"

#include "toolbox/logging.hpp"

#include "vm/class.hpp"
#include "vm/descriptor.hpp"            // for typedesc, methoddesc, etc
#include "vm/global.hpp"
#include "vm/initialize.hpp"
#include "vm/method.hpp"
#include "vm/options.hpp"
#include "vm/statistics.hpp"

#include "vm/jit/builtin.hpp"
#include "vm/jit/code.hpp"
#include "vm/jit/jit.hpp"
#include "vm/jit/parse.hpp"
#include "vm/jit/reg.hpp"
#include "vm/jit/show.hpp"
#include "vm/jit/stack.hpp"

#include "vm/jit/inline/inline.hpp"
#include "vm/jit/loop/loop.hpp"

#include "vm/jit/ir/instruction.hpp"

#include "vm/jit/verify/typecheck.hpp"


/* algorithm tuning constants *************************************************/

/* Algorithm Selection                                                        */
/* Define exactly one of the following three to select the inlining           */
/* heuristics.                                                                */

/*#define INLINE_DEPTH_FIRST*/
/*#define INLINE_BREADTH_FIRST*/
#define INLINE_KNAPSACK

/* Parameters for knapsack heuristics:                                        */

#if defined(INLINE_KNAPSACK)

#define INLINE_COUNTDOWN_INIT       1000
#define INLINE_COST_OFFSET          -16
#define INLINE_COST_BUDGET          100
/*#define INLINE_WEIGHT_BUDGET        5.0*/
/*#define INLINE_ADD_NEGATIVE_TO_BUDGET*/
/*#define INLINE_MAX_DEPTH            3*/
/*#define INLINE_DIVIDE_COST_BY_FREQ */

#endif

/* Parameters for depth-first heuristics:                                     */

#if defined(INLINE_DEPTH_FIRST)

#define INLINE_MAX_DEPTH            3
#define INLINE_MAX_BLOCK_EXPANSION  10
/*#define INLINE_MAX_ICMD_EXPANSION  10*/
/*#define INLINE_CANCEL_ON_THRESHOLD*/

#endif

/* Parameters for breadth-first heuristics:                                   */

#if defined(INLINE_BREADTH_FIRST)

/*#define INLINE_MAX_BLOCK_EXPANSION  10*/
#define INLINE_MAX_ICMD_EXPANSION  5

#endif


/* debugging ******************************************************************/

#if !defined(NDEBUG)
#define INLINE_VERBOSE
#define DOLOG(code)       do{ if (opt_TraceInlining >= 2) { code; } }while(0)
#define DOLOG_SHORT(code) do{ if (opt_TraceInlining >= 1) { code; } }while(0)
#else
#define DOLOG(code)
#endif

#if defined(ENABLE_VERIFIER) && !defined(NDEBUG)
/* Define this to verify the resulting code after inlining.                 */
/* Note: This is only useful for development and may require patches to the */
/*       verifier code.                                                     */
/* #define INLINE_VERIFY_RESULT */
#endif

/* types **********************************************************************/

typedef struct inline_node inline_node;
typedef struct inline_target_ref inline_target_ref;
typedef struct inline_context inline_context;
typedef struct inline_block_map inline_block_map;
typedef struct inline_site inline_site;
typedef struct inline_candidate inline_candidate;

struct inline_node {
	inline_context *ctx;

	jitdata *jd;
	methodinfo *m;
	inline_node *children;
	inline_node *next;                             /* next node at this depth */
	inline_node *prev;                             /* prev node at this depth */
	int depth;                                  /* inlining depth, 0 for root */

	/* info about the call site (if depth > 0)*/
	inline_node *parent;                /* node of the caller (NULL for root) */
	basicblock *callerblock;         /* original block containing the INVOKE* */
	instruction *callerins;               /* the original INVOKE* instruction */
	s4 callerpc;
	s4 *n_passthroughvars;
	int n_passthroughcount;
	int n_selfpassthroughcount;  /* # of pass-through vars of the call itself */
	exception_entry **o_handlers;
	int n_handlercount;                 /* # of handlers protecting this call */
	int n_resultlocal;
	int synclocal;                    /* variable used for synchr., or UNUSED */
	bool isstatic;                                   /* this is a static call */

	bool blockbefore;                  /* block boundary before inlined body? */
	bool blockafter;                   /* block boundary after inlined body?  */

	/* info about the callee */
	int localsoffset;
	int prolog_instructioncount;         /* # of ICMDs in the inlining prolog */
	int epilog_instructioncount;         /* # of ICMDs in the inlining epilog */
	int extra_instructioncount;
	int extra_exceptiontablelength;   /* # of extra handlers to put in caller */
	bool synchronize;                /* do we have to synchronize enter/exit? */
	basicblock *handler_monitorexit;     /* handler for synchronized inlinees */
	s4 *varmap;

	/* cumulative values */
	int cumul_instructioncount;  /* ICMDs in this node and its children       */
	int cumul_basicblockcount;   /* BBs started by this node and its children */
	int cumul_basicblockcount_root;  /* BBs that have to be added to the root */
	                                 /* node if this node is inlined          */
	int cumul_blockmapcount;
	int cumul_maxlocals;
	int cumul_exceptiontablelength;

	/* output */
	instruction *inlined_iinstr;
	instruction *inlined_iinstr_cursor;
	basicblock *inlined_basicblocks;
	basicblock *inlined_basicblocks_cursor;

	/* register data */
	registerdata *regdata;

	/* temporary */
	inline_target_ref *refs;
	instruction *inline_start_instruction;
	s4 *javalocals;

	/* XXX debug */
	char *indent;
	int debugnr;
};

struct inline_target_ref {
	inline_target_ref *next;
	union {
		basicblock **block;
		s4 *nr;
	} ref;
	basicblock *target;
	bool isnumber;
};

struct inline_block_map {
	inline_node *iln;
	basicblock *o_block;
	basicblock *n_block;
};

struct inline_context {
	inline_node *master;

	jitdata *resultjd;

	inline_candidate *candidates;

	int next_block_number;
	inline_block_map *blockmap;
	int blockmap_index;

	int maxinoutdepth;

	bool stopped;

	int next_debugnr; /* XXX debug */
};

struct inline_site {
	bool              speculative;  /* true, if inlining would be speculative */
	bool              inlined;      /* true, if this site has been inlined    */

	basicblock       *bptr;         /* basic block containing the call site   */
	instruction      *iptr;         /* the invocation instruction             */
	exception_entry **handlers;     /* active handlers at the call site       */
	s4                nhandlers;    /* number of active handlers              */
	s4                pc;           /* PC of the invocation instruction       */
};

struct inline_candidate {
	inline_candidate *next;
	int freq;
	int cost;
	double weight;
	inline_node *caller;
	methodinfo *callee;
	inline_site site;
};


/* prototypes *****************************************************************/

static bool inline_analyse_code(inline_node *iln);
static void inline_post_process(jitdata *jd);


/* debug helpers **************************************************************/

#if !defined(NDEBUG)
#include "inline_debug.inc"
#endif


/* statistics *****************************************************************/

/*#define INLINE_STATISTICS*/

#if !defined(NDEBUG)
#define INLINE_STATISTICS
#endif

#if defined(INLINE_STATISTICS)
int inline_stat_roots = 0;
int inline_stat_roots_transformed = 0;
int inline_stat_inlined_nodes = 0;
int inline_stat_max_depth = 0;

void inline_print_stats()
{
	printf("inlining statistics:\n");
	printf("    roots analysed   : %d\n", inline_stat_roots);
	printf("    roots transformed: %d\n", inline_stat_roots_transformed);
	printf("    inlined nodes    : %d\n", inline_stat_inlined_nodes);
	printf("    max depth        : %d\n", inline_stat_max_depth);
}
#endif


/* compilation of callees *****************************************************/

static bool inline_jit_compile_intern(jitdata *jd)
{
	methodinfo *m;

	/* XXX should share code with jit.c */

	assert(jd);

	/* XXX initialize the static function's class */

	m = jd->m;

	/* call the compiler passes ***********************************************/

	/* call parse pass */

	DOLOG( log_message_class("Parsing ", m->clazz) );
	if (!parse(jd)) {
		return false;
	}

	/* call stack analysis pass */

	if (!stack_analyse(jd)) {
		return false;
	}

	return true;
}


static bool inline_jit_compile(inline_node *iln)
{
	bool                r;
	methodinfo         *m;
	jitdata            *jd;

	/* XXX should share code with jit.c */

	assert(iln);
	m = iln->m;
	assert(m);

	/* enter a monitor on the method */

	m->mutex->lock();

	/* allocate jitdata structure and fill it */

	jd = jit_jitdata_new(m);
	iln->jd = jd;

	jd->flags = 0; /* XXX */

	/* initialize the register allocator */

	reg_setup(jd);

	/* setup the codegendata memory */

	/* XXX do a pseudo setup */
	jd->cd = (codegendata*) DumpMemory::allocate(sizeof(codegendata));
	MZERO(jd->cd, codegendata, 1);
	jd->cd->method = m;
	/* XXX uses too much dump memory codegen_setup(jd); */

	/* now call internal compile function */

	r = inline_jit_compile_intern(jd);

	if (r) {
		iln->regdata = jd->rd;
	}

	/* free some memory */
#if 0

#if defined(ENABLE_JIT)
# if defined(ENABLE_INTRP)
	if (!opt_intrp)
# endif
		codegen_free(jd);
#endif

#endif

	/* leave the monitor */

	m->mutex->unlock();

	return r;
}


/* inlining tree handling *****************************************************/

static void inline_insert_inline_node(inline_node *parent, inline_node *child)
{
	inline_node *first;
	inline_node *succ;

	assert(parent && child);

	child->parent = parent;

	child->debugnr = parent->ctx->next_debugnr++; /* XXX debug */

	first = parent->children;
	if (!first) {
		/* insert as only node */
		parent->children = child;
		child->next = child;
		child->prev = child;
		return;
	}

	/* {there is at least one child already there} */

	/* XXX is this search necessary, or could we always add at the end? */

	succ = first;
	while (succ->callerpc < child->callerpc) {
		succ = succ->next;
		if (succ == first) {
			/* insert as last node */
			child->prev = first->prev;
			child->next = first;
			child->prev->next = child;
			child->next->prev = child;
			return;
		}
	}

	assert(succ->callerpc > child->callerpc);

	/* insert before succ */

	child->prev = succ->prev;
	child->next = succ;
	child->prev->next = child;
	child->next->prev = child;

	if (parent->children == succ)
		parent->children = child;
}


static void inline_remove_inline_node(inline_node *parent, inline_node *child)
{
	assert(parent);
	assert(child);
	assert(child->parent == parent);

	if (child->prev == child) {
		/* remove the only child node */
		parent->children = NULL;
	}
	else {
		child->prev->next = child->next;
		child->next->prev = child->prev;

		if (parent->children == child)
			parent->children = child->next;
	}
}


/* inlining candidate handling ************************************************/

#if defined(INLINE_KNAPSACK) || defined(INLINE_BREADTH_FIRST)
static void inline_add_candidate(inline_context *ctx,
								 inline_node *caller,
								 methodinfo *callee,
								 inline_site *site)
{
	inline_candidate **link;
	inline_candidate *cand;

	cand = (inline_candidate*) DumpMemory::allocate(sizeof(inline_candidate));
#if defined(INLINE_DIVIDE_COST_BY_FREQ)
	cand->freq = INLINE_COUNTDOWN_INIT - callee->hitcountdown;
	if (cand->freq < 1)
#endif
		cand->freq = 1;
#if defined(INLINE_KNAPSACK)
	cand->cost = callee->jcodelength + INLINE_COST_OFFSET;
#endif
#if defined(INLINE_BREADTH_FIRST)
	cand->cost = caller->depth;
#endif
	cand->caller = caller;
	cand->callee = callee;
	cand->site = *site;

	cand->weight = (double)cand->cost / cand->freq;

	for (link = &(ctx->candidates); ; link = &((*link)->next)) {
		if (!*link || (*link)->weight > cand->weight) {
			cand->next = *link;
			*link = cand;
			break;
		}
	}
}
#endif /* defined(INLINE_KNAPSACK) || defined(INLINE_BREADTH_FIRST) */

#if defined(INLINE_KNAPSACK) || defined(INLINE_BREADTH_FIRST)
static inline_candidate * inline_pick_best_candidate(inline_context *ctx)
{
	inline_candidate *cand;

	cand = ctx->candidates;

	if (cand)
		ctx->candidates = cand->next;

	return cand;
}
#endif /* defined(INLINE_KNAPSACK) || defined(INLINE_BREADTH_FIRST) */

#if !defined(NDEBUG) && (defined(INLINE_KNAPSACK) || defined(INLINE_BREADTH_FIRST))
static void inline_candidate_println(inline_candidate *cand)
{
	printf("%10g (%5d / %5d) depth %2d ",
			cand->weight, cand->cost, cand->freq, cand->caller->depth + 1);
	method_println(cand->callee);
}
#endif /* !defined(NDEBUG) && (defined(INLINE_KNAPSACK) || defined(INLINE_BREADTH_FIRST)) */


#if !defined(NDEBUG) && (defined(INLINE_KNAPSACK) || defined(INLINE_BREADTH_FIRST))
static void inline_candidates_println(inline_context *ctx)
{
	inline_candidate *cand;

	for (cand = ctx->candidates; cand != NULL; cand = cand->next) {
		printf("    ");
		inline_candidate_println(cand);
	}
}
#endif /* !defined(NDEBUG) && (defined(INLINE_KNAPSACK) || defined(INLINE_BREADTH_FIRST)) */


/* variable handling **********************************************************/

static s4 inline_new_variable(jitdata *jd, Type type, s4 flags)
{
	s4 index;
	s4 newcount;

	index = jd->vartop++;
	if (index >= jd->varcount) {
		newcount = jd->vartop * 2; /* XXX */
		jd->var = (varinfo*) DumpMemory::reallocate(jd->var, sizeof(varinfo) * jd->varcount, sizeof(varinfo) * newcount);
		MZERO(jd->var + jd->varcount, varinfo, (newcount - jd->varcount));
		jd->varcount = newcount;
	}

	jd->var[index].type  = type;
	jd->var[index].flags = flags;

	return index;
}


static s4 inline_new_variable_clone(jitdata *jd, jitdata *origjd, s4 origidx)
{
	varinfo *v;
	s4       newidx;

	v = &(origjd->var[origidx]);

	newidx = inline_new_variable(jd, v->type, v->flags);

	jd->var[newidx].vv = v->vv;

	return newidx;
}


static s4 inline_new_temp_variable(jitdata *jd, Type type)
{
	return inline_new_variable(jd, type, 0);
}


static s4 inline_translate_variable(jitdata *jd, jitdata *origjd, s4 *varmap, s4 index)
{
	s4 idx;

	idx = varmap[index];

	if (idx < 0) {
		idx = inline_new_variable_clone(jd, origjd, index);
		varmap[index] = idx;
	}

	return idx;
}


static s4 *create_variable_map(inline_node *callee)
{
	s4 *varmap;
	s4 i, t;
	s4 varindex;
	s4 n_javaindex;
	s4 avail;
	varinfo *v;

	/* create the variable mapping */

	varmap = (s4*) DumpMemory::allocate(sizeof(s4) * callee->jd->varcount);
	for (i=0; i<callee->jd->varcount; ++i)
		varmap[i] = -1;

	/* translate local variables */

	for (i=0; i<callee->m->maxlocals; ++i) {
		for (t=0; t<5; ++t) {
			varindex = callee->jd->local_map[5*i + t];
			if (varindex == jitdata::UNUSED)
				continue;

			v = &(callee->jd->var[varindex]);
			assert(v->type == t || v->type == TYPE_VOID); /* XXX stack leaves VOID */
			v->type = (Type) t; /* XXX restore if it is TYPE_VOID */

			avail = callee->ctx->resultjd->local_map[5*(callee->localsoffset + i) + t];

			if (avail == jitdata::UNUSED) {
				avail = inline_new_variable_clone(callee->ctx->resultjd, callee->jd, varindex);
				callee->ctx->resultjd->local_map[5*(callee->localsoffset + i) + t] = avail;
			}

			varmap[varindex] = avail;
		}
	}

	/* for synchronized instance methods we need an extra local */

	if (callee->synchronize && !(callee->m->flags & ACC_STATIC)) {
		n_javaindex = callee->localsoffset - 1;
		assert(n_javaindex >= 0);
		assert(callee->parent);
		assert(n_javaindex == callee->parent->localsoffset + callee->parent->m->maxlocals);

		avail = callee->ctx->resultjd->local_map[5*n_javaindex + TYPE_ADR];

		if (avail == jitdata::UNUSED) {
			avail = inline_new_variable(callee->ctx->resultjd, TYPE_ADR, 0);
			callee->ctx->resultjd->local_map[5*n_javaindex + TYPE_ADR] = avail;
		}

		callee->synclocal = avail;
	}
	else {
		callee->synclocal = jitdata::UNUSED;
	}

	return varmap;
}


/* basic block translation ****************************************************/

#define INLINE_RETURN_REFERENCE(callee)  \
	( (basicblock *) (ptrint) (0x333 + (callee)->depth) )


static void inline_add_block_reference(inline_node *iln, basicblock **blockp)
{
	inline_target_ref *ref;

	ref = (inline_target_ref*) DumpMemory::allocate(sizeof(inline_target_ref));
	ref->ref.block = blockp;
	ref->isnumber = false;
	ref->next = iln->refs;
	iln->refs = ref;
}


#if 0
static void inline_add_blocknr_reference(inline_node *iln, s4 *nrp)
{
	inline_target_ref *ref;

	ref = (inline_target_ref*) DumpMemory::allocate(inline_target_ref);
	ref->ref.nr = nrp;
	ref->isnumber = true;
	ref->next = iln->refs;
	iln->refs = ref;
}
#endif


static void inline_block_translation(inline_node *iln, basicblock *o_bptr, basicblock *n_bptr)
{
	inline_context *ctx;

	ctx = iln->ctx;
	assert(ctx->blockmap_index < ctx->master->cumul_blockmapcount);

	ctx->blockmap[ctx->blockmap_index].iln = iln;
	ctx->blockmap[ctx->blockmap_index].o_block = o_bptr;
	ctx->blockmap[ctx->blockmap_index].n_block = n_bptr;

	ctx->blockmap_index++;
}


static basicblock * inline_map_block(inline_node *iln,
									 basicblock *o_block,
									 inline_node *targetiln)
{
	inline_block_map *bm;
	inline_block_map *bmend;

	assert(iln);
	assert(targetiln);

	if (!o_block)
		return NULL;

	bm = iln->ctx->blockmap;
	bmend = bm + iln->ctx->blockmap_index;

	while (bm < bmend) {
		assert(bm->iln && bm->o_block && bm->n_block);
		if (bm->o_block == o_block && bm->iln == targetiln)
			return bm->n_block;
		bm++;
	}

	assert(false);
	return NULL; /* not reached */
}


static void inline_resolve_block_refs(inline_target_ref **refs,
									  basicblock *o_bptr,
									  basicblock *n_bptr,
									  bool returnref)
{
	inline_target_ref *ref;
	inline_target_ref *prev;

	prev = NULL;
	for (ref = *refs; ref != NULL; ref = ref->next) {
		if (ref->isnumber && !returnref) {
			if (*(ref->ref.nr) == JAVALOCAL_FROM_RETADDR(o_bptr->nr)) {
				*(ref->ref.nr) = JAVALOCAL_FROM_RETADDR(n_bptr->nr);
				goto remove_ref;
			}
		}
		else {
			if (*(ref->ref.block) == o_bptr) {
				*(ref->ref.block) = n_bptr;
				goto remove_ref;
			}
		}

		/* skip this ref */

		prev = ref;
		continue;

remove_ref:
		/* remove this ref */

		if (prev) {
			prev->next = ref->next;
		}
		else {
			*refs = ref->next;
		}
	}
}


/* basic block creation *******************************************************/

static basicblock * create_block(inline_node *container,
								 inline_node *iln,
								 inline_node *inner,
								 int indepth)
{
	basicblock  *n_bptr;
	inline_node *outer;
	s4           i;
	s4           depth;
	s4           varidx;
	s4           newvaridx;

	assert(container);
	assert(iln);
	assert(inner);
	assert(indepth >= 0);

	n_bptr = container->inlined_basicblocks_cursor++;
	assert(n_bptr);
	assert((n_bptr - container->inlined_basicblocks) < container->cumul_basicblockcount);

	BASICBLOCK_INIT(n_bptr, iln->m);

	n_bptr->iinstr  = container->inlined_iinstr_cursor;
	n_bptr->next    = n_bptr + 1;
	n_bptr->nr      = container->ctx->next_block_number++;
	n_bptr->indepth = indepth;
	n_bptr->state   = basicblock::FINISHED; /* XXX */

	/* set the inlineinfo of the new block */

	if (iln->inline_start_instruction)
		n_bptr->inlineinfo = iln->inline_start_instruction->sx.s23.s3.inlineinfo;

	if (indepth > container->ctx->maxinoutdepth)
		container->ctx->maxinoutdepth = indepth;

	if (indepth) {
		n_bptr->invars = (s4*) DumpMemory::allocate(sizeof(s4) * indepth);


		for (i=0; i<indepth; ++i)
			n_bptr->invars[i] = -1; /* XXX debug */

		/* pass-through variables enter the block */

		outer = inner->parent;
		while (outer != NULL) {
			depth = outer->n_passthroughcount;

			assert(depth + inner->n_selfpassthroughcount <= indepth);

			for (i=0; i<inner->n_selfpassthroughcount; ++i) {
				varidx = inner->n_passthroughvars[i];
				newvaridx =
					inline_new_variable_clone(container->ctx->resultjd,
											  outer->jd,
											  varidx);
				n_bptr->invars[depth + i] = newvaridx;
				outer->varmap[varidx] = newvaridx;
			}
			inner = outer;
			outer = outer->parent;
		}
	}
	else {
		n_bptr->invars = NULL;
	}

	/* XXX for the verifier. should not be here */

	{
		varinfo *dv;

		dv = (varinfo*) DumpMemory::allocate(sizeof(varinfo) * (iln->ctx->resultjd->localcount + VERIFIER_EXTRA_LOCALS));
		MZERO(dv, varinfo,  iln->ctx->resultjd->localcount + VERIFIER_EXTRA_LOCALS);
		n_bptr->inlocals = dv;
	}

	return n_bptr;
}


static s4 *translate_javalocals(inline_node *iln, s4 *javalocals)
{
	s4 *jl;
	s4 i, j;

	jl = (s4*) DumpMemory::allocate(sizeof(s4) * iln->jd->maxlocals);

	for (i=0; i<iln->jd->maxlocals; ++i) {
		j = javalocals[i];
		if (j > jitdata::UNUSED)
			j = inline_translate_variable(iln->ctx->resultjd, iln->jd, iln->varmap, j);
		jl[i] = j;

#if 0
		if (j < jitdata::UNUSED) {
			/* an encoded returnAddress value - must be relocated */
			inline_add_blocknr_reference(iln, &(jl[i]));
		}
#endif
	}

	return jl;
}


static basicblock * create_body_block(inline_node *iln,
									  basicblock *o_bptr, s4 *varmap)
{
	basicblock *n_bptr;
	s4 i;

	n_bptr = create_block(iln, iln, iln,
						  o_bptr->indepth + iln->n_passthroughcount);

	n_bptr->type     = o_bptr->type;
	n_bptr->state    = o_bptr->state;
	n_bptr->bitflags = o_bptr->bitflags;

	/* resolve references to this block */

	inline_resolve_block_refs(&(iln->refs), o_bptr, n_bptr, false);

	/* translate the invars of the original block */

	for (i=0; i<o_bptr->indepth; ++i) {
		n_bptr->invars[iln->n_passthroughcount + i] =
			inline_translate_variable(iln->ctx->resultjd, iln->jd,
				varmap,
				o_bptr->invars[i]);
	}

	/* translate javalocals info (not for dead code) */

	if (n_bptr->state >= basicblock::REACHED)
		n_bptr->javalocals = translate_javalocals(iln, o_bptr->javalocals);

	return n_bptr;
}


static basicblock * create_epilog_block(inline_node *caller, inline_node *callee, s4 *varmap)
{
	basicblock *n_bptr;
	s4 retcount;
	s4 idx;

	/* number of return variables */

	retcount = (callee->n_resultlocal == -1
				&& callee->m->parseddesc->returntype.type != TYPE_VOID) ? 1 : 0;

	/* start the epilog block */

	n_bptr = create_block(caller, caller, callee,
						  callee->n_passthroughcount + retcount);

	/* resolve references to the return block */

	inline_resolve_block_refs(&(callee->refs),
							  INLINE_RETURN_REFERENCE(callee),
							  n_bptr,
							  true);

	/* return variable */

	if (retcount) {
		idx = inline_new_variable(caller->ctx->resultjd,
			   callee->m->parseddesc->returntype.type, 0 /* XXX */);
		n_bptr->invars[callee->n_passthroughcount] = idx;
		varmap[callee->callerins->dst.varindex] = idx;
	}

	/* set javalocals */

	n_bptr->javalocals = (s4*) DumpMemory::allocate(sizeof(s4) * caller->jd->maxlocals);
	MCOPY(n_bptr->javalocals, caller->javalocals, s4, caller->jd->maxlocals);

	/* set block flags & type */

	n_bptr->state = basicblock::FINISHED; // XXX original block flags
	n_bptr->type  = basicblock::TYPE_STD;

	return n_bptr;
}


static void close_block(inline_node *iln, inline_node *inner, basicblock *n_bptr, s4 outdepth)
{
	inline_node *outer;
	s4           i;
	s4           depth;
	s4           varidx;

	n_bptr->outdepth = outdepth;
	n_bptr->outvars = (s4*) DumpMemory::allocate(sizeof(s4) * outdepth);

	for (i=0; i<outdepth; ++i)
		n_bptr->outvars[i] = 0; /* XXX debug */

	if (outdepth > iln->ctx->maxinoutdepth)
		iln->ctx->maxinoutdepth = outdepth;

	/* pass-through variables leave the block */

	outer = inner->parent;
	while (outer != NULL) {
		depth = outer->n_passthroughcount;

		assert(depth + inner->n_selfpassthroughcount <= outdepth);

		for (i=0; i<inner->n_selfpassthroughcount; ++i) {
			varidx = inner->n_passthroughvars[i];
			n_bptr->outvars[depth + i] =
				inline_translate_variable(iln->ctx->resultjd,
										  outer->jd,
										  outer->varmap,
										  varidx);
		}
		inner = outer;
		outer = outer->parent;
	}
}


static void close_prolog_block(inline_node *iln,
							   basicblock *n_bptr,
							   inline_node *nextcall)
{
	/* XXX add original outvars! */
	close_block(iln, nextcall, n_bptr, nextcall->n_passthroughcount);

	/* pass-through variables */

	DOLOG( printf("closed prolog block:\n");
		   show_basicblock(iln->ctx->resultjd, n_bptr, SHOW_STACK); );
}


static void close_body_block(inline_node *iln,
							 basicblock *n_bptr,
							 basicblock *o_bptr,
							 s4 *varmap,
							 s4 retcount,
							 s4 retidx)
{
	s4 i;

	close_block(iln, iln, n_bptr, iln->n_passthroughcount + o_bptr->outdepth + retcount);

	/* translate the outvars of the original block */

	/* XXX reuse code */
	for (i=0; i<o_bptr->outdepth; ++i) {
		n_bptr->outvars[iln->n_passthroughcount + i] =
			inline_translate_variable(iln->ctx->resultjd, iln->jd, varmap,
					o_bptr->outvars[i]);
	}

	/* set the return variable, if any */

	if (retcount) {
		assert(retidx >= 0);
		n_bptr->outvars[iln->n_passthroughcount + o_bptr->outdepth] = retidx;
	}
}


/* inlined code generation ****************************************************/

static instruction * inline_instruction(inline_node *iln,
										ICMD         opcode,
										instruction *o_iptr)
{
	instruction *n_iptr;

	n_iptr = (iln->inlined_iinstr_cursor++);
	assert((n_iptr - iln->inlined_iinstr) < iln->cumul_instructioncount);

	n_iptr->opc = opcode;
	n_iptr->flags.bits = o_iptr->flags.bits & INS_FLAG_ID_MASK;
	n_iptr->line = o_iptr->line;

	return n_iptr;
}

static void inline_generate_sync_builtin(inline_node *iln,
										 inline_node *callee,
										 instruction *o_iptr,
										 s4 instancevar,
										 functionptr func)
{
	int          syncvar;
	instruction *n_ins;

	if (callee->m->flags & ACC_STATIC) {
		/* ACONST */
		syncvar = inline_new_temp_variable(iln->ctx->resultjd, TYPE_ADR);

		n_ins = inline_instruction(iln, ICMD_ACONST, o_iptr);
		n_ins->sx.val.c.cls = callee->m->clazz;
		n_ins->dst.varindex = syncvar;
		n_ins->flags.bits |= INS_FLAG_CLASS;
	}
	else {
		syncvar = instancevar;
	}

	assert(syncvar != jitdata::UNUSED);

	/* MONITORENTER / MONITOREXIT */

	n_ins = inline_instruction(iln, ICMD_BUILTIN, o_iptr);
	n_ins->sx.s23.s3.bte = builtintable_get_internal(func);
	n_ins->s1.argcount = 1; /* XXX add through-vars */
	n_ins->sx.s23.s2.args = (s4*) DumpMemory::allocate(sizeof(s4));
	n_ins->sx.s23.s2.args[0] = syncvar;
}

static s4 emit_inlining_prolog(inline_node *iln,
							   inline_node *callee,
							   instruction *o_iptr,
							   s4 *varmap)
{
	methodinfo *calleem;
	methoddesc *md;
	int i;
	int localindex;
	int type;
	instruction *n_ins;
	insinfo_inline *insinfo;
	s4 varindex;

	assert(iln && callee && o_iptr);

	calleem = callee->m;
	md = calleem->parseddesc;

	/* INLINE_START instruction */

	n_ins = inline_instruction(iln, ICMD_INLINE_START, o_iptr);

	insinfo = (insinfo_inline*) DumpMemory::allocate(sizeof(insinfo_inline));
	insinfo->method = callee->m;
	insinfo->outer = iln->m;
	insinfo->synclocal = callee->synclocal;
	insinfo->synchronize = callee->synchronize;
	insinfo->javalocals_start = NULL;
	insinfo->javalocals_end = NULL;

	/* info about stack vars live at the INLINE_START */

	insinfo->throughcount = callee->n_passthroughcount;
	insinfo->paramcount = md->paramcount;
	insinfo->stackvarscount = o_iptr->s1.argcount;
	insinfo->stackvars = (s4*) DumpMemory::allocate(sizeof(s4) * insinfo->stackvarscount);
	for (i=0; i<insinfo->stackvarscount; ++i)
		insinfo->stackvars[i] = iln->varmap[o_iptr->sx.s23.s2.args[i]];

	/* info about the surrounding inlining */

	if (iln->inline_start_instruction)
		insinfo->parent = iln->inline_start_instruction->sx.s23.s3.inlineinfo;
	else
		insinfo->parent = NULL;

	/* finish the INLINE_START instruction */

	n_ins->sx.s23.s3.inlineinfo = insinfo;
	callee->inline_start_instruction = n_ins;

	DOLOG( printf("%sprolog: ", iln->indent);
		   show_icmd(iln->ctx->resultjd, n_ins, false, SHOW_STACK); printf("\n"); );

	/* handle parameters for the inlined callee */

	localindex = callee->localsoffset + md->paramslots;

	for (i=md->paramcount-1; i>=0; --i) {
		assert(iln);

		type = md->paramtypes[i].type;

		localindex -= IS_2_WORD_TYPE(type) ? 2 : 1;
		assert(callee->regdata);

		/* translate the argument variable */

		varindex = varmap[o_iptr->sx.s23.s2.args[i]];
		assert(varindex != jitdata::UNUSED);

		/* remove preallocation from the argument variable */

		iln->ctx->resultjd->var[varindex].flags &= ~(PREALLOC | INMEMORY);

		/* check the instance slot against NULL */
		/* we don't need that for <init> methods, as the verifier  */
		/* ensures that they are only called for an uninit. object */
		/* (which may not be NULL).                                */

		if (!callee->isstatic && i == 0 && calleem->name != utf8::init) {
			assert(type == TYPE_ADR);
			n_ins = inline_instruction(iln, ICMD_CHECKNULL, o_iptr);
			n_ins->s1.varindex = varindex;
			n_ins->dst.varindex = n_ins->s1.varindex;
		}

		/* store argument into local variable of inlined callee */

		if (callee->jd->local_map[5*(localindex - callee->localsoffset) + type] != jitdata::UNUSED)
		{
			/* this value is used in the callee */

			if (i == 0 && callee->synclocal != jitdata::UNUSED) {
				/* we also need it for synchronization, so copy it */
				assert(type == TYPE_ADR);
				n_ins = inline_instruction(iln, ICMD_COPY, o_iptr);
			}
			else {
				n_ins = inline_instruction(iln, ICMD(ICMD_ISTORE + type), o_iptr);
				n_ins->sx.s23.s3.javaindex = jitdata::UNUSED;
			}
			n_ins->s1.varindex = varindex;
			n_ins->dst.varindex = iln->ctx->resultjd->local_map[5*localindex + type];
			assert(n_ins->dst.varindex != jitdata::UNUSED);
		}
		else if (i == 0 && callee->synclocal != jitdata::UNUSED) {
			/* the value is not used inside the callee, but we need it for */
			/* synchronization                                             */
			/* XXX In this case it actually makes no sense to create a     */
			/*     separate synchronization variable.                      */

			n_ins = inline_instruction(iln, ICMD_NOP, o_iptr);
		}
		else {
			/* this value is not used, pop it */

			n_ins = inline_instruction(iln, ICMD_POP, o_iptr);
			n_ins->s1.varindex = varindex;
		}

		DOLOG( printf("%sprolog: ", iln->indent);
			   show_icmd(iln->ctx->resultjd, n_ins, false, SHOW_STACK); printf("\n"); );
	}

	/* COPY for synchronized instance methods */

	if (callee->synclocal != jitdata::UNUSED) {
		n_ins = inline_instruction(iln, ICMD_COPY, o_iptr);
		n_ins->s1.varindex = varmap[o_iptr->sx.s23.s2.args[0]];
		n_ins->dst.varindex = callee->synclocal;

		assert(n_ins->s1.varindex != jitdata::UNUSED);
	}

	if (callee->synchronize) {
		inline_generate_sync_builtin(iln, callee, o_iptr,
									 (callee->isstatic) ? jitdata::UNUSED : varmap[o_iptr->sx.s23.s2.args[0]],
									 LOCK_monitor_enter);
	}

	/* INLINE_BODY instruction */

	n_ins = inline_instruction(iln, ICMD_INLINE_BODY, callee->jd->basicblocks[0].iinstr);
	n_ins->sx.s23.s3.inlineinfo = insinfo;

	return 0; /* XXX */
}


static void emit_inlining_epilog(inline_node *iln, inline_node *callee, instruction *o_iptr)
{
	instruction *n_ins;
	s4          *jl;

	assert(iln && callee && o_iptr);
	assert(callee->inline_start_instruction);

	if (callee->synchronize) {
		inline_generate_sync_builtin(iln, callee, o_iptr,
									 callee->synclocal,
									 LOCK_monitor_exit);
	}

	/* INLINE_END instruction */

	n_ins = inline_instruction(iln, ICMD_INLINE_END, o_iptr);
	n_ins->sx.s23.s3.inlineinfo = callee->inline_start_instruction->sx.s23.s3.inlineinfo;

	/* set the javalocals */

	jl = (s4*) DumpMemory::allocate(sizeof(s4) * iln->jd->maxlocals);
	MCOPY(jl, iln->javalocals, s4, iln->jd->maxlocals);
	n_ins->sx.s23.s3.inlineinfo->javalocals_end = jl;

	DOLOG( printf("%sepilog: ", iln->indent);
		   show_icmd(iln->ctx->resultjd, n_ins, false, SHOW_STACK); printf("\n"); );
}


#define TRANSLATE_VAROP(vo)  \
	n_iptr->vo.varindex = inline_translate_variable(jd, origjd, varmap, n_iptr->vo.varindex)


static void inline_clone_instruction(inline_node *iln,
									 jitdata *jd,
									 jitdata *origjd,
									 s4 *varmap,
									 instruction *o_iptr,
									 instruction *n_iptr)
{
	icmdtable_entry_t *icmdt;
	builtintable_entry *bte;
	methoddesc *md;
	s4 i, j;
	branch_target_t *table;
	lookup_target_t *lookup;
	inline_node *scope;

	*n_iptr = *o_iptr;

	icmdt = &(icmd_table[o_iptr->opc]);

	switch (icmdt->dataflow) {
		case DF_0_TO_0:
			break;

		case DF_3_TO_0:
			TRANSLATE_VAROP(sx.s23.s3);
		case DF_2_TO_0:
			TRANSLATE_VAROP(sx.s23.s2);
		case DF_1_TO_0:
			TRANSLATE_VAROP(s1);
			break;

		case DF_2_TO_1:
			TRANSLATE_VAROP(sx.s23.s2);
		case DF_1_TO_1:
		case DF_COPY:
		case DF_MOVE:
			TRANSLATE_VAROP(s1);
		case DF_0_TO_1:
			TRANSLATE_VAROP(dst);
			break;

		case DF_N_TO_1:
			n_iptr->sx.s23.s2.args = (s4*) DumpMemory::allocate(sizeof(s4) * n_iptr->s1.argcount);
			for (i=0; i<n_iptr->s1.argcount; ++i) {
				n_iptr->sx.s23.s2.args[i] =
					inline_translate_variable(jd, origjd, varmap,
							o_iptr->sx.s23.s2.args[i]);
			}
			TRANSLATE_VAROP(dst);
			break;

		case DF_INVOKE:
			INSTRUCTION_GET_METHODDESC(n_iptr, md);
clone_call:
			n_iptr->s1.argcount += iln->n_passthroughcount;
			n_iptr->sx.s23.s2.args = (s4*) DumpMemory::allocate(sizeof(s4) * n_iptr->s1.argcount);
			for (i=0; i<o_iptr->s1.argcount; ++i) {
				n_iptr->sx.s23.s2.args[i] =
					inline_translate_variable(jd, origjd, varmap,
							o_iptr->sx.s23.s2.args[i]);
			}
			for (scope = iln; scope != NULL; scope = scope->parent) {
				for (j = 0; j < scope->n_selfpassthroughcount; ++j) {
					n_iptr->sx.s23.s2.args[i++] =
						scope->parent->varmap[scope->n_passthroughvars[j]];
				}
			}
			if (md->returntype.type != TYPE_VOID)
				TRANSLATE_VAROP(dst);
			break;

		case DF_BUILTIN:
			bte = n_iptr->sx.s23.s3.bte;
			md = bte->md;
			goto clone_call;

		default:
			assert(0);
	}

	switch (icmdt->controlflow) {
		case CF_RET:
			TRANSLATE_VAROP(s1); /* XXX should be handled by data-flow */
			/* FALLTHROUGH */
		case CF_IF:
		case CF_GOTO:
			inline_add_block_reference(iln, &(n_iptr->dst.block));
			break;

		case CF_JSR:
			inline_add_block_reference(iln, &(n_iptr->sx.s23.s3.jsrtarget.block));
			break;

		case CF_TABLE:
			i = n_iptr->sx.s23.s3.tablehigh - n_iptr->sx.s23.s2.tablelow + 1 + 1 /* default */;

			table = (branch_target_t*) DumpMemory::allocate(sizeof(branch_target_t) *  i);
			MCOPY(table, o_iptr->dst.table, branch_target_t, i);
			n_iptr->dst.table = table;

			while (--i >= 0) {
				inline_add_block_reference(iln, &(table->block));
				table++;
			}
			break;

		case CF_LOOKUP:
			inline_add_block_reference(iln, &(n_iptr->sx.s23.s3.lookupdefault.block));

			i = n_iptr->sx.s23.s2.lookupcount;
			lookup = (lookup_target_t*) DumpMemory::allocate(sizeof(lookup_target_t) * i);
			MCOPY(lookup, o_iptr->dst.lookup, lookup_target_t, i);
			n_iptr->dst.lookup = lookup;

			while (--i >= 0) {
				inline_add_block_reference(iln, &(lookup->target.block));
				lookup++;
			}
			break;
	}

	/* XXX move this to dataflow section? */

	switch (n_iptr->opc) {
		case ICMD_ASTORE:
#if 0
			if (n_iptr->flags.bits & INS_FLAG_RETADDR)
				inline_add_blocknr_reference(iln, &(n_iptr->sx.s23.s2.retaddrnr));
#endif
			/* FALLTHROUGH! */
		case ICMD_ISTORE:
		case ICMD_LSTORE:
		case ICMD_FSTORE:
		case ICMD_DSTORE:
			stack_javalocals_store(n_iptr, iln->javalocals);
			break;
		default:
			break;
	}
}


static void inline_rewrite_method(inline_node *iln)
{
	basicblock *o_bptr;
	s4 len;
	instruction *o_iptr;
	instruction *n_iptr;
	inline_node *nextcall;
	basicblock *n_bptr;
	inline_block_map *bm;
	int i;
	int icount;
	jitdata *resultjd;
	jitdata *origjd;
	char indent[100]; /* XXX debug */
	s4 retcount;
	s4 retidx;

	assert(iln);

	resultjd = iln->ctx->resultjd;
	origjd = iln->jd;

	n_bptr = NULL;
	nextcall = iln->children;

	/* XXX debug */
	for (i=0; i<iln->depth; ++i)
		indent[i] = '\t';
	indent[i] = 0;
	iln->indent = indent;

	DOLOG( printf("%srewriting: ", indent); method_println(iln->m);
		   printf("%s(passthrough: %d+%d)\n",
				indent, iln->n_passthroughcount - iln->n_selfpassthroughcount,
				iln->n_passthroughcount); );

	/* set memory cursors */

	iln->inlined_iinstr_cursor = iln->inlined_iinstr;
	iln->inlined_basicblocks_cursor = iln->inlined_basicblocks;

	/* allocate temporary buffers */

	iln->javalocals = (s4*) DumpMemory::allocate(sizeof(s4) * iln->jd->maxlocals);

	/* loop over basic blocks */

	o_bptr = iln->jd->basicblocks;
	for (; o_bptr; o_bptr = o_bptr->next) {

		if (o_bptr->state < basicblock::REACHED) {

			/* ignore the dummy end block */

			if (o_bptr->icount == 0 && o_bptr->next == NULL) {
				/* enter the following block as translation, for exception handler ranges */
				inline_block_translation(iln, o_bptr, iln->inlined_basicblocks_cursor);
				continue;
			}

			DOLOG(
			printf("%s* skipping old L%03d (flags=%d, type=%d, oid=%d) of ",
					indent,
					o_bptr->nr, o_bptr->state, o_bptr->type,
					o_bptr->indepth);
			method_println(iln->m);
			);

			n_bptr = create_body_block(iln, o_bptr, iln->varmap);

			/* enter it in the blockmap */

			inline_block_translation(iln, o_bptr, n_bptr);

			close_body_block(iln, n_bptr, o_bptr, iln->varmap, 0, -1);
			continue;
		}

		len = o_bptr->icount;
		o_iptr = o_bptr->iinstr;

		DOLOG(
		printf("%s* rewriting old L%03d (flags=%d, type=%d, oid=%d) of ",
				indent,
				o_bptr->nr, o_bptr->state, o_bptr->type,
				o_bptr->indepth);
		method_println(iln->m);
		show_basicblock(iln->jd, o_bptr, SHOW_STACK);
		);

		if (iln->blockbefore || o_bptr != iln->jd->basicblocks) {
			/* create an inlined clone of this block */

			n_bptr = create_body_block(iln, o_bptr, iln->varmap);
			icount = 0;

			/* enter it in the blockmap */

			inline_block_translation(iln, o_bptr, n_bptr);

			/* initialize the javalocals */

			MCOPY(iln->javalocals, n_bptr->javalocals, s4, iln->jd->maxlocals);
		}
		else {
			s4 *jl;

			/* continue caller block */

			n_bptr = iln->inlined_basicblocks_cursor - 1;
			icount = n_bptr->icount;

			/* translate the javalocals */

			jl = translate_javalocals(iln, o_bptr->javalocals);
			iln->inline_start_instruction->sx.s23.s3.inlineinfo->javalocals_start = jl;

			MCOPY(iln->javalocals, jl, s4, iln->jd->maxlocals);
		}

		/* iterate over the ICMDs of this block */

		retcount = 0;
		retidx = jitdata::UNUSED;

		while (--len >= 0) {

			DOLOG( fputs(indent, stdout); show_icmd(iln->jd, o_iptr, false,  SHOW_STACK);
				   printf("\n") );

			/* handle calls that will be inlined */

			if (nextcall && o_iptr == nextcall->callerins) {

				/* write the inlining prolog */

				(void) emit_inlining_prolog(iln, nextcall, o_iptr, iln->varmap);
				icount += nextcall->prolog_instructioncount;

				/* end current block, or glue blocks together */

				n_bptr->icount = icount;

				if (nextcall->blockbefore) {
					close_prolog_block(iln, n_bptr, nextcall);
				}
				else {
					/* XXX */
				}

				/* check if the result is a local variable */

				if (nextcall->m->parseddesc->returntype.type != TYPE_VOID
						&& o_iptr->dst.varindex < iln->jd->localcount)
				{
					nextcall->n_resultlocal = iln->varmap[o_iptr->dst.varindex];
				}
				else
					nextcall->n_resultlocal = -1;

				/* set memory pointers in the callee */

				nextcall->inlined_iinstr = iln->inlined_iinstr_cursor;
				nextcall->inlined_basicblocks = iln->inlined_basicblocks_cursor;

				/* recurse */

				DOLOG( printf("%sentering inline ", indent);
					   show_icmd(origjd, o_iptr, false, SHOW_STACK); printf("\n") );

				inline_rewrite_method(nextcall);

				DOLOG( printf("%sleaving inline ", indent);
					   show_icmd(origjd, o_iptr, false, SHOW_STACK); printf("\n") );

				/* update memory cursors */

				assert(nextcall->inlined_iinstr_cursor
						<= iln->inlined_iinstr_cursor + nextcall->cumul_instructioncount);
				assert(nextcall->inlined_basicblocks_cursor
						== iln->inlined_basicblocks_cursor + nextcall->cumul_basicblockcount);
				iln->inlined_iinstr_cursor = nextcall->inlined_iinstr_cursor;
				iln->inlined_basicblocks_cursor = nextcall->inlined_basicblocks_cursor;

				/* start new block, or glue blocks together */

				if (nextcall->blockafter) {
					n_bptr = create_epilog_block(iln, nextcall, iln->varmap);
					icount = 0;
				}
				else {
					n_bptr = iln->inlined_basicblocks_cursor - 1;
					icount = n_bptr->icount;
					/* XXX */
				}

				/* emit inlining epilog */

				emit_inlining_epilog(iln, nextcall, o_iptr);
				icount += nextcall->epilog_instructioncount;

				/* proceed to next call */

				nextcall = nextcall->next;
			}
			else {
				n_iptr = (iln->inlined_iinstr_cursor++);
				assert((n_iptr - iln->inlined_iinstr) < iln->cumul_instructioncount);

				switch (o_iptr->opc) {
					case ICMD_RETURN:
						if (iln->depth == 0)
							goto default_clone;
						goto return_tail;

					case ICMD_IRETURN:
					case ICMD_ARETURN:
					case ICMD_LRETURN:
					case ICMD_FRETURN:
					case ICMD_DRETURN:
						if (iln->depth == 0)
							goto default_clone;
						retcount = 1;
						retidx = iln->varmap[o_iptr->s1.varindex];
						if (iln->n_resultlocal != -1) {
							/* store result in a local variable */

							DOLOG( printf("USING RESULTLOCAL %d\n", iln->n_resultlocal); );
							/* This relies on the same sequence of types for */
							/* ?STORE and ?RETURN opcodes.                   */
							n_iptr->opc                 = ICMD(ICMD_ISTORE + (o_iptr->opc - ICMD_IRETURN));
							n_iptr->s1.varindex         = retidx;
							n_iptr->dst.varindex        = iln->n_resultlocal;
							n_iptr->sx.s23.s3.javaindex = jitdata::UNUSED; /* XXX set real javaindex? */

							retcount = 0;
							retidx = jitdata::UNUSED;

							n_iptr = (iln->inlined_iinstr_cursor++);
							assert((n_iptr - iln->inlined_iinstr) < iln->cumul_instructioncount);
							icount++;
						}
						else if ((retidx < resultjd->localcount && iln->blockafter)
								|| !iln->blockafter) /* XXX do we really always need the MOVE? */
						{
							/* local must not become outvar, insert a MOVE */

							n_iptr->opc = ICMD_MOVE;
							n_iptr->s1.varindex = retidx;
							retidx = inline_new_temp_variable(resultjd,
															  resultjd->var[retidx].type);
							n_iptr->dst.varindex = retidx;

							n_iptr = (iln->inlined_iinstr_cursor++);
							assert((n_iptr - iln->inlined_iinstr) < iln->cumul_instructioncount);
							icount++;
						}
return_tail:
						if (iln->blockafter) {
							n_iptr->opc = ICMD_GOTO;
							n_iptr->dst.block = INLINE_RETURN_REFERENCE(iln);
							inline_add_block_reference(iln, &(n_iptr->dst.block));
						}
						else {
							n_iptr->opc = ICMD_NOP;
						}
						break;
#if 0
						if (o_bptr->next == NULL || (o_bptr->next->icount==0 && o_bptr->next->next == NULL)) {
							n_iptr->opc = ICMD_NOP;
							break;
						}
						goto default_clone;
						break;
#endif

					default:
default_clone:
						inline_clone_instruction(iln, resultjd, iln->jd, iln->varmap, o_iptr, n_iptr);
				}

				DOLOG( fputs(indent, stdout); show_icmd(resultjd, n_iptr, false, SHOW_STACK);
					   printf("\n"););

				icount++;
			}

			o_iptr++;
		}

		/* end of basic block */

		if (iln->blockafter || (o_bptr->next && o_bptr->next->next)) {
			close_body_block(iln, n_bptr, o_bptr, iln->varmap, retcount, retidx);
			n_bptr->icount = icount;

			DOLOG( printf("closed body block:\n");
				   show_basicblock(resultjd, n_bptr, SHOW_STACK); );
		}
		else {
			n_bptr->icount = icount;
			assert(iln->parent);
			if (retidx != jitdata::UNUSED)
				iln->parent->varmap[iln->callerins->dst.varindex] = retidx;
		}
	}

	bm = iln->ctx->blockmap;
	for (i=0; i<iln->ctx->blockmap_index; ++i, ++bm) {
		assert(bm->iln && bm->o_block && bm->n_block);
		if (bm->iln == iln)
			inline_resolve_block_refs(&(iln->refs), bm->o_block, bm->n_block, false);
	}

#if !defined(NDEBUG)
	if (iln->refs) {
		inline_target_ref *ref;
		ref = iln->refs;
		while (ref) {
			if (!iln->depth || ref->isnumber || *(ref->ref.block) != INLINE_RETURN_REFERENCE(iln)) {
				DOLOG( printf("XXX REMAINING REF at depth %d: %p\n", iln->depth,
					   (void*)*(ref->ref.block)) );
				assert(false);
			}
			ref = ref->next;
		}
	}
#endif
}


static exception_entry * inline_exception_tables(inline_node *iln,
												 exception_entry *n_extable,
												 exception_entry **prevextable)
{
	inline_node *child;
	inline_node *scope;
	exception_entry *et;

	assert(iln);
	assert(n_extable);
	assert(prevextable);

	child = iln->children;
	if (child) {
		do {
			n_extable = inline_exception_tables(child, n_extable, prevextable);
			child = child->next;
		} while (child != iln->children);
	}

	et = iln->jd->exceptiontable;
	for (; et != NULL; et = et->down) {
		assert(et);
		MZERO(n_extable, exception_entry, 1);
		n_extable->start     = inline_map_block(iln, et->start  , iln);
		n_extable->end       = inline_map_block(iln, et->end    , iln);
		n_extable->handler   = inline_map_block(iln, et->handler, iln);
		n_extable->catchtype = et->catchtype;

		if (*prevextable) {
			(*prevextable)->down = n_extable;
		}
		*prevextable = n_extable;

		n_extable++;
	}

	if (iln->handler_monitorexit) {
		exception_entry **activehandlers;

		MZERO(n_extable, exception_entry, 1);
		n_extable->start   = iln->inlined_basicblocks;
		n_extable->end     = iln->inlined_basicblocks_cursor;
		n_extable->handler = iln->handler_monitorexit;
		n_extable->catchtype.any = NULL; /* finally */

		if (*prevextable) {
			(*prevextable)->down = n_extable;
		}
		*prevextable = n_extable;

		n_extable++;

		/* We have to protect the created handler with the same handlers */
		/* that protect the method body itself.                          */

		for (scope = iln; scope->parent != NULL; scope = scope->parent) {

			activehandlers = scope->o_handlers;
			assert(activehandlers);

			while (*activehandlers) {

				assert(scope->parent);

				MZERO(n_extable, exception_entry, 1);
				n_extable->start     = iln->handler_monitorexit;
				n_extable->end       = iln->handler_monitorexit + 1; /* XXX ok in this case? */
				n_extable->handler   = inline_map_block(scope->parent,
														(*activehandlers)->handler,
														scope->parent);
				n_extable->catchtype = (*activehandlers)->catchtype;

				if (*prevextable) {
					(*prevextable)->down = n_extable;
				}
				*prevextable = n_extable;

				n_extable++;
				activehandlers++;
			}
		}
	}

	return n_extable;
}


static void inline_locals(inline_node *iln)
{
	inline_node *child;

	assert(iln);

	iln->varmap = create_variable_map(iln);

	child = iln->children;
	if (child) {
		do {
			inline_locals(child);
			child = child->next;
		} while (child != iln->children);
	}

	if (iln->regdata->memuse > iln->ctx->resultjd->rd->memuse)
		iln->ctx->resultjd->rd->memuse = iln->regdata->memuse;
	if (iln->regdata->argintreguse > iln->ctx->resultjd->rd->argintreguse)
		iln->ctx->resultjd->rd->argintreguse = iln->regdata->argintreguse;
	if (iln->regdata->argfltreguse > iln->ctx->resultjd->rd->argfltreguse)
		iln->ctx->resultjd->rd->argfltreguse = iln->regdata->argfltreguse;
}


static void inline_interface_variables(inline_node *iln)
{
	basicblock *bptr;
	jitdata *resultjd;
	s4 i;
	varinfo *v;

	resultjd = iln->ctx->resultjd;

	resultjd->interface_map = (interface_info*) DumpMemory::allocate(sizeof(interface_info) * 5 * iln->ctx->maxinoutdepth);
	for (i=0; i<5*iln->ctx->maxinoutdepth; ++i)
		resultjd->interface_map[i].flags = jitdata::UNUSED;

	for (bptr = resultjd->basicblocks; bptr != NULL; bptr = bptr->next) {
		assert(bptr->indepth  <= iln->ctx->maxinoutdepth);
		assert(bptr->outdepth <= iln->ctx->maxinoutdepth);

		for (i=0; i<bptr->indepth; ++i) {
			v = &(resultjd->var[bptr->invars[i]]);
			v->flags |= INOUT;
			if (v->type == TYPE_RET)
				v->flags |= PREALLOC;
			else
				v->flags &= ~PREALLOC;
			v->flags &= ~INMEMORY;
			assert(bptr->invars[i] >= resultjd->localcount);

			if (resultjd->interface_map[5*i + v->type].flags == jitdata::UNUSED) {
				resultjd->interface_map[5*i + v->type].flags = v->flags;
			}
			else {
				resultjd->interface_map[5*i + v->type].flags |= v->flags;
			}
		}

		for (i=0; i<bptr->outdepth; ++i) {
			v = &(resultjd->var[bptr->outvars[i]]);
			v->flags |= INOUT;
			if (v->type == TYPE_RET)
				v->flags |= PREALLOC;
			else
				v->flags &= ~PREALLOC;
			v->flags &= ~INMEMORY;
			assert(bptr->outvars[i] >= resultjd->localcount);

			if (resultjd->interface_map[5*i + v->type].flags == jitdata::UNUSED) {
				resultjd->interface_map[5*i + v->type].flags = v->flags;
			}
			else {
				resultjd->interface_map[5*i + v->type].flags |= v->flags;
			}
		}
	}
}


static void inline_write_exception_handlers(inline_node *master, inline_node *iln)
{
	basicblock *n_bptr;
	instruction *n_ins;
	inline_node *child;
	builtintable_entry *bte;
	s4 exvar;
	s4 syncvar;
	s4 i;

	child = iln->children;
	if (child) {
		do {
			inline_write_exception_handlers(master, child);
			child = child->next;
		} while (child != iln->children);
	}

	if (iln->synchronize) {
		/* create the monitorexit handler */
		n_bptr = create_block(master, iln, iln,
							  iln->n_passthroughcount + 1);
		n_bptr->type  = basicblock::TYPE_EXH;
		n_bptr->state = basicblock::FINISHED;

		exvar = inline_new_variable(master->ctx->resultjd, TYPE_ADR, 0);
		n_bptr->invars[iln->n_passthroughcount] = exvar;

		iln->handler_monitorexit = n_bptr;

		/* ACONST / ALOAD */

		n_ins = master->inlined_iinstr_cursor++;
		if (iln->m->flags & ACC_STATIC) {
			n_ins->opc = ICMD_ACONST;
			n_ins->sx.val.c.cls = iln->m->clazz;
			n_ins->flags.bits = INS_FLAG_CLASS;
		}
		else {
			n_ins->opc = ICMD_ALOAD;
			n_ins->s1.varindex = iln->synclocal;
			assert(n_ins->s1.varindex != jitdata::UNUSED);
		}
		/* XXX could be PREALLOCed for  builtin call */
		syncvar = inline_new_variable(master->ctx->resultjd, TYPE_ADR, 0);
		n_ins->dst.varindex = syncvar;
		n_ins->line = 0;

		/* MONITOREXIT */

		bte = builtintable_get_internal(LOCK_monitor_exit);

		n_ins = master->inlined_iinstr_cursor++;
		n_ins->opc = ICMD_BUILTIN;
		n_ins->s1.argcount = 1 + iln->n_passthroughcount + 1;
		n_ins->sx.s23.s2.args = (s4*) DumpMemory::allocate(sizeof(s4) * n_ins->s1.argcount);
		n_ins->sx.s23.s2.args[0] = syncvar;
		for (i=0; i < iln->n_passthroughcount + 1; ++i) {
			n_ins->sx.s23.s2.args[1 + i] = n_bptr->invars[i];
		}
		n_ins->sx.s23.s3.bte = bte;
		n_ins->line = 0;

		/* ATHROW */

		n_ins = master->inlined_iinstr_cursor++;
		n_ins->opc = ICMD_ATHROW;
		n_ins->flags.bits = 0;
		n_ins->s1.varindex = exvar;
		n_ins->line = 0;

		/* close basic block */

		close_block(iln, iln, n_bptr, iln->n_passthroughcount);
		n_bptr->icount = 3;
	}
}


/* second pass driver *********************************************************/

static bool inline_transform(inline_node *iln, jitdata *jd)
{
	instruction *n_ins;
	basicblock *n_bb;
	basicblock *n_bptr;
	exception_entry *n_ext;
	exception_entry *prevext;
	codegendata *n_cd;
	jitdata *n_jd;
	s4 i;
#if defined(INLINE_VERIFY_RESULT)
	static int debug_verify_inlined_code = 1;
#endif
#if defined(ENABLE_INLINING_DEBUG) || !defined(NDEBUG)
	static int debug_counter = 0;
#endif

	DOLOG( dump_inline_tree(iln, 0); );

	assert(iln && jd);

	n_ins = (instruction*) DumpMemory::allocate(sizeof(instruction) * iln->cumul_instructioncount);
	MZERO(n_ins, instruction, iln->cumul_instructioncount);
	iln->inlined_iinstr = n_ins;

	n_bb = (basicblock*) DumpMemory::allocate(sizeof(basicblock) * iln->cumul_basicblockcount);
	MZERO(n_bb, basicblock, iln->cumul_basicblockcount);
	iln->inlined_basicblocks = n_bb;

	iln->ctx->blockmap = (inline_block_map*) DumpMemory::allocate(sizeof(inline_block_map) * iln->cumul_blockmapcount);

	n_jd = jit_jitdata_new(iln->m);
	n_jd->flags = jd->flags;
	n_jd->code->optlevel = jd->code->optlevel;
	iln->ctx->resultjd = n_jd;

	reg_setup(n_jd);

	/* create the local_map */

	n_jd->local_map = (s4*) DumpMemory::allocate(sizeof(s4) *  5 * iln->cumul_maxlocals);
	for (i=0; i<5*iln->cumul_maxlocals; ++i)
		n_jd->local_map[i] = jitdata::UNUSED;

	/* create / coalesce local variables */

	n_jd->varcount = 0;
	n_jd->vartop = 0;
	n_jd->var = NULL;

	inline_locals(iln);

	n_jd->localcount = n_jd->vartop;

	/* extra variables for verification (debugging) */

#if defined(INLINE_VERIFY_RESULT)
	if (debug_verify_inlined_code) {
		n_jd->vartop   += VERIFIER_EXTRA_LOCALS + VERIFIER_EXTRA_VARS + 100 /* XXX m->maxstack */;
		if (n_jd->vartop > n_jd->varcount) {
			/* XXX why? */
			n_jd->var = (varinfo*) DumpMemory::realloc(n_jd->var, sizeof(varinfo) * n_jd->varcount, sizeof(varinfo) * n_jd->vartop);
			n_jd->varcount = n_jd->vartop;
		}
	}
#endif /* defined(INLINE_VERIFY_RESULT) */

	/* write inlined code */

	inline_rewrite_method(iln);

	/* create exception handlers */

	inline_write_exception_handlers(iln, iln);

	/* write the dummy end block */

	n_bptr        = create_block(iln, iln, iln, 0);
	n_bptr->state = basicblock::UNDEF;
	n_bptr->type  = basicblock::TYPE_STD;

	/* store created code in jitdata */

	n_jd->basicblocks = iln->inlined_basicblocks;
	n_jd->instructioncount = iln->cumul_instructioncount;
	n_jd->instructions = iln->inlined_iinstr;

	/* link the basic blocks (dummy end block is not counted) */

	n_jd->basicblockcount = (iln->inlined_basicblocks_cursor - iln->inlined_basicblocks) - 1;
	for (i=0; i<n_jd->basicblockcount + 1; ++i)
		n_jd->basicblocks[i].next = &(n_jd->basicblocks[i+1]);
	if (i)
		n_jd->basicblocks[i-1].next = NULL;

	/* check basicblock numbers */

#if !defined(NDEBUG)
	jit_check_basicblock_numbers(n_jd);
#endif

	/* create the exception table */

	if (iln->cumul_exceptiontablelength) {
		exception_entry *tableend;

		n_ext = (exception_entry*) DumpMemory::allocate(sizeof(exception_entry) * iln->cumul_exceptiontablelength);
		prevext = NULL;
		tableend = inline_exception_tables(iln, n_ext, &prevext);
		assert(tableend == n_ext + iln->cumul_exceptiontablelength);
		if (prevext)
			prevext->down = NULL;

		n_jd->exceptiontablelength = iln->cumul_exceptiontablelength;
		n_jd->exceptiontable = n_ext;
	}
	else {
		n_ext = NULL;
	}

	/*******************************************************************************/

	n_cd = n_jd->cd;
	memcpy(n_cd, jd->cd, sizeof(codegendata));

	n_cd->method = NULL; /* XXX */
	n_jd->maxlocals = iln->cumul_maxlocals;
	n_jd->maxinterfaces = iln->ctx->maxinoutdepth;

	inline_post_process(n_jd);

	inline_interface_variables(iln);

	/* for debugging, verify the inlined result */

#if defined(INLINE_VERIFY_RESULT)
	if (debug_verify_inlined_code) {
		debug_verify_inlined_code = 0;
		DOLOG( printf("VERIFYING INLINED RESULT...\n"); fflush(stdout); );
		if (!typecheck(n_jd)) {
			exceptions_clear_exception();
			DOLOG( printf("XXX INLINED RESULT DID NOT PASS VERIFIER XXX\n") );
			return false;
		}
		else {
			DOLOG( printf("VERIFICATION PASSED.\n") );
		}
		debug_verify_inlined_code = 1;
	}
#endif /* defined(INLINE_VERIFY_RESULT) */

	/* we need bigger free memory stacks (XXX these should not be allocated in reg_setup) */

	n_jd->rd->freemem = (s4*) DumpMemory::allocate(sizeof(s4) * (iln->ctx->maxinoutdepth + 1000)) /* XXX max vars/block */;

#if defined(ENABLE_INLINING_DEBUG) || !defined(NDEBUG)
	if (   (n_jd->instructioncount >= opt_InlineMinSize)
		&& (n_jd->instructioncount <= opt_InlineMaxSize))
	{
	   if (debug_counter < opt_InlineCount)
#endif /* defined(ENABLE_INLINING_DEBUG) || !defined(NDEBUG) */
	   {
			/* install the inlined result */

			*jd->code = *n_jd->code;
			n_jd->code = jd->code;
			*jd = *n_jd;

			/* statistics and logging */

#if !defined(NDEBUG)
			inline_stat_roots++;

			DOLOG_SHORT(
			printf("==== %d.INLINE ==================================================================\n",
				debug_counter);
			printf("\ninline tree:\n");
			dump_inline_tree(iln, 0);
			n_jd->flags |= JITDATA_FLAG_SHOWINTERMEDIATE | JITDATA_FLAG_SHOWDISASSEMBLE;
			/* debug_dump_inlined_code(iln, n_method, n_cd, n_rd); */
			printf("-------- DONE -----------------------------------------------------------\n");
			fflush(stdout);
			);
#endif
	   }

#if defined(ENABLE_INLINING_DEBUG) || !defined(NDEBUG)
		debug_counter++;
	}
#endif
	return true;
}


/******************************************************************************/
/* FIRST PASS: build inlining tree                                            */
/******************************************************************************/


/* inline_pre_parse_heuristics *************************************************

   Perform heuristic checks whether a call site should be inlined.
   These checks are evaluated before the callee has been parsed and analysed.

   IN:
       caller...........inlining node of the caller
	   callee...........the called method
	   site.............information on the call site

   RETURN VALUE:
       true........consider for inlining
	   false.......don't inline

*******************************************************************************/

static bool inline_pre_parse_heuristics(const inline_node *caller,
										const methodinfo *callee,
										inline_site *site)
{
#if defined(INLINE_MAX_DEPTH)
	if (caller->depth >= INLINE_MAX_DEPTH)
		return false;
#endif

	return true;
}


/* inline_post_parse_heuristics ************************************************

   Perform heuristic checks whether a call site should be inlined.
   These checks are evaluated after the callee has been parsed and analysed.

   IN:
       caller...........inlining node of the caller (const)
	   callee...........the called method (const)

   RETURN VALUE:
	   true........consider for inlining
	   false.......don't inline

*******************************************************************************/

static bool inline_post_parse_heuristics(const inline_node *caller,
										 const inline_node *callee)
{
	return true;
}


/* inline_afterwards_heuristics ************************************************

   Perform heuristic checks whether a call site should be inlined.
   These checks are evaluated after the inlining plan for the callee has
   been made.

   IN:
       caller...........inlining node of the caller (const)
	   callee...........the called method (const)

   RETURN VALUE:
	   true........consider for inlining
	   false.......don't inline

*******************************************************************************/

static bool inline_afterwards_heuristics(const inline_node *caller,
										 const inline_node *callee)
{
#if defined(INLINE_MAX_ICMD_EXPANSION) || defined(INLINE_MAX_BLOCK_EXPANSION)
	inline_node *cumulator;

#if defined(INLINE_DEPTH_FIRST)
	cumulator = caller;
#else
	cumulator = caller->ctx->master;
#endif
#endif

	if (0
#if defined(INLINE_MAX_BLOCK_EXPANSION)
		|| (cumulator->cumul_basicblockcount + callee->cumul_basicblockcount
			> INLINE_MAX_BLOCK_EXPANSION*caller->ctx->master->jd->basicblockcount)
#endif
#if defined(INLINE_MAX_ICMD_EXPANSION)
		|| (cumulator->cumul_instructioncount + callee->cumul_instructioncount
			> INLINE_MAX_ICMD_EXPANSION*caller->ctx->master->jd->instructioncount)
#endif
	   )
	{
		return false;
	}

	return true;
}


/* inline_is_monomorphic *******************************************************

   Check if the given call site can be proven to be monomorphic.

   IN:
	   callee...........the called method
	   call.............the invocation instruction

   OUT:
       site->speculative.....flags whether the inlining is speculative
	                         (only defined if return value is true)

   RETURN VALUE:
       true if the call site is (currently) monomorphic,
	   false if not or unknown

*******************************************************************************/

static bool inline_is_monomorphic(const methodinfo *callee,
								  const instruction *call,
								  inline_site *site)
{
	if ((callee->flags & (ACC_STATIC | ACC_FINAL | ACC_PRIVATE)
				|| call->opc == ICMD_INVOKESPECIAL))
	{
		site->speculative = false;
		return true;
	}

	/* XXX search single implementation for abstract monomorphics */

	if ((callee->flags & (ACC_METHOD_MONOMORPHIC | ACC_METHOD_IMPLEMENTED
					| ACC_ABSTRACT))
			== (ACC_METHOD_MONOMORPHIC | ACC_METHOD_IMPLEMENTED))
	{
		if (1) {
			DOLOG( printf("SPECULATIVE INLINE: "); method_println((methodinfo*)callee); );
			site->speculative = true;

			return true;
		}
	}

	/* possibly polymorphic call site */

	return false;
}


/* inline_can_inline ***********************************************************

   Check if inlining of the given call site is possible.

   IN:
       caller...........inlining node of the caller
	   callee...........the called method
	   call.............the invocation instruction

   OUT:
       site->speculative.....flags whether the inlining is speculative
	                         (only defined if return value is true)

   RETURN VALUE:
       true if inlining is possible, false if not

*******************************************************************************/

static bool inline_can_inline(const inline_node *caller,
							  const methodinfo *callee,
							  const instruction *call,
							  inline_site *site)
{
	const inline_node *active;

	/* cannot inline native methods */

	if (callee->flags & ACC_NATIVE)
		return false;

	/* cannot inline possibly polymorphic calls */

	if (!inline_is_monomorphic(callee, call, site))
		return false;

	/* cannot inline recursive calls */

	for (active = caller; active; active = active->parent) {
		if (callee == active->m) {
			DOLOG( printf("RECURSIVE!\n") );
			return false;
		}
	}

	/* inlining is possible */

	return true;
}


/* inline_create_callee_node ***************************************************

   Create an inlining node for the given callee.

   IN:
	   caller...........inlining node of the caller (const)
	   callee...........the called method

   RETURN VALUE:
       the new inlining node

*******************************************************************************/

static inline_node * inline_create_callee_node(const inline_node *caller,
											   methodinfo *callee)
{
	inline_node *cn;              /* the callee inline_node */

	cn = (inline_node*) DumpMemory::allocate(sizeof(inline_node));
	MZERO(cn, inline_node, 1);

	cn->depth = caller->depth + 1;
	cn->ctx = caller->ctx;
	cn->m = callee;
	cn->synchronize = (callee->flags & ACC_SYNCHRONIZED);
	cn->isstatic = (callee->flags & ACC_STATIC);

	return cn;
}


/* inline_set_callee_properties ************************************************

   Set properties of the inlined call site.

   IN:
       caller...........inlining node of the caller (const)
	   cn...............the called method
	   site.............info about the call site (const)

   OUT:
       *cn..............has the properties set

*******************************************************************************/

static void inline_set_callee_properties(const inline_node *caller,
										 inline_node *cn,
										 const inline_site *site)
{
	s4           argi;
	s4           i, j;
	basicblock  *bptr;

	/* set info about the call site */

	cn->callerblock = site->bptr;
	cn->callerins = site->iptr;
	cn->callerpc = site->pc;
	cn->o_handlers = site->handlers;
	cn->n_handlercount = caller->n_handlercount + site->nhandlers;

	/* determine if we need basic block boundaries before/after */

	cn->blockbefore = false;
	cn->blockafter = false;

	if (cn->jd->branchtoentry)
		cn->blockbefore = true;

	if (cn->jd->branchtoend)
		cn->blockafter = true;

	if (cn->jd->returncount > 1)
		cn->blockafter = true;

	/* XXX make safer and reusable (maybe store last real block) */
	for (bptr = cn->jd->basicblocks; bptr && bptr->next && bptr->next->next; bptr = bptr->next)
		;

	if (cn->jd->returnblock != bptr)
		cn->blockafter = true;

	/* info about the callee */

	cn->localsoffset = caller->localsoffset + caller->m->maxlocals;
	cn->prolog_instructioncount = cn->m->parseddesc->paramcount + 2;
	cn->epilog_instructioncount = 1; /* INLINE_END */
	cn->extra_instructioncount = 0;

	/* we need a CHECKNULL for instance methods, except for <init> */

	if (!cn->isstatic && cn->m->name != utf8::init)
		cn->prolog_instructioncount += 1;

	/* deal with synchronized callees */

	if (cn->synchronize) {
		methoddesc         *md;
		builtintable_entry *bte;

		/* we need basic block boundaries because of the handler */

		cn->blockbefore = true;
		cn->blockafter = true;

		/* for synchronized static methods                 */
		/* we need an ACONST, MONITORENTER in the prolog   */
		/* and ACONST, MONITOREXIT in the epilog           */

		/* for synchronized instance methods               */
		/* we need an COPY, MONITORENTER in the prolog     */
		/* and MONITOREXIT in the epilog                   */

		if (cn->isstatic) {
			cn->prolog_instructioncount += 2;
			cn->epilog_instructioncount += 2;
		}
		else {
			cn->prolog_instructioncount += 2;
			cn->epilog_instructioncount += 1;
			cn->localsoffset += 1;
		}

		/* and exception handler */
		/* ALOAD, builtin_monitorexit, ATHROW */

		cn->extra_instructioncount += 3;

		/* exception table entries */

		cn->extra_exceptiontablelength = 1 + cn->n_handlercount;

		/* add exception handler block */

		cn->cumul_basicblockcount_root++;

		/* we must call the builtins */

		bte = builtintable_get_internal(LOCK_monitor_enter);
		md = bte->md;
		if (md->memuse > cn->regdata->memuse)
			cn->regdata->memuse = md->memuse;
		if (md->argintreguse > cn->regdata->argintreguse)
			cn->regdata->argintreguse = md->argintreguse;

		bte = builtintable_get_internal(LOCK_monitor_exit);
		md = bte->md;
		if (md->memuse > cn->regdata->memuse)
			cn->regdata->memuse = md->memuse;
		if (md->argintreguse > cn->regdata->argintreguse)
			cn->regdata->argintreguse = md->argintreguse;
	}

	/* determine pass-through variables */

	i = site->iptr->s1.argcount - cn->m->parseddesc->paramcount; /* max # of pass-though vars */

	cn->n_passthroughvars = (s4*) DumpMemory::allocate(sizeof(s4) * i);
	j = 0;
	for (argi = site->iptr->s1.argcount - 1; argi >= cn->m->parseddesc->paramcount; --argi) {
		s4 idx = site->iptr->sx.s23.s2.args[argi];
		if (idx >= caller->jd->localcount) {
			cn->n_passthroughvars[j] = idx;
			j++;
		}
		else {
			DOLOG( printf("PASSING THROUGH LOCAL VARIABLE %d\n", idx); );
		}
	}
	assert(j <= i);
	cn->n_selfpassthroughcount = j;
	cn->n_passthroughcount = caller->n_passthroughcount + cn->n_selfpassthroughcount;
}


/* inline_cumulate_counters ****************************************************

   Cumulate counters after a node has been decided to become inlined.

   IN:
       caller...........inlining node of the caller
	   callee...........inlining node of the callee (const)

   OUT:
       *caller..........gets cumulated values added

*******************************************************************************/

static void inline_cumulate_counters(inline_node *caller,
									 const inline_node *cn)
{
	caller->cumul_instructioncount += cn->prolog_instructioncount;
	caller->cumul_instructioncount += cn->epilog_instructioncount;
	caller->cumul_instructioncount += cn->extra_instructioncount;
	caller->cumul_instructioncount += cn->cumul_instructioncount - 1 /*invoke*/;

	caller->cumul_basicblockcount += cn->cumul_basicblockcount;
	caller->cumul_basicblockcount_root += cn->cumul_basicblockcount_root;
	caller->cumul_blockmapcount += cn->cumul_blockmapcount;
	caller->cumul_exceptiontablelength += cn->cumul_exceptiontablelength;
	caller->cumul_exceptiontablelength += cn->extra_exceptiontablelength;

	if (cn->cumul_maxlocals > caller->cumul_maxlocals)
		caller->cumul_maxlocals = cn->cumul_maxlocals;

	/* XXX extra block after inlined call */
	if (cn->blockafter) {
		caller->cumul_basicblockcount += 1;
		caller->cumul_blockmapcount += 1;
	}
}


/* inline_analyse_callee *******************************************************

   Analyse an inlining candidate callee.

   IN:
       caller...........inlining node of the caller
	   callee...........the called method
	   site.............info about the call site

   OUT:
       site->inlined....true if the callee has been selected for inlining

   RETURN VALUE:
       the inline node of the callee, or
       NULL if an error has occurred (don't use the inlining plan in this case)

*******************************************************************************/

static inline_node * inline_analyse_callee(inline_node *caller,
										   methodinfo *callee,
										   inline_site *site)
{
	inline_node *cn;              /* the callee inline_node */

	/* create an inline tree node */

	cn = inline_create_callee_node(caller, callee);

	/* get the intermediate representation of the callee */

	if (!inline_jit_compile(cn))
		return NULL;

	/* evaluate heuristics after parsing the callee */

	if (!inline_post_parse_heuristics(caller, cn))
		return cn;

	/* the call site will be inlined */

	site->inlined = true;

	/* set info about the call site */

	inline_set_callee_properties(caller, cn, site);

	/* insert the node into the inline tree */

	inline_insert_inline_node(caller, cn);

	/* analyse recursively */

	if (!inline_analyse_code(cn))
		return NULL;

	if (!inline_afterwards_heuristics(caller, cn)) {
#if defined(INLINE_CANCEL_ON_THRESHOLD)
		return NULL;
#else
		inline_remove_inline_node(caller, cn);
		caller->ctx->stopped = true;
		site->inlined = false;
		return cn;
#endif
	}

	/* cumulate counters */

#if defined(INLINE_DEPTH_FIRST)
	inline_cumulate_counters(caller, cn);
#endif

#if defined(INLINE_BREADTH_FIRST)
	while (caller) {
		inline_cumulate_counters(caller, cn);
		caller = caller->parent;
	}
#endif

	return cn;
}


/* inline_process_candidate ****************************************************

   Process a selected inlining candidate.

   IN:
       cand.............the candidate

   RETURN VALUE:
       true........everything ok
	   false.......an error has occurred, don't use the plan

*******************************************************************************/

static bool inline_process_candidate(inline_candidate *cand)
{
	inline_node *cn;

	cn = inline_analyse_callee(cand->caller,
							   cand->callee,
							   &(cand->site));

	if (!cn)
		return false;

	if (!cand->site.inlined)
		return true;

	/* store assumptions */

	if (cand->site.speculative)
		method_add_assumption_monomorphic(cand->callee, cand->caller->ctx->master->m);

	return true;
}


/* inline_analyse_code *********************************************************

   Analyse the intermediate code of the given inlining node.

   IN:
       iln..............the inlining node

   OUT:
       *iln.............the inlining plan

   RETURN VALUE:
       true........everything ok
	   false.......an error has occurred, don't use the plan

*******************************************************************************/

static bool inline_analyse_code(inline_node *iln)
{
	methodinfo *m;
	basicblock *bptr;
	s4 len;
	instruction *iptr;
	methodinfo *callee;
	exception_entry **handlers;
	exception_entry *ex;
	s4 nhandlers;
	s4 blockendpc;
	jitdata *mjd;
	inline_site site;

	assert(iln);

	m = iln->m;
	mjd = iln->jd;

	/* initialize cumulative counters */

	iln->cumul_maxlocals = iln->localsoffset + m->maxlocals;
	iln->cumul_exceptiontablelength += mjd->exceptiontablelength;

	/* iterate over basic blocks */

	blockendpc = 0;

	for (bptr = mjd->basicblocks; bptr; bptr = bptr->next) {

		/* count the block */
		/* ignore dummy end blocks (but count them for the blockmap) */

		iln->cumul_blockmapcount++;
		if ((bptr != mjd->basicblocks || iln->blockbefore)
				&&
			(bptr->icount > 0 || bptr->next != NULL))
			iln->cumul_basicblockcount++;

		/* skip dead code */

		if (bptr->state < basicblock::REACHED)
			continue;

		/* allocate the buffer of active exception handlers */
		/* XXX this wastes some memory, but probably it does not matter */

		handlers = (exception_entry**) DumpMemory::allocate(sizeof(exception_entry*) * (mjd->exceptiontablelength + 1));

		/* determine the active exception handlers for this block     */
		/* XXX maybe the handlers of a block should be part of our IR */
		/* XXX this should share code with the type checkers          */
		nhandlers = 0;
		for (ex = mjd->exceptiontable; ex ; ex = ex->down) {
			if ((ex->start->nr <= bptr->nr) && (ex->end->nr > bptr->nr)) {
				handlers[nhandlers++] = ex;
			}
		}
		handlers[nhandlers] = NULL;

		len = bptr->icount;
		iptr = bptr->iinstr;

		blockendpc += len;
		iln->cumul_instructioncount += len;

		/* iterate over the instructions of the block */

		for (; --len >= 0; ++iptr) {

			switch (iptr->opc) {
				case ICMD_INVOKEVIRTUAL:
				case ICMD_INVOKESPECIAL:
				case ICMD_INVOKESTATIC:
				case ICMD_INVOKEINTERFACE:

					if (!INSTRUCTION_IS_UNRESOLVED(iptr) && !iln->ctx->stopped) {
						callee = iptr->sx.s23.s3.fmiref->p.method;

						if (inline_can_inline(iln, callee, iptr, &site)) {
							site.inlined = false;
							site.bptr = bptr;
							site.iptr = iptr;
							site.pc = blockendpc - len - 1;
							site.handlers = handlers;
							site.nhandlers = nhandlers;

							if (inline_pre_parse_heuristics(iln, callee, &site)) {
#if defined(INLINE_KNAPSACK) || defined(INLINE_BREADTH_FIRST)
								inline_add_candidate(iln->ctx, iln, callee, &site);
#else
								inline_candidate cand;
								cand.caller = iln;
								cand.callee = callee;
								cand.site   = site;

								if (!inline_process_candidate(&cand))
									return false;
#endif
							}
						}
					}
					break;

				case ICMD_RETURN:
				case ICMD_IRETURN:
				case ICMD_ARETURN:
				case ICMD_LRETURN:
				case ICMD_FRETURN:
				case ICMD_DRETURN:
					/* extra ICMD_MOVE may be necessary */
					iln->cumul_instructioncount++;
					break;
				default:
					break;
			}
		}

		/* end of basic block */
	}

	return true;
}


static void inline_cumulate_counters_recursive(inline_node *iln)
{
	inline_node *child;

	child = iln->children;
	if (child) {
		do {
			inline_cumulate_counters_recursive(child);
			inline_cumulate_counters(iln, child);
			child = child->next;
		} while (child != iln->children);
	}
}


/* inline_make_inlining_plan ***************************************************

   Make an inlining plan for the given root node

   IN:
       iln..............the root node

   OUT:
       *iln.............the inlining plan

   RETURN VALUE:
       true........everything ok
	   false.......an error has occurred, don't use the plan

*******************************************************************************/

#if defined(INLINE_KNAPSACK)
static bool inline_make_inlining_plan(inline_node *iln)
{
	inline_candidate *cand;
#if defined(INLINE_COST_BUDGET)
	s4 budget = INLINE_COST_BUDGET;
#   define BUDGETMEMBER cost
#endif
#if defined(INLINE_WEIGHT_BUDGET)
	double budget = INLINE_WEIGHT_BUDGET;
#   define BUDGETMEMBER weight
#endif

	inline_analyse_code(iln);

	DOLOG( printf("candidates in "); method_println(iln->m);
		   inline_candidates_println(iln->ctx); );

	while ((cand = inline_pick_best_candidate(iln->ctx)) != NULL)
	{
		if (cand->BUDGETMEMBER <= budget) {
			DOLOG( printf("    picking: "); inline_candidate_println(cand); );

			if (!inline_process_candidate(cand))
				return false;

#if !defined(INLINE_ADD_NEGATIVE_TO_BUDGET)
			if (cand->BUDGETMEMBER > 0)
#endif
				budget -= cand->BUDGETMEMBER;
		}
	}

	inline_cumulate_counters_recursive(iln);

	return true;
}
#endif /* defined(INLINE_KNAPSACK) */


#if defined(INLINE_DEPTH_FIRST)
static bool inline_make_inlining_plan(inline_node *iln)
{
	return inline_analyse_code(iln);
}
#endif /* defined(INLINE_DEPTH_FIRST) */


#if defined(INLINE_BREADTH_FIRST)
static bool inline_make_inlining_plan(inline_node *iln)
{
	inline_candidate *cand;

	inline_analyse_code(iln);

	DOLOG( printf("candidates in "); method_println(iln->m);
		   inline_candidates_println(iln->ctx); );

	while (!iln->ctx->stopped
		   && (cand = inline_pick_best_candidate(iln->ctx)) != NULL)
	{
		DOLOG( printf("    picking: "); inline_candidate_println(cand); );

		if (!inline_process_candidate(cand))
			return false;
	}

	return true;
}
#endif /* defined(INLINE_BREADTH_FIRST) */


/* statistics *****************************************************************/

#if defined(INLINE_STATISTICS)
static void inline_gather_statistics_recursive(inline_node *iln)
{
	inline_node *child;

	inline_stat_inlined_nodes++;

	if (iln->depth > inline_stat_max_depth)
		inline_stat_max_depth++;

	child = iln->children;
	if (child) {
		do {
			inline_gather_statistics_recursive(child);
			child = child->next;
		} while (child != iln->children);
	}
}
#endif /* defined(INLINE_STATISTICS) */


#if defined(INLINE_STATISTICS)
static void inline_gather_statistics(inline_node *iln)
{
	inline_stat_roots_transformed++;

	inline_gather_statistics_recursive(iln);
}
#endif /* defined(INLINE_STATISTICS) */


/* post processing ************************************************************/

#define POSTPROCESS_SRC(varindex)  live[varindex]--
#define POSTPROCESS_DST(varindex)  live[varindex]++

#define POSTPROCESS_SRCOP(s)  POSTPROCESS_SRC(iptr->s.varindex)
#define POSTPROCESS_DSTOP(d)  POSTPROCESS_DST(iptr->d.varindex)

#define MARKSAVED(varindex)  jd->var[varindex].flags |= SAVEDVAR

#define MARK_ALL_SAVED                                               \
    do {                                                             \
        for (i=0; i<jd->vartop; ++i)                                 \
            if (live[i])                                             \
                MARKSAVED(i);                                        \
    } while (0)

static void inline_post_process(jitdata *jd)
{
	codeinfo   *code;
	basicblock *bptr;
	instruction *iptr;
	instruction *iend;
	s4 i;
	icmdtable_entry_t *icmdt;
	s4 *live;
	methoddesc *md;
	builtintable_entry *bte;

	/* Get required compiler data. */

	code = jd->code;

	/* reset the SAVEDVAR flag of all variables */

	for (i=0; i<jd->vartop; ++i)
		jd->var[i].flags &= ~SAVEDVAR;

	/* allocate the life counters */

	live = (s4*) DumpMemory::allocate(sizeof(s4) * jd->vartop);
	MZERO(live, s4, jd->vartop);

	/* iterate over all basic blocks */

	for (bptr = jd->basicblocks; bptr != NULL; bptr = bptr->next) {
		if (bptr->state < basicblock::REACHED)
			continue;

		/* make invars live */

		for (i=0; i<bptr->indepth; ++i)
			POSTPROCESS_DST(bptr->invars[i]);

		iptr = bptr->iinstr;
		iend = iptr + bptr->icount;

		for (; iptr < iend; ++iptr) {

			icmdt = &(icmd_table[iptr->opc]);

			switch (icmdt->dataflow) {
				case DF_3_TO_0:
					POSTPROCESS_SRCOP(sx.s23.s3);
				case DF_2_TO_0:
					POSTPROCESS_SRCOP(sx.s23.s2);
				case DF_1_TO_0:
					POSTPROCESS_SRCOP(s1);
				case DF_0_TO_0:
					if (icmdt->flags & ICMDTABLE_CALLS) {
						code_unflag_leafmethod(code);
						MARK_ALL_SAVED;
					}
					break;

				case DF_2_TO_1:
					POSTPROCESS_SRCOP(sx.s23.s2);
				case DF_1_TO_1:
				case DF_MOVE:
					POSTPROCESS_SRCOP(s1);
				case DF_0_TO_1:
					if (icmdt->flags & ICMDTABLE_CALLS) {
						code_unflag_leafmethod(code);
						MARK_ALL_SAVED;
					}
				case DF_COPY:
					POSTPROCESS_DSTOP(dst);
					break;

				case DF_N_TO_1:
					for (i=0; i<iptr->s1.argcount; ++i) {
						POSTPROCESS_SRC(iptr->sx.s23.s2.args[i]);
					}
					if (icmdt->flags & ICMDTABLE_CALLS) {
						code_unflag_leafmethod(code);
						MARK_ALL_SAVED;
					}
					POSTPROCESS_DSTOP(dst);
					break;

				case DF_INVOKE:
					INSTRUCTION_GET_METHODDESC(iptr, md);
		post_process_call:
					code_unflag_leafmethod(code);
					for (i=0; i<md->paramcount; ++i) {
						POSTPROCESS_SRC(iptr->sx.s23.s2.args[i]);
					}
					for (; i<iptr->s1.argcount; ++i) {
						MARKSAVED(iptr->sx.s23.s2.args[i]);
					}
					if (md->returntype.type != TYPE_VOID)
						POSTPROCESS_DSTOP(dst);
					break;

				case DF_BUILTIN:
					bte = iptr->sx.s23.s3.bte;
					md = bte->md;
					goto post_process_call;

				default:
					assert(0);
			}

		} /* end instruction loop */

		/* consume outvars */

		for (i=0; i<bptr->outdepth; ++i)
			POSTPROCESS_SRC(bptr->outvars[i]);

#if !defined(NDEBUG)
		for (i=jd->localcount; i < jd->vartop; ++i)
			assert(live[i] == 0);
#endif

	} /* end basic block loop */
}


/* inline_create_root_node *****************************************************

   Create the root node of the inlining tree.

   IN:
	   jd...............the current jitdata of the root method

   RETURN VALUE:
       the root node of the inlining tree

*******************************************************************************/

static inline_node * inline_create_root_node(jitdata *jd)
{
	inline_node *iln;

	iln = (inline_node*) DumpMemory::allocate(sizeof(inline_node));
	MZERO(iln, inline_node, 1);

	iln->m = jd->m;
	iln->jd = jd;
	iln->regdata = jd->rd;

	iln->blockbefore = true;
	iln->blockafter = true;

	iln->cumul_instructioncount = 0;
	iln->cumul_basicblockcount = 1 /* dummy end block */;

	/* create inlining context */

	iln->ctx = (inline_context*) DumpMemory::allocate(sizeof(inline_context));
	MZERO(iln->ctx, inline_context, 1);
	iln->ctx->master = iln;
	iln->ctx->next_debugnr = 1; /* XXX debug */

	return iln;
}


/******************************************************************************/
/* MAIN DRIVER FUNCTION                                                       */
/******************************************************************************/

bool inline_inline(jitdata *jd)
{
	inline_node *iln;

	DOLOG( printf("==== INLINE ==================================================================\n");
		   show_method(jd, SHOW_STACK); );

#if defined(INLINE_STATISTICS)
	inline_stat_roots++;
#endif

	iln = inline_create_root_node(jd);

	if (inline_make_inlining_plan(iln)) {

		/* add blocks to the root node */

		iln->cumul_basicblockcount += iln->cumul_basicblockcount_root;
		iln->cumul_blockmapcount   += iln->cumul_basicblockcount_root;

		DOLOG( printf("==== INLINE TRANSFORM ========================================================\n"); );

		if (iln->children)
			inline_transform(iln, jd);

#if defined(INLINE_STATISTICS)
		inline_gather_statistics(iln);
#endif
	}

	DOLOG( printf("-------- DONE -----------------------------------------------------------\n");
		   fflush(stdout); );

	return true;
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
