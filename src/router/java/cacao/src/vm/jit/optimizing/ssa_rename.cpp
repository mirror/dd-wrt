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
#include "vm/jit/builtin.hpp"

#include "vm/jit/code.hpp"
#include "vm/jit/jit.hpp" /* icmd_table */

#include "vm/jit/optimizing/dominators.hpp"
#include "vm/jit/optimizing/graph.hpp"
#include "vm/jit/optimizing/lifetimes.hpp"
#include "vm/jit/optimizing/lsra.hpp"
#include "vm/jit/optimizing/ssa.hpp"
#include "vm/jit/optimizing/ssa_rename.hpp"

#if defined(SSA_DEBUG_VERBOSE)
#include "vm/options.hpp"   /* compileverbose */
#endif

/* function prototypes */
void ssa_rename_(jitdata *jd,  graphdata *gd, dominatordata *dd, int n);
int ssa_rename_def_(lsradata *ls, int a);

/* ssa_rename ******************************************************************

Rename the variables a (0 <= a < ls->ssavarcount) so that each new variable
has only one definition (SSA form).

ls->def_count[0..ls->ssavarcount[ holds the number of definitions of each var.
ls->var_0[0..ls->ssavarcount[ will be set to the new index of the first
                              definition of each old var.
ls->varcount_with_indices     will be se to the new maximum varcount of LOCAL
                              and IOVARS.

All other vars (TEMPVAR and PREALLOC) will get a new unique index above
ls->varcount_with_indices.

jd->var and jd->varcount will be set for this renamed vars.

*******************************************************************************/

void ssa_rename(jitdata *jd, graphdata *gd, dominatordata *dd)
{
	int i, mi, l, j, p, t;
	Type type;
	int flags;
	methoddesc *md = jd->m->parseddesc;

	varinfo *new_vars;
	lsradata *ls;

	ls = jd->ls;

	ssa_rename_init(jd, gd);

	/* Consider definition of Local Vars initialized with Arguments */
	/* in Block 0 */
	/* init is regarded as use too-> ssa_rename_use ->bullshit!!*/

 	for (p = 0, l= 0; p < md->paramcount; p++) {
 		t = md->paramtypes[p].type;
		mi = l * 5 + t;
		i = jd->local_map[mi];
		l++;
		if (IS_2_WORD_TYPE(t))
			l++;
		if (i == jitdata::UNUSED)
			continue;

		/* !!!!! locals are now numbered as the parameters !!!!       */
		/* !!!!! no additional increment for 2 word types !!!!!       */
		/* this happens later on! here we still need the increment    */
		/* index of var can be in the range from 0 up to not including*/
		/* jd->varcount */


		i = ls->new_varindex[i];

		/* ignore return value, since first definition gives 0 -> */
		/* no rename necessary */

		j = ssa_rename_def_(ls, i);
		_SSA_ASSERT(j == 0);
		jd->local_map[mi] = i;
	}
	ssa_rename_(jd, gd, dd, 0);

#if 0
	/* DO _NOT_ DO THIS! Look at java.util.stringtokenizer.counttokens!   */
	/* if there is no use of the defined Var itself by the phi function   */
	/* for a loop path, in which this var is not used, it will not be life*/
	/* in this path and overwritten! */

	/* Invalidate all xij from xi0=phi(xi1,xi2,xi3,..,xin) with xij == xi0*/
	/* this happens if the phi function is the first definition of x or in*/
	/* a path with a backedge xi has no definition */
	/* a phi(xij) = ...,xij,... with the only use and definition of xij by*/
	/* this phi function would otherwise "deadlock" the dead code         */
	/* elemination (invalidate means set it to UNUSED)                    */
	/* a phi function phi(xi0)=xi1,xi2,...xin wiht xij == xi0 for all j in*/
	/* [1,n] can be removed */

	for(i = 0; i < ls->ssavarcount; i++) {
	  for(t = 0; t < ls->basicblockcount; t++) {
	    if (ls->phi[t][i] != 0) {
	      remove_phi = true;
	      for(p = 1; p <= graph_get_num_predecessor(gd, t); p++) {
		if (ls->phi[t][i][0] == ls->phi[t][i][p])
		  ls->phi[t][i][p] = jitdata::UNUSED;
		else
		  remove_phi = false;
	      }
	    }
	    if (remove_phi)
	      ls->phi[t][i] = NULL;
	  }
	}
#endif

#if defined(SSA_DEBUG_CHECK) || defined(SSA_DEBUG_VERBOSE)
# if defined(SSA_DEBUG_VERBOSE)
	if (compileverbose) {
		printf("%s %s ", jd->m->clazz->name.begin(), jd->m->name.begin());
		if (code_is_leafmethod(jd->code))
			printf("**Leafmethod**");
		printf("\n");
	}
# endif
	if (strcmp(jd->m->clazz->name.begin(),"fp")==0)
		if (strcmp(jd->m->name.begin(),"testfloat")==0)
# if defined(SSA_DEBUG_VERBOSE)
			if (compileverbose)
				printf("12-------------------12\n");
# else
	        { int dummy=1; dummy++; }
# endif
#endif

	/* recreate rd->locals[][] */
	/* now only one (local_index/type) pair exists anymore     */
	/* all var[t][i] with var_to_index[var[t][i]] >= 0 are locals */
	/* max local index after SSA indexing is in ls->local_0[ls->max_locals] */

	new_vars = DMNEW(varinfo, ls->vartop);
	for(i = 0; i < ls->vartop ; i++)
		new_vars[i].type = (Type) jitdata::UNUSED;
	for(i = 0; i < jd->varcount; i++) {
			p = ls->new_varindex[i];
			if (p != jitdata::UNUSED) {
				if (p < ls->ssavarcount)
					p = ls->var_0[p];
				new_vars[p].type  = VAR(i)->type;
				new_vars[p].flags = VAR(i)->flags;
				ls->lifetime[p].v_index = p;
				ls->lifetime[p].type = VAR(i)->type;
			}
	}

	/* take care of newly indexed local & in/out vars */

	for(i = 0; i < ls->ssavarcount; i++) {
		j = ls->var_0[i];
		type = new_vars[j].type;
		flags = new_vars[j].flags;
		j++;
		for (; j < ls->var_0[i + 1]; j++) {
			new_vars[j].type = type;
			new_vars[j].flags = flags;
			ls->lifetime[j].v_index = j;
			ls->lifetime[j].type = type;
		}
	}

#ifdef SSA_DEBUG_VERBOSE
	if (compileverbose) {

		printf("ssa_rename: Vars: Orig:%3i SSAVar: %3i\n", jd->varcount,
			   ls->ssavarcount);
		for(i = 0; i < jd->varcount; i++) {
			printf("%3i(%3i,%3x) ",i,VAR(i)->type, VAR(i)->flags);
			ssa_show_variable(jd, i, VAR(i),0);
			j = ls->new_varindex[i];
			if ((j != jitdata::UNUSED) && (j < ls->ssavarcount))
				printf(" -> %3i ... %3i", ls->var_0[j], ls->var_0[j + 1] - 1);
			else
				printf(" -> %3i", j);
			printf("\n");
		}
	}
#endif

 	jd->var = new_vars;
 	jd->varcount = ls->vartop;
	jd->vartop = ls->vartop;

#ifdef SSA_DEBUG_VERBOSE
	if (compileverbose) {
		printf("ssa_rename: Vars: Orig:%3i SSAVar: %3i\n", jd->varcount,
			   ls->ssavarcount);
		for(i = 0; i < jd->varcount; i++) {
			printf("%3i(%3i,%3x) ",i,VAR(i)->type, VAR(i)->flags);
			ssa_show_variable(jd, i, VAR(i),0);
			printf("\n");
		}
	}
#endif
}

/* ssa_rename_init *************************************************************

Setup the data structure for ssa_rename

ls->def_count[0..ls->ssavarcount[ holds the number of definitions of each var.
ls->var_0[0..ls->ssavarcount[ will be set to the new index of the first
                              definition of each old var.
ls->varcount_with_indices     will be se to the new maximum varcount of LOCAL
                              and IOVARS.

All other vars (TEMPVAR and PREALLOC) will get a new unique index above
ls->varcount_with_indices.

jd->var and jd->varcount will be set for this renamed vars.

*******************************************************************************/

void ssa_rename_init(jitdata *jd, graphdata *gd)
{
	int a, i;
#if 0
	int p;
#endif
	lsradata *ls;

	ls = jd->ls;

	/* set up new locals */
	/* ls->new_varindex[0..jd->varcount[ holds the new unique index */
	/* for locals and iovars */

	/* ls->num_defs[index] gives the number of indizes which will be created  */
	/* from SSA */

	/* -> vars will be numbered in this sequence: L0(0)..L0(i) L1(0)..L1(j) ..*/
	/* ls->var_0[X] will point to each LX(0)  */
	/* ls->var_0[ls->ssavarcount] will hold ls->varcount_with_indices */

	/* as first step cummulate the num_defs array for locals and iovars       */
	/* last element is the maximum var count */

	ls->var_0 = DMNEW(int, MAX(1, ls->ssavarcount + 1));
	ls->var_0[0] = 0;
	ls->varcount_with_indices = 0;
	for(i = 0; i < ls->ssavarcount; i++) {
		ls->varcount_with_indices += ls->num_defs[i];
		ls->var_0[i+1] = ls->var_0[i] + ls->num_defs[i];
	}

#if 0
	/* Change the var indices in phi from La to La(0) */

	for(i = 0; i < ls->basicblockcount; i++)
		for (a = 0; a < ls->ssavarcount; a++)
			if (ls->phi[i][a] != NULL)
				for(p = 0; p < graph_get_num_predecessor(gd, i) + 1; p++)
					ls->phi[i][a][p] = ls->var_0[a];
#endif

	/* Initialization */

	ls->count     = DMNEW(int,   MAX(1, ls->ssavarcount));
	ls->stack     = DMNEW(int *, MAX(1, ls->ssavarcount));
	ls->stack_top = DMNEW(int,   MAX(1, ls->ssavarcount));
	for(a = 0; a < ls->ssavarcount; a++) {
		ls->count[a] = 0;
		ls->stack_top[a] = 0;

		/* stack a has to hold number of defs of a Elements + 1 */

		ls->stack[a] = DMNEW(int, ls->num_defs[a] + 1);
		ls->stack[a][ls->stack_top[a]++] = 0;
	}

	if (ls->ssavarcount > 0) {

		/* Create the num_var_use Array */

		ls->num_var_use = DMNEW(int *, ls->basicblockcount);
		for(i = 0; i < ls->basicblockcount; i++) {
			ls->num_var_use[i] =DMNEW(int, MAX(1, ls->varcount_with_indices));
			for(a = 0; a < ls->varcount_with_indices; a++)
				ls->num_var_use[i][a] = 0;
		}

		/* Create the use_sites Array of Bitvectors*/
		/* use max(1,..), to ensure that the array is created! */

		ls->use_sites =  DMNEW(bitvector, MAX(1, ls->varcount_with_indices));
		for(a = 0; a < ls->varcount_with_indices; a++)
			ls->use_sites[a] = bv_new(ls->basicblockcount);
	}

	/* init lifetimes */
	/* count number of TEMPVARs */

	ls->lifetimecount = 0;
	for(i = 0; i < jd->varcount; i++)
		if ((i >= jd->localcount) || (!(jd->var[i].flags & (INOUT | PREALLOC))))
			ls->lifetimecount++;

	ls->varcount = ls->varcount_with_indices + ls->lifetimecount;

	ls->lifetimecount = ls->varcount;
	ls->lifetime = DMNEW(struct lifetime, ls->lifetimecount);
	ls->lt_used = DMNEW(int, ls->lifetimecount);
	ls->lt_int = DMNEW(int, ls->lifetimecount);
	ls->lt_int_count = 0;
	ls->lt_flt = DMNEW(int, ls->lifetimecount);
	ls->lt_flt_count = 0;
	ls->lt_mem = DMNEW(int, ls->lifetimecount);
	ls->lt_mem_count = 0;
	for (i=0; i < ls->lifetimecount; i++) {
		ls->lifetime[i].type = jitdata::UNUSED;
		ls->lifetime[i].savedvar = 0;
		ls->lifetime[i].flags = 0;
		ls->lifetime[i].usagecount = 0;
		ls->lifetime[i].bb_last_use = -1;
		ls->lifetime[i].bb_first_def = -1;
		ls->lifetime[i].use = NULL;
		ls->lifetime[i].def = NULL;
		ls->lifetime[i].last_use = NULL;
	}

	/* for giving TEMP and PREALLOC vars a new unique index */

	ls->vartop = ls->varcount_with_indices;

#ifdef SSA_DEBUG_VERBOSE
	if (compileverbose) {
		printf("ssa_rename_init: Vars: Orig:%3i SSAVar: %3i\n", jd->varcount,
			   ls->ssavarcount);
		for(i = 0; i < jd->varcount; i++) {
			if ((i < jd->localcount) || ( VAR(i)->flags & INOUT)) {
				printf("%3i(%3i,%3x) ",i,VAR(i)->type, VAR(i)->flags);
				ssa_show_variable(jd, i, VAR(i),0);
				if ((i < ls->ssavarcount) || (VAR(i)->flags & INOUT)) {
					printf(" -> %3i", ls->new_varindex[i]);
				}
				printf("\n");
			}
		}
		ssa_print_phi(ls, gd);
	}
#endif
}

int ssa_rename_def_(lsradata *ls, int a) {
	int i;

	_SSA_CHECK_BOUNDS(a,0,ls->ssavarcount);
	ls->count[a]++;
	i = ls->count[a] - 1;
	/* push i on stack[a] */
	_SSA_CHECK_BOUNDS(ls->stack_top[a], 0, ls->num_defs[a] + 1);
	ls->stack[a][ls->stack_top[a]++] = i;
	return i;
}

int ssa_rename_def(jitdata *jd, int *def_count, int a) {
	int i, a1, ret;
	lsradata *ls;

	ls = jd->ls;

	a1 = ls->new_varindex[a];
	_SSA_CHECK_BOUNDS(a1, jitdata::UNUSED, ls->varcount);
	if ((a1 != jitdata::UNUSED) && (a1 < ls->ssavarcount)) {
		/* local or inoutvar -> normal ssa renaming */
		_SSA_ASSERT((a < jd->localcount) || (VAR(a)->flags & INOUT));
		/* !IS_TEMPVAR && !IS_PREALLOC == (IS_LOCALVAR || IS_INOUT) */

		def_count[a1]++;
		i = ssa_rename_def_(ls, a1);
		ret = ls->var_0[a1] + i;
	}
	else {
		/* TEMP or PREALLOC var */
		if (a1 == jitdata::UNUSED) {
			ls->new_varindex[a] = ls->vartop;
			ret = ls->vartop;
			ls->vartop++;
			_SSA_ASSERT( ls->vartop < ls->varcount);
		}
		else
			ret = a1;
	}
	return ret;
}

void ssa_rename_use_(lsradata *ls, int n, int a) {
	_SSA_CHECK_BOUNDS(a, 0, ls->varcount_with_indices);
	if (ls->ssavarcount > 0) {
		bv_set_bit(ls->use_sites[a], n);
		ls->num_var_use[n][a]++;
	}
}

int ssa_rename_use(lsradata *ls, int n, int a) {
	int a1, i;
	int ret;

	a1 = ls->new_varindex[a];
	_SSA_CHECK_BOUNDS(a1, jitdata::UNUSED, ls->varcount);
	if ((a1 != jitdata::UNUSED) && (a1 < ls->ssavarcount)) {
		/* local or inoutvar -> normal ssa renaming */
		/* i <- top(stack[a]) */

		_SSA_CHECK_BOUNDS(ls->stack_top[a1]-1, 0, ls->num_defs[a1]+1);
		i = ls->stack[a1][ls->stack_top[a1] - 1];
		_SSA_CHECK_BOUNDS(i, 0, ls->num_defs[a1]);

		ret = ls->var_0[a1] + i;
	}
	else {
		/* TEMP or PREALLOC var */
		if (a1 == jitdata::UNUSED) {
			ls->new_varindex[a] = ls->vartop;
			ret = ls->vartop;
			ls->vartop++;
			_SSA_ASSERT( ls->vartop < ls->varcount);
		}
		else
			ret = a1;
	}

	return ret;
}

#ifdef SSA_DEBUG_VERBOSE
void ssa_rename_print(instruction *iptr, char *op, int from,  int to) {
	if (compileverbose) {
		printf("ssa_rename_: ");
		if (iptr != NULL)
			printf("%s ", bytecode[iptr->opc].mnemonic);
		else
			printf("       ");

		printf("%s: %3i->%3i\n", op, from, to);
	}
}
#endif

/* ssa_rename_ ****************************************************************

Algorithm is based on "modern compiler implementation in C" from andrew
w. appel, edition 2004

page 443 Algorithm 19.7. Renaming Variables

*******************************************************************************/

void ssa_rename_(jitdata *jd, graphdata *gd, dominatordata *dd, int n) {
	int a, i, j, k, iindex, Y, v;
	int in_d, out_d;
	int *def_count;
	/* [0..ls->varcount[ Number of Definitions of this var in this  */
	/* Basic Block. Used to remove the entries off the stack at the */
	/* end of the function */
	instruction *iptr;
	s4 *in, *out, *argp;
	graphiterator iter_succ, iter_pred;
	struct lifetime *lt;

	methoddesc *md;
	methodinfo *m;
	builtintable_entry *bte;
	lsradata *ls;

	ls = jd->ls;
	m  = jd->m;

#ifdef SSA_DEBUG_VERBOSE
	if (compileverbose)
		printf("ssa_rename_: BB %i\n",n);
#endif

	_SSA_CHECK_BOUNDS(n, 0, ls->basicblockcount);

	def_count = DMNEW(int, MAX(1, ls->ssavarcount));
	for(i = 0; i < ls->ssavarcount; i++)
		def_count[i] = 0;

	/* change Store of possible phi functions from a to ai*/

	for(a = 0; a < ls->ssavarcount; a++)
		if (ls->phi[n][a] != NULL) {
			def_count[a]++;

			/* do not mark this store as use - maybe this phi function */
			/* can be removed for unused Vars*/

			j = ls->var_0[a] + ssa_rename_def_(ls, a);
#ifdef SSA_DEBUG_VERBOSE
			ssa_rename_print( NULL, "phi-st", ls->phi[n][a][0], j);
#endif
			ls->phi[n][a][0] = j;
		}

	in   = ls->basicblocks[n]->invars;
	in_d = ls->basicblocks[n]->indepth - 1;

	/* change use of instack Interface stackslots except top SBR and EXH */
	/* stackslots */

	if ((ls->basicblocks[n]->type == basicblock::TYPE_EXH) ||
		(ls->basicblocks[n]->type == basicblock::TYPE_SBR)) {
		in_d--;
	}
/* 	out   = ls->basicblocks[n]->outvars; */
/* 	out_d = ls->basicblocks[n]->outdepth - 1; */

/* 	for(; out_d > in_d; out_d--); */

	for (; in_d >= 0; in_d--) {
		/* Possible Use of ls->new_varindex[jd->var[in_d]] */
		_SSA_ASSERT(ls->new_varindex[in[in_d]] != jitdata::UNUSED);

		a = ls->new_varindex[in[in_d]];
		_SSA_CHECK_BOUNDS(a, 0, ls->ssavarcount);

		/* i <- top(stack[a]) */

		_SSA_CHECK_BOUNDS(ls->stack_top[a]-1, 0, ls->num_defs[a]+1);
		i = ls->stack[a][ls->stack_top[a]-1];
		_SSA_CHECK_BOUNDS(i, 0, ls->num_defs[a]);

		/* Replace use of x with xi */

#ifdef SSA_DEBUG_VERBOSE
			ssa_rename_print( NULL, "invar", in[in_d], ls->var_0[a]+i);
#endif
		in[in_d] = ls->var_0[a] + i;
		lt = ls->lifetime + in[in_d];

		lt->v_index = in[in_d];
		lt->bb_last_use = -1;
	}

	iptr = ls->basicblocks[n]->iinstr;

	for(iindex = 0; iindex < ls->basicblocks[n]->icount; iindex++, iptr++) {

		/* check for use (s1, s2, s3 or special (argp) ) */

		switch (icmd_table[iptr->opc].dataflow) {
		case DF_3_TO_0:
		case DF_3_TO_1: /* icmd has s1, s2 and s3 */
			j = ssa_rename_use(ls, n, iptr->sx.s23.s3.varindex);
#ifdef SSA_DEBUG_VERBOSE
			ssa_rename_print( iptr, "s3 ", iptr->sx.s23.s3.varindex, j);
#endif
			iptr->sx.s23.s3.varindex = j;

			/* now "fall through" for handling of s2 and s1 */

		case DF_2_TO_0:
		case DF_2_TO_1: /* icmd has s1 and s2 */
			j = ssa_rename_use(ls, n, iptr->sx.s23.s2.varindex);
#ifdef SSA_DEBUG_VERBOSE
			ssa_rename_print( iptr, "s2 ", iptr->sx.s23.s2.varindex, j);
#endif
			iptr->sx.s23.s2.varindex = j;

			/* now "fall through" for handling of s1 */

		case DF_1_TO_0:
		case DF_1_TO_1:
		case DF_MOVE:
		case DF_COPY:
			j = ssa_rename_use(ls, n, iptr->s1.varindex);
#ifdef SSA_DEBUG_VERBOSE
			ssa_rename_print( iptr, "s1 ", iptr->s1.varindex, j);
#endif
			iptr->s1.varindex = j;
			break;

		case DF_INVOKE:
		case DF_BUILTIN:
		case DF_N_TO_1:
			/* do not use md->paramcount, pass-through stackslots have */
			/* to be renamed, too */
			i = iptr->s1.argcount;
			argp = iptr->sx.s23.s2.args;
			while (--i >= 0) {
				j = ssa_rename_use(ls, n, *argp);
#ifdef SSA_DEBUG_VERBOSE
				ssa_rename_print( iptr, "arg", *argp, j);
#endif
				*argp = j;
				argp++;
			}
			break;
		}


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
			j = ssa_rename_def(jd, def_count, iptr->dst.varindex);
#ifdef SSA_DEBUG_VERBOSE
			ssa_rename_print( iptr, "dst", iptr->dst.varindex, j);
#endif
			iptr->dst.varindex = j;
		}

				/* ?????????????????????????????????????????????????????????? */
				/* Mark Def as use, too. Since param initialisation is in     */
				/* var_def and this would not remove this locals, if not used */
				/* elsewhere   */
				/* ?????????????????????????????????????????????????????????? */

	}

	/* change outstack Interface stackslots */
	out = ls->basicblocks[n]->outvars;
	out_d = ls->basicblocks[n]->outdepth - 1;

	for (;out_d >= 0; out_d--) {
		/* Possible Use of ls->new_varindex[jd->var[out_d]] */
		_SSA_ASSERT(ls->new_varindex[out[out_d]] != jitdata::UNUSED);

		a = ls->new_varindex[out[out_d]];
		_SSA_CHECK_BOUNDS(a, 0, ls->ssavarcount);

		/* i <- top(stack[a]) */

		_SSA_CHECK_BOUNDS(ls->stack_top[a]-1, 0, ls->num_defs[a]+1);
		i = ls->stack[a][ls->stack_top[a]-1];
		_SSA_CHECK_BOUNDS(i, 0, ls->num_defs[a]);

		/* Replace use of x with xi */

#ifdef SSA_DEBUG_VERBOSE
			ssa_rename_print( NULL, "outvar", out[out_d], ls->var_0[a]+i);
#endif
		out[out_d] = ls->var_0[a] + i;
		lt = ls->lifetime + out[out_d];

		lt->v_index = out[out_d];
		lt->bb_last_use = -1;
	}

	/* change use in phi Functions of Successors */

	Y = graph_get_first_successor(gd, n, &iter_succ);
	for(; Y != -1; Y = graph_get_next(&iter_succ)) {
		_SSA_CHECK_BOUNDS(Y, 0, ls->basicblockcount);
		k = graph_get_first_predecessor(gd, Y, &iter_pred);
		for (j = 0; (k != -1) && (k != n); j++)
			k = graph_get_next(&iter_pred);
		_SSA_ASSERT(k == n);

		/* n is jth Predecessor of Y */

		for(a = 0; a < ls->ssavarcount; a++)
			if (ls->phi[Y][a] != NULL) {

				/* i <- top(stack[a]) */

				if (ls->stack_top[a] == 1) {
					/* no definition till now in controll flow */
#ifdef SSA_DEBUG_VERBOSE
					if (compileverbose) {
						printf("Succ %3i Arg %3i \n", Y, j);
						ssa_rename_print( NULL, "phi-use", ls->phi[Y][a][j+1], jitdata::UNUSED);
					}
#endif
					ls->phi[Y][a][j+1] = jitdata::UNUSED;
				}
				else {
					_SSA_CHECK_BOUNDS(ls->stack_top[a]-1, 0, ls->num_defs[a]+1);
					i = ls->stack[a][ls->stack_top[a]-1];
					_SSA_CHECK_BOUNDS(i, 0, ls->num_defs[a]);

					/* change jth operand from a0 to ai */

					i = ls->var_0[a] + i;
#ifdef SSA_DEBUG_VERBOSE
					if (compileverbose) {
						printf("Succ %3i Arg %3i \n", Y, j);
						ssa_rename_print( NULL, "phi-use", ls->phi[Y][a][j+1], i);
					}
#endif
					ls->phi[Y][a][j+1] = i;
					_SSA_CHECK_BOUNDS(ls->phi[Y][a][j+1], 0,
									  ls->varcount_with_indices);

					/* use by phi function has to be remembered, too */

					ssa_rename_use_(ls, n, ls->phi[Y][a][j+1]);
				}

				/* ????????????????????????????????????????????? */
				/* why was this only for local vars before ?     */
				/* ????????????????????????????????????????????? */

			}
	}

	/* Call ssa_rename_ for all Childs of n of the Dominator Tree */
	for(i = 0; i < ls->basicblockcount; i++)
		if (dd->idom[i] == n)
			ssa_rename_(jd, gd, dd, i);

	/* pop Stack[a] for each definition of a var a in the original S */
	for(a = 0; a < ls->ssavarcount; a++) {
		ls->stack_top[a] -= def_count[a];
		_SSA_ASSERT(ls->stack_top[a] >= 0);
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
