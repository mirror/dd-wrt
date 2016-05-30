/* src/vm/jit/optimizing/dominators.hpp - Dominators and Dominance Frontier header

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


#ifndef DOMINATORS_HPP_
#define DOMINATORS_HPP_ 1

#include "config.h"

#include "vm/jit/optimizing/graph.hpp"

struct dominatordata {
	int *dfnum;           /* [0..ls->basicblockcount[ */
	int *vertex;          /* [0..ls->basicblockcount[ */
	int *parent;          /* [0..ls->basicblockcount[ */
	int *semi;            /* [0..ls->basicblockcount[ */
	int *ancestor;        /* [0..ls->basicblockcount[ */
	int *idom;            /* [0..ls->basicblockcount[ */
	int *samedom;         /* [0..ls->basicblockcount[ */
	int **bucket;         /* [0..ls->basicblockcount[[0..ls->bbc[ */
	int *num_bucket;      /* [0..ls->basicblockcount[ */
	int *best;            /* [0..ls->basicblockcount[ */
	int **DF;             /* [0..ls->basicblockcount[[0..ls->bbc[ */
	int *num_DF;          /* [0..ls->basicblockcount[ */
};	

struct dominatordata;

/* function prototypes */

dominatordata *compute_Dominators(graphdata *gd, int basicblockcount);
void           computeDF(graphdata *gd, dominatordata *dd, int basicblockcount, int n);

/* ............................... */

bool dominator_tree_build(jitdata *jd);

bool dominance_frontier_build(jitdata *jd);

void dominator_tree_validate(jitdata *jd, dominatordata *dd);

#endif // DOMINATORS_HPP_


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
 */
