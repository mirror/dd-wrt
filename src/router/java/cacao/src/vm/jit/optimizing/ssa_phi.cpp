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

#include "vm/jit/builtin.hpp"

#include "vm/jit/jit.hpp" /* icmd_table */

#include "vm/jit/optimizing/dominators.hpp"
#include "vm/jit/optimizing/graph.hpp"
#include "vm/jit/optimizing/lifetimes.hpp"
#include "vm/jit/optimizing/lsra.hpp"
#include "vm/jit/optimizing/ssa.hpp"
#include "vm/jit/optimizing/ssa_phi.hpp"

#if defined(SSA_DEBUG_VERBOSE)
#include "vm/options.hpp"   /* compileverbose */
#endif

/* ssa_place_phi_functions *****************************************************

Algorithm is based on "modern compiler implementation in C" from andrew
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


ls->phi[n][a][p] is created and populated.

n in [0..ls->basicblockcount[
a in [0..ls->ssavarcount[
p in [1..Count of Predecessors of Basicblock n]

For each basicblock Y in the dominance frontier of a basicblock n (0 <= n <
ls->basicblockcount) in which a variable (0 <= a < ls->ssavarcount) is defined
an entry in ls->phi[Y][a] is created.
This entry is an array with the number of predecessors of Y elements + 1
elements.
This elements are all set to the variable a and represent the phi function which
will get ai = phi(ai1, ai2, ..., aip) after ssa_rename.


ls->phi[n][a] == NULL, if no phi function for Variable a for Basicblock n exists

or

ls->phi[n][a][0] == a, representing the result of the phi function and
ls->phi[n][a][p] == a, representing the arguments of the phi function.

*******************************************************************************/


void ssa_place_phi_functions(jitdata *jd, graphdata *gd, dominatordata *dd)
{
	int a,i,j,n,Y;
	bitvector *def_sites;
	bitvector *A_phi;    /* [0..ls->basicblockcount[ of */
	                     /* ls->ssavarcount Bit         */
	worklist *W;
	int num_pred;

	lsradata *ls;

	ls = jd->ls;

	W = wl_new(ls->basicblockcount);

	def_sites = DMNEW(bitvector, ls->ssavarcount);
	for(a = 0; a < ls->ssavarcount; a++)
		def_sites[a] = bv_new(ls->basicblockcount);

	ls->phi = DMNEW(int **, ls->basicblockcount);
	A_phi = DMNEW(bitvector, ls->basicblockcount);
	for(i = 0; i < ls->basicblockcount; i++) {
		ls->phi[i] = DMNEW(int *, ls->ssavarcount);
		for(j = 0; j < ls->ssavarcount; j++)
			ls->phi[i][j] = NULL;
		A_phi[i] = bv_new(ls->ssavarcount);
	}

	/* copy var_def to def_sites */
	/* var_def is valid for 0.. jd->basicblockcount (bb 0 for param init) */

	for(n = 0; n <= jd->basicblockcount; n++)
		for(a = 0; a < ls->ssavarcount; a++)
			if (bv_get_bit(ls->var_def[n], a))
				bv_set_bit(def_sites[a], n);
#ifdef SSA_DEBUG_VERBOSE
	if (compileverbose) {
		printf("var Definitions:\n");
		for(i = 0; i < ls->ssavarcount; i++) {
			printf("def_sites[%3i]=%p:",i,(void *)def_sites[i]);
			for(j = 0; j < ls->basicblockcount; j++) {
				if ((j % 5) == 0) printf(" ");
				if (bv_get_bit(def_sites[i], j))
					printf("1");
				else
					printf("0");
			}
			printf(" (");

			printf("\n");
		}
	}
#endif

	for(a = 0; a < ls->ssavarcount; a++) {

		/* W<-def_sites(a) */

		for(n = 0; n < ls->basicblockcount; n++)
			if (bv_get_bit(def_sites[a],n)) {
				wl_add(W, n);
			}

		while (!wl_is_empty(W)) { /* W not empty */
			n = wl_get(W);

			for(i = 0; i < dd->num_DF[n]; i++) {
				Y = dd->DF[n][i];

				/* Node Y is in Dominance Frontier of n -> */
				/* insert phi function for a at top of Y*/

				_SSA_CHECK_BOUNDS(Y, 0, ls->basicblockcount);
				if (bv_get_bit( A_phi[Y], a) == 0) {

					/* a is not a Element of A_phi[Y] */
					/* a <- phi(a,a...,a) to be inserted at top of Block Y */
					/* phi has as many arguments, as Y has predecessors    */

					num_pred =  graph_get_num_predecessor(gd, Y);
					ls->phi[Y][a] = DMNEW(int, num_pred + 1);
					for (j = 0; j < num_pred + 1; j++)
						ls->phi[Y][a][j] = a;

					/* increment the number of definitions of a by one */
					/* for this phi function */

					ls->num_defs[a]++;

					bv_set_bit(A_phi[Y], a);
					if (bv_get_bit(ls->var_def[Y],a)==0) {

						/* Iterated Dominance Frontier Criterion:  */
						/* if Y had no definition for a, insert it */
						/* into the Worklist, since now it         */
						/* defines a through the phi function      */

						wl_add(W, Y);
					}
				}
			}
		}
	}
}

void ssa_generate_phi_moves(jitdata *jd, graphdata *gd) {
	int a, i, j, pred;
	graphiterator iter;
	lsradata *ls;

	ls = jd->ls;

	/* count moves to be inserted at the end of each block in moves[] */

	ls->num_phi_moves = DMNEW(int, ls->basicblockcount);
	for(i = 0; i < ls->basicblockcount; i++)
		ls->num_phi_moves[i] = 0;
	for(i = 0; i < ls->basicblockcount; i++)
		for(a = 0; a < ls->ssavarcount; a++)
			if (ls->phi[i][a] != NULL) {
#if 0
				if (ls->lifetime[ls->phi[i][a][0]].use == NULL) {
					/* Var defined (only <- SSA Form!) in this phi function */
					/* and not used anywhere -> delete phi function and set */
					/* var to unused */

					/* TODO: first delete use sites of arguments of phi */
					/* function */
					VAR(ls->lifetime[ls->phi[i][a][0]].v_index)->type = jitdata::UNUSED;
					ls->lifetime[ls->phi[i][a][0]].def = NULL;
					ls->phi[i][a] = NULL;
				}
				else
#endif
					{
					pred = graph_get_first_predecessor(gd, i, &iter);
					for(; pred != -1; pred = graph_get_next(&iter)) {
						ls->num_phi_moves[pred]++;
					}
				}
			}

	/* allocate ls->phi_moves */

	ls->phi_moves = DMNEW( int **, ls->basicblockcount);
	for(i = 0; i < ls->basicblockcount; i++) {
		ls->phi_moves[i] = DMNEW( int *, ls->num_phi_moves[i]);
		for(j = 0; j <ls->num_phi_moves[i]; j++)
			ls->phi_moves[i][j] = DMNEW(int, 2);
#ifdef SSA_DEBUG_VERBOSE
		if (compileverbose)
			printf("ssa_generate_phi_moves: ls_num_phi_moves[%3i] = %3i\n",
				   i, ls->num_phi_moves[i]);
#endif
	}

	/* populate ls->phi_moves */

	for(i = 0; i < ls->basicblockcount; i++)
		ls->num_phi_moves[i] = 0;
	for(i = 0; i < ls->basicblockcount; i++)
		for(a = 0; a < ls->ssavarcount; a++)
			if (ls->phi[i][a] != NULL) {
				pred = graph_get_first_predecessor(gd, i, &iter);
				for(j = 0; pred != -1; j++, pred = graph_get_next(&iter)) {
					/* target is phi[i][a][0] */
					/* source is phi[i][a][j+1] */
					if (ls->phi[i][a][j+1] != ls->varcount_with_indices) {
						/* valid move */
						if (ls->phi[i][a][0] != ls->phi[i][a][j+1]) {
							ls->phi_moves[pred][ls->num_phi_moves[pred]][0] =
								ls->phi[i][a][0];
							ls->phi_moves[pred][(ls->num_phi_moves[pred])++][1]=
								ls->phi[i][a][j+1];
						}
					}
				}
			}
}

#ifdef SSA_DEBUG_VERBOSE
void ssa_print_phi(lsradata *ls, graphdata *gd) {
	int i,j,k;

	printf("phi Functions (varcount_with_indices: %3i):\n",
		   ls->varcount_with_indices);
	for(i = 0; i < ls->basicblockcount; i++) {
		for(j = 0; j < ls->ssavarcount; j++) {
			if (ls->phi[i][j] != NULL) {
				printf("BB %3i %3i = phi(", i, ls->phi[i][j][0]);
				for(k = 1; k <= graph_get_num_predecessor(gd, i); k++)
					printf("%3i ",ls->phi[i][j][k]);
				printf(")\n");
			}
		}
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
