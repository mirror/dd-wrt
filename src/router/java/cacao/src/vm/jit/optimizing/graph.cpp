/* src/vm/jit/optimizing/graph.cpp - CFG

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

#include <stdlib.h>

#include "config.h"

#include "mm/dumpmemory.hpp"

#include "toolbox/bitvector.hpp"

#include "vm/jit/jit.hpp"

#include "vm/jit/optimizing/lsra.hpp"
#include "vm/jit/optimizing/ssa.hpp"
#include "vm/jit/optimizing/graph.hpp"

#ifdef GRAPH_DEBUG_VERBOSE
#include "vm/options.hpp"
#endif

/* Helpers for graph_make_cfg */
void graph_add_cfg( jitdata *jd, graphdata *gd, basicblock *, basicblock *);
void graph_add_exceptions(jitdata *jd, graphdata *gd);

void graph_add_edge( graphdata *gd, int from, int to );

/* Helper for graph_get_first_* */
int graph_get_first_(graph_element *ge, graphiterator *i);
void transform_CFG(jitdata *, graphdata *);

void graph_phi_moves(jitdata *jd, basicblock *bptr, basicblock *dst_goto);

#ifdef GRAPH_DEBUG_VERBOSE
void graph_print(lsradata *ls, graphdata *gd);
#endif


graphdata *graph_init(int basicblockcount) {
	graphdata *gd;
	int i;

	gd = NEW(graphdata);
#ifdef GRAPH_DEBUG_CHECK
	/* Remember basicblockcount, so Array Bound checks can be made */
	gd->basicblockcount = basicblockcount;
#endif

	gd->num_succ = DMNEW(int, basicblockcount);
	gd->successor = DMNEW(graph_element *, basicblockcount);

	gd->num_pred = DMNEW(int, basicblockcount);
	gd->predecessor = DMNEW(graph_element *, basicblockcount);

	for(i = 0; i < basicblockcount; i++) {
		gd->num_succ[i] = gd->num_pred[i] = 0;
		gd->successor[i] = NULL;
		gd->predecessor[i] = NULL;
	}
	return gd;
}

int graph_get_num_predecessor(graphdata *gd, int b_index) {
	_GRAPH_CHECK_BOUNDS(b_index, 0, gd->basicblockcount);
	return gd->num_pred[b_index];
}

int graph_get_num_successor(graphdata *gd, int b_index) {
	_GRAPH_CHECK_BOUNDS(b_index, 0, gd->basicblockcount);
	return gd->num_succ[b_index];
}

int graph_get_first_successor(graphdata *gd, int b_index, graphiterator *i) {
	_GRAPH_CHECK_BOUNDS(b_index, 0, gd->basicblockcount);
	return graph_get_first_(gd->successor[b_index], i);
}

int graph_get_first_predecessor(graphdata *gd, int b_index, graphiterator *i) {
	_GRAPH_CHECK_BOUNDS(b_index, 0, gd->basicblockcount);
	return graph_get_first_(gd->predecessor[b_index], i);
}

int graph_get_next(graphiterator *i) {
	if ((*i) == NULL)
		return -1;
	(*i) = (*i)->next;
	if ( (*i) == NULL )
		return -1;
	else
		return (*i)->value;
}

int graph_get_first_(graph_element *ge, graphiterator *i) {
	if ( ((*i) = ge) == NULL)
		return -1;
	else
		return (*i)->value;
}

bool graph_has_multiple_predecessors( graphdata *gd, int b_index) {
	_GRAPH_CHECK_BOUNDS(b_index, 0, gd->basicblockcount);
	return (gd->num_pred[b_index] > 1);
}

bool graph_has_multiple_successors( graphdata *gd, int b_index) {
	_GRAPH_CHECK_BOUNDS(b_index, 0, gd->basicblockcount);
	return (gd->num_succ[b_index] > 1);
}

void graph_add_edge( graphdata *gd, int from, int to ) {
	graphiterator i;
	graph_element *n;
	int b;

	/* prevent multiple similar edges from TABLESWITCH and */
	/* LOOKUPSWITCH */
	b = graph_get_first_successor(gd, from, &i);
	for( ; (b != -1) && ( b != to); b = graph_get_next(&i));
	if (b != -1) /* edge from->to already existing */
		return;

	/* We need two new graph_elements. One for successors and one for */
	/* predecessors */
	n = DMNEW(graph_element, 2);

	n->value=to;
	n->next=gd->successor[from];
	gd->successor[from]=n;
	gd->num_succ[from]++;

	n++; /* take next allocated graph_element */
	n->value=from;
	n->next=gd->predecessor[to];
	gd->predecessor[to]=n;
	gd->num_pred[to]++;
}

/* split the edge from BB from shown by iterator i with new_block */
void graph_split_edge(graphdata *gd, int from, graphiterator *i, int new_block) {
	graphiterator i_pred;
	graph_element *n;
	int l, succ;

	/* i->value is the BB index of the "old" successor */

	succ = (*i)->value;

	/* search for iterator showing predecessor edge from BB succ back to */
	/* from */

	l = graph_get_first_predecessor(gd, succ, &i_pred);
	for(; (l != -1) && (l != from); l = graph_get_next(&i_pred));
	_GRAPH_ASSERT(l == from);

	/* change CFG entries */

	(*i)->value = new_block;
	i_pred->value = new_block;

	/* and insert the CFG successor and predecesser entries for new_block */
	/* 2 entries needed */

	n = DMNEW(graph_element, 2);

	/* make the successor entry for new_block */

	n->value = succ;
	n->next = gd->successor[new_block];
	gd->successor[new_block] = n;
	gd->num_succ[new_block]++;

	/* make the predecessor entry for new_block */

	n++;
	n->value = from;
	n->next = gd->predecessor[new_block];
	gd->predecessor[new_block] = n;
	gd->num_pred[new_block]++;
}

/***********************************************
Generate the Control Flow Graph
( pred,succ,num_pred of lsradata structure)
************************************************/
void graph_make_cfg(jitdata *jd,graphdata *gd) {
	instruction *iptr;
	lookup_target_t *lookup;
	branch_target_t *table;
	int len, i, l;
	methodinfo *m;
	codegendata *cd;
	registerdata *rd;
	lsradata *ls;
	basicblock *bptr;

	m = jd->m;
	cd = jd->cd;
	rd = jd->rd;
	ls = jd->ls;

	/* Add edge from new Basic Block 0 (parameter initialization) */
	graph_add_edge(gd, 0, 1);

	for (bptr = jd->basicblocks; bptr != NULL; bptr = bptr->next) {
		if (bptr->state >= basicblock::REACHED) {
			if ((len = bptr->icount)) {
				/* set iptr to last non NOP instruction	*/
				iptr = bptr->iinstr + bptr->icount -1;
				while ((len>0) && (iptr->opc == ICMD_NOP)) {
					len--;
					iptr--;
				}
				/* block contains instructions	*/
				switch (iptr->opc) {		/* check type of last instruction */
				case ICMD_RETURN:
				case ICMD_IRETURN:
				case ICMD_LRETURN:
				case ICMD_FRETURN:
				case ICMD_DRETURN:
				case ICMD_ARETURN:
				case ICMD_ATHROW:
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
					/* Add branch target */
					graph_add_cfg(jd, gd, bptr, iptr->dst.block);
					/* Add fall through path */
					graph_add_cfg(jd, gd, bptr, bptr->next);
					break;


				case ICMD_GOTO:
					graph_add_cfg(jd, gd, bptr,  iptr->dst.block);
					break;					/* visit branch (goto) target	*/

				case ICMD_TABLESWITCH:		/* switch statement				*/
					table = iptr->dst.table;
					l = iptr->sx.s23.s2.tablelow;
					i = iptr->sx.s23.s3.tablehigh;
					i = i - l + 1;

					/* Add default target */

					graph_add_cfg(jd, gd, bptr,  table[0].block);
					table += i;

					while (--i >= 0) {
						graph_add_cfg(jd, gd, bptr,  table->block);
						--table;
					}
					break;

				case ICMD_LOOKUPSWITCH:		/* switch statement				*/
					lookup = iptr->dst.lookup;
					i = iptr->sx.s23.s2.lookupcount;

					while (--i >= 0) {
						graph_add_cfg(jd, gd, bptr, lookup->target.block);
						lookup++;
					}

					graph_add_cfg(jd, gd, bptr, iptr->sx.s23.s3.lookupdefault.block);
					break;

				case ICMD_RET:
				case ICMD_JSR:
					assert(0);
					break;

				case ICMD_NOP:
					assert(0);

				default:
					graph_add_cfg(jd, gd, bptr, bptr + 1 );
					break;
			    } /* switch (iptr->opc)*/
		    }     /* if (bptr->icount) */
	    }         /* if (bptr->state >= basicblock::REACHED) */
	}             /* for (bptr = ...; bptr != NULL; bptr = bptr->next) */

 	graph_add_exceptions(jd, gd);
	transform_CFG(jd, gd);

#ifdef GRAPH_DEBUG_VERBOSE
	if (compileverbose)
		graph_print(ls, gd);
#endif
}

/*****************************************************************
add Edges from guarded Areas to Exception handlers in the CFG
*****************************************************************/
void graph_add_exceptions(jitdata *jd, graphdata *gd) {
#if 0
	basicblock *bptr;
	raw_exception_entry *ex;
	codegendata *cd;

	cd = jd->cd;

	/* add cfg edges from all bb of a try block to the start of the according */
	/* exception handler to ensure the right order after depthfirst search    */

	ex=jd->exceptiontable;

#ifdef GRAPH_DEBUG_VERBOSE
	if (compileverbose)
		printf("ExTable(%i): ", jd->exceptiontablelength);
#endif

	for (; ex != NULL; ex = ex->down) {

#ifdef GRAPH_DEBUG_VERBOSE
		if (compileverbose)
			printf("[%i-%i]->%i ",ex->start->nr, ex->end->nr,
				   ex->handler->nr);
#endif

		_GRAPH_ASSERT(ex->handler->nr < jd->new_basicblockcount);
		_GRAPH_ASSERT(ex->handler->state >= basicblock::REACHED);
		_GRAPH_ASSERT(ex->start->nr <= ex->end->nr);

		/* loop all valid Basic Blocks of the guarded area and add CFG edges  */
		/* to the appropriate handler */
		for (bptr = ex->start; (bptr != NULL) && (bptr != ex->end); bptr = bptr->next)
			if (bptr->state >= basicblock::REACHED)
				graph_add_cfg(jd, gd, bptr, ex->handler);

		_GRAPH_ASSERT((bptr != NULL)
					  && ((bptr->state >=basicblock::REACHED) || (bptr == ex->end)));
	}
#ifdef GRAPH_DEBUG_VERBOSE
	if (compileverbose)
		printf("\n");
#endif
#endif
}


/******************************************************************
Add the CFG Edge into next und succ
******************************************************************/
void graph_add_cfg( jitdata *jd, graphdata *gd, basicblock *from,
					basicblock *to) {

	/* ignore Empty, Deleted,... Basic Blocks as target */
	/* TODO: Setup BasicBlock array before to avoid this */
	/*       best together with using the basicblock list, so lsra works */
	/*       with opt_loops, too */

	for (; (to != NULL) && (to->state < basicblock::REACHED); to = to->next);

	_GRAPH_ASSERT(to != NULL);


	/* add one to from and to, so the to be inserted Basic Block 0 is */
	/* already regarded */
	graph_add_edge( gd, from->nr + 1, to->nr + 1);
}


/*****************************************************************
Sort Basic Blocks using Depth First search in reverse post order
In: ls->basicblockcount, ls->basicblocks[], gd->
Out: ls->sorted, ls->sorted_rev
*****************************************************************/

void graph_DFS(lsradata *ls, graphdata *gd) {
	int *stack;
	int *visited;
	int stack_top;
	bool not_finished;
	int i,p,s, num_pred;
	graphiterator iter;

	ls->sorted = DMNEW(int, ls->basicblockcount);
	ls->sorted_rev = DMNEW(int, ls->basicblockcount);

	stack = DMNEW( int, ls->basicblockcount);
	visited = (int *)DMNEW( bool, ls->basicblockcount);
	for (i = 0; i < ls->basicblockcount; i++) {
		visited[i] = 0;
		ls->sorted[i]=-1;
		ls->sorted_rev[i]=-1;
	}

    stack[0] = 0; /* start with Block 0 */
	stack_top = 1;
	/* Start Block is handled right and can be put in sorted */
	visited[0] = graph_get_num_predecessor(gd , 0);
	p = 0;
	not_finished = true;
	while (not_finished) {
		while (stack_top != 0) {
			stack_top--;
			i = stack[stack_top];
			ls->sorted[p]=i;
			p++;
			s = graph_get_first_successor(gd, i, &iter);
			for (; s != -1; s = graph_get_next(&iter)) {
				visited[s]++;
				if (visited[s] ==  graph_get_num_predecessor(gd, s)) {
					/* push the node on the stack, only if all ancestors have */
					/* been visited */
					stack[stack_top] = s;
					stack_top++;
				}
			}
		}
		not_finished = false;
		for (i=1; i < ls->basicblockcount; i++) {
			/* search for visited blocks, which have not reached the num_pre */
			/* and put them on the stack -> happens with backedges */
			num_pred = graph_get_num_predecessor(gd, i);
			if ((visited[i] != 0) && (visited[i] < num_pred)) {
				stack[stack_top] = i;
				stack_top++;
				visited[i] = num_pred;
				not_finished=true;
				break;
			}
		}
	}

	for(i = 0; i < ls->basicblockcount; i++)
		if (ls->sorted[i] != -1)
			 ls->sorted_rev[ls->sorted[i]] = i;
}


void graph_init_basicblock(jitdata *jd, basicblock *bptr, int b_index) {
		bptr->nr         = b_index;
		bptr->icount     = 0;
		bptr->iinstr     = NULL;
		bptr->type       = basicblock::TYPE_STD;
		bptr->state      = basicblock::FINISHED;
		bptr->invars     = NULL;
		bptr->outvars    = NULL;
		bptr->indepth    = 0;
		bptr->outdepth   = 0;
		bptr->branchrefs = NULL;
		bptr->mpc        = -1;
		bptr->next       = NULL;
		bptr->method     = jd->m;
}

/*********************************************************************+
After the "original" CFG has been created, it has to be adopted for
SSA. (inluding insertion of new Basic Blocks - edge splitting)

**********************************************************************/
void transform_CFG(jitdata *jd, graphdata *gd) {
	int i, j, k, n, num_new_blocks;
	int **var_def;
	basicblock *tmp, *bptr;
	s4 *in, *out, *new_in_stack, *new_out_stack;
	graphiterator iter;
	int *num_succ;
	struct graph_element **successor;
	int *num_pred;
	struct graph_element **predecessor;
	methodinfo *m;
	codegendata *cd;
	registerdata *rd;
	lsradata *ls;

	m = jd->m;
	cd = jd->cd;
	rd = jd->rd;
	ls = jd->ls;

	/* with SSA a new Basic Block 0 is inserted for parameter initialisation  */
	/* & look for nodes with multiple successors leading to nodes with        */
	/* multiple predecessor -> if found insert a new block between to split   */
	/* this edge. As first step count how many blocks have to be inserted.    */

	num_new_blocks = 1;
	for(i = 0; i< jd->basicblockcount + 1; i++) {
		if (graph_has_multiple_successors(gd, i)) {
			k = graph_get_first_successor(gd, i, &iter);
			for(; k != -1; k = graph_get_next(&iter)) {
				/* check for successor blocks with more than one       */
				/* predecessor. For each found incremet num_new_blocks */
				if (graph_has_multiple_predecessors(gd, k))
					num_new_blocks++;
			}
		}
	}

	/* increase now basicblockcount accordingly. */
	ls->basicblockcount = jd->basicblockcount + num_new_blocks;

	ls->basicblocks = DMNEW(basicblock *, ls->basicblockcount);
	for(i = 0; i< ls->basicblockcount; i++)
		ls->basicblocks[i] = NULL;

	/* copy Basic Block References to ls->basicblocks */

	for (bptr = jd->basicblocks; bptr != NULL; bptr = bptr->next) {
		_GRAPH_ASSERT(bptr->nr < jd->basicblockcount);
		ls->basicblocks[bptr->nr + 1] = bptr;
		bptr->nr = bptr->nr+1;
	}

	/* Create new Basic Blocks:
	   0, [jd->basicblockcount..ls->basicblockcount[ */
	/* num_new_blocks have to be inserted*/

	tmp = DMNEW( basicblock, num_new_blocks);
	ls->basicblocks[0] = tmp;
	graph_init_basicblock( jd, tmp, 0);
	tmp++;
	ls->basicblocks[0]->next = ls->basicblocks[1];

	if (ls->basicblockcount > jd->basicblockcount + 1) {

		/* new Blocks have to be inserted                   */

		num_succ = DMNEW(int, ls->basicblockcount);
		successor = DMNEW(graph_element *, ls->basicblockcount);

		num_pred = DMNEW(int, ls->basicblockcount);
		predecessor = DMNEW(graph_element *, ls->basicblockcount);

		/* regard the + 1 for the already inserted new BB 0 */
		/* So recreate ls->var_def                          */

		var_def = DMNEW(int *, ls->basicblockcount);
		for(i = 0; i < jd->basicblockcount + 1; i++) {
			var_def[i] = ls->var_def[i];
			num_succ[i] = gd->num_succ[i];
			num_pred[i] = gd->num_pred[i];
			successor[i] = gd->successor[i];
			predecessor[i] = gd->predecessor[i];
		}
		for(i = jd->basicblockcount + 1; i < ls->basicblockcount; i++) {
			var_def[i] = bv_new(jd->varcount);
			graph_init_basicblock( jd, tmp, i);
			ls->basicblocks[i] = tmp;
			tmp++;

			num_succ[i] = 0;
			num_pred[i] = 0;
			successor[i] = NULL;
			predecessor[i] = NULL;
		}
		ls->var_def = var_def;
		gd->num_succ = num_succ;
		gd->num_pred = num_pred;
		gd->successor = successor;
		gd->predecessor = predecessor;
#ifdef GRAPH_DEBUG_CHECK
		/* Remember basicblockcount, so Array Bound checks can be made */
		gd->basicblockcount = ls->basicblockcount;
#endif
	}

	/* Now Split the edges */

	num_new_blocks = jd->basicblockcount + 1; /* first free new block index */
	for(i = 0; i < jd->basicblockcount + 1; i++) {
		if (graph_has_multiple_successors(gd, i)) {/* more than one successor */
			j = graph_get_first_successor( gd, i, &iter);
			for(; j != -1; j = graph_get_next(&iter)) {
				if (graph_has_multiple_predecessors(gd, j)) {
					/* and more than one predecessor -> split edge */
					_GRAPH_ASSERT(num_new_blocks < ls->basicblockcount);

					/* insert new Block num_new_blocks */

					/* splite the edge from BB i to j with the new BB */
					/* num_new_blocks ( i->j --> i->nnb->j)*/
					/* iter_succ shows the edge from i to j */

					graph_split_edge(gd, i, &iter, num_new_blocks);

					ls->basicblocks[num_new_blocks]->indepth =
						ls->basicblocks[i]->outdepth;
					out = ls->basicblocks[i]->outvars;
					ls->basicblocks[num_new_blocks]->invars = in = NULL;

					if (ls->basicblocks[num_new_blocks]->indepth > 0)
						new_in_stack = DMNEW( s4,
									  ls->basicblocks[num_new_blocks]->indepth);
						new_out_stack = DMNEW( s4,
									  ls->basicblocks[num_new_blocks]->indepth);

					for(n=0; n<ls->basicblocks[num_new_blocks]->indepth; n++) {
						new_in_stack[n] = out[n];
						new_out_stack[n] = out[n];
					}
					ls->basicblocks[num_new_blocks]->invars = new_in_stack;

					/* Create Outstack */
					ls->basicblocks[num_new_blocks]->outvars =
						new_out_stack;
					ls->basicblocks[num_new_blocks]->outdepth =
						ls->basicblocks[num_new_blocks]->indepth;

					_GRAPH_ASSERT(ls->basicblocks[num_new_blocks]->outdepth ==
								  ls->basicblocks[j]->indepth );

					num_new_blocks++;
				}
			}
		}
	}
}

void transform_BB(jitdata *jd, graphdata *gd) {
	int n, len;
	int pred, succ;
	basicblock *last_block;
	instruction *iptr;
	graphiterator iter;
	methodinfo *m;
	lsradata *ls;

	m = jd->m;
	ls = jd->ls;

	/* the "real" last Block is always an empty block        */
	/* so take the one before, to insert new blocks after it */
	last_block = &(jd->basicblocks[jd->basicblockcount - 1]);
	_GRAPH_ASSERT(last_block->next->next  == NULL);
	_GRAPH_ASSERT(last_block->next->state <= basicblock::REACHED);
	last_block->next->nr = ls->basicblockcount;

	/* look through new blocks */
	for(n = jd->basicblockcount + 1; n < ls->basicblockcount ; n++) {
		/* if a phi move happens at this block, we need this block */
		/* if not, remove him from the CFG */
		if (ls->num_phi_moves[n] > 0) {
			/* i can only have one predecessor and one successor! */
			_GRAPH_ASSERT( graph_has_multiple_predecessors(gd, n) == false);
			_GRAPH_ASSERT( graph_has_multiple_successors(gd, n) == false);

			succ = graph_get_first_successor(gd, n, &iter);
			pred = graph_get_first_predecessor(gd, n, &iter);

			/* set iptr to last instruction			      */
			len = ls->basicblocks[pred]->icount;
			iptr = ls->basicblocks[pred]->iinstr + len - 1;
			while ((len>0) && (iptr->opc == ICMD_NOP)) {
				len--;
				iptr--;
			}


			/* with JSR there can not be multiple successors  */
			_GRAPH_ASSERT(iptr->opc != ICMD_JSR);
			/* If the return Statment has more successors and  */
			/* one of these has more predecessor, we are in    */
			/* troubles - one would have to insert a new Block */
			/* after the one which executes the ICMD_JSR       */
			/* !!TODO!! if subroutines will not be inlined     */
			_GRAPH_ASSERT(iptr->opc != ICMD_RET);

			/* link new block into basicblocks list */
			/* if edge to split is the "fallthrough" path of the */
			/* conditional, then link the new block inbetween    */
			/* and generate no ICMD */
			/* else if edge to split is the branch, generate a   */
			/* ICMD_GOTO and add new BB at the end of the BB List*/
			if ((ls->basicblocks[pred]->next == ls->basicblocks[succ])
				&& (iptr->opc != ICMD_LOOKUPSWITCH)
				&& (iptr->opc != ICMD_TABLESWITCH)
				&& (iptr->opc != ICMD_GOTO)) {
				/* GOTO, *SWITCH have no fallthrough path */

				/* link into fallthrough path */


				ls->basicblocks[n]->next =
					ls->basicblocks[pred]->next;
				ls->basicblocks[pred]->next =
					ls->basicblocks[n];
#if 1
				/* generate no instructions */
				ls->basicblocks[n]->icount = 1;
				ls->basicblocks[n]->iinstr = NEW(instruction);
				ls->basicblocks[n]->iinstr[0].opc =	ICMD_NOP;
#else
				graph_phi_moves(jd, ls->basicblocks[n], NULL);
#endif
			} else {
				/* Block n is in the Branch path */
				/* link Block at the end */
				ls->basicblocks[n]->next =last_block->next;
				last_block->next = ls->basicblocks[n];
				last_block = ls->basicblocks[n];

				/* change the Branch Target to BB i */

				switch(iptr->opc) {
				case ICMD_LOOKUPSWITCH:
					{
						s4 i;
						lookup_target_t *lookup;

						lookup = iptr->dst.lookup;

						i = iptr->sx.s23.s2.lookupcount;

						while (--i >= 0) {
							if (lookup->target.block == ls->basicblocks[succ])
								/* target found -> change */
								lookup->target.block = ls->basicblocks[n];
							lookup++;
						}

						if (iptr->sx.s23.s3.lookupdefault.block == ls->basicblocks[succ])
							/* target found -> change */
							iptr->sx.s23.s3.lookupdefault.block = ls->basicblocks[n];
					}
					break;
				case ICMD_TABLESWITCH:
					{
						s4 i, l;
						branch_target_t *table;

						table = iptr->dst.table;

						l = iptr->sx.s23.s2.tablelow;
						i = iptr->sx.s23.s3.tablehigh;

						i = i - l + 1;

						if (table[0].block ==  ls->basicblocks[succ]) /* default target */
							/* target found -> change*/
							table[0].block = ls->basicblocks[n];

						table += i;

						while (--i >= 0) {
							if (table->block == ls->basicblocks[succ])
								/* target found -> change */
								table->block = ls->basicblocks[n];
							--table;
						}
					}
					break;
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
				case ICMD_IF_ACMPNE:
				case ICMD_GOTO:
					_GRAPH_ASSERT(iptr->dst.block == ls->basicblocks[succ]);
					iptr->dst.block = ls->basicblocks[n];
					break;
				default:
					/* Exception Edge to split */
					/* not handled by now -> fallback to regalloc */
					exit(1);
				}
#if 1
				/* Generate the ICMD_GOTO */
				ls->basicblocks[n]->icount = 1;
				ls->basicblocks[n]->iinstr =
					DNEW(instruction);
				ls->basicblocks[n]->iinstr->opc =
					ICMD_GOTO;
				ls->basicblocks[n]->iinstr->dst.block =
					ls->basicblocks[succ];
#else
				graph_phi_moves(jd, ls->basicblocks[n], ls->basicblocks[succ]);
#endif
			}
		}
	}
}

/* graph_phi_moves *************************************************************

generate the instruction array for Basicblock n (== ls->basicblocks[n])

IN:
basicblock *bptr       Basicblock to change with ->iinstr == NULL
basicblock *dst_goto   Destination Block for ICMD_GOTO at end of Block, or
                       NULL for no ICMD_GOTO

OUT:
bptr->iinstr           points to a newly allocated instruction array containing
                       the phi moves, the optional ICMD_GOTO at the end.
bptr->icount           Count of instructions in bptr->iinstr

*******************************************************************************/

void graph_phi_moves(jitdata *jd, basicblock *bptr, basicblock *dst_goto) {
	int lt_d,lt_s,i;
	lsradata    *ls;
	instruction *iptr;

	ls = jd->ls;

	_GRAPH_ASSERT(ls->num_phi_moves[bptr->nr] > 0);
	bptr->icount = ls->num_phi_moves[bptr->nr];
	if (dst_goto != NULL)
		bptr->icount++;
	bptr->iinstr = iptr = DMNEW(instruction, bptr->icount);

	_GRAPH_ASSERT(iptr != NULL);

	/* Moves from phi functions with highest indices have to be */
	/* inserted first, since this is the order as is used for   */
	/* conflict resolution */

	for(i = ls->num_phi_moves[bptr->nr] - 1; i >= 0 ; i--) {
		lt_d = ls->phi_moves[bptr->nr][i][0];
		lt_s = ls->phi_moves[bptr->nr][i][1];

#if defined(GRAPH_DEBUG_VERBOSE)
		if (compileverbose)
			printf("graph_phi_moves: BB %3i Move %3i <- %3i\n", bptr->nr,
				   lt_d, lt_s);
#endif
		if (lt_s == jitdata::UNUSED) {
#if defined(SSA_DEBUG_VERBOSE)
			if (compileverbose)
				printf(" ... not processed \n");
#endif
			continue;
		}

		_GRAPH_ASSERT(d->type != -1);
		_GRAPH_ASSERT(s->type == -1);

		iptr->opc = ICMD_MOVE;
		iptr->s1.varindex  = ls->lifetime[lt_s].v_index;
		iptr->dst.varindex = ls->lifetime[lt_d].v_index;
		iptr++;
	}

	if (dst_goto != NULL) {
		iptr->opc = ICMD_GOTO;
		iptr->dst.block = dst_goto;
	}
}

#ifdef GRAPH_DEBUG_VERBOSE
void graph_print(lsradata *ls, graphdata *gd) {
	int i,j;
	graphiterator iter;
	printf("graph_print: basicblockcount %3i\n", ls->basicblockcount);

	printf("CFG: \n");
	for(i = 0; i < ls->basicblockcount; i++) {
		printf("%3i: ",i);
		j = graph_get_first_successor(gd, i, &iter);
		for(; j != -1; j = graph_get_next(&iter)) {
			printf("%3i ",j);
		}
		printf("\n");
	}
	printf("Pred: \n");
	for(i = 0; i < ls->basicblockcount; i++) {
		printf("%3i: ",i);
		j = graph_get_first_predecessor(gd, i, &iter);
		for(; j != -1; j = graph_get_next(&iter)) {
			printf("%3i ", j);
		}
		printf("\n");
	}
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
