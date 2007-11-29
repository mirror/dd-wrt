/*
 * CFE boot loader OS Abstraction Layer.
 *
 * Copyright 2007, Broadcom Corporation
 * All Rights Reserved.                
 *                                     
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;   
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior      
 * written permission of Broadcom Corporation.                            
 *
 * $Id$
 */

#ifndef _cfe_osl_h_
#define _cfe_osl_h_

#include <lib_types.h>
#include <lib_string.h>
#include <lib_printf.h>
#include <lib_malloc.h>
#include <cpu_config.h>
#include <cfe_timer.h>
#include <cfe_iocb.h>
#include <cfe_devfuncs.h>
#include <addrspace.h>

#include <typedefs.h>

/* pick up osl required snprintf/vsnprintf */
#include <bcmstdlib.h>

/* dump string */
extern int (*xprinthook)(const char *str);
#define puts(str) do { if (xprinthook) xprinthook(str); } while (0)

/* assert and panic */
#ifdef BCMDBG_ASSERT
#define ASSERT(exp) \
	do { if (!(exp)) osl_assert(#exp, __FILE__, __LINE__); } while (0)
extern void osl_assert(char *exp, char *file, int line);
#else /* BCMDBG_ASSERT */
#define	ASSERT(exp)		do {} while (0)
#endif /* BCMDBG_ASSERT */

/* PCMCIA attribute space access macros */
#define	OSL_PCMCIA_READ_ATTR(osh, offset, buf, size) \
	bzero(buf, size)
#define	OSL_PCMCIA_WRITE_ATTR(osh, offset, buf, size) \
	do {} while (0)

/* PCI configuration space access macros */
#define	OSL_PCI_READ_CONFIG(loc, offset, size) \
	(offset == 8 ? 0 : 0xffffffff)
#define	OSL_PCI_WRITE_CONFIG(loc, offset, size, val) \
	do {} while (0)

/* PCI device bus # and slot # */
#define OSL_PCI_BUS(osh)	(0)
#define OSL_PCI_SLOT(osh)	(0)

/* register access macros */
#define wreg32(r, v)		(*(volatile uint32*)(r) = (uint32)(v))
#define rreg32(r)		(*(volatile uint32*)(r))
#ifdef IL_BIGENDIAN
#define wreg16(r, v)		(*(volatile uint16*)((ulong)(r)^2) = (uint16)(v))
#define rreg16(r)		(*(volatile uint16*)((ulong)(r)^2))
#define wreg8(r, v)		(*(volatile uint8*)((ulong)(r)^3) = (uint8)(v))
#define rreg8(r)		(*(volatile uint8*)((ulong)(r)^3))
#else
#define wreg16(r, v)		(*(volatile uint16*)(r) = (uint16)(v))
#define rreg16(r)		(*(volatile uint16*)(r))
#define wreg8(r, v)		(*(volatile uint8*)(r) = (uint8)(v))
#define rreg8(r)		(*(volatile uint8*)(r))
#endif
#define R_REG(osh, r) ({ \
	__typeof(*(r)) __osl_v; \
	switch (sizeof(*(r))) { \
	case sizeof(uint8):	__osl_v = rreg8((r)); break; \
	case sizeof(uint16):	__osl_v = rreg16((r)); break; \
	case sizeof(uint32):	__osl_v = rreg32((r)); break; \
	} \
	__osl_v; \
})
#define W_REG(osh, r, v) do { \
	switch (sizeof(*(r))) { \
	case sizeof(uint8):	wreg8((r), (v)); break; \
	case sizeof(uint16):	wreg16((r), (v)); break; \
	case sizeof(uint32):	wreg32((r), (v)); break; \
	} \
} while (0)
#define	AND_REG(osh, r, v)		W_REG(osh, (r), R_REG(osh, r) & (v))
#define	OR_REG(osh, r, v)		W_REG(osh, (r), R_REG(osh, r) | (v))

/* bcopy, bcmp, and bzero */
#define bcmp(b1, b2, len)	lib_memcmp((b1), (b2), (len))

struct osl_info {
	void *pdev;
	pktfree_cb_fn_t tx_fn;
	void *tx_ctx;
};

extern osl_t *osl_attach(void *pdev);
extern void osl_detach(osl_t *osh);

#define PKTFREESETCB(osh, _tx_fn, _tx_ctx) \
	do { \
	   osh->tx_fn = _tx_fn; \
	   osh->tx_ctx = _tx_ctx; \
	} while (0)

/* general purpose memory allocation */
#define	MALLOC(osh, size)	KMALLOC((size), 0)
#define	MFREE(osh, addr, size)	KFREE((addr))
#define	MALLOCED(osh)		(0)
#define	MALLOC_DUMP(osh, b)
#define	MALLOC_FAILED(osh)	(0)

/* uncached virtual address */
#define	OSL_UNCACHED(va)	((void*)UNCADDR((ulong)(va)))

/* host/bus architecture-specific address byte swap */
#define BUS_SWAP32(v)		(v)

/* get processor cycle count */
#define OSL_GETCYCLES(x)	((x) = 0)

/* microsecond delay */
#define	OSL_DELAY(usec)		cfe_usleep((cfe_cpu_speed/CPUCFG_CYCLESPERCPUTICK/1000000*(usec)))

#define OSL_ERROR(bcmerror)	osl_error(bcmerror)

/* map/unmap physical to virtual I/O */
#define	REG_MAP(pa, size)	((void*)UNCADDR((ulong)(pa)))
#define	REG_UNMAP(va)		do {} while (0)

/* dereference an address that may cause a bus exception */
#define	BUSPROBE(val, addr)	osl_busprobe(&(val), (uint32)(addr))
extern int osl_busprobe(uint32 *val, uint32 addr);

/* allocate/free shared (dma-able) consistent (uncached) memory */
#define	DMA_CONSISTENT_ALIGN	4096		/* 4k alignment */
#define	DMA_ALLOC_CONSISTENT(osh, size, pap, dmah) \
	osl_dma_alloc_consistent((size), (pap))
#define	DMA_FREE_CONSISTENT(osh, va, size, pa, dmah) \
	osl_dma_free_consistent((void*)(va))
extern void *osl_dma_alloc_consistent(uint size, ulong *pap);
extern void osl_dma_free_consistent(void *va);

/* map/unmap direction */
#define	DMA_TX			1	/* TX direction for DMA */
#define	DMA_RX			2	/* RX direction for DMA */

/* map/unmap shared (dma-able) memory */
#define	DMA_MAP(osh, va, size, direction, lb, dmah) ({ \
	cfe_flushcache(CFE_CACHE_FLUSH_D); \
	PHYSADDR((ulong)(va)); \
})
#define	DMA_UNMAP(osh, pa, size, direction, p, dmah) \
	do {} while (0)

/* API for DMA addressing capability */
#define OSL_DMADDRWIDTH(osh, addrwidth) do {} while (0)

/* shared (dma-able) memory access macros */
#define	R_SM(r)			*(r)
#define	W_SM(r, v)		(*(r) = (v))
#define	BZERO_SM(r, len)	lib_memset((r), '\0', (len))

/* generic packet structure */
#define LBUFSZ		4096		/* Size of Lbuf - 4k */
#define LBDATASZ	(LBUFSZ - sizeof(struct lbuf))
struct lbuf {
	struct lbuf	*next;		/* pointer to next lbuf if in a chain */
	struct lbuf	*link;		/* pointer to next lbuf if in a list */
	uchar		*head;		/* start of buffer */
	uchar		*end;		/* end of buffer */
	uchar		*data;		/* start of data */
	uchar		*tail;		/* end of data */
	uint		len;		/* nbytes of data */
	uchar		pkttag[OSL_PKTTAG_SZ]; /* pkttag area */
};

#define	PKTBUFSZ	2048	/* largest reasonable packet buffer, driver uses for ethernet MTU */

/* packet primitives */
#define	PKTGET(osh, len, send)		((void*)osl_pktget((len)))
#define	PKTFREE(osh, lb, send)		osl_pktfree((osh), (struct lbuf*)(lb), (send))
#define	PKTDATA(osh, lb)		(((struct lbuf*)(lb))->data)
#define	PKTLEN(osh, lb)			(((struct lbuf*)(lb))->len)
#define PKTHEADROOM(osh, lb)		(PKTDATA(osh, lb)-(((struct lbuf*)(lb))->head))
#define PKTTAILROOM(osh, lb)		((((struct lbuf*)(lb))->end)-(((struct lbuf*)(lb))->tail))
#define	PKTNEXT(osh, lb)		(((struct lbuf*)(lb))->next)
#define	PKTSETNEXT(osh, lb, x)		(((struct lbuf*)(lb))->next = (struct lbuf*)(x))
#define	PKTSETLEN(osh, lb, len)		osl_pktsetlen((struct lbuf*)(lb), (len))
#define	PKTPUSH(osh, lb, bytes)		osl_pktpush((struct lbuf*)(lb), (bytes))
#define	PKTPULL(osh, lb, bytes)		osl_pktpull((struct lbuf*)(lb), (bytes))
#define	PKTDUP(osh, lb)			osl_pktdup((struct lbuf*)(lb))
#define	PKTTAG(lb)			(((void *) ((struct lbuf *)(lb))->pkttag))
#define	PKTLINK(lb)			(((struct lbuf*)(lb))->link)
#define	PKTSETLINK(lb, x)		(((struct lbuf*)(lb))->link = (struct lbuf*)(x))
#define	PKTPRIO(lb)			(0)
#define	PKTSETPRIO(lb, x)		do {} while (0)
#define	PKTFRMNATIVE(buffer, lb)	osl_pkt_frmnative((buffer), (struct lbuf *)(lb))
#define	PKTTONATIVE(lb, buffer)		osl_pkt_tonative((lb), (buffer))
#define PKTSHARED(lb)                   (1)
#define PKTALLOCED(osh)			(0)
#define PKTLIST_DUMP(osh, buf)

extern void osl_pkt_frmnative(iocb_buffer_t *buffer, struct lbuf *lb);
extern void osl_pkt_tonative(struct lbuf* lb, iocb_buffer_t *buffer);
extern struct lbuf *osl_pktget(uint len);
extern void osl_pktfree(osl_t *osh, struct lbuf *lb, bool send);
extern void osl_pktsetlen(struct lbuf *lb, uint len);
extern uchar *osl_pktpush(struct lbuf *lb, uint bytes);
extern uchar *osl_pktpull(struct lbuf *lb, uint bytes);
extern struct lbuf *osl_pktdup(struct lbuf *lb);
extern int osl_error(int bcmerror);

#endif	/* _cfe_osl_h_ */
