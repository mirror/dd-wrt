/* src/mm/memory.cpp - memory management

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

#include "mm/memory.hpp"
#include <assert.h>                     // for assert
#include <stdint.h>                     // for int32_t
#include <stdlib.h>                     // for realloc
#include "config.h"                     // for ENABLE_MEMCHECK, etc
#include "threads/thread.hpp"           // for threads_sleep, etc
#include "toolbox/logging.hpp"          // for log_text
#include "vm/options.hpp"               // for opt_ProfileGCMemoryUsage, etc
#include "vm/os.hpp"                    // for os
#include "vm/types.hpp"                 // for u1
#include "vm/utf8.hpp"                  // for Utf8String
#include "vm/vm.hpp"                    // for vm_abort
#include "vm/statistics.hpp"

#if defined(__DARWIN__)
/* If we compile with -ansi on darwin, <sys/types.h> is not
   included. So let's do it here. */
# include <sys/types.h>
#endif

STAT_DECLARE_GROUP(memory_stat)
STAT_REGISTER_SUBGROUP(max_mem_stat,"max. mem","max. memory",memory_stat)
STAT_REGISTER_SUBGROUP(not_freed_mem_stat,"not freed","not freed",memory_stat)
STAT_REGISTER_GROUP_VAR(s4,maxmemusage,0,"maxmemusage","max. heap memory",max_mem_stat)
STAT_REGISTER_GROUP_VAR(s4,memoryusage,0,"memoryusage","heap memory not freed",not_freed_mem_stat)
/* memory_mprotect *************************************************************

   Convenience function for mprotect.  This function also does error
   checking.

*******************************************************************************/

void memory_mprotect(void *addr, size_t len, int prot)
{
	if (os::mprotect(addr, len, prot) != 0)
		os::abort_errno("memory_mprotect: os::mprotect failed");
}


/* memory_checked_alloc ********************************************************

   Allocated zeroed-out memory and does an OOM check.

   ERROR HANDLING:
      XXX If no memory could be allocated, this function justs *exists*.

*******************************************************************************/

void *memory_checked_alloc(size_t size)
{
	/* always allocate memory zeroed out */

	void *p = os::calloc(size, 1);

	if (p == NULL)
		vm_abort("memory_checked_alloc: calloc failed: out of memory");

	return p;
}


void *mem_alloc(int32_t size)
{
	void *m;

	if (size == 0)
		return NULL;

	STATISTICS(memoryusage += size);
	STATISTICS(maxmemusage.max(memoryusage.get()));

	m = memory_checked_alloc(size);

#if defined(ENABLE_MEMCHECK)
	/* XXX we would like to poison the memory, but callers rely on */
	/* the zeroing. This should change sooner or later.            */
	/* memset(m, MEMORY_CLEAR_BYTE, size); */
#endif

	return m;
}


void *mem_realloc(void *src, int32_t len1, int32_t len2)
{
	void *dst;

	/* prevent compiler warnings */

 	dst = NULL;

	if (src == NULL)
		if (len1 != 0)
			vm_abort("mem_realloc: reallocating memoryblock with address NULL, length != 0");

	STATISTICS(memoryusage += len2 - len1);

#if defined(ENABLE_MEMCHECK)
	if (len2 < len1)
		os::memset((u1*)dst + len2, MEMORY_CLEAR_BYTE, len1 - len2);
#endif

	dst = realloc(src, len2);

	if (dst == NULL)
		vm_abort("mem_realloc: realloc failed: out of memory");

#if defined(ENABLE_MEMCHECK)
	if (len2 > len1)
		os::memset((u1*)dst + len1, MEMORY_CLEAR_BYTE, len2 - len1);
#endif

	return dst;
}


void mem_free(void *m, int32_t size)
{
	if (!m) {
		if (size == 0)
			return;

		log_text("returned memoryblock with address NULL, length != 0");
		assert(0);
	}

	STATISTICS(memoryusage -= size);

#if defined(ENABLE_MEMCHECK)
	/* destroy the contents */
	os::memset(m, MEMORY_CLEAR_BYTE, size);
#endif

	os::free(m);
}


/* memory_thread ***************************************************************

   Prints regularly memory statistics.

*******************************************************************************/

static void memory_thread(void)
{
	int32_t seconds;

	/* Prevent compiler warning. */

	seconds = 1;

	/* If both arguments are specified, use the value of
	   ProfileMemoryUsage. */

	if (opt_ProfileGCMemoryUsage)
		seconds = opt_ProfileGCMemoryUsage;

	if (opt_ProfileMemoryUsage)
		seconds = opt_ProfileMemoryUsage;

	while (true) {
		/* sleep thread */

		threads_sleep(seconds * 1000, 0);

# if 0 && defined(ENABLE_STATISTICS)
		/* Print current date and time (only when we print to the
		   stdout). */

		if (!opt_ProfileMemoryUsageGNUPlot)
			statistics_print_date();

		/* print memory usage */

		if (opt_ProfileMemoryUsage)
			statistics_print_memory_usage();

		/* print GC memory usage */

		if (opt_ProfileGCMemoryUsage)
			statistics_print_gc_memory_usage();
# endif
	}
}


/* memory_start_thread *********************************************************

   Starts the memory profiling thread.

*******************************************************************************/

bool memory_start_thread(void)
{
	Utf8String name = Utf8String::from_utf8("Memory Profiler");

	/* start the memory profiling thread */

	if (!threads_thread_start_internal(name, memory_thread))
		return false;

	/* everything's ok */

	return true;
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
