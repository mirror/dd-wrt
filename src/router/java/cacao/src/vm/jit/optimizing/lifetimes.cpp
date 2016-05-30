/* src/vm/jit/optimizing/lifetimes.cpp - lifetime anaylsis

   Copyright (C) 2005-2013
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
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.

*/


#include "config.h"

#include <stdio.h>
#include <stdlib.h>

#include "mm/dumpmemory.hpp"

#include "toolbox/bitvector.hpp"
#include "toolbox/worklist.hpp"

#include "vm/exceptions.hpp"
#include "vm/descriptor.hpp"
#include "vm/resolve.hpp"
#include "vm/string.hpp"

#include "vm/jit/builtin.hpp"
#include "vm/jit/jit.hpp"
#include "vm/jit/optimizing/dominators.hpp"
#include "vm/jit/optimizing/graph.hpp"
#include "vm/jit/optimizing/lsra.hpp"
#include "vm/jit/optimizing/ssa.hpp"
#include "vm/jit/optimizing/lifetimes.hpp"

#ifdef LT_DEBUG_VERBOSE
#include "vm/options.hpp"
#endif

#include <time.h>
#include <errno.h>

/* function prototypes */
void _lt_scanlifetimes(jitdata *jd, graphdata *gd, basicblock *bptr, int);
void lt_usage(jitdata *, s4 , int , int , int );


void lt_lifeoutatblock(lsradata *ls, graphdata *gd, int *M, int b_index,
					   struct lifetime *lt, worklist *W);
void lt_lifeatstatement(lsradata *ls, graphdata *gd, int b_index,
						int iindex, struct lifetime *lt, bool life_in,
						worklist *W);
#if 0
void lt_lifeinatstatement(lsradata *ls, graphdata *gd, int *M, int b_index,
					   int iindex, struct lifetime *lt);
void lt_lifeoutatstatement(lsradata *ls, graphdata *gd, int *M, int b_index,
						int iindex, struct lifetime *lt);
#endif
#ifdef USAGE_COUNT
void lt_get_nesting(lsradata *ls, graphdata *gd, dominatordata *dd);
#endif

void lt_set_use_site(struct lifetime *lt, struct site *use_site) {
}

struct site *lt_get_first_use_site(struct lifetime *lt, lt_iterator *iter) {
	return ((*iter) = lt->use);
}

struct site *lt_get_next_site(lt_iterator *iter) {
	if ((*iter) == NULL)
		return NULL;
	else
		return ((*iter) = (*iter)->next);
}

struct site *lt_get_first_def_site(struct lifetime *lt, lt_iterator *iter) {
	return ((*iter) = lt->def);
}

bool lt_v_is_defined_at_s(lsradata *ls, int b_index, int iindex,
					   struct lifetime * lt) {
	struct site *def_site;
	bool is_defined_at_s;

	def_site = lt->def;
	is_defined_at_s = ((def_site->b_index == b_index)
					   && (def_site->iindex == iindex));
	return is_defined_at_s;
}

/****************************************************************************
Get Def & Use Sites
****************************************************************************/
void lt_scanlifetimes(jitdata *jd, graphdata *gd, dominatordata *dd) {
	int i, l, p;
	s4 t;
	methodinfo *m;
	lsradata *ls;
	methoddesc *md;

	ls = jd->ls;
	m  = jd->m;
	md = m->parseddesc;

	graph_DFS(ls, gd);

#ifdef USAGE_COUNT
	lt_get_nesting(ls, gd, dd);
#endif

#if defined(LT_DEBUG_VERBOSE)
	if (compileverbose) {
		printf("Sorted: ");
		for (i=0; i < ls->basicblockcount; i++) {
			l = ls->sorted[i];
			if (l != -1)
				l = ls->basicblocks[l]->nr;
			printf("%3i(%3i) ", ls->sorted[i], l);
		}
		printf("\n");
		printf("Sorted_rev: ");
		for (i=0; i < ls->basicblockcount; i++)
			printf("%3i ", ls->sorted_rev[i]);
		printf("\n");
	}
#endif

	for(i = ls->basicblockcount - 1; i>= 0; i--)
		if (ls->sorted[i] != -1)
			_lt_scanlifetimes(jd, gd, ls->basicblocks[ls->sorted[i]],
							  ls->sorted[i]);

	/* Parameter initialisiation for locals [0 .. paramcount[            */
	/* -> add local var write access at (bb=0,iindex=0)                 */

 	for (p = 0, l = 0; p < md->paramcount; p++) {
 		t = md->paramtypes[p].type;
		i = jd->local_map[l * 5 + t];
 		l++;
 		if (IS_2_WORD_TYPE(t))    /* increment local counter for 2 word types */
 			l++;
		if (i == jitdata::UNUSED)
			continue;
		i = ls->var_0[i];
/* 		_LT_ASSERT( i < jd->cd->maxlocals); */
/* 			printf("param %3i -> L %3i/%3i\n",p,i,t); */
		_LT_ASSERT(t == VAR(i)->type);

		/* Param to Local init happens before normal Code */

#ifdef LT_DEBUG_VERBOSE
		if (compileverbose)
			printf(" ok\n");
#endif
		lt_usage(jd, i, 0, 0, LT_DEF);
	}  /* end for */
}


bool lt_is_simple_lt(struct lifetime *lt) {
	lt_iterator i_def, i_use;
	struct site *def, *use;
	bool all_in_same_block;


	def = lt_get_first_def_site(lt, &i_def);
	use = lt_get_first_use_site(lt, &i_use);
	all_in_same_block = true;
	for (; (all_in_same_block && (use != NULL));
		 use = lt_get_next_site(&i_use)) {
		all_in_same_block =
			(use->iindex >= 0) && (use->b_index == def->b_index);
	}
	return all_in_same_block;
}

void lt_is_live(lsradata *ls, struct lifetime *lt, int b_index, int iindex) {
	int bb_sorted;

	bb_sorted = ls->sorted_rev[b_index];

	if ((lt->bb_last_use < bb_sorted) ||
		((lt->bb_last_use == bb_sorted) && (lt->i_last_use < iindex))) {
		lt->bb_last_use = bb_sorted;
		lt->i_last_use  = iindex;
	}
	if ((lt->bb_first_def > bb_sorted) ||
		((lt->bb_first_def == bb_sorted) && (lt->i_first_def > iindex))) {
		lt->bb_first_def = bb_sorted;
		lt->i_first_def  = iindex;
	}
}

void lt_set_simple_use(lsradata *ls, struct lifetime *lt) {
	lt_iterator i_use;
	struct site *use;

	/* SAVEDVAR is nowhere set!!!! */

	/* Def is first use */
/* 	lt->bb_first_def = ls->sorted_rev[lt->def->b_index]; */
/* 	lt->i_first_def = lt->def->iindex; */

	lt_is_live(ls, lt, lt->def->b_index, lt->def->iindex);

	/* get last use */
	use = lt_get_first_use_site(lt, &i_use);
/* 	lt->bb_last_use = ls->sorted_rev[use->b_index]; */
/* 	lt->i_last_use = use->iindex; */
	for (;  (use != NULL); use = lt_get_next_site(&i_use))
		lt_is_live(ls, lt, use->b_index, use->iindex);
/* 		if (use->iindex > lt->i_last_use) */
/* 			lt->i_last_use = use->iindex; */
}

void lt_lifeness_analysis(jitdata *jd, graphdata *gd) {
	int *M;      /* bit_vecor of visited blocks */
	int *use;    /* bit_vecor of blocks with use sites visited */
	worklist *W; /* Worklist of Basic Blocks, where lt is life-out */

	struct site *use_site, *u_site;
	lt_iterator iter, iter1;
	graphiterator pred_iter;

	int lt_index, i, pred, iindex, iindex1, b_index;
	struct lifetime *lt;
	int *phi;
/* #define MEASURE_RT */
#ifdef MEASURE_RT
	struct timespec time_start,time_end;
#endif

	lsradata *ls;
	methodinfo *m;

	ls = jd->ls;
	m  = jd->m;


#ifdef MEASURE_RT
	if (clock_gettime(CLOCK_THREAD_CPUTIME_ID,&(time_start)) != 0) {
		fprintf(stderr,"could not get time: %s\n",strerror(errno));
		abort();
	}
#endif

	M   = bv_new(ls->basicblockcount);
	use = bv_new(ls->basicblockcount);
	W   = wl_new(ls->basicblockcount);

#ifdef LT_DEBUG_VERBOSE
	if (compileverbose)
	printf("LT_ANALYSE: \n");
#endif
	for(lt_index = 0; lt_index < ls->lifetimecount; lt_index++) {
		lt = &(ls->lifetime[lt_index]);
		if (lt->type == -1)
			continue;
#ifdef LT_DEBUG_VERBOSE
	if (compileverbose)
		printf("LT: %3i:", lt_index);
#endif

		lt->savedvar = 0;

		_LT_ASSERT(lt->def != NULL);
		_LT_ASSERT(lt->def->next == NULL); /* SSA! */
/* 		_LT_ASSERT(lt->use != NULL); */

		lt->bb_last_use = -1;
		lt->bb_first_def = ls->basicblockcount;

		bv_reset(M, ls->basicblockcount);
		bv_reset(use, ls->basicblockcount);
		wl_reset(W, ls->basicblockcount);

		use_site = lt_get_first_use_site(lt, &iter);

		/* Make unused Vars life at their Def Site */

		if (use_site == NULL) {
			lt_is_live(ls, lt, lt->def->b_index, lt->def->iindex);
			if (lt->def->iindex < 0) {

				/* def only in phi function */

				lt_is_live(ls, lt, lt->def->b_index, 0);
			}
		}
		for (;use_site != NULL; use_site = lt_get_next_site(&iter)) {
			iindex  = use_site->iindex;
			if ((lt->def->b_index == use_site->b_index) &&
				(iindex < 0) &&
				(iindex <= lt->def->iindex)) {

/* 				bv_set_bit(use, use_site->b_index); */
				/* do normal analysis */
				/* there is a use in a phi function before def site */

			}
			else if (bv_get_bit(use, use_site->b_index)) {
				continue;
			}
			else {
				bv_set_bit(use, use_site->b_index);

				/* use sites of this basic block not visited till now */
				/* get use site of this bb with highest iindex lower than */
				/* def site */

				iindex1 = -1;
				u_site = use_site;
				for(iter1= iter; u_site != NULL;
					u_site = lt_get_next_site(&iter1)) {
					if ((u_site->b_index == use_site->b_index) &&
						(lt->def->b_index == use_site->b_index) &&
						(u_site->iindex >= 0) &&
						(u_site->iindex < lt->def->iindex) &&
						(u_site->iindex > iindex1)) {
						iindex1 = u_site->iindex;
					} else {
						if ((u_site->b_index == use_site->b_index) &&
							(u_site->iindex > iindex))
							iindex = u_site->iindex;
					}
				}
				if (iindex1 != -1)
					iindex = iindex1;
			}

#ifdef LT_DEBUG_VERBOSE
	if (compileverbose)
			printf("(%3i,%3i)", use_site->b_index, iindex);
#endif

			if (iindex < 0) {

				/* use in phi function */
				/* ls->phi[use_site->b_index][-use_site->iindex-1]*/

				lt_is_live(ls, lt, use_site->b_index, iindex);

				phi = ls->phi[use_site->b_index][-iindex-1];
				_LT_ASSERT(phi != NULL);

					pred = graph_get_first_predecessor(gd, use_site->b_index,
													   &pred_iter);
					for(i = 1; (pred != -1); i++,pred =
							graph_get_next(&pred_iter))
						if (lt->v_index == phi[i]) {

							/* Add "Life out Basic Blocks to Worklist */

							wl_add(W, pred);
						}
			}
			else /* lt is live-in at this statement */
				lt_lifeatstatement(ls, gd, use_site->b_index,
								   iindex, lt, true, W);
		} /* for (;use_site != NULL; use_site = lt_get_next_site(&iter)) */

		/* process Worklist */

		while (!wl_is_empty(W)) {
			b_index = wl_get(W);
			lt_lifeoutatblock(ls, gd, M, b_index, lt, W);
		}


#ifdef LT_DEBUG_VERBOSE
		if (compileverbose)
			printf("\n");
#endif

	} /* for(lt_index = 0; lt_index < ls->lifetimecount; lt_index++) */

#ifdef MEASURE_RT
	if (clock_gettime(CLOCK_THREAD_CPUTIME_ID,&(time_end)) != 0) {
		fprintf(stderr,"could not get time: %s\n",strerror(errno));
		abort();
	}

	{
		long diff;
		time_t atime;

		diff = (time_end.tv_nsec - time_start.tv_nsec) / 1000;
		atime = time_start.tv_sec;
		while (atime < time_end.tv_sec) {
			atime++;
			diff += 1000000;
		}
		printf("%8li %s.%s.%s\n", diff,
			UTF_TEXT(m->clazz->name),
			UTF_TEXT(m->name),
			UTF_TEXT(m->descriptor));
	}
#endif
}

/*******************************************************************************
lt_lifeatstatement


IN:     lsradata *ls    pointer to worklist created with wl_new
        graphdata *gd
        int b_index     Basic block index of instruction
        int iindex      index of instruction in Basic Block
        struct lifetime *lt  Pointer to lifetime structure
        bool life_in    TRUE  lifetime lt is life 'into' that instruction
                        FALSE lifetime lt is life 'out' of that instruction

IN/OUT: worklist *W     Worklist of Basic Blocks, where lt is life-out
*******************************************************************************/
void lt_lifeatstatement(lsradata *ls, graphdata *gd, int b_index,
						int iindex, struct lifetime *lt, bool life_in,
						worklist *W) {

	int prev_iindex; /* Statement before iindex */
	int pred;
	graphiterator pred_iter;
	instruction *iptr;

	while (true) {
		if (!life_in) {
#ifdef LT_DEBUG_VERBOSE
			if ((compileverbose) && (iindex >= 0))
				printf("LO@ST: vi %3i bi %3i ii %3i\n",
					   lt->v_index, b_index, iindex);
#endif

			/* lt->v_index is life-out at statement at (b_index,iindex) */

			/* Once a interference graph is needed, add here an edge (v,w) */
			/* to the ig, for each variable w defined at this instruction  */
			/* except v=lt->v_index */

			if (!lt_v_is_defined_at_s(ls, b_index, iindex, lt)) {

				/* v is life in at out of statement -> check if the SAVEDVAR */
				/* flag is needed to be set */

				if ((iindex >= 0) && (b_index != 0)) {

					/* real ICMD, no phi-function, no param initialisation */

					_LT_ASSERT(ls->basicblocks[b_index]->iinstr != NULL);

					iptr = ls->basicblocks[b_index]->iinstr + iindex;
					if (icmd_table[iptr->opc].flags & ICMDTABLE_CALLS)
						lt->savedvar = SAVEDVAR;
				}

				/* lt stays life-in at statement */

				life_in = true;
			} else {

				/* print LO verbose message only for phi functions, which */
				/* define this var */

#ifdef LT_DEBUG_VERBOSE
				if ((compileverbose) && (iindex < 0))
					printf("LO@ST: vi %3i bi %3i ii %3i\n",
						   lt->v_index, b_index, iindex);
				if ((compileverbose))
					printf("--> definition\n");
#endif

				lt_is_live(ls, lt, b_index, iindex);

				/* Stop - lt is defined and not life before this instruction */

				break;
			}
		}

		if (life_in) {

			/* lt->v_index is live-in at statement (b_index,iindex) */

#ifdef LT_DEBUG_VERBOSE
			if ((compileverbose) && (iindex >= 0))
				printf("LI@ST: vi %3i bi %3i ii %3i\n",
					   lt->v_index, b_index, iindex);
#endif

			lt_is_live(ls, lt, b_index, iindex);


			if (iindex == -ls->ssavarcount-1) {

#ifdef LT_DEBUG_VERBOSE
				if ((compileverbose))
					printf("LI@ST: vi %3i bi %3i ii %3i\n",
						   lt->v_index, b_index, iindex);
#endif
				/* iindex is the first statement of b_index */
				/* Statements -ls->ssavarcounts-1 .. -1 are possible phi functions*/
				/* lt->v_index is live-in at b_index */

				pred = graph_get_first_predecessor(gd, b_index, &pred_iter);

				/* Add "Life out Basic Blocks to Worklist */

				for(; pred != -1; pred = graph_get_next(&pred_iter))
					wl_add(W, pred);

				/* Stop here - beginning of Basic Block reached */

				break;
			} else {

				prev_iindex = iindex - 1;
				if (prev_iindex < 0)

					/* look through phi functions */

					for(; prev_iindex > -ls->ssavarcount-1; prev_iindex--)
						if (ls->phi[b_index][-prev_iindex-1] != NULL)
							break;

				/* lt is live out at instruction prev_iindex */

				iindex = prev_iindex;
				life_in = false;
			}
		}
	}
}


void lt_lifeoutatblock(lsradata *ls, graphdata *gd, int *M, int b_index,
					   struct lifetime *lt, worklist *W) {

#if defined(LT_DEBUG_VERBOSE)
	if (compileverbose) {
		printf("V %3i LO at BB %3i\n",lt->v_index, b_index);
	}
#endif

	/* lt->v_index is life out of Block b_index */
	if (!bv_get_bit(M, b_index)) { /* BB b_index not visited till now */
		bv_set_bit(M, b_index);

		/* lt->v_index is life out of last Statement of b_index */

		if (b_index != 0) {
			int i;
			i = ls->basicblocks[b_index]->icount - 1;
			for (;((i>0) && ((ls->basicblocks[b_index]->iinstr+i)->opc == ICMD_NOP));
				 i--);
			lt_lifeatstatement(ls, gd, b_index, i, lt, false, W);
		}
		else
			lt_lifeatstatement(ls, gd, b_index, 0, lt, false, W);
	}
}

void lt_move_use_sites(struct lifetime *from, struct lifetime *to) {
	struct site *s;

#if 0
	/* not anymore true for copy propagated lifetimes */
	_LT_ASSERT(from->use != NULL);
#endif
	if (from->use == NULL)
		return;
	for(s = from->use; s->next != NULL; s = s->next);

	s->next = to->use;
	to->use = from->use;
	from->use = NULL;
}

void lt_add_use_site(struct lifetime *lt, int block, int iindex) {
	struct site *n;

	n = DNEW(struct site);
	n->b_index = block;
	n->iindex = iindex;
	n->next = lt->use;
	lt->use = n;

	/* CFG is analysed from the end to the start -> so first found use site */
	/* is the last use of the Local Var */

	if (lt->last_use == NULL)
		lt->last_use = n;
}

void lt_remove_use_site(struct lifetime *lt, int block, int iindex) {
	struct site *n;

	/* check lt->use itself */

	if ((lt->use->b_index == block) && (lt->use->iindex == iindex)) {
		/* found */
		lt->use = lt->use->next;
	} else {

		/* look through list */

		for (n = lt->use; (n->next != NULL) && ((n->next->b_index != block) ||
									(n->next->iindex != iindex)); n = n->next);

		/* assert, that lt was found */

		_LT_ASSERT(n->next != NULL);
		_LT_ASSERT(n->next->b_index == block);
		_LT_ASSERT(n->next->iindex == iindex);

		n->next = n->next->next;
	}
}

void lt_add_def_site(struct lifetime *lt, int block, int iindex) {
	struct site *n;

	/* SSA <-> only one definition per lifetime! */

	_LT_ASSERT(lt->def == NULL);
	n = DNEW(struct site);
	n->b_index = block;
	n->iindex = iindex;
	n->next = NULL;
	lt->def = n;
}

void lt_usage(jitdata *jd,s4 v_index, int block, int instr,
					  int store)
{
	struct lifetime *n;
	lsradata *ls;

	ls = jd->ls;

	n = ls->lifetime + v_index;

	if (n->type == -1) { /* new local lifetime */
		_LT_ASSERT(0);
		n->v_index=v_index;
		n->type=VAR(v_index)->type;
		/* TODO: check!!!!  */
		/* All var are SAVEDVARS or this gets reset afterwards???? */
		n->savedvar = SAVEDVAR;
		n->flags = 0;
		n->usagecount = 0;

		n->bb_last_use = -1;
		n->bb_first_def = -1;

		n->use = NULL;
		n->def = NULL;
		n->last_use = NULL;
	}
 	_LT_ASSERT(VAR(v_index)->type == n->type);

	/* add access at (block, instr) to instruction list */
	/* remember last USE, so only write, if USE Field is undefined (==-1)   */
	/* count store as use, too -> defined and not used vars would overwrite */
	/* other vars */

	if (store == LT_USE) {
#ifdef USAGE_COUNT
		n->usagecount += ls->nesting[block];
#endif
		lt_add_use_site(n, block, instr);
	}
	if (store == LT_DEF) {
		lt_add_def_site(n, block, instr);
	}
}

/***************************************************************************
use sites: dead code elemination, LifenessAnalysis
def sites: dead code elemination
***************************************************************************/
void _lt_scanlifetimes(jitdata *jd, graphdata *gd, basicblock *bptr,
					   int b_index)
{
/* 	methodinfo         *lm; */
	builtintable_entry *bte;
	methoddesc         *md;
	int i, j, t, v;
	int iindex/*, b_index*/;
	instruction *iptr;
	s4 *argp;

	lsradata *ls;

	ls = jd->ls;

#ifdef LT_DEBUG_VERBOSE
	if (compileverbose)
		printf("_lt_scanlifetimes: BB %3i flags %3i\n", b_index, bptr->state);
#endif

	if (bptr->state >= basicblock::REACHED) {

/* 		b_index = bptr->nr; */

		/* get instruction count for BB */

		iindex = bptr->icount - 1;

		/* regard not setup new BB with maybe just in and outstack */

		if (iindex < 0)
			iindex = 0;

		/* Regard phi_functions (Definition of target, Use of source) */

		for(i = 0; i < ls->ssavarcount; i++) {
			if (ls->phi[b_index][i] != NULL) {
				/* Phi Function for var i at b_index exists */
				v = ls->phi[b_index][i][0];
				_LT_ASSERT( v != ls->varcount_with_indices);
				t = VAR(i)->type;

				/* Add definition of target add - phi index -1*/
#ifdef LT_DEBUG_VERBOSE
				if (compileverbose)
					printf("_lt_scanlifetimes: phi_def: v: %3i\n i: %3i\n",
						   v, -i-1);
#endif
				lt_usage(jd, v, b_index, -i-1, LT_DEF);

				/* Add Use of sources */

				for (j = 1; j <= graph_get_num_predecessor(gd, b_index); j++) {
					if (ls->phi[b_index][i][j] != ls->varcount_with_indices)
						if (ls->phi[b_index][i][j] != jitdata::UNUSED)
							lt_usage(jd, ls->phi[b_index][i][j], b_index,
									 -i-1, LT_USE);
				}
			}
		}

		if (bptr->iinstr != NULL) {
			/* set iptr to last instruction of BB */
			iptr = bptr->iinstr + iindex;
		} else
			iindex = -1;

		for (;iindex >= 0; iindex--, iptr--)  {
			i = jitdata::UNUSED;
			v = jitdata::UNUSED;

			if (icmd_table[iptr->opc].dataflow >= DF_DST_BASE)
				v = iptr->dst.varindex;

			/* check for use (s1, s2, s3 or special (argp) ) */
			/* and definitions (dst) */
			switch(icmd_table[iptr->opc].dataflow) {
			case DF_3_TO_0:
			case DF_3_TO_1: /* icmd has s1, s2 and s3 */
				lt_usage(jd, iptr->sx.s23.s3.varindex, b_index, iindex, LT_USE);

				/* now "fall through" for handling of s2 and s1 */

			case DF_2_TO_0:
			case DF_2_TO_1: /* icmd has s1 and s2 */
				lt_usage(jd, iptr->sx.s23.s2.varindex, b_index, iindex, LT_USE);

				/* now "fall through" for handling of s1 */

			case DF_1_TO_0:
			case DF_1_TO_1:
			case DF_MOVE:
			case DF_COPY: /* icmd has s1 */
				lt_usage(jd, iptr->s1.varindex, b_index, iindex, LT_USE);
				break;

			case DF_INVOKE:
				INSTRUCTION_GET_METHODDESC(iptr,md);
				i = md->paramcount;
				if (md->returntype.type == TYPE_VOID)
					v = jitdata::UNUSED;
				break;

			case DF_BUILTIN:
				bte = iptr->sx.s23.s3.bte;
				md = bte->md;
				i = md->paramcount;
				if (md->returntype.type == TYPE_VOID)
					v = jitdata::UNUSED;
				break;

			case DF_N_TO_1:
				i = iptr->s1.argcount;
				break;

			}

			if (i != jitdata::UNUSED) {
				argp = iptr->sx.s23.s2.args;
				while (--i >= 0) {
					lt_usage(jd, *argp, b_index, iindex, LT_USE);
					argp++;
				}
			}

			if (v != jitdata::UNUSED) {
				lt_usage(jd, v, b_index, iindex, LT_DEF);
			}
		} /* for (;iindex >= 0; iindex--, iptr--) */
	} /* if (bptr->state >= basicblock::REACHED) */
} /* scan_lifetimes */


#ifdef USAGE_COUNT
/*******************************************************************************
true, if i dominates j
*******************************************************************************/
bool dominates(dominatordata *dd, int i, int j) {
	bool dominates = false;

	while(!dominates && (dd->idom[j] != -1)) {
		dominates = (i == dd->idom[j]);
		j = dd->idom[j];
	}
	return dominates;
}

/*******************************************************************************
lt_get_nesting

Look for loops in the CFG and set the nesting depth of all Basicblocks in
gd->nesting:

The Loop Header BB h is an element of DF[n] for all Basicblocks n of this loop
So Look through all x element of DF[n] for a backedge n->x. If this
exists, increment nesting for all n with x in DF[n]
*******************************************************************************/
void lt_get_nesting(lsradata *ls, graphdata *gd, dominatordata *dd) {
	int i, j, lh;
	bitvector loop_header;
	worklist *loop, *loop1;

	int succ;
	graphiterator iter;

	int num_loops;

	int *loop_parent;
	int lh_p;

	/* init nesting to 1 and get loop_headers */
	ls->nesting = DMNEW(long, ls->basicblockcount);
	loop_header = bv_new(ls->basicblockcount);
	loop = wl_new(ls->basicblockcount);
	num_loops = 0;
	for(i = 0; i < ls->basicblockcount; i++) {
		ls->nesting[i] = 1;

		for(succ = graph_get_first_successor(gd, i, &iter); succ != -1;
			succ = graph_get_next(&iter)) {
			for (j = 0; j < dd->num_DF[i]; j++) {
				if (succ == dd->DF[i][j]) {
					/* There is an edge from i to DF[i][j] */

					/* look if DF[i][j] dominates i -> backedge */
					if (dominates(dd, dd->DF[i][j], i)) {
						/* this edge is a backedge */
						/* -> DF[i][j] is a loop header */
						_LT_CHECK_BOUNDS(dd->DF[i][j], 0, ls->basicblockcount);
						if (!bv_get_bit(loop_header, dd->DF[i][j])) {
							/* new loop_header found */
							num_loops++;
							bv_set_bit(loop_header, dd->DF[i][j]);
							ls->nesting[dd->DF[i][j]] = 10;
						}
						wl_add(loop, dd->DF[i][j]);
					}
				}
			}
		}
	}

	loop_parent = DMNEW(int , ls->basicblockcount);
	loop1 = wl_new(ls->basicblockcount);

	/* look for direct parents of nested loopheaders */
	/* (DF[loop_header[i]] has the element loop_header[j] with i != j */
	/* TODO: BULLSHIT:unfortunately not such an easy condition ;( */
	while(!wl_is_empty(loop)) {
		lh = wl_get(loop);
		wl_add(loop1, lh);

		loop_parent[lh] = -1;

		for (j = 0; j < dd->num_DF[lh]; j++) {
			_LT_CHECK_BOUNDS(dd->DF[lh][j], 0, ls->basicblockcount);
			if (lh != dd->DF[lh][j]) {
				if (bv_get_bit(loop_header, dd->DF[lh][j])) {
#ifdef LT_DEBUG_VERBOSE
					if (compileverbose)
						if (loop_parent[lh] != -1)
							printf("Warning: LoopHeader has more than one parent\n");
#endif
/* 					_LT_ASSERT( loop_parent[lh] == -1); */
					loop_parent[lh] = dd->DF[lh][j];
				}
			}
		}
	}

	/* create nesting for loopheaders */
	while(!wl_is_empty(loop1)) {
		lh = wl_get(loop1);
		for (lh_p = lh; lh_p != -1; lh_p = loop_parent[lh_p]) {
			ls->nesting[lh] *= 10;
		}
	}


	/* copy loopheader nesting to loop body */
	for(i = 0; i < ls->basicblockcount; i++) {
		if (!bv_get_bit(loop_header, i)) {
			/* Do not touch the nesting of a loopheader itself */
			for(j = 0; j < dd->num_DF[i]; j++) {
				_LT_CHECK_BOUNDS(dd->DF[i][j], 0, ls->basicblockcount);
				if (bv_get_bit(loop_header, dd->DF[i][j])) {
					/* DF[i][j] is a loop header -> copy nesting for i */
#ifdef LT_DEBUG_VERBOSE
					if (compileverbose)
						if (ls->nesting[i] != 1)
							printf("Warning: More than one loopheader for one BB\n");
/* 					_LT_ASSERT(ls->nesting[i] == 1); */
#endif
					ls->nesting[i] = ls->nesting[dd->DF[i][j]];
				}
			}
		}
	}

#ifdef LT_DEBUG_VERBOSE
	if (compileverbose) {
		printf("Num Loops: %3i\n",num_loops);
		for(i = 0; i < ls->basicblockcount; i++)
			printf("(BB%3i->N%3li) ",i, ls->nesting[i]);
		printf("\n");
	}
#endif
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
