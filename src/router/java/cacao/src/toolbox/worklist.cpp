/* src/toolbox/worklist.c - worklist implementation

   Copyright (C) 2005, 2006 R. Grafl, A. Krall, C. Kruegel, C. Oates,
   R. Obermaisser, M. Platter, M. Probst, S. Ring, E. Steiner,
   C. Thalinger, D. Thuernbeck, P. Tomsich, C. Ullrich, J. Wenninger,
   Institut f. Computersprachen - TU Wien

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

   Contact: cacao@complang.tuwien.ac.at

   Authors: Christian Ullrich


*/

#include "toolbox/worklist.hpp"
#include <cassert>
#include "bitvector.hpp"                // for bv_get_bit, bv_new, etc
#include "mm/dumpmemory.hpp"            // for DMNEW, DNEW

#if !defined(NDEBUG)

/// define WL_DEBUG_CHECK to activate the bound checks
// #define WL_DEBUG_CHECK

#endif

#if defined(WL_DEBUG_CHECK)
#define _WL_CHECK_BOUNDS(i,l,h) assert( ((i) >= (l)) && ((i) < (h)));
#define _WL_ASSERT(a) assert((a));
#else
#define _WL_CHECK_BOUNDS(i,l,h);
#define _WL_ASSERT(a);
#endif

/******************************************************************************

Worklist Implementation

New Elements (integers) are pushed on a stack and remembered in a
bitvector, to ensure efficitently uniqueness.

******************************************************************************/

/*******************************************************************************
wl_new

IN:     int size    size of worklist

RETURN: worklist *  new worklist
*******************************************************************************/
worklist *wl_new(int size) {
	worklist *w = DNEW(worklist);
	w->W_stack  = DMNEW(int, size);
	w->W_top    = 0;
	w->W_bv     = bv_new(size);
#ifdef WL_DEBUG_CHECK
	w->size     = size;
#endif
	return w;
}

/*******************************************************************************
wl_add

Adds the integer element to the worklist, if this value is not already
in the worklist.

IN:     worklist *w    pointer to worklist created with wl_new
        int element    integer element to be added
*******************************************************************************/
void wl_add(worklist *w, int element) {
	_WL_CHECK_BOUNDS(element, 0, w->size);

	if (!bv_get_bit(w->W_bv, element)) {
		_WL_ASSERT((w->W_top < w->size));
		w->W_stack[(w->W_top)++] = element;
		bv_set_bit(w->W_bv, element);
	}
}

/*******************************************************************************
wl_get

Returns and removes an element from the worklist.

IN:     worklist *w    pointer to worklist created with wl_new

RETURN  int            an element removed from the worklist
*******************************************************************************/
int wl_get(worklist *w) {
	_WL_ASSERT((w->W_top > 0));

	int element = w->W_stack[--(w->W_top)];
	bv_reset_bit(w->W_bv, element);
	return element;
}

/*******************************************************************************
wl_is_empty

Checks if the worklist is empty.

IN:     worklist *w    pointer to worklist created with wl_new

RETURN  bool           true if w is empty, false otherwise
*******************************************************************************/
bool wl_is_empty(worklist *w) {
	return (w->W_top == 0);
}

/*******************************************************************************
wl_reset

Empties the worklist.

IN:     worklist *w    pointer to worklist created with wl_new
*******************************************************************************/
void wl_reset(worklist *w, int size) {
	w->W_top = 0;
	bv_reset(w->W_bv, size);
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
