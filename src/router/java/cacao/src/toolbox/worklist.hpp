/* src/toolbox/worklist.hpp - worklist header

   Copyright (C) 2005-2013
   CACAOVM - Verein zu Foerderung der freien virtuellen Machine CACAO

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


#ifndef WORKLIST_HPP_
#define WORKLIST_HPP_ 1

#include "toolbox/bitvector.hpp"

struct worklist {
	int      *W_stack;
	int       W_top;
	bitvector W_bv;
#ifdef WL_DEBUG_CHECK
	int       size;
#endif
};

/* function prototypes */
worklist *wl_new(int size);
void wl_add(worklist *w, int element);
int wl_get(worklist *w);
bool wl_is_empty(worklist *w);
void wl_reset(worklist *w, int size);

#endif // BITVECTOR_HPP_


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
