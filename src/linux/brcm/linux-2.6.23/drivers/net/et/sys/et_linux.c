/*
 * Linux device driver for
 * Broadcom BCM47XX 10/100/1000 Mbps Ethernet Controller
 *
 * Copyright (C) 2009, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: et_linux.c,v 1.122.6.14 2010/06/20 19:39:07 Exp $
 */

#define __UNDEF_NO_VERSION__

#include <typedefs.h>

#include <linux/module.h>
#include <linuxver.h>
#include <bcmdefs.h>
#include <osl.h>

#include <linux/types.h>
#include <linux/errno.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/sockios.h>
#ifdef SIOCETHTOOL
#include <linux/ethtool.h>
#endif /* SIOCETHTOOL */
#include <linux/ip.h>

#include <asm/system.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/pgtable.h>
#include <asm/uaccess.h>

#include <epivers.h>
#include <bcmendian.h>
#include <bcmdefs.h>
#include <proto/ethernet.h>
#include <proto/vlan.h>
#include <bcmdevs.h>
#include <bcmenetmib.h>
#include <bcmgmacmib.h>
#include <bcmenetrxh.h>
#include <bcmenetphy.h>
#include <etioctl.h>
#include <bcmutils.h>
#include <pcicfg.h>
#include <et_dbg.h>
#include <hndsoc.h>
#include <bcmgmacrxh.h>
#include <etc.h>
#ifdef HNDCTF
#include <ctf/hndctf.h>
#endif /* HNDCTF */

MODULE_LICENSE("Proprietary");

typedef struct et_info {
	etc_info_t	*etc;		/* pointer to common os-independent data */
	struct net_device *dev;		/* backpoint to device */
	struct pci_dev *pdev;		/* backpoint to pci_dev */
	void		*osh;		/* pointer to os handle */
#ifdef HNDCTF
	ctf_t		*cih;		/* ctf instance handle */
#endif /* HNDCTF */
	spinlock_t	lock;		/* per-device perimeter lock */
	struct sk_buff_head txq[NUMTXQ];	/* send queue */
	void *regsva;			/* opaque chip registers virtual address */
	struct timer_list timer;	/* one second watchdog timer */
	struct net_device_stats stats;	/* stat counter reporting structure */
	int events;			/* bit channel between isr and dpc */
	struct et_info *next;		/* pointer to next et_info_t in chain */
#ifndef NAPI_POLL
	struct tasklet_struct tasklet;	/* dpc tasklet */
#endif /* NAPI_POLL */
	bool resched;			/* dpc was rescheduled */
} et_info_t;

static int et_found = 0;
static et_info_t *et_list = NULL;

/* defines */
#define	DATAHIWAT	50		/* data msg txq hiwat mark */

#define	ET_INFO(dev)	(et_info_t *)((dev)->priv)

#define ET_LOCK(et)	spin_lock_bh(&(et)->lock)
#define ET_UNLOCK(et)	spin_unlock_bh(&(et)->lock)

#define INT_LOCK(flags)		local_irq_save(flags)
#define INT_UNLOCK(flags)	local_irq_restore(flags)

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 4, 5)
#error Linux version must be newer than 2.4.5
#endif	/* LINUX_VERSION_CODE <= KERNEL_VERSION(2, 4, 5) */

/* prototypes called by etc.c */
void et_init(et_info_t *et, uint options);
void et_reset(et_info_t *et);
void et_link_up(et_info_t *et);
void et_link_down(et_info_t *et);
void et_up(et_info_t *et);
void et_down(et_info_t *et, int reset);
void et_dump(et_info_t *et, struct bcmstrbuf *b);

/* local prototypes */
static void et_free(et_info_t *et);
static int et_open(struct net_device *dev);
static int et_close(struct net_device *dev);
static int et_start(struct sk_buff *skb, struct net_device *dev);
static void et_sendnext(et_info_t *et);
static struct net_device_stats *et_get_stats(struct net_device *dev);
static int et_set_mac_address(struct net_device *dev, void *addr);
static void et_set_multicast_list(struct net_device *dev);
static void et_watchdog(ulong data);
static int et_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 20)
static irqreturn_t et_isr(int irq, void *dev_id);
#else
static irqreturn_t et_isr(int irq, void *dev_id, struct pt_regs *ptregs);
#endif
#ifdef NAPI_POLL
static int et_poll(struct net_device *dev, int *budget);
#else /* NAPI_POLL */
static void et_dpc(ulong data);
#endif /* NAPI_POLL */
static void et_sendup(et_info_t *et, struct sk_buff *skb);

/* recognized PCI IDs */
static struct pci_device_id et_id_table[] __devinitdata = {
	{ vendor: PCI_ANY_ID,
	device: PCI_ANY_ID,
	subvendor: PCI_ANY_ID,
	subdevice: PCI_ANY_ID,
	class: PCI_CLASS_NETWORK_OTHER << 8,
	class_mask: 0xffff00,
	driver_data: 0,
	},
	{ vendor: PCI_ANY_ID,
	device: PCI_ANY_ID,
	subvendor: PCI_ANY_ID,
	subdevice: PCI_ANY_ID,
	class: PCI_CLASS_NETWORK_ETHERNET << 8,
	class_mask: 0xffff00,
	driver_data: 0,
	},
	{ 0, }
};
MODULE_DEVICE_TABLE(pci, et_id_table);


#ifdef HNDCTF
static void
et_ctf_detach(ctf_t *ci, void *arg)
{
	et_info_t *et = (et_info_t *)arg;

	et->cih = NULL;

#ifdef CTFPOOL
	/* free the buffers in fast pool */
	osl_ctfpool_cleanup(et->osh);
#endif /* CTFPOOL */

	return;
}
#endif /* HNDCTF */

static int __devinit
et_probe(struct pci_dev *pdev, const struct pci_device_id *ent)
{
	struct net_device *dev;
	et_info_t *et;
	osl_t *osh;
	char name[128];
	int i, unit = et_found, err;

	ET_TRACE(("et%d: et_probe: bus %d slot %d func %d irq %d\n", unit,
	          pdev->bus->number, PCI_SLOT(pdev->devfn), PCI_FUNC(pdev->devfn), pdev->irq));

	if (!etc_chipmatch(pdev->vendor, pdev->device))
		return -ENODEV;

	osh = osl_attach(pdev, PCI_BUS, FALSE);
	ASSERT(osh);

	pci_set_master(pdev);
	if ((err = pci_enable_device(pdev)) != 0) {
		ET_ERROR(("et%d: et_probe: pci_enable_device() failed\n", unit));
		osl_detach(osh);
		return err;
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0)
	if ((dev = alloc_etherdev(0)) == NULL) {
		ET_ERROR(("et%d: et_probe: alloc_etherdev() failed\n", unit));
		osl_detach(osh);
		return -ENOMEM;
	}
#else
	if (!(dev = (struct net_device *) MALLOC(osh, sizeof(struct net_device)))) {
		ET_ERROR(("et%d: et_probe: out of memory, malloced %d bytes\n", unit,
		          MALLOCED(osh)));
		osl_detach(osh);
		return -ENOMEM;
	}
	bzero(dev, sizeof(struct net_device));

	if (!init_etherdev(dev, 0)) {
		ET_ERROR(("et%d: et_probe: init_etherdev() failed\n", unit));
		MFREE(osh, dev, sizeof(struct net_device));
		osl_detach(osh);
		return -ENOMEM;
	}
#endif /* >= 2.6.0 */

	/* allocate private info */
	if ((et = (et_info_t *)MALLOC(osh, sizeof(et_info_t))) == NULL) {
		ET_ERROR(("et%d: et_probe: out of memory, malloced %d bytes\n", unit,
		          MALLOCED(osh)));
		MFREE(osh, dev, sizeof(et_info_t));
		osl_detach(osh);
		return -ENOMEM;
	}
	bzero(et, sizeof(et_info_t));
	dev->priv = (void *)et;
	et->dev = dev;
	et->pdev = pdev;
	et->osh = osh;
	pci_set_drvdata(pdev, et);

	/* map chip registers (47xx: and sprom) */
	dev->base_addr = pci_resource_start(pdev, 0);
	if ((et->regsva = ioremap_nocache(dev->base_addr, PCI_BAR0_WINSZ)) == NULL) {
		ET_ERROR(("et%d: ioremap() failed\n", unit));
		goto fail;
	}

	spin_lock_init(&et->lock);

	for (i = 0; i < NUMTXQ; i++)
		skb_queue_head_init(&et->txq[i]);

	/* common load-time initialization */
	et->etc = etc_attach((void *)et, pdev->vendor, pdev->device, unit, osh, et->regsva);
	if (et->etc == NULL) {
		ET_ERROR(("et%d: etc_attach() failed\n", unit));
		goto fail;
	}

#ifdef HNDCTF
	et->cih = ctf_attach(osh, dev->name, &et_msg_level, et_ctf_detach, et);

	if ((ctf_dev_register(et->cih, dev, FALSE) != BCME_OK) ||
	    (ctf_enable(et->cih, dev, TRUE) != BCME_OK)) {
		ET_ERROR(("et%d: ctf_dev_register() failed\n", unit));
		goto fail;
	}
#endif /* HNDCTF */

#ifdef CTFPOOL
	/* create ctf packet pool with specified number of buffers */
	if (CTF_ENAB(et->cih) && (num_physpages >= 8192) &&
	    (osl_ctfpool_init(osh, CTFPOOLSZ, RXBUFSZ+BCMEXTRAHDROOM) < 0)) {
		ET_ERROR(("et%d: chipattach: ctfpool alloc/init failed\n", unit));
		goto fail;
	}
#endif /* CTFPOOL */

	bcopy(&et->etc->cur_etheraddr, dev->dev_addr, ETHER_ADDR_LEN);

	/* init 1 second watchdog timer */
	init_timer(&et->timer);
	et->timer.data = (ulong)dev;
	et->timer.function = et_watchdog;

#ifndef NAPI_POLL
	/* setup the bottom half handler */
	tasklet_init(&et->tasklet, et_dpc, (ulong)et);
#endif /* NAPI_POLL */

	/* register our interrupt handler */
	if (request_irq(pdev->irq, et_isr, IRQF_SHARED, dev->name, et)) {
		ET_ERROR(("et%d: request_irq() failed\n", unit));
		goto fail;
	}
	dev->irq = pdev->irq;

	/* add us to the global linked list */
	et->next = et_list;
	et_list = et;

	/* lastly, enable our entry points */
	dev->open = et_open;
	dev->stop = et_close;
	dev->hard_start_xmit = et_start;
	dev->get_stats = et_get_stats;
	dev->set_mac_address = et_set_mac_address;
	dev->set_multicast_list = et_set_multicast_list;
	dev->do_ioctl = et_ioctl;
#ifdef NAPI_POLL
	dev->poll = et_poll;
	dev->weight = (ET_GMAC(et->etc) ? 64 : 32);
#endif /* NAPI_POLL */

	if (register_netdev(dev)) {
		ET_ERROR(("et%d: register_netdev() failed\n", unit));
		goto fail;
	}

	et_found++;

	/* print hello string */
	(*et->etc->chops->longname)(et->etc->ch, name, sizeof(name));
	printf("%s: %s %s\n", dev->name, name, EPI_VERSION_STR);

	return (0);

fail:
	et_free(et);
	return (-ENODEV);
}

static int
et_suspend(struct pci_dev *pdev, DRV_SUSPEND_STATE_TYPE state)
{
	et_info_t *et;

	if ((et = (et_info_t *) pci_get_drvdata(pdev))) {
		netif_device_detach(et->dev);
		ET_LOCK(et);
		et_down(et, 1);
		ET_UNLOCK(et);
	}

	return 0;
}

static int
et_resume(struct pci_dev *pdev)
{
	et_info_t *et;

	if ((et = (et_info_t *) pci_get_drvdata(pdev))) {
		ET_LOCK(et);
		et_up(et);
		ET_UNLOCK(et);
		netif_device_attach(et->dev);
	}

	return 0;
}

/* Compatibility routines */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 4, 6)
static void
_et_suspend(struct pci_dev *pdev)
{
	et_suspend(pdev, 0);
}

static void
_et_resume(struct pci_dev *pdev)
{
	et_resume(pdev);
}
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2, 4, 6) */

static void __devexit
et_remove(struct pci_dev *pdev)
{
	et_info_t *et;

	if (!etc_chipmatch(pdev->vendor, pdev->device))
		return;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 12)
	et_suspend(pdev, 0);
#else
	et_suspend(pdev, PMSG_SUSPEND);
#endif

	if ((et = (et_info_t *) pci_get_drvdata(pdev))) {
		et_free(et);
		pci_set_drvdata(pdev, NULL);
	}
}

static struct pci_driver et_pci_driver = {
	name:		"et",
	probe:		et_probe,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 4, 6)
	suspend:	_et_suspend,
	resume:		_et_resume,
#else
	suspend:	et_suspend,
	resume:		et_resume,
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2, 4, 6) */
	remove:		__devexit_p(et_remove),
	id_table:	et_id_table,
	};

static int __init
et_module_init(void)
{

	return pci_module_init(&et_pci_driver);
}

static void __exit
et_module_exit(void)
{
	pci_unregister_driver(&et_pci_driver);
}

module_init(et_module_init);
module_exit(et_module_exit);

static void
et_free(et_info_t *et)
{
	et_info_t **prev;
	osl_t *osh;

	if (et == NULL)
		return;

	ET_TRACE(("et: et_free\n"));

	if (et->dev && et->dev->irq)
		free_irq(et->dev->irq, et);

#ifdef HNDCTF
	if (et->cih)
		ctf_dev_unregister(et->cih, et->dev);
#endif /* HNDCTF */

	if (et->dev) {
		unregister_netdev(et->dev);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0)
		free_netdev(et->dev);
#else
		MFREE(et->osh, et->dev, sizeof(struct net_device));
#endif
		et->dev = NULL;
	}

#ifdef CTFPOOL
	/* free the buffers in fast pool */
	osl_ctfpool_cleanup(et->osh);
#endif /* CTFPOOL */

#ifdef HNDCTF
	/* free ctf resources */
	if (et->cih)
		ctf_detach(et->cih);
#endif /* HNDCTF */

	/* free common resources */
	if (et->etc) {
		etc_detach(et->etc);
		et->etc = NULL;
	}

	/*
	 * unregister_netdev() calls get_stats() which may read chip registers
	 * so we cannot unmap the chip registers until after calling unregister_netdev() .
	 */
	if (et->regsva) {
		iounmap((void *)et->regsva);
		et->regsva = NULL;
	}

	/* remove us from the global linked list */
	for (prev = &et_list; *prev; prev = &(*prev)->next)
		if (*prev == et) {
			*prev = et->next;
			break;
		}

	osh = et->osh;
	MFREE(et->osh, et, sizeof(et_info_t));

	if (MALLOCED(osh))
		printf("Memory leak of bytes %d\n", MALLOCED(osh));
	ASSERT(MALLOCED(osh) == 0);

	osl_detach(osh);
}

static int
et_open(struct net_device *dev)
{
	et_info_t *et;

	et = ET_INFO(dev);

	ET_TRACE(("et%d: et_open\n", et->etc->unit));

	et->etc->promisc = (dev->flags & IFF_PROMISC)? TRUE: FALSE;
	et->etc->allmulti = (dev->flags & IFF_ALLMULTI)? TRUE: et->etc->promisc;

	ET_LOCK(et);
	et_up(et);
	ET_UNLOCK(et);

	OLD_MOD_INC_USE_COUNT;

	return (0);
}

static int
et_close(struct net_device *dev)
{
	et_info_t *et;

	et = ET_INFO(dev);

	ET_TRACE(("et%d: et_close\n", et->etc->unit));

	et->etc->promisc = FALSE;
	et->etc->allmulti = FALSE;

	ET_LOCK(et);
	et_down(et, 1);
	ET_UNLOCK(et);

	OLD_MOD_DEC_USE_COUNT;

	return (0);
}

/*
 * Yeah, queueing the packets on a tx queue instead of throwing them
 * directly into the descriptor ring in the case of dma is kinda lame,
 * but this results in a unified transmit path for both dma and pio
 * and localizes/simplifies the netif_*_queue semantics, too.
 */
static int BCMFASTPATH
et_start(struct sk_buff *skb, struct net_device *dev)
{
	et_info_t *et;
	uint32 q = 0;

	et = ET_INFO(dev);

	if (ET_GMAC(et->etc) && (et->etc->qos))
		q = etc_up2tc(PKTPRIO(skb));

	ET_TRACE(("et%d: et_start: len %d\n", et->etc->unit, skb->len));
	ET_LOG("et%d: et_start: len %d", et->etc->unit, skb->len);

	ET_LOCK(et);

	/* put it on the tx queue and call sendnext */
	__skb_queue_tail(&et->txq[q], skb);
	et->etc->txq_state |= (1 << q);
	et_sendnext(et);

	ET_UNLOCK(et);

	ET_LOG("et%d: et_start ret\n", et->etc->unit, 0);

	return (0);
}

static void BCMFASTPATH
et_sendnext(et_info_t *et)
{
	etc_info_t *etc;
	struct sk_buff *skb;
	void *p;
	uint32 priq = TX_Q0;

	etc = et->etc;

	ET_TRACE(("et%d: et_sendnext\n", etc->unit));
	ET_LOG("et%d: et_sendnext", etc->unit, 0);

	/* dequeue packets from highest priority queue and send */
	priq = etc_priq(etc->txq_state);
	ET_TRACE(("et%d: txq_state %x priq %d txavail %d\n",
	          etc->unit, etc->txq_state, priq, *(uint *)etc->txavail[priq]));
	while (etc->txq_state != 0) {
		if (skb_peek(&et->txq[priq]) == NULL) {
			etc->txq_state &= ~(1 << priq);
			priq = etc_priq(etc->txq_state);
			continue;
		}

#ifdef DMA
		/* current highest priority dma queue is full */
		if (*(uint *)(etc->txavail[priq]) == 0)
#else /* DMA */
		if (etc->pioactive != NULL)
#endif /* DMA */
			break;

		skb = __skb_dequeue(&et->txq[priq]);

		ET_PRHDR("tx", (struct ether_header *)skb->data, skb->len, etc->unit);
		ET_PRPKT("txpkt", skb->data, skb->len, etc->unit);

		/* Convert the packet. */
		if ((p = PKTFRMNATIVE(et->osh, skb)) == NULL) {
			PKTFREE(etc->osh, skb, TRUE);
			return;
		}

		(*etc->chops->tx)(etc->ch, p);

		etc->txframe++;
		etc->txbyte += skb->len;
	}

	/* no flow control when qos is enabled */
	if (!et->etc->qos) {
		/* stop the queue whenever txq fills */
		if ((skb_queue_len(&et->txq[TX_Q0]) > DATAHIWAT) && !netif_queue_stopped(et->dev))
			netif_stop_queue(et->dev);
		else if (netif_queue_stopped(et->dev) &&
		         (skb_queue_len(&et->txq[TX_Q0]) < (DATAHIWAT/2)))
			netif_wake_queue(et->dev);
	}
}

void
et_init(et_info_t *et, uint options)
{
	ET_TRACE(("et%d: et_init\n", et->etc->unit));
	ET_LOG("et%d: et_init", et->etc->unit, 0);

	et_reset(et);

	etc_init(et->etc, options);
}


void
et_reset(et_info_t *et)
{
	ET_TRACE(("et%d: et_reset\n", et->etc->unit));

	etc_reset(et->etc);

	/* zap any pending dpc interrupt bits */
	et->events = 0;

	/* dpc will not be rescheduled */
	et->resched = 0;
}

void
et_up(et_info_t *et)
{
	etc_info_t *etc;

	etc = et->etc;

	if (etc->up)
		return;

	ET_TRACE(("et%d: et_up\n", etc->unit));

	etc_up(etc);

	/* schedule one second watchdog timer */
	et->timer.expires = jiffies + HZ;
	add_timer(&et->timer);

	netif_start_queue(et->dev);
}

void
et_down(et_info_t *et, int reset)
{
	etc_info_t *etc;
	struct sk_buff *skb;
	int32 i;

	etc = et->etc;

	ET_TRACE(("et%d: et_down\n", etc->unit));

	netif_down(et->dev);
	netif_stop_queue(et->dev);

	/* stop watchdog timer */
	del_timer(&et->timer);

	etc_down(etc, reset);

	/* flush the txq(s) */
	for (i = 0; i < NUMTXQ; i++)
		while ((skb = skb_dequeue(&et->txq[i])))
			PKTFREE(etc->osh, skb, TRUE);

#ifndef NAPI_POLL
	/* kill dpc */
	ET_UNLOCK(et);
	tasklet_kill(&et->tasklet);
	ET_LOCK(et);
#endif /* NAPI_POLL */
}

/*
 * These are interrupt on/off entry points. Disable interrupts
 * during interrupt state transition.
 */
void
et_intrson(et_info_t *et)
{
	unsigned long flags;
	INT_LOCK(flags);
	(*et->etc->chops->intrson)(et->etc->ch);
	INT_UNLOCK(flags);
}

static void
et_watchdog(ulong data)
{
	et_info_t *et;

	et = ET_INFO((struct net_device *)data);

	ET_LOCK(et);

	etc_watchdog(et->etc);

	/* reschedule one second watchdog timer */
	et->timer.expires = jiffies + HZ;
	add_timer(&et->timer);

	ET_UNLOCK(et);
#ifdef CTFPOOL
	/* allocate and add a new skb to the pkt pool */
	if (CTF_ENAB(et->cih))
		osl_ctfpool_replenish(et->osh, CTFPOOL_REFILL_THRESH);
#endif /* CTFPOOL */
}


#ifdef SIOCETHTOOL
static int
et_ethtool(et_info_t *et, struct ethtool_cmd *ecmd)
{
	int ret = 0;
	int speed;
	struct ethtool_drvinfo *info;

	ET_LOCK(et);

	switch (ecmd->cmd) {
	case ETHTOOL_GSET:
		ecmd->supported = (SUPPORTED_10baseT_Half | SUPPORTED_10baseT_Full |
		                   SUPPORTED_100baseT_Half | SUPPORTED_100baseT_Full |
		                   SUPPORTED_Autoneg);
		ecmd->advertising = ADVERTISED_TP;
		ecmd->advertising |= (et->etc->advertise & ADV_10HALF) ?
		        ADVERTISED_10baseT_Half : 0;
		ecmd->advertising |= (et->etc->advertise & ADV_10FULL) ?
		        ADVERTISED_10baseT_Full : 0;
		ecmd->advertising |= (et->etc->advertise & ADV_100HALF) ?
		        ADVERTISED_100baseT_Half : 0;
		ecmd->advertising |= (et->etc->advertise & ADV_100FULL) ?
		        ADVERTISED_100baseT_Full : 0;
		ecmd->advertising |= (et->etc->advertise2 & ADV_1000FULL) ?
		        ADVERTISED_1000baseT_Full : 0;
		ecmd->advertising |= (et->etc->advertise2 & ADV_1000HALF) ?
		        ADVERTISED_1000baseT_Half : 0;
		ecmd->advertising |= (et->etc->forcespeed == ET_AUTO) ?
		        ADVERTISED_Autoneg : 0;
		if (et->etc->linkstate) {
			ecmd->speed = (et->etc->speed == 1000) ? SPEED_1000 :
			               ((et->etc->speed == 100) ? SPEED_100 : SPEED_10);
			ecmd->duplex = (et->etc->duplex == 1) ? DUPLEX_FULL : DUPLEX_HALF;
		} else {
			ecmd->speed = 0;
			ecmd->duplex = 0;
		}
		ecmd->port = PORT_TP;
		ecmd->phy_address = 0;
		ecmd->transceiver = XCVR_INTERNAL;
		ecmd->autoneg = (et->etc->forcespeed == ET_AUTO) ? AUTONEG_ENABLE : AUTONEG_DISABLE;
		ecmd->maxtxpkt = 0;
		ecmd->maxrxpkt = 0;
		break;
	case ETHTOOL_SSET:
		if (!capable(CAP_NET_ADMIN)) {
			ret = -EPERM;
			break;
		}
		else if (ecmd->speed == SPEED_10 && ecmd->duplex == DUPLEX_HALF)
			speed = ET_10HALF;
		else if (ecmd->speed == SPEED_10 && ecmd->duplex == DUPLEX_FULL)
			speed = ET_10FULL;
		else if (ecmd->speed == SPEED_100 && ecmd->duplex == DUPLEX_HALF)
			speed = ET_100HALF;
		else if (ecmd->speed == SPEED_100 && ecmd->duplex == DUPLEX_FULL)
			speed = ET_100FULL;
		else if (ecmd->speed == SPEED_1000 && ecmd->duplex == DUPLEX_FULL)
			speed = ET_1000FULL;
		else if (ecmd->autoneg == AUTONEG_ENABLE)
			speed = ET_AUTO;
		else {
			ret = -EINVAL;
			break;
		}
		ret = etc_ioctl(et->etc, ETCSPEED, &speed);
		break;
	case ETHTOOL_GDRVINFO:
		info = (struct ethtool_drvinfo *)ecmd;
		bzero(info, sizeof(struct ethtool_drvinfo));
		info->cmd = ETHTOOL_GDRVINFO;
		sprintf(info->driver, "et%d", et->etc->unit);
		strcpy(info->version, EPI_VERSION_STR);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	ET_UNLOCK(et);

	return (ret);
}
#endif /* SIOCETHTOOL */

static int
et_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	et_info_t *et;
	int error;
	char *buf;
	int size, ethtoolcmd;
	bool get = 0, set;
	et_var_t *var = NULL;
	void *buffer = NULL;

	et = ET_INFO(dev);

	ET_TRACE(("et%d: et_ioctl: cmd 0x%x\n", et->etc->unit, cmd));

	switch (cmd) {
#ifdef SIOCETHTOOL
	case SIOCETHTOOL:
		if (copy_from_user(&ethtoolcmd, ifr->ifr_data, sizeof(uint32)))
			return (-EFAULT);

		if (ethtoolcmd == ETHTOOL_GDRVINFO)
			size = sizeof(struct ethtool_drvinfo);
		else
			size = sizeof(struct ethtool_cmd);
		get = TRUE; set = TRUE;
		break;
#endif /* SIOCETHTOOL */
	case SIOCGETCDUMP:
		size = ETDUMPSZ;
		get = TRUE; set = FALSE;
		break;
	case SIOCGETCPHYRD:
	case SIOCGETCPHYRD2:
	case SIOCGETCROBORD:
		size = sizeof(int) * 2;
		get = TRUE; set = TRUE;
		break;
	case SIOCSETCPHYWR:
	case SIOCSETCPHYWR2:
	case SIOCSETCROBOWR:
		size = sizeof(int) * 2;
		get = FALSE; set = TRUE;
		break;
	case SIOCSETGETVAR:
		size = sizeof(et_var_t);
		set = TRUE;
		break;
	default:
		size = sizeof(int);
		get = FALSE; set = TRUE;
		break;
	}

	if ((buf = MALLOC(et->osh, size)) == NULL) {
		ET_ERROR(("et: et_ioctl: out of memory, malloced %d bytes\n", MALLOCED(et->osh)));
		return (-ENOMEM);
	}

	if (set && copy_from_user(buf, ifr->ifr_data, size)) {
		MFREE(et->osh, buf, size);
		return (-EFAULT);
	}

	if (cmd == SIOCSETGETVAR) {
		var = (et_var_t *)buf;
		if (var->buf) {
			if (!var->set)
				get = TRUE;

			if (!(buffer = (void *) MALLOC(et->osh, var->len))) {
				ET_ERROR(("et: et_ioctl: out of memory, malloced %d bytes\n",
					MALLOCED(et->osh)));
				MFREE(et->osh, buf, size);
				return (-ENOMEM);
			}

			if (copy_from_user(buffer, var->buf, var->len)) {
				MFREE(et->osh, buffer, var->len);
				MFREE(et->osh, buf, size);
				return (-EFAULT);
			}
		}
	}

	switch (cmd) {
#ifdef SIOCETHTOOL
	case SIOCETHTOOL:
		error = et_ethtool(et, (struct ethtool_cmd *)buf);
		break;
#endif /* SIOCETHTOOL */
	case SIOCSETGETVAR:
		ET_LOCK(et);
		error = etc_iovar(et->etc, var->cmd, var->set, buffer);
		ET_UNLOCK(et);
		if (!error && get)
			error = copy_to_user(var->buf, buffer, var->len);
		if (buffer)
			MFREE(et->osh, buffer, var->len);
		break;
	default:
		ET_LOCK(et);
		error = etc_ioctl(et->etc, cmd - SIOCSETCUP, buf) ? -EINVAL : 0;
		ET_UNLOCK(et);
		break;
	}

	if (!error && get)
		error = copy_to_user(ifr->ifr_data, buf, size);

	MFREE(et->osh, buf, size);

	return (error);
}

static struct net_device_stats *
et_get_stats(struct net_device *dev)
{
	et_info_t *et;
	etc_info_t *etc;
	struct net_device_stats *stats;

	et = ET_INFO(dev);

	ET_TRACE(("et%d: et_get_stats\n", et->etc->unit));

	ET_LOCK(et);

	etc = et->etc;
	stats = &et->stats;
	bzero(stats, sizeof(struct net_device_stats));

	/* refresh stats */
	if (et->etc->up)
		(*etc->chops->statsupd)(etc->ch);

	/* SWAG */
	stats->rx_packets = etc->rxframe;
	stats->tx_packets = etc->txframe;
	stats->rx_bytes = etc->rxbyte;
	stats->tx_bytes = etc->txbyte;
	stats->rx_errors = etc->rxerror;
	stats->tx_errors = etc->txerror;

	if (ET_GMAC(etc)) {
		gmacmib_t *mib;

		mib = etc->mib;
		stats->collisions = mib->tx_total_cols;
		stats->rx_length_errors = (mib->rx_oversize_pkts + mib->rx_undersize);
		stats->rx_crc_errors = mib->rx_crc_errs;
		stats->rx_frame_errors = mib->rx_align_errs;
		stats->rx_missed_errors = mib->rx_missed_pkts;
	} else {
		bcmenetmib_t *mib;

		mib = etc->mib;
		stats->collisions = mib->tx_total_cols;
		stats->rx_length_errors = (mib->rx_oversize_pkts + mib->rx_undersize);
		stats->rx_crc_errors = mib->rx_crc_errs;
		stats->rx_frame_errors = mib->rx_align_errs;
		stats->rx_missed_errors = mib->rx_missed_pkts;

	}

	stats->rx_fifo_errors = etc->rxoflo;
	stats->rx_over_errors = etc->rxoflo;
	stats->tx_fifo_errors = etc->txuflo;

	ET_UNLOCK(et);

	return (stats);
}

static int
et_set_mac_address(struct net_device *dev, void *addr)
{
	et_info_t *et;
	struct sockaddr *sa = (struct sockaddr *) addr;

	et = ET_INFO(dev);
	ET_TRACE(("et%d: et_set_mac_address\n", et->etc->unit));

	if (et->etc->up)
		return -EBUSY;

	bcopy(sa->sa_data, dev->dev_addr, ETHER_ADDR_LEN);
	bcopy(dev->dev_addr, &et->etc->cur_etheraddr, ETHER_ADDR_LEN);

	return 0;
}

static void
et_set_multicast_list(struct net_device *dev)
{
	et_info_t *et;
	etc_info_t *etc;
	struct dev_mc_list *mclist;
	int i;

	et = ET_INFO(dev);
	etc = et->etc;

	ET_TRACE(("et%d: et_set_multicast_list\n", etc->unit));

	ET_LOCK(et);

	if (etc->up) {
		etc->promisc = (dev->flags & IFF_PROMISC)? TRUE: FALSE;
		etc->allmulti = (dev->flags & IFF_ALLMULTI)? TRUE: etc->promisc;

		/* copy the list of multicasts into our private table */
		for (i = 0, mclist = dev->mc_list; mclist && (i < dev->mc_count);
			i++, mclist = mclist->next) {
			if (i >= MAXMULTILIST) {
				etc->allmulti = TRUE;
				i = 0;
				break;
			}
			etc->multicast[i] = *((struct ether_addr *)mclist->dmi_addr);
		}
		etc->nmulticast = i;

		et_init(et, ET_INIT_DEF_OPTIONS);
	}

	ET_UNLOCK(et);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 20)
static irqreturn_t BCMFASTPATH
et_isr(int irq, void *dev_id)
#else
static irqreturn_t BCMFASTPATH
et_isr(int irq, void *dev_id, struct pt_regs *ptregs)
#endif
{
	et_info_t *et;
	struct chops *chops;
	void *ch;
	uint events = 0;

	et = (et_info_t *)dev_id;
	chops = et->etc->chops;
	ch = et->etc->ch;

	/* guard against shared interrupts */
	if (!et->etc->up)
		goto done;

	/* get interrupt condition bits */
	events = (*chops->getintrevents)(ch, TRUE);


	/* not for us */
	if (!(events & INTR_NEW))
		goto done;

	ET_TRACE(("et%d: et_isr: events 0x%x\n", et->etc->unit, events));
	ET_LOG("et%d: et_isr: events 0x%x", et->etc->unit, events);

	/* disable interrupts */
	(*chops->intrsoff)(ch);

	/* save intstatus bits */
	ASSERT(et->events == 0);
	et->events = events;

	ASSERT(et->resched == FALSE);
#ifdef NAPI_POLL
	/* allow the device to be added to the cpu polling list if we are up */
	if (netif_rx_schedule_prep(et->dev)) {
		/* tell the network core that we have packets to send up */
		__netif_rx_schedule(et->dev);
	} else {
		ET_ERROR(("et%d: et_isr: intr while in poll!\n",
		          et->etc->unit));
		(*chops->intrson)(ch);
	}
#else /* NAPI_POLL */
	/* schedule dpc */
	tasklet_schedule(&et->tasklet);
#endif /* NAPI_POLL */

done:
	ET_LOG("et%d: et_isr ret", et->etc->unit, 0);

	return IRQ_RETVAL(events & INTR_NEW);
}

static inline int
et_rxevent(osl_t *osh, et_info_t *et, struct chops *chops, void *ch, int quota)
{
	uint processed = 0;
	void *p = NULL, *h = NULL, *t = NULL;
	struct sk_buff *skb;

	/* read the buffers first */
	while ((p = (*chops->rx)(ch))) {
		if (t == NULL)
			h = t = p;
		else {
			PKTSETLINK(t, p);
			t = p;
		}

		/* we reached quota already */
		if (++processed >= quota) {
			/* reschedule et_dpc()/et_poll() */
			et->resched = TRUE;
			break;
		}
	}

	/* prefetch the headers */
	if (h != NULL)
		ETPREFHDRS(PKTDATA(osh, h), PREFSZ);

	/* post more rx bufs */
	(*chops->rxfill)(ch);

	while ((p = h) != NULL) {
		h = PKTLINK(h);
		PKTSETLINK(p, NULL);
		/* prefetch the headers */
		if (h != NULL)
			ETPREFHDRS(PKTDATA(osh, h), PREFSZ);
		skb = PKTTONATIVE(osh, p);
		et_sendup(et, skb);
	}

	return (processed);
}

#ifdef NAPI_POLL
static int BCMFASTPATH
et_poll(struct net_device *dev, int *budget)
{
	int quota = min(RXBND, *budget);
	et_info_t *et = ET_INFO(dev);
#else /* NAPI_POLL */
static void BCMFASTPATH
et_dpc(ulong data)
{
	int quota = RXBND;
	et_info_t *et = (et_info_t *)data;
#endif /* NAPI_POLL */
	struct chops *chops;
	void *ch;
	osl_t *osh;
	uint nrx = 0;

	chops = et->etc->chops;
	ch = et->etc->ch;
	osh = et->etc->osh;

	ET_TRACE(("et%d: et_dpc: events 0x%x\n", et->etc->unit, et->events));
	ET_LOG("et%d: et_dpc: events 0x%x", et->etc->unit, et->events);

#ifndef NAPI_POLL
	ET_LOCK(et);
#endif /* NAPI_POLL */

	if (!et->etc->up)
		goto done;

	/* get interrupt condition bits again when dpc was rescheduled */
	if (et->resched) {
		et->events = (*chops->getintrevents)(ch, FALSE);
		et->resched = FALSE;
	}

	if (et->events & INTR_RX)
		nrx = et_rxevent(osh, et, chops, ch, quota);

	if (et->events & INTR_TX)
		(*chops->txreclaim)(ch, FALSE);

	/* handle error conditions, if reset required leave interrupts off! */
	if (et->events & INTR_ERROR) {
		if ((*chops->errors)(ch))
			et_init(et, ET_INIT_INTROFF);
		else
			if (nrx < quota)
				nrx += et_rxevent(osh, et, chops, ch, quota);
	}

	/* run the tx queue */
	if (et->etc->txq_state != 0)
		et_sendnext(et);

	/* clear this before re-enabling interrupts */
	et->events = 0;

	/* something may bring the driver down */
	if (!et->etc->up) {
		et->resched = FALSE;
		goto done;
	}

#ifndef NAPI_POLL
	/* there may be frames left, reschedule et_dpc() */
	if (et->resched)
		tasklet_schedule(&et->tasklet);
	/* re-enable interrupts */
	else
		(*chops->intrson)(ch);
#endif /* NAPI_POLL */

done:
	ET_LOG("et%d: et_dpc ret", et->etc->unit, 0);

#ifdef NAPI_POLL
	/* update number of frames processed */
	*budget -= nrx;
	dev->quota -= nrx;

	ET_TRACE(("et%d: et_poll: quota %d budget %d\n",
	          et->etc->unit, dev->quota, *budget));

	/* we got packets but no quota */
	if (et->resched)
		/* indicate that we are not done, don't enable
		 * interrupts yet. linux network core will call
		 * us again.
		 */
		return (1);

	netif_rx_complete(dev);

	/* enable interrupts now */
	(*chops->intrson)(ch);

	/* indicate that we are done */
	return (0);
#else /* NAPI_POLL */
	ET_UNLOCK(et);
	return;
#endif /* NAPI_POLL */
}

static void
et_error(et_info_t *et, struct sk_buff *skb, void *rxh)
{
	uchar eabuf[32];
	struct ether_header *eh;

	eh = (struct ether_header *)skb->data;
	bcm_ether_ntoa((struct ether_addr *)eh->ether_shost, eabuf);

	if (RXH_OVERSIZE(et->etc, rxh)) {
		ET_ERROR(("et%d: rx: over size packet from %s\n", et->etc->unit, eabuf));
	}
	if (RXH_CRC(et->etc, rxh)) {
		ET_ERROR(("et%d: rx: crc error from %s\n", et->etc->unit, eabuf));
	}
	if (RXH_OVF(et->etc, rxh)) {
		ET_ERROR(("et%d: rx: fifo overflow\n", et->etc->unit));
	}
	if (RXH_NO(et->etc, rxh)) {
		ET_ERROR(("et%d: rx: crc error (odd nibbles) from %s\n",
		          et->etc->unit, eabuf));
	}
	if (RXH_RXER(et->etc, rxh)) {
		ET_ERROR(("et%d: rx: symbol error from %s\n", et->etc->unit, eabuf));
	}
}

static inline int32
et_ctf_forward(et_info_t *et, struct sk_buff *skb)
{
#ifdef HNDCTF
	/* use slow path if ctf is disabled */
	if (!CTF_ENAB(et->cih))
		return (BCME_ERROR);

	/* try cut thru first */
	if (ctf_forward(et->cih, skb) != BCME_ERROR)
		return (BCME_OK);

	/* clear skipct flag before sending up */
	PKTCLRSKIPCT(et->osh, skb);
#endif /* HNDCTF */

#ifdef CTFPOOL
	/* allocate and add a new skb to the pkt pool */
	if (PKTISFAST(et->osh, skb))
		osl_ctfpool_add(et->osh);

	/* clear fast buf flag before sending up */
	PKTCLRFAST(et->osh, skb);

	/* re-init the hijacked field */
	CTFPOOLPTR(et->osh, skb) = NULL;
#endif /* CTFPOOL */

	/* map the unmapped buffer memory before sending up */
	PKTCTFMAP(et->osh, skb);

	return (BCME_ERROR);
}


void BCMFASTPATH
et_sendup(et_info_t *et, struct sk_buff *skb)
{
	etc_info_t *etc;
	void *rxh;
	uint16 flags;

	etc = et->etc;

	/* packet buffer starts with rxhdr */
	rxh = skb->data;

	/* strip off rxhdr */
	__skb_pull(skb, HWRXOFF);

	ET_TRACE(("et%d: et_sendup: %d bytes\n", et->etc->unit, skb->len));
	ET_LOG("et%d: et_sendup: len %d", et->etc->unit, skb->len);

	etc->rxframe++;
	etc->rxbyte += skb->len;

	/* eh should now be aligned 2-mod-4 */
	ASSERT(((ulong)skb->data & 3) == 2);

	/* strip off crc32 */
	__skb_trim(skb, skb->len - ETHER_CRC_LEN);

	ET_PRHDR("rx", (struct ether_header *)skb->data, skb->len, etc->unit);
	ET_PRPKT("rxpkt", skb->data, skb->len, etc->unit);

	/* get the error flags */
	flags = RXH_FLAGS(etc, rxh);

	/* check for reported frame errors */
	if (flags)
		goto err;

	skb->dev = et->dev;

#ifdef HNDCTF
	/* try cut thru' before sending up */
	if (et_ctf_forward(et, skb) != BCME_ERROR)
		return;
#endif /* HNDCTF */

	/* extract priority from payload and store it out-of-band
	 * in skb->priority
	 */
	if (et->etc->qos)
		pktsetprio(skb, TRUE);

	skb->protocol = eth_type_trans(skb, et->dev);

	/* send it up */
#ifdef NAPI_POLL
	netif_receive_skb(skb);
#else /* NAPI_POLL */
	netif_rx(skb);
#endif /* NAPI_POLL */

	ET_LOG("et%d: et_sendup ret", et->etc->unit, 0);

	return;

err:
	et_error(et, skb, rxh);
	PKTFRMNATIVE(etc->osh, skb);
	PKTFREE(etc->osh, skb, FALSE);

	return;
}

void
et_dump(et_info_t *et, struct bcmstrbuf *b)
{
	bcm_bprintf(b, "et%d: %s %s version %s\n", et->etc->unit,
		__DATE__, __TIME__, EPI_VERSION_STR);

#ifdef HNDCTF
#if defined(BCMDBG_DUMP)
	ctf_dump(et->cih, b);
#endif 
#endif /* HNDCTF */

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

/*
 * 47XX-specific shared mdc/mdio contortion:
 * Find the et associated with the same chip as <et>
 * and coreunit matching <coreunit>.
 */
void *
et_phyfind(et_info_t *et, uint coreunit)
{
	et_info_t *tmp;
	uint bus, slot;

	bus = et->pdev->bus->number;
	slot = PCI_SLOT(et->pdev->devfn);

	/* walk the list et's */
	for (tmp = et_list; tmp; tmp = tmp->next) {
		if (et->etc == NULL)
			continue;
		if (tmp->pdev == NULL)
			continue;
		if (tmp->pdev->bus->number != bus)
			continue;
		if (tmp->etc->nicmode)
			if (PCI_SLOT(tmp->pdev->devfn) != slot)
				continue;
		if (tmp->etc->coreunit != coreunit)
			continue;
		break;
	}
	return (tmp);
}

/* shared phy read entry point */
uint16
et_phyrd(et_info_t *et, uint phyaddr, uint reg)
{
	uint16 val;

	ET_LOCK(et);
	val = et->etc->chops->phyrd(et->etc->ch, phyaddr, reg);
	ET_UNLOCK(et);

	return (val);
}

/* shared phy write entry point */
void
et_phywr(et_info_t *et, uint phyaddr, uint reg, uint16 val)
{
	ET_LOCK(et);
	et->etc->chops->phywr(et->etc->ch, phyaddr, reg, val);
	ET_UNLOCK(et);
}
