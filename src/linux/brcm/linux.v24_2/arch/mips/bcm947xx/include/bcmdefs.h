/*
 * Misc system wide definitions
 *
 * Copyright 2007, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 * $Id$
 */

#ifndef	_bcmdefs_h_
#define	_bcmdefs_h_

/*
 * One doesn't need to include this file explicitly, gets included automatically if
 * typedefs.h is included.
 */

/* Reclaiming text and data :
 * The following macros specify special linker sections that can be reclaimed
 * after a system is considered 'up'.
 */
#if defined(BCMRECLAIM)
extern bool bcmreclaimed;
#define BCMINITDATA(_data)	__attribute__ ((__section__ (".dataini." #_data))) _data
#define BCMINITFN(_fn)		__attribute__ ((noinline, __section__ (".textini." #_fn))) _fn
#else
#define BCMINITDATA(_data)	_data
#define BCMINITFN(_fn)		_fn
#define bcmreclaimed		0
#endif

/* Reclaim uninit functions if BCMNODOWN is defined		*/
/* and if they are not already removed by -gc-sections	*/
#ifdef BCMNODOWN
#define BCMUNINITFN(_fn)		BCMINITFN(_fn)
#else
#define BCMUNINITFN(_fn)		_fn
#endif

#ifdef BCMRECLAIM
#define CONST
#else
#define CONST	const
#endif /* BCMRECLAIM */

#define BCMINIT(_id)		_id

/* Put some library data/code into ROM to reduce RAM requirements */
#if defined(BCMROMOFFLOAD)
#define BCMROMDATA(_data)	__attribute__ ((__section__ (".datarom." #_data))) _data
#define BCMROMFN(_fn)		__attribute__ ((noinline, long_call)) _fn
#else
#define BCMROMDATA(_data)	_data
#define BCMROMFN(_fn)		_fn
#endif

/* Bus types */
#define	SB_BUS			0	/* Silicon Backplane */
#define	PCI_BUS			1	/* PCI target */
#define	PCMCIA_BUS		2	/* PCMCIA target */
#define SDIO_BUS		3	/* SDIO target */
#define JTAG_BUS		4	/* JTAG */
#define USB_BUS			5	/* USB (does not support R/W REG) */

/* Allows optimization for single-bus support */
#ifdef BCMBUSTYPE
#define BUSTYPE(bus) (BCMBUSTYPE)
#else
#define BUSTYPE(bus) (bus)
#endif

/* Tell where the SPROM is hanging off */
#if defined(BCMSPROMBUS)
#define SPROMBUS	(BCMSPROMBUS)
#elif defined(SB_PCMCIA_SROM)
#define SPROMBUS	(PCMCIA_BUS)
#else
#define SPROMBUS	(PCI_BUS)
#endif

/* Defines for DMA Address Width - Shared between OSL and HNDDMA */
#define DMADDR_MASK_32 0x0		/* Address mask for 32-bits */
#define DMADDR_MASK_30 0xc0000000	/* Address mask for 30-bits */
#define DMADDR_MASK_0  0xffffffff	/* Address mask for 0-bits (hi-part) */

#define	DMADDRWIDTH_30  30 /* 30-bit addressing capability */
#define	DMADDRWIDTH_32  32 /* 32-bit addressing capability */
#define	DMADDRWIDTH_63  63 /* 64-bit addressing capability */
#define	DMADDRWIDTH_64  64 /* 64-bit addressing capability */

/* packet headroom necessary to accomodate the largest header in the system, (i.e TXOFF).
 * By doing, we avoid the need  to allocate an extra buffer for the header when bridging to WL.
 * There is a compile time check in wlc.c which ensure that this value is at least as big
 * as TXOFF. This value is used in dma_rxfill (hnddma.c).
 */
#define BCMEXTRAHDROOM 160

/* Headroom required for dongle-to-host communication.  Packets allocated
 * locally in the dongle (e.g. for CDC ioctls or RNDIS messages) should
 * leave this much room in front for low-level message headers which may
 * be needed to get across the dongle bus to the host.  (These messages
 * don't go over the network, so room for the full WL header above would
 * be a waste.).
 */
#ifdef BCMUSBDEV
#define BCMDONGLEHDRSZ 0
#else
#define BCMDONGLEHDRSZ 8
#endif

#ifdef BCMDBG

#define BCMDBG_ERR

#ifndef BCMDBG_ASSERT
#define BCMDBG_ASSERT
#endif /* BCMDBG_ASSERT */

#endif /* BCMDBG */

/* Brett's nifty macros for doing definition and get/set of bitfields
 * Usage example, e.g. a three-bit field (bits 4-6):
 *    #define <NAME>_M	BITFIELD_MASK(3)
 *    #define <NAME>_S	4
 * ...
 *    regval = R_REG(osh, &regs->regfoo);
 *    field = GFIELD(regval, <NAME>);
 *    regval = SFIELD(regval, <NAME>, 1);
 *    W_REG(osh, &regs->regfoo, regval);
 */
#define BITFIELD_MASK(width) ((1 << width)-1)
#define GFIELD(val, field) ((val >> field ## _S) & field ## _M)
#define SFIELD(val, field, bits) ((val & (~(field ## _M << field ## _S))) | (bits << field ## _S))

/* define BCMSMALL to remove misc features for memory constrained enviroments */
#ifdef BCMSMALL
#undef	BCMSPACE
#define bcmspace	FALSE	/* if (bcmspace) code is discarded */
#else
#define	BCMSPACE
#define bcmspace	TRUE	/* if (bcmspace) code is retained */
#endif

/* Max. nvram variable table size */
#define	MAXSZ_NVRAM_VARS	4096	

#endif /* _bcmdefs_h_ */
