/* src/vm/jit/optimizing/graph.hpp - control flow graph header

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


#ifndef LSRA_GRAPH_HPP_
#define LSRA_GRAPH_HPP_ 1

#include "config.h"

#include "vm/jit/optimizing/lsra.hpp"

#if !defined(NDEBUG)
# include <assert.h>
/* # define GRAPH_DEBUG_CHECK */
# define GRAPH_DEBUG_VERBOSE
#endif

#ifdef GRAPH_DEBUG_CHECK
# define _GRAPH_CHECK_BOUNDS(i,l,h) assert( ((i) >= (l)) && ((i) < (h)));
# define _GRAPH_ASSERT(a) assert((a));
#else
# define _GRAPH_CHECK_BOUNDS(i,l,h)
# define _GRAPH_ASSERT(a)
#endif

#if 0
struct _sbr {
	int header;          /* BB Index of subroutine start (SBR_HEADER) */
	struct graph_element *ret;   /* List of possible return BB indizes */
	struct _sbr *next;
};
#endif

struct graph_element {
  int value;
  struct graph_element *next;
};

struct graphdata {
/* #ifdef GRAPH_DEBUG_CHECK */
	int basicblockcount;
/* #endif */
	int *num_succ;
	struct graph_element **successor;
	int *num_pred;
	struct graph_element **predecessor;
};

struct graphdata;
struct graph_element;
typedef graph_element *graphiterator;

/* function prototypes */

void transform_BB(jitdata *, graphdata *gd);
graphdata *graph_init(int basicblockcount);
                                            /* Generate the CFG */
void graph_make_cfg(jitdata *,graphdata *);
void graph_DFS(lsradata *ls, graphdata *gd);
int graph_get_first_successor(graphdata *gd, int b_index, graphiterator *i);
int graph_get_first_predecessor(graphdata *gd, int b_index, graphiterator *i);
int graph_get_next(graphiterator *i);
int graph_get_num_predecessor(graphdata *gd, int b_index);
int graph_get_num_successor(graphdata *gd, int b_index);
bool graph_has_multiple_successors( graphdata *gd, int b_index);
bool graph_has_multiple_predecessors( graphdata *gd, int b_index);

void graph_add_edge( graphdata *gd, int from, int to );

#endif // LSRA_GRAPH_HPP_


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
