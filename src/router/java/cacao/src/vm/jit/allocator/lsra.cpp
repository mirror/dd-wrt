/* src/vm/jit/allocator/lsra.cpp - linear scan register allocator

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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>

#include "arch.hpp"
#include "md-abi.hpp"

#include "vm/types.hpp"

#include "mm/memory.hpp"

#include "toolbox/logging.hpp"

#include "vm/jit/builtin.hpp"
#include "vm/exceptions.hpp"
#include "vm/resolve.hpp"
#include "vm/options.hpp"
#include "vm/statistics.hpp"

#include "vm/jit/abi.hpp"
#include "vm/jit/reg.hpp"

#include "vm/jit/allocator/liveness.hpp"
#include "vm/jit/allocator/lsra.hpp"

STAT_DECLARE_VAR(int,count_locals_conflicts,0)

#ifdef USAGE_COUNT
extern char **prof_m_names;
extern u4   **prof_bb_freq;
extern int  prof_size;
#endif

/* function prototypes */
void lsra_init(jitdata *);
void lsra_setup(jitdata *);
void lsra_main(jitdata *);

void lsra_reg_setup(jitdata *, struct lsra_register *, struct lsra_register * );
void lsra_calc_lifetime_length(jitdata *);
void _lsra_main( jitdata *, int *, int, struct lsra_register *, int *);
void lsra_expire_old_intervalls(jitdata *, struct lifetime *,
								struct lsra_register *);
void spill_at_intervall(jitdata *, struct lifetime *);
void lsra_add_active(struct lifetime *, struct lifetime **, int *);
void _lsra_expire_old_intervalls(jitdata *, struct lifetime *,
								 struct lsra_register *, struct lifetime **,
								 int *);
void _spill_at_intervall(struct lifetime *, struct lifetime **, int *);

void lsra_alloc(jitdata *, int *, int, int *);
int lsra_getmem(struct lifetime *, struct freemem *, int *);
struct freemem *lsra_getnewmem(int *);
void lsra_setflags(int *, int);

#ifdef LSRA_DEBUG_VERBOSE
void lsra_dump_stack(stackelement_t* );
void print_lifetimes(jitdata *, int *, int);
#endif

#if !defined(LV)
void lsra_scan_registers_canditates(jitdata *, int);
void lsra_join_lifetimes(jitdata *, int);

void _lsra_new_stack( lsradata *, stackelement_t* , int , int, int);
void _lsra_from_stack(lsradata *, stackelement_t* , int , int, int);
void lsra_add_ss(struct lifetime *, stackelement_t* );
void lsra_usage_local(lsradata *, s4 , int , int , int , int );
#endif



bool lsra(jitdata *jd)
{
#if defined(ENABLE_STATISTICS)
	lsradata *ls;
	int locals_start;
	int i,j;
#endif
#if defined(LSRA_DEBUG_CHECK)
	methodinfo   *m;
	int      b_index;
	stackelement_t* in,out;
	int      ind, outd;
#endif

#if defined(LSRA_DEBUG_CHECK)
	m  = jd->m;
	b_index = 0;
	while (b_index < m->basicblockcount ) {

		if (m->basicblocks[b_index].flags >= basicblock::REACHED) {

			in=m->basicblocks[b_index].instack;
			ind=m->basicblocks[b_index].indepth;
			for (;ind != 0;in=in->prev, ind--) {
				                        /* ARGVAR or LOCALVAR in instack is ok*/
#if 0
				if (in->varkind == ARGVAR) printf("ARGVAR in instack: \n");
				if (in->varkind == LOCALVAR) printf("LOCALVAR in instack\n");
#endif
			}
			out=m->basicblocks[b_index].outstack;
			outd=m->basicblocks[b_index].outdepth;
			for (;outd != 0;out=out->prev, outd--) {
				if (out->varkind == ARGVAR)
					{ log_text("ARGVAR in outstack\n"); assert(0); }
				if (out->varkind == LOCALVAR)
					{ log_text("LOCALVAR in outstack\n"); assert(0); }
			}
		}
			b_index++;
	}
#endif

	jd->ls = DNEW(lsradata);
	lsra_init(jd);
	lsra_setup(jd);

#if defined(ENABLE_STATISTICS)
	/* find conflicts between locals for statistics */
	ls = jd->ls;
	/* local Variable Lifetimes are at the end of the lifetime array and  */
	/* have v_index >= 0 */
	for (locals_start = ls->lifetimecount-1; (locals_start >=0) &&
		(ls->lifetime[ls->lt_used[locals_start]].v_index >= 0);
		 locals_start--);
	for (i=locals_start + 1; i < ls->lifetimecount; i++)
		for (j=i+1; j < ls->lifetimecount; j++)
			if ( !((ls->lifetime[ls->lt_used[i]].i_end
				   < ls->lifetime[ls->lt_used[j]].i_start)
				|| (ls->lifetime[ls->lt_used[j]].i_end <
				   ls->lifetime[ls->lt_used[i]].i_start)) ) {
				count_locals_conflicts += 2;
				STATISTICS(count_locals_conflicts += 2);
			}
#endif
	/* Run LSRA */
	lsra_main(jd);

	/* everything's ok */

	return true;
}

/* sort Basic Blocks using Depth First Search in reverse post order in */
/* ls->sorted.  */
void lsra_DFS(jitdata *jd) {
	int *stack;
	int *visited;
	int stack_top;
	bool not_finished;
	int i,p;
    struct _list *succ;

	methodinfo *m;
	lsradata   *ls;

	m  = jd->m;
	ls = jd->ls;

	stack = DMNEW( int, m->basicblockcount + 1);
	visited = (int *)DMNEW( int, m->basicblockcount + 1);
	for (i = 0; i <= m->basicblockcount; i++) {
		visited[i] = 0;
		ls->sorted[i]=-1;
		ls->sorted_rev[i]=-1;
	}

    stack[0] = 0; /* start with Block 0 */
	stack_top = 1;
	visited[0] = ls->num_pred[0]; /* Start Block is handled right and can be */
	                              /* put in sorted */
	p = 0;
	not_finished = true;
	while (not_finished) {
		while (stack_top != 0) {
			stack_top--;
			i = stack[stack_top];
			ls->sorted[p]=i;
			p++; /*--;*/
			for (succ = ls->succ[i]; succ != NULL; succ = succ->next) {
				visited[succ->value]++;
				if (visited[succ->value] == ls->num_pred[succ->value]) {
					/* push the node on the stack, only if all ancestors have */
					/* been visited */
					stack[stack_top] = succ->value;
					stack_top++;
				}
			}
		}
		not_finished = false;
		for (i=1; i <= m->basicblockcount; i++) {
			/* search for visited blocks, which have not reached the num_pred */
			/* and put them on the stack -> happens with backedges */
			if ((visited[i] != 0) && (visited[i] < ls->num_pred[i])) {
				stack[stack_top] = i;
				stack_top++;
				visited[i] = ls->num_pred[i];
				not_finished=true;
				break;
			}
		}
	}
}

void lsra_get_backedges_(lsradata *ls, int basicblockcount) {
	struct _list *s;
	struct _backedge *n;
	struct _backedge *_backedges;
	int    i;


	_backedges = NULL;

	/* now look for backedges */
	ls->backedge_count = 0;
	for(i=0; i < basicblockcount; i++) {
		if (ls->sorted[i] != -1)
			for(s=ls->succ[ls->sorted[i]]; s != NULL; s=s->next) {
				if (i >= ls->sorted_rev[s->value]) {
					n=DNEW(struct _backedge);
					n->start = max(i, ls->sorted_rev[s->value]);
					n->end = min(i, ls->sorted_rev[s->value]);
					n->next = _backedges;
					_backedges = n;
					ls->backedge_count++;
				}
			}
	}
	/* put _backedges in ls->backedge array */
	ls->backedge = DMNEW(struct _backedge *, ls->backedge_count);
	for (n=_backedges, i=0; n != NULL; n=n->next, i++) {
		ls->backedge[i] = n;
		ls->backedge[i]->nesting = 1;
	}
}

void lsra_get_nesting(jitdata *jd) {
	methodinfo *m;
	lsradata   *ls;
	int    i,j, end;
	struct _backedge *n;

	m = jd->m;
	ls = jd->ls;

	for (i=0; i <= m->basicblockcount; i++)
		if (ls->sorted[i] != -1)
			ls->sorted_rev[ls->sorted[i]]=i;

	lsra_get_backedges_(ls, m->basicblockcount + 1);
	/* - sort backedge by increasing end: */
	for (i=0; i < ls->backedge_count; i++)
		for (j=i+1; j < ls->backedge_count; j++)
			if ((ls->backedge[i]->end > ls->backedge[j]->end) ||     /* -> swap */
				  ((ls->backedge[i]->end == ls->backedge[j]->end) &&
				   (ls->backedge[i]->start > ls->backedge[j]->start) )) {
				n=ls->backedge[i];
				ls->backedge[i]=ls->backedge[j];
				ls->backedge[j]=n;
			}

	/* create ls->nesting */
	/* look for nesting depth (overlapping backedges*/
	for (i=0; i < ls->backedge_count - 1; i++) {
		for (j = i + 1; (j < ls->backedge_count) &&
				 (ls->backedge[i]->start >= ls->backedge[j]->end); j++)
			ls->backedge[j]->nesting += ls->backedge[i]->nesting;
	}

    i = 0;
    j = 0;
	while ( (i < m->basicblockcount + 1)  ) {
		if (j < ls->backedge_count) {
			while ( i < ls->backedge[j]->end ) {
				ls->nesting[i] = 0;
				i++;
			}
			if ( (j+1) < ls->backedge_count)
				end = min(ls->backedge[j]->start, ls->backedge[j+1]->end - 1);
			else
				end = ls->backedge[j]->start;
			while (i <= end) {
				ls->nesting[i] = ls->backedge[j]->nesting;
				i++;
			}
			j++;
		} else {
			ls->nesting[i] = 0;
			i++;
		}
	}

#ifdef LSRA_DEBUG_VERBOSE
	if (compileverbose) {
	printf("sorted: \n");
	for (i=0; i < ls->backedge_count; i++)
		printf("Backedge: %i - %i, %i - %i\n", ls->sorted[ls->backedge[i]->start], ls->sorted[ls->backedge[i]->end], ls->backedge[i]->start, ls->backedge[i]->end);
	printf("Nesting Level \n");
	for (i=0; i<m->basicblockcount; i++) printf(" %3li", ls->nesting[i]);
	printf("\n");
	}
#endif
	for (i=0; i <= m->basicblockcount; i++) {
		ls->sorted_rev[i] = -1;
		ls->nesting[i] = 1+ls->nesting[i]*ls->nesting[i]*10;
	}
}

void lsra_get_backedges(jitdata *jd) {
	methodinfo *m;
	lsradata   *ls;
	struct _list **next;
	struct _backedge *n;
	int    i,j;
	bool   merged;

	m  = jd->m;
	ls = jd->ls;

	/* first remove artificial end basicblock from ls->sorted, succ and pred */
    j=-1;
	for (i=0; i < m->basicblockcount; i++) {
		for (next=&(ls->succ[i]); *next != NULL; next=&((*next)->next)) {
			if ( (*next)->value == m->basicblockcount ) {
				/* artificial end bb found */
				*next = (*next)->next;
				if (*next == NULL) break;
			}
		}
		for (next=&(ls->pred[i]); *next != NULL; next=&((*next)->next)) {
			if ( (*next)->value == m->basicblockcount ) {
				/* artificial end bb found */
				*next = (*next)->next;
				if (*next == NULL) break;
			}
		}
		if (j==-1)
			if (ls->sorted[i] == m->basicblockcount) j=i;
  	}

	/* if an artificial end block was removed -> change ls->sorted accordingly*/
	if (j!=-1)
		for (i=j+1; i <= m->basicblockcount; i++) {
			ls->sorted[i-1] = ls->sorted[i];
			ls->nesting[i-1] = ls->nesting[i];
		}

	for (i=0; i < m->basicblockcount; i++)
		if (ls->sorted[i] != -1)
			ls->sorted_rev[ls->sorted[i]]=i;

	lsra_get_backedges_(ls, m->basicblockcount);

	/* - sort backedge by increasing start */
	for (i=0; i < ls->backedge_count; i++)
		for (j=i+1; j < ls->backedge_count; j++)
			if (ls->backedge[i]->start > ls->backedge[j]->start) {
				/* -> swap */
				n = ls->backedge[i];
				ls->backedge[i] = ls->backedge[j];
				ls->backedge[j] = n;
			}

#ifdef LSRA_DEBUG_VERBOSE
	if (compileverbose) {
		printf("sorted: \n");
		for (i=0; i < ls->backedge_count; i++)
			printf("Backedge: %i - %i, %i - %i\n",
				   ls->sorted[ls->backedge[i]->start],
				   ls->sorted[ls->backedge[i]->end], ls->backedge[i]->start,
				   ls->backedge[i]->end);
		printf("Nesting Level \n");
		for (i=0; i<m->basicblockcount; i++) printf(" %3li", ls->nesting[i]);
		printf("\n");
	}
#endif

	merged = true;
	/* - merge overlapping backedges */
	while (merged) {
		merged = false;
		for (i=0; i < ls->backedge_count-1; i++) {
			if (ls->backedge[i] != NULL) {
				for (j = i + 1; (j < ls->backedge_count) && (ls->backedge[j] == NULL); j++ );
				if (j != ls->backedge_count) {
					if (ls->backedge[i]->start >= ls->backedge[j]->end) {
						merged = true;
						/* overlapping -> merge */
						ls->backedge[j]->end = min (ls->backedge[j]->end,
												    ls->backedge[i]->end);
						ls->backedge[i] = NULL;
					}
				}
			}
		}
	}
#ifdef LSRA_DEBUG_VERBOSE
	if (compileverbose) {
		printf("merged: \n");
		for (i = 0; i < ls->backedge_count; i++)
			if (ls->backedge[i] != NULL)
				printf("Backedge: %i - %i, %i - %i\n",
					   ls->sorted[ls->backedge[i]->start],
					   ls->sorted[ls->backedge[i]->end],
					   ls->backedge[i]->start, ls->backedge[i]->end);
	}
#endif
	/* - remove backedge[] == NULL from array */

	for (j = ls->backedge_count - 1; ((j>=0) && (ls->backedge[j] == NULL));
		 j--);
	i=j;
	while (i >= 0) {
		if (ls->backedge[i] == NULL) { /* swap backedge[i] and backedge[j]*/
			n=ls->backedge[j];
			ls->backedge[j] = ls->backedge[i];
			ls->backedge[i] = n;
			i--;
			j--;
			ls->backedge_count--;
		} else i--;
	}
#ifdef LSRA_DEBUG_VERBOSE
	if (compileverbose) {
		printf("ready: \n");
		for (i=0; i < ls->backedge_count; i++)
			printf("Backedge: %i - %i, %i - %i\n",
				   ls->sorted[ls->backedge[i]->start],
				   ls->sorted[ls->backedge[i]->end],ls->backedge[i]->start,
				   ls->backedge[i]->end);
	}
#endif
}

void lsra_add_cfg(jitdata *jd, int from, int to) {
	struct _list *n;
	methodinfo *m;
	lsradata   *ls;

	m  = jd->m;
	ls = jd->ls;

	/* ignore Empty, Deleted,... Basic Blocks as target */
	/* TODO: Setup BasicBlock array before to avoid this */
	/*       best together with using the basicblock list, so lsra works */
	/*       with opt_loops, too */
	for (;(to < m->basicblockcount) && (m->basicblocks[to].flags < basicblock::REACHED); to++);

	for (n=ls->succ[from]; (n!= NULL) && (n->value != to); n=n->next);
	if (n != NULL) return; /* edge from->to already existing */

	n=DNEW(struct _list);

	n->value=to;
	n->next=ls->succ[from];
	ls->succ[from]=n;

	n=DNEW(struct _list);
	n->value=from;
	n->next=ls->pred[to];
	ls->pred[to]=n;
	ls->num_pred[to]++;
}

/* add Edges from guarded Areas to Exception handlers in the CFG */
void lsra_add_exceptions(jitdata *jd) {
	methodinfo  *m;
	lsradata    *ls;
	int i;
	exceptiontable *ex;


	m  = jd->m;
	ls = jd->ls;

	ex = jd->exceptiontable;

	/* add cfg edges from all bb of a try block to the start of the according */
	/* exception handler to ensure the right order after depthfirst search    */

#ifdef LSRA_DEBUG_VERBOSE
	if (compileverbose)
		printf("ExTable(%i): ", jd->exceptiontablelength);
#endif

	for (; ex != NULL; ex = ex->down) {

#ifdef LSRA_DEBUG_VERBOSE
	if (compileverbose) {
		printf("[%i-%i]->%i ",ex->start->nr, ex->end->nr,
			   ex->handler->nr);
		if (ex->handler->nr >= m->basicblockcount) {
			log_text("Exceptionhandler Basicblocknummer invalid\n");
			abort();
		}
		if (m->basicblocks[ex->handler->nr].flags < basicblock::REACHED) {
			log_text("Exceptionhandler Basicblocknummer not reachable\n");
			abort();
		}
		if (ex->start->nr > ex->end->nr) {
			log_text("Guarded Area starts after its end\n");
			abort();
		}
	}
#endif
		/* loop all valid Basic Blocks of the guarded area and add CFG edges  */
		/* to the appropriate handler */
		for (i=ex->start->nr; (i <= ex->end->nr) &&
				 (i < m->basicblockcount); i++)
			if (m->basicblocks[i].flags >= basicblock::REACHED)
				lsra_add_cfg(jd, i, ex->handler->nr);
	}
#ifdef LSRA_DEBUG_VERBOSE
	if (compileverbose) {
		printf("\n");
	}
#endif
}

void lsra_add_jsr(jitdata *jd, int from, int to) {
	methodinfo *m;
	lsradata   *ls;
	struct _sbr *sbr, *n;
	struct _list *ret;

	ls = jd->ls;
	m  = jd->m;

	/* ignore Empty, Deleted,... Basic Blocks as target */
	/* TODO: Setup BasicBlock array before to avoid this */
	/*       best together with using the basicblock list, so lsra works */
	/*       with opt_loops, too */
	for (; (to < m->basicblockcount) && (m->basicblocks[to].flags < basicblock::REACHED);
		 to++);
#ifdef LSRA_DEBUG_CHECK
		if (to == m->basicblockcount)
			{ log_text("Invalid subroutine start index\n"); assert(0); }
#endif

	lsra_add_cfg(jd, from, to);

	/* from + 1 ist the return Basic Block Index */
	for (from++; (from < m->basicblockcount) &&
			 (m->basicblocks[from].flags < basicblock::REACHED); from++);
#ifdef LSRA_DEBUG_CHECK
	if (from == m->basicblockcount)
		{ log_text("Invalid return basic block index for jsr\n"); assert(0); }
#endif

	/* add subroutine info in ls->sbr.next */

	/* search for right place to insert */
	for (sbr = &(ls->sbr); (sbr->next != NULL) && (sbr->next->header < to); sbr=sbr->next);

	if ((sbr->next!= NULL) && (sbr->next->header == to)) {
		/* Entry for this sub already exist */
		sbr = sbr->next;
	} else {
		/* make new Entry and insert it in ls->sbr.next */
		n = DNEW( struct _sbr );
		n->header = to;
		n->ret = NULL;

		n->next = sbr->next;
		sbr->next = n;

		sbr = n;
	}

	/* now insert return adress in sbr->ret */
	ret = DNEW( struct _list);
	ret->value = from;
	ret->next = sbr->ret;
	sbr->ret = ret;
}

void lsra_add_sub( jitdata *jd, int b_index, struct _list *ret,
				   bool *visited )
{
	methodinfo *m;
	lsradata  *ls;
	struct _list *l;
	instruction *ip;
	bool next_block;

	m  = jd->m;
	ls = jd->ls;

	/* break at virtual End Block */
	if (b_index != m->basicblockcount) {
		visited[b_index] = true;
		next_block = false;

		if (m->basicblocks[b_index].flags < basicblock::REACHED)
			next_block = true;
		if (!next_block && !(m->basicblocks[b_index].icount))
			next_block = true;

		if (!next_block) {
			ip = m->basicblocks[b_index].iinstr
				+ m->basicblocks[b_index].icount -1;

			if (ip->opc == ICMD_JSR) /* nested Subroutines */
				next_block = true;
		}

		if (!next_block) {
			if (ip->opc == ICMD_RET) {
				/* subroutine return found -> add return adresses to CFG */
				for (l = ret; l != NULL; l = l->next)
					lsra_add_cfg(jd, b_index, l->value);
			} else { /* follow CFG */
				for ( l = ls->succ[b_index]; l != NULL; l = l->next)
					if (!visited[l->value])
						lsra_add_sub(jd, l->value, ret, visited);
			}
		} else { /* fall through to next block */
				if (!visited[b_index + 1])
					lsra_add_sub(jd, b_index + 1, ret, visited);
		}
	}
}

/* Add subroutines from ls->sbr list to CFG */
void lsra_add_subs(jitdata *jd) {
	methodinfo *m;
	lsradata   *ls;
	struct _sbr *sbr;
	bool *visited;
	int i;
#ifdef LSRA_DEBUG_VERBOSE
	struct _list *ret;
#endif

	m  = jd->m;
	ls = jd->ls;

	visited = (bool *)DMNEW(int, m->basicblockcount + 1);
	for (i=0; i <= m->basicblockcount; i++) visited[i] = false;
	for (sbr = ls->sbr.next; sbr != NULL; sbr=sbr->next) {

#ifdef LSRA_DEBUG_VERBOSE
		if (compileverbose) {
			printf("Subroutine Header: %3i Return Adresses:",sbr->header);
			for (ret = sbr->ret; ret != NULL; ret = ret->next)
				printf(" %3i", ret->value);
			printf("\n");
		}
#endif
		lsra_add_sub(jd, sbr->header, sbr->ret, visited );
	}
}

/* Generate the Control Flow Graph             */
/* ( pred,succ,num_pred of lsradata structure) */

void lsra_make_cfg(jitdata *jd) {
	methodinfo *m;
	lsradata *ls;
	instruction *ip;
	s4 *s4ptr;
	int high, low, count;
	int b_index, len;

	m  = jd->m;
	ls = jd->ls;

	b_index=0;
	while (b_index < m->basicblockcount ) {
		if ((m->basicblocks[b_index].flags >= basicblock::REACHED) &&
			(len = m->basicblocks[b_index].icount)) {
			/* block is valid and contains instructions	*/

			/* set ip to last instruction	*/
			ip = m->basicblocks[b_index].iinstr +
				m->basicblocks[b_index].icount -1;
			while ((len>0) && (ip->opc == ICMD_NOP)) {
				len--;
				ip--;
			}
			switch (ip->opc) {			/* check type of last instruction */
			    case ICMD_RETURN:
			    case ICMD_IRETURN:
			    case ICMD_LRETURN:
				case ICMD_FRETURN:
				case ICMD_DRETURN:
				case ICMD_ARETURN:
				case ICMD_ATHROW:
					lsra_add_cfg(jd, b_index, m->basicblockcount);
					break;				  /* function returns -> end of graph */

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
				case ICMD_IF_ACMPNE:		    /* branch -> add next block */
					lsra_add_cfg(jd, b_index, b_index+1);
					/* fall throu -> add branch target */

				case ICMD_GOTO:
					lsra_add_cfg(jd, b_index,  m->basicblockindex[ip->op1]);
					break;					/* visit branch (goto) target	*/

				case ICMD_TABLESWITCH:		/* switch statement				*/
					s4ptr = ip->val.a;

					lsra_add_cfg(jd, b_index,  m->basicblockindex[*s4ptr]);

					s4ptr++;
					low = *s4ptr;
					s4ptr++;
					high = *s4ptr;

					count = (high-low+1);

					while (--count >= 0) {
						s4ptr++;
						lsra_add_cfg(jd, b_index,
									 m->basicblockindex[*s4ptr]);
				    }
					break;

				case ICMD_LOOKUPSWITCH:		/* switch statement				*/
					s4ptr = ip->val.a;

					lsra_add_cfg(jd, b_index,  m->basicblockindex[*s4ptr]);

					++s4ptr;
					count = *s4ptr++;

					while (--count >= 0) {
						lsra_add_cfg(jd, b_index,
									 m->basicblockindex[s4ptr[1]]);
						s4ptr += 2;
				    }
					break;

				case ICMD_JSR:
					lsra_add_jsr(jd, b_index, m->basicblockindex[ip->op1]);
					break;

				case ICMD_RET:
					break;

				default:
					lsra_add_cfg(jd, b_index, b_index + 1 );
					break;
			} /* switch (ip->opc)*/
		}     /* if ((m->basicblocks[blockIndex].icount)&& */
		      /*     (m->basicblocks[b_index].flags >= basicblock::REACHED)) */
		b_index++;
	}             /* while (b_index < m->basicblockcount ) */
}

void lsra_init(jitdata *jd) {
	methodinfo   *m;
	lsradata     *ls;
	int i;

	ls = jd->ls;
	m  = jd->m;

	/* Init LSRA Data Structures */
	/* allocate lifetimes for all Basicblocks */
	/* + 1 for an artificial exit node */
	/* which is needed as "start" point for the reverse postorder sorting */
	ls->pred = DMNEW(struct _list *, m->basicblockcount+1);
	ls->succ = DMNEW(struct _list *, m->basicblockcount+1);
	ls->sorted = DMNEW(int , m->basicblockcount+1);
	ls->sorted_rev = DMNEW(int , m->basicblockcount+1);
	ls->num_pred = DMNEW(int , m->basicblockcount+1);
	ls->nesting = DMNEW(long , m->basicblockcount+1);
	for (i=0; i<m->basicblockcount; i++) {
		ls->pred[i]=NULL;
		ls->succ[i]=NULL;
		ls->sorted[i]=-1;
		ls->sorted_rev[i]=-1;
		ls->num_pred[i]=0;
		ls->nesting[i] = 1;
	}
	ls->pred[m->basicblockcount]=NULL;
	ls->succ[m->basicblockcount]=NULL;
	ls->sorted[m->basicblockcount]=-1;
	ls->sorted_rev[m->basicblockcount]=-1;
	ls->num_pred[m->basicblockcount]=0;

	ls->sbr.next = NULL;

	ls->v_index = -1;
}

void lsra_setup(jitdata *jd) {
	methodinfo *m;
	codegendata *cd;
	lsradata *ls;

#ifdef LSRA_DEBUG_VERBOSE
	basicblock  *bptr;
	struct _list *nl;
#endif

	int i;

#if !defined(LV)
	registerdata *rd;

	rd = jd->rd;
#endif

	ls = jd->ls;
	m  = jd->m;
	cd = jd->cd;

#if defined(ENABLE_LOOP)
	/* Loop optimization "destroys" the basicblock array */
	/* TODO: work with the basicblock list               */
	if (opt_loops) {
		log_text("lsra not possible with loop optimization\n");
		abort();
	}
#endif /* defined(ENABLE_LOOP) */

	/* Setup LSRA Data structures */

	/* Generate the Control Flow Graph */
	lsra_make_cfg(jd);
	/* gather nesting before adding of Exceptions and Subroutines!!! */

#ifdef USAGE_COUNT
	lsra_DFS(jd);
	lsra_get_nesting(jd);
#endif

#ifdef LSRA_DEBUG_VERBOSE
	if (compileverbose) {
	printf("Successors:\n");
	for (i=0; i < m->basicblockcount; i++) {
		printf("%3i->: ",i);
		for (nl=ls->succ[i]; nl!= NULL; nl=nl->next)
			printf("%3i ",nl->value);
		printf("\n");
	}
	printf("Predecessors:\n");
	for (i=0; i < m->basicblockcount; i++) {
		printf("%3i->: ",i);
		for (nl=ls->pred[i]; nl!= NULL; nl=nl->next)
			printf("%3i ",nl->value);
		printf("\n");
	}
	printf("Sorted: ");
	for (i=0; i < m->basicblockcount; i++) printf("%3i ", ls->sorted[i]);
	printf("\n");
	printf("Sorted_rev: ");
	for (i=0; i < m->basicblockcount; i++) printf("%3i ", ls->sorted_rev[i]);
	printf("\n");
	}
#endif

	/* add subroutines before exceptions! They "destroy" the CFG */
	lsra_add_subs(jd);
 	lsra_add_exceptions(jd);

	/* generate reverse post order sort */
	lsra_DFS(jd);

	/* setup backedge and nested data structures*/
	lsra_get_backedges(jd);

	liveness_init(jd);

	ls->lifetimecount = ls->maxlifetimes + jd->maxlocals * (TYPE_ADR+1);
	ls->lifetime = DMNEW(struct lifetime, ls->lifetimecount);
	ls->lt_used  = DMNEW(int, ls->lifetimecount);
	ls->lt_int   = DMNEW(int, ls->lifetimecount);
	ls->lt_int_count = 0;
	ls->lt_flt   = DMNEW(int, ls->lifetimecount);
	ls->lt_flt_count = 0;
	ls->lt_mem   = DMNEW(int, ls->lifetimecount);
	ls->lt_mem_count = 0;

	for (i=0; i < ls->lifetimecount; i++) ls->lifetime[i].type = -1;

#ifdef LSRA_DEBUG_VERBOSE
	if (compileverbose) {
	printf("Successors:\n");
	for (i=0; i < m->basicblockcount; i++) {
		printf("%3i->: ",i);
		for (nl=ls->succ[i]; nl!= NULL; nl=nl->next)
			printf("%3i ",nl->value);
		printf("\n");
	}
	printf("Predecessors:\n");
	for (i=0; i < m->basicblockcount; i++) {
		printf("%3i->: ",i);
		for (nl=ls->pred[i]; nl!= NULL; nl=nl->next)
			printf("%3i ",nl->value);
		printf("\n");
	}
	printf("Sorted: ");
	for (i=0; i < m->basicblockcount; i++) printf("%3i ", ls->sorted[i]);
	printf("\n");
	printf("Sorted_rev: ");
	for (i=0; i < m->basicblockcount; i++) printf("%3i ", ls->sorted_rev[i]);
	printf("\n");
	}
#endif


#ifdef LSRA_DEBUG_CHECK
	/* compare m->basicblocks[] with the list basicblocks->next */
	i=0;
	bptr = m->basicblocks;
	while (bptr != NULL) {
		if (i > m->basicblockcount){
			{ log_text("linked bb list does not correspond with bb array(1)\n");
				assert(0); }
		}
		if (bptr != &(m->basicblocks[i])){
			{ log_text("linked bb list does not correspond with bb array(2)\n");
				assert(0); }
		}

		i++;
		bptr=bptr->next;
	}
	if (i<m->basicblockcount){
		{ log_text("linked bb list does not correspond with bb array(3)\n");
			assert(0); }
	}

#endif

	liveness_setup(jd);

#if defined(LV)
	liveness(jd);
#else /* LV */
	{
		int p, t;
		methoddesc *md = m->parseddesc;

		/* Create Stack Slot lifetimes over all basic blocks */
		for (i=m->basicblockcount-1; i >= 0; i--) {
			if (ls->sorted[i] != -1) {
				lsra_scan_registers_canditates(jd, ls->sorted[i]);
				lsra_join_lifetimes(jd, ls->sorted[i]);
			}
		}

		/* Parameter initialisiation for locals [0 .. paramcount[            */
		/* -> add local var write access at (bb=0,iindex=-1)                 */
		/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!   */
		/* this needs a special treatment, wenn lifetimes get extended       */
		/* over backedges, since this parameter initialisation happens       */
		/* outside of Basic Block 0 !!!!                                     */
		/* this could have been avoided by marking the read access with -1,0 */

		for (p = 0, i = 0; p < md->paramcount; p++) {
			t = md->paramtypes[p].type;

			if (rd->locals[i][t].type >= 0)
				/* Param to Local init happens before normal Code */
				lsra_usage_local(ls, i, t, 0, -1, LSRA_STORE);
			i++;
			/* increment local counter a second time  */
			/* for 2 word types */
			if (IS_2_WORD_TYPE(t))
				i++;
		}  /* end for */
	}
#endif /* LV */

	lsra_calc_lifetime_length(jd);

#ifdef LSRA_DEBUG_VERBOSE
	if (compileverbose)
		printf("Basicblockcount: %4i\n",m->basicblockcount);
#endif
}


void lsra_reg_setup(jitdata *jd, struct lsra_register *int_reg,
					struct lsra_register *flt_reg ) {
	int i, j, iarg, farg;
	int int_sav_top;
	int flt_sav_top;
	bool *fltarg_used, *intarg_used;
	methoddesc *md;
	methodinfo *m;
	registerdata *rd;

	m  = jd->m;
	rd = jd->rd;
	md = m->parseddesc;

	int_reg->nregdesc = nregdescint;
	flt_reg->nregdesc = nregdescfloat;
	if (code_is_leafmethod(code)) {
		/* Temp and Argumentregister can be used as saved registers */

		int_reg->sav_top = INT_ARG_CNT + INT_TMP_CNT + INT_SAV_CNT;
		int_reg->sav_reg = DMNEW(int, int_reg->sav_top);
		int_reg->tmp_reg = NULL;
		int_reg->tmp_top = -1;
		flt_reg->sav_top = FLT_ARG_CNT + FLT_TMP_CNT + FLT_SAV_CNT;
		flt_reg->sav_reg = DMNEW(int, flt_reg->sav_top);
		flt_reg->tmp_reg = NULL;
		flt_reg->tmp_top = -1;

		/* additionaly precolour registers for Local Variables acting as */
		/* Parameters */

		farg = FLT_ARG_CNT;
		iarg = INT_ARG_CNT;

		intarg_used = DMNEW(bool, INT_ARG_CNT);
		for (i=0; i < INT_ARG_CNT; i++)
			intarg_used[i]=false;

		fltarg_used = DMNEW(bool, FLT_ARG_CNT);
		for (i=0; i < FLT_ARG_CNT; i++)
			fltarg_used[i]=false;

		int_sav_top=int_reg->sav_top;
		flt_sav_top=flt_reg->sav_top;

		for (i=0; (i < md->paramcount); i++) {
			if (!md->params[i].inmemory) {
				if (IS_INT_LNG_TYPE(md->paramtypes[i].type)) {
#if defined(SUPPORT_COMBINE_INTEGER_REGISTERS)
					if (IS_2_WORD_TYPE(md->paramtypes[i].type)) {
						int_reg->sav_reg[--int_sav_top] =
							rd->argintregs[GET_HIGH_REG(md->params[i].regoff)];
						intarg_used[GET_HIGH_REG(md->params[i].regoff)]=true;
						/*used -> don't copy later on */
						int_reg->sav_reg[--int_sav_top] =
							rd->argintregs[GET_LOW_REG(md->params[i].regoff)];
						intarg_used[GET_LOW_REG(md->params[i].regoff)]=true;
						/*used -> don't copy later on */
					} else
#endif
					{ /* !IS_2_WORD_TYPE(md->paramtypes[i].type */
						int_reg->sav_reg[--int_sav_top] =
							rd->argintregs[md->params[i].regoff];
						intarg_used[md->params[i].regoff]=true;
						/*used -> don't copy later on */
					}
				}
#if !defined(SUPPORT_PASS_FLOATARGS_IN_INTREGS)
				/* do not precolour float arguments if they are passed in     */
				/* integer registers. But these integer argument registers    */
				/* still be used in the method! */
				else { /* IS_FLT_DBL_TYPE(md->paramtypes[i].type */
						flt_reg->sav_reg[--flt_sav_top] =
							rd->argfltregs[md->params[i].regoff];
						fltarg_used[md->params[i].regoff]=true;
				}
#endif

			}
		}

		/* copy rest of argument registers to flt_reg->sav_reg and */
		/* int_reg->sav_reg; */
		for (i=0; i < INT_ARG_CNT; i++)
			if (!intarg_used[i])
				int_reg->sav_reg[--int_sav_top]=rd->argintregs[i];
		for (i=0; i < FLT_ARG_CNT; i++)
			if (!fltarg_used[i])
				flt_reg->sav_reg[--flt_sav_top]=rd->argfltregs[i];

		/* copy temp registers to flt_reg->sav_reg and int_reg->sav_reg */
		for (i=0; i < INT_TMP_CNT; i++)
			int_reg->sav_reg[--int_sav_top]=rd->tmpintregs[i];
		for (i=0; i < FLT_TMP_CNT; i++)
			flt_reg->sav_reg[--flt_sav_top]=rd->tmpfltregs[i];

	} else {
		/* non leaf method -> use Argument Registers [arg[int|flt]reguse */
		/* ... [INT|FLT]_ARG_CNT[ as temp reg */
		/* divide temp and saved registers */
		int argintreguse, argfltreguse;
#ifdef LV
		/* with Locals as non SAVEDVAR, the used arg[int|flt] as in params */
		/* of the method itself have to be regarded, or mismatch before    */
		/* block 0 with parameter copy could happen! */
		argintreguse = max(rd->argintreguse, md->argintreguse);
		argfltreguse = max(rd->argfltreguse, md->argfltreguse);
#else
		argintreguse = rd->argintreguse;
		argfltreguse = rd->argfltreguse;
#endif
		int_sav_top = int_reg->sav_top = INT_SAV_CNT;
		int_reg->sav_reg = DMNEW(int, int_reg->sav_top);
		int_reg->tmp_top = INT_TMP_CNT +
			max(0, (INT_ARG_CNT - argintreguse));
		int_reg->tmp_reg = DMNEW(int, int_reg->tmp_top);

		flt_sav_top =flt_reg->sav_top = FLT_SAV_CNT;
		flt_reg->sav_reg = DMNEW(int, flt_reg->sav_top);
		flt_reg->tmp_top = FLT_TMP_CNT +
			max(0 , (FLT_ARG_CNT - argfltreguse));
		flt_reg->tmp_reg = DMNEW(int, flt_reg->tmp_top);

		/* copy temp and unused argument registers to flt_reg->tmp_reg and */
		/* int_reg->tmp_reg */
		for (i=0; i < INT_TMP_CNT; i++)
			int_reg->tmp_reg[i]=rd->tmpintregs[i];
		for (j=argintreguse; j < INT_ARG_CNT; j++, i++)
			int_reg->tmp_reg[i]=rd->argintregs[j];
		for (i=0; i < FLT_TMP_CNT; i++)
			flt_reg->tmp_reg[i]=rd->tmpfltregs[i];
		for (j=argfltreguse; j < FLT_ARG_CNT; j++, i++)
			flt_reg->tmp_reg[i]=rd->argfltregs[j];
	}

	/* now copy saved registers to flt_reg->sav_reg and int_reg->sav_reg */
	for (i = INT_SAV_CNT-1; i >= 0; i--)
		int_reg->sav_reg[--int_sav_top]=rd->savintregs[i];
	for (i = FLT_SAV_CNT-1; i >= 0; i--)
		flt_reg->sav_reg[--flt_sav_top]=rd->savfltregs[i];
	/* done */
}

void lsra_insertion_sort( struct lsradata *ls, int *a, int lo, int hi) {
	int i,j,t,tmp;

	for (i=lo+1; i<=hi; i++) {
		j=i;
		t=ls->lifetime[a[j]].i_start;
		tmp = a[j];
		while ((j>lo) && (ls->lifetime[a[j-1]].i_start > t)) {
			a[j]=a[j-1];
			j--;
		}
		a[j]=tmp;
	}
}

void lsra_qsort( struct lsradata *ls, int *a, int lo, int hi) {
	int i,j,x,tmp;
	if (lo < hi) {
		if ( (lo+5)<hi) {
			i = lo;
			j = hi;
			x = ls->lifetime[a[(lo+hi)/2]].i_start;

			while (i <= j) {
				while (ls->lifetime[a[i]].i_start < x) i++;
				while (ls->lifetime[a[j]].i_start > x) j--;
				if (i <= j) {
					/* exchange a[i], a[j] */
					tmp = a[i];
					a[i] = a[j];
					a[j] = tmp;

					i++;
					j--;
				}
			}

			if (lo < j) lsra_qsort( ls, a, lo, j);
			if (i < hi) lsra_qsort( ls, a, i, hi);
		} else
			lsra_insertion_sort(ls, a, lo, hi);
	}
}

void lsra_param_sort(struct lsradata *ls, int *lifetime, int lifetime_count) {

	int param_count;
	int i,j,tmp;

	/* count number of parameters ( .i_start == -1) */
	for (param_count=0; (param_count < lifetime_count) &&
		 (ls->lifetime[lifetime[param_count]].i_start == -1); param_count++);

	if (param_count > 0) {
		/* now sort the parameters by v_index */
		for (i=0; i < param_count -1; i++)
			for (j=i+1; j < param_count; j++)
				if ( ls->lifetime[lifetime[i]].v_index >
					 ls->lifetime[lifetime[j]].v_index) {
					/* swap */
					tmp = lifetime[i];
					lifetime[i]=lifetime[j];
					lifetime[j]=tmp;
				}
	}
}

void lsra_main(jitdata *jd) {
#ifdef LSRA_DEBUG_VERBOSE
	int i;
#endif
	int lsra_mem_use;
	int lsra_reg_use;
	struct lsra_register flt_reg, int_reg;
	lsradata *ls;
	registerdata *rd;
#if defined(__I386__)
	methodinfo *m;

	m  = jd->m;
#endif
	ls = jd->ls;
	rd = jd->rd;

/* sort lifetimes by increasing start */
	lsra_qsort( ls, ls->lt_mem, 0, ls->lt_mem_count - 1);
	lsra_qsort( ls, ls->lt_int, 0, ls->lt_int_count - 1);
	lsra_qsort( ls, ls->lt_flt, 0, ls->lt_flt_count - 1);
/* sort local vars used as parameter */
	lsra_param_sort( ls, ls->lt_int, ls->lt_int_count);
	lsra_param_sort( ls, ls->lt_flt, ls->lt_flt_count);
	lsra_reg_setup(jd, &int_reg, &flt_reg);

#ifdef LSRA_DEBUG_VERBOSE
	if (compileverbose) {
		printf("INTSAV REG: ");
		for (i=0; i<int_reg.sav_top; i++)
			printf("%2i ",int_reg.sav_reg[i]);
		printf("\nINTTMP REG: ");
		for (i=0; i<int_reg.tmp_top; i++)
			printf("%2i ",int_reg.tmp_reg[i]);
		printf("\nFLTSAV REG: ");
		for (i=0; i<flt_reg.sav_top; i++)
			printf("%2i ",flt_reg.sav_reg[i]);
		printf("\nFLTTMP REG: ");
		for (i=0; i<flt_reg.tmp_top; i++)
			printf("%2i ",flt_reg.tmp_reg[i]);
		printf("\n");
	}
#endif
	ls->active_tmp = DMNEW( struct lifetime *, max(INT_REG_CNT, FLT_REG_CNT));
	ls->active_sav = DMNEW( struct lifetime *, max(INT_REG_CNT, FLT_REG_CNT));

	lsra_reg_use=INT_SAV_CNT; /* init to no saved reg used... */
	_lsra_main(jd, ls->lt_int, ls->lt_int_count, &int_reg, &lsra_reg_use);
	if (lsra_reg_use > INT_SAV_CNT)
		lsra_reg_use=INT_SAV_CNT;
	rd->savintreguse = lsra_reg_use;

	lsra_reg_use = FLT_SAV_CNT; /* no saved reg used... */
	_lsra_main(jd, ls->lt_flt, ls->lt_flt_count, &flt_reg, &lsra_reg_use);
	if (lsra_reg_use > FLT_SAV_CNT)
		lsra_reg_use=FLT_SAV_CNT;
	rd->savfltreguse=lsra_reg_use;

	/* rd->memuse was already set in stack.c to allocate stack space for */
	/* passing arguments to called methods */
#if defined(__I386__)
	if (checksync && code_is_synchronized(code)) {
		/* reserve 0(%esp) for Monitorenter/exit Argument on i386 */
		if (rd->memuse < 1)
			rd->memuse = 1;
	}
#endif

	lsra_mem_use = rd->memuse; /* Init with memuse from stack.c */

	lsra_alloc(jd, ls->lt_mem, ls->lt_mem_count, &lsra_mem_use);
	lsra_alloc(jd, ls->lt_int, ls->lt_int_count, &lsra_mem_use);
	lsra_alloc(jd, ls->lt_flt, ls->lt_flt_count, &lsra_mem_use);

	rd->memuse=lsra_mem_use;

#ifdef LSRA_DEBUG_VERBOSE
	if (compileverbose) {
		printf("Int RA complete \n");
		printf("Lifetimes after splitting int: \n");
		print_lifetimes(jd, ls->lt_int, ls->lt_int_count);

		printf("Flt RA complete \n");
		printf("Lifetimes after splitting flt:\n");
		print_lifetimes(jd, ls->lt_flt, ls->lt_flt_count);

		printf("Rest RA complete \n");
		printf("Lifetimes after leftt:\n");
		print_lifetimes(jd, ls->lt_mem, ls->lt_mem_count);
	}
#endif
}

void lsra_alloc(jitdata *jd, int *lifet, int lifetimecount, int *mem_use)
{
	int flags,regoff;
	struct lifetime *lt;
	struct freemem *fmem;
	struct stackslot *n;
	int lt_index;
	registerdata *rd;
	lsradata     *ls;

	rd = jd->rd;
	ls = jd->ls;

	fmem = DNEW(struct freemem);
	fmem->off  = -1;
	fmem->next = NULL;

	for (lt_index = 0; lt_index < lifetimecount; lt_index ++) {
		lt = &(ls->lifetime[lifet[lt_index]]);
#ifdef LSRA_MEMORY
		lt->reg = -1;
#endif
		if (lt->reg == -1) {
			flags = INMEMORY;
			regoff = lsra_getmem(lt, fmem, mem_use);
		} else {
			flags  = lt->savedvar;
			regoff = lt->reg;
		}

		if (lt->v_index < 0) {
			for (n = lt->local_ss; n != NULL; n = n->next) {
				lsra_setflags(&(n->s->flags), flags);
				n->s->regoff = regoff;
			}
		} else { /* local var */
			if (rd->locals[lt->v_index][lt->type].type >= 0) {
				rd->locals[lt->v_index][lt->type].flags  = flags;
				rd->locals[lt->v_index][lt->type].regoff = regoff;
			} else {
				log_text("Type Data mismatch\n");
				abort();
			}
		}
		lt->reg = regoff;
	}
}

void lsra_setflags(int *flags, int newflags)
{
	if ( newflags & INMEMORY)
		*flags |= INMEMORY;
	else
		*flags &= ~INMEMORY;

	if (newflags & SAVEDVAR)
		*flags |= SAVEDVAR;
}

int lsra_getmem(struct lifetime *lt, struct freemem *fmem, int *mem_use)
{
	struct freemem *fm, *p;

	/* no Memory Slot allocated till now or all are still live */
	if ((fmem->next == NULL) || (fmem->next->end > lt->i_start)) {
		fm=lsra_getnewmem(mem_use);
	} else {
		/* Memoryslot free */
		fm = fmem->next;
		fmem->next = fm->next;
		fm->next = NULL;
	}
	fm->end = lt->i_end;
	for (p = fmem; (p->next != NULL) && (p->next->end < fm->end); p = p->next);
	fm->next = p->next;
	p->next = fm;
	return fm->off;
}

struct freemem *lsra_getnewmem(int *mem_use)
{
	struct freemem *fm;

	fm = DNEW(struct freemem);
	fm->next = NULL;
	fm->off = *mem_use;
	(*mem_use)++;
	return fm;
}

void _lsra_main(jitdata *jd, int *lifet, int lifetimecount,
				struct lsra_register *reg, int *reg_use)
{
	struct lifetime *lt;
	int lt_index;
	int reg_index;
	int regsneeded;
	bool temp; /* reg from temp registers (true) or saved registers (false) */
	methodinfo *m;
	lsradata *ls;

	m  = jd->m;
	ls = jd->ls;

#if !defined(SUPPORT_COMBINE_INTEGER_REGISTERS)
	regsneeded = 0;
#endif
	if ((reg->tmp_top+reg->sav_top) == 0) {

		/* no registers available */
		for (lt_index = 0; lt_index < lifetimecount; lt_index++)
			ls->lifetime[lifet[lt_index]].reg = -1;
		return;
	}

	ls->active_tmp_top = 0;
	ls->active_sav_top = 0;

	for (lt_index = 0; lt_index < lifetimecount; lt_index++) {
		lt = &(ls->lifetime[lifet[lt_index]]);

#if defined(SUPPORT_COMBINE_INTEGER_REGISTERS)
	regsneeded = (lt->type == TYPE_LNG)?1:0;
#endif

		lsra_expire_old_intervalls(jd, lt, reg);
		reg_index = -1;
		temp = false;
		if (lt->savedvar || code_is_leafmethod(code)) {
			/* use Saved Reg (in case of leafmethod all regs are saved regs) */
			if (reg->sav_top > regsneeded) {
#if defined(SUPPORT_COMBINE_INTEGER_REGISTERS)
				if (regsneeded)
					reg_index = PACK_REGS(reg->sav_reg[--reg->sav_top],
										  reg->sav_reg[--reg->sav_top]);
				else
#endif

					reg_index = reg->sav_reg[--reg->sav_top];
			}
		} else { /* use Temp Reg or if none is free a Saved Reg */
			if (reg->tmp_top > regsneeded) {
				temp = true;
#if defined(SUPPORT_COMBINE_INTEGER_REGISTERS)
			if (regsneeded)
				reg_index = PACK_REGS(reg->tmp_reg[--reg->tmp_top],
									  reg->tmp_reg[--reg->tmp_top]);
			else
#endif
				reg_index = reg->tmp_reg[--reg->tmp_top];
			}
			else
				if (reg->sav_top > regsneeded) {

#if defined(SUPPORT_COMBINE_INTEGER_REGISTERS)
				if (regsneeded)
					reg_index = PACK_REGS(reg->sav_reg[--reg->sav_top],
										  reg->sav_reg[--reg->sav_top]);
				else
#endif
					reg_index = reg->sav_reg[--reg->sav_top];
				}
		}
		if (reg_index == -1) /* no reg is available anymore... -> spill */
			spill_at_intervall(jd, lt);
		else {
			lt->reg = reg_index;
			if (temp)
				lsra_add_active(lt, ls->active_tmp, &(ls->active_tmp_top));
			else {
				if (reg->sav_top<*reg_use) *reg_use=reg->sav_top;
				lsra_add_active(lt, ls->active_sav, &(ls->active_sav_top));
			}
		}
	}
}

void lsra_add_active(struct lifetime *lt, struct lifetime **active,
					 int *active_top)
{
	int i, j;

	for(i = 0; (i < *active_top) && (active[i]->i_end < lt->i_end); i++);
	for(j = *active_top; j > i; j--) active[j] = active[j-1];
	(*active_top)++;
	active[i] = lt;
}

void lsra_expire_old_intervalls(jitdata *jd, struct lifetime *lt,
								struct lsra_register *reg)
{
	_lsra_expire_old_intervalls(jd, lt, reg, jd->ls->active_tmp,
								&(jd->ls->active_tmp_top));
	_lsra_expire_old_intervalls(jd, lt, reg, jd->ls->active_sav,
								&(jd->ls->active_sav_top));
}

void _lsra_expire_old_intervalls(jitdata *jd, struct lifetime *lt,
								 struct lsra_register *reg,
								 struct lifetime **active, int *active_top)
{
	int i, j, k;

	for(i = 0; i < *active_top; i++) {
		if (active[i]->i_end > lt->i_start) break;

		/* make active[i]->reg available again */
		if (code_is_leafmethod(code)) {
			/* leafmethod -> don't care about type -> put all again into */
			/* reg->sav_reg */
#if defined(SUPPORT_COMBINE_INTEGER_REGISTERS)
			if (active[i]->type == TYPE_LNG) {
				reg->sav_reg[reg->sav_top++] = GET_LOW_REG(active[i]->reg);
				reg->sav_reg[reg->sav_top++] = GET_HIGH_REG(active[i]->reg);
			} else
#endif
				reg->sav_reg[reg->sav_top++] = active[i]->reg;
		} else {
			/* no leafmethod -> distinguish between temp and saved register */
#if defined(SUPPORT_COMBINE_INTEGER_REGISTERS)
			if (active[i]->type == TYPE_LNG) {
				/* no temp and saved regs are packed together, so looking at */
				/* LOW_REG is sufficient */
				if ( reg->nregdesc[ GET_LOW_REG(active[i]->reg)] == REG_SAV) {
					reg->sav_reg[reg->sav_top++] = GET_LOW_REG(active[i]->reg);
					reg->sav_reg[reg->sav_top++] = GET_HIGH_REG(active[i]->reg);
				} else {
					reg->tmp_reg[reg->tmp_top++] = GET_LOW_REG(active[i]->reg);
					reg->tmp_reg[reg->tmp_top++] = GET_HIGH_REG(active[i]->reg);
				}
			} else
#endif
			if ( reg->nregdesc[active[i]->reg] == REG_SAV) {
					reg->sav_reg[reg->sav_top++] = active[i]->reg;
			} else {
					reg->tmp_reg[reg->tmp_top++] = active[i]->reg;
			}
		}
	}

	/* active[0..i[ is to be removed */
	/* -> move [i..*active_top[ to [0..*active_top-i[ */
	for(k = 0, j = i; (j < *active_top); k++,j++)
		active[k] = active[j];

	(*active_top) -= i;

}

void spill_at_intervall(jitdata *jd, struct lifetime *lt )
{
	if (lt->savedvar || code_is_leafmethod(code)) {
		_spill_at_intervall(lt, jd->ls->active_sav, &(jd->ls->active_sav_top));
	} else {
		_spill_at_intervall(lt, jd->ls->active_tmp, &(jd->ls->active_tmp_top));
		if (lt->reg == -1) { /* no tmp free anymore */
			_spill_at_intervall(lt, jd->ls->active_sav,
								&(jd->ls->active_sav_top));
		}
	}
}

void _spill_at_intervall(struct lifetime *lt, struct lifetime **active,
						 int *active_top)
{
	int i, j;
#ifdef USAGE_COUNT_EXACT
	int u_min, i_min;
#endif

	if (*active_top == 0) {
		lt->reg=-1;
		return;
	}

	i = *active_top - 1;
#if defined(USAGE_COUNT_EXACT)
	/* find intervall which ends later or equal than than lt and has the lowest
	   usagecount lower than lt */
	i_min = -1;
	u_min = lt->usagecount;
	for (; (i >= 0) && (active[i]->i_end >= lt->i_end); i--) {
		if (active[i]->usagecount < u_min) {
			u_min = active[i]->usagecount;
			i_min = i;
		}
	}

	if (i_min != -1) {
		i = i_min;
#else
# if defined(USAGE_COUNT) && !defined(USAGE_COUNT_EXACT)
	if ((active[i]->i_end >= lt->i_end)
		 && (active[i]->usagecount < lt->usagecount)) {
# else /* "normal" LSRA heuristic */
	/* get last intervall from active */
	if (active[i]->i_end > lt->i_end) {
# endif
#endif
#if defined(SUPPORT_COMBINE_INTEGER_REGISTERS)
		/* Don't spill between one and two word int types */
		if ((active[i]->type == TYPE_LNG) != (lt->type == TYPE_LNG))
			return;
#endif
		lt->reg = active[i]->reg;
		active[i]->reg=-1;

		(*active_top)--;
		for (j = i; j < *active_top; j++)
			active[j] = active[j + 1];

		lsra_add_active(lt, active, active_top);
	} else {
		lt->reg=-1;
	}
}

void lsra_calc_lifetime_length(jitdata *jd) {
	methodinfo *m;
	lsradata *ls;
	codegendata *cd;
	struct lifetime *lt;
#if defined(LSRA_DEBUG_VERBOSE) || !defined(LV)
 	int i;
#endif
	int lt_index;
	int lifetimecount;
	int flags; /* 0 INMEMORY -> ls->lt_mem */
	           /* 1 INTREG   -> ls->lt_int  */
	           /* 2 FLTREG   -> ls->lt_flt  */


	m  = jd->m;
	ls = jd->ls;
	cd = jd->cd;

#ifdef LSRA_DEBUG_VERBOSE
	if (compileverbose) {
	printf("icount_block: ");
	for (i=0; i < m->basicblockcount; i++)
	    printf("(%3i-%3i) ",i, ls->icount_block[i]);
	printf("\n");
	}
#endif

	/* extend lifetime over backedges (for the lsra version without exact
	   liveness analysis)
	   now iterate through lifetimes and expand them */

	lifetimecount = 0;
	for(lt_index = 0 ;lt_index < ls->lifetimecount; lt_index++) {
		if ( ls->lifetime[lt_index].type != -1) { /* used lifetime */
			/* remember lt_index in lt_sorted */
			ls->lt_used[lifetimecount++] = lt_index;
			lt = &(ls->lifetime[lt_index]);
#if defined(SUPPORT_COMBINE_INTEGER_REGISTERS)
			/* prevent conflicts between lifetimes of type long by increasing
			   the lifetime by one instruction
			   i.e.(ri/rj)  ...
			       (rk/rl)  ICMD_LNEG
			   with i==l and/or j==k
			   to resolve this during codegeneration a temporary register
			   would be needed */
			if (lt->type == TYPE_LNG)
				lt->i_last_use++;
#endif

/* distribute lifetimes to lt_int, lt_flt and lt_mem */

			lt->reg = -1;

			switch (lt->type) {
			case TYPE_LNG:
				flags = 1;
				break;
			case TYPE_INT:
			case TYPE_ADR:
				flags=1;
				break;
			case TYPE_DBL:
			case TYPE_FLT:
#if defined(__I386__)
				/*
				 * for i386 put all floats in memory
				 */
				flags=0;
				break;
#endif
				flags=2;
				break;
			default:
				{ log_text("Unknown Type\n"); assert(0); }
			}

			switch (flags) {
			case 0: /* lt_used[lt_used_index] -> lt_rest */
				ls->lt_mem[ ls->lt_mem_count++ ] = lt_index;
				break;
			case 1: /* l->lifetimes -> lt_int */
				ls->lt_int[ ls->lt_int_count++ ] = lt_index;
				break;
			case 2: /* l->lifetimes -> lt_flt */
				ls->lt_flt[ ls->lt_flt_count++ ] = lt_index;
				break;
			}


			if (lt->i_first_def == INT_MAX) {
#ifdef LSRA_DEBUG_VERBOSE
 				printf("Warning: var not defined! vi: %i start: %i end: %i\n",
					   lt->v_index, lt->i_start, lt->i_end);
#endif
				lt->bb_first_def = 0;
				lt->i_first_def = 0;
			}

  			lt->i_start = lt->i_first_def;

			if (lt->i_last_use == -2) {
#ifdef LSRA_DEBUG_VERBOSE
 				printf("Warning: Var not used! vi: %i start: %i end: %i\n",
					   lt->v_index, lt->i_start, lt->i_end);
#endif
				lt->bb_last_use = lt->bb_first_def;
				lt->i_last_use = lt->i_first_def;
			}

			lt->i_end = lt->i_last_use;

#ifdef LSRA_DEBUG_VERBOSE
			if (lt->i_start > lt->i_end)
				printf("Warning: last use before first def! vi: %i start: %i end: %i\n", lt->v_index, lt->i_start, lt->i_end);
#endif

#if !defined(LV)
			if ((lt->bb_first_def != lt->bb_last_use) ||
				(lt->i_first_def == -1)) {
				/* Lifetime goes over more than one Basic Block ->  */
				/* check for necessary extension over backedges     */
				/* see lsra_get_backedges                           */
				/* Arguments are set "before" Block 0, so they have */
				/* a lifespan of more then one block, too           */

				for (i=0; i < ls->backedge_count; i++) {
					if (!( (lt->bb_first_def > ls->backedge[i]->start) ||
						   (lt->bb_last_use < ls->backedge[i]->end) )) {
						/* Live intervall intersects with a backedge */
						/* 	if (lt->bb_first_def <= ls->backedge[i]->start) */
						if (lt->bb_last_use <= ls->backedge[i]->start)
							lt->i_end =
								ls->icount_block[ls->backedge[i]->start] +
					  m->basicblocks[ls->sorted[ls->backedge[i]->start]].icount;
					}
				}
			}
#endif /* !defined(LV) */

#ifdef USAGE_PER_INSTR
			lt->usagecount = lt->usagecount / ( lt->i_end - lt->i_start + 1);
#endif
		}
	}
	ls->lifetimecount = lifetimecount;
}

#ifdef LSRA_DEBUG_VERBOSE
void print_lifetimes(jitdata *jd, int *lt, int lifetimecount)
{
	struct lifetime *n;
	int lt_index;
	int type,flags,regoff,varkind;

	lsradata     *ls;
	registerdata *rd;

	ls = jd->ls;
	rd = jd->rd;

	for (lt_index = 0; lt_index < lifetimecount; lt_index++) {
		n = &(ls->lifetime[lt[lt_index]]);
		if (n->savedvar == SAVEDVAR)
			printf("S");
		else
			printf(" ");
		if (n->v_index < 0) { /* stackslot */
			type = n->local_ss->s->type;
			flags=n->local_ss->s->flags;
			regoff=n->local_ss->s->regoff;
			varkind=n->local_ss->s->varkind;
		} else { /* local var */
			if (rd->locals[n->v_index][n->type].type>=0) {
				type = rd->locals[n->v_index][n->type].type;
				flags=rd->locals[n->v_index][n->type].flags;
				regoff=rd->locals[n->v_index][n->type].regoff;
				varkind=-1;
			} else
				{ log_text("Type Data mismatch 3\n"); assert(0); }
		}
#if !defined(LV)
		printf("i_Start: %3i(%3i,%3i) i_stop: %3i(%3i,%3i) reg: %3i VI: %3i type: %3i flags: %3i varkind: %3i usage: %3li ltflags: %xi \n",n->i_start, ls->sorted[n->bb_first_def], n->i_first_def,n->i_end, ls->sorted[n->bb_last_use], n->i_last_use,regoff,n->v_index,type,flags, varkind, n->usagecount, n->flags);
#else
		printf("i_Start: %3i i_stop: %3i reg: %3i VI: %3i type: %3i flags: %3i varkind: %3i usage: %3li ltflags: %xi \n",n->i_start, n->i_end, regoff,n->v_index,type,flags, varkind, n->usagecount, n->flags);
#endif
	}
	printf( "%3i Lifetimes printed \n",lt_index);
}
#endif



/******************************************************************************
Helpers for first LSRA Version without exact Liveness Analysis
 *****************************************************************************/

#if !defined(LV)
bool lsra_join_ss( struct lsradata *ls, struct stackelement *in,
				   struct stackelement *out, int join_flag) {
	struct lifetime *lt, *lto;
	struct stackslot *ss, *ss_last;


	if (in->varnum != out->varnum) {
		lt = &(ls->lifetime[-in->varnum - 1]);

#ifdef LSRA_DEBUG_CHECK
		if (join_flag == JOIN_BB)
			if (lt->type == -1) {
				log_text("lsra_join_ss: lifetime for instack not found\n");
				assert(0);
			}
#endif

		if (out->varnum >= 0) { /* no lifetime for this slot till now */
			lsra_add_ss(lt, out);
		} else {
			lto = &(ls->lifetime[-out->varnum - 1]);
			if ((join_flag == JOIN_DUP) || (join_flag == JOIN_OP))
				if ( (lt->flags & JOIN_BB) || (lto->flags & JOIN_BB)) {
					return false;
				}
			if (join_flag == JOIN_DUP)
				if ( (lt->flags & JOIN_OP) || (lto->flags & JOIN_OP)) {
					return false;
				}
#ifdef LSRA_DEBUG_CHECK
			if (lto->type == -1) {
				log_text("lsra_join_ss: lifetime for outstack not found\n");
				abort();
			}
#endif
#ifdef LSRA_DEBUG_CHECK
			if (lto->type != lt->type) {
				log_text("lsra_join_ss: in/out stack type mismatch\n");
				abort();
			}
#endif

			lt->flags |= JOINING;

			/* take Lifetime lto out of ls->lifetimes */
			lto->type = -1;

			/* merge lto into lt of in */

			ss_last = ss = lto->local_ss;
			while (ss != NULL) {
				ss_last = ss;
				ss->s->varnum = lt->v_index;
				ss = ss->next;
			}
			if (ss_last != NULL) {
				ss_last->next = lt->local_ss;
				lt->local_ss = lto->local_ss;
			}

			lt->savedvar |= lto->savedvar;
			lt->flags |= lto->flags | join_flag;
			lt->usagecount += lto->usagecount;

			/*join of i_first_def and i_last_use */
			if (lto->i_first_def < lt->i_first_def) {
				lt->i_first_def = lto->i_first_def;
			}
			if (lto->i_last_use > lt->i_last_use) {
				lt->i_last_use = lto->i_last_use;
			}
		}
	}
	return true;
}

/* join instack of Basic Block b_index with outstack of predecessors */
void lsra_join_lifetimes(jitdata *jd,int b_index) {
	methodinfo *m;
	lsradata *ls;
	struct stackelement *in, *i, *out;
    struct _list *pred;

	m  = jd->m;
	ls = jd->ls;

	/* do not join instack of Exception Handler */
	if (m->basicblocks[b_index].type == basicblock::TYPE_EXH)
		return;
	in=m->basicblocks[b_index].instack;
	/* do not join first instack element of a subroutine header */
	if (m->basicblocks[b_index].type == basicblock::TYPE_SBR)
		in=in->prev;

	if (in != NULL) {
		for (pred = ls->pred[b_index]; pred != NULL; pred = pred->next) {
			out = m->basicblocks[pred->value].outstack;
			for (i=in; (i != NULL); i = i->prev, out=out->prev) {
				lsra_join_ss(ls, i, out, JOIN_BB);
			}
		}
	}
}

struct stackslot *lsra_make_ss(stackelement_t* s, int bb_index)
{
	struct stackslot *ss;

	ss = DNEW(struct stackslot);
	ss->bb = bb_index;
	ss->s  = s;
	return ss;
}

void lsra_add_ss(struct lifetime *lt, stackelement_t* s) {
	struct stackslot *ss;

	/* Stackslot not in list? */
	if (s->varnum != lt->v_index) {
		ss = DNEW(struct stackslot);
		ss->s = s;
		ss->s->varnum = lt->v_index;
		ss->next = lt->local_ss;
		lt->local_ss = ss;
		if (s != NULL)
			lt->savedvar |= s->flags & SAVEDVAR;
		if (s != NULL)
			lt->type = s->type;
	}
}

struct lifetime *get_ss_lifetime(lsradata *ls, stackelement_t* s) {
	struct lifetime *n;

	if (s->varnum >= 0) { /* new stackslot lifetime */
#ifdef LSRA_DEBUG_CHECK_VERBOSE
		if (-ls->v_index - 1 >= ls->maxlifetimes) {
			printf("%i %i\n", -ls->v_index - 1, ls->maxlifetimes);
		}
#endif
		_LSRA_ASSERT(-ls->v_index - 1 < ls->maxlifetimes);

		n = &(ls->lifetime[-ls->v_index - 1]);
		n->type = s->type;
		n->v_index = ls->v_index--;
		n->usagecount = 0;

		n->bb_last_use  = -1;
		n->bb_first_def = -1;
		n->i_last_use   = -2; /* At -1 param init happens, so -2 is below all
								 possible instruction indices */
		n->i_first_def  = INT_MAX;
		n->local_ss = NULL;
		n->savedvar = 0;
		n->flags    = 0;
	} else
		n = &(ls->lifetime[-s->varnum - 1]);

	lsra_add_ss( n, s);
	return n;
}

#define IS_TEMP_VAR(s) (((s)->varkind != ARGVAR) && ((s)->varkind != LOCALVAR))

#define lsra_join_3_stack(ls, dst, src1, src2, join_type) \
	if ( IS_TEMP_VAR(dst) ) { \
		join_ret = false; \
		if ( IS_TEMP_VAR(src1) && ((src1)->type == (dst)->type)) { \
			join_ret = lsra_join_ss(ls, dst, src1, join_type);								\
		} \
		if ((!join_ret) && IS_TEMP_VAR(src2) && ((src2)->type == (dst)->type)) { \
			lsra_join_ss(ls, dst, src2, join_type); \
		} \
	}

#define lsra_join_2_stack(ls, dst, src, join_type) \
	if ( IS_TEMP_VAR(dst) ) { \
		if ( (IS_TEMP_VAR(src)) && ((src)->type == (dst)->type)) {	\
			lsra_join_ss(ls, dst, src, join_type); \
		} \
	}

#define lsra_join_dup(ls, s1, s2, s3) {				   \
		if (IS_TEMP_VAR(s1)) {						   \
			join_ret = false;						   \
			if (IS_TEMP_VAR(s2))				       \
				join_ret = lsra_join_ss(ls, s1, s2, JOIN); \
			                         /* undangerous join!*/\
			if (IS_TEMP_VAR(s3)) {				       \
				if (join_ret)	/* first join succesfull -> second of type */ \
					            /* JOIN_DUP */			\
                    lsra_join_ss(ls, s1, s3, JOIN_DUP); \
				else									\
	                lsra_join_ss(ls, s1, s3, JOIN); /* first join did not */ \
				                          /* happen -> second undangerous */ \
		    }									        \
		}										        \
        if (IS_TEMP_VAR(s2) && IS_TEMP_VAR(s3))	        \
	        lsra_join_ss(ls, s2, s3, JOIN_DUP);		    \
	}

#define lsra_new_stack(ls, s, block, instr) \
	if ((s)->varkind != ARGVAR) _lsra_new_stack(ls, s, block, instr, LSRA_STORE)
void _lsra_new_stack(lsradata *ls, stackelement_t* s, int block, int instr, int store)
{
	struct lifetime *n;

	if (s->varkind == LOCALVAR) {
		lsra_usage_local(ls, s->varnum, s->type, block, instr, LSRA_STORE);
	} else /* if (s->varkind != ARGVAR) */ {

		n=get_ss_lifetime(ls, s);

		if (store == LSRA_BB_IN)
			n->flags |= JOIN_BB;
		/* remember first def -> overwrite everytime */
		n->bb_first_def = ls->sorted_rev[block];
		n->i_first_def = ls->icount_block[ls->sorted_rev[block]] + instr;

		n->usagecount+=ls->nesting[ls->sorted_rev[block]];
	}
}

#define lsra_from_stack(ls, s, block, instr) \
	if ((s)->varkind != ARGVAR) _lsra_from_stack(ls, s, block, instr, LSRA_LOAD)
#define lsra_pop_from_stack(ls, s, block, instr) \
	if ((s)->varkind != ARGVAR) _lsra_from_stack(ls, s, block, instr, LSRA_POP)
void _lsra_from_stack(lsradata *ls, stackelement_t* s, int block, int instr, int store)
{
	struct lifetime *n;

	if (s->varkind == LOCALVAR) {
		lsra_usage_local(ls, s->varnum, s->type, block, instr, LSRA_LOAD);
	} else /* if (s->varkind != ARGVAR) */ {
		if (s->varkind == STACKVAR )
			/* No STACKVARS possible with lsra! */
			s->varkind = TEMPVAR;

		n=get_ss_lifetime(ls, s);

		if (store == LSRA_BB_OUT)
			n->flags |= JOIN_BB;
		if (n->flags & JOINING)
			n->flags &= ~JOINING;
		n->usagecount+=ls->nesting[ls->sorted_rev[block]];

		/* remember last USE, so only write, if USE Field is undefined (==-1) */
		if (n->bb_last_use == -1) {
			n->bb_last_use = ls->sorted_rev[block];
			n->i_last_use = ls->icount_block[ls->sorted_rev[block]] + instr;
		}
	}
}

void lsra_usage_local(lsradata *ls, s4 v_index, int type, int block, int instr,
					  int store)
{
	struct lifetime *n;

	n = &(ls->lifetime[ ls->maxlifetimes + v_index * (TYPE_ADR+1) + type]);

	if (n->type == -1) { /* new local lifetime */
		n->local_ss=NULL;
		n->v_index=v_index;
		n->type=type;
		n->savedvar = SAVEDVAR;
		n->flags = 0;
		n->usagecount = 0;

		n->bb_last_use = -1;
		n->bb_first_def = -1;
		n->i_last_use = -2;
		n->i_first_def = INT_MAX;
	}
	n->usagecount+=ls->nesting[ls->sorted_rev[block]];
	/* add access at (block, instr) to instruction list */
	/* remember last USE, so only write, if USE Field is undefined (==-1)   */
	/* count store as use, too -> defined and not used vars would overwrite */
	/* other vars */
	if (n->bb_last_use == -1) {
		n->bb_last_use = ls->sorted_rev[block];
		n->i_last_use = ls->icount_block[ls->sorted_rev[block]] + instr;
	}
	if (store == LSRA_STORE) {
		/* store == LSRA_STORE, remember first def -> overwrite everytime */
		n->bb_first_def = ls->sorted_rev[block];
		n->i_first_def = ls->icount_block[ls->sorted_rev[block]] + instr;
	}
}

#ifdef LSRA_DEBUG_VERBOSE
void lsra_dump_stack(stackelement_t* s)
{
	while (s!=NULL) {
		printf("%p(R%3i N%3i K%3i T%3i F%3i) ",(void *)s,s->regoff, s->varnum,
			   s->varkind, s->type, s->flags);
		s=s->prev;
	}
	printf("\n");
}
#endif


void lsra_scan_registers_canditates(jitdata *jd, int b_index)
{
/* 	methodinfo         *lm; */
	builtintable_entry *bte;
	methoddesc         *md;
	int i;
	int opcode;
	int iindex;
	stackelement_t*    src;
	stackelement_t*    dst;
	instruction *iptr;
	bool join_ret; /* for lsra_join* Macros */
	methodinfo *m;
	lsradata *ls;

	m  = jd->m;
	ls = jd->ls;

	/* get instruction count for BB and remember the max instruction count */
	/* of all BBs */
	iindex = m->basicblocks[b_index].icount - 1;

	src = m->basicblocks[b_index].instack;
	if (m->basicblocks[b_index].type != basicblock::TYPE_STD) {
		lsra_new_stack(ls, src, b_index, 0);
		src = src->prev;
	}
	for (;src != NULL; src=src->prev) {
/*******************************************************************************
Check this - ? For every incoming Stack Slot a lifetime has to be created ?
*******************************************************************************/
			_lsra_new_stack(ls, src, b_index, 0, LSRA_BB_IN);
	}
	src = m->basicblocks[b_index].outstack;
	for (;src != NULL; src=src->prev) {
			_lsra_from_stack(ls, src, b_index, iindex, LSRA_BB_OUT);
	}

	/* set iptr to last instruction of BB */
	iptr = m->basicblocks[b_index].iinstr + iindex;

	for (;iindex >= 0; iindex--, iptr--)  {

		/* get source and destination Stack for the current instruction     */
		/* destination stack is available as iptr->dst                      */

		dst = iptr->dst;

		/* source stack is either the destination stack of the previos      */
		/* instruction, or the basicblock instack for the first instruction */

		if (iindex) /* != 0 is > 0 here, since iindex ist always >= 0 */
			src=(iptr-1)->dst;
		else
			src=m->basicblocks[b_index].instack;
		opcode = iptr->opc;


		switch (opcode) {

			/* pop 0 push 0 */
		case ICMD_RET:
			/* local read (return adress) */
			lsra_usage_local(ls, iptr->op1, TYPE_ADR, b_index, iindex,
							 LSRA_LOAD);
			break;
		case ICMD_NOP:
/* 		case ICMD_ELSE_ICONST: */
		case ICMD_CHECKNULL:
		case ICMD_JSR:
		case ICMD_RETURN:
		case ICMD_GOTO:
		case ICMD_PUTSTATICCONST:
		case ICMD_INLINE_START:
		case ICMD_INLINE_END:
		case ICMD_INLINE_GOTO:
			break;

		case ICMD_IINC:
			/* local = local+<const> */
			lsra_usage_local(ls, iptr->op1, TYPE_INT, b_index, iindex,
							 LSRA_LOAD);
			lsra_usage_local(ls, iptr->op1, TYPE_INT, b_index, iindex,
							 LSRA_STORE);
			break;

			/* pop 0 push 1 const: const->stack */
		case ICMD_ICONST:
		case ICMD_LCONST:
		case ICMD_FCONST:
		case ICMD_DCONST:
		case ICMD_ACONST:
			/* new stack slot */
			lsra_new_stack(ls, dst, b_index, iindex);
			break;

			/* pop 0 push 1 load: local->stack */
		case ICMD_ILOAD:
		case ICMD_LLOAD:
		case ICMD_FLOAD:
		case ICMD_DLOAD:
		case ICMD_ALOAD:
			if (dst->varkind != LOCALVAR) {
				/* local->value on stack */
				lsra_usage_local(ls, iptr->op1, opcode - ICMD_ILOAD, b_index,
								 iindex, LSRA_LOAD);
				lsra_new_stack(ls, dst, b_index, iindex); /* value->stack */
			} else /* if (dst->varnum != iptr->op1) */ {
				/* local -> local */
				lsra_usage_local(ls, iptr->op1, opcode - ICMD_ILOAD, b_index,
								 iindex,LSRA_LOAD); /* local->value */
				lsra_usage_local(ls, dst->varnum, opcode - ICMD_ILOAD, b_index,
								 iindex, LSRA_STORE); /* local->value */
			}

			break;

			/* pop 2 push 1 */
			/* Stack(arrayref,index)->stack */
		case ICMD_IALOAD:
		case ICMD_LALOAD:
		case ICMD_FALOAD:
		case ICMD_DALOAD:
		case ICMD_AALOAD:

		case ICMD_BALOAD:
		case ICMD_CALOAD:
		case ICMD_SALOAD:
			/* stack->index */
			lsra_from_stack(ls, src, b_index, iindex);
			/* stack->arrayref */
			lsra_from_stack(ls, src->prev, b_index, iindex);
			/* arrayref[index]->stack */
			lsra_new_stack(ls, dst, b_index, iindex);
			break;

			/* pop 3 push 0 */
			/* stack(arrayref,index,value)->arrayref[index]=value */
		case ICMD_IASTORE:
		case ICMD_LASTORE:
		case ICMD_FASTORE:
		case ICMD_DASTORE:
		case ICMD_AASTORE:

		case ICMD_BASTORE:
		case ICMD_CASTORE:
		case ICMD_SASTORE:

			lsra_from_stack(ls, src,b_index, iindex); /* stack -> value */
			lsra_from_stack(ls, src->prev, b_index, iindex); /* stack -> index*/
			/* stack -> arrayref */
			lsra_from_stack(ls, src->prev->prev, b_index, iindex);
			break;

			/* pop 1 push 0 store: stack -> local */
		case ICMD_ISTORE:
		case ICMD_LSTORE:
		case ICMD_FSTORE:
		case ICMD_DSTORE:
		case ICMD_ASTORE:
			if (src->varkind != LOCALVAR) {
				lsra_from_stack(ls, src, b_index, iindex); /* stack -> value */
				lsra_usage_local(ls, iptr->op1, opcode-ICMD_ISTORE, b_index,
								 iindex, LSRA_STORE); /* local->value */
			} else /* if (src->varnum != iptr->op1) */ {
				lsra_usage_local(ls, iptr->op1, opcode-ICMD_ISTORE, b_index,
								 iindex, LSRA_STORE); /* local->value */
				lsra_usage_local(ls, src->varnum, opcode-ICMD_ISTORE, b_index,
								 iindex, LSRA_LOAD); /* local->value */
			}
			break;

			/* pop 1 push 0 */
		case ICMD_POP: /* throw away a stackslot */
			/* TODO: check if used anyway (DUP...) and change codegen to */
			/*       ignore this stackslot */
			lsra_pop_from_stack(ls, src, b_index, iindex);
			break;

			/* pop 1 push 0 */
		case ICMD_IRETURN:
		case ICMD_LRETURN:
		case ICMD_FRETURN:
		case ICMD_DRETURN:
		case ICMD_ARETURN: /* stack(value) -> [empty]    */

		case ICMD_ATHROW:  /* stack(objref) -> undefined */

		case ICMD_PUTSTATIC: /* stack(value) -> static_field */
		case ICMD_PUTFIELDCONST:

			/* pop 1 push 0 branch */
		case ICMD_IFNULL: /* stack(value) -> branch? */
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
			lsra_from_stack(ls, src, b_index, iindex); /* stack -> value */
			break;

			/* pop 2 push 0 */
		case ICMD_POP2: /* throw away 2 stackslots */
			/* TODO: check if used anyway (DUP...) and change codegen to */
			/*       ignore this stackslot */
			lsra_pop_from_stack(ls, src, b_index, iindex);
			lsra_pop_from_stack(ls, src->prev, b_index, iindex);
			break;

			/* pop 2 push 0 branch */

		case ICMD_IF_ICMPEQ: /* stack (v1,v2) -> branch(v1,v2) */
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
		case ICMD_PUTFIELD: /* stack(objref,value) -> objref = value */

		case ICMD_IASTORECONST:
		case ICMD_LASTORECONST:
		case ICMD_AASTORECONST:
		case ICMD_BASTORECONST:
		case ICMD_CASTORECONST:
		case ICMD_SASTORECONST:
			lsra_from_stack(ls, src, b_index, iindex); 	   /* stack -> value*/
			lsra_from_stack(ls, src->prev, b_index, iindex);
			break;

			/* pop 0 push 1 dup */
		case ICMD_DUP: /* src == dst->prev, src -> dst */
			/* lsra_from_stack(ls, src,b_index,iindex);*/
			lsra_new_stack(ls, dst, b_index, iindex);

#ifdef JOIN_DUP_STACK
			/* src is identical to dst->prev */
			lsra_join_2_stack(ls, src, dst, JOIN_DUP);
#endif
			break;

			/* pop 0 push 2 dup */
		case ICMD_DUP2:
			/* lsra_from_stack(ls, src,b_index, iindex); */
			/* lsra_from_stack(ls, src->prev, b_index, iindex); */
			lsra_new_stack(ls, dst->prev, b_index, iindex);
			lsra_new_stack(ls, dst, b_index, iindex);

#ifdef JOIN_DUP_STACK
			lsra_join_2_stack(ls, src, dst, JOIN_DUP);
			lsra_join_2_stack(ls, src->prev, dst->prev, JOIN_DUP);
			/* src is identical to dst->prev->prev */
			/* src->prev is identical to dst->prev->prev->prev */
#endif
			break;

			/* pop 2 push 3 dup */
		case ICMD_DUP_X1:
			lsra_from_stack(ls, src, b_index, iindex+1);
			lsra_from_stack(ls, src->prev, b_index, iindex+1);
			lsra_new_stack(ls, dst->prev->prev, b_index, iindex);
			lsra_new_stack(ls, dst->prev, b_index, iindex);
			lsra_new_stack(ls, dst, b_index, iindex);
#ifdef JOIN_DUP_STACK
			lsra_join_dup(ls, src, dst, dst->prev->prev);
			lsra_join_2_stack(ls, src->prev, dst->prev, JOIN);
#endif
			break;

			/* pop 3 push 4 dup */
		case ICMD_DUP_X2:
			lsra_from_stack(ls, src,b_index, iindex+1);
			lsra_from_stack(ls, src->prev, b_index, iindex+1);
			lsra_from_stack(ls, src->prev->prev, b_index, iindex+1);
			lsra_new_stack(ls, dst->prev->prev->prev, b_index, iindex);
			lsra_new_stack(ls, dst->prev->prev, b_index, iindex);
			lsra_new_stack(ls, dst->prev, b_index, iindex);
			lsra_new_stack(ls, dst, b_index, iindex);

#ifdef JOIN_DUP_STACK
			lsra_join_dup(ls, src, dst, dst->prev->prev->prev);
			lsra_join_2_stack(ls, src->prev, dst->prev, JOIN);
			lsra_join_2_stack(ls, src->prev->prev, dst->prev->prev, JOIN);
#endif
			break;

			/* pop 3 push 5 dup */
		case ICMD_DUP2_X1:
			lsra_from_stack(ls, src, b_index, iindex+1);
			lsra_from_stack(ls, src->prev, b_index, iindex+1);
			lsra_from_stack(ls, src->prev->prev, b_index, iindex+1);
			lsra_new_stack(ls, dst->prev->prev->prev->prev, b_index, iindex);
			lsra_new_stack(ls, dst->prev->prev->prev, b_index, iindex);
			lsra_new_stack(ls, dst->prev->prev, b_index, iindex);
			lsra_new_stack(ls, dst->prev, b_index, iindex);
			lsra_new_stack(ls, dst, b_index, iindex);

#ifdef JOIN_DUP_STACK
			lsra_join_dup(ls, src, dst, dst->prev->prev->prev);
			lsra_join_dup(ls, src->prev, dst->prev,
						  dst->prev->prev->prev->prev);
			lsra_join_2_stack(ls, src->prev->prev, dst->prev->prev, JOIN);
#endif
			break;

			/* pop 4 push 6 dup */
		case ICMD_DUP2_X2:
			lsra_from_stack(ls, src, b_index, iindex+1);
			lsra_from_stack(ls, src->prev, b_index, iindex+1);
			lsra_from_stack(ls, src->prev->prev, b_index, iindex+1);
			lsra_from_stack(ls, src->prev->prev->prev, b_index, iindex+1);
			lsra_new_stack(ls, dst->prev->prev->prev->prev->prev, b_index,
						   iindex);
			lsra_new_stack(ls, dst->prev->prev->prev->prev, b_index, iindex);
			lsra_new_stack(ls, dst->prev->prev->prev, b_index, iindex);
			lsra_new_stack(ls, dst->prev->prev, b_index, iindex);
			lsra_new_stack(ls, dst->prev, b_index, iindex);
			lsra_new_stack(ls, dst, b_index, iindex);

#ifdef JOIN_DUP_STACK
			lsra_join_dup(ls, src, dst, dst->prev->prev->prev->prev);
			lsra_join_dup(ls, src->prev, dst->prev,
						  dst->prev->prev->prev->prev->prev);
			lsra_join_2_stack(ls, src->prev->prev, dst->prev->prev, JOIN);
			lsra_join_2_stack(ls, src->prev->prev->prev, dst->prev->prev->prev,
							  JOIN);
#endif
			break;

			/* pop 2 push 2 swap */
		case ICMD_SWAP:
			lsra_from_stack(ls, src, b_index, iindex+1);
			lsra_from_stack(ls, src->prev, b_index, iindex+1);
			lsra_new_stack(ls, dst->prev, b_index, iindex);
			lsra_new_stack(ls, dst, b_index, iindex);

			lsra_join_2_stack(ls, src->prev, dst, JOIN);
			lsra_join_2_stack(ls, src, dst->prev, JOIN);

			break;

			/* pop 2 push 1 */

		case ICMD_LADD:
		case ICMD_LSUB:
		case ICMD_LMUL:

		case ICMD_LOR:
		case ICMD_LAND:
		case ICMD_LXOR:

		case ICMD_LSHL:
		case ICMD_LSHR:
		case ICMD_LUSHR:

		case ICMD_IADD:
		case ICMD_IMUL:

		case ICMD_ISHL:
		case ICMD_ISHR:
		case ICMD_IUSHR:
		case ICMD_IAND:
		case ICMD_IOR:
		case ICMD_IXOR:


		case ICMD_FADD:
		case ICMD_FSUB:
		case ICMD_FMUL:

		case ICMD_DADD:
		case ICMD_DSUB:
		case ICMD_DMUL:
		case ICMD_DDIV:
		case ICMD_DREM:
			lsra_from_stack(ls, src, b_index, iindex);
			lsra_from_stack(ls, src->prev, b_index, iindex);
			lsra_new_stack(ls, dst, b_index, iindex);
#ifdef JOIN_DEST_STACK
			lsra_join_3_stack(ls, dst, src->prev, src, JOIN_OP);
#endif
			break;

		case ICMD_ISUB:
			lsra_from_stack(ls, src, b_index, iindex);
			lsra_from_stack(ls, src->prev,b_index,iindex);
			lsra_new_stack(ls, dst, b_index, iindex);
#ifdef JOIN_DEST_STACK
			lsra_join_2_stack(ls, src, dst, JOIN_OP);
#endif
			break;

		case ICMD_LDIV:
		case ICMD_LREM:

		case ICMD_IDIV:
		case ICMD_IREM:

		case ICMD_FDIV:
		case ICMD_FREM:

		case ICMD_LCMP:
		case ICMD_FCMPL:
		case ICMD_FCMPG:
		case ICMD_DCMPL:
		case ICMD_DCMPG:
			lsra_from_stack(ls, src, b_index, iindex);
			lsra_from_stack(ls, src->prev, b_index, iindex);
			lsra_new_stack(ls, dst, b_index, iindex);
			break;

			/* pop 1 push 1 */
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

/* 		case ICMD_IFEQ_ICONST: */
/* 		case ICMD_IFNE_ICONST: */
/* 		case ICMD_IFLT_ICONST: */
/* 		case ICMD_IFGE_ICONST: */
/* 		case ICMD_IFGT_ICONST: */
/* 		case ICMD_IFLE_ICONST: */

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
			lsra_from_stack(ls, src, b_index, iindex);
			lsra_new_stack(ls, dst, b_index, iindex);
#ifdef JOIN_DEST_STACK
			lsra_join_2_stack(ls, src, dst, JOIN_OP);
#endif
			break;

			/* TODO: check if for these ICMDs JOIN_DEST_STACK works, too! */
		case ICMD_ARRAYLENGTH:
		case ICMD_INSTANCEOF:

		case ICMD_NEWARRAY:
		case ICMD_ANEWARRAY:

		case ICMD_GETFIELD:
			lsra_from_stack(ls, src, b_index, iindex);
			lsra_new_stack(ls, dst, b_index, iindex);
			break;

			/* pop 0 push 1 */
		case ICMD_GETSTATIC:

		case ICMD_NEW:
			lsra_new_stack(ls, dst, b_index, iindex);
			break;

			/* pop many push any */

		case ICMD_INVOKESTATIC:
		case ICMD_INVOKESPECIAL:
		case ICMD_INVOKEVIRTUAL:
		case ICMD_INVOKEINTERFACE:
			INSTRUCTION_GET_METHODDESC(iptr,md);
			i = md->paramcount;
			while (--i >= 0) {
				lsra_from_stack(ls, src, b_index, iindex);
				src = src->prev;
			}
			if (md->returntype.type != TYPE_VOID)
				lsra_new_stack(ls, dst, b_index, iindex);
			break;


		case ICMD_BUILTIN:
			bte = iptr->val.a;
			md = bte->md;
			i = md->paramcount;
			while (--i >= 0) {
				lsra_from_stack(ls, src, b_index, iindex);
				src = src->prev;
			}
			if (md->returntype.type != TYPE_VOID)
				lsra_new_stack(ls, dst, b_index, iindex);
			break;

		case ICMD_MULTIANEWARRAY:
			i = iptr->op1;
			while (--i >= 0) {
				lsra_from_stack(ls, src, b_index, iindex);
				src = src->prev;
			}
			lsra_new_stack(ls, dst, b_index, iindex);
			break;

		default:
			exceptions_throw_internalerror("Unknown ICMD %d during register allocation",
										   iptr->opc);
			return;
		} /* switch */
	}
}
#endif /* defined(LV) */


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
