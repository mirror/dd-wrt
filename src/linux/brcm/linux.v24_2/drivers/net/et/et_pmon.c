/*
 * MIPS, PMON-specific polled-mode device driver for
 * Broadcom BCM44XX and BCM47XX 10/100 Mbps Ethernet Controller.
 *
 * Copyright 2004, Broadcom Corporation  
 * All Rights Reserved.                  
 *                                       
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;     
 * the contents of this file may not be disclosed to third parties, copied  
 * or duplicated in any form, in whole or in part, without the prior        
 * written permission of Broadcom Corporation.                              
 * $Id$
 */

#include <osl.h>
#include <epivers.h>
#include <bcmendian.h>
#include <proto/ethernet.h>
#include <ether.h>			/* pmon ether.h goes after ours */
#include <bcmenetmib.h>
#include <bcmenetrxh.h>
#include <bcmutils.h>
#include <bcmdevs.h>
#include <bcmnvram.h>
#include <et_dbg.h>
#include <etc.h>

struct dma_mem {
	uchar	desc[2][8192];
	uchar	buf[NBUFS][BUFSZ];
};

typedef struct et_info {
	etc_info_t	*etc;		/* pointer to common os-independent data */
	struct lbuf	*txlb;		/* pending tx lbuf */
	struct dma_mem	*dmamem;	/* malloc'd dma memory */
	struct lbuf	*lbfree;	/* lbuf freelist */
} et_info_t;

static et_info_t et_global;
static int initdone = 0;
static int ddi;

#define	ETHER_OK	(void*)1
#define	ETHER_FAIL	0

void
et_initmem(et_info_t *et)
{
	struct lbuf *lb;
	int i;
	static struct dma_mem *dma_space;

	/* malloc a chunk of memory */
	if (!initdone) {
		if ((dma_space = et->dmamem = MALLOC(NULL, sizeof (struct dma_mem) + 4096)) == NULL) {
			ET_ERROR(("et: et_initmem: out of memory, fail to get %d bytes!\n", (uint) sizeof (struct dma_mem)));
			return;
		}
		initdone = 1;
	} else
		et->dmamem = dma_space;

	/* round up to 4Kbyte aligned */
	et->dmamem = (struct dma_mem*) OSL_UNCACHED( ROUNDUP((uintptr)et->dmamem, 4096) );

	/* init lbuf freelist */
	for (i = 0; i < NBUFS; i++) {
		lb = (struct lbuf*) et->dmamem->buf[i];
		lb->next = et->lbfree;
		et->lbfree = lb;
	}
}

void*
et_dma_alloc_consistent(void *dev, uint size, ulong *pap)
{
	et_info_t *et;
	uchar *p;

	et = (et_info_t*) dev;

	/* we support only the two dma descriptor rings */
	ASSERT(size <= 8192);
	ASSERT(ddi < 2);
	ASSERT(et->dmamem);

	p = et->dmamem->desc[ddi];
	ddi++;

	*pap = (ulong) log2phy((uint32)p);

	return ((void*)OSL_UNCACHED(p));
}

void*
et_pktget(void *dev, uint len, bool send)
{
	et_info_t *et;
	struct lbuf *lb;

	et = (et_info_t*)dev;

	ASSERT(len <= BUFSZ);

	if ((lb = et->lbfree) == NULL) {
		ET_ERROR(("et_pktget: out of buffers\n"));
		return (NULL);
	}

	et->lbfree = lb->next;

	ASSERT(ISALIGNED((uintptr)lb, BUFSZ));

	lb->next = NULL;
	lb->buf = (uchar*) &lb[1];
	lb->len = len;

	return ((void*)lb);
}

void
et_pktfree(void *dev, struct lbuf *lb, bool send)
{
	et_info_t *et;

	et = (et_info_t*)dev;

	ASSERT(ISALIGNED((uintptr)lb, BUFSZ));
	ASSERT(lb->next == NULL);
	ASSERT(lb->len <= BUFSZ);
	ASSERT(lb->buf == (uchar*) &lb[1]);
	ASSERT(et == &et_global);

	lb->next = et->lbfree;
	lb->buf = (uchar*) 0xdeadbeef;
	lb->len = 0;

	et->lbfree = lb;
}

/*
 * Trivial shim functions.
 */

void
et_init(et_info_t *et)
{
	ET_TRACE(("et%d: et_init\n", et->etc->unit));
	etc_reset(et->etc);
	etc_init(et->etc);
}

void
et_reset(et_info_t *et)
{
	ET_TRACE(("et%d: et_reset\n", et->etc->unit));

	etc_reset(et->etc);

	/* free any pending tx packet */
	if (et->txlb) {
		PKTFREE(et, et->txlb, TRUE);
		et->txlb = NULL;
	}
}

void
et_link_up(et_info_t *et)
{
	ET_ERROR(("et%d: link up (%d%s)\n",
		et->etc->unit, et->etc->speed, (et->etc->duplex? "FD" : "HD")));
}

void
et_link_down(et_info_t *et)
{
	ET_ERROR(("et%d: link down\n", et->etc->unit));
}

void
et_up(et_info_t *et)
{
	ET_TRACE(("et%d: et_up\n", et->etc->unit));
	etc_up(et->etc);
}

void
et_down(et_info_t *et, uint reset)
{
	ET_TRACE(("et%d: et_down\n", et->etc->unit));
	etc_down(et->etc, reset);
}

void
et_dump(et_info_t *et, uchar *buf, uint len)
{
}

/* check for chip errors - return nonzero if error */
int
et_errchk(et_info_t *et)
{
	if (((*et->etc->chops->getintrevents)(et->etc->ch)) & INTR_ERROR)
		if ((*et->etc->chops->errors)(et->etc->ch)) {
			et_init(et);
			return (-1);
		}

	return (0);
}

/*
 * et_recv()
 *
 * Description: Returns next received frame.
 * Arguments: >=1500 byte buffer to receive frame
 * Returns: Length of received frame or 0 if no data available
 */
uint
et_recv(uchar *buf)
{
	et_info_t *et;
	struct lbuf *lb;
	uint len;
	uint events;

	et = &et_global;

	events = (*et->etc->chops->getintrevents)(et->etc->ch);

	ET_TRACE(("et%d: et_recv: events 0x%x\n", et->etc->unit, events));

	/* check for any errors */
	if (et_errchk(et))
		return (0);

	if (events & INTR_RX) {
		lb = (struct lbuf*) (*et->etc->chops->rx)(et->etc->ch);
		if (lb == NULL)
			return (0);

		/* copy received frame from lbuf to calling buf */
		len = lb->len;
		bcopy((uchar*) OSL_UNCACHED(lb->buf), buf, len);

		PKTFREE(et, lb, FALSE);

		/* post more rx bufs */
		(*et->etc->chops->rxfill)(et->etc->ch);

		return (len);
	}

	return (0);
}

/*
 * et_send()
 *
 * Description: Transmit one frame.
 * Arguments: <=1500 byte buffer containing frame to send, length of frame in bytes
 * Returns: 0 on success
 */
void
et_send(et_info_t *et, struct lbuf *lb)
{
	ET_TRACE(("et%d: et_send; len %d\n", et->etc->unit, lb->len));

	ASSERT(*et->etc->txavail > 0);

	ET_PRHDR("tx", (struct ether_header*) lb->buf, lb->len, et->etc->unit);
	ET_PRPKT("txpkt", lb->buf, lb->len, et->etc->unit);

	/* transmit the frame */
	(*et->etc->chops->tx)(et->etc->ch, (void*)lb);

	/* wait for frame to complete */
	while (!((*et->etc->chops->getintrevents)(et->etc->ch) & INTR_TX));

	/* reclaim any completed tx frames */
	(*et->etc->chops->txreclaim)(et->etc->ch, FALSE);
}

static bool
linkstate(etc_info_t *etc)
{
	etc_watchdog(etc);
	return (etc->linkstate);
}

/*
 * et_probe()
 *
 * Description: Probes for, allocates, and initializes hardware
 * Arguments: Address of hardware registers
 * Returns: 0 on success, nonzero error on failure.
 */
int
et_probe(et_info_t *et, uchar *regpa, uchar *etheraddr)
{
	char name[128];
	int unit=0;

	if (nvram_match("et_port", "1")) 
		unit = 1;

	ET_TRACE(("et: et_probe: regpa 0x%x\n", (uint)regpa));

	bzero((char*)et, sizeof (et_info_t));

	et_initmem(et);

	/* common load-time initialization */
	if ((et->etc = etc_attach(et, VENDOR_BROADCOM, BCM47XX_ENET_ID, unit, (void*)et, regpa)) == NULL) {
		ET_ERROR(("et%d: etc_attach failed\n", unit));
		return (-2);
	}

	/* reset ddi */
	ddi = 0;

	/* this is a polling driver - the chip intmask stays zero */
	(*et->etc->chops->intrsoff)(et->etc->ch);

	/* local ether address override */
	if (etheraddr)
		bcopy(&et->etc->cur_etheraddr, etheraddr, ETHER_ADDR_LEN);

	/* hello world */
	(*et->etc->chops->longname)(et->etc->ch, name, sizeof (name));
	printf("et%d: %s %s\n", unit, name, EPI_VERSION_STR);

	et_up(et);

	/* spinwait for 2sec for link to be established  */
	SPINWAIT((linkstate(et->etc) == FALSE), 2000000);
	if (!linkstate(et->etc))
	{
		ET_ERROR(("Failed to establish link\n"));
		initdone = 1;
		ddi = 0;
		return(-1);
	}

	return (0);
}

void
et_assert(char *exp, char *file, uint line)
{
	printf("assertion \"%s\" failed: file \"%s\", line %d\n", exp, file, line);
}

/*
 * 47XX-specific shared mdc/mdio contortion:
 * Find the et associated with the same chip as <et>
 * and coreunit matching <coreunit>.
 *
 * Having support for only a single et (et_global),
 * means that phy addresses _have_ to be local.
 */
void*
et_phyfind(et_info_t *et, uint coreunit)
{
	ASSERT(0);
	return (NULL);
}

/* shared phy read entry point */
uint16
et_phyrd(et_info_t *et, uint phyaddr, uint reg)
{
	ASSERT(0);
	return (NULL);
}

/* shared phy write entry point */
void
et_phywr(et_info_t *et, uint phyaddr, uint reg, uint16 val)
{
	ASSERT(0);
}

/* main entry point for the driver */
volatile void*
bcmet(int op, void *vp1, void *vp2)
{
	et_info_t *et;
	etc_info_t *etc;
	uint events;
	struct lbuf *lb;
	bcmenetrxh_t *rxh;
	int len;
	char *data;

	et = &et_global;
	etc = et->etc;

	if (initdone && etc->up) {
		if (op == ETHER_INIT) {
			(*etc->chops->reset)(etc->ch);
			(*etc->chops->rxreclaim)(etc->ch);
			etc_detach(etc);
		}
		else
			et_errchk(et);
	}

	switch (op) {
	case ETHER_RXRDY:	/* int ether_driver(ETHER_RXRDY, void, void) */
		events = (*etc->chops->getintrevents)(etc->ch);
		return ((events & INTR_RX)? ETHER_OK: ETHER_FAIL);

	case ETHER_GETRXREC:	/* RXREC *ether_driver(ETHER_GETRXREC, void, void) */
		events = (*etc->chops->getintrevents)(etc->ch);
		if ((events & INTR_RX) == 0)
			return (ETHER_FAIL);

		if ((lb = (struct lbuf*) (*etc->chops->rx)(etc->ch)) == NULL)
			return (ETHER_FAIL);

		rxh = (bcmenetrxh_t*) lb->buf;
		if (ltoh16(rxh->flags) & (RXF_NO | RXF_RXER | RXF_CRC | RXF_OV)) {
			ET_ERROR(("et%d: rx error: flags 0x%x\n", etc->unit, rxh->flags));
			PKTFREE(et, lb, FALSE);
			return (ETHER_FAIL);
		}

		ET_TRACE(("et%d: ETHER_GETRXREC: 0x%x\n", et->etc->unit, (uint32)lb));

		return ((void*)lb);

	case ETHER_GETRBA:	/* char *ether_driver(ETHER_GETRBA,RXrec *vp1, int *len) */
		lb = (struct lbuf*) vp1;
		rxh = (bcmenetrxh_t*)lb->buf;
		len = ltoh16(rxh->len);
		data = &lb->buf[HWRXOFF];
		*(int*)vp2 = len;
		ET_TRACE(("et%d: ETHER_GETRBA: len 0x%x\n", etc->unit, len));
		ET_PRHDR("rx", (struct ether_header*) data, len, etc->unit);
		ET_PRPKT("rxpkt", data, len, etc->unit);
		return (data);

	case ETHER_RXDONE:	/* int ether_driver(ETHER_RXDONE, RXrec *vp1, void) */
		lb = (struct lbuf*) vp1;
		PKTFREE(et, lb, FALSE);
		(*etc->chops->rxfill)(etc->ch);
		ET_TRACE(("et%d: ETHER_RXDONE\n", etc->unit));
		return (ETHER_OK);

	case ETHER_GETTBA:	/* char *ether_driver(ETHER_GETTBA, void, int *len) */
		/* alloc send lbuf */
		if ((lb = (struct lbuf *) PKTGET(et, *(int*)vp2, TRUE)) == NULL) {
			ET_ERROR(("et%d: ETHER_GETTBA: pktget failed\n", etc->unit));
			return (NULL);
		}
		ASSERT(et->txlb == NULL);
		et->txlb = (void*) lb;
		ET_TRACE(("et%d: ETHER_GETTBA: pkt 0x%x\n", etc->unit, (uint)lb));
		return ((void*) lb->buf);

	case ETHER_TBARDY:	/* int ether_driver(ETHER_TBRDY, void, void) */
		ASSERT(et->txlb);
		lb = et->txlb;
		et->txlb = NULL;
		et_send(et, lb);
		ET_TRACE(("et%d: ETHER_TBARDY\n", etc->unit));
		return (ETHER_OK);

	case ETHER_MAC_ADDR: /* not implemented */
		return (ETHER_OK);

	case ETHER_INIT:	/* int ether_driver(ETHER_INIT, Uchar *macAddr, Uchar *REG_BASE) */
		if (et_probe(et, (uchar*)vp2, (uchar*)vp1) != 0)
			return (0);
	return (ETHER_OK);

	default:
		return (ETHER_FAIL);

	}

	return (0);
}
