/*
 * Mac OS X Independent Layer
 *
 * Copyright (C) 2015, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * Copyright (c) 2002 Apple Computer, Inc.  All rights reserved.
 *
 * $Id: macosx_osl.h 520438 2014-12-11 20:32:06Z $
 */

#ifndef _macosx_osl_h_
#define _macosx_osl_h_

#include <typedefs.h>
#include <sys/cdefs.h>
__BEGIN_DECLS
#include <sys/kpi_mbuf.h>
__END_DECLS
#include <IOKit/IOLib.h>
#ifdef  __cplusplus
#include <IOKit/pci/IOPCIDevice.h>
#else
typedef void IOPCIDevice;
#endif

#include <libkern/OSDebug.h>

#ifdef USE_KLOG
#include "KernelDebugging.h"
/* KernelDebugging.h defines ASSERT, but we want the panic() version */
#ifdef ASSERT
#undef ASSERT
#endif
#endif /* USE_KLOG */

#define DECLSPEC_ALIGN(x) __declspec(align(x))

#if ENABLE_MBUF_PANICPARANOIDCHECK
#define MBUF_PANICPARANOIDCHECK(_m, _f, _l)				\
do {									\
    mbuf_type_t type = mbuf_type((const mbuf_t)(_m));			\
    if ((type == MBUF_TYPE_FREE) || (((uint32_t)type) > 0x20))		\
	osl_panic("%s:%5d, mbuf paranoid check, freed mbuf[%p] type[0x%4x]", _f, _l, _m, type);	\
} while (0);
#else
#define MBUF_PANICPARANOIDCHECK(_m, _f, _l)
#endif


/* assert and panic */
#define	ASSERT(exp)		do {} while (0)

#define OSL_LOG osl_log

/* verify some compile settings */
#ifdef __BIG_ENDIAN__
#ifndef IL_BIGENDIAN
#error "IL_BIGENDIAN was not defined for a big-endian compile"
#endif
#else
#ifdef IL_BIGENDIAN
#error "IL_BIGENDIAN was defined for a little-endian compile"
#endif
#endif /* __BIG_ENDIAN__ */

#ifndef DMA
#error "DMA not defined"
#endif
#ifndef MACOSX
#error "MACOSX not defined"
#endif

__BEGIN_DECLS
extern int osl_printf(const char *fmt, ...);
__END_DECLS

#ifdef USE_KLOG
#define printf(ARGS...)		KernelDebugLogInternal(2, 'pciW' , ## ARGS)
#else
#define printf			osl_printf
#endif /* USE_KLOG */

/* microsecond delay */
#define	OSL_DELAY(us)		IODelay(us)

#define OSL_GETCYCLES(c)	(void)(c = 0)

/* PCMCIA attribute space access macros */
#define	OSL_PCMCIA_READ_ATTR(osh, offset, buf, size) 	\
	({ \
	 BCM_REFERENCE(osh); \
	 BCM_REFERENCE(buf); \
	 ASSERT(0); \
	 })
#define	OSL_PCMCIA_WRITE_ATTR(osh, offset, buf, size) 	\
	({ \
	 BCM_REFERENCE(osh); \
	 BCM_REFERENCE(buf); \
	 ASSERT(0); \
	 })

/* PCI configuration space access macros */
#define	OSL_PCI_READ_CONFIG(osh, offset, size)		\
		osl_pci_read_config((osh), offset, size)
#define	OSL_PCI_WRITE_CONFIG(osh, offset, size, val)	\
		osl_pci_write_config((osh), offset, size, val)

/* PCI device bus # and slot # */
#define OSL_PCI_BUS(osh)	({BCM_REFERENCE(osh); 0;})
#define OSL_PCI_SLOT(osh)	({BCM_REFERENCE(osh); 0;})
#define OSL_PCIE_DOMAIN(osh)	({BCM_REFERENCE(osh); 0;})
#define OSL_PCIE_BUS(osh)	({BCM_REFERENCE(osh); 0;})

/* host/bus architecture specific swap */
#define BUS_SWAP32(v)		htol32(v)

/* uncached/cached virtual address */
#define	OSL_UNCACHED(va)	(va)
#define	OSL_CACHED(va)		(va)

#define OSL_PREF_RANGE_LD(va, sz)	BCM_REFERENCE(va)
#define OSL_PREF_RANGE_ST(va, sz)	BCM_REFERENCE(va)

/* map/unmap virtual to physical */
#define REG_MAP(pa, size)	((void*)(uintptr)(pa))
#define REG_UNMAP(pa)		({BCM_REFERENCE(pa); (void)0;})
#define OSL_CACHE_FLUSH(va, len)	{BCM_REFERENCE((va)); BCM_REFERENCE((len));}
#define OSL_CACHE_INV(va, len)		BCM_REFERENCE(va)
#define OSL_PREFETCH(ptr)		BCM_REFERENCE(ptr)

#if defined(BCM_BACKPLANE_TIMEOUT) && defined(BCMDBG)
/* register access macros when  BCM_BACKPLANE_TIMEOUT is enabled */
#define R_REG(osh, r)	\
		read_bpt_reg((osh), (r), (sizeof(*r)))
#else
/* register access macros */
#define	R_REG(osh, r) \
	({ \
	 BCM_REFERENCE(osh); \
	 (uint32)((sizeof *(r) == sizeof(uint32)) ? OSReadLittleInt32((uint32*)(r), 0) : \
	 ((sizeof *(r) == sizeof(uint16)) ? OSReadLittleInt16((uint16*)(r), 0) : \
	 *(volatile uint8*)(r))); \
	 })
#endif /* #if defined(BCM_BACKPLANE_TIMEOUT) && defined(BCMDBG) */

#define	W_REG(osh, r, v)	do {								\
				BCM_REFERENCE(osh);						\
				if (sizeof *(r) == sizeof (uint32))				\
					OSWriteLittleInt32((uint32*)(r), 0, (v));		\
				else if (sizeof *(r) == sizeof (uint16))			\
					OSWriteLittleInt16((uint16*)(r), 0, (uint16)(v));	\
				else								\
					*(volatile uint8*)(r) = (uint8)(v);			\
				OSSynchronizeIO();						\
			} while (0)

#define	AND_REG(osh, r, v)	W_REG(osh, (r), R_REG(osh, r) & (v))
#define	OR_REG(osh, r, v)	W_REG(osh, (r), R_REG(osh, r) | (v))


/* shared memory access macros */
#define	R_SM(a)			(*(a))
#define	W_SM(a, v)		(*(a) = (v))
#define	BZERO_SM(a, len)	bzero(a, len)

#ifdef MALLOC
#undef MALLOC
#endif
#ifdef MFREE
#undef MFREE
#endif


#define	MALLOC(osh, size)		osl_malloc(osh, (size))
#define	MALLOCZ(osh, size)		osl_mallocz(osh, (size))
#define	MFREE(osh, addr, size)		osl_mfree(osh, addr, (size))
#define MALLOCED(osh)			osl_malloced(osh)
/* JS:   need to define/implement these debug features */
#define MALLOC_DUMP(osh, b)		BCM_REFERENCE(osh)

#define MALLOC_FAILED(osh)		osl_malloc_failed(osh)

#define BUSPROBE(val, addr)		val = R_REG(addr)

/* allocate/free shared (dma-able) consistent memory */
#define DMA_CONSISTENT_ALIGN		PAGE_SIZE
#define	DMA_ALLOC_CONSISTENT(osh, size, align, tot, pa, dmah)	\
		osl_dma_alloc_consistent(osh, size, (align), (tot), pa, (void **)dmah)
#define	DMA_FREE_CONSISTENT(osh, va, size, pa, dmah)	\
		osl_dma_free_consistent(osh, va, size, dmah)

#define	DMA_ALLOC_CONSISTENT_FORCE32(osh, size, align, tot, pa, dmah)	\
		osl_dma_alloc_consistent_force32(osh, size, (align), (tot), pa, (void **)dmah)
#define	DMA_FREE_CONSISTENT_FORCE32(osh, va, size, pa, dmah)	\
		osl_dma_free_consistent(osh, va, size, dmah)

/* map/unmap direction */
#define	DMA_TX	1
#define	DMA_RX	0

/* map/unmap shared (dma-able) memory */
#define	DMA_MAP(osh, va, size, direction, p, dmah) \
		osl_dma_map(osh, va, size, direction, (mbuf_t)(p), (dmah))
#define	DMA_UNMAP(osh, pa, size, direction, p, dmah) \
	({ \
	 BCM_REFERENCE(osh); \
	 BCM_REFERENCE(pa); \
	 BCM_REFERENCE(size); \
	 BCM_REFERENCE(direction); \
	 BCM_REFERENCE(p); \
	 BCM_REFERENCE(dmah); \
	 })

#define OSL_DMADDRWIDTH(osh, width)	osl_dma_addrwidth(osh, width)

#define OSH_NULL   NULL

/* the largest reasonable packet buffer driver uses for ethernet MTU in bytes */
#define	PKTBUFSZ	2044

/* packet primitives */
#define	PKTGET(osh, len, send)		osl_pktget(osh, len)
#define	PKTFREE(osh, m, send)		osl_pktfree(osh, (mbuf_t)(m), (send))
#define	PKTDUP(osh, m)			osl_pktdup(osh, (mbuf_t)(m))
#define	PKTDATA(osh, m)			((uint8*)(osl_pktdata(osh, (mbuf_t)(m))))
#define	PKTLEN(osh, m)			(int)(osl_pktlen(osh, (mbuf_t)(m)))
#define	PKTSETLEN(osh, m, len)		osl_pktsetlen(osh, (mbuf_t)(m), len)
#define	PKTHEADROOM(osh, m)		((uint32)osl_pktheadroom(osh, (mbuf_t)(m)))
#define	PKTTAILROOM(osh, m)		((uint32)osl_pkttailroom(osh, (mbuf_t)(m)))
#define	PKTPUSH(osh, m, nbytes)		osl_pktpush(osh, (mbuf_t)(m), nbytes)
#define	PKTPULL(osh, m, nbytes)		osl_pktpull(osh, (mbuf_t)(m), nbytes)
#define	PKTNEXT(osh, m)			osl_pktnext(osh, (mbuf_t)(m))
#define	PKTSETNEXT(osh, m, n)		osl_pktsetnext(osh, (mbuf_t)(m), (mbuf_t)(n))
#define	PKTLINK(m)			osl_pktlink((mbuf_t)(m))
#define	PKTSETLINK(m, n)		osl_pktsetlink((mbuf_t)(m), (mbuf_t)(n))
#define	PKTTAG(m)			osl_pkt_gettag((mbuf_t)(m))
#define PKTFRMNATIVE(osh, m)		osl_pkt_frmnative(osh, m)
#define PKTTONATIVE(osh, m)		osl_pkt_tonative(osh, (mbuf_t)(m))
#define	PKTPRIO(m)			osl_getprio((m))
#define	PKTSETPRIO(m, n)		osl_setprio((m), (n))
#define	PKTSHARED(m)			osl_pktshared(osh, (mbuf_t)(m))
#define PKTALLOCED(osh)			({BCM_REFERENCE(osh); (0);})
#define PKTSUMNEEDED(m)			({BCM_REFERENCE(m); (0);})
#define PKTSETSUMGOOD(m, x)		({BCM_REFERENCE(m); (0);})
#define PKTSETPOOL(osh, m, x, y)	BCM_REFERENCE(osh)
#define PKTPOOL(osh, m)			({BCM_REFERENCE(osh); FALSE;})
#define PKTFREELIST(m)          PKTLINK(m)
#define PKTSETFREELIST(m, x)    PKTSETLINK((m), (x))
#define PKTPTR(m)               (m)
#define PKTID(m)                ({BCM_REFERENCE(m); 0;})
#define PKTSETID(m, id)         ({BCM_REFERENCE(m); BCM_REFERENCE(id);})
#define PKTLIST_DUMP(osh, buf)		BCM_REFERENCE(osh)
#define PKTSHRINK(osh, m)		({BCM_REFERENCE(osh); (m);})
#define PKTORPHAN(pkt)			((void)pkt)

#define OSL_ERROR(e)			osl_error(e)

/* get system up time in miliseconds */
#define OSL_SYSUPTIME()			osl_sysuptime()
#define OSL_SYSUPTIME_US()		osl_sysuptime_us()
#define OSL_SYSUPTIME_US64()		osl_sysuptime_us64()

#ifdef bcmp
#undef bcmp
#endif
#define bcmp(b1, b2, len)		memcmp((b1), (b2), (size_t)(len))

#define OSL_RAND()		osl_rand()

#if !defined(ENABLE_DBGMEM) /* over-rideable from project file */
#define ENABLE_DBGMEM                          0
#endif /* ENABLE_DBGMEM */

#if ENABLE_DBGMEM

#define DBGMEM_INFO_ALLOCSIZE_MIN              (1 * (1024*1024))
#define DBGMEM_INFO_ALLOCSIZE_MAX              (64 * (1024*1024))

#define DBGMEM_INFO_TYPE_FREE                  0
#define DBGMEM_INFO_TYPE_MALLOC                1
#define DBGMEM_INFO_TYPE_MBUFFREE              4
#define DBGMEM_INFO_TYPE_MBUFALLOC             5
#define DBGMEM_INFO_TYPE_DMAFREE               8
#define DBGMEM_INFO_TYPE_DMAALLOC              9

#define DBGMEM_INFO_TRACEFLAG_FREE             (1 <<  (DBGMEM_INFO_TYPE_FREE))
#define DBGMEM_INFO_TRACEFLAG_MALLOC           (1 <<  (DBGMEM_INFO_TYPE_MALLOC))
#define DBGMEM_INFO_TRACEFLAG_MBUFFREE         (1 <<  (DBGMEM_INFO_TYPE_MBUFFREE))
#define DBGMEM_INFO_TRACEFLAG_MBUFALLOC        (1 <<  (DBGMEM_INFO_TYPE_MBUFALLOC))
#define DBGMEM_INFO_TRACEFLAG_DMAFREE          (1 <<  (DBGMEM_INFO_TYPE_DMAFREE))
#define DBGMEM_INFO_TRACEFLAG_DMAALLOC         (1 <<  (DBGMEM_INFO_TYPE_DMAALLOC))


#define DBGMEM_INFO_ISENABLED()                (g_brcm_dbgmem_info.enable)
#define DBGMEM_INFO_HASELEMENTS()              (g_brcm_dbgmem_info.elements)
#define DBGMEM_INFO_ISTRACINGENABLED() (DBGMEM_INFO_ISENABLED() && DBGMEM_INFO_HASELEMENTS())
#define DBGMEM_INFO_GETELEMENT(_idx)   (DBGMEM_INFO_ISTRACINGENABLED() ? \
	((struct brcm_dbgmem_info_element_s *)&g_brcm_dbgmem_info.elements[(_idx)]) : NULL)
#define DBGMEM_INFO_GETIDX()                   (g_brcm_dbgmem_info.idx)
#define DBGMEM_INFO_GETMAX()                   (g_brcm_dbgmem_info.max)
#define DBGMEM_INFO_SETMAX(_value)             (g_brcm_dbgmem_info.max = (_value))

#define DBGMEM_INFO_INCIDX(_inc)               \
do {                                           \
    if (DBGMEM_INFO_ISTRACINGENABLED())        \
	g_brcm_dbgmem_info.idx = (g_brcm_dbgmem_info.idx + (_inc)) % g_brcm_dbgmem_info.max; \
} while (0);

#define DBGMEM_INFO_SETENABLE(_value)          (g_brcm_dbgmem_info.enable = (_value))
#define DBGMEM_INFO_GETENABLE()                g_brcm_dbgmem_info.enable
#define DBGMEM_INFO_SETTRACEFLAGS(_value)      (g_brcm_dbgmem_info.traceflags = (_value))
#define DBGMEM_INFO_GETTRACEFLAGS()            g_brcm_dbgmem_info.traceflags
#define DBGMEM_INFO_SETALLOCSIZE(_value)       (g_brcm_dbgmem_info.allocsize = (_value))
#define DBGMEM_INFO_GETALLOCSIZE()             g_brcm_dbgmem_info.allocsize
#define DBGMEM_INFO_SETALLOCED(_value)         (g_brcm_dbgmem_info.alloced = (_value))
#define DBGMEM_INFO_GETALLOCED()               g_brcm_dbgmem_info.alloced

#define DBGMEM_INFO_SETELEMENT(_elem, _type, _tag, _tagstr, _file, _line, _timestamp,       \
		_size, _ptr, _ptrdata, _physa, _arg0, _arg1, _arg2, _arg3)  \
do {                                           \
	if (_elem && DBGMEM_INFO_ISTRACINGENABLED() && \
		(DBGMEM_INFO_GETTRACEFLAGS() & (1 << (_type)))) { \
		_elem->type           = (_type);       \
		_elem->tag            = (_tag);        \
		_elem->tagstr         = (_tagstr);     \
		_elem->line           = (_line);       \
		_elem->file           = (const char *)(_file);  \
		_elem->timestamp      = (uint64_t)(_timestamp); \
		_elem->size           = (size_t)(_size);        \
		_elem->ptr            = (const void *)(_ptr);   \
		_elem->ptrdata        = (const void *)(_ptrdata); \
		_elem->physa          = (const void *)(_physa);   \
		_elem->arg0           = (const void *)(_arg0);    \
		_elem->arg1           = (const void *)(_arg1);    \
		_elem->arg2           = (const void *)(_arg2);    \
		_elem->arg3           = (const void *)(_arg3);    \
		_elem->btdepth        = OSBacktrace(&_elem->backtrace[0], 16); \
						\
		DBGMEM_INFO_INCIDX(1);          \
	}                                       \
} while (0);

#define DBGMEM_INFO_RECORDTRACE(_type, _tag, _tagstr, _file, _line, _timestamp,             \
			_size, _ptr, _ptrdata, _physa, _arg0, _arg1, _arg2, _arg3) \
do {                                           \
	/* Add to circular element array */        \
	struct brcm_dbgmem_info_element_s *elem =  \
	DBGMEM_INFO_GETELEMENT(DBGMEM_INFO_GETIDX()); \
					       \
	DBGMEM_INFO_SETELEMENT(elem, (_type), (_tag), (_tagstr), (_file), (_line), \
			(_timestamp), (_size), (_ptr), \
			(_ptrdata), (_physa),         \
			(_arg0), (_arg1), (_arg2), (_arg3)); \
} while (0);

struct brcm_dbgmem_info_element_s {
	uint        type;        /* Refer to DBGMEM_INFO_TYPE_XXXXX */
	uint64_t    tag;
	const char  *tagstr;

	uint        line;
	const char  *file;

	uint64_t    timestamp;
	size_t      size;
	const void  *ptr;

	const void  *ptrdata;
	const void  *physa;

	const void  *arg0;
	const void  *arg1;
	const void  *arg2;
	const void  *arg3;

	unsigned    btdepth;
	void        *backtrace[16];
};

struct g_brcm_dbgmem_info_s {
	int         enable;
	int         alloced;
	uint32_t    allocsize;

	uint32_t    traceflags;

	uint32_t    idx;
	uint32_t    max;

	struct brcm_dbgmem_info_element_s *elements;
};

extern struct g_brcm_dbgmem_info_s g_brcm_dbgmem_info;

#else /* !ENABLE_DBGMEM */

/* NULL macros if feature is not enabled */
#define DBGMEM_INFO_RECORDTRACE(_type, _tag, _tagstr, _file, _line, _timestamp, \
			_size, _ptr, _ptrdata, _physa, _arg0, _arg1, _arg2, _arg3)

#endif /* !ENABLE_DBGMEM */


__BEGIN_DECLS

/* OSL initialization */
extern osl_t*	osl_attach(IOPCIDevice *pcidev);
extern void	osl_detach(osl_t *osh);
extern errno_t	osl_alloc_private_mbufs(osl_t *osh);

extern void osl_pktfree_cb_set(osl_t *osh, pktfree_cb_fn_t tx_fn, void *tx_ctx);
#define PKTFREESETCB(osh, tx_fn, tx_ctx) osl_pktfree_cb_set(osh, tx_fn, tx_ctx)

/* PCI configuration space access */
extern uint32	osl_pci_read_config(osl_t *osh, uint offset, uint size);
extern void	osl_pci_write_config(osl_t *osh, uint offset, uint size, uint val);

/* memory allocation */

extern void*	osl_malloc(osl_t *osh, uint size);
extern void*	osl_mallocz(osl_t *osh, uint size);
extern void	osl_mfree(osl_t *osh, void *addr, uint size);
extern uint	osl_malloced(osl_t *osh);
extern uint	osl_malloc_failed(osl_t *osh);

/* map/unmap shared (dma-able) memory */
extern dmaaddr_t	osl_dma_map(osl_t *osh, void *va, int size, uint direction, mbuf_t mbuf,
                                    hnddma_seg_map_t *dmah);
extern void*	osl_dma_alloc_consistent(osl_t *osh, uint size, uint16 align, uint *tot,
	dmaaddr_t *pa, void **dmah);
extern void		osl_dma_free_consistent(osl_t *osh, void *va, uint size, void *dmah);
extern void*	osl_dma_alloc_consistent_force32(osl_t *osh, uint size, uint16 align, uint *tot,
	dmaaddr_t *pa, void **dmah);
extern void		osl_dma_addrwidth(osl_t *osh, uint width);

/* packet primitives */
extern mbuf_t	osl_pktget(osl_t *osh, uint len);
extern void	osl_pktfree(osl_t *osh, mbuf_t m, bool send);
extern mbuf_t	osl_pktdup(osl_t *osh, mbuf_t m);
extern uint8*	osl_pktpush(osl_t *osh, mbuf_t m, uint nbytes);
extern uint8*	osl_pktpull(osl_t *osh, mbuf_t m, uint nbytes);
extern void*	osl_pkt_gettag(mbuf_t m);
extern mbuf_t	osl_pkt_frmnative(osl_t *osh, mbuf_t m);
extern mbuf_t	osl_pkt_tonative(osl_t *osh, mbuf_t m);

extern void*	osl_pktdata(osl_t *osh, mbuf_t m);			/* mbuf_data */
extern size_t	osl_pktlen(osl_t *osh, mbuf_t m);			/* mbuf_len */
extern size_t	osl_pktsetlen(osl_t *osh, mbuf_t m, uint len );		/* mbuf_setlen */
extern size_t	osl_pktheadroom(osl_t *osh, mbuf_t m);			/* mbuf_leadingspace */
extern size_t	osl_pkttailroom(osl_t *osh, mbuf_t m);			/* mbuf_trailingspace */
extern mbuf_t	osl_pktnext(osl_t *osh, mbuf_t m);			/* mbuf_next */
extern errno_t	osl_pktsetnext(osl_t *osh, mbuf_t m, mbuf_t n );	/* mbuf_setnext */
extern mbuf_t	osl_pktlink(mbuf_t m);					/* mbuf_nextpkt */
extern errno_t	osl_pktsetlink(mbuf_t m, mbuf_t n );			/* mbuf_setnextpkt */
extern int	osl_pktshared(osl_t *osh, mbuf_t m);			/* mbuf_mclhasreference */

extern int osl_getprio(mbuf_t m);
extern void osl_setprio(mbuf_t m, int prio);

extern int osl_error(int bcmerror);
extern void osl_assert(const char *file, int line, const char *exp);
extern void osl_panic(const char *fmt, ...);
extern uint32 osl_sysuptime();
extern uint32 osl_sysuptime_us();
extern uint64 osl_sysuptime_us64();
extern void osl_log(const char * tag, const char *fmt, ...);
extern uint32 osl_rand(void);

#ifdef BCMDONGLEHOST
extern void* osl_spin_alloc_init();
extern void osl_spin_free(void *l);
extern void osl_spin_lock(void *l);
extern void osl_spin_unlock(void *l);
#endif /* BCMDONGLEHOST */

/* strrchr proto not in Mac OS X Kernel.framework strings.h */
char *strrchr(const char *s, int c);

extern uint32 g_assert_type;
__END_DECLS

#endif	/* _macosx_osl_h_ */
