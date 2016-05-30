/* src/mm/gc.hpp - gc independant interface for heap managment

   Copyright (C) 1996-2005, 2006, 2007, 2008
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


#ifndef _GC_HPP
#define _GC_HPP

#include <stddef.h>                     // for size_t
#include <stdint.h>                     // for int32_t, etc.
#include "threads/thread.hpp"
#include "vm/global.hpp"                // for java_object_t

class GC {
public:
};


/**
 * Critical section for the GC.
 */
class GCCriticalSection {
public:
	GCCriticalSection()  { enter(); }
	~GCCriticalSection() { leave(); }

	inline static void enter ();
	inline static void leave ();
	inline static bool inside();
};

/**
 * Enters a LLNI critical section which prevents the GC from moving
 * objects around on the collected heap.
 *
 * There are no race conditions possible while entering such a critical
 * section, because each thread only modifies its own thread local flag
 * and the GC reads the flags while the world is stopped.
 */
void GCCriticalSection::enter()
{
#if defined(ENABLE_GC_CACAO)
	threadobject* t = thread_get_current();

	// Sanity check.
	assert(t->gc_critical == false);

	t->gc_critical = true;
#endif
}

/**
 * Leaves a LLNI critical section and allows the GC to move objects
 * around on the collected heap again.
 */
void GCCriticalSection::leave()
{
#if defined(ENABLE_GC_CACAO)
	threadobject* t = thread_get_current();

	// Sanity check.
	assert(t->gc_critical == true);

	t->gc_critical = false;
#endif
}


/**
 * Checks if the calling thread is inside a GC critical section.
 *
 * @return true if inside, false otherwise.
 */
bool GCCriticalSection::inside()
{
#if defined(ENABLE_GC_CACAO)
	threadobject* t = thread_get_current();
	return t->gc_critical;
#else
	return true;
#endif
}

/* reference types ************************************************************/

enum {
	GC_REFTYPE_THREADOBJECT,
	GC_REFTYPE_CLASSLOADER,
	GC_REFTYPE_JNI_GLOBALREF,
	GC_REFTYPE_FINALIZER,
	GC_REFTYPE_LOCALREF,
	GC_REFTYPE_STACK,
	GC_REFTYPE_CLASSREF,
	GC_REFTYPE_LOCKRECORD
};

/* function prototypes ********************************************************/

void    gc_init(size_t heapmaxsize, size_t heapstartsize);

void*   heap_alloc_uncollectable(size_t size);
void*   heap_alloc(size_t size, int references, methodinfo *finalizer, bool collect);
void    heap_free(void *p);

#if defined(ENABLE_GC_CACAO)
void    heap_init_objectheader(java_object_t *o, uint32_t size);
int32_t heap_get_hashcode(java_object_t *o);

void    gc_reference_register(java_object_t **ref, int32_t reftype);
void    gc_reference_unregister(java_object_t **ref);

void    gc_weakreference_register(java_object_t **ref, int32_t reftype);
void    gc_weakreference_unregister(java_object_t **ref);
#endif

void    gc_call(void);
int64_t gc_get_heap_size(void);
int64_t gc_get_free_bytes(void);
int64_t gc_get_total_bytes(void);
int64_t gc_get_max_heap_size(void);
void    gc_invoke_finalizers(void);
void    gc_finalize_all(void);
void*   gc_out_of_memory(size_t bytes_requested);

void gc_register_current_thread();
void gc_unregister_current_thread();

/* inlined functions **********************************************************/

static inline int32_t heap_hashcode(java_object_t* obj)
{
#if defined(ENABLE_GC_CACAO)
	return heap_get_hashcode(obj);
#else
	return (int32_t)(intptr_t) obj;
#endif
}

#endif // _GC_HPP


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
