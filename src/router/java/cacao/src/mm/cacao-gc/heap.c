/* mm/cacao-gc/heap.c - GC module for heap management

   Copyright (C) 1996-2013
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
#include "vm/types.hpp"

#include "threads/lock.hpp"

#include "gc.h"
#include "final.h"
#include "heap.h"
#include "mark.h"
#include "region.h"
#include "mm/memory.hpp"
#include "native/include/java_lang_String.h"
#include "native/llni.hpp"
#include "toolbox/logging.hpp"

#include "vm/global.hpp"
#include "vm/options.hpp"
#include "vm/rt-timing.hpp"
#include "vm/string.hpp"
#include "vm/vm.hpp"


/* Global Variables ***********************************************************/

s4 heap_current_size;  /* current size of the heap */
s4 heap_maximal_size;  /* maximal size of the heap */
regioninfo_t *heap_region_sys;
regioninfo_t *heap_region_main;


void heap_init_objectheader(java_object_t *o, u4 bytelength)
{
	u4 wordcount;

	/* initialize the header flags */
	o->hdrflags = 0;

	/* align the size */
	/* TODO */

	/* calculate the wordcount as stored in the header */
    /* TODO: improve this to save wordcount and without header bytes */
    if ((bytelength & 0x03) == 0) {
        GC_ASSERT((bytelength & 0x03) == 0);
        wordcount = (bytelength >> 2);
        GC_ASSERT(wordcount != 0);
    } else {
        wordcount = GC_SIZE_DUMMY;
    }

	/* set the wordcount in the header */
    if (wordcount >= GC_SIZE_DUMMY) {
        GC_SET_SIZE(o, GC_SIZE_DUMMY);
    } else {
        GC_SET_SIZE(o, wordcount);
    }

}


void heap_update_references(rootset_t *rs, regioninfo_t *region, u4 offset)
{
	java_object_t  *o;
	java_object_t  *ref;
	java_object_t **refptr;
	u1* start;
	u1* end;
	int i;

	GC_LOG( dolog("GC: Updating all references (offset=%x) ...", offset); );

	start = region->base - offset;
	end = region->ptr - offset;
	GC_LOG( printf("Region previously was [ %p ; %p]\n", start, end); );

	GC_LOG2( printf("updating in root-sets ..."); );

	/* walk through all rootsets */
	while (rs) {

		/* walk through the references of this rootset */
		for (i = 0; i < rs->refcount; i++) {

			/* load the reference */
			refptr = rs->refs[i].ref;
			ref = *( refptr );

			GC_LOG2( printf("\troot pointer to %p\n", (void *) ref); );

			/* update the references */
			if (POINTS_INTO(ref, start, end))
				*refptr = ((u1 *) ref) + offset;

		}

		/* skip to next rootset in chain */
		rs = rs->next;

	}


	o = region->base;
	while (o < region->ptr) {

		GC_LOG2( printf("updating in %p ...\n", (void *) o); );

		if (IS_ARRAY(o)) {

			/* walk through the references of an Array */
			FOREACH_ARRAY_REF(o,ref,refptr,

				GC_LOG2( printf("\tarray-entry %p -> %p\n", (void *) ref, ((u1 *) ref) + offset); );

				if (POINTS_INTO(ref, start, end))
					*refptr = ((u1 *) ref) + offset;

			);

		} else {

			/* walk through the references of an Object */
			FOREACH_OBJECT_REF(o,ref,refptr,

				GC_LOG2( printf("\tobject-field %p -> %p\n", (void *) ref, ((u1 *) ref) + offset); );

				if (POINTS_INTO(ref, start, end))
					*refptr = ((u1 *) ref) + offset;

			);

		}

		/* skip to next object */
		o = ((u1 *) o) + get_object_size(o);

	}

}


void heap_increase_size(rootset_t *rs)
{
	s4 newsize;
	s4 resize_offset;

	/* only a quick sanity check */
	GC_ASSERT(heap_current_size <= heap_maximal_size);

	/* check if we are allowed to enlarge the heap */
	if (heap_current_size == heap_maximal_size)
		vm_abort("heap_increase_size: reached maximal heap size: out of memory");

	/* find out how much to increase the heap??? */
	newsize = 2 * heap_current_size; /* XXX TODO: better heuristic here */
	dolog("GC: Increasing Heap Size to %d bytes", newsize); /* XXX remove me */
	GC_LOG( dolog("GC: Increasing Heap Size to %d bytes", newsize); );

	/* resize the main heap region */
	resize_offset = region_resize(heap_region_main, newsize);

	/* update all references if necesarry */
	if (resize_offset != 0)
		heap_update_references(rs, heap_region_main, resize_offset);
	else
		dolog("GC WARNING: References are not updated after heap resizing!");

	/* set the new values */
	heap_current_size = newsize;

	GC_LOG( dolog("GC: Increasing Heap Size was successful");
			heap_println_usage(); );

	/* only a quick sanity check */
	GC_ASSERT(heap_current_size <= heap_maximal_size);

}


s4 heap_get_hashcode(java_object_t *o)
{
	s4 hashcode;

	if (!o)
		return 0;

	/* TODO: we need to lock the object here i think!!! */

	/* check if there is a hash attached to this object */
	if (GC_TEST_FLAGS(o, HDRFLAG_HASH_ATTACHED)) {

		hashcode = *( (s4 *) ( ((u1 *) o) + get_object_size(o) - SIZEOF_VOID_P ) ); /* TODO: clean this up!!! */
		GC_LOG2( dolog("GC: Hash re-taken: %d (0x%08x)", hashcode, hashcode); );

	} else {

		GC_SET_FLAGS(o, HDRFLAG_HASH_TAKEN);

		hashcode = (s4) (ptrint) o;
		GC_LOG2( dolog("GC: Hash taken: %d (0x%08x)", hashcode, hashcode); );

	}

	return hashcode;
}


static java_object_t *heap_alloc_intern(u4 bytelength, regioninfo_t *region, bool collect)
{
	java_object_t *p;

	/* only a quick sanity check */
	GC_ASSERT(region);
	GC_ASSERT(bytelength >= sizeof(java_object_t));

#if !defined(NDEBUG) && defined(ENABLE_THREADS)
	/* check the current VM state for sanity */
	GC_ASSERT(!THREADOBJECT->gc_critical);
	GC_ASSERT(THREADOBJECT->flags & THREAD_FLAG_IN_NATIVE);
#endif

	/* align objects in memory */
	bytelength = GC_ALIGN(bytelength, GC_ALIGN_SIZE);

	/* lock the region */
	LOCK_MONITOR_ENTER(region);

#if !defined(NDEBUG)
	/* heavy stress test */
	if (opt_GCStress && collect)
		gc_collect(0);
#endif

	/* check for sufficient free space */
	if (bytelength > region->free) {
		dolog("GC: Region out of memory! (collect=%d)", collect);

		if (collect) {
			gc_collect(0);
#if 0
			GC_ASSERT(region->free >= bytelength);
#else
			if (region->free < bytelength) {
				dolog("GC: OOM OOM OOM OOM OOM OOM OOM OOM OOM OOM");
				exceptions_throw_outofmemoryerror();
				return NULL;
			}
#endif
		} else
			return NULL;
	}

	/* allocate the object in this region */
	p = (java_object_t *) region->ptr;
	region->ptr += bytelength;
	region->free -= bytelength;

	/* unlock the region */
	LOCK_MONITOR_EXIT(region);

	/* clear allocated memory region */
	GC_ASSERT(p);
	MSET(p, 0, u1, bytelength);

	/* set the header information */
	heap_init_objectheader(p, bytelength);

	return p;
}


/* heap_alloc ******************************************************************

   Allocates memory on the Java heap.

*******************************************************************************/

void *heap_alloc(u4 size, u4 references, methodinfo *finalizer, bool collect)
{
	java_object_t *p;
	java_handle_t *h;
/* TODO port to new rt timing */
#if 0
#if defined(ENABLE_RT_TIMING)
	struct timespec time_start, time_end;
#endif

	RT_TIMING_GET_TIME(time_start);
#endif

	p = heap_alloc_intern(size, heap_region_main, collect);

	if (p == NULL)
		return NULL;

#if defined(GCCONF_HDRFLAG_REFERENCING)
	/* We can't use a bool here for references, as it's passed as a
	   bitmask in builtin_new.  Thus we check for != 0. */
	if (references != 0) {
		GC_SET_FLAGS(p, HDRFLAG_REFERENCING);
	}
#endif

	/* register the finalizer for this object */
	if (finalizer != NULL) {
		final_register(p, finalizer);
	}

	h = LLNI_WRAP(p);

/* TODO port to new rt timing */
#if 0
	RT_TIMING_GET_TIME(time_end);
	RT_TIMING_TIME_DIFF(time_start, time_end, RT_TIMING_GC_ALLOC);
#endif

	return h;
}


void *heap_alloc_uncollectable(u4 size)
{
	java_object_t *p;

	/* loader.c does this a lot for classes with fieldscount equal zero */
	if (size == 0)
		return NULL;

	p = heap_alloc_intern(size, heap_region_sys, false);

	if (p == NULL)
		return NULL;

	/* TODO: can this be overwritten by cloning??? */
	/* remember this object as uncollectable */
	GC_SET_FLAGS(p, HDRFLAG_UNCOLLECTABLE);

	return p;
}


void heap_free(void *p)
{
	GC_LOG( dolog("GC: Free %p", p); );
	GC_ASSERT(0);
}


/* Debugging ******************************************************************/

#if !defined(NDEBUG)
void heap_println_usage()
{
	printf("Current Heap Usage: Size=%d Free=%d\n",
			heap_current_size, heap_region_main->free);

	GC_ASSERT(heap_current_size == heap_region_main->size);
}
#endif


#if !defined(NDEBUG)
void heap_print_object_flags(java_object_t *o)
{
	printf("0x%02x [%s%s%s%s]",
		GC_GET_SIZE(o),
		GC_TEST_FLAGS(o, HDRFLAG_HASH_ATTACHED) ? "A" : " ",
		GC_TEST_FLAGS(o, HDRFLAG_HASH_TAKEN)    ? "T" : " ",
		GC_TEST_FLAGS(o, HDRFLAG_UNCOLLECTABLE) ? "U" : " ",
		GC_TEST_FLAGS(o, GC_FLAG_MARKED)        ? "M" : " ");
}
#endif


#if !defined(NDEBUG)
void heap_print_object(java_object_t *o)
{
	java_array_t *a;
	classinfo    *c;

	/* check for null pointers */
	if (o == NULL) {
		printf("(NULL)");
		return;
	}

	/* print general information */
#if SIZEOF_VOID_P == 8
	printf("0x%016llx: ", (unsigned long long) o);
#else
	printf("0x%08lx: ", (unsigned long) o);
#endif

	/* check for invalid heap references */
	if (!POINTS_INTO(o, heap_region_main->base, heap_region_main->end) &&
		!POINTS_INTO(o, heap_region_sys->base, heap_region_sys->end))
	{
		printf("<<< No Heap Reference >>>");
		return;
	}

	/* print object flags */
	heap_print_object_flags(o);
	printf(" ");

	GC_ASSERT(o->vftbl);

	/* TODO */
	/* maybe this is not really an object */
	if (/*IS_CLASS*/ o->vftbl->class == class_java_lang_Class) {

		/* get the class information */
		c = (classinfo *) o;

		/* print the class information */
		printf("CLS ");
		class_print(c);

	} else if (/*IS_ARRAY*/ o->vftbl->arraydesc != NULL) {

		/* get the array information */
		a = (java_array_t *) o;
		c = o->vftbl->class;

		/* print the array information */
		printf("ARR ");
		/*class_print(c);*/
		utf_display_printable_ascii_classname(c->name);
		printf(" (size=%d)", a->size);

	} else /*IS_OBJECT*/ {

		/* get the object class */
		c = o->vftbl->class;

		/* print the object information */
		printf("OBJ ");
		/*class_print(c);*/
		utf_display_printable_ascii_classname(c->name);
		if (c == class_java_lang_String) {
			printf(" (string=\"");
			utf_display_printable_ascii(
					JavaString((java_lang_String*) o).to_utf8();
			printf("\")");
		}

	}
}
#endif

#if !defined(NDEBUG)
void heap_dump_region(regioninfo_t *region, bool marked_only)
{
	java_object_t *o;
	u4             o_size;

	/* some basic sanity checks */
	GC_ASSERT(region->base <= region->ptr);

	printf("Heap-Dump:\n");

	/* walk the region in a linear style */
	o = (java_object_t *) region->base;
	while (o < region->ptr) {

		if (!marked_only || GC_IS_MARKED(o)) {
			printf("\t");
			heap_print_object(o);
			printf("\n");
		}

		/* get size of object */
		o_size = get_object_size(o);

		/* walk to next object */
		GC_ASSERT(o_size != 0);
		o = ((u1 *) o) + o_size;
	}

	printf("Heap-Dump finished.\n");
}
#endif


s4 get_object_size(java_object_t *o)
{
	java_array_t *a;
	classinfo    *c;
	s4            o_size;

	/* we can assume someone initialized the header */
	GC_ASSERT(o->hdrflags != 0);

	/* get the wordcount from the header */
	o_size = GC_GET_SIZE(o);

	/* maybe we need to calculate the size by hand */
	if (o_size != GC_SIZE_DUMMY) {
		GC_ASSERT(o_size != 0);
		o_size = o_size << 2;
	} else {

		/* TODO */
		/* maybe this is not really an object */
		if (/*IS_CLASS*/ o->vftbl->class == class_java_lang_Class) {
			/* we know the size of a classinfo */
			o_size = sizeof(classinfo);

		} else if (/*IS_ARRAY*/ o->vftbl->arraydesc != NULL) {
			/* compute size of this array */
			a = (java_array_t *) o;
			c = o->vftbl->class;
			o_size = c->vftbl->arraydesc->dataoffset +
					a->size * c->vftbl->arraydesc->componentsize;

		} else /*IS_OBJECT*/ {
			/* get the object size */
			c = o->vftbl->class;
			o_size = c->instancesize;
			GC_LOG( dolog("Got size (from Class): %d bytes", o_size); );
		}
	
	}

	/* align the size */
	o_size = GC_ALIGN(o_size, GC_ALIGN_SIZE);

	/* the hashcode attached to this object might increase the size */
	if (GC_TEST_FLAGS(o, HDRFLAG_HASH_ATTACHED))
		o_size += SIZEOF_VOID_P;

	return o_size;
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
