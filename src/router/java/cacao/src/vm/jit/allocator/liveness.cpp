/* src/vm/jit/allocator/liveness.c - liveness analysis for lsra

   Copyright (C) 2005, 2006, 2007, 2008
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

#include <limits.h>
#include <stdlib.h>

#include "vm/types.hpp"

#include "mm/memory.hpp"

#include "toolbox/logging.hpp"
#include "toolbox/worklist.hpp"

#include "vm/jit/builtin.hpp"
#include "vm/exceptions.hpp"
#include "vm/global.hpp"
#include "vm/method.hpp"
#include "vm/resolve.hpp"
#include "vm/jit/codegen-common.hpp"
#include "vm/jit/jit.hpp"
#include "vm/jit/allocator/lsra.hpp"
#include "vm/jit/allocator/liveness.hpp"


/* function prototypes */
void liveness_scan_registers_canditates(jitdata *jd, int b_index, int iindex,
										stackelement_t* src, lv_sets *sets);
void liveness_set_stack(lsradata *ls, int block, int g_iindex, stackelement_t* s,
						lv_sets *sets, int op);
void liveness_set_local(lsradata *ls, int block, int g_iindex, s4 v_index,
						int type, lv_sets *sets, int op);

void liveness_add_ss(struct lifetime *lt, stackelement_t* s) {
	struct stackslot *ss;
	/* Stackslot noch nicht eingetragen? */

	if (s->varnum != lt->v_index) {
		ss = DNEW(struct stackslot);
		ss->s = s;
		ss->s->varnum = lt->v_index;
		ss->next = lt->local_ss;
		lt->local_ss = ss;
		if (s != NULL) lt->savedvar |= s->flags & SAVEDVAR;
		if (s != NULL) lt->type = s->type;
	}
}

void liveness_join_ss( struct lsradata *ls, struct stackelement *in,
				   struct stackelement *out) {
	struct lifetime *lt, *lto;
	struct stackslot *ss, *ss_last;


	if (in->varnum != out->varnum) {
		lt = &(ls->lifetime[-in->varnum - 1]);

#ifdef LV_DEBUG_CHECK
		if (lt->type == -1) {
#ifdef LV_DEBUG_VERBOSE
			log_text("liveness_join_ss: lifetime for instack not found\n");
#endif
			assert(0);
		}
#endif

		if (out->varnum >= 0) { /* no lifetime for this slot till now */
			liveness_add_ss(lt, out);
		} else {
			lto = &(ls->lifetime[-out->varnum - 1]);

#ifdef LV_DEBUG_CHECK
			if (lto->type == -1) {
#ifdef LV_DEBUG_VERBOSE
				log_text("liveness_join_ss: lifetime for outstack not found\n");
#endif
				assert(0);
			}
#endif
#ifdef LV_DEBUG_CHECK
			if (lto->type != lt->type) {
#ifdef LV_DEBUG_VERBOSE
				log_text("liveness_join_ss: in/out stack type mismatch\n");
#endif
				assert(0);
			}
#endif

			/* take Lifetime lto out of ls->lifetimes */
			lto->type = -1;

			/* merge lto into lt of in */

			/* change varnums of all lto->local_ss->s to lt->v_index */
			ss_last = ss = lto->local_ss;
			while (ss != NULL) {
				ss_last = ss;
				ss->s->varnum = lt->v_index;
				ss = ss->next;
			}
			/* link both lto->local_ss list to lt->local_ss */
			if (ss_last != NULL) {
				ss_last->next = lt->local_ss;
				lt->local_ss = lto->local_ss;
			}

			lt->savedvar |= lto->savedvar;
			lt->flags |= lto->flags;
			lt->usagecount += lto->usagecount;

			/*join of [bb|i]_first_def und [bb|i]_last_use */
			if (lto->bb_first_def < lt->bb_first_def) {
				lt->bb_first_def = lto->bb_first_def;
				lt->i_first_def = lto->i_first_def;
			} else if ((lto->bb_first_def == lt->bb_first_def) &&
					   ( lto->i_first_def < lt->i_first_def)) {
				lt->i_first_def = lto->i_first_def;
			}
			if (lto->bb_last_use > lt->bb_last_use) {
				lt->bb_last_use = lto->bb_last_use;
				lt->i_last_use = lto->i_last_use;
			} else if ((lto->bb_last_use == lt->bb_last_use) &&
					   ( lto->i_last_use > lt->i_last_use)) {
				lt->i_last_use = lto->i_last_use;
			}
		}
	}
}

/* join instack of Basic Block b_index with outstack of predecessors */
void liveness_join_lifetimes(jitdata *jd, int b_index) {
	struct stackelement *in, *i, *out;
    struct _list *pred;

	lsradata *ls;
	methodinfo *m;

	ls = jd->ls;
	m  = jd->m;

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
				liveness_join_ss(ls, i, out);
			}
		}
	}
}

void liveness_setup(jitdata *jd) {
	int i, icount, b_index;
	stackelement_t* src;
	methodinfo *m;
	lsradata *ls;

	ls = jd->ls;
	m  = jd->m;

	ls->icount_block = DMNEW(int, m->basicblockcount);
	ls->icount_block[0] = icount = 0;
	for(i = 0; i < m->basicblockcount; i++) {
		if (i != 0) {
			/* create a global instruction index in icount_block */
			if (ls->sorted[i-1] != -1)
				icount += m->basicblocks[ ls->sorted[i-1] ].icount;
				ls->icount_block[i] = icount;
		}

		if ((b_index = ls->sorted[i]) != -1) {
			/* 'valid' Basic Block */

			/* adapt in- and outstacks for LSRA */
			src = m->basicblocks[b_index].instack;
			if (m->basicblocks[b_index].type != basicblock::TYPE_STD) {
#ifdef LV_DEBUG_CHECK
				if (src == NULL) {
#ifdef LV_DEBUG_VERBOSE
				log_text("No Incoming Stackslot for Exception/Subroutine BB\n");
#endif
					assert(0);
				}
#endif
				if (src->varkind == STACKVAR)
					src->varkind = TEMPVAR;
				src = src->prev;
			}
			for (;src != NULL; src=src->prev) {
				/* no ARGVAR possible at BB Boundaries with LSRA! */
				/* -> change to TEMPVAR                           */
				if (src->varkind == ARGVAR ) {
					src->varkind = TEMPVAR;
					/* On Architectures with own return registers a return
					   stackslot is marked as varkind=ARGVAR with varnum=-1
					   but for lsra a varkind==TEMPVAR, varnum=-1 would mean,
					   that already a lifetime was allocated! */
					if (src->varnum < 0) src->varnum = 0;
				}
				else if (src->varkind == LOCALVAR ) {
					/* only allowed for top most ss at sbr or exh entries! */
#ifdef LV_DEBUG_VERBOSE
					log_text("LOCALVAR at basicblock instack\n");
#endif
					abort();
				} else {
					if (src->varkind == STACKVAR )
						/* no Interfaces at BB Boundaries with LSRA! */
						/* -> change to TEMPVAR                      */
						src->varkind = TEMPVAR;
				}
			}

			src = m->basicblocks[b_index].outstack;
			for (;src != NULL; src=src->prev) {
				if (src->varkind == ARGVAR ) {
#ifdef LV_DEBUG_VERBOSE
					log_text("ARGVAR at basicblock outstack\n");
#endif
					abort();
				} else if (src->varkind == LOCALVAR ) {
#ifdef LV_DEBUG_VERBOSE
					log_text("LOCALVAR at basicblock outstack\n");
#endif
					abort();
				} else {
					/* no Interfaces at BB Boundaries with LSRA! */
					/* -> change to TEMPVAR                      */
					if (src->varkind == STACKVAR )
						src->varkind = TEMPVAR;
				}
			}
		}
	}
}

void liveness_init(jitdata *jd) {
	int i, b_index, len;
	int lifetimes;
	stackelement_t* src, dst;
	instruction *iptr;
	methodinfo *m;
	lsradata *ls;

	ls = jd->ls;
	m  = jd->m;

	lifetimes = 0;

	for(i = 0; i < m->basicblockcount; i++) {
		if ((b_index = ls->sorted[i]) != -1) {
			/* 'valid' Basic Block */

			/* Scan Number of Stack Lifetimes */
			lifetimes += m->basicblocks[b_index].indepth;

			dst = m->basicblocks[b_index].instack;
			len = m->basicblocks[b_index].icount;
			iptr = m->basicblocks[b_index].iinstr;
			for (;len>0; len--, iptr++) {
				src = dst;
				dst = iptr->dst;

				switch(iptr->opc) {
				case ICMD_SWAP:
				case ICMD_DUP2:
					lifetimes += 2;
					break;
				case ICMD_DUP_X1:
					lifetimes += 3;
					break;
				case ICMD_DUP2_X1:
					lifetimes += 5;
					break;
				case ICMD_DUP_X2:
					lifetimes += 4;
					break;
				case ICMD_DUP2_X2:
					lifetimes += 6;
					break;

				default:
					if (( dst != NULL) && (src != dst))
						lifetimes++;
				}
			}
		}
	}
	ls->maxlifetimes = lifetimes;
	ls->lifetimecount = lifetimes + jd->maxlocals * (TYPE_ADR+1);
}

void liveness_scan_basicblock(jitdata *jd, int b_index,
							  lv_sets *sets, int lifetimes) {
	int iindex;
	stackelement_t*    src;
	instruction *iptr;
	int i;
	methodinfo *m;
	lsradata *ls;

	ls = jd->ls;
	m  = jd->m;

	src = m->basicblocks[b_index].instack;

	iindex = m->basicblocks[b_index].icount - 1;
	/* set iptr to last instruction of BB */
	iptr = m->basicblocks[b_index].iinstr + iindex;

	bv_reset(sets->in, lifetimes);

	for (;iindex >= 0; iindex--, iptr--)  {
		/* get source Stack for the current instruction     */
		/* destination stack is available as iptr->dst                      */
		/* source stack is either the destination stack of the previos      */
		/* instruction, or the basicblock instack for the first instruction */
		if (iindex) /* != 0 is > 0 here, since iindex ist always >= 0 */
			src=(iptr-1)->dst;
		else
			src=m->basicblocks[b_index].instack;

		/* Reset kill and gen bitvectors for use in */
		/* liveness_scan_register_canditates */

		bv_reset(sets->kill, lifetimes);
		bv_reset(sets->gen, lifetimes);

		/* get gen and kill set of instruction */

		liveness_scan_registers_canditates(jd, b_index, iindex, src, sets);

		/* tmp = out(instr) - kill(instr) */

		bv_minus(sets->tmp, sets->out, sets->kill, lifetimes);

		/* in  = gen(instr) union tmp = gen union (out - kill) */

		bv_union(sets->in, sets->gen, sets->tmp, lifetimes);

		/* Set SAVEDVAR flag for locals */

		if (op_needs_saved[iptr->opc])
			for(i = ls->maxlifetimes; i < ls->lifetimecount; i++)
				if (!ls->lifetime[i].savedvar)
					if ( bv_get_bit(sets->in,i) && bv_get_bit(sets->out,i) )
						ls->lifetime[i].savedvar = SAVEDVAR;

        /* out(instr-1) = in(instr) (only one successor)*/

		bv_copy(sets->out, sets->in, lifetimes);
	}
	/* create gen sets for incoming stackslots */

	/* global instruction index for bb b_index */

	iindex = ls->icount_block[ls->sorted_rev[b_index]];
	bv_reset(sets->kill, lifetimes);
	bv_reset(sets->gen, lifetimes);
	src = m->basicblocks[b_index].instack;
	if (m->basicblocks[b_index].type != basicblock::TYPE_STD) {
		liveness_set_stack(ls, b_index, iindex, src, sets, LV_KILL);
		src = src->prev;
	}
	for (;src != NULL; src=src->prev) {
		/* only TEMP or LOCALVAR by now possible      */
		/* liveness_set_stack redirects for LOCALVARS */
		liveness_set_stack(ls, b_index, iindex, src,	sets, LV_GEN);
		_LV_ASSERT( ((src->varkind == LOCALVAR) || (src->varkind == TEMPVAR)) );
	}
	/* in  = gen union (out - kill) */
	bv_minus(sets->tmp, sets->out, sets->kill, lifetimes);
	bv_union(sets->in, sets->gen, sets->tmp, lifetimes);
}

void liveness(jitdata *jd) {
	bitvector *out;
	bitvector *in;
	bitvector params;
	char *buff;
	worklist *W;
	int b_index;
    struct _list *succ;
	struct _list *pred;
	bitvector visited;
	lv_sets   sets;
	int       i, p, t;
	methoddesc *md;
#ifdef LV_DEBUG_CHECK
	stackelement_t* s;
#endif
	methodinfo *m;
	registerdata *rd;
	lsradata *ls;
	/***************************************************************************
 TODO:
 - Exact Lifeness Information for intra Basic Blocks Stackslots are trivial
 They should not be included in the gen, kill, in and out sets to improve
 performance.
 - Local Vars as represented in rd->locals "are quite sparse". An intermediate
 representation for really used index/type pairs should be implemented.
	***************************************************************************/

	ls = jd->ls;
	m  = jd->m;
	rd = jd->rd;

	if (ls->lifetimecount == 0)
		return;
	ls->lifetime = DMNEW(struct lifetime, ls->lifetimecount);
	for (i=0; i < ls->lifetimecount; i++) ls->lifetime[i].type = -1;

	sets.gen = bv_new(ls->lifetimecount);
	sets.kill = bv_new(ls->lifetimecount);
	sets.tmp = bv_new(ls->lifetimecount);
	sets.out = bv_new(ls->lifetimecount);
	sets.in =  bv_new(ls->lifetimecount);

	params =  bv_new(ls->lifetimecount);

	visited = bv_new(m->basicblockcount);
	buff = DMNEW(char, ls->lifetimecount+1);

	out = DMNEW(bitvector, m->basicblockcount);
	in  = DMNEW(bitvector, m->basicblockcount);
	for(i = 0; i < m->basicblockcount; i++) {
		out[i] = bv_new(ls->lifetimecount);
		in[i]  = bv_new(ls->lifetimecount);
	}

	/* set in[0] to arguments */
 	/* <-> kill at 0, -1 */
	md = m->parseddesc;
	for (p = 0, i = 0; p < md->paramcount; p++) {
 		t = md->paramtypes[p].type;

		if (rd->locals[i][t].type >= 0)
			/* Param to Local init happens before normal Code */
			liveness_set_local(ls, 0, -1, i, t, &sets, LV_KILL);
 		i++;
 		if (IS_2_WORD_TYPE(t))    /* increment local counter a second time  */
 			i++;                  /* for 2 word types */
	}  /* end for */
	bv_copy(params, sets.kill, ls->lifetimecount);

	/* fill Worklist so that last node will be taken out first */
	W = wl_new(m->basicblockcount);
	for (i = 0; i < m->basicblockcount; i++)
		if (ls->sorted[i] != -1)
			wl_add(W, ls->sorted[i]);

	/* Worklist algorithm*/
	while (!wl_is_empty(W)) {
		b_index = wl_get(W);

		/* out[b_index] = for all s element of successors(b_index) union in[s]*/
		for (succ = ls->succ[b_index]; succ != NULL; succ = succ->next)
			bv_union(out[b_index], out[b_index], in[succ->value],
					 ls->lifetimecount);

		bv_copy(sets.out, out[b_index], ls->lifetimecount);

		/* compute in[b_index] */
		liveness_scan_basicblock(jd, b_index, &sets, ls->lifetimecount);

		if (!bv_get_bit(visited, b_index)) {
			liveness_join_lifetimes(jd, b_index);
			bv_set_bit(visited, b_index);
		}

		if (!bv_equal(sets.in, in[b_index], ls->lifetimecount)) {
			bv_copy(in[b_index], sets.in, ls->lifetimecount);
	 		for(pred = ls->pred[b_index]; pred != NULL; pred = pred->next)
				wl_add(W, pred->value);
		}
	}

#ifdef LV_DEBUG_CHECK
	s = m->basicblocks[b_index].instack;
	if ((s != NULL) && (m->basicblocks[b_index].flags != basicblock::TYPE_STD))
		s = s->prev;
	for( ; s != NULL; s = s->prev) {
#ifdef LV_DEBUG_VERBOSE
		if (!bv_get_bit(in[b_index], -s->varnum - 1)) {
			log_text("liveness: Error In Stacklot not live!\n");
		}
#endif
		_LV_ASSERT( (bv_get_bit(in[b_index], -s->varnum - 1)) );
	}
#endif

	for (i = 0; i < m->basicblockcount; i++)
		if ((b_index=ls->sorted[i]) != -1) {
			for(t = 0; t < ls->lifetimecount; t++) {
				if (ls->lifetime[t].type != -1) {
					if (bv_get_bit(in[b_index], t)) {
						p = ls->icount_block[ls->sorted_rev[b_index]];
						if (p < ls->lifetime[t].i_first_def)
							ls->lifetime[t].i_first_def = p;
					}
					if (bv_get_bit(out[b_index], t)) {
						p =
   ls->icount_block[ls->sorted_rev[b_index]]+m->basicblocks[b_index].icount - 1;
						if (p > ls->lifetime[t].i_last_use)
							ls->lifetime[t].i_last_use = p;
					}
				}
			}
		}

}

struct lifetime *liveness_get_ss_lifetime(lsradata *ls, stackelement_t* s) {
	struct lifetime *n;

	if (s->varnum >= 0) { /* new stackslot lifetime */
#ifdef LV_DEBUG_VERBOSE
		if (-ls->v_index - 1 >= ls->maxlifetimes) {
			printf("%i %i\n", -ls->v_index - 1, ls->maxlifetimes);
		}
#endif
		_LV_ASSERT(-ls->v_index - 1 < ls->maxlifetimes);

		n = &(ls->lifetime[-ls->v_index - 1]);
		n->type = s->type;
		n->v_index = ls->v_index--;
		n->usagecount = 0;

		n->i_last_use = -2;
		n->i_first_def = INT_MAX;
		n->local_ss = NULL;
		n->savedvar = 0;
		n->flags = 0;
	} else
		n = &(ls->lifetime[-s->varnum - 1]);

	liveness_add_ss( n, s);
	return n;
}

void liveness_set_stack(lsradata *ls, int block, int g_iindex, stackelement_t* s,
						lv_sets *sets,
						int op) {
	struct lifetime *n;

	if (s->varkind == LOCALVAR) {
		liveness_set_local(ls, block, g_iindex, s->varnum, s->type, sets, op);
	} else if (s->varkind == TEMPVAR) {
		n = liveness_get_ss_lifetime(ls, s);
		if (op == LV_KILL) {
			bv_set_bit(sets->kill, -s->varnum - 1);
			if (n->i_first_def > g_iindex) {
				n->i_first_def = g_iindex;
			}
		} else {
			/* LV_GEN */
			bv_set_bit(sets->gen, -s->varnum - 1);
			if (n->i_last_use < g_iindex) {
				n->i_last_use = g_iindex;
			}
		}
		n->usagecount+=ls->nesting[ls->sorted_rev[block]];
	}
}

void liveness_set_local(lsradata *ls, int block, int g_iindex, s4 v_index,
						int type, lv_sets *sets, int op) {
	struct lifetime *n;

	n = &(ls->lifetime[ ls->maxlifetimes + v_index * (TYPE_ADR+1) + type]);

	if (n->type == -1) { /* new local lifetime */
		n->local_ss=NULL;
		n->v_index=v_index;
		n->type=type;
		/* TODO: Look if local really needs to be a savedvar */
		n->savedvar = 0; /* SAVEDVAR; */
		n->flags = 0;
		n->usagecount = 0;

		n->i_last_use = -2;
		n->i_first_def = INT_MAX;
	}
	n->usagecount+=ls->nesting[ls->sorted_rev[block]];
	if (op == LV_KILL) {
		bv_set_bit(sets->kill, ls->maxlifetimes + v_index * (TYPE_ADR+1)+ type);
		if (n->i_first_def > g_iindex) {
			n->i_first_def = g_iindex;
		}
	} else {
		/* LV_GEN */
		bv_set_bit(sets->gen, ls->maxlifetimes + v_index * (TYPE_ADR+1) + type);
		if (n->i_last_use < g_iindex) {
			n->i_last_use = g_iindex;
		}
	}
}

void liveness_scan_registers_canditates(jitdata *jd, int b_index, int iindex,
										stackelement_t* src, lv_sets *sets)
{
/* 	methodinfo         *lm; */
	builtintable_entry *bte;
	methoddesc         *md;
	int i, g_iindex;
	instruction *iptr;
	stackelement_t*    dst;
	methodinfo *m;
	lsradata   *ls;

	m  = jd->m;
	ls = jd->ls;

	iptr = m->basicblocks[b_index].iinstr + iindex;
	dst = iptr->dst;
	g_iindex = ls->icount_block[ls->sorted_rev[b_index]] + iindex;
	switch (iptr->opc) {
		/* pop 0 push 0 */
	case ICMD_RET:
		/* local read (return adress) */
		liveness_set_local(ls, b_index, g_iindex, iptr->op1, TYPE_ADR, sets, LV_GEN);

		break;
	case ICMD_NOP:
/* 	case ICMD_ELSE_ICONST: */
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
		liveness_set_local(ls, b_index, g_iindex, iptr->op1, TYPE_INT, sets,  LV_GEN);
		liveness_set_local(ls, b_index, g_iindex, iptr->op1, TYPE_INT, sets,  LV_KILL);
		break;

		/* pop 0 push 1 const: const->stack */
	case ICMD_ICONST:
	case ICMD_LCONST:
	case ICMD_FCONST:
	case ICMD_DCONST:
	case ICMD_ACONST:
		/* new stack slot */
		liveness_set_stack(ls, b_index, g_iindex, dst, sets, LV_KILL);
		break;

		/* pop 0 push 1 load: local->stack */
	case ICMD_ILOAD:
	case ICMD_LLOAD:
	case ICMD_FLOAD:
	case ICMD_DLOAD:
	case ICMD_ALOAD:
		if (dst->varkind != LOCALVAR) {
			/* local->value on stack */
			liveness_set_local(ls, b_index, g_iindex, iptr->op1,
							   iptr->opc - ICMD_ILOAD, sets, LV_GEN);
			/* value->stack */
			liveness_set_stack(ls, b_index, g_iindex, dst, sets, LV_KILL);
		} else /* if (dst->varnum != iptr->op1) */ {
			/* local -> local */
			liveness_set_local(ls, b_index, g_iindex, iptr->op1,
							   iptr->opc - ICMD_ILOAD, sets, LV_GEN);
			liveness_set_local(ls, b_index, g_iindex, dst->varnum,
							   iptr->opc - ICMD_ILOAD, sets, LV_KILL);
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
		liveness_set_stack(ls, b_index, g_iindex, src, sets, LV_GEN);
		/* stack->arrayref */
		liveness_set_stack(ls, b_index, g_iindex, src->prev, sets, LV_GEN);
		/* arrayref[index]->stack */
		liveness_set_stack(ls, b_index, g_iindex, dst, sets, LV_KILL);
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
		/* stack -> value */
		liveness_set_stack(ls, b_index, g_iindex, src, sets, LV_GEN);
		/* stack -> index*/
		liveness_set_stack(ls, b_index, g_iindex, src->prev, sets, LV_GEN);
		/* stack -> arrayref */
		liveness_set_stack(ls, b_index, g_iindex, src->prev->prev, sets, LV_GEN);
		break;

		/* pop 1 push 0 store: stack -> local */
	case ICMD_ISTORE:
	case ICMD_LSTORE:
	case ICMD_FSTORE:
	case ICMD_DSTORE:
	case ICMD_ASTORE:
		if (src->varkind != LOCALVAR) {
			/* stack -> value */
			liveness_set_stack(ls, b_index, g_iindex, src, sets, LV_GEN);
			liveness_set_local(ls, b_index, g_iindex, iptr->op1, iptr->opc - ICMD_ISTORE,
							   sets, LV_KILL);
		} else {
			liveness_set_local(ls, b_index, g_iindex, src->varnum, iptr->opc-ICMD_ISTORE,
							   sets, LV_GEN);
			liveness_set_local(ls, b_index, g_iindex, iptr->op1, iptr->opc - ICMD_ISTORE,
							   sets, LV_KILL);
		}
		break;

		/* pop 1 push 0 */
	case ICMD_POP: /* throw away a stackslot */
		/* TODO: check if used anyway (DUP...) and change codegen to */
		/*       ignore this stackslot */
		liveness_set_stack(ls, b_index, g_iindex, src, sets, LV_GEN);
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
		/* stack -> value */
		liveness_set_stack(ls, b_index, g_iindex, src, sets, LV_GEN);
		break;

		/* pop 2 push 0 */
	case ICMD_POP2: /* throw away 2 stackslots */
		/* TODO: check if used anyway (DUP...) and change codegen to */
		/*       ignore this stackslot */
		liveness_set_stack(ls, b_index, g_iindex, src, sets, LV_GEN);
		liveness_set_stack(ls, b_index, g_iindex, src->prev, sets, LV_GEN);
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
		/* stack -> value*/
		liveness_set_stack(ls, b_index, g_iindex, src, sets, LV_GEN);
		liveness_set_stack(ls, b_index, g_iindex, src->prev, sets, LV_GEN);
		break;

		/* pop 0 push 1 dup */
	case ICMD_DUP:
		/* src == dst->prev */
		/* src -> dst */

		/* src and dst->prev are identical */
		/*		liveness_set_stack(ls, b_index, g_iindex, src, sets, LV_KILL);
				liveness_set_stack(ls, b_index, g_iindex, dst->prev, sets, LV_GEN);*/

		liveness_set_stack(ls, b_index, g_iindex, dst, sets, LV_KILL);

		break;

		/* pop 0 push 2 dup */
	case ICMD_DUP2:
		/* src       ==  dst->prev->prev */
		/* src->prev == dst->prev->prev->prev */
		/* src       ->  dst */
		/* src->prev -> dst->prev-> */

		liveness_set_stack(ls, b_index, g_iindex, dst, sets, LV_KILL);
		liveness_set_stack(ls, b_index, g_iindex, dst->prev, sets, LV_KILL);
		break;

		/* pop 2 push 3 dup */
	case ICMD_DUP_X1:
		/* src       -> dst */
		/* src->prev -> dst->prev */
		/* src       -> dst->prev->prev */

		liveness_set_stack(ls, b_index, g_iindex, dst, sets, LV_KILL);
		liveness_set_stack(ls, b_index, g_iindex, dst->prev, sets, LV_KILL);
		liveness_set_stack(ls, b_index, g_iindex, dst->prev->prev, sets,
						   LV_KILL);
		liveness_set_stack(ls, b_index, g_iindex + 1, src, sets, LV_GEN);
		liveness_set_stack(ls, b_index, g_iindex + 1, src->prev, sets, LV_GEN);
		break;

		/* pop 3 push 4 dup */
	case ICMD_DUP_X2:
		/* src             -> dst                   */
		/* src->prev       -> dst->prev             */
		/* src->prev->prev -> dst->prev->prev       */
		/* dst (=src)      -> dst->prev->prev->prev */

		liveness_set_stack(ls, b_index, g_iindex, dst, sets, LV_KILL);
		liveness_set_stack(ls, b_index, g_iindex, dst->prev, sets, LV_KILL);
		liveness_set_stack(ls, b_index, g_iindex, dst->prev->prev, sets,
						   LV_KILL);
		liveness_set_stack(ls, b_index, g_iindex, dst->prev->prev->prev, sets,
						   LV_KILL);
		liveness_set_stack(ls, b_index, g_iindex + 1, src, sets, LV_GEN);
		liveness_set_stack(ls, b_index, g_iindex + 1, src->prev, sets, LV_GEN);
		liveness_set_stack(ls, b_index, g_iindex + 1, src->prev->prev, sets,
						   LV_GEN);
		break;

		/* pop 3 push 5 dup */
	case ICMD_DUP2_X1:
		/* src                    -> dst                   */
		/* src->prev              -> dst->prev             */
		/* src->prev->prev        -> dst->prev->prev       */
		/* dst (=src)             -> dst->prev->prev->prev */
		/* dst->prev (=src->prev) -> dst->prev->prev->prev->prev */
		liveness_set_stack(ls, b_index, g_iindex, dst, sets, LV_KILL);
		liveness_set_stack(ls, b_index, g_iindex, dst->prev, sets, LV_KILL);
		liveness_set_stack(ls, b_index, g_iindex, dst->prev->prev, sets,
						   LV_KILL);
		liveness_set_stack(ls, b_index, g_iindex, dst->prev->prev->prev, sets,
						   LV_KILL);
		liveness_set_stack(ls, b_index, g_iindex, dst->prev->prev->prev->prev,
						   sets, LV_KILL);
		liveness_set_stack(ls, b_index, g_iindex + 1, src, sets, LV_GEN);
		liveness_set_stack(ls, b_index, g_iindex + 1, src->prev, sets, LV_GEN);
		liveness_set_stack(ls, b_index, g_iindex + 1, src->prev->prev, sets,
						   LV_GEN);
		break;

		/* pop 4 push 6 dup */
	case ICMD_DUP2_X2:
		/* src                    -> dst                   */
		/* src->prev              -> dst->prev             */
		/* src->prev->prev        -> dst->prev->prev       */
		/* src->prev->prev->prev  -> dst->prev->prev->prev       */
		/* dst (=src)             -> dst->prev->prev->prev->prev */
		/* dst->prev (=src->prev) -> dst->prev->prev->prev->prev->prev */
		liveness_set_stack(ls, b_index, g_iindex, dst, sets, LV_KILL);
		liveness_set_stack(ls, b_index, g_iindex, dst->prev, sets, LV_KILL);
		liveness_set_stack(ls, b_index, g_iindex, dst->prev->prev, sets,
						   LV_KILL);
		liveness_set_stack(ls, b_index, g_iindex, dst->prev->prev->prev, sets,
						   LV_KILL);
		liveness_set_stack(ls, b_index, g_iindex, dst->prev->prev->prev->prev,
						   sets, LV_KILL);
		liveness_set_stack(ls, b_index, g_iindex,
						   dst->prev->prev->prev->prev->prev, sets, LV_KILL);
		liveness_set_stack(ls, b_index, g_iindex + 1, src, sets, LV_GEN);
		liveness_set_stack(ls, b_index, g_iindex + 1, src->prev, sets, LV_GEN);
		liveness_set_stack(ls, b_index, g_iindex + 1, src->prev->prev, sets,
						   LV_GEN);
		liveness_set_stack(ls, b_index, g_iindex + 1, src->prev->prev->prev,
						   sets, LV_GEN);
		break;

		/* pop 2 push 2 swap */
	case ICMD_SWAP:
		/* src       -> dst->prev */
		/* src->prev -> dst       */
		liveness_set_stack(ls, b_index, g_iindex, dst, sets, LV_KILL);
		liveness_set_stack(ls, b_index, g_iindex, dst->prev, sets, LV_KILL);
		liveness_set_stack(ls, b_index, g_iindex + 1, src, sets, LV_GEN);
		liveness_set_stack(ls, b_index, g_iindex + 1, src->prev, sets, LV_GEN);
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

	case ICMD_ISUB:

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
		liveness_set_stack(ls, b_index, g_iindex, dst, sets, LV_KILL);
		liveness_set_stack(ls, b_index, g_iindex, src, sets, LV_GEN);
		liveness_set_stack(ls, b_index, g_iindex, src->prev, sets, LV_GEN);
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

/* 	case ICMD_IFEQ_ICONST: */
/* 	case ICMD_IFNE_ICONST: */
/* 	case ICMD_IFLT_ICONST: */
/* 	case ICMD_IFGE_ICONST: */
/* 	case ICMD_IFGT_ICONST: */
/* 	case ICMD_IFLE_ICONST: */

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
		liveness_set_stack(ls, b_index, g_iindex, dst, sets, LV_KILL);
		liveness_set_stack(ls, b_index, g_iindex, src, sets, LV_GEN);
		break;

		/* pop 0 push 1 */
	case ICMD_GETSTATIC:

	case ICMD_NEW:
		liveness_set_stack(ls, b_index, g_iindex, dst, sets, LV_KILL);
		break;

		/* pop many push any */

	case ICMD_INVOKESTATIC:
	case ICMD_INVOKESPECIAL:
	case ICMD_INVOKEVIRTUAL:
	case ICMD_INVOKEINTERFACE:
			INSTRUCTION_GET_METHODDESC(iptr,md);
			i = md->paramcount;
			while (--i >= 0) {
				liveness_set_stack(ls, b_index, g_iindex, src, sets, LV_GEN);
				src = src->prev;
			}
			if (md->returntype.type != TYPE_VOID)
				liveness_set_stack(ls, b_index, g_iindex, dst, sets, LV_KILL);
			break;

	case ICMD_BUILTIN:
		bte = iptr->val.a;
		md = bte->md;
		i = md->paramcount;
		while (--i >= 0) {
			liveness_set_stack(ls, b_index, g_iindex, src, sets, LV_GEN);
			src = src->prev;
		}
		if (md->returntype.type != TYPE_VOID)
			liveness_set_stack(ls, b_index, g_iindex, dst, sets, LV_KILL);
		break;

	case ICMD_MULTIANEWARRAY:
		i = iptr->op1;
		while (--i >= 0) {
			liveness_set_stack(ls, b_index, g_iindex, src, sets, LV_GEN);
			src = src->prev;
		}
		liveness_set_stack(ls, b_index, g_iindex, dst, sets, LV_KILL);
		break;

	default:
		exceptions_throw_internalerror("Unknown ICMD %d during register allocation",
									   iptr->opc);
		return;
	} /* switch */
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
