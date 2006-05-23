/*
 * Generic Broadcom Home Networking Division (HND) DMA module.
 * This supports the following chips: BCM42xx, 44xx, 47xx .
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
#include <bcmendian.h>
#include <sbconfig.h>
#include <bcmutils.h>
#include <bcmdevs.h>
#include <sbutils.h>

struct dma_info;	/* forward declaration */
#define di_t struct dma_info

#include <sbhnddma.h>
#include <hnddma.h>

/* debug/trace */
#define	DMA_ERROR(args)
#define	DMA_TRACE(args)

/* default dma message level (if input msg_level pointer is null in dma_attach()) */
static uint dma_msg_level =
	0;

#define	MAXNAMEL	8

/* dma engine software state */
typedef struct dma_info {
	hnddma_t	hnddma;		/* exported structure */
	uint		*msg_level;	/* message level pointer */
	char		name[MAXNAMEL];	/* callers name for diag msgs */
	
	void		*osh;		/* os handle */
	sb_t		*sbh;		/* sb handle */
	
	bool		dma64;		/* dma64 enabled */
	bool		addrext;	/* this dma engine supports DmaExtendedAddrChanges */
	
	dma32regs_t	*d32txregs;	/* 32 bits dma tx engine registers */
	dma32regs_t	*d32rxregs;	/* 32 bits dma rx engine registers */
	dma64regs_t	*d64txregs;	/* 64 bits dma tx engine registers */
	dma64regs_t	*d64rxregs;	/* 64 bits dma rx engine registers */

	uint32		dma64align;	/* either 8k or 4k depends on number of dd */
	dma32dd_t	*txd32;		/* pointer to dma32 tx descriptor ring */
	dma64dd_t	*txd64;		/* pointer to dma64 tx descriptor ring */
	uint		ntxd;		/* # tx descriptors tunable */	
	uint		txin;		/* index of next descriptor to reclaim */
	uint		txout;		/* index of next descriptor to post */
	uint		txavail;	/* # free tx descriptors */
	void		**txp;		/* pointer to parallel array of pointers to packets */
	ulong		txdpa;		/* physical address of descriptor ring */
	uint		txdalign;	/* #bytes added to alloc'd mem to align txd */
	uint		txdalloc;	/* #bytes allocated for the ring */

	dma32dd_t	*rxd32;		/* pointer to dma32 rx descriptor ring */
	dma64dd_t	*rxd64;		/* pointer to dma64 rx descriptor ring */
	uint		nrxd;		/* # rx descriptors tunable */	
	uint		rxin;		/* index of next descriptor to reclaim */
	uint		rxout;		/* index of next descriptor to post */
	void		**rxp;		/* pointer to parallel array of pointers to packets */
	ulong		rxdpa;		/* physical address of descriptor ring */
	uint		rxdalign;	/* #bytes added to alloc'd mem to align rxd */
	uint		rxdalloc;	/* #bytes allocated for the ring */

	/* tunables */
	uint		rxbufsize;	/* rx buffer size in bytes */
	uint		nrxpost;	/* # rx buffers to keep posted */
	uint		rxoffset;	/* rxcontrol offset */
	uint		ddoffsetlow;	/* add to get dma address of descriptor ring, low 32 bits */
	uint		ddoffsethigh;	/* add to get dma address of descriptor ring, high 32 bits */
	uint		dataoffsetlow;	/* add to get dma address of data buffer, low 32 bits */
	uint		dataoffsethigh;	/* add to get dma address of data buffer, high 32 bits */
} dma_info_t;

#ifdef BCMDMA64
#define	DMA64_ENAB(di)	((di)->dma64)
#else
#define	DMA64_ENAB(di)	(0)
#endif

/* descriptor bumping macros */
#define	XXD(x, n)	((x) & ((n) - 1))
#define	TXD(x)		XXD((x), di->ntxd)
#define	RXD(x)		XXD((x), di->nrxd)
#define	NEXTTXD(i)	TXD(i + 1)
#define	PREVTXD(i)	TXD(i - 1)
#define	NEXTRXD(i)	RXD(i + 1)
#define	NTXDACTIVE(h, t)	TXD(t - h)
#define	NRXDACTIVE(h, t)	RXD(t - h)

/* macros to convert between byte offsets and indexes */
#define	B2I(bytes, type)	((bytes) / sizeof(type))
#define	I2B(index, type)	((index) * sizeof(type))

#define	PCI32ADDR_HIGH		0xc0000000	/* address[31:30] */
#define	PCI32ADDR_HIGH_SHIFT	30


/* prototypes */
static bool dma_isaddrext(dma_info_t *di);
static bool dma_alloc(dma_info_t *di, uint direction);

static bool dma32_alloc(dma_info_t *di, uint direction);
static void dma32_txreset(dma_info_t *di);
static void dma32_rxreset(dma_info_t *di);
static bool dma32_txsuspendedidle(dma_info_t *di);
static int  dma32_txfast(dma_info_t *di, void *p0, uint32 coreflags);
static void* dma32_getnexttxp(dma_info_t *di, bool forceall);
static void* dma32_getnextrxp(dma_info_t *di, bool forceall);
static void dma32_txrotate(di_t *di);

/* prototype or stubs */
#ifdef BCMDMA64
static bool dma64_alloc(dma_info_t *di, uint direction);
static void dma64_txreset(dma_info_t *di);
static void dma64_rxreset(dma_info_t *di);
static bool dma64_txsuspendedidle(dma_info_t *di);
static int  dma64_txfast(dma_info_t *di, void *p0, uint32 coreflags);
static void* dma64_getnexttxp(dma_info_t *di, bool forceall);
static void* dma64_getnextrxp(dma_info_t *di, bool forceall);
static void dma64_txrotate(di_t *di);
#else
static bool dma64_alloc(dma_info_t *di, uint direction) { return TRUE; }
static void dma64_txreset(dma_info_t *di) {}
static void dma64_rxreset(dma_info_t *di) {}
static bool dma64_txsuspendedidle(dma_info_t *di) { return TRUE;}
static int  dma64_txfast(dma_info_t *di, void *p0, uint32 coreflags) { return 0; }
static void* dma64_getnexttxp(dma_info_t *di, bool forceall) { return NULL; }
static void* dma64_getnextrxp(dma_info_t *di, bool forceall) { return NULL; }
static void dma64_txrotate(di_t *di) { return; }
#endif



void* 
dma_attach(osl_t *osh, char *name, sb_t *sbh, void *dmaregstx, void *dmaregsrx,
	   uint ntxd, uint nrxd, uint rxbufsize, uint nrxpost, uint rxoffset, uint *msg_level)
{
	dma_info_t *di;
	uint size;

	/* allocate private info structure */
	if ((di = MALLOC(osh, sizeof (dma_info_t))) == NULL) {
		return (NULL);
	}
	bzero((char*)di, sizeof (dma_info_t));

	di->msg_level = msg_level ? msg_level : &dma_msg_level;

	if (sbh != NULL)
		di->dma64 = ((sb_coreflagshi(sbh, 0, 0) & SBTMH_DMA64) == SBTMH_DMA64);

#ifndef BCMDMA64
	if (di->dma64) {
		DMA_ERROR(("dma_attach: driver doesn't have the capability to support 64 bits DMA\n"));
		goto fail;
	}
#endif
	
	/* check arguments */
	ASSERT(ISPOWEROF2(ntxd));
	ASSERT(ISPOWEROF2(nrxd));
	if (nrxd == 0)
		ASSERT(dmaregsrx == NULL);
	if (ntxd == 0)
		ASSERT(dmaregstx == NULL);


	/* init dma reg pointer */
	if (di->dma64) {
		ASSERT(ntxd <= D64MAXDD);
		ASSERT(nrxd <= D64MAXDD);
		di->d64txregs = (dma64regs_t *)dmaregstx;
		di->d64rxregs = (dma64regs_t *)dmaregsrx;

		di->dma64align = D64RINGALIGN;
		if ((ntxd < D64MAXDD / 2) && (nrxd < D64MAXDD / 2)) {
			/* for smaller dd table, HW relax the alignment requirement */
			di->dma64align = D64RINGALIGN / 2;
		}
	} else {
		ASSERT(ntxd <= D32MAXDD);
		ASSERT(nrxd <= D32MAXDD);
		di->d32txregs = (dma32regs_t *)dmaregstx;
		di->d32rxregs = (dma32regs_t *)dmaregsrx;
	}


	/* make a private copy of our callers name */
	strncpy(di->name, name, MAXNAMEL);
	di->name[MAXNAMEL-1] = '\0';

	di->osh = osh;
	di->sbh = sbh;

	/* save tunables */
	di->ntxd = ntxd;
	di->nrxd = nrxd;
	di->rxbufsize = rxbufsize;
	di->nrxpost = nrxpost;
	di->rxoffset = rxoffset;

	/* 
	 * figure out the DMA physical address offset for dd and data 
	 *   for old chips w/o sb, use zero
	 *   for new chips w sb, 
	 *     PCI/PCIE: they map silicon backplace address to zero based memory, need offset
	 *     Other bus: use zero
	 *     SB_BUS BIGENDIAN kludge: use sdram swapped region for data buffer, not descriptor
	 */
	di->ddoffsetlow = 0;
	di->dataoffsetlow = 0;
	if (sbh != NULL) {	
		if (sbh->bustype == PCI_BUS) {  /* for pci bus, add offset */
			if ((sbh->buscoretype == SB_PCIE) && di->dma64){
				di->ddoffsetlow = 0;
				di->ddoffsethigh = SB_PCIE_DMA_H32;
			} else {
				di->ddoffsetlow = SB_PCI_DMA;
				di->ddoffsethigh = 0;
			}
			di->dataoffsetlow =  di->ddoffsetlow;
			di->dataoffsethigh =  di->ddoffsethigh;
		} 
#if defined(__mips__) && defined(IL_BIGENDIAN)
		/* use sdram swapped region for data buffers but not dma descriptors */
		di->dataoffsetlow = di->dataoffsetlow + SB_SDRAM_SWAPPED;
#endif
	}

	di->addrext = dma_isaddrext(di);

	DMA_TRACE(("%s: dma_attach: osh %p ntxd %d nrxd %d rxbufsize %d nrxpost %d rxoffset %d ddoffset 0x%x dataoffset 0x%x\n", 
		   name, osh, ntxd, nrxd, rxbufsize, nrxpost, rxoffset, di->ddoffsetlow, di->dataoffsetlow));

	/* allocate tx packet pointer vector */
	if (ntxd) {
		size = ntxd * sizeof (void*);
		if ((di->txp = MALLOC(osh, size)) == NULL) {
			DMA_ERROR(("%s: dma_attach: out of tx memory, malloced %d bytes\n", di->name, MALLOCED(osh)));
			goto fail;
		}
		bzero((char*)di->txp, size);
	}

	/* allocate rx packet pointer vector */
	if (nrxd) {
		size = nrxd * sizeof (void*);
		if ((di->rxp = MALLOC(osh, size)) == NULL) {
			DMA_ERROR(("%s: dma_attach: out of rx memory, malloced %d bytes\n", di->name, MALLOCED(osh)));
			goto fail;
		}
		bzero((char*)di->rxp, size);
	} 

	/* allocate transmit descriptor ring, only need ntxd descriptors but it must be aligned */
	if (ntxd) {
		if (!dma_alloc(di, DMA_TX))
			goto fail;
	}

	/* allocate receive descriptor ring, only need nrxd descriptors but it must be aligned */
	if (nrxd) {
		if (!dma_alloc(di, DMA_RX))
			goto fail;
	}

	if ((di->ddoffsetlow == SB_PCI_DMA) && (di->txdpa > SB_PCI_DMA_SZ) && !di->addrext) {
		DMA_ERROR(("%s: dma_attach: txdpa 0x%lx: addrext not supported\n", di->name, di->txdpa));
		goto fail;
	}
	if ((di->ddoffsetlow == SB_PCI_DMA) && (di->rxdpa > SB_PCI_DMA_SZ) && !di->addrext) {
		DMA_ERROR(("%s: dma_attach: rxdpa 0x%lx: addrext not supported\n", di->name, di->rxdpa));
		goto fail;
	}

	return ((void*)di);

fail:
	dma_detach((void*)di);
	return (NULL);
}

static bool
dma_alloc(dma_info_t *di, uint direction)
{
	if (DMA64_ENAB(di)) {
		return dma64_alloc(di, direction);
	} else {
		return dma32_alloc(di, direction);
	}
}

/* may be called with core in reset */
void
dma_detach(dma_info_t *di)
{
	if (di == NULL)
		return;

	DMA_TRACE(("%s: dma_detach\n", di->name));

	/* shouldn't be here if descriptors are unreclaimed */
	ASSERT(di->txin == di->txout);
	ASSERT(di->rxin == di->rxout);

	/* free dma descriptor rings */
	if (di->txd32)
		DMA_FREE_CONSISTENT(di->osh, ((int8*)di->txd32 - di->txdalign), di->txdalloc, (di->txdpa - di->txdalign));
	if (di->rxd32)
		DMA_FREE_CONSISTENT(di->osh, ((int8*)di->rxd32 - di->rxdalign), di->rxdalloc, (di->rxdpa - di->rxdalign));

	/* free packet pointer vectors */
	if (di->txp)
		MFREE(di->osh, (void*)di->txp, (di->ntxd * sizeof (void*)));
	if (di->rxp)
		MFREE(di->osh, (void*)di->rxp, (di->nrxd * sizeof (void*)));

	/* free our private info structure */
	MFREE(di->osh, (void*)di, sizeof (dma_info_t));
}

/* return TRUE if this dma engine supports DmaExtendedAddrChanges, otherwise FALSE */
static bool
dma_isaddrext(dma_info_t *di)
{
	uint32 w;

	if (DMA64_ENAB(di)) {
		OR_REG(&di->d64txregs->control, D64_XC_AE);
		w = R_REG(&di->d32txregs->control);
		AND_REG(&di->d32txregs->control, ~D64_XC_AE);
		return ((w & XC_AE) == D64_XC_AE);
	} else {
		OR_REG(&di->d32txregs->control, XC_AE);
		w = R_REG(&di->d32txregs->control);
		AND_REG(&di->d32txregs->control, ~XC_AE);
		return ((w & XC_AE) == XC_AE);
	}
}

void
dma_txreset(dma_info_t *di)
{
	DMA_TRACE(("%s: dma_txreset\n", di->name));

	if (DMA64_ENAB(di))
		dma64_txreset(di);
	else
		dma32_txreset(di);
}

void
dma_rxreset(dma_info_t *di)
{
	DMA_TRACE(("%s: dma_rxreset\n", di->name));

	if (DMA64_ENAB(di))
		dma64_rxreset(di);
	else
		dma32_rxreset(di);
}

/* initialize descriptor table base address */
static void
dma_ddtable_init(dma_info_t *di, uint direction, ulong pa)
{
	if (DMA64_ENAB(di)) {
		if (direction == DMA_TX) {
			W_REG(&di->d64txregs->addrlow, pa + di->ddoffsetlow);
			W_REG(&di->d64txregs->addrhigh, di->ddoffsethigh);
		} else {
			W_REG(&di->d64rxregs->addrlow, pa + di->ddoffsetlow);
			W_REG(&di->d64rxregs->addrhigh, di->ddoffsethigh);
		}
	} else {
		uint32 offset = di->ddoffsetlow;
		if ((offset != SB_PCI_DMA) || !(pa & PCI32ADDR_HIGH)) {
			if (direction == DMA_TX)	
				W_REG(&di->d32txregs->addr, (pa + offset));
			else
				W_REG(&di->d32rxregs->addr, (pa + offset));
		} else {        
			/* dma32 address extension */
			uint32 ae;
			ASSERT(di->addrext);
			ae = (pa & PCI32ADDR_HIGH) >> PCI32ADDR_HIGH_SHIFT;
	
			if (direction == DMA_TX) {
				W_REG(&di->d32txregs->addr, ((pa & ~PCI32ADDR_HIGH) + offset));
				SET_REG(&di->d32txregs->control, XC_AE, (ae << XC_AE_SHIFT));
			} else {
				W_REG(&di->d32rxregs->addr, ((pa & ~PCI32ADDR_HIGH) + offset));
				SET_REG(&di->d32rxregs->control, RC_AE, (ae << RC_AE_SHIFT));
			}
		}
	}
}

/* init the tx or rx descriptor */
static INLINE void
dma32_dd_upd(dma_info_t *di, dma32dd_t *ddring, ulong pa, uint outidx, uint32 *ctrl)
{
	uint offset = di->dataoffsetlow;

	if ((offset != SB_PCI_DMA) || !(pa & PCI32ADDR_HIGH)) {
		W_SM(&ddring[outidx].addr, BUS_SWAP32(pa + offset));
		W_SM(&ddring[outidx].ctrl, BUS_SWAP32(*ctrl));
	} else {        
		/* address extension */
		uint32 ae;
		ASSERT(di->addrext);
		ae = (pa & PCI32ADDR_HIGH) >> PCI32ADDR_HIGH_SHIFT;

		*ctrl |= (ae << CTRL_AE_SHIFT);
		W_SM(&ddring[outidx].addr, BUS_SWAP32((pa & ~PCI32ADDR_HIGH) + offset));
		W_SM(&ddring[outidx].ctrl, BUS_SWAP32(*ctrl));
	}
}

/* init the tx or rx descriptor */
static INLINE void
dma64_dd_upd(dma_info_t *di, dma64dd_t *ddring, ulong pa, uint outidx, uint32 *flags, uint32 bufcount)
{
	uint32 bufaddr_low = pa + di->dataoffsetlow;
	uint32 bufaddr_high = 0 + di->dataoffsethigh;

	uint32 ctrl2 = bufcount & D64_CTRL2_BC_MASK;

	W_SM(&ddring[outidx].addrlow, BUS_SWAP32(bufaddr_low));
	W_SM(&ddring[outidx].addrhigh, BUS_SWAP32(bufaddr_high));
	W_SM(&ddring[outidx].ctrl1, BUS_SWAP32(*flags));
	W_SM(&ddring[outidx].ctrl2, BUS_SWAP32(ctrl2));
}

void
dma_txinit(dma_info_t *di)
{
	DMA_TRACE(("%s: dma_txinit\n", di->name));

	di->txin = di->txout = 0;
	di->txavail = di->ntxd - 1;

	/* clear tx descriptor ring */
	if (DMA64_ENAB(di)) {
		BZERO_SM((void*)di->txd64, (di->ntxd * sizeof (dma64dd_t)));
		W_REG(&di->d64txregs->control, XC_XE);
		dma_ddtable_init(di, DMA_TX, di->txdpa);
	} else {
		BZERO_SM((void*)di->txd32, (di->ntxd * sizeof (dma32dd_t)));
		W_REG(&di->d32txregs->control, XC_XE);
		dma_ddtable_init(di, DMA_TX, di->txdpa);
	}
}

bool
dma_txenabled(dma_info_t *di)
{
	uint32 xc;
	
	/* If the chip is dead, it is not enabled :-) */
	if (DMA64_ENAB(di)) {
		xc = R_REG(&di->d64txregs->control);
		return ((xc != 0xffffffff) && (xc & D64_XC_XE));
	} else {
		xc = R_REG(&di->d32txregs->control);
		return ((xc != 0xffffffff) && (xc & XC_XE));
	}
}

void
dma_txsuspend(dma_info_t *di)
{
	DMA_TRACE(("%s: dma_txsuspend\n", di->name));
	if (DMA64_ENAB(di))
		OR_REG(&di->d64txregs->control, D64_XC_SE);
	else
		OR_REG(&di->d32txregs->control, XC_SE);
}

void
dma_txresume(dma_info_t *di)
{
	DMA_TRACE(("%s: dma_txresume\n", di->name));
	if (DMA64_ENAB(di))
		AND_REG(&di->d64txregs->control, ~D64_XC_SE);
	else
		AND_REG(&di->d32txregs->control, ~XC_SE);
}

bool
dma_txsuspendedidle(dma_info_t *di)
{
	if (DMA64_ENAB(di))
		return dma64_txsuspendedidle(di);
	else
		return dma32_txsuspendedidle(di);
}

bool
dma_txsuspended(dma_info_t *di)
{
	if (DMA64_ENAB(di))
		return ((R_REG(&di->d64txregs->control) & D64_XC_SE) == D64_XC_SE);
	else
		return ((R_REG(&di->d32txregs->control) & XC_SE) == XC_SE);
}

bool
dma_txstopped(dma_info_t *di)
{
	if (DMA64_ENAB(di))
		return ((R_REG(&di->d64txregs->status0) & D64_XS0_XS_MASK) == D64_XS0_XS_STOPPED);
	else
		return ((R_REG(&di->d32txregs->status) & XS_XS_MASK) == XS_XS_STOPPED);
}

bool
dma_rxstopped(dma_info_t *di)
{
	if (DMA64_ENAB(di))
		return ((R_REG(&di->d64rxregs->status0) & D64_RS0_RS_MASK) == D64_RS0_RS_STOPPED);
	else
		return ((R_REG(&di->d32rxregs->status) & RS_RS_MASK) == RS_RS_STOPPED);
}

void
dma_fifoloopbackenable(dma_info_t *di)
{
	DMA_TRACE(("%s: dma_fifoloopbackenable\n", di->name));
	if (DMA64_ENAB(di))
		OR_REG(&di->d64txregs->control, D64_XC_LE);
	else
		OR_REG(&di->d32txregs->control, XC_LE);
}

void
dma_rxinit(dma_info_t *di)
{
	DMA_TRACE(("%s: dma_rxinit\n", di->name));

	di->rxin = di->rxout = 0;

	/* clear rx descriptor ring */
	if (DMA64_ENAB(di)) {
                BZERO_SM((void*)di->rxd64, (di->nrxd * sizeof (dma64dd_t)));
		dma_rxenable(di);
		dma_ddtable_init(di, DMA_RX, di->rxdpa);
	} else {
		BZERO_SM((void*)di->rxd32, (di->nrxd * sizeof (dma32dd_t)));
		dma_rxenable(di);
		dma_ddtable_init(di, DMA_RX, di->rxdpa);
	}
}

void
dma_rxenable(dma_info_t *di)
{
	DMA_TRACE(("%s: dma_rxenable\n", di->name));
	if (DMA64_ENAB(di))
		W_REG(&di->d64rxregs->control, ((di->rxoffset << D64_RC_RO_SHIFT) | D64_RC_RE));
	else
		W_REG(&di->d32rxregs->control, ((di->rxoffset << RC_RO_SHIFT) | RC_RE));
}

bool
dma_rxenabled(dma_info_t *di)
{
	uint32 rc;

	if (DMA64_ENAB(di)) { 
		rc = R_REG(&di->d64rxregs->control);
		return ((rc != 0xffffffff) && (rc & D64_RC_RE));
	} else {
		rc = R_REG(&di->d32rxregs->control);
		return ((rc != 0xffffffff) && (rc & RC_RE));
	}
}


/* !! tx entry routine */
int
dma_txfast(dma_info_t *di, void *p0, uint32 coreflags)
{
	if (DMA64_ENAB(di)) { 
		return dma64_txfast(di, p0, coreflags);
	} else {
		return dma32_txfast(di, p0, coreflags);
	}
}

/* !! rx entry routine, returns a pointer to the next frame received, or NULL if there are no more */
void*
dma_rx(dma_info_t *di)
{
	void *p;
	uint len;
	int skiplen = 0;

	while ((p = dma_getnextrxp(di, FALSE))) {
		/* skip giant packets which span multiple rx descriptors */
		if (skiplen > 0) {
			skiplen -= di->rxbufsize;
			if (skiplen < 0)
				skiplen = 0;
			PKTFREE(di->osh, p, FALSE);
			continue;
		}

		len = ltoh16(*(uint16*)(PKTDATA(di->osh, p)));
		DMA_TRACE(("%s: dma_rx len %d\n", di->name, len));

		/* bad frame length check */
		if (len > (di->rxbufsize - di->rxoffset)) {
			DMA_ERROR(("%s: dma_rx: bad frame length (%d)\n", di->name, len));
			if (len > 0)
				skiplen = len - (di->rxbufsize - di->rxoffset);
			PKTFREE(di->osh, p, FALSE);
			di->hnddma.rxgiants++;
			continue;
		}

		/* set actual length */
		PKTSETLEN(di->osh, p, (di->rxoffset + len));

		break;
	}

	return (p);
}

/* post receive buffers */
void
dma_rxfill(dma_info_t *di)
{
	void *p;
	uint rxin, rxout;
	uint32 ctrl;
	uint n;
	uint i;
	uint32 pa;
	uint rxbufsize;

	/*
	 * Determine how many receive buffers we're lacking
	 * from the full complement, allocate, initialize,
	 * and post them, then update the chip rx lastdscr.
	 */

	rxin = di->rxin;
	rxout = di->rxout;
	rxbufsize = di->rxbufsize;

	n = di->nrxpost - NRXDACTIVE(rxin, rxout);

	DMA_TRACE(("%s: dma_rxfill: post %d\n", di->name, n));

	for (i = 0; i < n; i++) {
		if ((p = PKTGET(di->osh, rxbufsize, FALSE)) == NULL) {
			DMA_ERROR(("%s: dma_rxfill: out of rxbufs\n", di->name));
			di->hnddma.rxnobuf++;
			break;
		}

		/* Do a cached write instead of uncached write since DMA_MAP
		 * will flush the cache. */
		*(uint32*)(PKTDATA(di->osh, p)) = 0;

		pa = (uint32) DMA_MAP(di->osh, PKTDATA(di->osh, p), rxbufsize, DMA_RX, p);
		ASSERT(ISALIGNED(pa, 4));

		/* save the free packet pointer */
		ASSERT(di->rxp[rxout] == NULL);
		di->rxp[rxout] = p;

		if (DMA64_ENAB(di)) {
			/* prep the descriptor control value */
			if (rxout == (di->nrxd - 1))
				ctrl = CTRL_EOT;

			dma64_dd_upd(di, di->rxd64, pa, rxout, &ctrl, rxbufsize);
		} else {
			/* prep the descriptor control value */
			ctrl = rxbufsize;
			if (rxout == (di->nrxd - 1))
				ctrl |= CTRL_EOT;
			dma32_dd_upd(di, di->rxd32, pa, rxout, &ctrl);
		}

		rxout = NEXTRXD(rxout);
	}

	di->rxout = rxout;

	/* update the chip lastdscr pointer */
	if (DMA64_ENAB(di)) {
		W_REG(&di->d64rxregs->ptr, I2B(rxout, dma64dd_t));
	} else {
		W_REG(&di->d32rxregs->ptr, I2B(rxout, dma32dd_t));
	}
}

void
dma_txreclaim(dma_info_t *di, bool forceall)
{
	void *p;

	DMA_TRACE(("%s: dma_txreclaim %s\n", di->name, forceall ? "all" : ""));

	while ((p = dma_getnexttxp(di, forceall)))
		PKTFREE(di->osh, p, TRUE);
}

/*
 * Reclaim next completed txd (txds if using chained buffers) and
 * return associated packet.
 * If 'force' is true, reclaim txd(s) and return associated packet
 * regardless of the value of the hardware "curr" pointer.
 */
void*
dma_getnexttxp(dma_info_t *di, bool forceall)
{
	if (DMA64_ENAB(di)) {
		return dma64_getnexttxp(di, forceall);
	} else {
		return dma32_getnexttxp(di, forceall);
	}
}
	
/* like getnexttxp but no reclaim */
void*
dma_peeknexttxp(dma_info_t *di)
{
	uint end, i;

	if (DMA64_ENAB(di)) {
		end = B2I(R_REG(&di->d64txregs->status0) & D64_XS0_CD_MASK, dma64dd_t);
	} else {
		end = B2I(R_REG(&di->d32txregs->status) & XS_CD_MASK, dma32dd_t);
	}

	for (i = di->txin; i != end; i = NEXTTXD(i))
		if (di->txp[i])
			return (di->txp[i]);

	return (NULL);
}

/*
 * Rotate all active tx dma ring entries "forward" by (ActiveDescriptor - txin).
 */
void
dma_txrotate(di_t *di)
{
	if (DMA64_ENAB(di)) {
		dma64_txrotate(di);
	} else {
		dma32_txrotate(di);
	}
}

void
dma_rxreclaim(dma_info_t *di)
{
	void *p;

	DMA_TRACE(("%s: dma_rxreclaim\n", di->name));

	while ((p = dma_getnextrxp(di, TRUE)))
		PKTFREE(di->osh, p, FALSE);
}

void *
dma_getnextrxp(dma_info_t *di, bool forceall)
{
	if (DMA64_ENAB(di)) {
		return dma64_getnextrxp(di, forceall);
	} else {
		return dma32_getnextrxp(di, forceall);
	}
}

uintptr
dma_getvar(dma_info_t *di, char *name)
{
	if (!strcmp(name, "&txavail"))
		return ((uintptr) &di->txavail);
	else {
		ASSERT(0);
	}
	return (0);
}

void
dma_txblock(dma_info_t *di)
{
	di->txavail = 0;
}

void
dma_txunblock(dma_info_t *di)
{
	di->txavail = di->ntxd - NTXDACTIVE(di->txin, di->txout) - 1;
}

uint
dma_txactive(dma_info_t *di)
{
	return (NTXDACTIVE(di->txin, di->txout));
}
	
void
dma_rxpiomode(dma32regs_t *regs)
{
	W_REG(&regs->control, RC_FM);
}

void
dma_txpioloopback(dma32regs_t *regs)
{
	OR_REG(&regs->control, XC_LE);
}




/*** 32 bits DMA non-inline functions ***/
static bool
dma32_alloc(dma_info_t *di, uint direction)
{
	uint size;
	uint ddlen;
	void *va;

	ddlen = sizeof (dma32dd_t);

	size = (direction == DMA_TX) ? (di->ntxd * ddlen) : (di->nrxd * ddlen);

	if (!ISALIGNED(DMA_CONSISTENT_ALIGN, D32RINGALIGN))
		size += D32RINGALIGN;


	if (direction == DMA_TX) {
		if ((va = DMA_ALLOC_CONSISTENT(di->osh, size, &di->txdpa)) == NULL) {
			DMA_ERROR(("%s: dma_attach: DMA_ALLOC_CONSISTENT(ntxd) failed\n", di->name));
			return FALSE;
		}

		di->txd32 = (dma32dd_t*) ROUNDUP((uintptr)va, D32RINGALIGN);
		di->txdalign = (uint)((int8*)di->txd32 - (int8*)va);
		di->txdpa += di->txdalign;
		di->txdalloc = size;
		ASSERT(ISALIGNED((uintptr)di->txd32, D32RINGALIGN));
	} else {
		if ((va = DMA_ALLOC_CONSISTENT(di->osh, size, &di->rxdpa)) == NULL) {
			DMA_ERROR(("%s: dma_attach: DMA_ALLOC_CONSISTENT(nrxd) failed\n", di->name));
			return FALSE;
		}
		di->rxd32 = (dma32dd_t*) ROUNDUP((uintptr)va, D32RINGALIGN);
		di->rxdalign = (uint)((int8*)di->rxd32 - (int8*)va);
		di->rxdpa += di->rxdalign;
		di->rxdalloc = size;
		ASSERT(ISALIGNED((uintptr)di->rxd32, D32RINGALIGN));
	}

	return TRUE;
}

static void 
dma32_txreset(dma_info_t *di)
{
	uint32 status;

	/* suspend tx DMA first */
	W_REG(&di->d32txregs->control, XC_SE);
	SPINWAIT((status = (R_REG(&di->d32txregs->status) & XS_XS_MASK)) != XS_XS_DISABLED &&
		 status != XS_XS_IDLE &&
		 status != XS_XS_STOPPED,
		 10000);

	W_REG(&di->d32txregs->control, 0);
	SPINWAIT((status = (R_REG(&di->d32txregs->status) & XS_XS_MASK)) != XS_XS_DISABLED,
		 10000);

	if (status != XS_XS_DISABLED) {
		DMA_ERROR(("%s: dma_txreset: dma cannot be stopped\n", di->name));
	}

	/* wait for the last transaction to complete */
	OSL_DELAY(300);
}

static void 
dma32_rxreset(dma_info_t *di)
{
	uint32 status;

	W_REG(&di->d32rxregs->control, 0);
	SPINWAIT((status = (R_REG(&di->d32rxregs->status) & RS_RS_MASK)) != RS_RS_DISABLED,
		 10000);

	if (status != RS_RS_DISABLED) {
		DMA_ERROR(("%s: dma_rxreset: dma cannot be stopped\n", di->name));
	}
}

static bool
dma32_txsuspendedidle(dma_info_t *di)
{
	if (!(R_REG(&di->d32txregs->control) & XC_SE))
		return 0;
	
	if ((R_REG(&di->d32txregs->status) & XS_XS_MASK) != XS_XS_IDLE)
		return 0;

	OSL_DELAY(2);
	return ((R_REG(&di->d32txregs->status) & XS_XS_MASK) == XS_XS_IDLE);
}

/*
 * supports full 32bit dma engine buffer addressing so
 * dma buffers can cross 4 Kbyte page boundaries.
 */
static int
dma32_txfast(dma_info_t *di, void *p0, uint32 coreflags)
{
	void *p, *next;
	uchar *data;
	uint len;
	uint txout;
	uint32 ctrl;
	uint32 pa;	

	DMA_TRACE(("%s: dma_txfast\n", di->name));

	txout = di->txout;
	ctrl = 0;

	/*
	 * Walk the chain of packet buffers
	 * allocating and initializing transmit descriptor entries.
	 */
	for (p = p0; p; p = next) {
		data = PKTDATA(di->osh, p);
		len = PKTLEN(di->osh, p);
		next = PKTNEXT(di->osh, p);

		/* return nonzero if out of tx descriptors */
		if (NEXTTXD(txout) == di->txin)
			goto outoftxd;

		if (len == 0)
			continue;

		/* get physical address of buffer start */
		pa = (uint32) DMA_MAP(di->osh, data, len, DMA_TX, p);

		/* build the descriptor control value */
		ctrl = len & CTRL_BC_MASK;

		ctrl |= coreflags;
		
		if (p == p0)
			ctrl |= CTRL_SOF;
		if (next == NULL)
			ctrl |= (CTRL_IOC | CTRL_EOF);
		if (txout == (di->ntxd - 1))
			ctrl |= CTRL_EOT;

		if (DMA64_ENAB(di)) {
			dma64_dd_upd(di, di->txd64, pa, txout, &ctrl, len);
		} else {
			dma32_dd_upd(di, di->txd32, pa, txout, &ctrl);
		}

		ASSERT(di->txp[txout] == NULL);

		txout = NEXTTXD(txout);
	}

	/* if last txd eof not set, fix it */
	if (!(ctrl & CTRL_EOF))
		W_SM(&di->txd32[PREVTXD(txout)].ctrl, BUS_SWAP32(ctrl | CTRL_IOC | CTRL_EOF));

	/* save the packet */
	di->txp[PREVTXD(txout)] = p0;

	/* bump the tx descriptor index */
	di->txout = txout;

	/* kick the chip */
	if (DMA64_ENAB(di)) {
		W_REG(&di->d64txregs->ptr, I2B(txout, dma64dd_t));
	} else {
		W_REG(&di->d32txregs->ptr, I2B(txout, dma32dd_t));
	}

	/* tx flow control */
	di->txavail = di->ntxd - NTXDACTIVE(di->txin, di->txout) - 1;

	return (0);

 outoftxd:
	DMA_ERROR(("%s: dma_txfast: out of txds\n", di->name));
	PKTFREE(di->osh, p0, TRUE);
	di->txavail = 0;
	di->hnddma.txnobuf++;
	return (-1);
}

static void*
dma32_getnexttxp(dma_info_t *di, bool forceall)
{
	uint start, end, i;
	void *txp;

	DMA_TRACE(("%s: dma_getnexttxp %s\n", di->name, forceall ? "all" : ""));

	txp = NULL;

	start = di->txin;
	if (forceall)
		end = di->txout;
	else
		end = B2I(R_REG(&di->d32txregs->status) & XS_CD_MASK, dma32dd_t);

	if ((start == 0) && (end > di->txout))
		goto bogus;

	for (i = start; i != end && !txp; i = NEXTTXD(i)) {
		DMA_UNMAP(di->osh, (BUS_SWAP32(R_SM(&di->txd32[i].addr)) - di->dataoffsetlow),
			  (BUS_SWAP32(R_SM(&di->txd32[i].ctrl)) & CTRL_BC_MASK), DMA_TX, di->txp[i]);

		W_SM(&di->txd32[i].addr, 0xdeadbeef);
		txp = di->txp[i];
		di->txp[i] = NULL;
	}

	di->txin = i;

	/* tx flow control */
	di->txavail = di->ntxd - NTXDACTIVE(di->txin, di->txout) - 1;

	return (txp);

bogus:
/*
	DMA_ERROR(("dma_getnexttxp: bogus curr: start %d end %d txout %d force %d\n",
		start, end, di->txout, forceall));
*/
	return (NULL);
}

static void *
dma32_getnextrxp(dma_info_t *di, bool forceall)
{
	uint i;
	void *rxp;

	/* if forcing, dma engine must be disabled */
	ASSERT(!forceall || !dma_rxenabled(di));

	i = di->rxin;

	/* return if no packets posted */
	if (i == di->rxout)
		return (NULL);

	/* ignore curr if forceall */
	if (!forceall && (i == B2I(R_REG(&di->d32rxregs->status) & RS_CD_MASK, dma32dd_t)))
		return (NULL);

	/* get the packet pointer that corresponds to the rx descriptor */
	rxp = di->rxp[i];
	ASSERT(rxp);
	di->rxp[i] = NULL;

	/* clear this packet from the descriptor ring */
	DMA_UNMAP(di->osh, (BUS_SWAP32(R_SM(&di->rxd32[i].addr)) - di->dataoffsetlow),
		  di->rxbufsize, DMA_RX, rxp);
	W_SM(&di->rxd32[i].addr, 0xdeadbeef);

	di->rxin = NEXTRXD(i);

	return (rxp);
}

static void
dma32_txrotate(di_t *di)
{
	uint ad;
	uint nactive;
	uint rot;
	uint old, new;
	uint32 w;
	uint first, last;

	ASSERT(dma_txsuspendedidle(di));

	nactive = dma_txactive(di);
	ad = B2I(((R_REG(&di->d32txregs->status) & XS_AD_MASK) >> XS_AD_SHIFT), dma32dd_t);
	rot = TXD(ad - di->txin);

	ASSERT(rot < di->ntxd);

	/* full-ring case is a lot harder - don't worry about this */
	if (rot >= (di->ntxd - nactive)) {
		DMA_ERROR(("%s: dma_txrotate: ring full - punt\n", di->name));
		return;
	}

	first = di->txin;
	last = PREVTXD(di->txout);

	/* move entries starting at last and moving backwards to first */
	for (old = last; old != PREVTXD(first); old = PREVTXD(old)) {
		new = TXD(old + rot);

		/*
		 * Move the tx dma descriptor.
		 * EOT is set only in the last entry in the ring.
		 */
		w = R_SM(&di->txd32[old].ctrl) & ~CTRL_EOT;
		if (new == (di->ntxd - 1))
			w |= CTRL_EOT;
		W_SM(&di->txd32[new].ctrl, w);
		W_SM(&di->txd32[new].addr, R_SM(&di->txd32[old].addr));

		/* zap the old tx dma descriptor address field */
		W_SM(&di->txd32[old].addr, 0xdeadbeef);

		/* move the corresponding txp[] entry */
		ASSERT(di->txp[new] == NULL);
		di->txp[new] = di->txp[old];
		di->txp[old] = NULL;
	}

	/* update txin and txout */
	di->txin = ad;
	di->txout = TXD(di->txout + rot);
	di->txavail = di->ntxd - NTXDACTIVE(di->txin, di->txout) - 1;

	/* kick the chip */
	W_REG(&di->d32txregs->ptr, I2B(di->txout, dma32dd_t));
}

/*** 64 bits DMA non-inline functions ***/

#ifdef BCMDMA64

static bool
dma64_alloc(dma_info_t *di, uint direction)
{
	uint size;
	uint ddlen;
	uint32 alignbytes;
	void *va;

	ddlen = sizeof (dma64dd_t);

	size = (direction == DMA_TX) ? (di->ntxd * ddlen) : (di->nrxd * ddlen);

	alignbytes = di->dma64align;

	if (!ISALIGNED(DMA_CONSISTENT_ALIGN, alignbytes))
		size += alignbytes;


	if (direction == DMA_TX) {
		if ((va = DMA_ALLOC_CONSISTENT(di->osh, size, &di->txdpa)) == NULL) {
			DMA_ERROR(("%s: dma_attach: DMA_ALLOC_CONSISTENT(ntxd) failed\n", di->name));
			return FALSE;
		}

		di->txd64 = (dma64dd_t*) ROUNDUP((uintptr)va, alignbytes);
		di->txdalign = (uint)((int8*)di->txd64 - (int8*)va);
		di->txdpa += di->txdalign;
		di->txdalloc = size;
		ASSERT(ISALIGNED((uintptr)di->txd64, alignbytes));
	} else {
		if ((va = DMA_ALLOC_CONSISTENT(di->osh, size, &di->rxdpa)) == NULL) {
			DMA_ERROR(("%s: dma_attach: DMA_ALLOC_CONSISTENT(nrxd) failed\n", di->name));
			return FALSE;
		}
		di->rxd64 = (dma64dd_t*) ROUNDUP((uintptr)va, alignbytes);
		di->rxdalign = (uint)((int8*)di->rxd64 - (int8*)va);
		di->rxdpa += di->rxdalign;
		di->rxdalloc = size;
		ASSERT(ISALIGNED((uintptr)di->rxd64, alignbytes));
	}

	return TRUE;
}

static void 
dma64_txreset(dma_info_t *di)
{
	uint32 status;

	/* suspend tx DMA first */
	W_REG(&di->d64txregs->control, D64_XC_SE);
	SPINWAIT((status = (R_REG(&di->d64txregs->status0) & D64_XS0_XS_MASK)) != D64_XS0_XS_DISABLED &&
		 status != D64_XS0_XS_IDLE &&
		 status != D64_XS0_XS_STOPPED,
		 10000);

	W_REG(&di->d64txregs->control, 0);
	SPINWAIT((status = (R_REG(&di->d64txregs->status0) & D64_XS0_XS_MASK)) != D64_XS0_XS_DISABLED,
		 10000);

	if (status != D64_XS0_XS_DISABLED) {
		DMA_ERROR(("%s: dma_txreset: dma cannot be stopped\n", di->name));
	}

	/* wait for the last transaction to complete */
	OSL_DELAY(300);
}

static void 
dma64_rxreset(dma_info_t *di)
{
	uint32 status;

	W_REG(&di->d64rxregs->control, 0);
	SPINWAIT((status = (R_REG(&di->d64rxregs->status0) & D64_RS0_RS_MASK)) != D64_RS0_RS_DISABLED,
		 10000);

	if (status != D64_RS0_RS_DISABLED) {
		DMA_ERROR(("%s: dma_rxreset: dma cannot be stopped\n", di->name));
	}
}

static bool
dma64_txsuspendedidle(dma_info_t *di)
{

	if (!(R_REG(&di->d64txregs->control) & D64_XC_SE))
		return 0;
	
	if ((R_REG(&di->d64txregs->status0) & D64_XS0_XS_MASK) == D64_XS0_XS_IDLE)
		return 1;

	return 0;
}

/*
 * supports full 32bit dma engine buffer addressing so
 * dma buffers can cross 4 Kbyte page boundaries.
 */
static int
dma64_txfast(dma_info_t *di, void *p0, uint32 coreflags)
{
	void *p, *next;
	uchar *data;
	uint len;
	uint txout;
	uint32 flags;
	uint32 pa;	

	DMA_TRACE(("%s: dma_txfast\n", di->name));

	txout = di->txout;
	flags = 0;

	/*
	 * Walk the chain of packet buffers
	 * allocating and initializing transmit descriptor entries.
	 */
	for (p = p0; p; p = next) {
		data = PKTDATA(di->osh, p);
		len = PKTLEN(di->osh, p);
		next = PKTNEXT(di->osh, p);

		/* return nonzero if out of tx descriptors */
		if (NEXTTXD(txout) == di->txin)
			goto outoftxd;

		if (len == 0)
			continue;

		/* get physical address of buffer start */
		pa = (uint32) DMA_MAP(di->osh, data, len, DMA_TX, p);

		flags = coreflags;
		
		if (p == p0)
			flags |= D64_CTRL1_SOF;
		if (next == NULL)
			flags |= (D64_CTRL1_IOC | D64_CTRL1_EOF);
		if (txout == (di->ntxd - 1))
			flags |= D64_CTRL1_EOT;

		dma64_dd_upd(di, di->txd64, pa, txout, &flags, len);

		ASSERT(di->txp[txout] == NULL);

		txout = NEXTTXD(txout);
	}

	/* if last txd eof not set, fix it */
	if (!(flags & D64_CTRL1_EOF))
		W_SM(&di->txd64[PREVTXD(txout)].ctrl1, BUS_SWAP32(flags | D64_CTRL1_IOC | D64_CTRL1_EOF));

	/* save the packet */
	di->txp[PREVTXD(txout)] = p0;

	/* bump the tx descriptor index */
	di->txout = txout;

	/* kick the chip */
	W_REG(&di->d64txregs->ptr, I2B(txout, dma64dd_t));

	/* tx flow control */
	di->txavail = di->ntxd - NTXDACTIVE(di->txin, di->txout) - 1;

	return (0);

outoftxd:
	DMA_ERROR(("%s: dma_txfast: out of txds\n", di->name));
	PKTFREE(di->osh, p0, TRUE);
	di->txavail = 0;
	di->hnddma.txnobuf++;
	return (-1);
}

static void*
dma64_getnexttxp(dma_info_t *di, bool forceall)
{
	uint start, end, i;
	void *txp;

	DMA_TRACE(("%s: dma_getnexttxp %s\n", di->name, forceall ? "all" : ""));

	txp = NULL;

	start = di->txin;
	if (forceall)
		end = di->txout;
	else
		end = B2I(R_REG(&di->d64txregs->status0) & D64_XS0_CD_MASK, dma64dd_t);

	if ((start == 0) && (end > di->txout))
		goto bogus;

	for (i = start; i != end && !txp; i = NEXTTXD(i)) {
		DMA_UNMAP(di->osh, (BUS_SWAP32(R_SM(&di->txd64[i].addrlow)) - di->dataoffsetlow),
			  (BUS_SWAP32(R_SM(&di->txd64[i].ctrl2)) & D64_CTRL2_BC_MASK), DMA_TX, di->txp[i]);

		W_SM(&di->txd64[i].addrlow, 0xdeadbeef);
		W_SM(&di->txd64[i].addrhigh, 0xdeadbeef);

		txp = di->txp[i];
		di->txp[i] = NULL;
	}

	di->txin = i;

	/* tx flow control */
	di->txavail = di->ntxd - NTXDACTIVE(di->txin, di->txout) - 1;

	return (txp);

bogus:
/*
	DMA_ERROR(("dma_getnexttxp: bogus curr: start %d end %d txout %d force %d\n",
		start, end, di->txout, forceall));
*/
	return (NULL);
}

static void *
dma64_getnextrxp(dma_info_t *di, bool forceall)
{
	uint i;
	void *rxp;

	/* if forcing, dma engine must be disabled */
	ASSERT(!forceall || !dma_rxenabled(di));

	i = di->rxin;

	/* return if no packets posted */
	if (i == di->rxout)
		return (NULL);

	/* ignore curr if forceall */
	if (!forceall && (i == B2I(R_REG(&di->d64rxregs->status0) & D64_RS0_CD_MASK, dma64dd_t)))
		return (NULL);

	/* get the packet pointer that corresponds to the rx descriptor */
	rxp = di->rxp[i];
	ASSERT(rxp);
	di->rxp[i] = NULL;

	/* clear this packet from the descriptor ring */
	DMA_UNMAP(di->osh, (BUS_SWAP32(R_SM(&di->rxd64[i].addrlow)) - di->dataoffsetlow),
		  di->rxbufsize, DMA_RX, rxp);

	W_SM(&di->rxd64[i].addrlow, 0xdeadbeef);
	W_SM(&di->rxd64[i].addrhigh, 0xdeadbeef);

	di->rxin = NEXTRXD(i);

	return (rxp);
}

static void
dma64_txrotate(di_t *di)
{
	uint ad;
	uint nactive;
	uint rot;
	uint old, new;
	uint32 w;
	uint first, last;

	ASSERT(dma_txsuspendedidle(di));

	nactive = dma_txactive(di);
	ad = B2I((R_REG(&di->d64txregs->status1) & D64_XS1_AD_MASK), dma64dd_t);
	rot = TXD(ad - di->txin);

	ASSERT(rot < di->ntxd);

	/* full-ring case is a lot harder - don't worry about this */
	if (rot >= (di->ntxd - nactive)) {
		DMA_ERROR(("%s: dma_txrotate: ring full - punt\n", di->name));
		return;
	}

	first = di->txin;
	last = PREVTXD(di->txout);

	/* move entries starting at last and moving backwards to first */
	for (old = last; old != PREVTXD(first); old = PREVTXD(old)) {
		new = TXD(old + rot);

		/*
		 * Move the tx dma descriptor.
		 * EOT is set only in the last entry in the ring.
		 */
		w = R_SM(&di->txd64[old].ctrl1) & ~D64_CTRL1_EOT;
		if (new == (di->ntxd - 1))
			w |= D64_CTRL1_EOT;
		W_SM(&di->txd64[new].ctrl1, w);

		w = R_SM(&di->txd64[old].ctrl2);
		W_SM(&di->txd64[new].ctrl2, w);

		W_SM(&di->txd64[new].addrlow, R_SM(&di->txd64[old].addrlow));
		W_SM(&di->txd64[new].addrhigh, R_SM(&di->txd64[old].addrhigh));

		/* zap the old tx dma descriptor address field */
		W_SM(&di->txd64[old].addrlow, 0xdeadbeef);
		W_SM(&di->txd64[old].addrhigh, 0xdeadbeef);

		/* move the corresponding txp[] entry */
		ASSERT(di->txp[new] == NULL);
		di->txp[new] = di->txp[old];
		di->txp[old] = NULL;
	}

	/* update txin and txout */
	di->txin = ad;
	di->txout = TXD(di->txout + rot);
	di->txavail = di->ntxd - NTXDACTIVE(di->txin, di->txout) - 1;

	/* kick the chip */
	W_REG(&di->d64txregs->ptr, I2B(di->txout, dma64dd_t));
}

#endif

