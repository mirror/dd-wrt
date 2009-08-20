/*
 * HND RTE packet buffer routines.
 *
 * No caching,
 * Just a thin packet buffering data structure layer atop hndrte_malloc/free .
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: hndrte_lbuf.c,v 1.39.134.1 2008/05/24 00:21:48 Exp $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <hndrte.h>
#include <bcmutils.h>
#include <hndrte_lbuf.h>

#ifdef BCMDBG
#define LBUF_MSG(x) printf x
#else
#define LBUF_MSG(x)
#endif

static const uint lbsize[] = {
	MAXPKTBUFSZ >> 4,
	MAXPKTBUFSZ >> 3,
	MAXPKTBUFSZ >> 2,
	MAXPKTBUFSZ >> 1,
	MAXPKTBUFSZ,
	4096 + LBUFSZ		/* ctrl queries on bus can be 4K */
};

void
lb_init()
{
	ASSERT(sizeof(struct lbuf) == LBUFSZ);
}

struct lbuf *
#if defined(BCMDBG_MEM) || defined(BCMDBG_MEMFAIL)
lb_alloc(uint size, char *file, int line)
#else
lb_alloc(uint size)
#endif
{
	struct lbuf *lb;
	uint tot;
	int i;

#if defined(BCMDBG_MEM) || defined(BCMDBG_MEMFAIL)
	if (!file)
		file = "unknown";

	tot = 0;
	for (i = 0; i < ARRAYSIZE(lbsize); i++)
		if ((LBUFSZ + ROUNDUP(size, sizeof(int))) <= lbsize[i]) {
			tot = lbsize[i];
			break;
		}
	if (tot == 0) {
		printf("lb_alloc: size too big (%ld); alloc failed; file %s; line %d\n",
		       (LBUFSZ + size), file, line);
		return (NULL);
	}

	if ((lb = (struct lbuf*)hndrte_malloc_align(tot, 0, file, line)) == NULL) {
		printf("lb_alloc: size (%ld); alloc failed; file %s; line %d\n", (LBUFSZ + size),
		       file, line);
		return (NULL);
	}

#else

	tot = 0;
	for (i = 0; i < ARRAYSIZE(lbsize); i++)
		if ((LBUFSZ + ROUNDUP(size, sizeof(int))) <= lbsize[i]) {
			tot = lbsize[i];
			break;
		}
	if (tot == 0) {
		LBUF_MSG(("lb_alloc: size too big (%ld); alloc failed;\n",
		       (LBUFSZ + size)));
		return (NULL);
	}

	if ((lb = (struct lbuf*)hndrte_malloc_align(tot, 0)) == NULL) {
		LBUF_MSG(("lb_alloc: size (%ld); alloc failed;\n", (LBUFSZ + size)));
		return (NULL);
	}
#endif /* BCMDBG_MEM */

	ASSERT(ISALIGNED((uintptr)lb, sizeof(int)));

	bzero((char*)lb, LBUFSZ);

	lb->head = (uchar*) &lb[1];
	lb->end = lb->head + tot - LBUFSZ;
	lb->data = lb->end - ROUNDUP(size, sizeof(int));
	lb->len = size;

	return (lb);
}

void
lb_free(struct lbuf *lb)
{
	struct lbuf *next;

	while (lb) {
		ASSERT(lb_sane(lb));
		ASSERT(lb->link == NULL);

		next = lb->next;
		lb->data = (uchar*) 0xdeadbeef;
		hndrte_free(lb);
		lb = next;
	}
}

struct lbuf *
lb_dup(struct lbuf *lb)
{
	struct lbuf *lb_dup;

#if defined(BCMDBG_MEM) || defined(BCMDBG_MEMFAIL)
	if (!(lb_dup = lb_alloc(lb->len, __FILE__, __LINE__)))
#else
	if (!(lb_dup = lb_alloc(lb->len)))
#endif
		return (NULL);

	bcopy(lb->data, lb_dup->data, lb->len);

	return (lb_dup);
}

bool
lb_sane(struct lbuf *lb)
{
	int insane = 0;

	insane |= (lb->data < lb->head);
	insane |= (lb->data + lb->len > lb->end);

	if (insane)
		LBUF_MSG(("lb_sane:\nlbuf %p data %p head %p end %p len %d flags %d\n",
		       lb, lb->data, lb->head, lb->end, lb->len, lb->flags));

	return (!insane);
}
