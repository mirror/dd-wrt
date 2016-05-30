/* src/vm/jit/optimizing/dominators.cpp - dominators and dominance frontier

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

#include "config.h"
# include <cassert>

#include "mm/dumpmemory.hpp"

#include "toolbox/bitvector.hpp"

#include "vm/jit/jit.hpp"

#include "vm/jit/optimizing/graph.hpp"
#include "vm/jit/optimizing/dominators.hpp"

#include "vm/jit/show.hpp"

#if !defined(NDEBUG)
/* # define DOM_DEBUG_CHECK */
# define DOM_DEBUG_VERBOSE
#endif

#ifdef DOM_DEBUG_CHECK
# define _DOM_CHECK_BOUNDS(i,l,h) assert( ((i) >= (l)) && ((i) < (h)));
# define _DOM_ASSERT(a) assert((a));
#else
# define _DOM_CHECK_BOUNDS(i,l,h)
# define _DOM_ASSERT(a)
#endif

/* function prototypes */
void dom_Dominators_init(dominatordata *dd, int basicblockcount);
#ifdef DOM_DEBUG_CHECK
int dom_AncestorWithLowestSemi(dominatordata *dd, int v, int basicblockcount);
void dom_Link(dominatordata *dd, int p, int n, int basicblockcount);
void dom_DFS(graphdata *gd, dominatordata *dd, int p, int n, int *N,
			 int basicblockcount);
#else
int dom_AncestorWithLowestSemi(dominatordata *dd, int v);
void dom_Link(dominatordata *dd, int p, int n);
void dom_DFS(graphdata *gd, dominatordata *dd, int p, int n, int *N);
#endif

/*************************************
Calculate Dominators
*************************************/
dominatordata *compute_Dominators(graphdata *gd, int basicblockcount) {
	int i,j,n,N,p,s,s_,v,y;
	graphiterator iter;
	dominatordata *dd;

	dd = (dominatordata*) DumpMemory::allocate(sizeof(dominatordata));

	dom_Dominators_init(dd, basicblockcount);
	
	N=0;

	/* 1 ist the root node of the method                    */
	/* 0 is the artificial parent, where locals are set to their parameters */
	dom_DFS(gd, dd, -1, 0, &N
#ifdef DOM_DEBUG_CHECK
			,basicblockcount
#endif
			);

	for(i = N-1; i > 0; i--) {
		_DOM_CHECK_BOUNDS(i, 0, basicblockcount);
		n = dd->vertex[i];
		_DOM_CHECK_BOUNDS(n, 0, basicblockcount);
		p = dd->parent[n];
		s = p;
		j = graph_get_first_predecessor(gd, n, &iter);
		for (; j != -1; j = graph_get_next(&iter)) {
		_DOM_CHECK_BOUNDS(j, 0, basicblockcount);
			if (dd->dfnum[j] <= dd->dfnum[n])
				s_ = j;
			else
				s_ = dd->semi[dom_AncestorWithLowestSemi(dd, j
#ifdef DOM_DEBUG_CHECK
														 ,basicblockcount
#endif
														 )];
		_DOM_CHECK_BOUNDS(s_, 0, basicblockcount);
		_DOM_CHECK_BOUNDS(s, 0, basicblockcount);
			if (dd->dfnum[s_] < dd->dfnum[s])
				s = s_;
		}
		dd->semi[n] = s;
		_DOM_CHECK_BOUNDS(dd->num_bucket[s], 0, basicblockcount);
		dd->bucket[s][dd->num_bucket[s]] = n;
		dd->num_bucket[s]++;
		dom_Link(dd, p, n
#ifdef DOM_DEBUG_CHECK
				 , basicblockcount
#endif
				 );
		_DOM_CHECK_BOUNDS(p, 0, basicblockcount);
		for(j = 0; j < dd->num_bucket[p]; j++) {
		_DOM_CHECK_BOUNDS(j, 0, basicblockcount);
			v = dd->bucket[p][j];
			y = dom_AncestorWithLowestSemi(dd, v
#ifdef DOM_DEBUG_CHECK
										   , basicblockcount
#endif
										   );
		_DOM_CHECK_BOUNDS(y, 0, basicblockcount);
		_DOM_CHECK_BOUNDS(v, 0, basicblockcount);
	    if (dd->semi[y] == dd->semi[v])
				dd->idom[v] = p;
			else
				dd->samedom[v] = y;
		}
		dd->num_bucket[p] = 0;
	}
	for(i = 1; i < N; i++) {
		n = dd->vertex[i];
		_DOM_CHECK_BOUNDS(n, 0, basicblockcount);
	    if (dd->samedom[n] != -1) {
			_DOM_CHECK_BOUNDS(dd->samedom[n], 0, basicblockcount);
			dd->idom[n] = dd->idom[dd->samedom[n]];
		}
	}
	return dd;
}

/********************************************
compute Dominace Frontier
********************************************/
void computeDF(graphdata *gd, dominatordata *dd, int basicblockcount, int n) {
	int c,i,j;
	bool *_S;
	graphiterator iter;

	_S = (bool*) DumpMemory::allocate(sizeof(bool) * basicblockcount);
	for(i = 0; i < basicblockcount; i++)
		_S[i] = false;
	i = graph_get_first_successor(gd, n, &iter);
	for (; i != -1; i = graph_get_next(&iter)) {
		_DOM_CHECK_BOUNDS(i, 0, basicblockcount);
		if (dd->idom[i] != n)
			_S[i] = true;
	}
	for(c=0; c < basicblockcount; c++) {
		if (dd->idom[c] == n) {
			computeDF(gd, dd, basicblockcount, c);
			for(j=0; j < dd->num_DF[c]; j++) {
		_DOM_CHECK_BOUNDS(dd->DF[c][j], 0, basicblockcount);
	            if (n != dd->idom[dd->DF[c][j]])
					/* n does not dominate DF[c][j] -> traverse idom list? */
					_S[dd->DF[c][j]] = true;
			}
		}
	}
	for(i = 0; i < basicblockcount; i++)
		if (_S[i]) {
		_DOM_CHECK_BOUNDS(dd->num_DF[n], 0, basicblockcount);
			dd->DF[n][dd->num_DF[n]] = i;
			dd->num_DF[n]++;
		}
}


void dom_Dominators_init(dominatordata *dd, int basicblockcount) {
	int i;

	dd->dfnum      = (int*)  DumpMemory::allocate(sizeof(int) * basicblockcount);
	dd->vertex     = (int*)  DumpMemory::allocate(sizeof(int) * basicblockcount);
	dd->parent     = (int*)  DumpMemory::allocate(sizeof(int) * basicblockcount);
	dd->semi       = (int*)  DumpMemory::allocate(sizeof(int) * basicblockcount);
	dd->ancestor   = (int*)  DumpMemory::allocate(sizeof(int) * basicblockcount);
	dd->idom       = (int*)  DumpMemory::allocate(sizeof(int) * basicblockcount);
	dd->samedom    = (int*)  DumpMemory::allocate(sizeof(int) * basicblockcount);
	dd->bucket     = (int**) DumpMemory::allocate(sizeof(int*) * basicblockcount);
	dd->num_bucket = (int*)  DumpMemory::allocate(sizeof(int) * basicblockcount);
	dd->DF         = (int**) DumpMemory::allocate(sizeof(int*) * basicblockcount);
	dd->num_DF     = (int*)  DumpMemory::allocate(sizeof(int) * basicblockcount);
	dd->best       = (int*)  DumpMemory::allocate(sizeof(int) * basicblockcount);
	for (i=0; i < basicblockcount; i++) {
		dd->dfnum[i] = -1;
		dd->semi[i] = dd->ancestor[i] = dd->idom[i] = dd->samedom[i] = -1;
		dd->num_bucket[i] = 0;
		dd->bucket[i] = (int*) DumpMemory::allocate(sizeof(int) * basicblockcount);
		dd->num_DF[i] = 0;
		dd->DF[i] = (int*) DumpMemory::allocate(sizeof(int) * basicblockcount);
	}
}

/**************************************
Create Depth First Spanning Tree
**************************************/
#ifdef DOM_DEBUG_CHECK
void dom_DFS(graphdata *gd, dominatordata *dd, int p, int n, int *N,
			 int basicblockcount) {
#else
void dom_DFS(graphdata *gd, dominatordata *dd, int p, int n, int *N) {
#endif
    int i;
	graphiterator iter;

	_DOM_CHECK_BOUNDS(n,0,basicblockcount);
	if (dd->dfnum[n] == -1) { /* not visited till now? */
		dd->dfnum[n] = *N;
		_DOM_CHECK_BOUNDS(*N,0,basicblockcount);
		dd->vertex[*N] = n;
		dd->parent[n] = p;
		(*N)++;
		i = graph_get_first_successor(gd, n, &iter);
		for (; i != -1; i = graph_get_next(&iter)) {
			dom_DFS(gd, dd, n, i, N
#ifdef DOM_DEBUG_CHECK
					, basicblockcount
#endif
					);
		}
	}
}

#ifdef DOM_DEBUG_CHECK
int dom_AncestorWithLowestSemi(dominatordata *dd, int v, int basicblockcount) {
#else
int dom_AncestorWithLowestSemi(dominatordata *dd, int v) {
#endif
	int a,b;

	_DOM_CHECK_BOUNDS(v, 0, basicblockcount);
	a = dd->ancestor[v];
	_DOM_CHECK_BOUNDS(a,0,basicblockcount);
	if (dd->ancestor[a] != -1) {
		b = dom_AncestorWithLowestSemi(dd, a
#ifdef DOM_DEBUG_CHECK
									   , basicblockcount
#endif
									   );
		dd->ancestor[v] = dd->ancestor[a];
		_DOM_CHECK_BOUNDS(b,0,basicblockcount);
		_DOM_CHECK_BOUNDS(dd->best[v],0,basicblockcount);
		_DOM_CHECK_BOUNDS(dd->semi[dd->best[v]],0,basicblockcount);
		if (dd->dfnum[dd->semi[b]] < dd->dfnum[dd->semi[dd->best[v]]])
			dd->best[v] = b;
	}
	return dd->best[v];
}

#ifdef DOM_DEBUG_CHECK
void dom_Link(dominatordata *dd, int p, int n, int basicblockcount) {
#else
void dom_Link(dominatordata *dd, int p, int n) {
#endif
	_DOM_CHECK_BOUNDS(n,0,basicblockcount);
	dd->ancestor[n] = p;
	dd->best[n] = n;
}

/*********************************************************/

typedef struct basicblock_info basicblock_info;

struct basicblock_info {
	basicblock *bb;
	int dfnum;
	basicblock_info *parent;
	basicblock_info *semi;
	basicblock_info *ancestor;
	basicblock_info *best;
	basicblock_info *idom;
	basicblock_info *samedom;
	basicblock_info **bucket;
	unsigned bucketcount;
};

typedef struct dominator_tree_info dominator_tree_info;

struct dominator_tree_info {
	jitdata *jd;
	basicblock_info *basicblocks;
	basicblock_info **df_map;
	unsigned df_counter;
};

static dominator_tree_info *dominator_tree_init(jitdata *jd) {
	dominator_tree_info *di;
	basicblock *itb;
	basicblock_info *iti;

	di = (dominator_tree_info*) DumpMemory::allocate(sizeof(dominator_tree_info));

	di->jd = jd;

	di->basicblocks = (basicblock_info*) DumpMemory::allocate(sizeof(basicblock_info) * jd->basicblockcount);
	MZERO(di->basicblocks, basicblock_info, jd->basicblockcount);
	
	for (iti = di->basicblocks; iti != di->basicblocks + jd->basicblockcount; ++iti) {
		iti->dfnum = -1;
		iti->bucket = (basicblock_info**) DumpMemory::allocate(sizeof(basicblock_info*) * jd->basicblockcount);
		iti->bucketcount = 0;
	}

	for (itb = jd->basicblocks; itb; itb = itb->next) {
		di->basicblocks[itb->nr].bb = itb;
	}

	di->df_map = (basicblock_info**) DumpMemory::allocate(sizeof(basicblock_info*) * jd->basicblockcount);
	MZERO(di->df_map, basicblock_info *, jd->basicblockcount);

	di->df_counter = 0;

	return di;
}

static inline basicblock_info *dominator_tree_get_basicblock(dominator_tree_info *di, basicblock *bb) {
	return di->basicblocks + bb->nr;
}

static void dominator_tree_depth_first_search(
	dominator_tree_info *di, basicblock_info *parent, basicblock_info *node
) {
	basicblock **it;

	if (node->dfnum == -1) {

		node->dfnum = di->df_counter;
		node->parent = parent;
		di->df_map[di->df_counter] = node;
		di->df_counter += 1;

		for (it = node->bb->successors; it != node->bb->successors + node->bb->successorcount; ++it) {
			dominator_tree_depth_first_search(
				di, node, 
				dominator_tree_get_basicblock(di, *it)
			);
		}
	}
}

void dominator_tree_link(dominator_tree_info *di, basicblock_info *parent, basicblock_info *node) {
	node->ancestor = parent;
	node->best = node;
}

basicblock_info *dominator_tree_ancestor_with_lowest_semi(
	dominator_tree_info *di, basicblock_info *node
) {
	basicblock_info *a, *b;

	a = node->ancestor;

	if (a->ancestor != NULL) {
		b = dominator_tree_ancestor_with_lowest_semi(di, a);
		node->ancestor = a->ancestor;
		if (b->semi->dfnum < node->best->semi->dfnum) {
			node->best = b;
		}
	}

	return node->best;
}

void dominator_tree_build_intern(jitdata *jd) {
	
	dominator_tree_info *di;
	basicblock_info *node;
	basicblock_info *semicand;
	basicblock_info *pred;
	basicblock **itb;
	basicblock_info **itii;
	basicblock_info *v, *y;
	int i;

	di = dominator_tree_init(jd);

	dominator_tree_depth_first_search(di, NULL, dominator_tree_get_basicblock(di, jd->basicblocks));

	for (i = di->df_counter - 1; i >= 1; --i) {
		node = di->df_map[i];

		node->semi = node->parent;

		for (
			itb = node->bb->predecessors; 
			itb != node->bb->predecessors + node->bb->predecessorcount; 
			++itb
		) {

			pred = dominator_tree_get_basicblock(di, *itb);

			if (pred->dfnum <= node->dfnum) {
				semicand = pred;
			} else {
				semicand = dominator_tree_ancestor_with_lowest_semi(di, pred)->semi;
			}

			if (semicand->dfnum < node->semi->dfnum) {
				node->semi = semicand;
			}
		}

		node->semi->bucket[node->semi->bucketcount] = node;
		node->semi->bucketcount += 1;

		dominator_tree_link(di, node->parent, node);

		for (itii = node->parent->bucket; itii != node->parent->bucket + node->parent->bucketcount; ++itii) {
			v = *itii;
			y = dominator_tree_ancestor_with_lowest_semi(di, v);
			if (y->semi == v->semi) {
				v->idom = node->parent;
			} else {
				v->samedom = y;
			}
		}

		node->parent->bucketcount = 0;
	}

	for (i = 1; i < di->df_counter; ++i) {
		node = di->df_map[i];
		if (node->samedom) {
			node->idom = node->samedom->idom;
		}

		node->bb->idom = node->idom->bb;
		node->idom->bb->domsuccessorcount += 1;
	}
}

void dominator_tree_link_children(jitdata *jd) {
	basicblock *bb;
	/* basicblock number => current number of successors */
	unsigned *numsuccessors;

	// Create new dump memory area.
	DumpMemoryArea dma;

	/* Allocate memory for successors */

	for (bb = jd->basicblocks; bb; bb = bb->next) {
		if (bb->domsuccessorcount > 0) {
			bb->domsuccessors = (basicblock**) DumpMemory::allocate(sizeof(basicblock*) * bb->domsuccessorcount);
		}
	}

	/* Allocate memory for per basic block counter of successors */

	numsuccessors = (unsigned*) DumpMemory::allocate(sizeof(unsigned) * jd->basicblockcount);
	MZERO(numsuccessors, unsigned, jd->basicblockcount);

	/* Link immediate dominators with successors */

	for (bb = jd->basicblocks; bb; bb = bb->next) {
		if (bb->idom) {
			bb->idom->domsuccessors[numsuccessors[bb->idom->nr]] = bb;
			numsuccessors[bb->idom->nr] += 1;
		}
	}
}

bool dominator_tree_build(jitdata *jd) {
	// Create new dump memory area.
	DumpMemoryArea dma;

	dominator_tree_build_intern(jd);

	dominator_tree_link_children(jd);

	return true;
}

typedef struct dominance_frontier_item dominance_frontier_item;

struct dominance_frontier_item {
	basicblock *bb;
	dominance_frontier_item *next;
};

typedef struct dominance_frontier_list dominance_frontier_list;

struct dominance_frontier_list {
	dominance_frontier_item *first;
	unsigned count;
};

void dominance_frontier_list_add(dominance_frontier_list *list, basicblock *bb) {
	dominance_frontier_item *item;
	
	for (item = list->first; item; item = item->next) {
		if (item->bb == bb) return;
	}

	item = (dominance_frontier_item*) DumpMemory::allocate(sizeof(dominance_frontier_item));
	item->bb = bb;
	item->next = list->first;
	list->first = item;
	list->count += 1;
}

typedef struct dominance_frontier_info dominance_frontier_info;

struct dominance_frontier_info {
	jitdata *jd;
	dominance_frontier_list *map;
};

dominance_frontier_info *dominance_frontier_init(jitdata *jd) {
	dominance_frontier_info *dfi = (dominance_frontier_info*) DumpMemory::allocate(sizeof(dominance_frontier_info));

	dfi->jd = jd;

	dfi->map = (dominance_frontier_list*) DumpMemory::allocate(sizeof(dominance_frontier_list) * jd->basicblockcount);
	MZERO(dfi->map, dominance_frontier_list, jd->basicblockcount);

	return dfi;
}

bool dominance_frontier_dominates(basicblock *d, basicblock *x) {
	x = x->idom;

	while (x != NULL) {
		if (x == d) {
			return true;
		}
		x = x->idom;
	}

	return false;
}

void dominance_frontier_for_block(dominance_frontier_info *dfi, basicblock *b) {
	basicblock **it;
	dominance_frontier_item *itdf;
	dominance_frontier_list s = { NULL, 0 };

	for (it = b->successors; it != b->successors + b->successorcount; ++it) {
		if ((*it)->idom != b) {
			dominance_frontier_list_add(&s, *it);
		}
	}

	for (it = b->domsuccessors; it != b->domsuccessors + b->domsuccessorcount; ++it) {
		dominance_frontier_for_block(dfi, *it);
		for (itdf = dfi->map[(*it)->nr].first; itdf; itdf = itdf->next) {
			if (! dominance_frontier_dominates(b, itdf->bb)) {
				dominance_frontier_list_add(&s, itdf->bb);
			}
		}
	}

	dfi->map[b->nr] = s;
}

void dominance_frontier_store(dominance_frontier_info *dfi) {
	basicblock *bb;
	dominance_frontier_item *itdf;
	basicblock **itout;

	for (bb = dfi->jd->basicblocks; bb; bb = bb->next) {
		if (bb->nr < dfi->jd->basicblockcount) {
			if (dfi->map[bb->nr].count > 0) {
				bb->domfrontiercount = dfi->map[bb->nr].count;
				itout = bb->domfrontier = (basicblock**) DumpMemory::allocate(sizeof(basicblock*) * bb->domfrontiercount);
				for (itdf = dfi->map[bb->nr].first; itdf; itdf = itdf->next) {
					*itout = itdf->bb;
					itout += 1;
				}
			}
		}
	}
}

bool dominance_frontier_build(jitdata *jd) {
	// Create new dump memory area.
	DumpMemoryArea dma;

	dominance_frontier_info *dfi = dominance_frontier_init(jd);
	dominance_frontier_for_block(dfi, jd->basicblocks);
	dominance_frontier_store(dfi);
}

void dominator_tree_validate(jitdata *jd, dominatordata *_dd) {
	graphdata *gd;
	int i, j;
	basicblock *bptr, **it;
	dominatordata *dd;
	int *itnr;
	bool found;

	// Create new dump memory area.
	DumpMemoryArea dma;

	fprintf(stderr, "%s/%s: \n", jd->m->clazz->name.begin(), jd->m->name.begin());
	gd = graph_init(jd->basicblockcount);

	for (bptr = jd->basicblocks; bptr; bptr = bptr->next) {
		for (it = bptr->successors; it != bptr->successors + bptr->successorcount; ++it) {
			graph_add_edge(gd, bptr->nr, (*it)->nr);
		}
	}

	dd = compute_Dominators(gd, jd->basicblockcount);

	for (bptr = jd->basicblocks; bptr; bptr = bptr->next) {
		if (bptr->state >= basicblock::REACHED) {
			if (bptr->idom == NULL) {
				if (!(dd->idom[bptr->nr] == -1)) {
					printf("-- %d %d\n", dd->idom[bptr->nr], bptr->nr);
					assert(0);
				}
			} else {
				assert(dd->idom[bptr->nr] == bptr->idom->nr);
			}
		}
	}

	computeDF(gd, dd, jd->basicblockcount, 0);

	for (bptr = jd->basicblocks; bptr; bptr = bptr->next) {
		if (bptr->state >= basicblock::REACHED) {
			assert(bptr->domfrontiercount == dd->num_DF[bptr->nr]);
			for (itnr = dd->DF[bptr->nr]; itnr != dd->DF[bptr->nr] + dd->num_DF[bptr->nr]; ++itnr) {
				found = false;
				for (it = bptr->domfrontier; it != bptr->domfrontier + bptr->domfrontiercount; ++it) {
					if ((*it)->nr == *itnr) {
						found =true; break;
					}
				}
				assert(found);
			}
		}
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
 */
