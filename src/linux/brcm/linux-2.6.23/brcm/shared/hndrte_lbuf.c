/*
 * HND RTE packet buffer routines.
 *
 * No caching,
 * Just a thin packet buffering data structure layer atop hndrte_malloc/free .
 *
 * Copyright (C) 2009, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: hndrte_lbuf.c,v 1.44 2009/01/28 19:56:59 Exp $
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

#define LBBUF_BIN_SIZE	6
static const uint lbsize[LBBUF_BIN_SIZE] = {
	MAXPKTBUFSZ >> 4,
	MAXPKTBUFSZ >> 3,
	MAXPKTBUFSZ >> 2,
	MAXPKTBUFSZ >> 1,
	MAXPKTBUFSZ,
	4096 + LBUFSZ		/* ctrl queries on bus can be 4K */
};

static uint lbbuf_hist[LBBUF_BIN_SIZE] = {0};
static uint lbbuf_allocfail = 0;

void
lb_init()
{
	ASSERT(sizeof(struct lbuf) == LBUFSZ);
}

struct lbuf *
#if defined(BCMDBG_MEM) || defined(BCMDBG_MEMFAIL)
lb_alloc(uint size, const char *file, int line)
#else
lb_alloc(uint size)
#endif
{
	struct lbuf *lb;
	uint tot;
	int i;
	bool from_pt = FALSE;

	tot = 0;

#ifdef HNDRTE_PT_GIANT
	if (size > MAXPKTBUFSZ + LBUFSZ) {	/* giant rx pkt, get from fixed block partition */

		tot = LBUFSZ + ROUNDUP(size, sizeof(int));
		/* ASSERT(tot <= MEM_PT_BLKSIZE); */
		if ((lb = (struct lbuf*)hndrte_malloc_pt(tot)) != NULL) {
			from_pt = TRUE;
			goto success;
		}

		LBUF_MSG(("lb_alloc: size (%ld); alloc failed;\n", tot));
		goto error;
	}
#endif

	for (i = 0; i < ARRAYSIZE(lbsize); i++)
		if ((LBUFSZ + ROUNDUP(size, sizeof(int))) <= lbsize[i]) {
			tot = lbsize[i];
			lbbuf_hist[i]++;
			break;
		}

#if defined(BCMDBG_MEM) || defined(BCMDBG_MEMFAIL)
	if (!file)
		file = "unknown";

	if (tot == 0) {
		printf("lb_alloc: size too big (%ld); alloc failed; file %s; line %d\n",
		       (LBUFSZ + size), file, line);
		goto error;
	}

	if ((lb = (struct lbuf*)hndrte_malloc_align(tot, 0, file, line)) == NULL) {
		printf("lb_alloc: size (%ld); alloc failed; file %s; line %d\n", (LBUFSZ + size),
		       file, line);
		goto error;
	}

#else
	if (tot == 0) {
		LBUF_MSG(("lb_alloc: size too big (%ld); alloc failed;\n",
		       (LBUFSZ + size)));
		goto error;
	}

	if ((lb = (struct lbuf*)hndrte_malloc_align(tot, 0)) == NULL) {
		LBUF_MSG(("lb_alloc: size (%ld); alloc failed;\n", (LBUFSZ + size)));
		goto error;
	}
#endif /* BCMDBG_MEM || BCMDBG_MEMFAIL */

success:
	ASSERT(ISALIGNED((uintptr)lb, sizeof(int)));

	bzero((char*)lb, LBUFSZ);

	lb->head = (uchar*) &lb[1];
	lb->end = lb->head + tot - LBUFSZ;
	lb->data = lb->end - ROUNDUP(size, sizeof(int));
	lb->len = size;
	lb->refcnt = 1;

	if (from_pt)
		lb->flags |= LBF_PTBLK;

	return (lb);

error:
	lbbuf_allocfail++;
	return NULL;

	if (0)	/* for compile error */
		goto success;
}

static void
lb_free_one(struct lbuf *lb)
{
	ASSERT(lb_sane(lb));

	if (!lb_isclone(lb)) {
		ASSERT(lb->refcnt > 0);
		if (--lb->refcnt == 0) {
			if (!lb_isptblk(lb)) {
				lb->data = (uchar*) 0xdeadbeef;
				hndrte_free(lb);
			} else {
#ifdef HNDRTE_PT_GIANT
				hndrte_free_pt(lb);
#else
				ASSERT(0);
#endif
			}
		}
	} else {
		struct lbuf *orig = ((struct lbuf_clone*)lb)->orig;

		/* clones do not get refs, just originals */
		ASSERT(lb->refcnt == 0);
		lb->data = (uchar*) 0xdeadbeef;
		hndrte_free(lb);

		lb_free_one(orig);
	}
}

void
lb_free(struct lbuf *lb)
{
	struct lbuf *next;

	while (lb) {
		ASSERT(lb->link == NULL);

		next = lb->next;
		lb_free_one(lb);
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

struct lbuf *
#if defined(BCMDBG_MEM) || defined(BCMDBG_MEMFAIL)
lb_clone(struct lbuf *lb, int offset, int len, const char *file, int line)
#else
lb_clone(struct lbuf *lb, int offset, int len)
#endif
{
	struct lbuf *clone;
	struct lbuf *orig;

	ASSERT(offset >= 0 && len >= 0);

	if (offset < 0 || len < 0)
	        return (NULL);

#if defined(BCMDBG_MEM) || defined(BCMDBG_MEMFAIL)
	if (!file)
		file = "unknown";

	clone = (struct lbuf*)hndrte_malloc_align(sizeof(struct lbuf_clone), 0, file, line);

	if (clone == NULL) {
		printf("lb_clone: size (%ld); alloc failed; file %s; line %d\n",
		       LBUFSZ, file, line);
		return (NULL);
	}
#else
	if ((clone = (struct lbuf*)hndrte_malloc_align(sizeof(struct lbuf_clone), 0)) == NULL) {
		LBUF_MSG(("lb_clone: size (%ld); alloc failed;\n", LBUFSZ));
		return (NULL);
	}
#endif /* BCMDBG_MEM || BCMDBG_MEMFAIL */

	/* find the actual non-clone original lbuf */
	if (lb_isclone(lb))
		orig = ((struct lbuf_clone*)lb)->orig;
	else
		orig = lb;

	ASSERT(orig->refcnt < 255);
	orig->refcnt++;

	ASSERT(ISALIGNED((uintptr)clone, sizeof(int)));
	bzero((char*)clone, sizeof(struct lbuf_clone));

	/* clone's data extent is a subset of the current data of lb */
	ASSERT(offset + len <= lb->len);
	clone->head = lb->data + offset;
	clone->end = clone->head + len;
	clone->data = clone->head;
	clone->len = len;
	clone->flags |= LBF_CLONE;

	((struct lbuf_clone*)clone)->orig = orig;

	return (clone);
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

#ifdef BCMDBG
void
lb_dump(void)
{
	uint i;
	LBUF_MSG(("allocfail %d\n", lbbuf_allocfail));
	for (i = 0; i < LBBUF_BIN_SIZE; i++) {
		LBUF_MSG(("bin[%d] %d ", i, lbbuf_hist[i]));
	}
	LBUF_MSG(("\n"));
}
#endif	/* BCMDBG */
