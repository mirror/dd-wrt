/* mm/cacao-gc/region.c - GC module for region management

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


#include "config.h"
#include "vm/types.hpp"

#include "region.h"
#include "mm/memory.hpp"
#include "toolbox/logging.hpp"


/* region_init *****************************************************************

   Allocates the memory for a heap region and initiates the regioninfo struct
   accordingly.

*******************************************************************************/

void *region_create(regioninfo_t *region, u4 size)
{
	u1 *ptr;

	/* some sanity check */
	GC_ASSERT(region);

	/* allocate memory for the region */
	ptr = MNEW(u1, size);

	if (ptr == NULL)
		return NULL;

	/* initiate structure */
	region->base = ptr;
	region->end  = ptr + size;
	region->ptr  = ptr;
	region->size = size;
	region->free = size;

#if defined(ENABLE_THREADS)
	/* initiate the header for locking */
	lock_init_object_lock((java_object_t *) region);
#endif

#if defined(ENABLE_MEMCHECK)
	/* poison this region */
	/* TODO: this should really be done MNEW above! */
	region_invalidate(region);
#endif

	GC_LOG( dolog("GC: Region allocated at [ %p ; %p ]", region->base, region->end); );

	return ptr;
}


u4 region_resize(regioninfo_t *region, u4 size)
{
	u1 *ptr;
	u4 offset;
	u4 used;

	/* reallocate memory for the region */
	ptr = MREALLOC(region->base, u1, region->size, size);

	if (ptr == NULL)
		vm_abort("region_resize: realloc failed!");

	/* was the region moved? */
	offset = ptr - region->base;
	used   = region->size - region->free;

	/* update structure */
	region->base = ptr;
	region->end  = ptr + size;
	region->ptr  = ptr + used;
	region->size = size;
	region->free = size - used;

#if defined(ENABLE_MEMCHECK)
	/* poison this region */
	region_invalidate(region);
#endif

	GC_LOG( dolog("GC: Region resized to [ %p ; %p ]", region->base, region->end); );

	return offset;
}


/* region_invalidate ***********************************************************

   Invalidates the free memory area inside a heap region by overwriting it with
   the clear byte.

   REMEMBER: The region has to be compacted for this to work properly.

*******************************************************************************/

#if defined(ENABLE_MEMCHECK)
void region_invalidate(regioninfo_t *region)
{
	/* some sanity check */
	GC_ASSERT(region->free == region->end - region->ptr);

	/* invalidate free memory */
	memset(region->ptr, MEMORY_CLEAR_BYTE, region->free);
}
#endif /* defined(ENABLE_MEMCHECK) */


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
