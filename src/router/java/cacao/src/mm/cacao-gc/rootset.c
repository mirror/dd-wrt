/* mm/cacao-gc/rootset.c - GC module for root set management

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

#include "mm/memory.hpp"

#include "threads/threadlist.h"
#include "threads/thread.hpp"

#include "toolbox/logging.hpp"

#include "vm/global.hpp"
#include "vm/jit/replace.hpp"
#include "vm/jit/stacktrace.hpp"


rootset_t *rootset_create(void)
{
	rootset_t *rs;

	/* allocate memory for rootset */
	rs = DNEW(rootset_t);

	rs->next     = NULL;
	rs->capacity = ROOTSET_INITIAL_CAPACITY;
	rs->refcount = 0;

	return rs;
}


rootset_t *rootset_resize(rootset_t *rs)
{
	s4 size_old;
	s4 size_new;

	/* double the capacity of this rootset */
	size_old = sizeof(rootset_t) + (rs->capacity - ROOTSET_INITIAL_CAPACITY) * sizeof(rootset_entry_t);
	rs->capacity *= 2;
	size_new = sizeof(rootset_t) + (rs->capacity - ROOTSET_INITIAL_CAPACITY) * sizeof(rootset_entry_t);

	GC_LOG2( printf("Resizing rootset to capacity %d (%d -> %d)\n", rs->capacity, size_old, size_new); );

	/* reallocate memory for rootset */
	/* XXX DMREALLOC(ptr,type,num1,num2) */
	rs = DMREALLOC(rs, u1, size_old, size_new);

	return rs;
}


/* rootset_from_globals ********************************************************

   Searches global variables to compile the global root set out of references
   contained in them.

   REMEMBER: All threads are stopped, so we don't have to lock the
   lists in this function.

   SEARCHES IN:
     - thread objects (threads.c)
     - classloader objects (loader.c)
     - global reference table (jni.c)
     - finalizer entries (final.c)

*******************************************************************************/

#define ROOTSET_ADD(adr,mrk,tp) \
	if (refcount >= rs->capacity) \
		rs = rootset_resize(rs); \
	rs->refs[refcount].ref     = (adr); \
	rs->refs[refcount].marks   = (mrk); \
	rs->refs[refcount].reftype = (tp); \
	refcount++;

static rootset_t *rootset_from_globals(rootset_t *rs)
{
	list_final_entry_t          *fe;
	list_gcref_entry_t          *re;
	int refcount;

	GC_LOG( dolog("GC: Acquiring Root-Set from globals ..."); );

	/* initialize the rootset struct */
	GC_ASSERT(rs);
	GC_ASSERT(rs->refcount == 0);
	rs->thread = ROOTSET_DUMMY_THREAD;

	refcount = rs->refcount;

	/* walk through all registered strong references */
	for (re = list_first(gc_reflist_strong); re != NULL; re = list_next(gc_reflist_strong, re)) {
		GC_LOG2( printf("Found Registered Reference: %p at %p of type %d\n", *(re->ref), re->ref, ref->reftype); );

		/* add this registered reference to the root set */
		ROOTSET_ADD(re->ref, true, re->reftype)
	}

	/* walk through all registered weak references */
	for (re = list_first(gc_reflist_weak); re != NULL; re = list_next(gc_reflist_weak, re)) {
		GC_LOG2( printf("Found Registered Weak Reference: %p at %p of type %d\n", *(re->ref), re->ref, ref->reftype); );

		/* add this registered reference to the root set */
		ROOTSET_ADD(re->ref, false, re->reftype)
	}

	/* walk through all finalizer entries */
	for (fe = list_first(final_list); fe != NULL; fe = list_next(final_list, fe)) {
		GC_LOG2( printf("Found Finalizer Entry: %p\n", (void *) fe->o); );

		/* add this object with finalizer to the root set */
		ROOTSET_ADD(&( fe->o ), false, GC_REFTYPE_FINALIZER)
	}

	/* remeber how many references there are inside this root set */
	rs->refcount = refcount;

	return rs;
}


static rootset_t *rootset_from_classes(rootset_t *rs)
{
	classinfo         *c;
	fieldinfo         *f;
	void *sys_start, *sys_end;
	int refcount;
	int i;

	GC_LOG( dolog("GC: Acquiring Root-Set from classes ..."); );

	/* TODO: cleanup!!! */
	sys_start = heap_region_sys->base;
	sys_end = heap_region_sys->ptr;

	refcount = rs->refcount;

	/* walk through all classinfo blocks */
	c = sys_start;
	while (c < (classinfo *) sys_end) {

		GC_LOG2( printf("Searching in class "); class_print(c); printf("\n"); );

		/* walk through all fields */
		f = c->fields;
		for (i = 0; i < c->fieldscount; i++, f++) {

			/* check if this is a static reference */
			if (!IS_ADR_TYPE(f->type) || !(f->flags & ACC_STATIC))
				continue;

			/* check for outside or null pointers */
			if (f->value->a == NULL)
				continue;

			GC_LOG2( printf("Found Static Field Reference: %p\n", (void *) f->value->a);
					printf("\tfrom field: "); field_print(f); printf("\n");
					printf("\tto object : "); heap_print_object(f->value->a); printf("\n"); );

			/* add this static field reference to the root set */
			ROOTSET_ADD(f->value, true, GC_REFTYPE_CLASSREF);

		}

		/* skip to next classinfo block */
		c++;
		c = (classinfo *) (GC_ALIGN((ptrint) c, GC_ALIGN_SIZE));

	}

	/* remeber how many references there are inside this root set */
	rs->refcount = refcount;

	return rs;
}


/* rootset_from_thread *********************************************************

   Searches the stack of the passed thread for references and compiles a
   root set out of them.

   NOTE: uses dump memory!

   IN:
	  thread...TODO
      rs.......TODO

   OUT:
	  TODO!!!

*******************************************************************************/

static rootset_t *rootset_from_thread(threadobject *thread, rootset_t *rs)
{
	executionstate_t *es;
	sourcestate_t    *ss;
	sourceframe_t    *sf;
	localref_table   *lrt;
	int refcount;
	int i;

#if defined(ENABLE_THREADS)
	GC_ASSERT(thread != NULL);
	GC_LOG( dolog("GC: Acquiring Root-Set from thread (tid=%p) ...", (void *) thread->impl.tid); );
#else
	GC_ASSERT(thread == NULL);
	GC_LOG( dolog("GC: Acquiring Root-Set from single-thread ..."); );
#endif

	GC_LOG2( printf("Stacktrace of thread:\n");
			threads_thread_print_stacktrace(thread); );

	/* get the sourcestate of the threads */
	es = GC_EXECUTIONSTATE;
	ss = GC_SOURCESTATE;

	GC_ASSERT(es);
	GC_ASSERT(ss);

	/* print our full source state */
	GC_LOG2( replace_sourcestate_println(ss); );

	/* initialize the rootset struct */
	GC_ASSERT(rs);
	GC_ASSERT(rs->refcount == 0);
	rs->thread = thread;

	refcount = rs->refcount;

	/* now inspect the source state to compile the root set */
	for (sf = ss->frames; sf != NULL; sf = sf->down) {

		GC_LOG( printf("Source Frame: localcount=%d, stackdepth=%d, syncslots=%d\n", sf->javalocalcount, sf->javastackdepth, sf->syncslotcount); );

		for (i = 0; i < sf->javalocalcount; i++) {

			/* we only need to consider references */
			if (sf->javalocaltype[i] != TYPE_ADR)
				continue;

			GC_LOG2( printf("Found Reference (Java Local): %p\n", (void *) sf->javalocals[i].a); );

			/* add this reference to the root set */
			ROOTSET_ADD((java_object_t **) &( sf->javalocals[i] ), true, GC_REFTYPE_STACK);

		}

		for (i = 0; i < sf->javastackdepth; i++) {

			/* we only need to consider references */
			if (sf->javastacktype[i] != TYPE_ADR)
				continue;

			GC_LOG2( printf("Found Reference (Java Stack): %p\n", (void *) sf->javastack[i].a); );

			/* add this reference to the root set */
			ROOTSET_ADD((java_object_t **) &( sf->javastack[i] ), true, GC_REFTYPE_STACK);

		}

		for (i = 0; i < sf->syncslotcount; i++) {

			GC_LOG( printf("Found Reference (Sync Slot): %p\n", (void *) sf->syncslots[i].a); );

			/* add this reference to the root set */
			ROOTSET_ADD((java_object_t **) &( sf->syncslots[i] ), true, GC_REFTYPE_STACK);

		}
	}

	/* now walk through all local references of this thread */
#if defined(ENABLE_THREADS)
	lrt = thread->_localref_table;
#else
	lrt = LOCALREFTABLE;
#endif
	while (lrt) {

		for (i = 0; i < lrt->used; i++) {

			/* there should be no null pointers in here */
			GC_ASSERT(lrt->refs[i] != NULL);

			GC_LOG2( printf("Found LocalRef: %p\n", (void *) lrt->refs[i]); );

			/* add this reference to the root set */
			ROOTSET_ADD(&( lrt->refs[i] ), true, GC_REFTYPE_LOCALREF);

		}

		lrt = lrt->prev;
	}

	/* remeber how many references there are inside this root set */
	rs->refcount = refcount;

	return rs;
}


rootset_t *rootset_readout()
{
	rootset_t    *rs_top;
	rootset_t    *rs;
	threadobject *t;

	/* find the global rootset ... */
	rs_top = rootset_create();
	rs_top = rootset_from_globals(rs_top);
	rs_top = rootset_from_classes(rs_top);

	/* ... and the rootsets for the threads */
	rs = rs_top;
#if defined(ENABLE_THREADS)
	for (t = threadlist_first(); t != NULL; t = threadlist_next(t)) {

		/* ignore threads which are in state NEW */
		if (t->state == THREAD_STATE_NEW)
			continue;

		rs->next = rootset_create();
		rs->next = rootset_from_thread(t, rs->next);

		rs = rs->next;
	}
#else
	t = THREADOBJECT;

	rs->next = rootset_create();
	rs->next = rootset_from_thread(t, rs->next);
#endif

	return rs_top;
}


void rootset_writeback(rootset_t *rs)
{
	threadobject     *thread;

	/* walk through all rootsets */
	while (rs) {
		thread = rs->thread;

		/* does this rootset belong to a thread? */
		if (thread != ROOTSET_DUMMY_THREAD) {
#if defined(ENABLE_THREADS)
			GC_ASSERT(thread != NULL);
			GC_LOG( dolog("GC: Writing back Root-Set to thread (tid=%p) ...", (void *) thread->impl.tid); );
#else
			GC_ASSERT(thread == NULL);
			GC_LOG( dolog("GC: Writing back Root-Set to single-thread ..."); );
#endif

			/* now rebuild the stack of the thread */
			replace_gc_into_native(thread);
		}

		rs = rs->next;
	}

}


/* Debugging ******************************************************************/

#if !defined(NDEBUG)
static const char* reftype_names[] = {
		"THREADOBJECT", "CLASSLOADER ", "GLOBAL-REF  ",
		"FINALIZER   ", "LOCAL-REF   ", "ON-STACK-ADR",
		"STATIC FIELD", "LOCKRECORD  "
};

void rootset_print(rootset_t *rs)
{
	java_object_t *o;
	int i;

	/* walk through all rootsets in the chain */
	printf("Root Set Chain:\n");
	while (rs) {

		/* print the thread this rootset belongs to */
		if (rs->thread == ROOTSET_DUMMY_THREAD) {
			printf("\tGlobal Root Set:\n");
		} else {
#if defined(ENABLE_THREADS)
			printf("\tLocal Root Set with Thread-Id %p:\n", (void *) rs->thread->impl.tid);
#else
			printf("\tLocal Root Set:\n");
#endif
		}

		/* print the references in this rootset */
		printf("\tReferences (%d / %d):\n", rs->refcount, rs->capacity);
		for (i = 0; i < rs->refcount; i++) {

			o = *( rs->refs[i].ref );

			/*printf("\t\tReference at %p points to ...\n", (void *) rs->refs[i]);*/
			printf("\t\t");
			printf("%s ", reftype_names[rs->refs[i].reftype]);
			if (rs->refs[i].marks)
				printf("STRONG");
			else
				printf("  WEAK");
			printf(" ");
			heap_print_object(o);
			printf("\n");

		}

		rs = rs->next;

	}

}
#endif


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
