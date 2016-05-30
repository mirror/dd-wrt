/* src/mm/cacao-gc/gc.h - main garbage collector header

   Copyright (C) 2006-2013
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


#ifdef GC_CONST
# error Why is the BoehmGC header included???
#endif


#ifndef _GC_H
#define _GC_H


#include "config.h"
#include "vm/types.hpp"

#include "threads/thread.hpp"

#include "toolbox/list.hpp"
#include "vm/jit/replace.hpp"


/* Configuration Switches *****************************************************/

#define GCCONF_FINALIZER
/*#define GCCONF_HDRFLAG_REFERENCING*/


/* Debugging ******************************************************************/

#define GC_DEBUGGING
/*#define GC_DEBUGGING2*/

#if !defined(NDEBUG) && defined(GC_DEBUGGING)
# include <assert.h>
# include "vm/options.hpp"
# define GC_LOG(code) if (opt_verbosegc) { code; }
# define GC_ASSERT(assertion) assert(assertion)
#else
# define GC_LOG(code)
# define GC_ASSERT(assertion)
#endif

#if !defined(NDEBUG) && defined(GC_DEBUGGING2)
# define GC_LOG2(code) GC_LOG(code)
#else
# define GC_LOG2(code)
#endif


/* Development Break **********************************************************/

#if 0 && defined(ENABLE_THREADS)
# error "GC does not work with threads enabled!"
#endif

#if 1 && defined(ENABLE_INTRP)
# error "GC does not work with interpreter enabled!"
#endif

#if 1 && defined(ENABLE_JVMTI)
# error "GC does not work with JVMTI enabled!"
#endif

#if 1 && !defined(ENABLE_REPLACEMENT)
# error "GC does only work with replacement enabled!"
#endif

#if 1 && !defined(ENABLE_HANDLES)
# error "GC does only work with handles (indirection cells) enabled!"
#endif

#if 1 && !defined(__ALPHA__) && !defined(__ARM__) && !defined(__I386__) && !defined(__POWERPC__) && !defined(__X86_64__) && !defined(__SPARC_64__)
# error "GC was only ported to some architectures so far!"
#endif


/* Helper Macros **************************************************************/

#define GC_SET_FLAGS(obj, flags)   ((obj)->hdrflags |=  (flags))
#define GC_CLEAR_FLAGS(obj, flags) ((obj)->hdrflags &= ~(flags))
#define GC_TEST_FLAGS(obj, flags)  ((obj)->hdrflags  &  (flags))

#define POINTS_INTO(ptr, ptr_start, ptr_end) \
	((void *) (ptr) >= (ptr_start) && (void *) (ptr) < (ptr_end))

#define GC_ALIGN_SIZE SIZEOF_VOID_P
#define GC_ALIGN(val,size) ((((val) + (size) - 1) / (size)) * (size))


/* Global Variables ***********************************************************/

extern bool gc_pending;
extern bool gc_notify_finalizer;

extern list_t *gc_reflist_strong;
extern list_t *gc_reflist_weak;


/* Structures *****************************************************************/

typedef struct list_gcref_entry_t list_gcref_entry_t;

struct list_gcref_entry_t {
	listnode_t      linkage;
	java_object_t **ref;
#if !defined(NDEBUG)
	s4              reftype;
#endif
};


/* Global GC mutext stuff *****************************************************/

#if defined(ENABLE_THREADS)
# define GC_MUTEX_LOCK   threads_mutex_gc_lock()
# define GC_MUTEX_UNLOCK threads_mutex_gc_unlock()
#else
# define GC_MUTEX_LOCK
# define GC_MUTEX_UNLOCK
#endif


/* No-Thread specific stuff ***************************************************/

#if defined(ENABLE_THREADS)
# define GC_EXECUTIONSTATE (thread->es)
# define GC_SOURCESTATE    (thread->ss)
#else
# define GC_EXECUTIONSTATE (_no_threads_executionstate)
# define GC_SOURCESTATE    (_no_threads_sourcestate)

extern executionstate_t *_no_threads_executionstate;
extern sourcestate_t    *_no_threads_sourcestate;

#endif


/* Prototypes *****************************************************************/

void gc_collect(s4 level);

#if defined(ENABLE_THREADS)
bool gc_suspend(threadobject *thread, u1 *pc, u1 *sp);
#endif


/* Statistics *****************************************************************/

#if defined(ENABLE_STATISTICS)

#define GCSTAT_INIT(cnt)  { (cnt) = 0; }
#define GCSTAT_COUNT(cnt) { (cnt)++; }
#define GCSTAT_DEC(cnt)   { (cnt)--; GC_ASSERT((cnt) >= 0); }
#define GCSTAT_COUNT_MAX(cnt,max) { (cnt)++; if ((cnt) > (max)) (max) = (cnt); }

extern int gcstat_collections;
extern int gcstat_collections_forced;
extern int gcstat_mark_depth;
extern int gcstat_mark_depth_max;
extern int gcstat_mark_count;

void gcstat_println();

#else /* defined(ENABLE_STATISTICS) */

#define GCSTAT_INIT(cnt)
#define GCSTAT_COUNT(cnt)
#define GCSTAT_DEC(cnt)
#define GCSTAT_COUNT_MAX(cnt,max)

#endif /* defined(ENABLE_STATISTICS) */


#endif /* _GC_H */

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
