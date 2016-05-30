/* src/vm/jit/allocator/liveness.hpp - liveness header

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


#ifndef LIVENESS_HPP_
#define LIVENESS_HPP_ 1

#include "toolbox/bitvector.hpp"
#include "vm/jit/allocator/lsra.hpp"

#if !defined(NDEBUG)
#include <assert.h>
#define LV_DEBUG_CHECK
/* #define LV_DEBUG_VERBOSE */
#endif

#if defined(LV_DEBUG_CHECK)
#define _LV_CHECK_BOUNDS(i,l,h) assert( ((i) >= (l)) && ((i) < (h)));
#define _LV_ASSERT(a) assert((a));
#else
#define _LV_CHECK_BOUNDS(i,l,h);
#define _LV_ASSERT(a);
#endif

#define LV_KILL 0
#define LV_GEN  1

/* #define LV */

/* function prototypes */
void liveness(jitdata *);
void liveness_init(jitdata *);
void liveness_setup(jitdata *);

typedef struct liveness_sets lv_sets;

struct liveness_sets {
	bitvector in;
	bitvector out;
	bitvector gen;
	bitvector kill;
	bitvector tmp;
};

#endif // LIVENESS_HPP_


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
