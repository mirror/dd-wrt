/*
 * Copyright (c) 2007 Damien Miller.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "common.h"
#include "freelist.h"
#include "log.h"

#define FREELIST_MAX_ALLOC	0x1000000
#define FREELIST_ALLOC_ALIGN	16
#define FREELIST_INITIAL_ALLOC	16

#ifndef roundup
#define roundup(x, y) ((((x) + (y) - 1)/(y))*(y))
#endif /* roundup */

#undef FREELIST_DEBUG
#ifdef FREELIST_DEBUG
# define FLOGIT(a) logit a
#else
# define FLOGIT(a)
#endif

void
freelist_init(struct freelist *fl, size_t allocsz)
{
	FLOGIT((LOG_DEBUG, "%s: %s(%p, %zu)", __func__, __func__, fl, allocsz));
	bzero(fl, sizeof(fl));
	fl->allocsz = roundup(allocsz, FREELIST_ALLOC_ALIGN);
	fl->free_entries = NULL;
}

static int
freelist_grow(struct freelist *fl)
{
	size_t i, oldnalloc, need;
	void *p;

	FLOGIT((LOG_DEBUG, "%s: %s(%p)", __func__, __func__, fl));
	FLOGIT((LOG_DEBUG, "%s: nalloc = %zu", __func__, fl->nalloc));

	/* Sanity check */
	if (fl->nalloc > FREELIST_MAX_ALLOC)
		return -1;

	oldnalloc = fl->nalloc;
	if (fl->nalloc == 0)
		fl->nalloc = FREELIST_INITIAL_ALLOC;
	else
		fl->nalloc <<= 1;
	if (fl->nalloc > FREELIST_MAX_ALLOC)
		fl->nalloc = FREELIST_MAX_ALLOC;
	FLOGIT((LOG_DEBUG, "%s: nalloc now %zu", __func__, fl->nalloc));

	/* Check for integer overflow */
	if (SIZE_MAX / fl->nalloc < fl->allocsz ||
	    SIZE_MAX / fl->nalloc < sizeof(*fl->free_entries)) {
		FLOGIT((LOG_DEBUG, "%s: integer overflow", __func__));
 resize_fail:
		fl->nalloc = oldnalloc;
		return -1;
	}

	/* Allocate freelist - max size of nalloc */
	need = fl->nalloc * sizeof(*fl->free_entries);
	if ((p = realloc(fl->free_entries, need)) == NULL) {
		FLOGIT((LOG_DEBUG, "%s: realloc(%zu) failed", __func__, need));
		goto resize_fail;
	}

	/* Allocate the entries */
	fl->free_entries = p;
	need = (fl->nalloc - oldnalloc) * fl->allocsz;
	if ((p = malloc(need)) == NULL) {
		FLOGIT((LOG_DEBUG, "%s: malloc(%zu) failed", __func__, need));
		goto resize_fail;
	}
	/*
	 * XXX store these malloc ranges in a tree or list, so we can
	 * validate them in _get/_put. Check that r_low <= addr < r_high, and
	 * (addr - r_low) % fl->allocsz == 0
	 */

	fl->navail = fl->nalloc - oldnalloc;
	for (i = 0; i < fl->navail; i++)
		fl->free_entries[i] = (u_char *)p + (i * fl->allocsz);
	for (i = fl->navail; i < fl->nalloc; i++)
		fl->free_entries[i] = NULL;

	FLOGIT((LOG_DEBUG, "%s: done, navail = %zu", __func__, fl->navail));
	return 0;
}

void *
freelist_get(struct freelist *fl)
{
	void *r;

	FLOGIT((LOG_DEBUG, "%s: %s(%p)", __func__, __func__, fl));
	FLOGIT((LOG_DEBUG, "%s: navail = %zu", __func__, fl->navail));

	if (fl->navail == 0) {
		if (freelist_grow(fl) == -1)
			return NULL;
	}

	/* Sanity check */
	if (fl->navail == 0 || fl->navail > FREELIST_MAX_ALLOC ||
	    fl->free_entries[fl->navail - 1] == NULL) {
		logit(LOG_ERR, "%s: invalid navail", __func__);
		raise(SIGSEGV);
	}

	fl->navail--;
	r = fl->free_entries[fl->navail];
	fl->free_entries[fl->navail] = NULL;

	FLOGIT((LOG_DEBUG, "%s: done, navail = %zu", __func__, fl->navail));
	return r;
}

void
freelist_put(struct freelist *fl, void *p)
{
	FLOGIT((LOG_DEBUG, "%s: %s(%p, %zu)", __func__, __func__, fl, p));
	FLOGIT((LOG_DEBUG, "%s: navail = %zu", __func__, fl->navail));
	FLOGIT((LOG_DEBUG, "%s: nalloc = %zu", __func__, fl->navail));

	/* Sanity check */
	if (fl->navail >= fl->nalloc) {
		logit(LOG_ERR, "%s: freelist navail >= nalloc", __func__);
		raise(SIGSEGV);
	}
	if (fl->free_entries[fl->navail] != NULL) {
		logit(LOG_ERR, "%s: free_entries[%lu] != NULL",
		    __func__, (unsigned long)fl->navail);
		raise(SIGSEGV);
	}

	fl->free_entries[fl->navail] = p;
	fl->navail++;

	FLOGIT((LOG_DEBUG, "%s: done, navail = %zu", __func__, fl->navail));
}


