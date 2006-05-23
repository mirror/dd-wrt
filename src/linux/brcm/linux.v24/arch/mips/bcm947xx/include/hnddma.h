/*
 * Generic Broadcom Home Networking Division (HND) DMA engine SW interface
 * This supports the following chips: BCM42xx, 44xx, 47xx .
 *
 * Copyright 2005, Broadcom Corporation      
 * All Rights Reserved.      
 *       
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY      
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM      
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS      
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.      
 * $Id$
 */

#ifndef	_hnddma_h_
#define	_hnddma_h_

/* export structure */
typedef volatile struct {
	/* rx error counters */
	uint		rxgiants;	/* rx giant frames */
	uint		rxnobuf;	/* rx out of dma descriptors */
	/* tx error counters */
	uint		txnobuf;	/* tx out of dma descriptors */
} hnddma_t;

#ifndef di_t
#define	di_t	void
#endif

#ifndef osl_t 
#define osl_t void
#endif

/* externs */
extern void * dma_attach(osl_t *osh, char *name, sb_t *sbh, void *dmaregstx, void *dmaregsrx, 
			 uint ntxd, uint nrxd, uint rxbufsize, uint nrxpost, uint rxoffset, uint *msg_level);
extern void dma_detach(di_t *di);
extern void dma_txreset(di_t *di);
extern void dma_rxreset(di_t *di);
extern void dma_txinit(di_t *di);
extern bool dma_txenabled(di_t *di);
extern void dma_rxinit(di_t *di);
extern void dma_rxenable(di_t *di);
extern bool dma_rxenabled(di_t *di);
extern void dma_txsuspend(di_t *di);
extern void dma_txresume(di_t *di);
extern bool dma_txsuspended(di_t *di);
extern bool dma_txsuspendedidle(di_t *di);
extern bool dma_txstopped(di_t *di);
extern bool dma_rxstopped(di_t *di);
extern int dma_txfast(di_t *di, void *p, uint32 coreflags);
extern void dma_fifoloopbackenable(di_t *di);
extern void *dma_rx(di_t *di);
extern void dma_rxfill(di_t *di);
extern void dma_txreclaim(di_t *di, bool forceall);
extern void dma_rxreclaim(di_t *di);
extern uintptr dma_getvar(di_t *di, char *name);
extern void *dma_getnexttxp(di_t *di, bool forceall);
extern void *dma_peeknexttxp(di_t *di);
extern void *dma_getnextrxp(di_t *di, bool forceall);
extern void dma_txblock(di_t *di);
extern void dma_txunblock(di_t *di);
extern uint dma_txactive(di_t *di);
extern void dma_txrotate(di_t *di);

extern void dma_rxpiomode(dma32regs_t *);
extern void dma_txpioloopback(dma32regs_t *);


#endif	/* _hnddma_h_ */
