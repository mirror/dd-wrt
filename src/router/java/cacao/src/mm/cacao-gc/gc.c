/* src/mm/cacao-gc/gc.c - main garbage collector methods

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

#include <signal.h>
#include <stdint.h>

#include "vm/types.hpp"

#include "threads/lock.hpp"
#include "threads/thread.hpp"

#include "compact.h"
#include "copy.h"
#include "final.h"
#include "gc.h"
#include "heap.h"
#include "mark.h"
#include "region.h"
#include "rootset.h"
#include "mm/memory.hpp"
#include "toolbox/logging.hpp"

#include "vm/finalizer.hpp"
#include "vm/rt-timing.hpp"
#include "vm/vm.hpp"


/* Global Variables ***********************************************************/

bool gc_pending;
bool gc_running;
bool gc_notify_finalizer;

list_t *gc_reflist_strong;
list_t *gc_reflist_weak;

#if !defined(ENABLE_THREADS)
executionstate_t *_no_threads_executionstate;
sourcestate_t    *_no_threads_sourcestate;
#endif


/* gc_init *********************************************************************

   Initializes the garbage collector.

*******************************************************************************/

#define GC_SYS_SIZE (20*1024*1024)

void gc_init(u4 heapmaxsize, u4 heapstartsize)
{
	if (opt_verbosegc)
		dolog("GC: Initialising with heap-size %d (max. %d)",
			heapstartsize, heapmaxsize);

#if defined(ENABLE_HANDLES)
	/* check our indirection cells */
	if (OFFSET(java_handle_t, heap_object) != 0)
		vm_abort("gc_init: indirection cell offset is displaced: %d", OFFSET(java_handle_t, heap_object));
	if (OFFSET(hashtable_classloader_entry, object) != 0)
		vm_abort("gc_init: classloader entry cannot be used as indirection cell: %d", OFFSET(hashtable_classloader_entry, object));
	if (OFFSET(hashtable_global_ref_entry, o) != 0)
		vm_abort("gc_init: global reference entry cannot be used as indirection cell: %d", OFFSET(hashtable_global_ref_entry, o));
#endif

	/* finalizer stuff */
	final_init();

	/* set global variables */
	gc_pending = false;
	gc_running = false;

	/* create list for external references */
	gc_reflist_strong = list_create(OFFSET(list_gcref_entry_t, linkage));
	gc_reflist_weak   = list_create(OFFSET(list_gcref_entry_t, linkage));

	/* region for uncollectable objects */
	heap_region_sys = NEW(regioninfo_t);
	if (!region_create(heap_region_sys, GC_SYS_SIZE))
		vm_abort("gc_init: region_create failed: out of memory");

	/* region for java objects */
	heap_region_main = NEW(regioninfo_t);
	if (!region_create(heap_region_main, heapstartsize))
		vm_abort("gc_init: region_create failed: out of memory");

	heap_current_size = heapstartsize;
	heap_maximal_size = heapmaxsize;
}


/* gc_reference_register *******************************************************

   Register an external reference which points onto the Heap. The
   reference needs to be cleared (set to NULL) when registering and
   has to be set after it has been registered (to avoid a race condition).
   
   STRONG REFERENCE: gets updated and keeps objects alive
   WEAK REFERENCE:   only gets updated (or maybe cleared)

*******************************************************************************/

static void gc_reference_register_intern(list_t *list, java_object_t **ref, int32_t reftype)
{
	list_gcref_entry_t *re;

	/* the global GC lock also guards the reference lists */
	GC_MUTEX_LOCK;

	GC_LOG2( printf("Registering Reference at %p\n", (void *) ref); );

	/* the reference needs to be registered before it is set, so make sure the
	   reference is not yet set */
	GC_ASSERT(*ref == NULL);

#if !defined(NDEBUG)
	/* check if this reference is already registered */
	for (re = list_first(list); re != NULL; re = list_next(list, re)) {
		if (re->ref == ref)
			vm_abort("gc_reference_register_intern: reference already registered");
	}
#endif

	/* create a new reference entry */
	re = NEW(list_gcref_entry_t);

	re->ref     = ref;
#if !defined(NDEBUG)
	re->reftype = reftype;
#endif

	/* add the entry to the given list */
	list_add_last(list, re);

	/* the global GC lock also guards the reference lists */
	GC_MUTEX_UNLOCK;
}

void gc_reference_register(java_object_t **ref, int32_t reftype)
{
	gc_reference_register_intern(gc_reflist_strong, ref, reftype);
}

void gc_weakreference_register(java_object_t **ref, int32_t reftype)
{
	gc_reference_register_intern(gc_reflist_weak, ref, reftype);
}


/* gc_reference_unregister *****************************************************

   Unregister a previously registered external reference.

*******************************************************************************/

static void gc_reference_unregister_intern(list_t *list, java_object_t **ref)
{
	list_gcref_entry_t *re;

	/* the global GC lock also guards the reference lists */
	GC_MUTEX_LOCK;

	GC_LOG2( printf("Un-Registering Reference at %p\n", (void *) ref); );

	/* search for the appropriate reference entry */
	for (re = list_first(list); re != NULL; re = list_next(list, re)) {
		if (re->ref == ref) {
			/* remove the entry from the given list */
			list_remove(list, re);

			/* free the reference entry */
			FREE(re, list_gcref_entry_t);

			break;
		}
	}

	vm_abort("gc_reference_unregister_intern: reference not found");

	/* the global GC lock also guards the reference lists */
	GC_MUTEX_UNLOCK;
}

void gc_reference_unregister(java_object_t **ref)
{
	gc_reference_unregister_intern(gc_reflist_strong, ref);
}

void gc_weakreference_unregister(java_object_t **ref)
{
	gc_reference_unregister_intern(gc_reflist_weak, ref);
}


/* gc_collect ******************************************************************

   This is the main machinery which manages a collection. It should be run by
   the thread which triggered the collection.

   IN:
     XXX

   STEPS OF A COLLECTION:
     XXX

*******************************************************************************/

void gc_collect(s4 level)
{
	rootset_t    *rs;
	int32_t       dumpmarker;
#if !defined(NDEBUG)
	stacktrace_t *st;
#endif
/* TODO port to new rt-timing */
#if 0
#if defined(ENABLE_RT_TIMING)
	struct timespec time_start, time_suspend, time_rootset, time_mark, time_compact, time_end;
#endif
#endif

	/* enter the global gc lock */
	GC_MUTEX_LOCK;

	/* remember start of dump memory area */
	DMARKER;

	GCSTAT_COUNT(gcstat_collections);

/* TODO port to new rt-timing */
#if 0
	RT_TIMING_GET_TIME(time_start);
#endif

	/* let everyone know we want to do a collection */
	GC_ASSERT(!gc_pending);
	gc_pending = true;

	/* finalizer is not notified, unless marking tells us to do so */
	gc_notify_finalizer = false;

#if defined(ENABLE_THREADS)
	/* stop the world here */
	GC_LOG( dolog("GC: Suspending threads ..."); );
	GC_LOG( threads_dump(); );
	threads_stopworld();
	/*GC_LOG( threads_dump(); );*/
	GC_LOG( dolog("GC: Suspension finished."); );
#endif

#if !defined(NDEBUG)
	/* get the stacktrace of the current thread and make sure it is non-empty */
	GC_LOG( printf("Stacktrace of current thread:\n"); );
	st = stacktrace_get_current();
	if (st == NULL)
		vm_abort("gc_collect: no stacktrace available for current thread!");
	GC_LOG( stacktrace_print(st); );
#endif

	/* sourcestate of the current thread, assuming we are in the native world */
	GC_LOG( dolog("GC: Stackwalking current thread ..."); );
#if defined(ENABLE_THREADS)
	GC_ASSERT(THREADOBJECT->flags & THREAD_FLAG_IN_NATIVE);
#endif
	replace_gc_from_native(THREADOBJECT, NULL, NULL);

	/* everyone is halted now, we consider ourselves running */
	GC_ASSERT(!gc_running);
	gc_pending = false;
	gc_running = true;

/* TODO port to new rt-timing */
#if 0
	RT_TIMING_GET_TIME(time_suspend);
#endif

	GC_LOG( heap_println_usage(); );
	/*GC_LOG( heap_dump_region(heap_region_main, false); );*/

	/* find the global and local rootsets */
	rs = rootset_readout();

#if !defined(NDEBUG)
	/* print the rootsets if debugging is enabled */
	if (opt_GCDebugRootSet)
		rootset_print(rs);
#endif

/* TODO port to new rt-timing */
#if 0
	RT_TIMING_GET_TIME(time_rootset);
#endif

#if 1

	/* mark the objects considering the given rootset */
	mark_me(rs);
	/*GC_LOG( heap_dump_region(heap_region_main, false); );*/

/* TODO port to new rt-timing */
#if 0
	RT_TIMING_GET_TIME(time_mark);
#endif

	/* compact the heap */
	compact_me(rs, heap_region_main);
	/*GC_LOG( heap_dump_region(heap_region_main, false); );*/

#if defined(ENABLE_MEMCHECK)
	/* invalidate the rest of the main region */
	region_invalidate(heap_region_main);
#endif

/* TODO port to new rt-timing */
#if 0
	RT_TIMING_GET_TIME(time_compact);
#endif

	/* check if we should increase the heap size */
	if (gc_get_free_bytes() < gc_get_heap_size() / 3) /* TODO: improve this heuristic */
		heap_increase_size(rs);

#else

	/* copy the heap to new region */
	{
		regioninfo_t *src, *dst;

		src = heap_region_main;
		dst = NEW(regioninfo_t);
		region_create(dst, heap_current_size);
		copy_me(heap_region_main, dst, rs);
		heap_region_main = dst;

		/* invalidate old heap */
		memset(src->base, 0x66, src->size);
	}
#endif

	/* TODO: check my return value! */
	/*heap_increase_size();*/

	/* write back the rootset to update root references */
	GC_LOG( rootset_print(rs); );
	rootset_writeback(rs);

#if defined(ENABLE_STATISTICS)
	if (opt_verbosegc)
		gcstat_println();
#endif

	/* we are no longer running */
	gc_running = false;

#if defined(ENABLE_THREADS)
	/* start the world again */
	GC_LOG( dolog("GC: Reanimating world ..."); );
	threads_startworld();
	/*GC_LOG( threads_dump(); );*/
#endif

#if defined(GCCONF_FINALIZER)
	/* does the finalizer need to be notified */
	if (gc_notify_finalizer)
		finalizer_notify();
#endif

/* TODO port to new rt-timing */
#if 0
	RT_TIMING_GET_TIME(time_end);

	RT_TIMING_TIME_DIFF(time_start  , time_suspend, RT_TIMING_GC_SUSPEND);
	RT_TIMING_TIME_DIFF(time_suspend, time_rootset, RT_TIMING_GC_ROOTSET1)
	RT_TIMING_TIME_DIFF(time_rootset, time_mark   , RT_TIMING_GC_MARK);
	RT_TIMING_TIME_DIFF(time_mark   , time_compact, RT_TIMING_GC_COMPACT);
	RT_TIMING_TIME_DIFF(time_compact, time_end    , RT_TIMING_GC_ROOTSET2);
	RT_TIMING_TIME_DIFF(time_start  , time_end    , RT_TIMING_GC_TOTAL);
#endif

    /* free dump memory area */
    DRELEASE;

	/* leave the global gc lock */
	GC_MUTEX_UNLOCK;

	/* XXX move this to an appropriate place */
	lock_hashtable_cleanup();
}


#if defined(ENABLE_THREADS)
bool gc_suspend(threadobject *thread, u1 *pc, u1 *sp)
{
	codeinfo         *code;

	/* check if the thread suspended itself */
	if (pc == NULL) {
		GC_LOG( dolog("GC: Suspended myself!"); );
		return true;
	}

	/* thread was forcefully suspended */
	GC_LOG( dolog("GC: Suspending thread (tid=%p)", thread->impl.tid); );

	/* check where this thread came to a halt */
	if (thread->flags & THREAD_FLAG_IN_NATIVE) {

		if (thread->gc_critical) {
			GC_LOG( dolog("\tNATIVE &  CRITICAL -> retry"); );

			GC_ASSERT(0);

			/* wait till this thread suspends itself */
			return false;

		} else {
			GC_LOG( dolog("\tNATIVE & SAFE -> suspend"); );

			/* we assume we are in a native! */
			replace_gc_from_native(thread, pc, sp);

			/* suspend me now */
			return true;

		}

	} else {
		code = code_find_codeinfo_for_pc_nocheck(pc);

		if (code != NULL) {
			GC_LOG( dolog("\tJIT (pc=%p) & KNOWN(codeinfo=%p) -> replacement",
					pc, code); );

			/* arm the replacement points of the code this thread is in */
			replace_activate_replacement_points(code, false);

			/* wait till this thread suspends itself */
			return false;

		} else {
			GC_LOG( dolog("\tJIT (pc=%p) & UN-KNOWN -> retry", pc); );

			/* re-suspend me later */
			/* TODO: implement me! */
			/* TODO: (this is a rare race condition which was not yet triggered) */
			GC_ASSERT(0);
			return false;

		}

	}

	/* this point should never be reached */
	GC_ASSERT(0);

}
#endif


/* gc_call *********************************************************************

   Forces a full collection of the whole Java Heap.
   This is the function which is called by java.lang.Runtime.gc()

*******************************************************************************/

void gc_call(void)
{
	if (opt_verbosegc)
		dolog("GC: Forced Collection ...");

	GCSTAT_COUNT(gcstat_collections_forced);

	gc_collect(0);

	if (opt_verbosegc)
		dolog("GC: Forced Collection finished.");
}


/* gc_invoke_finalizers ********************************************************

   Forces invocation of all the finalizers for objects which are reclaimable.
   This is the function which is called by the finalizer thread.

*******************************************************************************/

void gc_invoke_finalizers(void)
{
	if (opt_verbosegc)
		dolog("GC: Invoking finalizers ...");

	final_invoke();

	if (opt_verbosegc)
		dolog("GC: Invoking finalizers finished.");
}


/* gc_finalize_all *************************************************************

   Forces the finalization of all objects on the Java Heap.
   This is the function which is called by java.lang.Runtime.exit()

   We do this by setting all objects with finalizers to reclaimable,
   which is inherently dangerouse because objects may still be alive.

*******************************************************************************/

void gc_finalize_all(void)
{
#if !defined(NDEBUG)
	/* doing this is deprecated, inform the user */
	dolog("gc_finalize_all: Deprecated!");
#endif

	/* set all objects with finalizers to reclaimable */
	final_set_all_reclaimable();

	/* notify the finalizer thread */
	finalizer_notify();
}


/* Informational getter functions *********************************************/

s8 gc_get_heap_size(void)     { return heap_current_size; }
s8 gc_get_free_bytes(void)    { return heap_region_main->free; }
s8 gc_get_total_bytes(void)   { return heap_region_main->size - heap_region_main->free; }
s8 gc_get_max_heap_size(void) { return heap_maximal_size; }


/* Statistics *****************************************************************/

#if defined(ENABLE_STATISTICS)
int gcstat_collections;
int gcstat_collections_forced;
int gcstat_mark_depth;
int gcstat_mark_depth_max;
int gcstat_mark_count;

void gcstat_println()
{
	printf("\nGCSTAT - General Statistics:\n");
	printf("\t# of collections: %d\n", gcstat_collections);
	printf("\t# of forced collections: %d\n", gcstat_collections_forced);

    printf("\nGCSTAT - Marking Statistics:\n");
    printf("\t# of objects marked: %d\n", gcstat_mark_count);
    printf("\tMaximal marking depth: %d\n", gcstat_mark_depth_max);

	printf("\nGCSTAT - Compaction Statistics:\n");

	printf("\n");
}
#endif /* defined(ENABLE_STATISTICS) */


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
