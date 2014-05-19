/*
 * Fast Ethernet Controller (FEC) driver for Motorola MPC8xx.
 * Copyright (c) 1997 Dan Malek (dmalek@jlc.net)
 *
 * Right now, I am very wasteful with the buffers.  I allocate memory
 * pages and then divide them into 2K frame buffers.  This way I know I
 * have buffers large enough to hold one frame within one buffer descriptor.
 * Once I get this working, I will use 64 or 128 byte CPM buffers, which
 * will be much more memory efficient and will easily handle lots of
 * small packets.
 *
 * Much better multiple PHY support by Magnus Damm.
 * Copyright (c) 2000 Ericsson Radio Systems AB.
 *
 * Support for FEC controller of ColdFire processors.
 * Copyright (c) 2001-2005 Greg Ungerer (gerg@snapgear.com)
 *
 * Bug fixes and cleanup by Philippe De Muyter (phdm@macqel.be)
 * Copyright (c) 2004-2006 Macq Electronique SA.
 *
 * Support for FEC IEEE 1588.
 * Copyright (C) 2010-2013 Freescale Semiconductor, Inc.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/ptrace.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/ioport.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/clk.h>
#include <mach/clock.h>
#include <linux/platform_device.h>
#include <linux/phy.h>
#include <linux/fec.h>

#include <asm/cacheflush.h>

#ifndef CONFIG_ARM
#include <asm/coldfire.h>
#include <asm/mcfsim.h>
#endif

#include "fec.h"
#include "fec_1588.h"

#if defined(CONFIG_ARCH_MXC) || defined(CONFIG_SOC_IMX28)
#define FEC_ALIGNMENT	0xf
#else
#define FEC_ALIGNMENT	0x3
#endif

#define DRIVER_NAME	"fec"

/* Controller is ENET-MAC */
#define FEC_QUIRK_ENET_MAC		(1 << 0)
/* Controller needs driver to swap frame */
#define FEC_QUIRK_SWAP_FRAME		(1 << 1)
/* ENET IP errata ticket TKT168103 */
#define FEC_QUIRK_BUG_TKT168103		(1 << 2)

static struct platform_device_id fec_devtype[] = {
	{
		.name = "enet",
		.driver_data = FEC_QUIRK_ENET_MAC | FEC_QUIRK_BUG_TKT168103,
	},
	{
		.name = "fec",
		.driver_data = 0,
	},
	{
		.name = "imx28-fec",
		.driver_data = FEC_QUIRK_ENET_MAC | FEC_QUIRK_SWAP_FRAME |
				FEC_QUIRK_BUG_TKT168103,
	},
	{ }
};

static unsigned char macaddr[ETH_ALEN];
module_param_array(macaddr, byte, NULL, 0);
MODULE_PARM_DESC(macaddr, "FEC Ethernet MAC address");

#if defined(CONFIG_M5272)
/*
 * Some hardware gets it MAC address out of local flash memory.
 * if this is non-zero then assume it is the address to get MAC from.
 */
#if defined(CONFIG_NETtel)
#define	FEC_FLASHMAC	0xf0006006
#elif defined(CONFIG_GILBARCONAP) || defined(CONFIG_SCALES)
#define	FEC_FLASHMAC	0xf0006000
#elif defined(CONFIG_CANCam)
#define	FEC_FLASHMAC	0xf0020000
#elif defined (CONFIG_M5272C3)
#define	FEC_FLASHMAC	(0xffe04000 + 4)
#elif defined(CONFIG_MOD5272)
#define FEC_FLASHMAC 	0xffc0406b
#else
#define	FEC_FLASHMAC	0
#endif
#endif /* CONFIG_M5272 */

/* The number of Tx and Rx buffers.  These are allocated from the page
 * pool.  The code may assume these are power of two, so it it best
 * to keep them that size.
 * We don't need to allocate pages for the transmitter.  We just use
 * the skbuffer directly.
 */
#define FEC_ENET_RX_PAGES	192
#define FEC_ENET_RX_FRSIZE	2048
#define FEC_ENET_RX_FRPPG	(PAGE_SIZE / FEC_ENET_RX_FRSIZE)
#define RX_RING_SIZE		(FEC_ENET_RX_FRPPG * FEC_ENET_RX_PAGES)
#define FEC_ENET_TX_FRSIZE	2048
#define FEC_ENET_TX_FRPPG	(PAGE_SIZE / FEC_ENET_TX_FRSIZE)
#define TX_RING_SIZE		128	/* Must be power of two */
#define TX_RING_MOD_MASK	127	/*   for this to work */

#define BUFDES_SIZE ((RX_RING_SIZE + TX_RING_SIZE) * sizeof(struct bufdesc))

/* Interrupt events/masks. */
#define FEC_ENET_HBERR	((uint)0x80000000)	/* Heartbeat error */
#define FEC_ENET_BABR	((uint)0x40000000)	/* Babbling receiver */
#define FEC_ENET_BABT	((uint)0x20000000)	/* Babbling transmitter */
#define FEC_ENET_GRA	((uint)0x10000000)	/* Graceful stop complete */
#define FEC_ENET_TXF	((uint)0x08000000)	/* Full frame transmitted */
#define FEC_ENET_TXB	((uint)0x04000000)	/* A buffer was transmitted */
#define FEC_ENET_RXF	((uint)0x02000000)	/* Full frame received */
#define FEC_ENET_RXB	((uint)0x01000000)	/* A buffer was received */
#define FEC_ENET_MII	((uint)0x00800000)	/* MII interrupt */
#define FEC_ENET_EBERR	((uint)0x00400000)	/* SDMA bus error */
#define FEC_ENET_TS_AVAIL       ((uint)0x00010000)
#define FEC_ENET_TS_TIMER       ((uint)0x00008000)
#define FEC_ENET_MII_CLK       ((uint)2500000)
#define FEC_ENET_HOLD_TIME     ((uint)0x100)  /* 2 internal clock cycle*/

#define FEC_DEFAULT_IMASK (FEC_ENET_TXF | FEC_ENET_RXF | FEC_ENET_MII)
#if defined(CONFIG_FEC_1588)
#define FEC_1588_IMASK	  (FEC_ENET_TS_AVAIL | FEC_ENET_TS_TIMER)
#else
#define FEC_1588_IMASK	0
#endif

/* The FEC stores dest/src/type, data, and checksum for receive packets.
 */
#define PKT_MAXBUF_SIZE		1518
#define PKT_MINBUF_SIZE		64
#define PKT_MAXBLR_SIZE		1520

/* Pause frame feild and FIFO threshold */
#define FEC_ENET_FCE		(1 << 5)
#define FEC_ENET_RSEM_V		0x84
#define FEC_ENET_RSFL_V		16
#define FEC_ENET_RAEM_V		0x8
#define FEC_ENET_RAFL_V		0x8
#define FEC_ENET_OPD_V		0xFFF0

/*
 * The 5270/5271/5280/5282/532x RX control register also contains maximum frame
 * size bits. Other FEC hardware does not, so we need to take that into
 * account when setting it.
 */
#if defined(CONFIG_M523x) || defined(CONFIG_M527x) || defined(CONFIG_M528x) || \
    defined(CONFIG_M520x) || defined(CONFIG_M532x) || \
    defined(CONFIG_ARCH_MXC) || defined(CONFIG_SOC_IMX28)
#define	OPT_FRAME_SIZE	(PKT_MAXBUF_SIZE << 16)
#else
#define	OPT_FRAME_SIZE	0
#endif

/* The FEC buffer descriptors track the ring buffers.  The rx_bd_base and
 * tx_bd_base always point to the base of the buffer descriptors.  The
 * cur_rx and cur_tx point to the currently available buffer.
 * The dirty_tx tracks the current buffer that is being sent by the
 * controller.  The cur_tx and dirty_tx are equal under both completely
 * empty and completely full conditions.  The empty/ready indicator in
 * the buffer descriptor determines the actual condition.
 */
struct fec_enet_private {
	/* Hardware registers of the FEC device */
	void __iomem *hwp;

	struct net_device *netdev;

	struct clk *clk;
	struct clk *mdc_clk;

	/* The saved address of a sent-in-place packet/buffer, for skfree(). */
	unsigned char *tx_bounce[TX_RING_SIZE];
	struct	sk_buff* tx_skbuff[TX_RING_SIZE];
	struct	sk_buff* rx_skbuff[RX_RING_SIZE];
	ushort	skb_cur;
	ushort	skb_dirty;

	/* CPM dual port RAM relative addresses */
	dma_addr_t	bd_dma;
	/* Address of Rx and Tx buffers */
	struct bufdesc	*rx_bd_base;
	struct bufdesc	*tx_bd_base;
	/* The next free ring entry */
	struct bufdesc	*cur_rx, *cur_tx;
	/* The ring entries to be free()ed */
	struct bufdesc	*dirty_tx;

	uint	tx_full;
	/* hold while accessing the HW like ringbuffer for tx/rx but not MAC */
	spinlock_t hw_lock;

	struct	platform_device *pdev;

	int	opened;

	/* Phylib and MDIO interface */
	struct	mii_bus *mii_bus;
	struct	phy_device *phy_dev;
	int	mii_timeout;
	uint	phy_speed;
	phy_interface_t	phy_interface;
	int	index;
	int	link;
	int	full_duplex;
	struct	completion mdio_done;
	struct delayed_work fixup_trigger_tx;

	struct  fec_ptp_private *ptp_priv;
	uint    ptimer_present;

	struct napi_struct napi;
	int	napi_weight;
	bool	use_napi;
};

#define FEC_NAPI_WEIGHT 64
#ifdef CONFIG_FEC_NAPI
#define FEC_NAPI_ENABLE TRUE
#else
#define FEC_NAPI_ENABLE FALSE
#endif

static irqreturn_t fec_enet_interrupt(int irq, void * dev_id);
static void fec_enet_tx(struct net_device *dev);
static int fec_rx_poll(struct napi_struct *napi, int budget);
static void fec_enet_rx(struct net_device *dev);
static int fec_enet_close(struct net_device *dev);
static void fec_restart(struct net_device *dev, int duplex);
static void fec_stop(struct net_device *dev);

/* FEC MII MMFR bits definition */
#define FEC_MMFR_ST		(1 << 30)
#define FEC_MMFR_OP_READ	(2 << 28)
#define FEC_MMFR_OP_WRITE	(1 << 28)
#define FEC_MMFR_PA(v)		((v & 0x1f) << 23)
#define FEC_MMFR_RA(v)		((v & 0x1f) << 18)
#define FEC_MMFR_TA		(2 << 16)
#define FEC_MMFR_DATA(v)	(v & 0xffff)

#define FEC_MII_TIMEOUT		30 /* ms */

/* Transmitter timeout */
#define TX_TIMEOUT (2 * HZ)

static void *swap_buffer(void *bufaddr, int len)
{
	int i;
	unsigned int *buf = bufaddr;

	for (i = 0; i < (len + 3) / 4; i++, buf++)
		*buf = cpu_to_be32(*buf);

	return bufaddr;
}

static inline
void *fec_enet_get_pre_txbd(struct net_device *ndev)
{
	struct fec_enet_private *fep = netdev_priv(ndev);
	struct bufdesc *bdp = fep->cur_tx;

	if (bdp == fep->tx_bd_base)
		return bdp + TX_RING_SIZE;
	else
		return bdp - 1;

}

/* MTIP enet IP have one IC issue recorded at PDM ticket:TKT168103
 * The TDAR bit after being set by software is not acted upon by the
 * ENET module due to the timing of when the ENET state machine
 * clearing the TDAR bit occurring coincident or momentarily after
 * the software sets the bit.
 * This forces ENET module to check the Transmit buffer descriptor
 * and take action if the “ready” flag is set. Otherwise the ENET
 * returns to idle mode.
 */
static void fixup_trigger_tx_func(struct work_struct *work)
{
	struct fec_enet_private *fep =
			container_of(work, struct fec_enet_private,
					fixup_trigger_tx.work);

	writel(0, fep->hwp + FEC_X_DES_ACTIVE);
}

static netdev_tx_t
fec_enet_start_xmit(struct sk_buff *skb, struct net_device *ndev)
{
	struct fec_enet_private *fep = netdev_priv(ndev);
	const struct platform_device_id *id_entry =
				platform_get_device_id(fep->pdev);
	struct bufdesc *bdp, *bdp_pre;
	void *bufaddr;
	unsigned short	status;
	unsigned long   estatus;
	unsigned long flags;

	spin_lock_irqsave(&fep->hw_lock, flags);
	if (!fep->link) {
		/* Link is down or autonegotiation is in progress. */
		netif_stop_queue(ndev);
		spin_unlock_irqrestore(&fep->hw_lock, flags);
		return NETDEV_TX_BUSY;
	}

	/* Fill in a Tx ring entry */
	bdp = fep->cur_tx;
	status = bdp->cbd_sc;

	if (status & BD_ENET_TX_READY) {
		/* Ooops.  All transmit buffers are full.  Bail out.
		 * This should not happen, since ndev->tbusy should be set.
		 */
		printk("%s: tx queue full!.\n", ndev->name);
		netif_stop_queue(ndev);
		spin_unlock_irqrestore(&fep->hw_lock, flags);
		return NETDEV_TX_BUSY;
	}

	/* Clear all of the status flags */
	status &= ~BD_ENET_TX_STATS;

	/* Set buffer length and buffer pointer */
	bufaddr = skb->data;
	bdp->cbd_datlen = skb->len;

	/*
	 * On some FEC implementations data must be aligned on
	 * 4-byte boundaries. Use bounce buffers to copy data
	 * and get it aligned. Ugh.
	 */
	if (((unsigned long) bufaddr) & FEC_ALIGNMENT) {
		unsigned int index;
		index = bdp - fep->tx_bd_base;
		bufaddr = PTR_ALIGN(fep->tx_bounce[index], FEC_ALIGNMENT + 1);
		memcpy(bufaddr, (void *)skb->data, skb->len);
	}

	if (fep->ptimer_present) {
		if (fec_ptp_do_txstamp(skb)) {
			estatus = BD_ENET_TX_TS;
			status |= BD_ENET_TX_PTP;
		} else
			estatus = 0;
#ifdef CONFIG_ENHANCED_BD
		bdp->cbd_esc = (estatus | BD_ENET_TX_INT);
		bdp->cbd_bdu = 0;
#endif
	}
	/*
	 * Some design made an incorrect assumption on endian mode of
	 * the system that it's running on. As the result, driver has to
	 * swap every frame going to and coming from the controller.
	 */
	if (id_entry->driver_data & FEC_QUIRK_SWAP_FRAME)
		swap_buffer(bufaddr, skb->len);

	/* Save skb pointer */
	fep->tx_skbuff[fep->skb_cur] = skb;

	ndev->stats.tx_bytes += skb->len;
	fep->skb_cur = (fep->skb_cur+1) & TX_RING_MOD_MASK;

	/* Push the data cache so the CPM does not get stale memory
	 * data.
	 */
	bdp->cbd_bufaddr = dma_map_single(&fep->pdev->dev, bufaddr,
			FEC_ENET_TX_FRSIZE, DMA_TO_DEVICE);

	/* Send it on its way.  Tell FEC it's ready, interrupt when done,
	 * it's the last BD of the frame, and to put the CRC on the end.
	 */
	status |= (BD_ENET_TX_READY | BD_ENET_TX_INTR
			| BD_ENET_TX_LAST | BD_ENET_TX_TC);
	bdp->cbd_sc = status;

	/* Trigger transmission start */
	writel(0, fep->hwp + FEC_X_DES_ACTIVE);

	bdp_pre = fec_enet_get_pre_txbd(ndev);
	if ((id_entry->driver_data & FEC_QUIRK_BUG_TKT168103) &&
		!(bdp_pre->cbd_sc & BD_ENET_TX_READY))
		schedule_delayed_work(&fep->fixup_trigger_tx,
					 msecs_to_jiffies(1));

	/* If this was the last BD in the ring, start at the beginning again. */
	if (status & BD_ENET_TX_WRAP)
		bdp = fep->tx_bd_base;
	else
		bdp++;

	if (bdp == fep->dirty_tx) {
		fep->tx_full = 1;
		netif_stop_queue(ndev);
	}

	fep->cur_tx = bdp;

	spin_unlock_irqrestore(&fep->hw_lock, flags);

	return NETDEV_TX_OK;
}

static void
fec_timeout(struct net_device *ndev)
{
	struct fec_enet_private *fep = netdev_priv(ndev);

	ndev->stats.tx_errors++;

	netif_device_detach(ndev);
	fec_stop(ndev);

	fec_restart(ndev, fep->full_duplex);
	netif_device_attach(ndev);
	ndev->trans_start = jiffies; /* prevent tx timeout */
	if (fep->link && !fep->tx_full)
		netif_wake_queue(ndev);
}

static void
fec_rx_int_is_enabled(struct net_device *ndev, bool enabled)
{
	struct fec_enet_private *fep = netdev_priv(ndev);
	uint    int_events;

	int_events = readl(fep->hwp + FEC_IMASK);
	if (enabled)
		int_events |= FEC_ENET_RXF;
	else
		int_events &= ~FEC_ENET_RXF;
	writel(int_events, fep->hwp + FEC_IMASK);
}

#ifdef CONFIG_NET_POLL_CONTROLLER
static void fec_enet_netpoll(struct net_device *ndev)
{
	disable_irq(ndev->irq);
	fec_enet_interrupt(ndev->irq, ndev);
	enable_irq(ndev->irq);
}
#endif

static void
fec_enet_tx(struct net_device *ndev)
{
	struct	fec_enet_private *fep;
	struct  fec_ptp_private *fpp;
	struct bufdesc *bdp;
	unsigned short status;
	struct	sk_buff	*skb;

	fep = netdev_priv(ndev);
	fpp = fep->ptp_priv;
	spin_lock(&fep->hw_lock);
	bdp = fep->dirty_tx;

	while (((status = bdp->cbd_sc) & BD_ENET_TX_READY) == 0) {
		if (bdp == fep->cur_tx && fep->tx_full == 0)
			break;

		if (bdp->cbd_bufaddr)
			dma_unmap_single(&fep->pdev->dev, bdp->cbd_bufaddr,
				FEC_ENET_TX_FRSIZE, DMA_TO_DEVICE);
		bdp->cbd_bufaddr = 0;

		skb = fep->tx_skbuff[fep->skb_dirty];
		if (!skb)
			break;
		/* Check for errors. */
		if (status & (BD_ENET_TX_HB | BD_ENET_TX_LC |
				   BD_ENET_TX_RL | BD_ENET_TX_UN |
				   BD_ENET_TX_CSL)) {
			ndev->stats.tx_errors++;
			if (status & BD_ENET_TX_HB)  /* No heartbeat */
				ndev->stats.tx_heartbeat_errors++;
			if (status & BD_ENET_TX_LC)  /* Late collision */
				ndev->stats.tx_window_errors++;
			if (status & BD_ENET_TX_RL)  /* Retrans limit */
				ndev->stats.tx_aborted_errors++;
			if (status & BD_ENET_TX_UN)  /* Underrun */
				ndev->stats.tx_fifo_errors++;
			if (status & BD_ENET_TX_CSL) /* Carrier lost */
				ndev->stats.tx_carrier_errors++;
		} else {
			ndev->stats.tx_packets++;
		}

		if (status & BD_ENET_TX_READY)
			printk("HEY! Enet xmit interrupt and TX_READY.\n");

		/* Deferred means some collisions occurred during transmit,
		 * but we eventually sent the packet OK.
		 */
		if (status & BD_ENET_TX_DEF)
			ndev->stats.collisions++;

#if defined(CONFIG_ENHANCED_BD)
		if (fep->ptimer_present) {
			if (bdp->cbd_esc & BD_ENET_TX_TS)
				fec_ptp_store_txstamp(fpp, skb, bdp);
		}
#elif defined(CONFIG_IN_BAND)
		if (fep->ptimer_present) {
			if (status & BD_ENET_TX_PTP)
				fec_ptp_store_txstamp(fpp, skb, bdp);
		}
#endif

		/* Free the sk buffer associated with this last transmit */
		dev_kfree_skb_any(skb);
		fep->tx_skbuff[fep->skb_dirty] = NULL;
		fep->skb_dirty = (fep->skb_dirty + 1) & TX_RING_MOD_MASK;

		/* Update pointer to next buffer descriptor to be transmitted */
		if (status & BD_ENET_TX_WRAP)
			bdp = fep->tx_bd_base;
		else
			bdp++;

		/* Since we have freed up a buffer, the ring is no longer full
		 */
		if (fep->tx_full) {
			fep->tx_full = 0;
			if (netif_queue_stopped(ndev))
				netif_wake_queue(ndev);
		}
	}
	fep->dirty_tx = bdp;
	spin_unlock(&fep->hw_lock);
}

/*NAPI polling Receive packets */
static int fec_rx_poll(struct napi_struct *napi, int budget)
{
	struct  fec_enet_private *fep =
		container_of(napi, struct fec_enet_private, napi);
	struct net_device *ndev = napi->dev;
	struct  fec_ptp_private *fpp = fep->ptp_priv;
	const struct platform_device_id *id_entry =
				platform_get_device_id(fep->pdev);
	int pkt_received = 0;
	struct bufdesc *bdp;
	unsigned short status;
	struct	sk_buff	*skb;
	ushort	pkt_len;
	__u8 *data;

	if (fep->use_napi)
		WARN_ON(!budget);

#ifdef CONFIG_M532x
	flush_cache_all();
#endif

	/* First, grab all of the stats for the incoming packet.
	 * These get messed up if we get called due to a busy condition.
	 */
	bdp = fep->cur_rx;

	while (!((status = bdp->cbd_sc) & BD_ENET_RX_EMPTY)) {
		if (pkt_received >= budget)
			break;
		pkt_received++;

		/* Since we have allocated space to hold a complete frame,
		 * the last indicator should be set.
		 */
		if ((status & BD_ENET_RX_LAST) == 0)
			dev_err(&ndev->dev, "FEC ENET: rcv is not +last\n");

		if (!fep->opened)
			goto rx_processing_done;

		/* Check for errors. */
		if (status & (BD_ENET_RX_LG | BD_ENET_RX_SH | BD_ENET_RX_NO |
			   BD_ENET_RX_CR | BD_ENET_RX_OV)) {
			ndev->stats.rx_errors++;
			if (status & (BD_ENET_RX_LG | BD_ENET_RX_SH)) {
				/* Frame too long or too short. */
				ndev->stats.rx_length_errors++;
			}
			if (status & BD_ENET_RX_NO)	/* Frame alignment */
				ndev->stats.rx_frame_errors++;
			if (status & BD_ENET_RX_CR)	/* CRC Error */
				ndev->stats.rx_crc_errors++;
			if (status & BD_ENET_RX_OV)	/* FIFO overrun */
				ndev->stats.rx_fifo_errors++;
		}

		/* Report late collisions as a frame error.
		 * On this error, the BD is closed, but we don't know what we
		 * have in the buffer.  So, just drop this frame on the floor.
		 */
		if (status & BD_ENET_RX_CL) {
			ndev->stats.rx_errors++;
			ndev->stats.rx_frame_errors++;
			goto rx_processing_done;
		}

		/* Process the incoming frame. */
		ndev->stats.rx_packets++;
		pkt_len = bdp->cbd_datlen;
		ndev->stats.rx_bytes += pkt_len;
		data = (__u8 *)__va(bdp->cbd_bufaddr);

		if (bdp->cbd_bufaddr)
			dma_unmap_single(&fep->pdev->dev, bdp->cbd_bufaddr,
				FEC_ENET_RX_FRSIZE, DMA_FROM_DEVICE);

		if (id_entry->driver_data & FEC_QUIRK_SWAP_FRAME)
			swap_buffer(data, pkt_len);

		/* This does 16 byte alignment, exactly what we need.
		 * The packet length includes FCS, but we don't want to
		 * include that when passing upstream as it messes up
		 * bridging applications.
		 */
		skb = dev_alloc_skb(pkt_len - 4 + NET_IP_ALIGN);

		if (unlikely(!skb)) {
			dev_err(&ndev->dev,
			"%s: Memory squeeze, dropping packet.\n", ndev->name);
			ndev->stats.rx_dropped++;
		} else {
			skb_reserve(skb, NET_IP_ALIGN);
			skb_put(skb, pkt_len - 4);	/* Make room */
			skb_copy_to_linear_data(skb, data, pkt_len - 4);
			/* 1588 messeage TS handle */
			if (fep->ptimer_present)
				fec_ptp_store_rxstamp(fpp, skb, bdp);
			skb->protocol = eth_type_trans(skb, ndev);
			netif_receive_skb(skb);
		}

		bdp->cbd_bufaddr = dma_map_single(&fep->pdev->dev, data,
				FEC_ENET_RX_FRSIZE, DMA_FROM_DEVICE);
rx_processing_done:
		/* Clear the status flags for this buffer */
		status &= ~BD_ENET_RX_STATS;

		/* Mark the buffer empty */
		status |= BD_ENET_RX_EMPTY;
		bdp->cbd_sc = status;
#ifdef CONFIG_ENHANCED_BD
		bdp->cbd_esc = BD_ENET_RX_INT;
		bdp->cbd_prot = 0;
		bdp->cbd_bdu = 0;
#endif

		/* Update BD pointer to next entry */
		if (status & BD_ENET_RX_WRAP)
			bdp = fep->rx_bd_base;
		else
			bdp++;
		/* Doing this here will keep the FEC running while we process
		 * incoming frames.  On a heavily loaded network, we should be
		 * able to keep up at the expense of system resources.
		 */
		writel(0, fep->hwp + FEC_R_DES_ACTIVE);
	}
	fep->cur_rx = bdp;

	if (pkt_received < budget) {
		napi_complete(napi);
		fec_rx_int_is_enabled(ndev, true);
	}

	return pkt_received;
}

/* During a receive, the cur_rx points to the current incoming buffer.
 * When we update through the ring, if the next incoming buffer has
 * not been given to the system, we just set the empty indicator,
 * effectively tossing the packet.
 */
static void
fec_enet_rx(struct net_device *ndev)
{
	struct fec_enet_private *fep = netdev_priv(ndev);
	struct  fec_ptp_private *fpp = fep->ptp_priv;
	const struct platform_device_id *id_entry =
				platform_get_device_id(fep->pdev);
	struct bufdesc *bdp;
	unsigned short status;
	struct	sk_buff	*skb;
	ushort	pkt_len;
	__u8 *data;

#ifdef CONFIG_M532x
	flush_cache_all();
#endif

	/* First, grab all of the stats for the incoming packet.
	 * These get messed up if we get called due to a busy condition.
	 */
	bdp = fep->cur_rx;

	while (!((status = bdp->cbd_sc) & BD_ENET_RX_EMPTY)) {

		/* Since we have allocated space to hold a complete frame,
		 * the last indicator should be set.
		 */
		if ((status & BD_ENET_RX_LAST) == 0)
			printk("FEC ENET: rcv is not +last\n");

		if (!fep->opened)
			goto rx_processing_done;

		/* Check for errors. */
		if (status & (BD_ENET_RX_LG | BD_ENET_RX_SH | BD_ENET_RX_NO |
			   BD_ENET_RX_CR | BD_ENET_RX_OV)) {
			ndev->stats.rx_errors++;
			if (status & (BD_ENET_RX_LG | BD_ENET_RX_SH)) {
				/* Frame too long or too short. */
				ndev->stats.rx_length_errors++;
			}
			if (status & BD_ENET_RX_NO)	/* Frame alignment */
				ndev->stats.rx_frame_errors++;
			if (status & BD_ENET_RX_CR)	/* CRC Error */
				ndev->stats.rx_crc_errors++;
			if (status & BD_ENET_RX_OV)	/* FIFO overrun */
				ndev->stats.rx_fifo_errors++;
		}

		/* Report late collisions as a frame error.
		 * On this error, the BD is closed, but we don't know what we
		 * have in the buffer.  So, just drop this frame on the floor.
		 */
		if (status & BD_ENET_RX_CL) {
			ndev->stats.rx_errors++;
			ndev->stats.rx_frame_errors++;
			goto rx_processing_done;
		}

		/* Process the incoming frame. */
		ndev->stats.rx_packets++;
		pkt_len = bdp->cbd_datlen;
		ndev->stats.rx_bytes += pkt_len;
		data = (__u8*)__va(bdp->cbd_bufaddr);

		if (bdp->cbd_bufaddr)
			dma_unmap_single(&fep->pdev->dev, bdp->cbd_bufaddr,
				FEC_ENET_TX_FRSIZE, DMA_FROM_DEVICE);

		if (id_entry->driver_data & FEC_QUIRK_SWAP_FRAME)
			swap_buffer(data, pkt_len);

		/* This does 16 byte alignment, exactly what we need.
		 * The packet length includes FCS, but we don't want to
		 * include that when passing upstream as it messes up
		 * bridging applications.
		 */
		skb = dev_alloc_skb(pkt_len - 4 + NET_IP_ALIGN);

		if (unlikely(!skb)) {
			printk("%s: Memory squeeze, dropping packet.\n",
					ndev->name);
			ndev->stats.rx_dropped++;
		} else {
			skb_reserve(skb, NET_IP_ALIGN);
			skb_put(skb, pkt_len - 4);	/* Make room */
			skb_copy_to_linear_data(skb, data, pkt_len - 4);
			/* 1588 messeage TS handle */
			if (fep->ptimer_present)
				fec_ptp_store_rxstamp(fpp, skb, bdp);
			skb->protocol = eth_type_trans(skb, ndev);
			netif_rx(skb);
		}

		bdp->cbd_bufaddr = dma_map_single(&fep->pdev->dev, data,
				FEC_ENET_TX_FRSIZE, DMA_FROM_DEVICE);
rx_processing_done:
		/* Clear the status flags for this buffer */
		status &= ~BD_ENET_RX_STATS;

		/* Mark the buffer empty */
		status |= BD_ENET_RX_EMPTY;
		bdp->cbd_sc = status;
#ifdef CONFIG_ENHANCED_BD
		bdp->cbd_esc = BD_ENET_RX_INT;
		bdp->cbd_prot = 0;
		bdp->cbd_bdu = 0;
#endif

		/* Update BD pointer to next entry */
		if (status & BD_ENET_RX_WRAP)
			bdp = fep->rx_bd_base;
		else
			bdp++;
		/* Doing this here will keep the FEC running while we process
		 * incoming frames.  On a heavily loaded network, we should be
		 * able to keep up at the expense of system resources.
		 */
		writel(0, fep->hwp + FEC_R_DES_ACTIVE);
	}
	fep->cur_rx = bdp;
}

static irqreturn_t
fec_enet_interrupt(int irq, void *dev_id)
{
	struct net_device *ndev = dev_id;
	struct fec_enet_private *fep = netdev_priv(ndev);
	struct fec_ptp_private *fpp = fep->ptp_priv;
	uint int_events;
	ulong flags;
	irqreturn_t ret = IRQ_NONE;

	do {
		int_events = readl(fep->hwp + FEC_IEVENT);
		writel(int_events, fep->hwp + FEC_IEVENT);

		if (int_events & FEC_ENET_RXF) {
			ret = IRQ_HANDLED;
			spin_lock_irqsave(&fep->hw_lock, flags);

			if (fep->use_napi) {
				/* Disable the RX interrupt */
				if (napi_schedule_prep(&fep->napi)) {
					fec_rx_int_is_enabled(ndev, false);
					__napi_schedule(&fep->napi);
				}
			} else
				fec_enet_rx(ndev);

			spin_unlock_irqrestore(&fep->hw_lock, flags);
		}

		/* Transmit OK, or non-fatal error. Update the buffer
		 * descriptors. FEC handles all errors, we just discover
		 * them as part of the transmit process.
		 */
		if (int_events & FEC_ENET_TXF) {
			ret = IRQ_HANDLED;
			fec_enet_tx(ndev);
		}

		if (int_events & FEC_ENET_TS_TIMER) {
			ret = IRQ_HANDLED;
			if (fep->ptimer_present && fpp)
				fpp->prtc++;
		}

		if (int_events & FEC_ENET_MII) {
			ret = IRQ_HANDLED;
			complete(&fep->mdio_done);
		}
	} while (int_events);

	return ret;
}



/* ------------------------------------------------------------------------- */
static void __inline__ fec_get_mac(struct net_device *ndev)
{
	struct fec_enet_private *fep = netdev_priv(ndev);
	struct fec_platform_data *pdata = fep->pdev->dev.platform_data;
	unsigned char *iap, tmpaddr[ETH_ALEN];

	/*
	 * try to get mac address in following order:
	 *
	 * 1) module parameter via kernel command line in form
	 *    fec.macaddr=0x00,0x04,0x9f,0x01,0x30,0xe0
	 */
	iap = macaddr;

	/*
	 * 2) from flash or fuse (via platform data)
	 */
	if (!is_valid_ether_addr(iap)) {
#ifdef CONFIG_M5272
		if (FEC_FLASHMAC)
			iap = (unsigned char *)FEC_FLASHMAC;
#else
		if (pdata)
			memcpy(iap, pdata->mac, ETH_ALEN);
#endif
	}

	/*
	 * 3) FEC mac registers set by bootloader
	 */
	if (!is_valid_ether_addr(iap)) {
		*((unsigned long *) &tmpaddr[0]) =
			be32_to_cpu(readl(fep->hwp + FEC_ADDR_LOW));
		*((unsigned short *) &tmpaddr[4]) =
			be16_to_cpu(readl(fep->hwp + FEC_ADDR_HIGH) >> 16);
		iap = &tmpaddr[0];
	}

	memcpy(ndev->dev_addr, iap, ETH_ALEN);

	/* Adjust MAC if using macaddr */
	if (iap == macaddr)
		 ndev->dev_addr[ETH_ALEN-1] = macaddr[ETH_ALEN-1] + fep->pdev->id;
}

/* ------------------------------------------------------------------------- */

/*
 * Phy section
 */
static void fec_enet_adjust_link(struct net_device *ndev)
{
	struct fec_enet_private *fep = netdev_priv(ndev);
	struct phy_device *phy_dev = fep->phy_dev;
	struct fec_platform_data *pdata = fep->pdev->dev.platform_data;
	unsigned long flags;

	int status_change = 0;

	spin_lock_irqsave(&fep->hw_lock, flags);

	/* Prevent a state halted on mii error */
	if (fep->mii_timeout && phy_dev->state == PHY_HALTED) {
		phy_dev->state = PHY_RESUMING;
		goto spin_unlock;
	}

	/* Duplex link change */
	if (phy_dev->link) {
		if (fep->full_duplex != phy_dev->duplex) {
			fec_restart(ndev, phy_dev->duplex);
			status_change = 1;
		}
	}

	/* Link on or off change */
	if (phy_dev->link != fep->link) {
		fep->link = phy_dev->link;
		if (phy_dev->link) {
			fec_restart(ndev, phy_dev->duplex);
			if (!fep->tx_full)
				netif_wake_queue(ndev);
		} else
			fec_stop(ndev);
		status_change = 1;
	}

spin_unlock:
	spin_unlock_irqrestore(&fep->hw_lock, flags);

	if (status_change) {
		if (!phy_dev->link && phy_dev && pdata && pdata->power_hibernate)
			pdata->power_hibernate(phy_dev);
		phy_print_status(phy_dev);
	}
}

static int fec_enet_mdio_read(struct mii_bus *bus, int mii_id, int regnum)
{
	struct fec_enet_private *fep = bus->priv;
	unsigned long time_left;

	fep->mii_timeout = 0;
	init_completion(&fep->mdio_done);

	/* start a read op */
	writel(FEC_MMFR_ST | FEC_MMFR_OP_READ |
		FEC_MMFR_PA(mii_id) | FEC_MMFR_RA(regnum) |
		FEC_MMFR_TA, fep->hwp + FEC_MII_DATA);

	/* wait for end of transfer */
	time_left = wait_for_completion_timeout(&fep->mdio_done,
			msecs_to_jiffies(FEC_MII_TIMEOUT));
	if (time_left == 0) {
		fep->mii_timeout = 1;
		printk(KERN_ERR "FEC: MDIO read timeout, mii_id=%d\n", mii_id);
		return -ETIMEDOUT;
	}

	/* return value */
	return FEC_MMFR_DATA(readl(fep->hwp + FEC_MII_DATA));
}

static int fec_enet_mdio_write(struct mii_bus *bus, int mii_id, int regnum,
			   u16 value)
{
	struct fec_enet_private *fep = bus->priv;
	unsigned long time_left;

	fep->mii_timeout = 0;
	init_completion(&fep->mdio_done);

	/* start a write op */
	writel(FEC_MMFR_ST | FEC_MMFR_OP_WRITE |
		FEC_MMFR_PA(mii_id) | FEC_MMFR_RA(regnum) |
		FEC_MMFR_TA | FEC_MMFR_DATA(value),
		fep->hwp + FEC_MII_DATA);

	/* wait for end of transfer */
	time_left = wait_for_completion_timeout(&fep->mdio_done,
			msecs_to_jiffies(FEC_MII_TIMEOUT));
	if (time_left == 0) {
		fep->mii_timeout = 1;
		printk(KERN_ERR "FEC: MDIO write timeout, mii_id=%d\n", mii_id);
		return -ETIMEDOUT;
	}

	return 0;
}

static int fec_enet_mdio_reset(struct mii_bus *bus)
{
	return 0;
}

static int fec_enet_mii_probe(struct net_device *ndev)
{
	struct fec_enet_private *fep = netdev_priv(ndev);
	struct phy_device *phy_dev = NULL;
	char mdio_bus_id[MII_BUS_ID_SIZE];
	char phy_name[MII_BUS_ID_SIZE + 3];
	int phy_id;
	int dev_id = fep->pdev->id;

	fep->phy_dev = NULL;

	/* check for attached phy */
	for (phy_id = 0; (phy_id < PHY_MAX_ADDR); phy_id++) {
		if ((fep->mii_bus->phy_mask & (1 << phy_id)))
			continue;
		if (fep->mii_bus->phy_map[phy_id] == NULL)
			continue;
		if (fep->mii_bus->phy_map[phy_id]->phy_id == 0)
			continue;
		if (dev_id--)
			continue;
		strncpy(mdio_bus_id, fep->mii_bus->id, MII_BUS_ID_SIZE);
		break;
	}

	if (phy_id >= PHY_MAX_ADDR) {
		printk(KERN_INFO "%s: no PHY, assuming direct connection "
			"to switch\n", ndev->name);
		strncpy(mdio_bus_id, "0", MII_BUS_ID_SIZE);
		phy_id = 0;
	}

	snprintf(phy_name, MII_BUS_ID_SIZE, PHY_ID_FMT, mdio_bus_id, phy_id);
	phy_dev = phy_connect(ndev, phy_name, &fec_enet_adjust_link, 0,
		fep->phy_interface);

	if (IS_ERR(phy_dev)) {
		printk(KERN_ERR "%s: could not attach to PHY\n", ndev->name);
		return PTR_ERR(phy_dev);
	}

	/* mask with MAC supported features */
	if (cpu_is_mx6q() || cpu_is_mx6dl())
		phy_dev->supported &= PHY_GBIT_FEATURES;
	else
		phy_dev->supported &= PHY_BASIC_FEATURES;

	/* enable phy pause frame for any platform */
	phy_dev->supported |= ADVERTISED_Pause;

	phy_dev->advertising = phy_dev->supported;

	fep->phy_dev = phy_dev;
	fep->link = 0;
	fep->full_duplex = 0;

	printk(KERN_INFO "%s: Freescale FEC PHY driver [%s] "
		"(mii_bus:phy_addr=%s, irq=%d)\n", ndev->name,
		fep->phy_dev->drv->name, dev_name(&fep->phy_dev->dev),
		fep->phy_dev->irq);

	return 0;
}

static int fec_enet_mii_init(struct platform_device *pdev)
{
	static struct mii_bus *fec0_mii_bus;
	struct net_device *ndev = platform_get_drvdata(pdev);
	struct fec_enet_private *fep = netdev_priv(ndev);
	const struct platform_device_id *id_entry =
				platform_get_device_id(fep->pdev);
	int err = -ENXIO, i;

	/*
	 * The dual fec interfaces are not equivalent with enet-mac.
	 * Here are the differences:
	 *
	 *  - fec0 supports MII & RMII modes while fec1 only supports RMII
	 *  - fec0 acts as the 1588 time master while fec1 is slave
	 *  - external phys can only be configured by fec0
	 *
	 * That is to say fec1 can not work independently. It only works
	 * when fec0 is working. The reason behind this design is that the
	 * second interface is added primarily for Switch mode.
	 *
	 * Because of the last point above, both phys are attached on fec0
	 * mdio interface in board design, and need to be configured by
	 * fec0 mii_bus.
	 */
	if ((id_entry->driver_data & FEC_QUIRK_ENET_MAC) && pdev->id) {
		/* fec1 uses fec0 mii_bus */
		fep->mii_bus = fec0_mii_bus;
		return 0;
	}

	fep->mii_timeout = 0;

	/*
	 * Set MII speed to 2.5 MHz (= clk_get_rate() / 2 * phy_speed)
	 */
	fep->phy_speed = DIV_ROUND_UP(clk_get_rate(fep->mdc_clk),
					(FEC_ENET_MII_CLK << 2)) << 1;
	/* set hold time to 2 internal clock cycle */
	if (cpu_is_mx6q() || cpu_is_mx6dl())
		fep->phy_speed |= FEC_ENET_HOLD_TIME;

	writel(fep->phy_speed, fep->hwp + FEC_MII_SPEED);

	fep->mii_bus = mdiobus_alloc();
	if (fep->mii_bus == NULL) {
		err = -ENOMEM;
		goto err_out;
	}

	fep->mii_bus->name = "fec_enet_mii_bus";
	fep->mii_bus->read = fec_enet_mdio_read;
	fep->mii_bus->write = fec_enet_mdio_write;
	fep->mii_bus->reset = fec_enet_mdio_reset;
	snprintf(fep->mii_bus->id, MII_BUS_ID_SIZE, "%x", pdev->id + 1);
	fep->mii_bus->priv = fep;
	fep->mii_bus->parent = &pdev->dev;

	fep->mii_bus->irq = kmalloc(sizeof(int) * PHY_MAX_ADDR, GFP_KERNEL);
	if (!fep->mii_bus->irq) {
		err = -ENOMEM;
		goto err_out_free_mdiobus;
	}

	for (i = 0; i < PHY_MAX_ADDR; i++)
		fep->mii_bus->irq[i] = PHY_POLL;

	if (mdiobus_register(fep->mii_bus))
		goto err_out_free_mdio_irq;

	/* save fec0 mii_bus */
	if (id_entry->driver_data & FEC_QUIRK_ENET_MAC)
		fec0_mii_bus = fep->mii_bus;

	return 0;

err_out_free_mdio_irq:
	kfree(fep->mii_bus->irq);
err_out_free_mdiobus:
	mdiobus_free(fep->mii_bus);
err_out:
	return err;
}

static void fec_enet_mii_remove(struct fec_enet_private *fep)
{
	if (fep->phy_dev)
		phy_disconnect(fep->phy_dev);
	mdiobus_unregister(fep->mii_bus);
	kfree(fep->mii_bus->irq);
	mdiobus_free(fep->mii_bus);
}

static int fec_enet_get_settings(struct net_device *ndev,
				  struct ethtool_cmd *cmd)
{
	struct fec_enet_private *fep = netdev_priv(ndev);
	struct phy_device *phydev = fep->phy_dev;

	if (!phydev)
		return -ENODEV;

	return phy_ethtool_gset(phydev, cmd);
}

static int fec_enet_set_settings(struct net_device *ndev,
				 struct ethtool_cmd *cmd)
{
	struct fec_enet_private *fep = netdev_priv(ndev);
	struct phy_device *phydev = fep->phy_dev;

	if (!phydev)
		return -ENODEV;

	return phy_ethtool_sset(phydev, cmd);
}

static void fec_enet_get_drvinfo(struct net_device *ndev,
				 struct ethtool_drvinfo *info)
{
	struct fec_enet_private *fep = netdev_priv(ndev);

	strcpy(info->driver, fep->pdev->dev.driver->name);
	strcpy(info->version, "Revision: 1.0");
	strcpy(info->bus_info, dev_name(&ndev->dev));
}

static struct ethtool_ops fec_enet_ethtool_ops = {
	.get_settings		= fec_enet_get_settings,
	.set_settings		= fec_enet_set_settings,
	.get_drvinfo		= fec_enet_get_drvinfo,
	.get_link		= ethtool_op_get_link,
};

static int fec_enet_ioctl(struct net_device *ndev, struct ifreq *rq, int cmd)
{
	struct fec_enet_private *fep = netdev_priv(ndev);
	struct fec_ptp_private *priv = fep->ptp_priv;
	struct phy_device *phydev = fep->phy_dev;
	int retVal = 0;

	if (!netif_running(ndev))
		return -EINVAL;

	if (!phydev)
		return -ENODEV;

	if ((cmd >= PTP_ENBL_TXTS_IOCTL) &&
			(cmd <= PTP_FLUSH_TIMESTAMP)) {
		if (fep->ptimer_present)
			retVal = fec_ptp_ioctl(priv, rq, cmd);
		else
			retVal = -ENODEV;
	} else
		retVal = phy_mii_ioctl(phydev, rq, cmd);

	return retVal;
}

static void fec_enet_free_buffers(struct net_device *ndev)
{
	struct fec_enet_private *fep = netdev_priv(ndev);
	int i;
	struct sk_buff *skb;
	struct bufdesc	*bdp;

	bdp = fep->rx_bd_base;
	for (i = 0; i < RX_RING_SIZE; i++) {
		skb = fep->rx_skbuff[i];

		if (bdp->cbd_bufaddr)
			dma_unmap_single(&fep->pdev->dev, bdp->cbd_bufaddr,
					FEC_ENET_RX_FRSIZE, DMA_FROM_DEVICE);
		if (skb)
			dev_kfree_skb(skb);
		bdp++;
	}

	bdp = fep->tx_bd_base;
	for (i = 0; i < TX_RING_SIZE; i++)
		kfree(fep->tx_bounce[i]);
}

static int fec_enet_alloc_buffers(struct net_device *ndev)
{
	struct fec_enet_private *fep = netdev_priv(ndev);
	int i;
	struct sk_buff *skb;
	struct bufdesc	*bdp;

	bdp = fep->rx_bd_base;
	for (i = 0; i < RX_RING_SIZE; i++) {
		skb = dev_alloc_skb(FEC_ENET_RX_FRSIZE);
		if (!skb) {
			fec_enet_free_buffers(ndev);
			return -ENOMEM;
		}
		fep->rx_skbuff[i] = skb;

		bdp->cbd_bufaddr = dma_map_single(&fep->pdev->dev, skb->data,
				FEC_ENET_RX_FRSIZE, DMA_FROM_DEVICE);
		bdp->cbd_sc = BD_ENET_RX_EMPTY;
#ifdef CONFIG_ENHANCED_BD
		bdp->cbd_esc = BD_ENET_RX_INT;
#endif
		bdp++;
	}

	/* Set the last buffer to wrap. */
	bdp--;
	bdp->cbd_sc |= BD_SC_WRAP;

	bdp = fep->tx_bd_base;
	for (i = 0; i < TX_RING_SIZE; i++) {
		fep->tx_bounce[i] = kmalloc(FEC_ENET_TX_FRSIZE, GFP_KERNEL);
		if (!fep->tx_bounce[i]) {
			fec_enet_free_buffers(ndev);
			return -ENOMEM;
		}

		bdp->cbd_sc = 0;
		bdp->cbd_bufaddr = 0;
#ifdef CONFIG_ENHANCED_BD
		bdp->cbd_esc = BD_ENET_TX_INT;
#endif
		bdp++;
	}

	/* Set the last buffer to wrap. */
	bdp--;
	bdp->cbd_sc |= BD_SC_WRAP;

	return 0;
}

static int
fec_enet_open(struct net_device *ndev)
{
	struct fec_enet_private *fep = netdev_priv(ndev);
	struct fec_platform_data *pdata = fep->pdev->dev.platform_data;
	int ret;

	if (fep->use_napi)
		napi_enable(&fep->napi);

	/* I should reset the ring buffers here, but I don't yet know
	 * a simple way to do that.
	 */
	clk_enable(fep->clk);
	ret = fec_enet_alloc_buffers(ndev);
	if (ret)
		return ret;

	/* Probe and connect to PHY when open the interface */
	ret = fec_enet_mii_probe(ndev);
	if (ret) {
		fec_enet_free_buffers(ndev);
		return ret;
	}

	phy_start(fep->phy_dev);
	netif_start_queue(ndev);
	fep->opened = 1;

	ret = -EINVAL;
	if (pdata->init && pdata->init(fep->phy_dev))
		return ret;

	return 0;
}

static int
fec_enet_close(struct net_device *ndev)
{
	struct fec_enet_private *fep = netdev_priv(ndev);

	fep->opened = 0;
	if (fep->use_napi)
		napi_disable(&fep->napi);

	fec_stop(ndev);

	if (fep->phy_dev) {
		phy_stop(fep->phy_dev);
		phy_disconnect(fep->phy_dev);
	}

	fec_enet_free_buffers(ndev);

	/* Clock gate close for saving power */
	clk_disable(fep->clk);

	return 0;
}

/* Set or clear the multicast filter for this adaptor.
 * Skeleton taken from sunlance driver.
 * The CPM Ethernet implementation allows Multicast as well as individual
 * MAC address filtering.  Some of the drivers check to make sure it is
 * a group multicast address, and discard those that are not.  I guess I
 * will do the same for now, but just remove the test if you want
 * individual filtering as well (do the upper net layers want or support
 * this kind of feature?).
 */

#define HASH_BITS	6		/* #bits in hash */
#define CRC32_POLY	0xEDB88320

static void set_multicast_list(struct net_device *ndev)
{
	struct fec_enet_private *fep = netdev_priv(ndev);
	struct netdev_hw_addr *ha;
	unsigned int i, bit, data, crc, tmp;
	unsigned char hash;

	if (ndev->flags & IFF_PROMISC) {
		tmp = readl(fep->hwp + FEC_R_CNTRL);
		tmp |= 0x8;
		writel(tmp, fep->hwp + FEC_R_CNTRL);
		return;
	}

	tmp = readl(fep->hwp + FEC_R_CNTRL);
	tmp &= ~0x8;
	writel(tmp, fep->hwp + FEC_R_CNTRL);

	if (ndev->flags & IFF_ALLMULTI) {
		/* Catch all multicast addresses, so set the
		 * filter to all 1's
		 */
		writel(0xffffffff, fep->hwp + FEC_GRP_HASH_TABLE_HIGH);
		writel(0xffffffff, fep->hwp + FEC_GRP_HASH_TABLE_LOW);

		return;
	}

	/* Clear filter and add the addresses in hash register
	 */
	writel(0, fep->hwp + FEC_GRP_HASH_TABLE_HIGH);
	writel(0, fep->hwp + FEC_GRP_HASH_TABLE_LOW);

	netdev_for_each_mc_addr(ha, ndev) {
		/* Only support group multicast for now */
		if (!(ha->addr[0] & 1))
			continue;

		/* calculate crc32 value of mac address */
		crc = 0xffffffff;

		for (i = 0; i < ndev->addr_len; i++) {
			data = ha->addr[i];
			for (bit = 0; bit < 8; bit++, data >>= 1) {
				crc = (crc >> 1) ^
				(((crc ^ data) & 1) ? CRC32_POLY : 0);
			}
		}

		/* only upper 6 bits (HASH_BITS) are used
		 * which point to specific bit in he hash registers
		 */
		hash = (crc >> (32 - HASH_BITS)) & 0x3f;

		if (hash > 31) {
			tmp = readl(fep->hwp + FEC_GRP_HASH_TABLE_HIGH);
			tmp |= 1 << (hash - 32);
			writel(tmp, fep->hwp + FEC_GRP_HASH_TABLE_HIGH);
		} else {
			tmp = readl(fep->hwp + FEC_GRP_HASH_TABLE_LOW);
			tmp |= 1 << hash;
			writel(tmp, fep->hwp + FEC_GRP_HASH_TABLE_LOW);
		}
	}
}

/* Set a MAC change in hardware. */
static int
fec_set_mac_address(struct net_device *ndev, void *p)
{
	struct fec_enet_private *fep = netdev_priv(ndev);
	struct sockaddr *addr = p;

	if (!is_valid_ether_addr(addr->sa_data))
		return -EADDRNOTAVAIL;

	memcpy(ndev->dev_addr, addr->sa_data, ndev->addr_len);

	writel(ndev->dev_addr[3] | (ndev->dev_addr[2] << 8) |
		(ndev->dev_addr[1] << 16) | (ndev->dev_addr[0] << 24),
		fep->hwp + FEC_ADDR_LOW);
	writel((ndev->dev_addr[5] << 16) | (ndev->dev_addr[4] << 24),
		fep->hwp + FEC_ADDR_HIGH);
	return 0;
}

static const struct net_device_ops fec_netdev_ops = {
	.ndo_open		= fec_enet_open,
	.ndo_stop		= fec_enet_close,
	.ndo_start_xmit		= fec_enet_start_xmit,
	.ndo_set_multicast_list = set_multicast_list,
	.ndo_change_mtu		= eth_change_mtu,
	.ndo_validate_addr	= eth_validate_addr,
	.ndo_tx_timeout		= fec_timeout,
	.ndo_set_mac_address	= fec_set_mac_address,
	.ndo_do_ioctl		= fec_enet_ioctl,
#ifdef CONFIG_NET_POLL_CONTROLLER
	.ndo_poll_controller    = fec_enet_netpoll,
#endif
};

/* Init TX buffer descriptors
 */
static void fec_enet_txbd_init(struct net_device *dev)
{
	struct fec_enet_private *fep = netdev_priv(dev);
	struct bufdesc *bdp;
	int i;

	/* ...and the same for transmit */
	bdp = fep->tx_bd_base;
	for (i = 0; i < TX_RING_SIZE; i++) {

		/* Initialize the BD for every fragment in the page. */
		bdp->cbd_sc = 0;
		bdp++;
	}

	/* Set the last buffer to wrap */
	bdp--;
	bdp->cbd_sc |= BD_SC_WRAP;
}

 /*
  * XXX:  We need to clean up on failure exits here.
  *
  */
static int fec_enet_init(struct net_device *ndev)
{
	struct fec_enet_private *fep = netdev_priv(ndev);
	struct bufdesc *cbd_base;
	struct bufdesc *bdp;
	int i;

	/* Allocate memory for buffer descriptors. */
	cbd_base = dma_alloc_noncacheable(NULL, BUFDES_SIZE, &fep->bd_dma,
			GFP_KERNEL);
	if (!cbd_base) {
		printk("FEC: allocate descriptor memory failed?\n");
		return -ENOMEM;
	}

	spin_lock_init(&fep->hw_lock);

	fep->netdev = ndev;

	/* Get the Ethernet address */
	fec_get_mac(ndev);

	/* Set receive and transmit descriptor base. */
	fep->rx_bd_base = cbd_base;
	fep->tx_bd_base = cbd_base + RX_RING_SIZE;

	/* The FEC Ethernet specific entries in the device structure */
	ndev->watchdog_timeo = TX_TIMEOUT;
	ndev->netdev_ops = &fec_netdev_ops;
	ndev->ethtool_ops = &fec_enet_ethtool_ops;

	fep->use_napi = FEC_NAPI_ENABLE;
	fep->napi_weight = FEC_NAPI_WEIGHT;
	if (fep->use_napi) {
		fec_rx_int_is_enabled(ndev, false);
		netif_napi_add(ndev, &fep->napi, fec_rx_poll, fep->napi_weight);
	}

	/* Initialize the receive buffer descriptors. */
	bdp = fep->rx_bd_base;
	for (i = 0; i < RX_RING_SIZE; i++) {

		/* Initialize the BD for every fragment in the page. */
		bdp->cbd_sc = 0;
		bdp->cbd_bufaddr = 0;
		bdp++;
	}

	/* Set the last buffer to wrap */
	bdp--;
	bdp->cbd_sc |= BD_SC_WRAP;

	/* Init transmit descriptors */
	fec_enet_txbd_init(ndev);

	fec_restart(ndev, 0);

	return 0;
}

/* This function is called to start or restart the FEC during a link
 * change.  This only happens when switching between half and full
 * duplex.
 */
static void
fec_restart(struct net_device *dev, int duplex)
{
	struct fec_enet_private *fep = netdev_priv(dev);
	const struct platform_device_id *id_entry =
				platform_get_device_id(fep->pdev);
	int i, ret;
	u32 val, temp_mac[2], reg = 0;

	/* Whack a reset.  We should wait for this. */
	writel(1, fep->hwp + FEC_ECNTRL);
	udelay(10);

	/* if uboot don't set MAC address, get MAC address
	 * from command line; if command line don't set MAC
	 * address, get from OCOTP; otherwise, allocate random
	 * address.
	 */
	memcpy(&temp_mac, dev->dev_addr, ETH_ALEN);
	writel(cpu_to_be32(temp_mac[0]), fep->hwp + FEC_ADDR_LOW);
	writel(cpu_to_be32(temp_mac[1]), fep->hwp + FEC_ADDR_HIGH);

	/* Clear any outstanding interrupt. */
	writel(0xffc00000, fep->hwp + FEC_IEVENT);

	/* Reset all multicast.	*/
	writel(0, fep->hwp + FEC_GRP_HASH_TABLE_HIGH);
	writel(0, fep->hwp + FEC_GRP_HASH_TABLE_LOW);
#ifndef CONFIG_M5272
	writel(0, fep->hwp + FEC_HASH_TABLE_HIGH);
	writel(0, fep->hwp + FEC_HASH_TABLE_LOW);
#endif

	/* Set maximum receive buffer size. */
	writel(PKT_MAXBLR_SIZE, fep->hwp + FEC_R_BUFF_SIZE);

	/* Set receive and transmit descriptor base. */
	writel(fep->bd_dma, fep->hwp + FEC_R_DES_START);
	writel((unsigned long)fep->bd_dma + sizeof(struct bufdesc) * RX_RING_SIZE,
			fep->hwp + FEC_X_DES_START);
	/* Reinit transmit descriptors */
	fec_enet_txbd_init(dev);

	fep->dirty_tx = fep->cur_tx = fep->tx_bd_base;
	fep->cur_rx = fep->rx_bd_base;

	/* Reset SKB transmit buffers. */
	fep->skb_cur = fep->skb_dirty = 0;
	for (i = 0; i <= TX_RING_MOD_MASK; i++) {
		if (fep->tx_skbuff[i]) {
			dev_kfree_skb_any(fep->tx_skbuff[i]);
			fep->tx_skbuff[i] = NULL;
		}
	}

	/* Enable MII mode */
	if (duplex) {
		/* MII enable / FD enable */
		writel(OPT_FRAME_SIZE | 0x04, fep->hwp + FEC_R_CNTRL);
		writel(0x04, fep->hwp + FEC_X_CNTRL);
	} else {
		/* MII enable / No Rcv on Xmit */
		writel(OPT_FRAME_SIZE | 0x06, fep->hwp + FEC_R_CNTRL);
		writel(0x0, fep->hwp + FEC_X_CNTRL);
	}
	fep->full_duplex = duplex;

	/* Set MII speed */
	writel(fep->phy_speed, fep->hwp + FEC_MII_SPEED);

	/*
	 * The phy interface and speed need to get configured
	 * differently on enet-mac.
	 */
	if (id_entry->driver_data & FEC_QUIRK_ENET_MAC) {
		val = readl(fep->hwp + FEC_R_CNTRL);

		/* MII or RMII */
		if (fep->phy_interface == PHY_INTERFACE_MODE_RGMII)
			val |= (1 << 6);
		else if (fep->phy_interface == PHY_INTERFACE_MODE_RMII)
			val |= (1 << 8);
		else
			val &= ~(1 << 8);

		/* 10M or 100M */
		if (fep->phy_dev && fep->phy_dev->speed == SPEED_100)
			val &= ~(1 << 9);
		else
			val |= (1 << 9);

		/* Enable pause frame
		 * ENET pause frame has two issues as ticket TKT116501
		 * The issues have been fixed on Rigel TO1.1 and Arik TO1.2
		 */
		if ((cpu_is_mx6q() &&
			(mx6q_revision() >= IMX_CHIP_REVISION_1_2)) ||
			(cpu_is_mx6dl() &&
			(mx6dl_revision() >= IMX_CHIP_REVISION_1_1)))
			val |= FEC_ENET_FCE;

		writel(val, fep->hwp + FEC_R_CNTRL);
	}

	if (fep->ptimer_present) {
		/* Set Timer count */
		ret = fec_ptp_start(fep->ptp_priv);
		if (ret) {
			fep->ptimer_present = 0;
			reg = 0x0;
		} else
#if defined(CONFIG_SOC_IMX28) || defined(CONFIG_ARCH_MX6)
			reg = 0x00000010;
#else
			reg = 0x0;
#endif
	} else
		reg = 0x0;

	if (cpu_is_mx25() || cpu_is_mx53() || cpu_is_mx6sl()) {
		if (fep->phy_interface == PHY_INTERFACE_MODE_RMII) {
			/* disable the gasket and wait */
			writel(0, fep->hwp + FEC_MIIGSK_ENR);
			while (readl(fep->hwp + FEC_MIIGSK_ENR) & 4)
				udelay(1);

			/*
			 * configure the gasket:
			 *   RMII, 50 MHz, no loopback, no echo
			 */
			writel(1, fep->hwp + FEC_MIIGSK_CFGR);

			/* re-enable the gasket */
			writel(2, fep->hwp + FEC_MIIGSK_ENR);
			udelay(10);
			if (!(readl(fep->hwp + FEC_MIIGSK_ENR) & 4)) {
				udelay(100);
				if (!(readl(fep->hwp + FEC_MIIGSK_ENR) & 4))
					dev_err(&fep->pdev->dev,
						"switch to RMII failed!\n");
			}
		}
	}

	/* ENET enable */
	val = reg | (0x1 << 1);

	/* if phy work at 1G mode, set ENET RGMII speed to 1G */
	if (fep->phy_dev && (fep->phy_dev->supported &
		(SUPPORTED_1000baseT_Half | SUPPORTED_1000baseT_Full)) &&
		fep->phy_interface == PHY_INTERFACE_MODE_RGMII &&
		fep->phy_dev->speed == SPEED_1000)
		val |= (0x1 << 5);

	/* RX FIFO threshold setting for ENET pause frame feature
	 * Only set the parameters after ticket TKT116501 fixed.
	 * The issue has been fixed on Rigel TO1.1 and Arik TO1.2
	 */
	if ((cpu_is_mx6q() &&
		(mx6q_revision() >= IMX_CHIP_REVISION_1_2)) ||
		(cpu_is_mx6dl() &&
		(mx6dl_revision() >= IMX_CHIP_REVISION_1_1))) {
		writel(FEC_ENET_RSEM_V, fep->hwp + FEC_R_FIFO_RSEM);
		writel(FEC_ENET_RSFL_V, fep->hwp + FEC_R_FIFO_RSFL);
		writel(FEC_ENET_RAEM_V, fep->hwp + FEC_R_FIFO_RAEM);
		writel(FEC_ENET_RAFL_V, fep->hwp + FEC_R_FIFO_RAFL);

		/* OPD */
		writel(FEC_ENET_OPD_V, fep->hwp + FEC_OPD);
	}

	if (cpu_is_mx6q() || cpu_is_mx6dl()) {
		/* enable endian swap */
		val |= (0x1 << 8);
		/* enable ENET store and forward mode */
		writel(0x1 << 8, fep->hwp + FEC_X_WMRK);
	}
	writel(val, fep->hwp + FEC_ECNTRL);

	writel(0, fep->hwp + FEC_R_DES_ACTIVE);

	/* Enable interrupts we wish to service */
	if (cpu_is_mx6q() || cpu_is_mx6dl() || cpu_is_mx2() || cpu_is_mx3())
		val = (FEC_1588_IMASK | FEC_DEFAULT_IMASK);
	else
		val = FEC_DEFAULT_IMASK;
	writel(val, fep->hwp + FEC_IMASK);
}

static void
fec_stop(struct net_device *dev)
{
	struct fec_enet_private *fep = netdev_priv(dev);

	/* We cannot expect a graceful transmit stop without link !!! */
	if (fep->link) {
		writel(1, fep->hwp + FEC_X_CNTRL); /* Graceful transmit stop */
		udelay(10);
		if (!(readl(fep->hwp + FEC_IEVENT) & FEC_ENET_GRA))
			printk("fec_stop : Graceful transmit stop did not complete !\n");
	}

	/* Whack a reset.  We should wait for this. */
	writel(1, fep->hwp + FEC_ECNTRL);
	udelay(10);

	if (cpu_is_mx6q() || cpu_is_mx6dl())
		/* FIXME: we have to enable enet to keep mii interrupt works. */
		writel(2, fep->hwp + FEC_ECNTRL);

	writel(fep->phy_speed, fep->hwp + FEC_MII_SPEED);
	if (fep->ptimer_present)
		fec_ptp_stop(fep->ptp_priv);
	writel(FEC_DEFAULT_IMASK, fep->hwp + FEC_IMASK);

	if (netif_running(dev))
		netif_stop_queue(dev);
	netif_carrier_off(dev);     /* prevent tx timeout */
	fep->link = 0;
}

static int __devinit
fec_probe(struct platform_device *pdev)
{
	struct fec_enet_private *fep;
	struct fec_platform_data *pdata;
	struct net_device *ndev;
	int i, irq, ret = 0;
	struct resource *r;

	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!r)
		return -ENXIO;

	r = request_mem_region(r->start, resource_size(r), pdev->name);
	if (!r)
		return -EBUSY;

	/* Init network device */
	ndev = alloc_etherdev(sizeof(struct fec_enet_private));
	if (!ndev) {
		ret = -ENOMEM;
		goto failed_alloc_etherdev;
	}

	SET_NETDEV_DEV(ndev, &pdev->dev);

	/* setup board info structure */
	fep = netdev_priv(ndev);

	fep->hwp = ioremap(r->start, resource_size(r));
	fep->pdev = pdev;

	if (!fep->hwp) {
		ret = -ENOMEM;
		goto failed_ioremap;
	}

	platform_set_drvdata(pdev, ndev);

	pdata = pdev->dev.platform_data;
	if (pdata)
		fep->phy_interface = pdata->phy;

	if (pdata->gpio_irq > 0) {
		gpio_request(pdata->gpio_irq, "gpio_enet_irq");
		gpio_direction_input(pdata->gpio_irq);

		irq = gpio_to_irq(pdata->gpio_irq);
		ret = request_irq(irq, fec_enet_interrupt,
				IRQF_TRIGGER_RISING,
				 pdev->name, ndev);
		if (ret)
			goto failed_irq;
	} else {
		/* This device has up to three irqs on some platforms */
		for (i = 0; i < 3; i++) {
			irq = platform_get_irq(pdev, i);
			if (i && irq < 0)
				break;
			ret = request_irq(irq, fec_enet_interrupt,
					IRQF_DISABLED, pdev->name, ndev);
			if (ret) {
				while (--i >= 0) {
					irq = platform_get_irq(pdev, i);
					free_irq(irq, ndev);
				}
				goto failed_irq;
			}
		}
	}

	fep->clk = clk_get(&pdev->dev, "fec_clk");
	if (IS_ERR(fep->clk)) {
		ret = PTR_ERR(fep->clk);
		goto failed_clk;
	}
	fep->mdc_clk = clk_get(&pdev->dev, "fec_mdc_clk");
	if (IS_ERR(fep->mdc_clk)) {
		ret = PTR_ERR(fep->mdc_clk);
		goto failed_clk;
	}
	clk_enable(fep->clk);

	ret = fec_enet_init(ndev);
	if (ret)
		goto failed_init;

	ret = fec_enet_mii_init(pdev);
	if (ret)
		goto failed_mii_init;

	if (fec_ptp_malloc_priv(&(fep->ptp_priv))) {
		if (fep->ptp_priv) {
			fep->ptp_priv->hwp = fep->hwp;
			ret = fec_ptp_init(fep->ptp_priv, pdev->id);
			if (ret)
				printk(KERN_WARNING "IEEE1588: ptp-timer is unavailable\n");
			else
				fep->ptimer_present = 1;
		} else
			printk(KERN_ERR "IEEE1588: failed to malloc memory\n");
	}

	/* Carrier starts down, phylib will bring it up */
	netif_carrier_off(ndev);
	clk_disable(fep->clk);

	INIT_DELAYED_WORK(&fep->fixup_trigger_tx, fixup_trigger_tx_func);

	ret = register_netdev(ndev);
	if (ret)
		goto failed_register;

	return 0;

failed_register:
	fec_enet_mii_remove(fep);
	if (fep->ptimer_present)
		fec_ptp_cleanup(fep->ptp_priv);
	kfree(fep->ptp_priv);
failed_mii_init:
failed_init:
	clk_disable(fep->clk);
	clk_put(fep->clk);
	clk_put(fep->mdc_clk);
failed_clk:
	if (pdata->gpio_irq < 0)
		free_irq(irq, ndev);
	else {
		for (i = 0; i < 3; i++) {
			irq = platform_get_irq(pdev, i);
			if (irq > 0)
				free_irq(irq, ndev);
		}
	}
failed_irq:
	iounmap(fep->hwp);
failed_ioremap:
	free_netdev(ndev);
failed_alloc_etherdev:
	release_mem_region(r->start, resource_size(r));

	return ret;
}

static int __devexit
fec_drv_remove(struct platform_device *pdev)
{
	struct net_device *ndev = platform_get_drvdata(pdev);
	struct fec_enet_private *fep = netdev_priv(ndev);
	struct resource *r;

	cancel_delayed_work_sync(&fep->fixup_trigger_tx);
	fec_stop(ndev);
	fec_enet_mii_remove(fep);
	clk_disable(fep->clk);
	clk_put(fep->clk);
	clk_put(fep->mdc_clk);
	iounmap((void __iomem *)ndev->base_addr);
	if (fep->ptimer_present)
		fec_ptp_cleanup(fep->ptp_priv);
	kfree(fep->ptp_priv);
	unregister_netdev(ndev);
	free_netdev(ndev);

	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	BUG_ON(!r);
	release_mem_region(r->start, resource_size(r));

	platform_set_drvdata(pdev, NULL);

	return 0;
}

#ifdef CONFIG_PM
static int
fec_suspend(struct device *dev)
{
	struct net_device *ndev = dev_get_drvdata(dev);
	struct fec_enet_private *fep = netdev_priv(ndev);

	if (netif_running(ndev)) {
		netif_device_detach(ndev);
		fec_stop(ndev);
		clk_disable(fep->clk);
	}

	return 0;
}

static int
fec_resume(struct device *dev)
{
	struct net_device *ndev = dev_get_drvdata(dev);
	struct fec_enet_private *fep = netdev_priv(ndev);

	if (netif_running(ndev)) {
		clk_enable(fep->clk);
		fec_restart(ndev, fep->full_duplex);
		netif_device_attach(ndev);
	}

	return 0;
}

static const struct dev_pm_ops fec_pm_ops = {
	.suspend	= fec_suspend,
	.resume		= fec_resume,
	.freeze		= fec_suspend,
	.thaw		= fec_resume,
	.poweroff	= fec_suspend,
	.restore	= fec_resume,
};
#endif

static struct platform_driver fec_driver = {
	.driver	= {
		.name	= DRIVER_NAME,
		.owner	= THIS_MODULE,
#ifdef CONFIG_PM
		.pm	= &fec_pm_ops,
#endif
	},
	.id_table = fec_devtype,
	.probe	= fec_probe,
	.remove	= __devexit_p(fec_drv_remove),
};

static int fec_mac_addr_setup(char *mac_addr)
{
	char *ptr, *p = mac_addr;
	unsigned long tmp;
	int i = 0, ret = 0;

	while (p && (*p) && i < 6) {
		ptr = strchr(p, ':');
		if (ptr)
			*ptr++ = '\0';

		if (strlen(p)) {
			ret = strict_strtoul(p, 16, &tmp);
			if (ret < 0 || tmp > 0xff)
				break;
			macaddr[i++] = tmp;
		}
		p = ptr;
	}

	return 0;
}

__setup("fec_mac=", fec_mac_addr_setup);

static int __init
fec_enet_module_init(void)
{
	printk(KERN_INFO "FEC Ethernet Driver\n");

	return platform_driver_register(&fec_driver);
}

static void __exit
fec_enet_cleanup(void)
{
	platform_driver_unregister(&fec_driver);
}

module_exit(fec_enet_cleanup);
module_init(fec_enet_module_init);

MODULE_LICENSE("GPL");
