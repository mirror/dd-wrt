/* mm/cacao-gc/copy.c - GC module for copying heap regions

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

#include "gc.h"
#include "heap.h"
#include "mark.h"
#include "region.h"
#include "rootset.h"
#include "mm/memory.hpp"
#include "toolbox/logging.hpp"
#include "vm/global.hpp"


/* Global Variables ***********************************************************/

static java_object_t *next;


static u4 copy_object(u1 *old, u1 *new, u4 size)
{
	u4 new_size;

	GC_LOG2( printf("\tcopy_object: %p -> %p\n", old, new); );

	/* copy old object content to new location */
	MCOPY(new, old, u1, size);

	/* invalidate old object */
	/* TODO: implement me, but remember not to destroy the header! */

	new_size = size;

	/* check if we need to attach the hashcode to the object */
	if (GC_TEST_FLAGS((java_object_t *) new, HDRFLAG_HASH_TAKEN)) {

		GC_LOG( printf("need to attach hash to %p\n", new); );

	}

	return new_size;
}


#define GC_FORWARD(ref,refptr,start,end) \
	*(refptr) = copy_forward(ref, start, end)

static void *copy_forward(java_object_t *o, void *src_start, void *src_end)
{
	s4 o_size;

	/* TODO: this is only to make debug output more readable; remove me! */
	if (o == NULL)
		return NULL;

	if (POINTS_INTO(o, src_start, src_end)) {

		/* update all references which point into the source region */

		/* NOTE: we use the marking bit here to mark object which have already
		 * been copied; in such a case the *vftbl contains the location of
		 * the copy */ 
		if (GC_IS_MARKED(o)) {

			GC_LOG2( printf("\tForwarding reference: %p -> ", (void *) o);
					heap_print_object((java_object_t *) o->vftbl);
					printf("\n"); );

			/* return the location of an already existing copy */
			return o->vftbl;

		} else {

			GC_LOG2( printf("\tCopying object to %p: ", (void *) next);
					heap_print_object(o); printf("\n"); );

			/* calculate the size of the object to be copied */
			o_size = get_object_size(o);

			/* copy the object pointed to by O to location NEXT */
			o_size = copy_object(o, next, o_size);

			/* remember where the copy is located and mark original */
			GC_SET_MARKED(o);
			o->vftbl = (void *) next;

			/* increment NEXT to point past the copy of the object */
			next = ((u1 *) next) + o_size;

			/* return the location of the copy */
			return o->vftbl;

		}

	} else {

		GC_LOG2( printf("\tDoing nothing for outside reference: ");
				heap_print_object(o); printf("\n"); );

		/* do not change references not pointing into the source region */
		return o;

	}
}


void copy_me(regioninfo_t *src, regioninfo_t *dst, rootset_t *rs)
{
	java_object_t  *scan;
	/*java_object_t *next;*/
	java_object_t  *ref;
	java_object_t **refptr;
	int i;

	/* initialize the scan and next pointer */
	scan = (java_object_t *) dst->base;
	next = (java_object_t *) dst->base;

	GC_LOG( dolog("GC: Copying object from rootset ..."); );

	/* for each root pointer R: replace R with forward(R) */
	while (rs) {
		for (i = 0; i < rs->refcount; i++) {

			/* load the root reference */
			ref = *( rs->refs[i].ref );

			/* forward the object */
			GC_FORWARD(ref, rs->refs[i].ref, src->base, src->end);

		}

		rs = rs->next;
	}

	GC_LOG( dolog("GC: Copying referenced objects ...") );

	/* update all references for objects in the destination region.
	 * when scan catches up with next, the algorithm is finished */
	while (scan < next)
	{

		GC_LOG2( printf("Will also forward reference in ");
		 		heap_print_object(scan); printf("\n"); );

		if (IS_ARRAY(scan)) {

			/* walk through the references of an Array */
			FOREACH_ARRAY_REF(scan,ref,refptr,

				GC_FORWARD(ref, refptr, src->base, src->end);

			);

		} else {

			/* walk through the references of an Object */
			FOREACH_OBJECT_REF(scan,ref,refptr,

				GC_FORWARD(ref, refptr, src->base, src->end);

			);

		}

		scan = ((u1 *) scan) + get_object_size(scan); 
	}

	/* update destination region information */
	/* TODO: there is more to update! */
	dst->ptr = scan;

	/* some basic assumptions */
	GC_ASSERT(scan == next);
	GC_ASSERT(scan < dst->end);
}


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
