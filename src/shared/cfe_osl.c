/*
 * CFE OS Independent Layer
 *
 * Copyright 2005, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id$
 */

#include <typedefs.h>
#include <osl.h>

struct lbuf *
osl_pktget(uint len)
{
	uchar *buf;
	struct lbuf *lb;

	ASSERT(len <= LBDATASZ);

	if (!(buf = KMALLOC(LBUFSZ, 0)))
		return NULL;

	lb = (struct lbuf *) &buf[LBDATASZ];
	bzero(lb, sizeof(struct lbuf));
	lb->head = lb->data = buf;
	lb->end = buf + len;
	lb->len = len;
	lb->tail = lb->data + len;

	return lb;
}

void
osl_pktfree(struct lbuf *lb)
{
	struct lbuf *next;

	for (; lb; lb = next) {
		ASSERT(!lb->link);
		next = lb->next;
		KFREE((void *) KERNADDR(PHYSADDR((ulong) lb->head)));
	}
}

struct lbuf *
osl_pktdup(struct lbuf *lb)
{
	struct lbuf *dup;

	if (!(dup = osl_pktget(lb->len)))
		return NULL;

	bcopy(lb->data, dup->data, lb->len);
	ASSERT(!lb->link);

	return dup;
}

void
osl_pktsetlen(struct lbuf *lb, uint len)
{
	ASSERT((lb->data + len) <= lb->end);

	lb->len = len;
	lb->tail = lb->data + len;
}

uchar *
osl_pktpush(struct lbuf *lb, uint bytes)
{
	ASSERT((lb->data - bytes) >= lb->head);

	lb->data -= bytes;
	lb->len += bytes;

	return lb->data;
}

uchar *
osl_pktpull(struct lbuf *lb, uint bytes)
{
	ASSERT((lb->data + bytes) <= lb->end);
	ASSERT(lb->len >= bytes);

	lb->data += bytes;
	lb->len -= bytes;

	return lb->data;
}

void *
osl_dma_alloc_consistent(uint size, ulong *pap)
{
	void *buf;

	if (!(buf = KMALLOC(size, DMA_CONSISTENT_ALIGN)))
		return NULL;

	*((ulong *) pap) = PHYSADDR((ulong) buf);

	cfe_flushcache(CFE_CACHE_FLUSH_D);

	return (void *) UNCADDR((ulong) buf);
}

void
osl_dma_free_consistent(void *va)
{
	KFREE((void *) KERNADDR(PHYSADDR((ulong) va)));
}


int
osl_busprobe(uint32 *val, uint32 addr)
{
	*val = R_REG((volatile uint32 *) addr);

	return 0;
} 

/* translate bcmerros*/
int 
osl_error(int bcmerror)
{
	if (bcmerror)
		return -1;
	else 
		return 0;
}
