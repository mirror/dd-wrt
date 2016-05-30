/* src/vm/jit/optimizing/ssa.cpp - static single-assignment form

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

#include "vm/descriptor.hpp"

#include "vm/jit/cfg.hpp"
#include "vm/jit/builtin.hpp"

#include "vm/jit/code.hpp"
#include "vm/jit/jit.hpp" /* icmd_table */

#include "vm/jit/ir/bytecode.hpp"

#include "vm/jit/optimizing/dominators.hpp"
#include "vm/jit/optimizing/graph.hpp"
#include "vm/jit/optimizing/lifetimes.hpp"
#include "vm/jit/optimizing/lsra.hpp"

#include "vm/jit/optimizing/ssa.hpp"
#include "vm/jit/optimizing/ssa_rename.hpp"
#include "vm/jit/optimizing/ssa_phi.hpp"

#if defined(SSA_DEBUG_VERBOSE)
#include "vm/options.hpp"   /* compileverbose */
#endif

/* function prototypes */
void ssa_set_local_def(lsradata *, int , int);
void ssa_set_interface(jitdata *, basicblock *, s4 *);

void dead_code_elimination(jitdata *jd, graphdata *gd);
void copy_propagation(jitdata *jd, graphdata *gd);
void ssa_replace_use_sites(jitdata *, graphdata *, struct lifetime *,
						int , worklist *);

void ssa_set_def(lsradata *, int , int );
void ssa_set_local_def(lsradata *, int , int );
void ssa_set_iovar(lsradata *, s4 , int , s4 *);
void ssa_set_interface(jitdata *, basicblock *, s4 *);

#ifdef SSA_DEBUG_VERBOSE
void ssa_print_trees(jitdata *jd, graphdata *gd, dominatordata *dd);
void ssa_print_lt(lsradata *ls);
void _ssa_print_lt(struct lifetime *lt);
#endif

/* ssa ************************************************************************

SSA main procedure:

SSA Algorithms are based on "modern compiler implementation in C" from andrew
w. appel, edition 2004

Corrections:
page 441 Algorithm 19.6. Inserting phi-functions:

...
    for each Y in DF[n]
-       if Y not element of A phi [n]
+       if a not element of A phi [n]
            insert the statment a <- ...
...
...
-       A phi [n] <- A phi [n] join {Y}
+       A phi [n] <- A phi [n] join {a}
-      if Y not element of A orig [n]
+      if a not element of A orig [Y]
           W <- W join {Y}

******************************************************************************/
void xssa(jitdata *jd);
void yssa(jitdata *jd);
void ssa(jitdata *jd) {
	struct dominatordata *dd;
	lsradata *ls;
	graphdata *gd;

#if defined(ENABLE_LOOPS)
	/* Loop optimization "destroys" the basicblock array */
	/* TODO: work with the basicblock list               */
	if (opt_loops) {
		log_text("lsra not possible with loop optimization\n");
		exit(1);
	}
#endif

	cfg_add_root(jd);
	cfg_add_exceptional_edges(jd);
	/*pythonpass_run(jd, "foo", "cfg_print");*/
	/*dominator_tree_validate(jd, dd);*/
	/*pythonpass_run(jd, "ssa2", "main");*/
	/*pythonpass_run(jd, "alt_ssa", "main");*/
	/*pythonpass_run(jd, "foo", "before");*/

	/*if (getenv("XSSA")) {
		dominator_tree_build(jd);
		dominance_frontier_build(jd);
		xssa(jd);
	} else */{
		yssa(jd);
	}
	/*pythonpass_run(jd, "foo", "after");*/
	cfg_remove_root(jd);
	return;

	ls = jd->ls;

    ssa_init(jd);
	/* Generate the Control Flow Graph */
	/* Add one for a Basic Block 0 to be inserted, so lateron */
	/* with SSA Parameter initialization is handled right */

	gd = graph_init(jd->basicblockcount + 1);
	graph_make_cfg(jd, gd);

	dd = compute_Dominators(gd, ls->basicblockcount);
	computeDF(gd, dd, ls->basicblockcount, 0);

	ssa_place_phi_functions(jd, gd, dd);
	ssa_rename(jd, gd, dd);
#ifdef SSA_DEBUG_VERBOSE
	if (compileverbose) {
		printf("Phi before Cleanup\n");
		ssa_print_phi(ls, gd);
		ssa_print_lt(ls);
	}
#endif
	lt_scanlifetimes(jd, gd, dd);
#ifdef SSA_DEBUG_VERBOSE
	if (compileverbose) {
		ssa_print_lt(ls);
	}
#endif
	if (opt_ssa_dce) {
		dead_code_elimination(jd, gd);
#ifdef SSA_DEBUG_VERBOSE
		if (compileverbose) {
			printf("Phi after dead code elemination\n");
			ssa_print_phi(ls, gd);
			ssa_print_lt(ls);
		}
#endif
	}
	if (opt_ssa_cp) {
		copy_propagation(jd, gd);
#ifdef SSA_DEBUG_VERBOSE
		if (compileverbose) {
			printf("Phi after copy propagation\n");
			ssa_print_phi(ls, gd);
			ssa_print_lt(ls);
		}
#endif
	}

	ssa_generate_phi_moves(jd, gd);
	transform_BB(jd, gd);

	lt_lifeness_analysis(jd, gd);

#ifdef SSA_DEBUG_CHECK
	{
		int i, j, pred, in_d, out_d;
		graphiterator iter_pred;
		s4 *in, *out;
		bool phi_define;
		methodinfo *m;

		m = jd->m;

		for(i = 0; i < ls->basicblockcount; i++) {
			if (ls->basicblocks[i]->indepth != 0) {
				pred = graph_get_first_predecessor(gd, i, &iter_pred);
				for (; (pred != -1); pred = graph_get_next(&iter_pred)) {
					in_d = ls->basicblocks[i]->indepth - 1;
					in = ls->basicblocks[i]->invars;
					for (; in_d >= 0; in_d--) {
						phi_define = false;
						for (j = 0; (!phi_define) && (j<ls->ssavarcount); j++) {
							if (ls->phi[i][j] != NULL)
								if (ls->phi[i][j][0] == in[in_d])
									phi_define = true;
						}
						if (!phi_define) {
							/* in not defined in phi function -> check with */
							/* outstack(s)  of predecessor(s) */
							out_d = ls->basicblocks[pred]->outdepth - 1;
							out = ls->basicblocks[pred]->outvars;
							_SSA_ASSERT(out_d >= in_d);
							for(; out_d > in_d; out_d--);
							if ((in[in_d] != out[out_d]) ||
							(VAR(in[in_d])->flags != VAR(out[out_d])->flags)) {
								printf("Method: %s %s\n",
									   m->clazz->name.begin(), m->name.begin());
									printf("Error: Stack Varnum Mismatch BBin %3i BBout %3i Stackdepth %3i\n", i, pred, in_d);
								if (compileverbose)
									printf("Error: Stack Varnum Mismatch BBin %3i BBout %3i Stackdepth %3i\n", i, pred, in_d);
/* 								else */
/* 									_SSA_ASSERT(0); */
							}
						}
					}
				}
			}
		}
	}

#endif

#ifdef SSA_DEBUG_VERBOSE
	if (compileverbose)
		ssa_print_trees(jd, gd, dd);
#endif

}

/* ssa_init *******************************************************************

Initialise data structures for ssa

IOVARS of same stackdepth and same type are coalesced:
interface_map[ 5 * stackdepth + type ] = new_varindex with
0 <= new_varindex < ls->ssavarcount

TODO: check if coalescing of IOVARS of same stackdepth and type only of adjacent
basic blocks could decrease the number of phi functions and so improve ssa
analysis performance!

All LOCALVARS and IOVARS get a new unique varindex:
ls->new_varindex[0..jd->varcount[ = new_varindex with
0 <= new_varindex < ls->ssavarcount

The jd->varcount bits long bitvectors ls->var_def[0..jd->basicblockindex+1[
 are set  to the definitions of LOCALVARS and IOVARS. (So the only the first
ls->ssavarcount bits of each of these vectors contain valid data, but
ls->ssavarcount is computed at the same time as the definitons are stored.)

The basic block number used as index for the bitvector array ls->var_def is
already shifted by one to make space for the new basic block 0 for parameter
initialization.

******************************************************************************/

void ssa_init(jitdata *jd) {
	int         p, t, v;
	methoddesc  *md;
	int         i, l, b_index, len;
	instruction *iptr;
	basicblock  *bptr;
	s4          *interface_map; /* holds an new unique index for every used  */
	                            /* basic block inoutvar described by a dupel */
                                /*(depth,type). Used to coalesce the inoutvars*/
	methodinfo   *m;
	lsradata     *ls;
	builtintable_entry *bte;

	ls = jd->ls;
	m = jd->m;
	md = m->parseddesc;


#if defined(SSA_DEBUG_CHECK) || defined(SSA_DEBUG_VERBOSE)
# if defined(SSA_DEBUG_VERBOSE)
	if (compileverbose) {
		printf("%s %s ", m->clazz->name.begin(), m->name.begin());
		if (code_is_leafmethod(jd->code))
			printf("**Leafmethod**");
		printf("\n");
	}
# endif
	if (strcmp(m->clazz->name.begin(), "spec/benchmarks/_213_javac/Parser")==0)
		if (strcmp(m->name.begin(),"parseTerm")==0)
# if defined(SSA_DEBUG_VERBOSE)
			if (compileverbose)
				printf("12-------------------12\n");
# else
	        { int dummy=1; dummy++; }
# endif
#endif

#ifdef SSA_DEBUG_VERBOSE
    if (compileverbose)
		printf("ssa_init: basicblockcount %3i localcount %3i\n",
			   jd->basicblockcount, jd->localcount);
#endif

	/* As first step all definitions of local variables and in/out vars are   */
	/* gathered. in/outvars are coalesced for same type and depth             */
	/* "normal" tempvars (just living within one basicblock are) ignored      */

	ls->num_defs = DMNEW(int, jd->varcount);
	ls->new_varindex = DMNEW(int , jd->varcount);

	for(p = 0; p < jd->varcount; p++) {
		ls->num_defs[p] = 0;
		ls->new_varindex[p] = jitdata::UNUSED;
	}

	/* init Var Definition bitvectors */

	ls->var_def = DMNEW(int *, jd->basicblockcount + 1);
	for(i = 0; i < jd->basicblockcount + 1; i++) {
		ls->var_def[i] =  bv_new(jd->varcount);
	}

	ls->ssavarcount = 0;

	/* Add parameters first in right order, so the new local indices          */
	/* 0..p will correspond to "their" parameters                             */
	/* They get defined at the artificial Block 0, the real method bbs will   */
	/* be ed to start at block 1                                              */

	/* don't look at already eliminated (not used) parameters (locals) */

 	for (p = 0, l = 0; p < md->paramcount; p++) {
 		t = md->paramtypes[p].type;
		i = jd->local_map[l * 5 + t];
 		l++;
 		if (IS_2_WORD_TYPE(t))    /* increment local counter a second time  */
 			l++;                  /* for 2 word types */
		if (i == jitdata::UNUSED)
			continue;
		ssa_set_local_def(ls, -1, i);
	}

	_SSA_ASSERT(ls->ssavarcount < jd->varcount);

	/* coalesce bb in/out vars */

	interface_map = DMNEW(s4, jd->stackcount * 5);
	for(i = 0; i < jd->stackcount * 5; i++)
		interface_map[i] = jitdata::UNUSED;

	bptr = jd->basicblocks;

	for(; bptr != NULL; bptr = bptr->next) {
#ifdef SSA_DEBUG_VERBOSE
	if (compileverbose)
		printf("ssa_init: BB %3i state %3x\n",bptr->nr, bptr->state);
#endif
		if (bptr->state >= basicblock::REACHED) {

			/* 'valid' Basic Block */

			b_index = bptr->nr;

			len = bptr->icount;
			iptr = bptr->iinstr;
			ssa_set_interface(jd, bptr, interface_map);

			/* !!!!!!!!! not true for now !!!!!!!!! */
			/* All baseline optimizations from stack.c are turned off for */
			/* SSA! */

			for (; len > 0; len--, iptr++) {

				/* Look for definitions (iptr->dst). INVOKE and BUILTIN have */
				/* an optional dst - so they to be checked first */

				v = jitdata::UNUSED;
				if (icmd_table[iptr->opc].dataflow == DF_INVOKE) {
						INSTRUCTION_GET_METHODDESC(iptr,md);
						if (md->returntype.type != TYPE_VOID)
							v = iptr->dst.varindex;
				}
				else if (icmd_table[iptr->opc].dataflow == DF_BUILTIN) {
						bte = iptr->sx.s23.s3.bte;
						md = bte->md;
						if (md->returntype.type != TYPE_VOID)
							v = iptr->dst.varindex;
				}
				else if (icmd_table[iptr->opc].dataflow >= DF_DST_BASE) {
 					v = iptr->dst.varindex;
				}

				if (v != jitdata::UNUSED) {
					if (( v < jd->localcount) || ( VAR(v)->flags & INOUT )) {

				  /* !IS_TEMPVAR && !IS_PREALLOC == (IS_LOCALVAR || IS_INOUT) */

						ssa_set_local_def(ls, b_index, v);
					}
				}
			}
		}
	}
	_SSA_ASSERT(ls->ssavarcount < jd->varcount);

#ifdef SSA_DEBUG_VERBOSE
	if (compileverbose) {
		printf("ssa_init: Vars: Orig:%3i SSAVar: %3i\n", jd->varcount,
			   ls->ssavarcount);
		for(i = 0; i < jd->varcount; i++) {
			if ((i < jd->localcount) || ( VAR(i)->flags & INOUT)) {
				printf("%3i(%3i,%3x) ",i,VAR(i)->type, VAR(i)->flags);
				ssa_show_variable(jd, i, VAR(i),0);
				if (i < ls->ssavarcount) {
					printf(" -> %3i", ls->new_varindex[i]);
				}
				printf("\n");
			}
		}
	}
#endif
}

/* ssa_set_def ****************************************************************

Helper for ssa_set_local_def and ssa_set_interface

The definition of a var is stored in the bitvector array ls->var_def.

The number of definitons of each var is counted, so the number of new vars with
SSA is known.

******************************************************************************/

void ssa_set_def(lsradata *ls, int b_index, int varindex) {

	/* b_index + 1 to leave space for the param init block 0 */

	bv_set_bit(ls->var_def[b_index + 1], varindex);

	/* count number of defs for every var since SSA */
	/* will create a new var for every definition */

	ls->num_defs[varindex]++;
}

/* ssa_set_local_def **********************************************************

Helper for ssa_init

Assigns a new unique index for the local var varindex (if not already done so)
and then calls ssa_set_def to remember the definition in the bitvector array
ls->var_def

******************************************************************************/

void ssa_set_local_def(lsradata *ls, int b_index, int varindex) {

	if (ls->new_varindex[varindex] == jitdata::UNUSED) {
		ls->new_varindex[varindex] = ls->ssavarcount++;
	}

	ssa_set_def(ls, b_index, ls->new_varindex[varindex]);
}

/* ssa_set_local_def **********************************************************

Helper for ssa_set_interface

IN: ls              pointer to lsradata structure
    iovar           varindex of INOUTVAR to process
	map_index       stackdepth * 5 + type, used for coalescing IOVARS.

IN/OUT
	interface_map   used for coalescing IOVARS. interface_map[map_index] ==
	                UNUSED, if this map_index (==stackdepth,type tupel) did not
					occur till now. Then interface_map[map_index] will be set
					to a new unique index.
	ls->new_varindex will be set to this new unique index to map the old
	                 varindizes to the new ones.

Assigns a new unique index for the local var varindex (if not already done so)
and then calls ssa_set_def to remember the definition in the bitvector array
ls->var_def

******************************************************************************/

void ssa_set_iovar(lsradata *ls, s4 iovar, int map_index, s4 *interface_map) {
		if (interface_map[map_index] == jitdata::UNUSED)
			interface_map[map_index] = ls->ssavarcount++;

		ls->new_varindex[iovar] = interface_map[map_index];
}


/* ssa_set_interface ***********************************************************

Helper for ssa_init

IN: ls              pointer to lsradata structure
    *bptr           pointer to the basic block to be processed

IN/OUT
	interface_map   used for coalescing IOVARS. interface_map[map_index] ==
	                UNUSED, if this map_index (==stackdepth,type tupel) did not
					occur till now. Then interface_map[map_index] will be set
					to a new unique index. (see ssa_set_iovar)

Searches the basic block given by *bptr for IN and OUTVARS and coalesces them
for each unique stackdepth,type dupel. For each OUTVAR with a different or no
INVAR at the same stackdepth the definition of this OUTVAR in this basic block
is remembered in ls->var_def. (see ssa_set_def)

******************************************************************************/

void ssa_set_interface(jitdata *jd, basicblock *bptr, s4 *interface_map) {
	s4 *out, *in;
	int in_d, out_d;
	int o_map_index, i_map_index;
	lsradata *ls;

	ls = jd->ls;

	out = bptr->outvars;
	in = bptr->invars;
	in_d = bptr->indepth - 1;
	out_d = bptr->outdepth - 1;

	/* ignore top Stackelement of instack in case of EXH or SBR blocks */
	/* These are no Interface stackslots! */
	if ((bptr->type == basicblock::TYPE_EXH) ||
		(bptr->type == basicblock::TYPE_SBR)) {
		in_d--;
	}

	/* invars with no corresponding outvars are not defined here */
	/* just set up the interface_map */

	for(;(in_d > out_d); in_d--) {
		i_map_index = in_d * 5 + VAR(in[in_d])->type;
		ssa_set_iovar(ls, in[in_d], i_map_index, interface_map);
	}

	while((out_d >= 0)) {
		/* set up interface_map */

		o_map_index = out_d * 5 + VAR(out[out_d])->type;
		if (in_d >= 0) {
			i_map_index = in_d * 5 + VAR(in[in_d])->type;
			ssa_set_iovar(ls, in[in_d], i_map_index, interface_map);
		}
		ssa_set_iovar(ls, out[out_d], o_map_index, interface_map);
		if (in_d == out_d) {
			out_d--;
			in_d--;
		}
		else {
			out_d--;
		}
	}
}

#ifdef SSA_DEBUG_VERBOSE
void ssa_print_trees(jitdata *jd, graphdata *gd, dominatordata *dd) {
	int i,j;
	lsradata *ls;

	ls = jd->ls;

	printf("ssa_printtrees: maxlocals %3i", jd->localcount);

	printf("Dominator Tree: \n");
	for(i = 0; i < ls->basicblockcount; i++) {
		printf("%3i:",i);
		for(j = 0; j < ls->basicblockcount; j++) {
			if (dd->idom[j] == i) {
				printf(" %3i", j);
			}
		}
		printf("\n");
	}

	printf("Dominator Forest:\n");
	for(i = 0; i < ls->basicblockcount; i++) {
		printf("%3i:",i);
		for(j = 0; j < dd->num_DF[i]; j++) {
				printf(" %3i", dd->DF[i][j]);
		}
		printf("\n");
	}
#if 0
	if (ls->ssavarcount > 0) {
	printf("Use Sites\n");
   	for(i = 0; i < ls->ssavarcount; i++) {
		printf("use_sites[%3i]=%p:",i,(void *)ls->use_sites[i]);
		for(j = 0; j < ls->basicblockcount; j++) {
			if ((j % 5) == 0) printf(" ");
			if (bv_get_bit(ls->use_sites[i], j))
				printf("1");
			else
				printf("0");
		}
		printf("\n");
	}
	}
#endif
	printf("var Definitions:\n");
   	for(i = 0; i < jd->basicblockcount; i++) {
		printf("var_def[%3i]=%p:",i,(void *)ls->var_def[i]);
		for(j = 0; j < ls->ssavarcount; j++) {
			if ((j % 5) == 0) printf(" ");
			if (bv_get_bit(ls->var_def[i], j))
				printf("1");
			else
				printf("0");
		}
		printf(" (");
		for(j=0; j < ((((ls->ssavarcount * 5+7)/8) + sizeof(int) - 1)/sizeof(int));
			j++)
			printf("%8x",ls->var_def[i][j]);
		printf(")\n");
	}
}
#endif


/****************************************************************************
Optimizations
****************************************************************************/


/****************************************************************************
Dead Code Elimination
****************************************************************************/
void dead_code_elimination(jitdata *jd, graphdata *gd) {
	int a, i, source;
	worklist *W;

	instruction *iptr;
	struct lifetime *lt, *s_lt;

	bool remove_statement;
	struct site *use;

	lsradata *ls;
#ifdef SSA_DEBUG_VERBOSE
	methodinfo *m;

	m  = jd->m;
#endif
	ls = jd->ls;

	W = wl_new(ls->lifetimecount);
	if (ls->lifetimecount > 0) {

		/* put all lifetimes into Worklist W */

		for(a = 0; a < ls->lifetimecount; a++) {
			if (ls->lifetime[a].type != jitdata::UNUSED) {
				wl_add(W, a);
			}
		}
	}

	/* Remove unused lifetimes */

	while(!wl_is_empty(W)) {

		/* take a var out of Worklist */

		a = wl_get(W);

		lt = &(ls->lifetime[a]);
		if ((lt->def == NULL) || (lt->type == jitdata::UNUSED))

			/* lifetime was already removed -> no defs anymore */

			continue;

		/* Remove lifetimes, which are only used in the phi function which */
		/* defines them! */

		remove_statement = (lt->use != NULL) && (lt->use->iindex < 0);
		for(use = lt->use; (remove_statement && (use != NULL));
			use = use->next)
		{
			remove_statement = remove_statement &&
				(use->b_index == lt->def->b_index) &&
				(use->iindex == lt->def->iindex);
		}

		if (remove_statement) {
#ifdef SSA_DEBUG_CHECK

			/* def == use can only happen in phi functions */

			if (remove_statement)
				_SSA_ASSERT(lt->use->iindex < 0);
#endif

			/* give it free for removal */

			lt->use = NULL;
		}

		if (lt->use == NULL) {

			/* Look at statement of definition of a and remove it,           */
			/* if the Statement has no sideeffects other than the assignemnt */
			/* of a */

			if (lt->def->iindex < 0 ) {

				/* phi function */
				/* delete use of sources , delete phi functions  */

				_SSA_ASSERT(ls->phi[lt->def->b_index][-lt->def->iindex-1] !=
							NULL);

				for (i = 1;i <= graph_get_num_predecessor(gd, lt->def->b_index);
					 i++) {
					source =
						ls->phi[lt->def->b_index][-lt->def->iindex-1][i];
					if ((source != ls->varcount_with_indices) &&
						(source != lt->v_index) &&
						(source != jitdata::UNUSED)) {

						/* phi Argument was not already removed (already in
						   because of "selfdefinition") */

						s_lt = &(ls->lifetime[source]);

						/* remove it */

						lt_remove_use_site(s_lt,lt->def->b_index,
										   lt->def->iindex);

						/*  put it on the Worklist */

						wl_add(W, source);
					}
				}

				/* now delete phi function itself */
#ifdef SSA_DEBUG_VERBOSE
					if (compileverbose) {
						printf("dce: BB%3i phi for var %3i=%3i removed \n",
							   lt->def->b_index, -lt->def->iindex - 1,
							   lt->v_index);
					}
#endif

				ls->phi[lt->def->b_index][-lt->def->iindex-1] = NULL;
			}
			else {

				/* "normal" Use by ICMD */

				remove_statement = false;
				if (lt->def->b_index != 0) {

					/* do not look at artificial block 0 (parameter init) */

					iptr = ls->basicblocks[lt->def->b_index]->iinstr +
						lt->def->iindex;

					if (icmd_table[iptr->opc].flags & ICMDTABLE_PEI) {

						/* if ICMD could throw an exception do not remove it! */

						remove_statement = false;
#ifdef SSA_DEBUG_VERBOSE
						if (compileverbose) {
							printf("dce: PEI: BB%3i II%3i %s not removed \n",
								   lt->def->b_index, lt->def->iindex,
								   bytecode[iptr->opc].mnemonic);
						}
#endif

					}
					else {
						remove_statement = true;

						switch (icmd_table[iptr->opc].dataflow) {
						case DF_3_TO_0:
						case DF_3_TO_1: /* icmd has s1, s2 and s3 */
							a = iptr->sx.s23.s3.varindex;
							s_lt = ls->lifetime + a;
							lt_remove_use_site(s_lt, lt->def->b_index,
											   lt->def->iindex);
							wl_add(W, a);

							/* "fall through" for handling of s2 and s1 */

						case DF_2_TO_0:
						case DF_2_TO_1: /* icmd has s1 and s2 */
							a = iptr->sx.s23.s2.varindex;
							s_lt = ls->lifetime + a;
							lt_remove_use_site(s_lt, lt->def->b_index,
											   lt->def->iindex);
							wl_add(W, a);

							/* "fall through" for handling of s1 */

							/* DF_{IINC,STORE,LOAD} are DF_{1_TO_1,MOVE,COPY} */

						case DF_1_TO_0:
						case DF_1_TO_1:
						case DF_MOVE:
						case DF_COPY:
							a = iptr->s1.varindex;
							s_lt = ls->lifetime + a;
							lt_remove_use_site(s_lt, lt->def->b_index,
											   lt->def->iindex);
							wl_add(W, a);
						}
#ifdef SSA_DEBUG_VERBOSE
						if (compileverbose) {
							printf("dce: BB%3i II%3i %s removed \n",
								   lt->def->b_index, lt->def->iindex,
								   bytecode[iptr->opc].mnemonic);
						}
#endif
					}

					if (remove_statement) {

						/* remove statement */

#ifdef SSA_DEBUG_VERBOSE
						if (compileverbose)
							printf("dce: %s %s:at BB %3i II %3i NOP-<%s\n",
								   m->clazz->name.begin(), m->name.begin(),
								   lt->def->b_index, lt->def->iindex,
								   icmd_table[iptr->opc].name);
#endif
						iptr->opc = ICMD_NOP;
  					}
				} /* (lt->def->b_index != 0) */
			} /* if (lt->def->iindex < 0 ) else */

			/* remove definition of a */

#ifdef SSA_DEBUG_VERBOSE
			if (compileverbose)
				printf("dce: var %3i removed\n", lt->v_index);
#endif
			VAR(lt->v_index)->type = (Type) jitdata::UNUSED;
			lt->type = (Type) jitdata::UNUSED;
			lt->def = NULL;
/* 			jd->var */
		} /* if (lt->use == NULL) */
	} /* while(!wl_is_empty(W)) */
} /* dead_code_elimination */

/****************************************************************************
Simple Constant Propagation
****************************************************************************/

void simple_constant_propagation() {
}

/****************************************************************************
Optimization
*******************************************************************************/
void copy_propagation(jitdata *jd, graphdata *gd) {
	int a, i, source;
	int only_source;

	worklist *W;

	instruction *iptr;
	struct lifetime *lt, *s_lt;

	lsradata *ls;

	ls = jd->ls;

	W = wl_new(ls->lifetimecount);
	if (ls->lifetimecount > 0) {
		/* put all lifetimes on Worklist */
		for(a = 0; a < ls->lifetimecount; a++) {
			if (ls->lifetime[a].type != jitdata::UNUSED) {
				wl_add(W, a);
			}
		}
	}

	while(!wl_is_empty(W)) {
		/* take a var out of Worklist */
		a = wl_get(W);

		lt = ls->lifetime + a;
		if (lt->type == jitdata::UNUSED)
			continue;
		_SSA_ASSERT(lt->def != NULL);
#if 0
		_SSA_ASSERT(lt->use != NULL);
#endif
		if (lt->def->iindex < 0 ) {

			/* phi function */
			/* look, if phi function degenerated to a x = phi(y) */
			/* and if so, substitute y for every use of x */

			_SSA_ASSERT(ls->phi[lt->def->b_index][-lt->def->iindex-1] != NULL);

			only_source = ls->varcount_with_indices;
			for (i = 1; i <= graph_get_num_predecessor(gd, lt->def->b_index);
				 i++) {
					source = ls->phi[lt->def->b_index][-lt->def->iindex-1][i];
					if ((source != ls->varcount_with_indices) &&
						(source != jitdata::UNUSED)) {
						if (only_source == ls->varcount_with_indices) {

							/* first valid source argument of phi function */

							only_source = source;
						} else {

							/* second valid source argument of phi function */
							/* exit for loop */

							only_source = ls->varcount_with_indices;
							break;
						}
					}
			}

			if (only_source != ls->varcount_with_indices) {

#ifdef SSA_DEBUG_VERBOSE
				if (compileverbose)
					printf(
						   "-- copy propagation phi-func: BB %3i II %3i: %3i -> %3i\n",
						   lt->def->b_index, lt->def->iindex,
						   only_source, lt->v_index);
#endif
				s_lt = ls->lifetime + only_source;

				/* replace all use sites of lt with the var_index only_source */

				ssa_replace_use_sites( jd, gd, lt, only_source, W);

				/* delete def of lt */

				ls->phi[lt->def->b_index][-lt->def->iindex-1] = NULL;

				/* remove this deleted use site of s_lt */

				lt_remove_use_site(s_lt, lt->def->b_index, lt->def->iindex);

				lt->def = NULL;

				/* move use sites from lt to s_lt */

				lt_move_use_sites(lt, s_lt);

				/* invalidate lt */

				lt->type = (Type) jitdata::UNUSED;
				VAR(lt->v_index)->type = (Type) jitdata::UNUSED;

				/* add s_lt again to Worklist W */

				wl_add(W, s_lt->v_index);
#ifdef SSA_DEBUG_VERBOSE
				if (compileverbose)
					_ssa_print_lt(s_lt);
#endif
			} /* if (only_source != ls->varcount_with_indices) */
		}
		else { /* if (lt->def->iindex < 0 ) */

			/* def in argument passing - no propagation possible */
			/* (there is no ICMD for this... */

			if (lt->def->b_index == 0)
				continue;

			/* def in "normal" ICMD */

			iptr = ls->basicblocks[lt->def->b_index]->iinstr +
				lt->def->iindex;

			switch(iptr->opc) {
			case ICMD_ISTORE:
			case ICMD_LSTORE:
			case ICMD_FSTORE:
			case ICMD_DSTORE:
			case ICMD_ASTORE:
			case ICMD_ILOAD:
			case ICMD_LLOAD:
			case ICMD_FLOAD:
			case ICMD_DLOAD:
			case ICMD_ALOAD:
			case ICMD_MOVE:
#ifdef SSA_DEBUG_VERBOSE
				if (compileverbose)
					printf(
						   "-- copy propagation %3i %s: BB %3i II %3i: %3i -> %3i\n",
						   iptr->opc, bytecode[iptr->opc].mnemonic,
						   lt->def->b_index, lt->def->iindex,
						   iptr->s1.varindex, iptr->dst.varindex);
#endif
				s_lt = ls->lifetime + iptr->s1.varindex;

				_SSA_ASSERT( lt->v_index == iptr->dst.varindex);

				/* replace all use sites of lt with the var_index */
				/* iptr->s1.varindex (==lt->v_index) */

				ssa_replace_use_sites(jd, gd, lt, iptr->s1.varindex, W);

				_SSA_ASSERT(lt->def->next == NULL);
				_SSA_ASSERT(s_lt->def != NULL);
				_SSA_ASSERT(s_lt->def->next == NULL);

				/* this ICMD is not a PEI -> so no danger in deleting it!     */
				/* delete def of lt (ICMD_NOP) */

				/* lt->def->iindex > 0 -> ICMD */

				iptr->opc = ICMD_NOP;

				/* remove this deleted use site of s_lt */

				lt_remove_use_site(s_lt, lt->def->b_index, lt->def->iindex);

				/* invalidate def site of lt */

				lt->def = NULL;

				/* move use sites from lt to s_lt */

				lt_move_use_sites(lt, s_lt);

				/* invalidate lt */

				lt->type = (Type) jitdata::UNUSED;
				VAR(lt->v_index)->type = (Type) jitdata::UNUSED;

				/* add s_lt again to Worklist W */
				wl_add(W, s_lt->v_index);
#ifdef SSA_DEBUG_VERBOSE
				if (compileverbose)
					_ssa_print_lt(s_lt);
#endif
				break;
			default:
				break;
			}
		} /* if (lt->def->iindex < 0 ) */
	} /* while(!wl_is_empty(W)) */
}

void ssa_replace_use_sites(jitdata *jd, graphdata *gd, struct lifetime *lt,
						   int new_v_index, worklist *W) {
	struct site *s;
	int i, source;
	instruction *iptr;
	s4 *argp;

	methoddesc *md;
	builtintable_entry *bte;
	lsradata *ls;

	ls = jd->ls;
	md = jd->m->parseddesc;


	for(s = lt->use; s != NULL; s = s->next) {
		if (s->iindex < 0) {

			/* Use in phi function */

			for (i = 1;i <= graph_get_num_predecessor(gd, s->b_index); i++) {
				source = ls->phi[s->b_index][-s->iindex-1][i];
				if (source == lt->v_index) {

					/* check if this use in this phi function is a     */
					/* "selfdefinition" (x = phi(...,x,...))           */
					/* if so replace the use with -1 (x=phi(...,-1,...)*/

					if (new_v_index == ls->phi[s->b_index][-s->iindex-1][0]) {
#ifdef SSA_DEBUG_VERBOSE
						if (W != NULL) {
							if (compileverbose)
								printf(
							 "copy propagation phi: BB %3i I %3i: %3i -> %3i\n",
							 s->b_index, s->iindex, -1, source);
						}
#endif
						ls->phi[s->b_index][-s->iindex-1][i] = -1;

						/* remove this use site of use site */
						lt_remove_use_site(lt, s->b_index, s->iindex);
					}
					else {
#ifdef SSA_DEBUG_VERBOSE
						if (W != NULL) {
							if (compileverbose)
								printf(
							 "copy propagation phi: BB %3i I %3i: %3i -> %3i\n",
							 s->b_index, s->iindex, new_v_index, source);
						}
#endif
						ls->phi[s->b_index][-s->iindex-1][i] = new_v_index;
					}
				}
			}
			if (W != NULL) {

				/* Add var, which is defined by this phi function to */
				/* the worklist */

				source = ls->phi[s->b_index][-s->iindex-1][0];
				wl_add(W, source);
			}
		}
		else { /* use in ICMD */

			iptr = ls->basicblocks[s->b_index]->iinstr +
				s->iindex;

		/* check for use (s1, s2, s3 or special (argp) ) */

			i = jitdata::UNUSED;
			switch (icmd_table[iptr->opc].dataflow) {
			case DF_3_TO_0:
			case DF_3_TO_1: /* icmd has s1, s2 and s3 */
				if (iptr->sx.s23.s3.varindex == lt->v_index) {
#ifdef SSA_DEBUG_VERBOSE
					if (W != NULL) {
						if (compileverbose)
							printf("copy propagation loc3: BB %3i I %3i: %3i -> \
                                    %3i\n", s->b_index, s->iindex,
								   new_v_index, iptr->sx.s23.s3.varindex);
					}
#endif
					iptr->sx.s23.s3.varindex = new_v_index;
				}

				/* now "fall through" for handling of s2 and s1 */

			case DF_2_TO_0:
			case DF_2_TO_1: /* icmd has s1 and s2 */
				if (iptr->sx.s23.s2.varindex == lt->v_index) {
#ifdef SSA_DEBUG_VERBOSE
					if (W != NULL) {
						if (compileverbose)
							printf("copy propagation loc2: BB %3i I %3i: %3i -> \
                                    %3i\n", s->b_index, s->iindex,
								   new_v_index, iptr->sx.s23.s2.varindex);
					}
#endif
					iptr->sx.s23.s2.varindex = new_v_index;
				}

				/* now "fall through" for handling of s1 */

			case DF_1_TO_0:
			case DF_1_TO_1:
			case DF_MOVE:
			case DF_COPY:
				if (iptr->s1.varindex == lt->v_index) {
#ifdef SSA_DEBUG_VERBOSE
					if (W != NULL) {
						if (compileverbose)
							printf(
							"copy propagation loc1: BB %3i I %3i: %3i -> %3i\n",
							s->b_index, s->iindex, new_v_index,
							iptr->s1.varindex);
					}
#endif
					iptr->s1.varindex = new_v_index;
				}
				break;

				/* TODO: */
				/* ? really look at md->paramcount or iptr->s1.argcount */
				/* has to be taken, so that pass-through stackslots get */
				/* renamed, too? */

			case DF_INVOKE:
				INSTRUCTION_GET_METHODDESC(iptr,md);
				i = md->paramcount;
				break;
			case DF_BUILTIN:
				bte = iptr->sx.s23.s3.bte;
				md = bte->md;
				i = md->paramcount;
				break;
			case DF_N_TO_1:
				i = iptr->s1.argcount;
				break;
			}
			if (i != jitdata::UNUSED) {
				argp = iptr->sx.s23.s2.args;
				while (--i >= 0) {
					if (*argp == lt->v_index) {
#ifdef SSA_DEBUG_VERBOSE
						if (W != NULL) {
							if (compileverbose)
								printf(
									   "copy propagation locN: BB %3i I %3i: %3i -> %3i\n"
									   , s->b_index, s->iindex, new_v_index, *argp);
						}
#endif
						*argp = new_v_index;

					}
					argp++;
				}
			}

		} /* if (s->iindex < 0) */
	} /* for(s = lt->use; s != NULL; s = s->next) */
}

#ifdef SSA_DEBUG_VERBOSE
void ssa_print_lt(lsradata *ls) {
	int i;
	struct lifetime *lt;

	printf("SSA LT Def/Use\n");
	for(i = 0; i < ls->lifetimecount; i++) {
		lt = ls->lifetime + i;
		_ssa_print_lt(lt);
	}
}

void _ssa_print_lt(struct lifetime *lt) {
	struct site *use;

	if (lt->type != jitdata::UNUSED) {
		printf("VI %3i Type %3i Def: ",lt->v_index, lt->type);
		if (lt->def != NULL)
			printf("%3i,%3i Use: ",lt->def->b_index, lt->def->iindex);
		else
			printf("%3i,%3i Use: ",0,0);
		for(use = lt->use; use != NULL; use = use->next)
			printf("%3i,%3i ",use->b_index, use->iindex);
		printf("\n");
	}
}

void ssa_show_variable(jitdata *jd, int index, varinfo *v, int stage)
{
	char type;
	char kind;

	switch (v->type) {
		case TYPE_INT: type = 'i'; break;
		case TYPE_LNG: type = 'l'; break;
		case TYPE_FLT: type = 'f'; break;
		case TYPE_DBL: type = 'd'; break;
		case TYPE_ADR: type = 'a'; break;
		case TYPE_RET: type = 'r'; break;
		default:       type = '?';
	}

	if (index < jd->localcount) {
		kind = 'L';
		if (v->flags & (PREALLOC | INOUT))
				printf("<INVALID FLAGS!>");
	}
	else {
		if (v->flags & PREALLOC) {
			kind = 'A';
			if (v->flags & INOUT)
				printf("<INVALID FLAGS!>");
		}
		else if (v->flags & INOUT)
			kind = 'I';
		else
			kind = 'T';
	}

	printf("%c%c%3d", kind, type, index);

	if (v->flags & SAVEDVAR)
		putchar('!');

	putchar(' ');
	fflush(stdout);
}
#endif

/****************************************************************************
Optimization
****************************************************************************/

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
