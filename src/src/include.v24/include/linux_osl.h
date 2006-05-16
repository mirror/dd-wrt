/*
 * Linux OS Independent Layer
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

#ifndef _linux_osl_h_
#define _linux_osl_h_

#include <typedefs.h>

/* use current 2.4.x calling conventions */
#include <linuxver.h>

/* assert and panic */
#ifdef __GNUC__
#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#if GCC_VERSION > 30100
#define	ASSERT(exp)		do {} while (0)
#else
/* ASSERT could causes segmentation fault on GCC3.1, use empty instead*/
#define	ASSERT(exp)		
#endif
#endif

/* microsecond delay */
#define	OSL_DELAY(usec)		osl_delay(usec)
extern void osl_delay(uint usec);

/* PCMCIA attribute space access macros */
#if defined(CONFIG_PCMCIA) || defined(CONFIG_PCMCIA_MODULE)
struct pcmcia_dev {
	dev_link_t link;	/* PCMCIA device pointer */
	dev_node_t node;	/* PCMCIA node structure */
	void *base;		/* Mapped attribute memory window */
	size_t size;		/* Size of window */
	void *drv;		/* Driver data */
};
#endif
#define	OSL_PCMCIA_READ_ATTR(osh, offset, buf, size) \
	osl_pcmcia_read_attr((osh), (offset), (buf), (size))
#define	OSL_PCMCIA_WRITE_ATTR(osh, offset, buf, size) \
	osl_pcmcia_write_attr((osh), (offset), (buf), (size))
extern void osl_pcmcia_read_attr(osl_t *osh, uint offset, void *buf, int size);
extern void osl_pcmcia_write_attr(osl_t *osh, uint offset, void *buf, int size);

/* PCI configuration space access macros */
#define	OSL_PCI_READ_CONFIG(osh, offset, size) \
	osl_pci_read_config((osh), (offset), (size))
#define	OSL_PCI_WRITE_CONFIG(osh, offset, size, val) \
	osl_pci_write_config((osh), (offset), (size), (val))
extern uint32 osl_pci_read_config(osl_t *osh, uint size, uint offset);
extern void osl_pci_write_config(osl_t *osh, uint offset, uint size, uint val);

/* PCI device bus # and slot # */
#define OSL_PCI_BUS(osh)	osl_pci_bus(osh)
#define OSL_PCI_SLOT(osh)	osl_pci_slot(osh)
extern uint osl_pci_bus(osl_t *osh);
extern uint osl_pci_slot(osl_t *osh);

/* OSL initialization */
extern osl_t *osl_attach(void *pdev);
extern void osl_detach(osl_t *osh);

/* host/bus architecture-specific byte swap */
#define BUS_SWAP32(v)		(v)

/* general purpose memory allocation */

#if defined(BCMDBG_MEM)

#define	MALLOC(osh, size)	osl_debug_malloc((osh), (size), __LINE__, __FILE__)
#define	MFREE(osh, addr, size)	osl_debug_mfree((osh), (addr), (size), __LINE__, __FILE__)
#define MALLOCED(osh)		osl_malloced((osh))
#define	MALLOC_DUMP(osh, buf, sz) osl_debug_memdump((osh), (buf), (sz))
extern void *osl_debug_malloc(osl_t *osh, uint size, int line, char* file);
extern void osl_debug_mfree(osl_t *osh, void *addr, uint size, int line, char* file);
extern char *osl_debug_memdump(osl_t *osh, char *buf, uint sz);

#else

#define	MALLOC(osh, size)	osl_malloc((osh), (size))
#define	MFREE(osh, addr, size)	osl_mfree((osh), (addr), (size))
#define MALLOCED(osh)		osl_malloced((osh))

#endif	/* BCMDBG_MEM */

#define	MALLOC_FAILED(osh)	osl_malloc_failed((osh))

extern void *osl_malloc(osl_t *osh, uint size);
extern void osl_mfree(osl_t *osh, void *addr, uint size);
extern uint osl_malloced(osl_t *osh);
extern uint osl_malloc_failed(osl_t *osh);

/* allocate/free shared (dma-able) consistent memory */
#define	DMA_CONSISTENT_ALIGN	PAGE_SIZE
#define	DMA_ALLOC_CONSISTENT(osh, size, pap) \
	osl_dma_alloc_consistent((osh), (size), (pap))
#define	DMA_FREE_CONSISTENT(osh, va, size, pa) \
	osl_dma_free_consistent((osh), (void*)(va), (size), (pa))
extern void *osl_dma_alloc_consistent(osl_t *osh, uint size, ulong *pap);
extern void osl_dma_free_consistent(osl_t *osh, void *va, uint size, ulong pa);

/* map/unmap direction */
#define	DMA_TX	1
#define	DMA_RX	2

/* map/unmap shared (dma-able) memory */
#define	DMA_MAP(osh, va, size, direction, p) \
	osl_dma_map((osh), (va), (size), (direction))
#define	DMA_UNMAP(osh, pa, size, direction, p) \
	osl_dma_unmap((osh), (pa), (size), (direction))
extern uint osl_dma_map(osl_t *osh, void *va, uint size, int direction);
extern void osl_dma_unmap(osl_t *osh, uint pa, uint size, int direction);

/* register access macros */
#if defined(BCMJTAG)
#include <bcmjtag.h>
#define	R_REG(r)	bcmjtag_read(NULL, (uint32)(r), sizeof (*(r)))
#define	W_REG(r, v)	bcmjtag_write(NULL, (uint32)(r), (uint32)(v), sizeof (*(r)))
#endif

/*
 * BINOSL selects the slightly slower function-call-based binary compatible osl.
 * Macros expand to calls to functions defined in linux_osl.c .
 */
#ifndef BINOSL

/* string library, kernel mode */
#define	printf(fmt, args...)	printk(fmt, ## args)
#include <linux/kernel.h>
#include <linux/string.h>

/* register access macros */
#if !defined(BCMJTAG)
#ifndef IL_BIGENDIAN   
#define R_REG(r) ( \
	sizeof(*(r)) == sizeof(uint8) ? readb((volatile uint8*)(r)) : \
	sizeof(*(r)) == sizeof(uint16) ? readw((volatile uint16*)(r)) : \
	readl((volatile uint32*)(r)) \
)
#define W_REG(r, v) do { \
	switch (sizeof(*(r))) { \
	case sizeof(uint8):	writeb((uint8)(v), (volatile uint8*)(r)); break; \
	case sizeof(uint16):	writew((uint16)(v), (volatile uint16*)(r)); break; \
	case sizeof(uint32):	writel((uint32)(v), (volatile uint32*)(r)); break; \
	} \
} while (0)
#else	/* IL_BIGENDIAN */
#define R_REG(r) ({ \
	__typeof(*(r)) __osl_v; \
	switch (sizeof(*(r))) { \
	case sizeof(uint8):	__osl_v = readb((volatile uint8*)((uint32)r^3)); break; \
	case sizeof(uint16):	__osl_v = readw((volatile uint16*)((uint32)r^2)); break; \
	case sizeof(uint32):	__osl_v = readl((volatile uint32*)(r)); break; \
	} \
	__osl_v; \
})
#define W_REG(r, v) do { \
	switch (sizeof(*(r))) { \
	case sizeof(uint8):	writeb((uint8)(v), (volatile uint8*)((uint32)r^3)); break; \
	case sizeof(uint16):	writew((uint16)(v), (volatile uint16*)((uint32)r^2)); break; \
	case sizeof(uint32):	writel((uint32)(v), (volatile uint32*)(r)); break; \
	} \
} while (0)
#endif
#endif

#define	AND_REG(r, v)		W_REG((r), R_REG(r) & (v))
#define	OR_REG(r, v)		W_REG((r), R_REG(r) | (v))

/* bcopy, bcmp, and bzero */
#define	bcopy(src, dst, len)	memcpy((dst), (src), (len))
#define	bcmp(b1, b2, len)	memcmp((b1), (b2), (len))
#define	bzero(b, len)		memset((b), '\0', (len))

/* uncached virtual address */
#ifdef mips
#define OSL_UNCACHED(va)	KSEG1ADDR((va))
#include <asm/addrspace.h>
#else
#define OSL_UNCACHED(va)	(va)
#endif

/* get processor cycle count */
#if defined(mips)
#define	OSL_GETCYCLES(x)	((x) = read_c0_count() * 2)
#elif defined(__i386__)
#define	OSL_GETCYCLES(x)	rdtscl((x))
#else
#define OSL_GETCYCLES(x)	((x) = 0)
#endif

/* dereference an address that may cause a bus exception */
#ifdef mips
#if defined(MODULE) && (LINUX_VERSION_CODE < KERNEL_VERSION(2,4,17))
#define BUSPROBE(val, addr)	panic("get_dbe() will not fixup a bus exception when compiled into a module")
#else
#define	BUSPROBE(val, addr)	get_dbe((val), (addr))
#include <asm/paccess.h>
#endif
#else
#define	BUSPROBE(val, addr)	({ (val) = R_REG((addr)); 0; })
#endif

/* map/unmap physical to virtual I/O */
#define	REG_MAP(pa, size)	ioremap_nocache((unsigned long)(pa), (unsigned long)(size))
#define	REG_UNMAP(va)		iounmap((void *)(va))

/* shared (dma-able) memory access macros */
#define	R_SM(r)			*(r)
#define	W_SM(r, v)		(*(r) = (v))
#define	BZERO_SM(r, len)	memset((r), '\0', (len))

/* packet primitives */
#define	PKTGET(osh, len, send)		osl_pktget((osh), (len), (send))
#define	PKTFREE(osh, skb, send)		osl_pktfree((skb))
#define	PKTDATA(osh, skb)		(((struct sk_buff*)(skb))->data)
#define	PKTLEN(osh, skb)		(((struct sk_buff*)(skb))->len)
#define PKTHEADROOM(osh, skb)		(PKTDATA(osh,skb)-(((struct sk_buff*)(skb))->head))
#define PKTTAILROOM(osh, skb)		((((struct sk_buff*)(skb))->end)-(((struct sk_buff*)(skb))->tail))
#define	PKTNEXT(osh, skb)		(((struct sk_buff*)(skb))->next)
#define	PKTSETNEXT(skb, x)		(((struct sk_buff*)(skb))->next = (struct sk_buff*)(x))
#define	PKTSETLEN(osh, skb, len)	__skb_trim((struct sk_buff*)(skb), (len))
#define	PKTPUSH(osh, skb, bytes)	skb_push((struct sk_buff*)(skb), (bytes))
#define	PKTPULL(osh, skb, bytes)	skb_pull((struct sk_buff*)(skb), (bytes))
#define	PKTDUP(osh, skb)		skb_clone((struct sk_buff*)(skb), GFP_ATOMIC)
#define	PKTCOOKIE(skb)			((void*)((struct sk_buff*)(skb))->csum)
#define	PKTSETCOOKIE(skb, x)		(((struct sk_buff*)(skb))->csum = (uint)(x))
#define	PKTLINK(skb)			(((struct sk_buff*)(skb))->prev)
#define	PKTSETLINK(skb, x)		(((struct sk_buff*)(skb))->prev = (struct sk_buff*)(x))
#define	PKTPRIO(skb)			(((struct sk_buff*)(skb))->priority)
#define	PKTSETPRIO(skb, x)		(((struct sk_buff*)(skb))->priority = (x))
extern void *osl_pktget(osl_t *osh, uint len, bool send);
extern void osl_pktfree(void *skb);

#else	/* BINOSL */                                    

/* string library */
#ifndef LINUX_OSL
#undef printf
#define	printf(fmt, args...)		osl_printf((fmt), ## args)
#undef sprintf
#define sprintf(buf, fmt, args...)	osl_sprintf((buf), (fmt), ## args)
#undef strcmp
#define	strcmp(s1, s2)			osl_strcmp((s1), (s2))
#undef strncmp
#define	strncmp(s1, s2, n)		osl_strncmp((s1), (s2), (n))
#undef strlen
#define strlen(s)			osl_strlen((s))
#undef strcpy
#define	strcpy(d, s)			osl_strcpy((d), (s))
#undef strncpy
#define	strncpy(d, s, n)		osl_strncpy((d), (s), (n))
#endif
extern int osl_printf(const char *format, ...);
extern int osl_sprintf(char *buf, const char *format, ...);
extern int osl_strcmp(const char *s1, const char *s2);
extern int osl_strncmp(const char *s1, const char *s2, uint n);
extern int osl_strlen(const char *s);
extern char* osl_strcpy(char *d, const char *s);
extern char* osl_strncpy(char *d, const char *s, uint n);

/* register access macros */
#if !defined(BCMJTAG)
#define R_REG(r) ( \
	sizeof(*(r)) == sizeof(uint8) ? osl_readb((volatile uint8*)(r)) : \
	sizeof(*(r)) == sizeof(uint16) ? osl_readw((volatile uint16*)(r)) : \
	osl_readl((volatile uint32*)(r)) \
)
#define W_REG(r, v) do { \
	switch (sizeof(*(r))) { \
	case sizeof(uint8):	osl_writeb((uint8)(v), (volatile uint8*)(r)); break; \
	case sizeof(uint16):	osl_writew((uint16)(v), (volatile uint16*)(r)); break; \
	case sizeof(uint32):	osl_writel((uint32)(v), (volatile uint32*)(r)); break; \
	} \
} while (0)
#endif

#define	AND_REG(r, v)		W_REG((r), R_REG(r) & (v))
#define	OR_REG(r, v)		W_REG((r), R_REG(r) | (v))
extern uint8 osl_readb(volatile uint8 *r);
extern uint16 osl_readw(volatile uint16 *r);
extern uint32 osl_readl(volatile uint32 *r);
extern void osl_writeb(uint8 v, volatile uint8 *r);
extern void osl_writew(uint16 v, volatile uint16 *r);
extern void osl_writel(uint32 v, volatile uint32 *r);

/* bcopy, bcmp, and bzero */
extern void bcopy(const void *src, void *dst, int len);
extern int bcmp(const void *b1, const void *b2, int len);
extern void bzero(void *b, int len);

/* uncached virtual address */
#define OSL_UNCACHED(va)	osl_uncached((va))
extern void *osl_uncached(void *va);

/* get processor cycle count */
#define OSL_GETCYCLES(x)	((x) = osl_getcycles())
extern uint osl_getcycles(void);

/* dereference an address that may target abort */
#define	BUSPROBE(val, addr)	osl_busprobe(&(val), (addr))
extern int osl_busprobe(uint32 *val, uint32 addr);

/* map/unmap physical to virtual */
#define	REG_MAP(pa, size)	osl_reg_map((pa), (size))
#define	REG_UNMAP(va)		osl_reg_unmap((va))
extern void *osl_reg_map(uint32 pa, uint size);
extern void osl_reg_unmap(void *va);

/* shared (dma-able) memory access macros */
#define	R_SM(r)			*(r)
#define	W_SM(r, v)		(*(r) = (v))
#define	BZERO_SM(r, len)	bzero((r), (len))

/* packet primitives */
#define	PKTGET(osh, len, send)		osl_pktget((osh), (len), (send))
#define	PKTFREE(osh, skb, send)		osl_pktfree((skb))
#define	PKTDATA(osh, skb)		osl_pktdata((osh), (skb))
#define	PKTLEN(osh, skb)		osl_pktlen((osh), (skb))
#define PKTHEADROOM(osh, skb)		osl_pktheadroom((osh), (skb))
#define PKTTAILROOM(osh, skb)		osl_pkttailroom((osh), (skb))
#define	PKTNEXT(osh, skb)		osl_pktnext((osh), (skb))
#define	PKTSETNEXT(skb, x)		osl_pktsetnext((skb), (x))
#define	PKTSETLEN(osh, skb, len)	osl_pktsetlen((osh), (skb), (len))
#define	PKTPUSH(osh, skb, bytes)	osl_pktpush((osh), (skb), (bytes))
#define	PKTPULL(osh, skb, bytes)	osl_pktpull((osh), (skb), (bytes))
#define	PKTDUP(osh, skb)		osl_pktdup((osh), (skb))
#define	PKTCOOKIE(skb)			osl_pktcookie((skb))
#define	PKTSETCOOKIE(skb, x)		osl_pktsetcookie((skb), (x))
#define	PKTLINK(skb)			osl_pktlink((skb))
#define	PKTSETLINK(skb, x)		osl_pktsetlink((skb), (x))
#define	PKTPRIO(skb)			osl_pktprio((skb))
#define	PKTSETPRIO(skb, x)		osl_pktsetprio((skb), (x))
extern void *osl_pktget(osl_t *osh, uint len, bool send);
extern void osl_pktfree(void *skb);
extern uchar *osl_pktdata(osl_t *osh, void *skb);
extern uint osl_pktlen(osl_t *osh, void *skb);
extern uint osl_pktheadroom(osl_t *osh, void *skb);
extern uint osl_pkttailroom(osl_t *osh, void *skb);
extern void *osl_pktnext(osl_t *osh, void *skb);
extern void osl_pktsetnext(void *skb, void *x);
extern void osl_pktsetlen(osl_t *osh, void *skb, uint len);
extern uchar *osl_pktpush(osl_t *osh, void *skb, int bytes);
extern uchar *osl_pktpull(osl_t *osh, void *skb, int bytes);
extern void *osl_pktdup(osl_t *osh, void *skb);
extern void *osl_pktcookie(void *skb);
extern void osl_pktsetcookie(void *skb, void *x);
extern void *osl_pktlink(void *skb);
extern void osl_pktsetlink(void *skb, void *x);
extern uint osl_pktprio(void *skb);
extern void osl_pktsetprio(void *skb, uint x);

#endif	/* BINOSL */

#define OSL_ERROR(bcmerror)	osl_error(bcmerror)
extern int osl_error(int bcmerror);

/* the largest reasonable packet buffer driver uses for ethernet MTU in bytes */
#define	PKTBUFSZ	2048

#endif	/* _linux_osl_h_ */
