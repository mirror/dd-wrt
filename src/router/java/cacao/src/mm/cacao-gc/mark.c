/* src/mm/cacao-gc/mark.c - GC module for marking heap objects

   Copyright (C) 2006, 2008
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
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.

*/


#include "config.h"

#include "gc.h"
#include "final.h"
#include "heap.h"
#include "mark.h"
#include "rootset.h"
#include "mm/memory.hpp"
#include "toolbox/logging.hpp"

#include "vm/global.hpp"
#include "vm/linker.hpp"
#include "vm/vm.hpp"


/* Helper Macros **************************************************************/

#define MARK(o) \
	GCSTAT_COUNT_MAX(gcstat_mark_depth, gcstat_mark_depth_max); \
	mark_recursive(o); \
	GCSTAT_DEC(gcstat_mark_depth);


/* mark_recursive **************************************************************

   Recursively mark all objects (including this) which are referenced.

   TODO, XXX: We need to implement a non-recursive version of this!!!

   IN:
	  o.....heap-object to be marked (either OBJECT or ARRAY)

*******************************************************************************/

void mark_recursive(java_object_t *o)
{
	vftbl_t            *t;
	classinfo          *c;
	fieldinfo          *f;
	java_objectarray_t *oa;
	arraydescriptor    *desc;
	java_object_t      *ref;
	void *start, *end;
	int i;

	/* TODO: this needs cleanup!!! */
	start = heap_region_main->base;
	end = heap_region_main->ptr;

	/* uncollectable objects should never get marked this way */
	/* the reference should point into the heap */
	GC_ASSERT(o);
	GC_ASSERT(!GC_TEST_FLAGS(o, HDRFLAG_UNCOLLECTABLE));
	GC_ASSERT(POINTS_INTO(o, start, end));

	/* mark this object */
	GC_SET_MARKED(o);
	GCSTAT_COUNT(gcstat_mark_count);

	/* get the class of this object */
	/* TODO: maybe we do not need this yet, look to move down! */
	t = o->vftbl;
	GC_ASSERT(t);
	c = t->class;
	GC_ASSERT(c);

#if defined(GCCONF_HDRFLAG_REFERENCING)
	/* does this object has pointers? */
	/* TODO: check how often this happens, maybe remove this check! */
	if (!GC_TEST_FLAGS(o, HDRFLAG_REFERENCING))
		return;
#endif

	/* check if we are marking an array */
	if ((desc = t->arraydesc) != NULL) {
		/* this is an ARRAY */

		/* check if the array contains references */
		if (desc->arraytype != ARRAYTYPE_OBJECT)
			return;

		/* for object-arrays we need to check every entry */
		oa = (java_objectarray_t *) o;
		for (i = 0; i < oa->header.size; i++) {

			/* load the reference value */
			ref = (java_object_t *) (oa->data[i]);

			/* check for outside or null pointers */
			if (!POINTS_INTO(ref, start, end))
				continue;

			GC_LOG2( printf("Found (%p) from Array\n", (void *) ref); );

			/* do the recursive marking */
			if (!GC_IS_MARKED(ref)) {
				GCSTAT_COUNT_MAX(gcstat_mark_depth, gcstat_mark_depth_max);
				mark_recursive(ref);
				GCSTAT_DEC(gcstat_mark_depth);
			}

		}

	} else {
		/* this is an OBJECT */

		/* for objects we need to check all (non-static) fields */
		for (; c; c = c->super) {
		for (i = 0; i < c->fieldscount; i++) {
			f = &(c->fields[i]);

			/* check if this field contains a non-static reference */
			if (!IS_ADR_TYPE(f->type) || (f->flags & ACC_STATIC))
				continue;

			/* load the reference value */
			ref = *( (java_object_t **) ((s1 *) o + f->offset) );

			/* check for outside or null pointers */
			if (!POINTS_INTO(ref, start, end))
				continue;

			GC_LOG2( printf("Found (%p) from Field ", (void *) ref);
					field_print(f); printf("\n"); );

			/* do the recursive marking */
			if (!GC_IS_MARKED(ref)) {
				GCSTAT_COUNT_MAX(gcstat_mark_depth, gcstat_mark_depth_max);
				mark_recursive(ref);
				GCSTAT_DEC(gcstat_mark_depth);
			}

		}
		}

	}

}


/* mark_post *******************************************************************

   Perform some post-marking cleanup tasks.

   TASKS:
      - mark unmarked objects with Finalizers
      - clear unmarked Weak References

*******************************************************************************/

void mark_post(rootset_t *rs)
{
	java_object_t      *ref;
#if defined(GCCONF_FINALIZER)
	list_final_entry_t *fe;
	u4                  f_type;
#endif
	void *start, *end;
	int i;

	/* TODO: this needs cleanup!!! */
	start = heap_region_main->base;
	end = heap_region_main->ptr;

#if defined(GCCONF_FINALIZER)
	/* objects with finalizers will also be marked here. if they have not been
	   marked before the finalization is triggered */

	/* REMEMBER: All threads are stopped, so we don't have to lock the
	   list here. */

	fe = list_first(final_list);

	while (fe) {
		f_type = fe->type;
		ref    = fe->o;

		/* we do not care about objects which have been marked already */
		if (!GC_IS_MARKED(ref)) {

			switch (f_type) {

			case FINAL_REACHABLE: /* object was reachable before */
				GC_LOG2( printf("Finalizer triggered for: ");
						heap_print_object(ref); printf("\n"); );

				/* object is now reclaimable */
				fe->type = FINAL_RECLAIMABLE;

				/* notify the finalizer after collection finished */
				gc_notify_finalizer = true;

				/* keep the object alive until finalizer finishes */
				MARK(ref);
				break;

			case FINAL_RECLAIMABLE: /* object not yet finalized */
				GC_LOG( printf("Finalizer not yet started for: ");
						heap_print_object(ref); printf("\n"); );

				/* keep the object alive until finalizer finishes */
				MARK(ref);
				break;

#if 0
			case FINAL_FINALIZING: /* object is still being finalized */
				GC_LOG( printf("Finalizer not yet finished for: ");
						heap_print_object(ref); printf("\n"); );

				/* keep the object alive until finalizer finishes */
				MARK(ref);
				break;
#endif

			default: /* case not yet covered */
				vm_abort("mark_post: uncovered case (type=%d)", f_type);

			}
		}

		fe = list_next(final_list, fe);
	}
#endif /*defined(GCCONF_FINALIZER)*/

	/* Clear all references in the rootset which have not yet been
	   marked. This applies to registered weak references. */

	while (rs) {
		GC_LOG( dolog("GC: Clearing in rootset (%d entries) ...", rs->refcount); );

		/* mark all references of the rootset */
		for (i = 0; i < rs->refcount; i++) {

			/* load the reference */
			ref = *( rs->refs[i].ref );

			/* check for outside or null pointers */
			if (!POINTS_INTO(ref, start, end))
				continue;

			/* is this a marking reference? */
			if (rs->refs[i].marks) {
				assert(GC_IS_MARKED(ref));
			} else {

				/* clear unmarked references */
				if (!GC_IS_MARKED(ref)) {
					GC_LOG( printf("Clearing Weak Reference %p at %p\n", ref, rs->refs[i]); );

					*( rs->refs[i].ref ) = NULL;
				}
			}
		}

		rs = rs->next;
	}

}


/* mark_me *********************************************************************

   Marks all Heap Objects which are reachable from a given root-set.

   REMEMBER: Assumes all threads are stopped!

   IN:
	  rs.....root set containing the references

*******************************************************************************/

void mark_me(rootset_t *rs)
{
	rootset_t     *rstop;
	java_object_t *ref;
	void *start, *end;
	int i;

	/* TODO: this needs cleanup!!! */
	start = heap_region_main->base;
	end = heap_region_main->ptr;
	rstop = rs;

	GCSTAT_INIT(gcstat_mark_count);
	GCSTAT_INIT(gcstat_mark_depth);
	GCSTAT_INIT(gcstat_mark_depth_max);

	while (rs) {
		GC_LOG( dolog("GC: Marking from rootset (%d entries) ...", rs->refcount); );

		/* mark all references of the rootset */
		for (i = 0; i < rs->refcount; i++) {

			/* is this a marking reference? */
			if (!rs->refs[i].marks)
				continue;

			/* load the reference */
			ref = *( rs->refs[i].ref );

			/* check for outside or null pointers */
			if (!POINTS_INTO(ref, start, end))
				continue;

			/* do the marking here */
			MARK(ref);

		}

		rs = rs->next;
	}

	GC_LOG( dolog("GC: Marking postprocessing ..."); );

	/* perform some post processing of the marked heap */
	mark_post(rstop);

	GC_LOG( dolog("GC: Marking finished."); );

#if defined(ENABLE_STATISTICS)
	GC_ASSERT(gcstat_mark_depth == 0);
#endif
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
