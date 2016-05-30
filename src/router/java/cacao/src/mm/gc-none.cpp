/* src/mm/gc-none.cpp - allocates memory through malloc (no GC)

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

#include <stdlib.h>

#include "vm/types.hpp"

#include "boehm-gc/include/gc.h"

#include "mm/gc.hpp"
#include "mm/memory.hpp"

#include "toolbox/logging.hpp"

#include "vm/jit/builtin.hpp"
#include "vm/global.hpp"
#include "vm/loader.hpp"
#include "vm/options.hpp"
#include "vm/os.hpp"
#include "vm/vm.hpp"


/* global stuff ***************************************************************/

#define MMAP_HEAPADDRESS    0x10000000  /* try to map the heap to this addr.  */
#define ALIGNSIZE           8

static void *mmapptr = NULL;
static int mmapsize = 0;
static void *mmaptop = NULL;


void* heap_alloc(size_t size, int references, methodinfo *finalizer, bool collect)
{
	void *m;

	mmapptr = (void *) MEMORY_ALIGN((ptrint) mmapptr, ALIGNSIZE);
	
	m = mmapptr;
	mmapptr = (void *) ((ptrint) mmapptr + size);

	if (mmapptr > mmaptop)
		vm_abort("heap_alloc: out of memory");

	MSET(m, 0, u1, size);

	return m;
}


void* heap_alloc_uncollectable(size_t size)
{
	return heap_alloc(size, false, NULL, false);
}


void heap_free(void* p)
{
	/* nop */
}



void gc_init(size_t heapmaxsize, size_t heapstartsize)
{
	heapmaxsize = MEMORY_ALIGN(heapmaxsize, ALIGNSIZE);

#if defined(HAVE_MMAP)
	mmapptr = mmap((void *) MMAP_HEAPADDRESS,
				   (size_t) heapmaxsize,
				   PROT_READ | PROT_WRITE,
				   MAP_PRIVATE |
# if defined(MAP_ANONYMOUS)
				   MAP_ANONYMOUS,
# elif defined(MAP_ANON)
				   MAP_ANON,
# else
				   0,
# endif
				   -1,
				   (off_t) 0);

	if (mmapptr == MAP_FAILED)
		vm_abort("gc_init: out of memory");
#else
	mmapptr = malloc(heapmaxsize);

	if (mmapptr == NULL)
		vm_abort("gc_init: out of memory");
#endif

	mmapsize = heapmaxsize;
	mmaptop = (void *) ((ptrint) mmapptr + mmapsize);
}


void gc_call(void)
{
	log_text("GC call: nothing done...");
	/* nop */
}


s8 gc_get_heap_size(void)
{
	return 0;
}


s8 gc_get_free_bytes(void)
{
	return 0;
}


s8 gc_get_total_bytes(void)
{
	return 0;
}


s8 gc_get_max_heap_size(void)
{
	return 0;
}


void gc_invoke_finalizers(void)
{
	/* nop */
}


void gc_finalize_all(void)
{
	/* nop */
}


void gc_register_current_thread()
{
	/* nop */
}

void gc_unregister_current_thread()
{
	/* nop */
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
 */
