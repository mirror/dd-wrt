/*
 * MIPS PMON boot loader OS Abstraction Layer.
 *
 * Copyright 2005, Broadcom Corporation      
 * All Rights Reserved.                      
 *                                           
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;         
 * the contents of this file may not be disclosed to third parties, copied      
 * or duplicated in any form, in whole or in part, without the prior            
 * written permission of Broadcom Corporation.                                  
 * $Id$
 */

#ifndef _pmon_osl_h_
#define _pmon_osl_h_

#include <typedefs.h>
#include <mips.h>
#include <string.h>
#include <utypes.h>

extern int printf(char *fmt,...);
extern int sprintf(char *dst,char *fmt,...);

#define	OSL_UNCACHED(va)	phy2k1(log2phy((va)))
#define	REG_MAP(pa, size)	phy2k1((pa))
#define	REG_UNMAP(va)		/* nop */

/* Common macros */

#define BUSPROBE(val, addr) ((val) = *(addr))

#define	ASSERT(exp)

#define	OSL_PCMCIA_READ_ATTR(osh, offset, buf, size) bzero(buf, size)
#define	OSL_PCMCIA_WRITE_ATTR(osh, offset, buf, size)

/* kludge */
#define	OSL_PCI_READ_CONFIG(loc, offset, size)	((offset == 8)? 0: 0xffffffff)
#define	OSL_PCI_WRITE_CONFIG(loc, offset, size, val)	ASSERT(0)

#define wreg32(r,v)	(*(volatile uint32 *)(r) = (v))
#define rreg32(r)	(*(volatile uint32 *)(r))
#ifdef IL_BIGENDIAN
#define wreg16(r,v)	(*(volatile uint16 *)((uint32)r^2) = (v))
#define rreg16(r)	(*(volatile uint16 *)((uint32)r^2))
#else
#define wreg16(r,v)	(*(volatile uint16 *)(r) = (v))
#define rreg16(r)	(*(volatile uint16 *)(r))
#endif

#include <memory.h>
#define	bcopy(src, dst, len)	memcpy(dst, src, len)
#define	bcmp(b1, b2, len)	memcmp(b1, b2, len)
#define	bzero(b, len)		memset(b, '\0', len)

/* register access macros */
#define	R_REG(r)	((sizeof *(r) == sizeof (uint32))? rreg32(r): rreg16(r))
#define	W_REG(r,v)	((sizeof *(r) == sizeof (uint32))? wreg32(r,(uint32)v): wreg16(r,(uint16)v))
#define	AND_REG(r, v)	W_REG((r), R_REG(r) & (v))
#define	OR_REG(r, v)	W_REG((r), R_REG(r) | (v))

#define	R_SM(r)			*(r)
#define	W_SM(r, v)		(*(r) = (v))
#define	BZERO_SM(r, len)	memset(r, '\0', len)

/* Host/Bus architecture specific swap. Noop for little endian systems, possible swap on big endian */
#define BUS_SWAP32(v)	(v)

#define	OSL_DELAY(usec)		delay_us(usec)
extern void delay_us(uint usec);

#define OSL_GETCYCLES(x) ((x) = 0)

#define osl_attach(pdev)	(pdev)
#define osl_detach(osh)

#define	MALLOC(osh, size)	malloc(size)
#define	MFREE(osh, addr, size)	free(addr)
#define	MALLOCED(osh)		(0)
#define	MALLOC_DUMP(osh, buf, sz)
#define	MALLOC_FAILED(osh)
extern void *malloc();
extern void free(void *addr);

#define	DMA_CONSISTENT_ALIGN	sizeof (int)
#define	DMA_ALLOC_CONSISTENT(osh, size, pap)	et_dma_alloc_consistent(osh, size, pap)
#define	DMA_FREE_CONSISTENT(osh, va, size, pa)
extern void* et_dma_alloc_consistent(void *osh, uint size, ulong *pap);
#define	DMA_TX 0
#define	DMA_RX 1

#define	DMA_MAP(osh, va, size, direction, p)	osl_dma_map(osh, (void*)va, size, direction)
#define	DMA_UNMAP(osh, pa, size, direction, p)	/* nop */
extern void* osl_dma_map(void *osh, void *va, uint size, uint direction);

struct lbuf {
	struct lbuf *next;	/* pointer to next lbuf on freelist */
	uchar *buf;		/* pointer to buffer */
	uint len;		/* nbytes of data */
};

/* the largest reasonable packet buffer driver uses for ethernet MTU in bytes */
#define	PKTBUFSZ	2048

/* packet primitives */
#define	PKTGET(drv, len, send)		et_pktget(drv, len, send)
#define	PKTFREE(drv, lb, send)		et_pktfree(drv, (struct lbuf*)lb, send)
#define	PKTDATA(drv, lb)		((uchar*)OSL_UNCACHED(((struct lbuf*)lb)->buf))
#define	PKTLEN(drv, lb)			((struct lbuf*)lb)->len
#define PKTHEADROOM(drv, lb)		(0)
#define PKTTAILROOM(drv, lb)		(0)
#define	PKTNEXT(drv, lb)		NULL
#define	PKTSETNEXT(lb, x)		ASSERT(0)
#define	PKTSETLEN(drv, lb, bytes)	((struct lbuf*)lb)->len = bytes
#define	PKTPUSH(drv, lb, bytes)		ASSERT(0)
#define	PKTPULL(drv, lb, bytes)		ASSERT(0)
#define	PKTDUP(drv, lb)			ASSERT(0)
#define	PKTLINK(lb)			((struct lbuf*)lb)->next
#define	PKTSETLINK(lb, x)		((struct lbuf*)lb)->next = (struct lbuf*)x
#define	PKTPRIO(lb)			(0)
#define	PKTSETPRIO(lb, x)		do {} while (0)
extern void *et_pktget(void *drv, uint len, bool send);
extern void et_pktfree(void *drv, struct lbuf *lb, bool send);

#endif	/* _pmon_osl_h_ */
