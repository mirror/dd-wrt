/* mm/cacao-gc/region.h - GC header for region management

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


#ifndef _REGION_H
#define _REGION_H

#include "vm/types.hpp"


/* Forward Typedefs ***********************************************************/

typedef struct regioninfo_t regioninfo_t;


#include "gc.h"


/* Structures *****************************************************************/

struct regioninfo_t {
#if defined(ENABLE_THREADS)
	java_object_t header; /* needed for locking */
#endif
	u1 *base;     /* pointer to the start of this region */
	u1 *end;      /* pointer to the end of this region */
	u1 *ptr;      /* pointer to the beginning of the free space */
	s4  size;     /* total size of the region (end - ptr) */
	s4  free;     /* free space in this region */
};


/* Prototypes *****************************************************************/

void *region_create(regioninfo_t *region, u4 size);

#if defined(ENABLE_MEMCHECK)
void region_invalidate(regioninfo_t *region);
#endif


#endif /* _REGION_H */

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
