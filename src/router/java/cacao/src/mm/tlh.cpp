/* src/mm/tlh.c

   Copyright (C) 2008-2013
   CACAOVM - Verein zu Foerderung der freien virtuellen Machine CACAO

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

#include "mm/tlh.hpp"
#include <assert.h>                     // for assert
#include <sys/mman.h>                   // for mmap, munmap, MAP_ANONYMOUS, etc
#include "config.h"                     // for SIZEOF_VOID_P
#include "mm/memory.hpp"                // for MZERO

static const int TLH_MAX_SIZE = (20 * 1024 * 1024);

static inline bool tlh_avail(tlh_t *tlh, unsigned n) {
	/*
    ---  --- --- ---
                    ^ end
            ^ top
                    ^ top + 2
	*/
	return (tlh->top + n) <= tlh->end;
}

void tlh_init(tlh_t *tlh) {

	void *heap = mmap(
		NULL,
		TLH_MAX_SIZE,
		PROT_READ|PROT_WRITE,
		MAP_ANONYMOUS|MAP_PRIVATE,
		-1,
		0
	);

	if (heap == MAP_FAILED) {
		/* The top pointer points to end, so all allocations will fail. */
		tlh->start = NULL;
		tlh->end = NULL;
		tlh->top = NULL;
		tlh->base = NULL;
	} else {
		tlh->start = (uint8_t *)heap;
		tlh->top = tlh->start;
		tlh->base = tlh->start;
		tlh->end = tlh->start + TLH_MAX_SIZE;
	}

	tlh->overflows = 0;
}

void tlh_destroy(tlh_t *tlh) {
	int res = munmap(tlh->start, TLH_MAX_SIZE);
	if (res == -1) {
		/* TODO */
		assert(0);
	}
	tlh->start = NULL;
	tlh->end = NULL;
	tlh->top = NULL;
	tlh->base = NULL;
}

void tlh_add_frame(tlh_t *tlh) {
	if (tlh_avail(tlh, SIZEOF_VOID_P)) {
		*(uint8_t **)tlh->top = tlh->base;
		tlh->base = tlh->top;
		tlh->top += SIZEOF_VOID_P;
	} else {
		tlh->overflows += 1;
	}
}

void tlh_remove_frame(tlh_t *tlh) {
	if (tlh->overflows > 0) {
		tlh->overflows -= 1;
	} else {
		tlh->top = tlh->base;
		tlh->base = *(uint8_t **)tlh->top;
	}
}

void *tlh_alloc(tlh_t *tlh, size_t size) {
	void *ret;
	if (tlh_avail(tlh, size)) {
		ret = tlh->top;
		tlh->top += size;
		MZERO(ret, char, size);
	} else {
		ret = NULL;
	}
	return ret;
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
