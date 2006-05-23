/*
 * HND Minimal OS Abstraction Layer.
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

#ifndef _min_osl_h_
#define _min_osl_h_

#include <typedefs.h>
#include <sbconfig.h>
#include <mipsinc.h>

/* Cache support */
extern void caches_on(void);
extern void blast_dcache(void);
extern void blast_icache(void);

/* uart output */
extern void putc(int c);

/* lib functions */
extern int printf(const char *fmt, ...);
extern int sprintf(char *buf, const char *fmt, ...);
extern int strcmp(const char *s1, const char *s2);
extern int strncmp(const char *s1, const char *s2, uint n);
extern char *strcpy(char *dest, const char *src);
extern char *strncpy(char *dest, const char *src, uint n);
extern uint strlen(const char *s);
extern char *strchr(const char *str,int c);
extern char *strrchr(const char *str, int c);
extern char *strcat(char *d, const char *s);
extern void *memset(void *dest, int c, uint n);
extern void *memcpy(void *dest, const void *src, uint n);
extern int memcmp(const void *s1, const void *s2, uint n);
#define	bcopy(src, dst, len)	memcpy((dst), (src), (len))
#define	bcmp(b1, b2, len)	memcmp((b1), (b2), (len))
#define	bzero(b, len)		memset((b), '\0', (len))

/* assert & debugging */
#define	ASSERT(exp)		do {} while (0)

/* PCMCIA attribute space access macros */
#define	OSL_PCMCIA_READ_ATTR(osh, offset, buf, size) \
	ASSERT(0)
#define	OSL_PCMCIA_WRITE_ATTR(osh, offset, buf, size) \
	ASSERT(0)

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
#define wreg16(r, v)		(*(volatile uint16*)(r) = (uint16)(v))
#define rreg16(r)		(*(volatile uint16*)(r))
#define wreg8(r, v)		(*(volatile uint8*)(r) = (uint8)(v))
#define rreg8(r)		(*(volatile uint8*)(r))
#define R_REG(r) ({ \
	__typeof(*(r)) __osl_v; \
	switch (sizeof(*(r))) { \
	case sizeof(uint8):	__osl_v = rreg8((r)); break; \
	case sizeof(uint16):	__osl_v = rreg16((r)); break; \
	case sizeof(uint32):	__osl_v = rreg32((r)); break; \
	} \
	__osl_v; \
})
#define W_REG(r, v) do { \
	switch (sizeof(*(r))) { \
	case sizeof(uint8):	wreg8((r), (v)); break; \
	case sizeof(uint16):	wreg16((r), (v)); break; \
	case sizeof(uint32):	wreg32((r), (v)); break; \
	} \
} while (0)
#define	AND_REG(r, v)		W_REG((r), R_REG(r) & (v))
#define	OR_REG(r, v)		W_REG((r), R_REG(r) | (v))

/* general purpose memory allocation */
#define	MALLOC(osh, size)	malloc(size)
#define	MFREE(osh, addr, size)	free(addr)
#define	MALLOCED(osh)		0
#define	MALLOC_FAILED(osh)	0
#define	MALLOC_DUMP(osh, buf, sz)
extern int free(void *ptr);
extern void *malloc(uint size);

/* uncached virtual address */
#define	OSL_UNCACHED(va)	((void*)KSEG1ADDR((ulong)(va)))

/* host/bus architecture-specific address byte swap */
#define BUS_SWAP32(v)		(v)

/* microsecond delay */
#define	OSL_DELAY(usec)		udelay(usec)
extern void udelay(uint32 usec);

/* map/unmap physical to virtual I/O */
#define	REG_MAP(pa, size)	((void*)KSEG1ADDR((ulong)(pa)))
#define	REG_UNMAP(va)		do {} while (0)

/* dereference an address that may cause a bus exception */
#define	BUSPROBE(val, addr)	(uint32 *)(addr) = (val)

/* Misc stubs */
#define osl_attach(pdev)	((osl_t*)pdev)
#define osl_detach(osh)
extern void *osl_init(void);
#define OSL_ERROR(bcmerror)	osl_error(bcmerror)
extern int osl_error(int);

#endif	/* _min_osl_h_ */
