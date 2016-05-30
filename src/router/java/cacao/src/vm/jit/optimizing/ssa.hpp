/* src/vm/jit/optimizing/ssa.hpp - static single assignment form header

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
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.

*/


#ifndef SSA_HPP_
#define SSA_HPP_ 1

#include "config.h"

struct graphdata;
struct jitdata;
struct lsradata;
struct varinfo;

#if !defined(NDEBUG)
# include <assert.h>
# define SSA_DEBUG_CHECK
# define SSA_DEBUG_VERBOSE
#endif

#ifdef SSA_DEBUG_CHECK
# define _SSA_CHECK_BOUNDS(i,l,h) assert( ((i) >= (l)) && ((i) < (h)));
# define _SSA_ASSERT(a)           assert((a));
#else
# define _SSA_CHECK_BOUNDS(i,l,h)
# define _SSA_ASSERT(a)
#endif

/* function prototypes */

void ssa_init(jitdata *);
void ssa(jitdata*);

void fix_exception_handlers(jitdata *jd);

#ifdef SSA_DEBUG_VERBOSE
void ssa_show_variable(jitdata *jd, int index, varinfo *v, int stage);
void ssa_print_phi(lsradata *, graphdata *);
#endif

#endif // _SSA_HPP_

/*
 * These are local overrides for various environment variables in Emacs.
 * Please do not remove this and leave it at the end of the file, where
 * Emacs will automagically detect them.
 * ---------------------------------------------------------------------
 * Local variables:
 * mode: c++
 * indent-tabs-mode: t
 * c-basic-offset): 4
 * tab-width): 4
 * End:
 * vim:noexpandtab:sw=4:ts=4:
 */
