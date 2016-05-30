/* mm/cacao-gc/mark.h - GC header for marking heap objects

   Copyright (C) 2006 R. Grafl, A. Krall, C. Kruegel,
   C. Oates, R. Obermaisser, M. Platter, M. Probst, S. Ring,
   E. Steiner, C. Thalinger, D. Thuernbeck, P. Tomsich, C. Ullrich,
   J. Wenninger, Institut f. Computersprachen - TU Wien

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


#ifndef _MARK_H
#define _MARK_H


#include "config.h"
#include "vm/types.hpp"

#include "rootset.h"


/* Helper Macros **************************************************************/

#define GC_FLAG_MARKED        (HDRFLAG_MARK1 | HDRFLAG_MARK2)

#define GC_IS_MARKED(obj)    GC_TEST_FLAGS(obj, GC_FLAG_MARKED)
#define GC_SET_MARKED(obj)   GC_SET_FLAGS(obj, GC_FLAG_MARKED)
#define GC_CLEAR_MARKED(obj) GC_CLEAR_FLAGS(obj, GC_FLAG_MARKED)


/* Prototypes *****************************************************************/

void mark_me(rootset_t *rs);


#endif /* _MARK_H */

/*
 * These are local overrides for various environment variables in Emacs.
 * Please do not remove this and leave it at the end of the file, where
 * Emacs will automagically detect them.
 * ---------------------------------------------------------------------
 * Local variables:
 * mode: c
 * indent-tabs-mode: t
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim:noexpandtab:sw=4:ts=4:
 */
