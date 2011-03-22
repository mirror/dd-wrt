/*
 * HND RTE packet buffer definitions.
 *
 * Copyright (C) 2009, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: hndrte_lbuf.h,v 13.35 2009/01/28 19:56:24 Exp $
 */

#ifndef _hndrte_lbuf_h_
#define	_hndrte_lbuf_h_

#include <bcmdefs.h>

struct lbuf {
	struct lbuf	*next;		/* next lbuf in a chain of lbufs forming one packet */
	struct lbuf	*link;		/* first lbuf of next packet in a list of packets */
	uchar		*head;		/* fixed start of buffer */
	uchar		*end;		/* fixed end of buffer */
	uchar		*data;		/* variable start of data */
	uint16		len;		/* nbytes of data */
	uint16		flags;		/* private flags; don't touch */
	uint16		dmapad;		/* padding to be added for tx dma */
	uint8		dataOff;	/* offset to beginning of data in 4-byte words */
	uint8		refcnt;		/* external references to this lbuf */
	uint32		pkttag[(OSL_PKTTAG_SZ + 3) / 4];  /* 4-byte-aligned packet tag area */
};

#define	LBUFSZ		sizeof(struct lbuf)

/* Total maximum packet buffer size including lbuf header */
#define MAXPKTBUFSZ	1920		/* enough to fit a 1500 MTU plus overhead */

/* private flags - don't reference directly */
#define	LBF_PRI		0x0007		/* priority (low 3 bits of flags) */
#define LBF_SUM_NEEDED	0x0008
#define LBF_SUM_GOOD	0x0010
#define LBF_MSGTRACE	0x0020
#define LBF_CLONE	0x0040
#define LBF_PTBLK	0x0100		/* came from fixed block in a partition, not main malloc */

#define	LBP(lb)		((struct lbuf *)(lb))

/* lbuf clone structure
 * if lbuf->flags LBF_CLONE bit is set, the lbuf can be cast to an lbuf_clone
 */
struct lbuf_clone {
	struct lbuf	lbuf;
	struct lbuf	*orig;	/* original non-clone lbuf providing the buffer space */
};

/* prototypes */
extern void lb_init(void);
#if defined(BCMDBG_MEM) || defined(BCMDBG_MEMFAIL)
extern struct lbuf *lb_alloc(uint size, const char *file, int line);
extern struct lbuf *lb_clone(struct lbuf *lb, int offset, int len, const char *file, int line);
#else
extern struct lbuf *lb_alloc(uint size);
extern struct lbuf *lb_clone(struct lbuf *lb, int offset, int len);
#endif /* BCMDBG_MEM || BCMDBG_MEMFAIL */

extern struct lbuf *lb_dup(struct lbuf *lb);
extern void lb_free(struct lbuf *lb);
extern bool lb_sane(struct lbuf *lb);
#ifdef BCMDBG
extern void lb_dump(void);
#endif

static INLINE uchar *
lb_push(struct lbuf *lb, uint len)
{
	ASSERT(lb_sane(lb));
	ASSERT((lb->data - len) >= lb->head);
	lb->data -= len;
	lb->len += len;
	return (lb->data);
}

static INLINE uchar *
lb_pull(struct lbuf *lb, uint len)
{
	ASSERT(lb_sane(lb));
	ASSERT(len <= lb->len);
	lb->data += len;
	lb->len -= len;
	return (lb->data);
}

static INLINE void
lb_setlen(struct lbuf *lb, uint len)
{
	ASSERT(lb_sane(lb));
	ASSERT(lb->data + len <= lb->end);
	lb->len = len;
}

static INLINE uint
lb_pri(struct lbuf *lb)
{
	ASSERT(lb_sane(lb));
	return (lb->flags & LBF_PRI);
}

static INLINE void
lb_setpri(struct lbuf *lb, uint pri)
{
	ASSERT(lb_sane(lb));
	ASSERT((pri & LBF_PRI) == pri);
	lb->flags = (lb->flags & ~LBF_PRI) | (pri & LBF_PRI);
}

static INLINE bool
lb_sumneeded(struct lbuf *lb)
{
	ASSERT(lb_sane(lb));
	return ((lb->flags & LBF_SUM_NEEDED) != 0);
}

static INLINE void
lb_setsumneeded(struct lbuf *lb, bool summed)
{
	ASSERT(lb_sane(lb));
	if (summed)
		lb->flags |= LBF_SUM_NEEDED;
	else
		lb->flags &= ~LBF_SUM_NEEDED;
}

static INLINE bool
lb_sumgood(struct lbuf *lb)
{
	ASSERT(lb_sane(lb));
	return ((lb->flags & LBF_SUM_GOOD) != 0);
}

static INLINE void
lb_setsumgood(struct lbuf *lb, bool summed)
{
	ASSERT(lb_sane(lb));
	if (summed)
		lb->flags |= LBF_SUM_GOOD;
	else
		lb->flags &= ~LBF_SUM_GOOD;
}

static INLINE bool
lb_msgtrace(struct lbuf *lb)
{
	ASSERT(lb_sane(lb));
	return ((lb->flags & LBF_MSGTRACE) != 0);
}

static INLINE void
lb_setmsgtrace(struct lbuf *lb, bool set)
{
	ASSERT(lb_sane(lb));
	if (set)
		lb->flags |= LBF_MSGTRACE;
	else
		lb->flags &= ~LBF_MSGTRACE;
}

/* get Data Offset */
static INLINE uint8
lb_dataoff(struct lbuf *lb)
{
	ASSERT(lb_sane(lb));
	return (lb->dataOff);
}

/* set Data Offset */
static INLINE void
lb_setdataoff(struct lbuf *lb, uint8 dataOff)
{
	ASSERT(lb_sane(lb));
	lb->dataOff = dataOff;
}

static INLINE int
lb_isclone(struct lbuf *lb)
{
	ASSERT(lb_sane(lb));
	return ((lb->flags & LBF_CLONE) != 0);
}

static INLINE int
lb_isptblk(struct lbuf *lb)
{
	ASSERT(lb_sane(lb));
	return ((lb->flags & LBF_PTBLK) != 0);
}

#endif	/* _hndrte_lbuf_h_ */
