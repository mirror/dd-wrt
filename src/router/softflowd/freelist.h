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

#ifndef _FREELIST_H
#define _FREELIST_H

#include "common.h"

/* Simple freelist of fixed-sized allocations */
struct freelist {
	size_t allocsz;
	size_t nalloc;
	size_t navail;
	void **free_entries;
};

/*
 * Initialise a freelist.
 * allocsz is the size of the individual allocations
 */
void freelist_init(struct freelist *freelist, size_t allocsz);

/*
 * Get an entry from a freelist.
 * Will allocate new entries if necessary
 * Returns pointer to allocated memory or NULL on failure.
 */
void *freelist_get(struct freelist *freelist);

/*
 * Returns an entry to the freelist.
 * p must be a pointer to an allocation from the freelist.
 */
void freelist_put(struct freelist *freelist, void *p);

#endif /* FREELIST_H */

