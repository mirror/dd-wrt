/* src/mm/gc-boehm.cpp - interface for boehm gc

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

#include "mm/gc.hpp"
#include <stddef.h>                     // for size_t, NULL
#include <stdint.h>                     // for int64_t, uint8_t
#include "config.h"
#include "mm/memory.hpp"                // for MSET
#include "toolbox/logging.hpp"          // for dolog
#include "vm/exceptions.hpp"
#include "vm/finalizer.hpp"             // for finalizer_notify, etc
#include "vm/options.hpp"
#include "vm/os.hpp"                    // for os
#include "vm/rt-timing.hpp"             // for RT_REGISTER_GROUP, etc

#include "gc-boehm.hpp"
#include "boehm-gc/include/javaxfc.h"   // for GC_finalize_all

struct methodinfo;

/* global variables ***********************************************************/

static bool in_gc_out_of_memory = false;    /* is GC out of memory?           */
static size_t gc_max_heap_size = 0;


/* prototype static functions *************************************************/

static void gc_ignore_warnings(char *msg, GC_word arg);


/* gc_init *********************************************************************

   Initializes the boehm garbage collector.

*******************************************************************************/

void gc_init(size_t heapmaxsize, size_t heapstartsize)
{
	size_t heapcurrentsize;

	TRACESUBSYSTEMINITIALIZATION("gc_init");

	/* just to be sure (should be set to 1 by JAVA_FINALIZATION macro) */

	GC_java_finalization = 1;

	/* Ignore pointers that do not point to the start of an object. */

	GC_all_interior_pointers = 0;

	/* suppress warnings */

	GC_set_warn_proc(gc_ignore_warnings);

	/* install a GC notifier */

	GC_finalize_on_demand = 1;
	GC_finalizer_notifier = finalizer_notify;

	/* define OOM function */

	GC_oom_fn = gc_out_of_memory;

	GC_INIT();

	/* set the maximal heap size */

	GC_set_max_heap_size(heapmaxsize);
	gc_max_heap_size = heapmaxsize;

	/* set the initial heap size */

	heapcurrentsize = GC_get_heap_size();

	if (heapstartsize > heapcurrentsize)
		GC_expand_hp(heapstartsize - heapcurrentsize);
}


static void gc_ignore_warnings(char *msg, GC_word arg)
{
}


void *heap_alloc_uncollectable(size_t size)
{
	void *p;

	p = GC_MALLOC_UNCOLLECTABLE(size);

	/* clear allocated memory region */

	MSET(p, 0, uint8_t, size);

	return p;
}


// register heap timer
RT_REGISTER_GROUP(heap_group,"heap","heap time")
// register heap timer
RT_REGISTER_GROUP_TIMER(heap_timer,"heap","allocation time",heap_group)

/* heap_alloc ******************************************************************

   Allocates memory on the Java heap.

*******************************************************************************/

void *heap_alloc(size_t size, int references, methodinfo *finalizer, bool collect)
{
	void *p;

	RT_TIMER_START(heap_timer);

	/* We can't use a bool here for references, as it's passed as a
	   bitmask in builtin_new.  Thus we check for != 0. */

	if (references != 0)
		p = GC_MALLOC(size);
	else
		p = GC_MALLOC_ATOMIC(size);

	if (p == NULL)
		return NULL;

	if (finalizer != NULL)
		GC_REGISTER_FINALIZER_NO_ORDER(p, finalizer_run, 0, 0, 0);

	/* clear allocated memory region */

	MSET(p, 0, uint8_t, size);

	RT_TIMER_STOP(heap_timer);

	return p;
}


void heap_free(void *p)
{
	GC_FREE(p);
}

void gc_call(void)
{
  	if (opt_verbosegc)
		dolog("Garbage Collection:  previous/now = %d / %d ",
			  0, 0);

	GC_gcollect();
}


int64_t gc_get_heap_size(void)
{
	return GC_get_heap_size();
}


int64_t gc_get_free_bytes(void)
{
	return GC_get_free_bytes();
}


/* gc_get_total_bytes **********************************************************

   Returns the number of total bytes currently used on the Java heap.

*******************************************************************************/

int64_t gc_get_total_bytes(void)
{
	return GC_get_total_bytes();
}


int64_t gc_get_max_heap_size(void)
{
	return gc_max_heap_size;
}


void gc_invoke_finalizers(void)
{
	GC_invoke_finalizers();
}


void gc_finalize_all(void)
{
	GC_finalize_all();
}


/* gc_out_of_memory ************************************************************

   This function is called when boehm detects that it is OOM.

*******************************************************************************/

void *gc_out_of_memory(size_t bytes_requested)
{
	/* if this happens, we are REALLY out of memory */

	if (in_gc_out_of_memory) {
		/* this is all we can do... */
		os::abort("gc_out_of_memory: out of memory");
	}

	in_gc_out_of_memory = true;

	/* try to release some memory */

	gc_call();

	/* now instantiate the exception */

	exceptions_throw_outofmemoryerror();

	in_gc_out_of_memory = false;

	return NULL;
}

void gc_register_current_thread()
{
#ifdef ENABLE_THREADS
	// Register the thread with Boehm-GC.  
	// This must happen before the thread allocates any memory from the GC heap.

	struct GC_stack_base sb;

	if (GC_get_stack_base(&sb) != GC_SUCCESS)
		vm_abort("threads_attach_current_thread: GC_get_stack_base failed");

	GC_register_my_thread(&sb);
#endif
}

void gc_unregister_current_thread()
{
#ifdef ENABLE_THREADS
	GC_unregister_my_thread();
#endif
}


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
 * vim:noexpandtab:sw=4:ts=4:
 */
