/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2009-2012 Cavium, Inc
 */

#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/capability.h>
#include <linux/net_tstamp.h>
#include <linux/interrupt.h>
#include <linux/netdevice.h>
#include <linux/spinlock.h>
#include <linux/of_mdio.h>
#include <linux/module.h>
#include <linux/of_net.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/phy.h>
#include <linux/io.h>
#include <linux/irqdomain.h>

#include <asm/bitfield.h>

#include <asm/octeon/octeon.h>
#include <asm/octeon/cvmx-mixx-defs.h>
#include <asm/octeon/cvmx-agl-defs.h>

#include "octeon-bgx.h"
#include "octeon_common.h"
#include "octeon-common-nexus.h"

#define DRV_NAME "octeon_mgmt"
#define DRV_VERSION "2.0"
#define DRV_DESCRIPTION \
	"Cavium Networks Octeon MII (management) port Network Driver"

#define OCTEON_MGMT_NAPI_WEIGHT 16

/* Ring sizes that are powers of two allow for more efficient modulo
 * opertions.
 */
#define OCTEON_MGMT_RX_RING_SIZE 4096
#define OCTEON_MGMT_TX_RING_SIZE 128

/* 14 bits of length */
#define OCTEON_MGMT_RE_LEN_MASK 0x3fffULL

/* 6 or 8 bits of code. 7th and 8th bits are always zero, so just look
 * at 6 bits.
 */
#define OCTEON_MGMT_RE_CODE_MASK 0x3fULL

#define RING_ENTRY_CODE_DONE 0xf
#define RING_ENTRY_CODE_MORE 0x10

#define MIX_ORING1	0x0
#define MIX_ORING2	0x8
#define MIX_IRING1	0x10
#define MIX_IRING2	0x18
#define MIX_CTL		0x20
#define MIX_IRHWM	0x28
#define MIX_IRCNT	0x30
#define MIX_ORHWM	0x38
#define MIX_ORCNT	0x40
#define MIX_ISR		0x48
#define MIX_INTENA	0x50
#define MIX_REMCNT	0x58
#define MIX_BIST	0x78

#define AGL_GMX_PRT_CFG			0x10
#define AGL_GMX_RX_FRM_CTL		0x18
#define AGL_GMX_RX_STATS_CTL		0x50

#define AGL_GMX_RX_STATS_PKTS_DRP	0xb0
#define AGL_GMX_RX_STATS_OCTS_DRP	0xb8
#define AGL_GMX_RX_STATS_PKTS_BAD	0xc0

#define AGL_GMX_TX_CLK			0x208
#define AGL_GMX_TX_STATS_CTL		0x268
#define AGL_GMX_TX_CTL			0x270
#define AGL_GMX_TX_STAT0		0x280
#define AGL_GMX_TX_STAT1		0x288
#define AGL_GMX_TX_STAT2		0x290
#define AGL_GMX_TX_STAT3		0x298
#define AGL_GMX_TX_STAT4		0x2a0
#define AGL_GMX_TX_STAT5		0x2a8
#define AGL_GMX_TX_STAT6		0x2b0
#define AGL_GMX_TX_STAT7		0x2b8
#define AGL_GMX_TX_STAT8		0x2c0
#define AGL_GMX_TX_STAT9		0x2c8

struct octeon_mgmt {
	/* bgx_priv Must be first element.  May be unused in the case
	 * where the MAC is not BGX
	 */
	struct bgx_port_netdev_priv bgx_priv;
	struct net_device *netdev;
	u64 mix;
	u64 agl;
	u64 agl_prt_ctl;
	int port;
	int numa_node;
	int irq;
	int irq_orthresh;
	int irq_irthresh;
	bool has_rx_tstamp;
	bool has_o3_structs;
	u64 *tx_ring;
	dma_addr_t tx_ring_handle;
	unsigned int tx_next;
	unsigned int tx_next_clean;
	unsigned int tx_current_fill;
	/* The tx_list lock also protects the ring related variables */
	struct sk_buff_head tx_list;

	/* RX variables only touched in napi_poll.  No locking necessary. */
	u64 *rx_ring;
	dma_addr_t rx_ring_handle;
	u64 re_addr_mask;
	u8 re_len_shift;
	u8 re_code_shift;
	u8 re_ts_bit;
	unsigned int rx_next;
	unsigned int rx_next_fill;
	unsigned int rx_current_fill;
	struct sk_buff_head rx_list;

	spinlock_t lock;
	unsigned int last_duplex;
	unsigned int last_link;
	unsigned int last_speed;
	struct device *dev;
	struct napi_struct napi;
	struct tasklet_struct tx_clean_tasklet;
	struct device_node *phy_np;
	resource_size_t mix_phys;
	resource_size_t mix_size;
	resource_size_t agl_phys;
	resource_size_t agl_size;
	resource_size_t agl_prt_ctl_phys;
	resource_size_t agl_prt_ctl_size;
};

static void octeon_mgmt_set_rx_irq(struct octeon_mgmt *p, int enable)
{
	union cvmx_mixx_intena mix_intena;
	unsigned long flags;

	spin_lock_irqsave(&p->lock, flags);
	mix_intena.u64 = cvmx_read_csr(p->mix + MIX_INTENA);
	mix_intena.s.ithena = enable ? 1 : 0;
	cvmx_write_csr(p->mix + MIX_INTENA, mix_intena.u64);
	spin_unlock_irqrestore(&p->lock, flags);
}

static void octeon_mgmt_set_tx_irq(struct octeon_mgmt *p, int enable)
{
	union cvmx_mixx_intena mix_intena;
	unsigned long flags;

	spin_lock_irqsave(&p->lock, flags);
	mix_intena.u64 = cvmx_read_csr(p->mix + MIX_INTENA);
	mix_intena.s.othena = enable ? 1 : 0;
	cvmx_write_csr(p->mix + MIX_INTENA, mix_intena.u64);
	spin_unlock_irqrestore(&p->lock, flags);
}

static void octeon_mgmt_enable_rx_irq(struct octeon_mgmt *p)
{
	if (p->has_o3_structs)
		enable_irq(p->irq_irthresh);
	else
		octeon_mgmt_set_rx_irq(p, 1);
}

static void octeon_mgmt_disable_rx_irq(struct octeon_mgmt *p)
{
	if (p->has_o3_structs)
		disable_irq_nosync(p->irq_irthresh);
	else
		octeon_mgmt_set_rx_irq(p, 0);
}

static void octeon_mgmt_enable_tx_irq(struct octeon_mgmt *p)
{
	if (p->has_o3_structs)
		enable_irq(p->irq_orthresh);
	else
		octeon_mgmt_set_tx_irq(p, 1);
}

static void octeon_mgmt_disable_tx_irq(struct octeon_mgmt *p)
{
	if (p->has_o3_structs)
		disable_irq_nosync(p->irq_orthresh);
	else
		octeon_mgmt_set_tx_irq(p, 0);
}

static unsigned int ring_max_fill(unsigned int ring_size)
{
	return ring_size - 8;
}

static unsigned int ring_size_to_bytes(unsigned int ring_size)
{
	return ring_size * sizeof(u64);
}

static void octeon_mgmt_rx_fill_ring(struct net_device *netdev)
{
	struct octeon_mgmt *p = netdev_priv(netdev);

	while (p->rx_current_fill < ring_max_fill(OCTEON_MGMT_RX_RING_SIZE)) {
		u64 size;
		u64 raw_re;
		struct sk_buff *skb;

		/* CN56XX pass 1 needs 8 bytes of padding.  */
		size = netdev->mtu + OCTEON_FRAME_HEADER_LEN + 8 + NET_IP_ALIGN;

		skb = netdev_alloc_skb(netdev, size);
		if (!skb)
			break;
		skb_reserve(skb, NET_IP_ALIGN);
		__skb_queue_tail(&p->rx_list, skb);

		raw_re = dma_map_single(p->dev, skb->data,
					size,
					DMA_FROM_DEVICE);
		raw_re |= (size << p->re_len_shift);

		/* Put it in the ring.  */
		p->rx_ring[p->rx_next_fill] = raw_re;
		/* Make sure there is no reorder of filling the ring and ringing
		 * the bell
		 */
		wmb();
		dma_sync_single_for_device(p->dev, p->rx_ring_handle,
					   ring_size_to_bytes(OCTEON_MGMT_RX_RING_SIZE),
					   DMA_BIDIRECTIONAL);
		p->rx_next_fill =
			(p->rx_next_fill + 1) % OCTEON_MGMT_RX_RING_SIZE;
		p->rx_current_fill++;
		/* Ring the bell.  */
		cvmx_write_csr(p->mix + MIX_IRING2, 1);
	}
}

static void octeon_mgmt_clean_tx_buffers(struct octeon_mgmt *p)
{
	union cvmx_mixx_orcnt mix_orcnt;
	u64 re;
	struct sk_buff *skb;
	int cleaned = 0;
	unsigned long flags;

	mix_orcnt.u64 = cvmx_read_csr(p->mix + MIX_ORCNT);

	while (mix_orcnt.s.orcnt) {
		spin_lock_irqsave(&p->tx_list.lock, flags);

		mix_orcnt.u64 = cvmx_read_csr(p->mix + MIX_ORCNT);

		if (mix_orcnt.s.orcnt == 0) {
			spin_unlock_irqrestore(&p->tx_list.lock, flags);
			break;
		}

		dma_sync_single_for_cpu(p->dev, p->tx_ring_handle,
					ring_size_to_bytes(OCTEON_MGMT_TX_RING_SIZE),
					DMA_BIDIRECTIONAL);

		re = p->tx_ring[p->tx_next_clean];
		p->tx_next_clean =
			(p->tx_next_clean + 1) % OCTEON_MGMT_TX_RING_SIZE;
		skb = __skb_dequeue(&p->tx_list);

		mix_orcnt.u64 = 0;
		mix_orcnt.s.orcnt = 1;

		/* Acknowledge to hardware that we have the buffer.  */
		cvmx_write_csr(p->mix + MIX_ORCNT, mix_orcnt.u64);
		p->tx_current_fill--;

		spin_unlock_irqrestore(&p->tx_list.lock, flags);

		dma_unmap_single(p->dev, re & p->re_addr_mask,
				 (re >> p->re_len_shift) & OCTEON_MGMT_RE_LEN_MASK,
				 DMA_TO_DEVICE);

		/* Read the hardware TX timestamp if one was recorded */
		if (unlikely((re >> p->re_ts_bit) & 1)) {
			struct skb_shared_hwtstamps ts;
			u64 ns;

			memset(&ts, 0, sizeof(ts));
			/* Read the timestamp */
			ns = cvmx_read_csr(CVMX_MIXX_TSTAMP(p->port));
			/* Remove the timestamp from the FIFO */
			cvmx_write_csr(CVMX_MIXX_TSCTL(p->port), 0);
			/* Tell the kernel about the timestamp */
			ts.hwtstamp = ns_to_ktime(ns);
			skb_tstamp_tx(skb, &ts);
		}

		dev_kfree_skb_any(skb);
		cleaned++;

		mix_orcnt.u64 = cvmx_read_csr(p->mix + MIX_ORCNT);
	}
	if (p->has_o3_structs) {
		union cvmx_mixx_isr mixx_isr;
		mixx_isr.u64 = 0;
		mixx_isr.s.orthresh = 1;
		cvmx_write_csr(p->mix + MIX_ISR, mixx_isr.u64);
	}

	if (cleaned && netif_queue_stopped(p->netdev))
		netif_wake_queue(p->netdev);
}

static void octeon_mgmt_clean_tx_tasklet(struct tasklet_struct *t)
{
	struct octeon_mgmt *p = from_tasklet(p, t, tx_clean_tasklet);
	octeon_mgmt_clean_tx_buffers(p);
	octeon_mgmt_enable_tx_irq(p);
}

static void octeon_mgmt_update_rx_stats(struct net_device *netdev)
{
	struct octeon_mgmt *p = netdev_priv(netdev);
	unsigned long flags;
	u64 drop, bad;

	if (p->has_o3_structs)
		return;

	/* These reads also clear the count registers.  */
	drop = cvmx_read_csr(p->agl + AGL_GMX_RX_STATS_PKTS_DRP);
	bad = cvmx_read_csr(p->agl + AGL_GMX_RX_STATS_PKTS_BAD);

	if (drop || bad) {
		/* Do an atomic update. */
		spin_lock_irqsave(&p->lock, flags);
		netdev->stats.rx_errors += bad;
		netdev->stats.rx_dropped += drop;
		spin_unlock_irqrestore(&p->lock, flags);
	}
}

static void octeon_mgmt_update_tx_stats(struct net_device *netdev)
{
	struct octeon_mgmt *p = netdev_priv(netdev);
	unsigned long flags;

	union cvmx_agl_gmx_txx_stat0 s0;
	union cvmx_agl_gmx_txx_stat1 s1;

	/* These reads also clear the count registers.  */
	s0.u64 = cvmx_read_csr(p->agl + AGL_GMX_TX_STAT0);
	s1.u64 = cvmx_read_csr(p->agl + AGL_GMX_TX_STAT1);

	if (s0.s.xsdef || s0.s.xscol || s1.s.scol || s1.s.mcol) {
		/* Do an atomic update. */
		spin_lock_irqsave(&p->lock, flags);
		netdev->stats.tx_errors += s0.s.xsdef + s0.s.xscol;
		netdev->stats.collisions += s1.s.scol + s1.s.mcol;
		spin_unlock_irqrestore(&p->lock, flags);
	}
}

/*
 * Dequeue a receive skb and its corresponding ring entry.  The ring
 * entry is returned, *pskb is updated to point to the skb.
 */
static u64 octeon_mgmt_dequeue_rx_buffer(struct octeon_mgmt *p,
					 struct sk_buff **pskb)
{
	u64 entry;

	dma_sync_single_for_cpu(p->dev, p->rx_ring_handle,
				ring_size_to_bytes(OCTEON_MGMT_RX_RING_SIZE),
				DMA_BIDIRECTIONAL);

	entry = p->rx_ring[p->rx_next];
	p->rx_next = (p->rx_next + 1) % OCTEON_MGMT_RX_RING_SIZE;
	p->rx_current_fill--;
	*pskb = __skb_dequeue(&p->rx_list);

	dma_unmap_single(p->dev, entry & p->re_addr_mask,
			 ETH_FRAME_LEN + OCTEON_FRAME_HEADER_LEN,
			 DMA_FROM_DEVICE);

	return entry;
}


static int octeon_mgmt_receive_one(struct octeon_mgmt *p)
{
	struct net_device *netdev = p->netdev;
	union cvmx_mixx_ircnt mix_ircnt;
	u64 re;
	int code, code2;
	int len, len2;
	struct sk_buff *skb;
	struct sk_buff *skb2;
	struct sk_buff *skb_new;
	u64 re2;
	int rc = 1;


	re = octeon_mgmt_dequeue_rx_buffer(p, &skb);
	code = (re >> p->re_code_shift) & OCTEON_MGMT_RE_CODE_MASK;
	len = (re >> p->re_len_shift) & OCTEON_MGMT_RE_LEN_MASK;
	if (likely(code == RING_ENTRY_CODE_DONE)) {
		/* A good packet, send it up. */
		skb_put(skb, len);
good:
		/* Process the RX timestamp if it was recorded */
		if (p->has_rx_tstamp) {
			/* The first 8 bytes are the timestamp */
			u64 ns = *(u64 *)skb->data;
			struct skb_shared_hwtstamps *ts;
			ts = skb_hwtstamps(skb);
			ts->hwtstamp = ns_to_ktime(ns);
			__skb_pull(skb, 8);
		}
		skb->protocol = eth_type_trans(skb, netdev);
		netdev->stats.rx_packets++;
		netdev->stats.rx_bytes += skb->len;
		netif_receive_skb(skb);
		rc = 0;
	} else if (code == RING_ENTRY_CODE_MORE) {
		/* Packet split across skbs.  This can happen if we
		 * increase the MTU.  Buffers that are already in the
		 * rx ring can then end up being too small.  As the rx
		 * ring is refilled, buffers sized for the new MTU
		 * will be used and we should go back to the normal
		 * non-split case.
		 */
		skb_put(skb, len);
		do {
			re2 = octeon_mgmt_dequeue_rx_buffer(p, &skb2);
			code2 = (re2 >> p->re_code_shift) & OCTEON_MGMT_RE_CODE_MASK;
			len2 = (re2 >> p->re_len_shift) & OCTEON_MGMT_RE_LEN_MASK;
			if (code2 != RING_ENTRY_CODE_MORE
				&& code2 != RING_ENTRY_CODE_DONE)
				goto split_error;
			skb_put(skb2,  len2);
			skb_new = skb_copy_expand(skb, 0, skb2->len,
						  GFP_ATOMIC);
			if (!skb_new)
				goto split_error;
			if (skb_copy_bits(skb2, 0, skb_tail_pointer(skb_new),
					  skb2->len))
				goto split_error;
			skb_put(skb_new, skb2->len);
			dev_kfree_skb_any(skb);
			dev_kfree_skb_any(skb2);
			skb = skb_new;
		} while (code2 == RING_ENTRY_CODE_MORE);
		goto good;
	} else {
		/* Some other error, discard it. */
		dev_kfree_skb_any(skb);
		/* Error statistics are accumulated in
		 * octeon_mgmt_update_rx_stats.
		 */
	}
	goto done;
split_error:
	/* Discard the whole mess. */
	dev_kfree_skb_any(skb);
	dev_kfree_skb_any(skb2);
	while (code2 == RING_ENTRY_CODE_MORE) {
		re2 = octeon_mgmt_dequeue_rx_buffer(p, &skb2);
		code2 = (re2 >> p->re_code_shift) & OCTEON_MGMT_RE_CODE_MASK;
		dev_kfree_skb_any(skb2);
	}
	netdev->stats.rx_errors++;

done:
	/* Tell the hardware we processed a packet.  */
	mix_ircnt.u64 = 0;
	mix_ircnt.s.ircnt = 1;
	cvmx_write_csr(p->mix + MIX_IRCNT, mix_ircnt.u64);
	return rc;
}

static int octeon_mgmt_receive_packets(struct octeon_mgmt *p, int budget)
{
	unsigned int work_done = 0;
	union cvmx_mixx_ircnt mix_ircnt;
	int rc;

	mix_ircnt.u64 = cvmx_read_csr(p->mix + MIX_IRCNT);
	while (work_done < budget && mix_ircnt.s.ircnt) {

		rc = octeon_mgmt_receive_one(p);
		if (!rc)
			work_done++;

		/* Check for more packets. */
		mix_ircnt.u64 = cvmx_read_csr(p->mix + MIX_IRCNT);
	}

	octeon_mgmt_rx_fill_ring(p->netdev);

	return work_done;
}

static int octeon_mgmt_napi_poll(struct napi_struct *napi, int budget)
{
	struct octeon_mgmt *p = container_of(napi, struct octeon_mgmt, napi);
	struct net_device *netdev = p->netdev;
	unsigned int work_done = 0;

	work_done = octeon_mgmt_receive_packets(p, budget);

	if (work_done < budget) {
		/* We stopped because no more packets were available. */
		napi_complete_done(napi, work_done);
		if (p->has_o3_structs) {
			union cvmx_mixx_isr mixx_isr;
			mixx_isr.u64 = 0;
			mixx_isr.s.irthresh = 1;
			cvmx_write_csr(p->mix + MIX_ISR, mixx_isr.u64);
		}
		octeon_mgmt_enable_rx_irq(p);
	}
	octeon_mgmt_update_rx_stats(netdev);

	return work_done;
}

/* Reset the hardware to clean state.  */
static void octeon_mgmt_common_reset_hw(struct octeon_mgmt *p)
{
	union cvmx_mixx_ctl mix_ctl;
	union cvmx_mixx_bist mix_bist;

	mix_ctl.u64 = 0;
	cvmx_write_csr(p->mix + MIX_CTL, mix_ctl.u64);
	do {
		mix_ctl.u64 = cvmx_read_csr(p->mix + MIX_CTL);
	} while (mix_ctl.s.busy);
	mix_ctl.s.reset = 1;
	cvmx_write_csr(p->mix + MIX_CTL, mix_ctl.u64);
	cvmx_read_csr(p->mix + MIX_CTL);
	octeon_io_clk_delay(64);

	mix_bist.u64 = cvmx_read_csr(p->mix + MIX_BIST);
	if (mix_bist.u64)
		dev_warn(p->dev, "MIX failed BIST (0x%016llx)\n",
			(unsigned long long)mix_bist.u64);

}

/* Reset the hardware to clean state.  */
static void octeon_mgmt_reset_hw(struct octeon_mgmt *p)
{
	union cvmx_agl_gmx_bist agl_gmx_bist;

	octeon_mgmt_common_reset_hw(p);

	agl_gmx_bist.u64 = cvmx_read_csr(CVMX_AGL_GMX_BIST);
	if (agl_gmx_bist.u64)
		dev_warn(p->dev, "AGL failed BIST (0x%016llx)\n",
			 (unsigned long long)agl_gmx_bist.u64);
}

static void octeon_mgmt_set_rx_filtering(struct net_device *netdev)
{
	struct octeon_mgmt *p = netdev_priv(netdev);

	cvm_oct_common_set_rx_filtering(netdev, p->agl, &p->lock);
}

static int octeon_mgmt_set_mac_address(struct net_device *netdev, void *addr)
{
	struct octeon_mgmt *p = netdev_priv(netdev);

	cvm_oct_common_set_mac_address(netdev, addr, p->agl, &p->lock);

	return 0;
}

static int octeon_mgmt_change_mtu(struct net_device *netdev, int new_mtu)
{
	struct octeon_mgmt *p = netdev_priv(netdev);

	/* Limit the MTU to make sure the ethernet packets are between
	 * 64 bytes and 16383 bytes.
	 */
	int ret = cvm_oct_common_change_mtu(netdev, new_mtu, p->agl, 0, 16383);

	if (ret)
		return ret;

	return 0;
}

static irqreturn_t octeon_mgmt_interrupt(int cpl, void *dev_id)
{
	struct net_device *netdev = dev_id;
	struct octeon_mgmt *p = netdev_priv(netdev);
	union cvmx_mixx_isr mixx_isr;

	mixx_isr.u64 = cvmx_read_csr(p->mix + MIX_ISR);

	/* Clear any pending interrupts */
	cvmx_write_csr(p->mix + MIX_ISR, mixx_isr.u64);
	cvmx_read_csr(p->mix + MIX_ISR);

	if (mixx_isr.s.irthresh) {
		octeon_mgmt_disable_rx_irq(p);
		napi_schedule(&p->napi);
	}
	if (mixx_isr.s.orthresh) {
		octeon_mgmt_disable_tx_irq(p);
		tasklet_schedule(&p->tx_clean_tasklet);
	}

	return IRQ_HANDLED;
}

static irqreturn_t octeon_mgmt_o3_or_handler(int irq, void *dev_id)
{
	struct octeon_mgmt *p = dev_id;

	octeon_mgmt_disable_tx_irq(p);
	tasklet_schedule(&p->tx_clean_tasklet);

	return IRQ_HANDLED;
}

static irqreturn_t octeon_mgmt_o3_ir_handler(int irq, void *dev_id)
{
	struct octeon_mgmt *p = dev_id;

	octeon_mgmt_disable_rx_irq(p);
	napi_schedule(&p->napi);

	return IRQ_HANDLED;
}

static int octeon_mgmt_ioctl_hwtstamp(struct net_device *netdev,
				      struct ifreq *rq, int cmd)
{
	struct octeon_mgmt *p = netdev_priv(netdev);
	struct hwtstamp_config config;
	union cvmx_mio_ptp_clock_cfg ptp;
	union cvmx_agl_gmx_rxx_frm_ctl rxx_frm_ctl;
	bool have_hw_timestamps = false;

	if (copy_from_user(&config, rq->ifr_data, sizeof(config)))
		return -EFAULT;

	if (config.flags) /* reserved for future extensions */
		return -EINVAL;

	/* Check the status of hardware for tiemstamps */
	if (OCTEON_IS_MODEL(OCTEON_CN6XXX)) {
		/* Get the current state of the PTP clock */
		ptp.u64 = cvmx_read_csr(CVMX_MIO_PTP_CLOCK_CFG);
		if (!ptp.s.ext_clk_en) {
			/* The clock has not been configured to use an
			 * external source.  Program it to use the main clock
			 * reference.
			 */
			u64 clock_comp = (NSEC_PER_SEC << 32) /	octeon_get_io_clock_rate();
			if (!ptp.s.ptp_en)
				cvmx_write_csr(CVMX_MIO_PTP_CLOCK_COMP, clock_comp);
			pr_info("PTP Clock: Using sclk reference at %lld Hz\n",
				(NSEC_PER_SEC << 32) / clock_comp);
		} else {
			/* The clock is already programmed to use a GPIO */
			u64 clock_comp = cvmx_read_csr(CVMX_MIO_PTP_CLOCK_COMP);
			pr_info("PTP Clock: Using GPIO %d at %lld Hz\n",
				ptp.s.ext_clk_in,
				(NSEC_PER_SEC << 32) / clock_comp);
		}

		/* Enable the clock if it wasn't done already */
		if (!ptp.s.ptp_en) {
			ptp.s.ptp_en = 1;
			cvmx_write_csr(CVMX_MIO_PTP_CLOCK_CFG, ptp.u64);
		}
		have_hw_timestamps = true;
	}

	if (!have_hw_timestamps)
		return -EINVAL;

	switch (config.tx_type) {
	case HWTSTAMP_TX_OFF:
	case HWTSTAMP_TX_ON:
		break;
	default:
		return -ERANGE;
	}

	switch (config.rx_filter) {
	case HWTSTAMP_FILTER_NONE:
		p->has_rx_tstamp = false;
		rxx_frm_ctl.u64 = cvmx_read_csr(p->agl + AGL_GMX_RX_FRM_CTL);
		rxx_frm_ctl.s.ptp_mode = 0;
		cvmx_write_csr(p->agl + AGL_GMX_RX_FRM_CTL, rxx_frm_ctl.u64);
		break;
	case HWTSTAMP_FILTER_ALL:
	case HWTSTAMP_FILTER_SOME:
	case HWTSTAMP_FILTER_PTP_V1_L4_EVENT:
	case HWTSTAMP_FILTER_PTP_V1_L4_SYNC:
	case HWTSTAMP_FILTER_PTP_V1_L4_DELAY_REQ:
	case HWTSTAMP_FILTER_PTP_V2_L4_EVENT:
	case HWTSTAMP_FILTER_PTP_V2_L4_SYNC:
	case HWTSTAMP_FILTER_PTP_V2_L4_DELAY_REQ:
	case HWTSTAMP_FILTER_PTP_V2_L2_EVENT:
	case HWTSTAMP_FILTER_PTP_V2_L2_SYNC:
	case HWTSTAMP_FILTER_PTP_V2_L2_DELAY_REQ:
	case HWTSTAMP_FILTER_PTP_V2_EVENT:
	case HWTSTAMP_FILTER_PTP_V2_SYNC:
	case HWTSTAMP_FILTER_PTP_V2_DELAY_REQ:
		p->has_rx_tstamp = have_hw_timestamps;
		config.rx_filter = HWTSTAMP_FILTER_ALL;
		if (p->has_rx_tstamp) {
			rxx_frm_ctl.u64 = cvmx_read_csr(p->agl + AGL_GMX_RX_FRM_CTL);
			rxx_frm_ctl.s.ptp_mode = 1;
			cvmx_write_csr(p->agl + AGL_GMX_RX_FRM_CTL, rxx_frm_ctl.u64);
		}
		break;
	default:
		return -ERANGE;
	}

	if (copy_to_user(rq->ifr_data, &config, sizeof(config)))
		return -EFAULT;

	return 0;
}

static int octeon_mgmt_ioctl(struct net_device *netdev,
			     struct ifreq *rq, int cmd)
{
	switch (cmd) {
	case SIOCSHWTSTAMP:
		return octeon_mgmt_ioctl_hwtstamp(netdev, rq, cmd);
	default:
		if (netdev->phydev)
			return phy_mii_ioctl(netdev->phydev, rq, cmd);
		return -EINVAL;
	}
}

static int octeon_mgmt_o3_ioctl(struct net_device *netdev,
				struct ifreq *rq, int cmd)
{
	if (netdev->phydev)
		return phy_mii_ioctl(netdev->phydev, rq, cmd);
	return -EINVAL;
}

static void octeon_mgmt_disable_link(struct octeon_mgmt *p)
{
	union cvmx_agl_gmx_prtx_cfg prtx_cfg;

	/* Disable GMX before we make any changes. */
	prtx_cfg.u64 = cvmx_read_csr(p->agl + AGL_GMX_PRT_CFG);
	prtx_cfg.s.en = 0;
	prtx_cfg.s.tx_en = 0;
	prtx_cfg.s.rx_en = 0;
	cvmx_write_csr(p->agl + AGL_GMX_PRT_CFG, prtx_cfg.u64);

	if (OCTEON_IS_MODEL(OCTEON_CN6XXX)) {
		int i;
		for (i = 0; i < 10; i++) {
			prtx_cfg.u64 = cvmx_read_csr(p->agl + AGL_GMX_PRT_CFG);
			if (prtx_cfg.s.tx_idle == 1 || prtx_cfg.s.rx_idle == 1)
				break;
			mdelay(1);
			i++;
		}
	}
}

static void octeon_mgmt_enable_link(struct octeon_mgmt *p)
{
	union cvmx_agl_gmx_prtx_cfg prtx_cfg;

	/* Restore the GMX enable state only if link is set */
	prtx_cfg.u64 = cvmx_read_csr(p->agl + AGL_GMX_PRT_CFG);
	prtx_cfg.s.tx_en = 1;
	prtx_cfg.s.rx_en = 1;
	prtx_cfg.s.en = 1;
	cvmx_write_csr(p->agl + AGL_GMX_PRT_CFG, prtx_cfg.u64);
}

static void octeon_mgmt_update_link(struct octeon_mgmt *p)
{
	struct net_device *ndev = p->netdev;
	struct phy_device *phydev = ndev->phydev;
	union cvmx_agl_gmx_prtx_cfg prtx_cfg;

	prtx_cfg.u64 = cvmx_read_csr(p->agl + AGL_GMX_PRT_CFG);

	if (!ndev->phydev)
		return;

	if (!ndev->phydev->link)
		prtx_cfg.s.duplex = 1;
	else
		prtx_cfg.s.duplex = phydev->duplex;

	switch (phydev->speed) {
	case 10:
		prtx_cfg.s.speed = 0;
		prtx_cfg.s.slottime = 0;

		if (OCTEON_IS_MODEL(OCTEON_CN6XXX)) {
			prtx_cfg.s.burst = 1;
			prtx_cfg.s.speed_msb = 1;
		}
		break;
	case 100:
		prtx_cfg.s.speed = 0;
		prtx_cfg.s.slottime = 0;

		if (OCTEON_IS_MODEL(OCTEON_CN6XXX)) {
			prtx_cfg.s.burst = 1;
			prtx_cfg.s.speed_msb = 0;
		}
		break;
	case 1000:
		/* 1000 MBits is only supported on 6XXX chips */
		if (OCTEON_IS_MODEL(OCTEON_CN6XXX)) {
			prtx_cfg.s.speed = 1;
			prtx_cfg.s.speed_msb = 0;
			/* Only matters for half-duplex */
			prtx_cfg.s.slottime = 1;
			prtx_cfg.s.burst = phydev->duplex;
		}
		break;
	case 0:  /* No link */
	default:
		break;
	}

	/* Write the new GMX setting with the port still disabled. */
	cvmx_write_csr(p->agl + AGL_GMX_PRT_CFG, prtx_cfg.u64);

	/* Read GMX CFG again to make sure the config is completed. */
	prtx_cfg.u64 = cvmx_read_csr(p->agl + AGL_GMX_PRT_CFG);

	if (OCTEON_IS_MODEL(OCTEON_CN6XXX)) {
		union cvmx_agl_gmx_txx_clk agl_clk;
		union cvmx_agl_prtx_ctl prtx_ctl;

		prtx_ctl.u64 = cvmx_read_csr(p->agl_prt_ctl);
		agl_clk.u64 = cvmx_read_csr(p->agl + AGL_GMX_TX_CLK);
		/* MII (both speeds) and RGMII 1000 speed. */
		agl_clk.s.clk_cnt = 1;
		if (prtx_ctl.s.mode == 0) { /* RGMII mode */
			if (phydev->speed == 10)
				agl_clk.s.clk_cnt = 50;
			else if (phydev->speed == 100)
				agl_clk.s.clk_cnt = 5;
		}
		cvmx_write_csr(p->agl + AGL_GMX_TX_CLK, agl_clk.u64);
	}
}

static void octeon_mgmt_adjust_link(struct net_device *netdev)
{
	struct octeon_mgmt *p = netdev_priv(netdev);
	struct phy_device *phydev = netdev->phydev;
	unsigned long flags;
	int link_changed = 0;

	if (!phydev)
		return;

	spin_lock_irqsave(&p->lock, flags);


	if (!phydev->link && p->last_link)
		link_changed = -1;

	if (phydev->link &&
	    (p->last_duplex != phydev->duplex ||
	     p->last_link != phydev->link ||
	     p->last_speed != phydev->speed)) {
		octeon_mgmt_disable_link(p);
		link_changed = 1;
		octeon_mgmt_update_link(p);
		octeon_mgmt_enable_link(p);
	}

	p->last_link = phydev->link;
	p->last_speed = phydev->speed;
	p->last_duplex = phydev->duplex;

	spin_unlock_irqrestore(&p->lock, flags);

	if (link_changed != 0) {
		if (link_changed > 0) {
			pr_info("%s: Link is up - %d/%s\n", netdev->name,
				phydev->speed,
				phydev->duplex == DUPLEX_FULL ?
				"Full" : "Half");
		} else {
			pr_info("%s: Link is down\n", netdev->name);
		}
	}
}

static int octeon_mgmt_init_phy(struct net_device *netdev)
{
	struct octeon_mgmt *p = netdev_priv(netdev);
	struct phy_device *phydev = NULL;

	if (octeon_is_simulation() || p->phy_np == NULL) {
		/* No PHYs in the simulator. */
		netif_carrier_on(netdev);
		return 0;
	}

	phydev = of_phy_connect(netdev, p->phy_np,
				octeon_mgmt_adjust_link, 0,
				PHY_INTERFACE_MODE_MII);

	if (!phydev)
		return -ENODEV;

	return 0;
}

static int octeon_mgmt_release_mem(struct octeon_mgmt *p)
{
	/* dma_unmap is a nop on Octeon, so just free everything.  */
	skb_queue_purge(&p->tx_list);
	skb_queue_purge(&p->rx_list);

	if (p->rx_ring_handle)
		dma_unmap_single(p->dev, p->rx_ring_handle,
				 ring_size_to_bytes(OCTEON_MGMT_RX_RING_SIZE),
				 DMA_BIDIRECTIONAL);
	kfree(p->rx_ring);

	if (p->tx_ring_handle)
		dma_unmap_single(p->dev, p->tx_ring_handle,
				 ring_size_to_bytes(OCTEON_MGMT_TX_RING_SIZE),
				 DMA_BIDIRECTIONAL);
	kfree(p->tx_ring);

	return 0;
}

static int octeon_mgmt_stop(struct net_device *netdev)
{
	struct octeon_mgmt *p = netdev_priv(netdev);

	napi_disable(&p->napi);
	netif_stop_queue(netdev);

	if (netdev->phydev) {
		phy_stop(netdev->phydev);
		phy_disconnect(netdev->phydev);
	}

	netif_carrier_off(netdev);

	octeon_mgmt_reset_hw(p);

	if (p->irq)
		free_irq(p->irq, netdev);

	return octeon_mgmt_release_mem(p);
}

static int octeon_mgmt_o3_stop(struct net_device *netdev)
{
	struct octeon_mgmt *p = netdev_priv(netdev);

	napi_disable(&p->napi);
	netif_stop_queue(netdev);

	bgx_port_disable(netdev);

	octeon_mgmt_reset_hw(p);


	if (p->irq_orthresh)
		free_irq(p->irq_orthresh, p);
	if (p->irq_irthresh)
		free_irq(p->irq_irthresh, p);

	return octeon_mgmt_release_mem(p);
}

static int octeon_mgmt_alloc_rings(struct net_device *netdev)
{
	struct octeon_mgmt *p = netdev_priv(netdev);

	/* Allocate ring buffers.  */
	p->tx_ring = kzalloc(ring_size_to_bytes(OCTEON_MGMT_TX_RING_SIZE),
			     GFP_KERNEL);
	if (!p->tx_ring)
		return -ENOMEM;
	p->tx_ring_handle =
		dma_map_single(p->dev, p->tx_ring,
			       ring_size_to_bytes(OCTEON_MGMT_TX_RING_SIZE),
			       DMA_BIDIRECTIONAL);
	p->tx_next = 0;
	p->tx_next_clean = 0;
	p->tx_current_fill = 0;


	p->rx_ring = kzalloc(ring_size_to_bytes(OCTEON_MGMT_RX_RING_SIZE),
			     GFP_KERNEL);
	if (!p->rx_ring)
		return -ENOMEM;
	p->rx_ring_handle =
		dma_map_single(p->dev, p->rx_ring,
			       ring_size_to_bytes(OCTEON_MGMT_RX_RING_SIZE),
			       DMA_BIDIRECTIONAL);

	p->rx_next = 0;
	p->rx_next_fill = 0;
	p->rx_current_fill = 0;

	return 0;
}

static int octeon_mgmt_o3_open(struct net_device *netdev)
{
	struct octeon_mgmt *p = netdev_priv(netdev);
	union cvmx_mixx_ctl mix_ctl;
	union cvmx_mixx_oring1 oring1;
	union cvmx_mixx_iring1 iring1;
	union cvmx_mixx_irhwm mix_irhwm;
	union cvmx_mixx_orhwm mix_orhwm;
	struct irq_domain *domain;
	int intsn_orthresh;
	int intsn_irthresh;

	if (octeon_mgmt_alloc_rings(netdev))
		goto err_nomem;

	bgx_port_mix_assert_reset(netdev, p->port, true);
	octeon_mgmt_common_reset_hw(p);

	mix_ctl.u64 = cvmx_read_csr(p->mix + MIX_CTL);

	/* Bring it out of reset if needed. */
	if (mix_ctl.s.reset) {
		mix_ctl.s.reset = 0;
		cvmx_write_csr(p->mix + MIX_CTL, mix_ctl.u64);
		do {
			mix_ctl.u64 = cvmx_read_csr(p->mix + MIX_CTL);
		} while (mix_ctl.s.reset);
	}
	bgx_port_mix_assert_reset(netdev, p->port, false);

	oring1.u64 = 0;
	oring1.cn78xx.obase = p->tx_ring_handle >> 3;
	oring1.cn78xx.osize = OCTEON_MGMT_TX_RING_SIZE;
	cvmx_write_csr(p->mix + MIX_ORING1, oring1.u64);

	iring1.u64 = 0;
	iring1.cn78xx.ibase = p->rx_ring_handle >> 3;
	iring1.cn78xx.isize = OCTEON_MGMT_RX_RING_SIZE;
	cvmx_write_csr(p->mix + MIX_IRING1, iring1.u64);

	/* Enable the port HW. Packets are not allowed until
	 * cvmx_mgmt_port_enable() is called.
	 */
	mix_ctl.u64 = 0;
	mix_ctl.s.crc_strip = 1;    /* Strip the ending CRC */
	mix_ctl.s.en = 1;           /* Enable the port */
	mix_ctl.s.nbtarb = 0;       /* Arbitration mode */
	/* MII CB-request FIFO programmable high watermark */
	mix_ctl.s.mrq_hwm = 1;
#ifdef __LITTLE_ENDIAN
	mix_ctl.s.lendian = 1;
#endif
	cvmx_write_csr(p->mix + MIX_CTL, mix_ctl.u64);

	octeon_mgmt_rx_fill_ring(netdev);

	/* Clear any pending interrupts */
	cvmx_write_csr(p->mix + MIX_ISR, cvmx_read_csr(p->mix + MIX_ISR));

	/* Interrupt every single RX packet */
	mix_irhwm.u64 = 0;
	mix_irhwm.s.irhwm = 0;
	cvmx_write_csr(p->mix + MIX_IRHWM, mix_irhwm.u64);

	/* Interrupt when we have 1 or more packets to clean.  */
	mix_orhwm.u64 = 0;
	mix_orhwm.s.orhwm = 0;
	cvmx_write_csr(p->mix + MIX_ORHWM, mix_orhwm.u64);

	napi_enable(&p->napi);

	intsn_orthresh = 0x10002 + 0x10 * p->port;
	intsn_irthresh = 0x10003 + 0x10 * p->port;

	domain = octeon_irq_get_block_domain(p->numa_node, intsn_orthresh >> 12);
	p->irq_orthresh = irq_create_mapping(domain, intsn_orthresh);
	p->irq_irthresh = irq_create_mapping(domain, intsn_irthresh);

	if (request_irq(p->irq_orthresh, octeon_mgmt_o3_or_handler, 0, netdev->name, p)) {
		netdev_err(netdev, "request_irq(%d) failed.\n", p->irq_orthresh);
		goto err_noirq;
	}
	if (request_irq(p->irq_irthresh, octeon_mgmt_o3_ir_handler, 0, netdev->name, p)) {
		netdev_err(netdev, "request_irq(%d) failed.\n", p->irq_irthresh);
		goto err_noirq;
	}

	p->last_link = 0;
	p->last_speed = 0;

	netif_wake_queue(netdev);

	return bgx_port_enable(netdev);

err_noirq:
err_nomem:
	octeon_mgmt_o3_stop(netdev);
	return -ENOMEM;
}

static int octeon_mgmt_open(struct net_device *netdev)
{
	struct octeon_mgmt *p = netdev_priv(netdev);
	union cvmx_mixx_ctl mix_ctl;
	union cvmx_agl_gmx_inf_mode agl_gmx_inf_mode;
	union cvmx_mixx_oring1 oring1;
	union cvmx_mixx_iring1 iring1;
	union cvmx_agl_gmx_rxx_frm_ctl rxx_frm_ctl;
	union cvmx_mixx_irhwm mix_irhwm;
	union cvmx_mixx_orhwm mix_orhwm;
	union cvmx_mixx_intena mix_intena;
	struct sockaddr sa;

	if (octeon_mgmt_alloc_rings(netdev))
		goto err_nomem;

	octeon_mgmt_reset_hw(p);

	mix_ctl.u64 = cvmx_read_csr(p->mix + MIX_CTL);

	/* Bring it out of reset if needed. */
	if (mix_ctl.s.reset) {
		mix_ctl.s.reset = 0;
		cvmx_write_csr(p->mix + MIX_CTL, mix_ctl.u64);
		do {
			mix_ctl.u64 = cvmx_read_csr(p->mix + MIX_CTL);
		} while (mix_ctl.s.reset);
	}

	if (OCTEON_IS_MODEL(OCTEON_CN5XXX)) {
		agl_gmx_inf_mode.u64 = 0;
		agl_gmx_inf_mode.s.en = 1;
		cvmx_write_csr(CVMX_AGL_GMX_INF_MODE, agl_gmx_inf_mode.u64);
	}
	if (OCTEON_IS_MODEL(OCTEON_CN56XX_PASS1_X)
		|| OCTEON_IS_MODEL(OCTEON_CN52XX_PASS1_X)) {
		/* Force compensation values, as they are not
		 * determined properly by HW
		 */
		union cvmx_agl_gmx_drv_ctl drv_ctl;

		drv_ctl.u64 = cvmx_read_csr(CVMX_AGL_GMX_DRV_CTL);
		if (p->port) {
			drv_ctl.s.byp_en1 = 1;
			drv_ctl.s.nctl1 = 6;
			drv_ctl.s.pctl1 = 6;
		} else {
			drv_ctl.s.byp_en = 1;
			drv_ctl.s.nctl = 6;
			drv_ctl.s.pctl = 6;
		}
		cvmx_write_csr(CVMX_AGL_GMX_DRV_CTL, drv_ctl.u64);
	}

	oring1.u64 = 0;
	oring1.cn63xx.obase = p->tx_ring_handle >> 3;
	oring1.cn63xx.osize = OCTEON_MGMT_TX_RING_SIZE;
	cvmx_write_csr(p->mix + MIX_ORING1, oring1.u64);

	iring1.u64 = 0;
	iring1.cn63xx.ibase = p->rx_ring_handle >> 3;
	iring1.cn63xx.isize = OCTEON_MGMT_RX_RING_SIZE;
	cvmx_write_csr(p->mix + MIX_IRING1, iring1.u64);

	memcpy(sa.sa_data, netdev->dev_addr, ETH_ALEN);
	octeon_mgmt_set_mac_address(netdev, &sa);

	octeon_mgmt_change_mtu(netdev, netdev->mtu);

	/* Enable the port HW. Packets are not allowed until
	 * cvmx_mgmt_port_enable() is called.
	 */
	mix_ctl.u64 = 0;
	mix_ctl.s.crc_strip = 1;    /* Strip the ending CRC */
	mix_ctl.s.en = 1;           /* Enable the port */
	mix_ctl.s.nbtarb = 0;       /* Arbitration mode */
	/* MII CB-request FIFO programmable high watermark */
	mix_ctl.s.mrq_hwm = 1;
#ifdef __LITTLE_ENDIAN
	mix_ctl.s.lendian = 1;
#endif
	cvmx_write_csr(p->mix + MIX_CTL, mix_ctl.u64);

	/* Read the PHY to find the mode of the interface. */
	if (octeon_mgmt_init_phy(netdev)) {
		dev_err(p->dev, "Cannot initialize PHY on MIX%d.\n", p->port);
		goto err_noirq;
	}

	/* Set the mode of the interface, RGMII/MII. */
	if (OCTEON_IS_MODEL(OCTEON_CN6XXX) && netdev->phydev) {
		union cvmx_agl_prtx_ctl agl_prtx_ctl;
		int rgmii_mode =
			(linkmode_test_bit(ETHTOOL_LINK_MODE_1000baseT_Half_BIT,
					   netdev->phydev->supported) |
			 linkmode_test_bit(ETHTOOL_LINK_MODE_1000baseT_Full_BIT,
					   netdev->phydev->supported)) != 0;

		agl_prtx_ctl.u64 = cvmx_read_csr(p->agl_prt_ctl);
		agl_prtx_ctl.s.mode = rgmii_mode ? 0 : 1;
		cvmx_write_csr(p->agl_prt_ctl,	agl_prtx_ctl.u64);

		/* MII clocks counts are based on the 125Mhz
		 * reference, which has an 8nS period. So our delays
		 * need to be multiplied by this factor.
		 */
#define NS_PER_PHY_CLK 8

		/* Take the DLL and clock tree out of reset */
		agl_prtx_ctl.u64 = cvmx_read_csr(p->agl_prt_ctl);
		agl_prtx_ctl.s.clkrst = 0;
		if (rgmii_mode) {
			agl_prtx_ctl.s.dllrst = 0;
			agl_prtx_ctl.s.clktx_byp = 0;
		}
		cvmx_write_csr(p->agl_prt_ctl,	agl_prtx_ctl.u64);
		cvmx_read_csr(p->agl_prt_ctl); /* Force write out before wait */

		/* Wait for the DLL to lock. External 125 MHz
		 * reference clock must be stable at this point.
		 */
		ndelay(256 * NS_PER_PHY_CLK);

		/* Enable the interface */
		agl_prtx_ctl.u64 = cvmx_read_csr(p->agl_prt_ctl);
		agl_prtx_ctl.s.enable = 1;
		cvmx_write_csr(p->agl_prt_ctl, agl_prtx_ctl.u64);

		/* Read the value back to force the previous write */
		agl_prtx_ctl.u64 = cvmx_read_csr(p->agl_prt_ctl);

		/* Enable the compensation controller */
		agl_prtx_ctl.s.comp = 1;
		agl_prtx_ctl.s.drv_byp = 0;
		cvmx_write_csr(p->agl_prt_ctl,	agl_prtx_ctl.u64);
		/* Force write out before wait. */
		cvmx_read_csr(p->agl_prt_ctl);

		/* For compensation state to lock. */
		ndelay(1040 * NS_PER_PHY_CLK);

		/* Default Interframe Gaps are too small.  Recommended
		 * workaround is.
		 *
		 * AGL_GMX_TX_IFG[IFG1]=14
		 * AGL_GMX_TX_IFG[IFG2]=10
		 */
		cvmx_write_csr(CVMX_AGL_GMX_TX_IFG, 0xae);
	}

	octeon_mgmt_rx_fill_ring(netdev);

	/* Clear statistics. */
	/* Clear on read. */
	cvmx_write_csr(p->agl + AGL_GMX_RX_STATS_CTL, 1);
	cvmx_write_csr(p->agl + AGL_GMX_RX_STATS_PKTS_DRP, 0);
	cvmx_write_csr(p->agl + AGL_GMX_RX_STATS_PKTS_BAD, 0);

	cvmx_write_csr(p->agl + AGL_GMX_TX_STATS_CTL, 1);
	cvmx_write_csr(p->agl + AGL_GMX_TX_STAT0, 0);
	cvmx_write_csr(p->agl + AGL_GMX_TX_STAT1, 0);

	/* Clear any pending interrupts */
	cvmx_write_csr(p->mix + MIX_ISR, cvmx_read_csr(p->mix + MIX_ISR));

	if (request_irq(p->irq, octeon_mgmt_interrupt, 0, netdev->name,
			netdev)) {
		dev_err(p->dev, "request_irq(%d) failed.\n", p->irq);
		goto err_noirq;
	}

	/* Interrupt every single RX packet */
	mix_irhwm.u64 = 0;
	mix_irhwm.s.irhwm = 0;
	cvmx_write_csr(p->mix + MIX_IRHWM, mix_irhwm.u64);

	/* Interrupt when we have 1 or more packets to clean.  */
	mix_orhwm.u64 = 0;
	mix_orhwm.s.orhwm = 0;
	cvmx_write_csr(p->mix + MIX_ORHWM, mix_orhwm.u64);

	napi_enable(&p->napi);

	/* Enable receive and transmit interrupts */
	mix_intena.u64 = 0;
	mix_intena.s.ithena = 1;
	mix_intena.s.othena = 1;
	cvmx_write_csr(p->mix + MIX_INTENA, mix_intena.u64);

	/* Enable packet I/O. */

	rxx_frm_ctl.u64 = 0;
	rxx_frm_ctl.s.ptp_mode = p->has_rx_tstamp ? 1 : 0;
	rxx_frm_ctl.s.pre_align = 1;
	/* When set, disables the length check for non-min sized pkts
	 * with padding in the client data.
	 */
	rxx_frm_ctl.s.pad_len = 1;
	/* When set, disables the length check for VLAN pkts */
	rxx_frm_ctl.s.vlan_len = 1;
	/* When set, PREAMBLE checking is  less strict */
	rxx_frm_ctl.s.pre_free = 1;
	/* Control Pause Frames can match station SMAC */
	rxx_frm_ctl.s.ctl_smac = 0;
	/* Control Pause Frames can match globally assign Multicast address */
	rxx_frm_ctl.s.ctl_mcst = 1;
	/* Forward pause information to TX block */
	rxx_frm_ctl.s.ctl_bck = 1;
	/* Drop Control Pause Frames */
	rxx_frm_ctl.s.ctl_drp = 1;
	/* Strip off the preamble */
	rxx_frm_ctl.s.pre_strp = 1;
	/* This port is configured to send PREAMBLE+SFD to begin every
	 * frame.  GMX checks that the PREAMBLE is sent correctly.
	 */
	rxx_frm_ctl.s.pre_chk = 1;
	cvmx_write_csr(p->agl + AGL_GMX_RX_FRM_CTL, rxx_frm_ctl.u64);

	/* Configure the port duplex, speed and enables */
	octeon_mgmt_disable_link(p);
	if (netdev->phydev)
		octeon_mgmt_update_link(p);
	octeon_mgmt_enable_link(p);

	p->last_link = 0;
	p->last_speed = 0;
	/* PHY is not present in simulator. The carrier is enabled
	 * while initializing the phy for simulator, leave it enabled.
	 */
	if (netdev->phydev) {
		netif_carrier_off(netdev);
		phy_start(netdev->phydev);
	}

	netif_wake_queue(netdev);

	return 0;

err_noirq:
err_nomem:
	octeon_mgmt_stop(netdev);
	return -ENOMEM;
}

static int octeon_mgmt_xmit(struct sk_buff *skb, struct net_device *netdev)
{
	struct octeon_mgmt *p = netdev_priv(netdev);
	unsigned long flags;
	int rv = NETDEV_TX_BUSY;
	u64 re;
	u64 addr;

	addr = dma_map_single(p->dev, skb->data,
			      skb->len,
			      DMA_TO_DEVICE);

	re = addr | (((u64)skb->len & OCTEON_MGMT_RE_LEN_MASK) << p->re_len_shift);
	if ((skb_shinfo(skb)->tx_flags & SKBTX_HW_TSTAMP))
		re |= 1ull << p->re_ts_bit;

	spin_lock_irqsave(&p->tx_list.lock, flags);

	if (unlikely(p->tx_current_fill >= ring_max_fill(OCTEON_MGMT_TX_RING_SIZE) - 1)) {
		spin_unlock_irqrestore(&p->tx_list.lock, flags);
		netif_stop_queue(netdev);
		spin_lock_irqsave(&p->tx_list.lock, flags);
	}

	if (unlikely(p->tx_current_fill >=
		     ring_max_fill(OCTEON_MGMT_TX_RING_SIZE))) {
		spin_unlock_irqrestore(&p->tx_list.lock, flags);
		dma_unmap_single(p->dev, addr, skb->len,
				 DMA_TO_DEVICE);
		goto out;
	}

	__skb_queue_tail(&p->tx_list, skb);

	/* Put it in the ring.  */
	p->tx_ring[p->tx_next] = re;
	p->tx_next = (p->tx_next + 1) % OCTEON_MGMT_TX_RING_SIZE;
	p->tx_current_fill++;

	spin_unlock_irqrestore(&p->tx_list.lock, flags);

	dma_sync_single_for_device(p->dev, p->tx_ring_handle,
				   ring_size_to_bytes(OCTEON_MGMT_TX_RING_SIZE),
				   DMA_BIDIRECTIONAL);

	netdev->stats.tx_packets++;
	netdev->stats.tx_bytes += skb->len;

	/* Ring the bell.  */
	cvmx_write_csr(p->mix + MIX_ORING2, 1);

	netif_trans_update(netdev);
	rv = NETDEV_TX_OK;
out:
	if (!p->has_o3_structs)
		octeon_mgmt_update_tx_stats(netdev);
	return rv;
}

#ifdef CONFIG_NET_POLL_CONTROLLER
static void octeon_mgmt_poll_controller(struct net_device *netdev)
{
	struct octeon_mgmt *p = netdev_priv(netdev);

	octeon_mgmt_receive_packets(p, 16);
	octeon_mgmt_update_rx_stats(netdev);
}
#endif

static int octeon_mgmt_o3_init(struct net_device *netdev)
{
	const u8 *mac;

	mac = bgx_port_get_mac(netdev);
	if (mac && is_valid_ether_addr(mac)) {
		eth_hw_addr_set(netdev, mac);
		netdev->addr_assign_type &= ~NET_ADDR_RANDOM;
	} else {
		eth_hw_addr_random(netdev);
	}
	bgx_port_set_rx_filtering(netdev);

	return bgx_port_change_mtu(netdev, netdev->mtu);
}

static int octeon_mgmt_o3_set_mac_address(struct net_device *netdev, void *addr)
{
	int r = eth_mac_addr(netdev, addr);

	if (r)
		return r;

	bgx_port_set_rx_filtering(netdev);

	return 0;
}

static void octeon_mgmt_get_drvinfo(struct net_device *netdev,
				    struct ethtool_drvinfo *info)
{
	strlcpy(info->driver, DRV_NAME, sizeof(info->driver));
	strlcpy(info->version, DRV_VERSION, sizeof(info->version));
	strlcpy(info->fw_version, "N/A", sizeof(info->fw_version));
	strlcpy(info->bus_info, "N/A", sizeof(info->bus_info));
}

static int octeon_mgmt_nway_reset(struct net_device *dev)
{
	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	if (dev->phydev) {
		phy_start(dev->phydev);
		return phy_start_aneg(dev->phydev);
	}

	return -EOPNOTSUPP;
}

static const struct ethtool_ops octeon_mgmt_ethtool_ops = {
	.get_drvinfo = octeon_mgmt_get_drvinfo,
	.nway_reset = octeon_mgmt_nway_reset,
	.get_link = ethtool_op_get_link,
	.get_link_ksettings = phy_ethtool_get_link_ksettings,
	.set_link_ksettings = phy_ethtool_set_link_ksettings,
};

static const struct net_device_ops octeon_mgmt_ops = {
	.ndo_open =			octeon_mgmt_open,
	.ndo_stop =			octeon_mgmt_stop,
	.ndo_start_xmit =		octeon_mgmt_xmit,
	.ndo_set_rx_mode =		octeon_mgmt_set_rx_filtering,
	.ndo_set_mac_address =		octeon_mgmt_set_mac_address,
	.ndo_eth_ioctl =			octeon_mgmt_ioctl,
	.ndo_change_mtu =		octeon_mgmt_change_mtu,
#ifdef CONFIG_NET_POLL_CONTROLLER
	.ndo_poll_controller =		octeon_mgmt_poll_controller,
#endif
};

static const struct ethtool_ops octeon_mgmt_o3_ethtool_ops = {
	.get_drvinfo = octeon_mgmt_get_drvinfo,
	.get_link_ksettings = bgx_port_ethtool_get_settings,
	.set_link_ksettings = bgx_port_ethtool_set_settings,
	.nway_reset = bgx_port_ethtool_nway_reset,
	.get_link = ethtool_op_get_link,
};

static const struct net_device_ops octeon_mgmt_o3_ops = {
	.ndo_init		= octeon_mgmt_o3_init,
	.ndo_open		= octeon_mgmt_o3_open,
	.ndo_stop		= octeon_mgmt_o3_stop,
	.ndo_start_xmit		= octeon_mgmt_xmit,
	.ndo_set_rx_mode	= bgx_port_set_rx_filtering,
	.ndo_set_mac_address	= octeon_mgmt_o3_set_mac_address,
	.ndo_do_ioctl		= octeon_mgmt_o3_ioctl,
	.ndo_change_mtu		= bgx_port_change_mtu,
#ifdef CONFIG_NET_POLL_CONTROLLER
	.ndo_poll_controller	= octeon_mgmt_poll_controller,
#endif
};

static int octeon_mgmt_remove(struct platform_device *pdev)
{
	struct net_device *netdev = dev_get_drvdata(&pdev->dev);
	struct octeon_mgmt *p = netdev_priv(netdev);

	if (p->napi.dev)
		netif_napi_del(&p->napi);
	tasklet_kill(&p->tx_clean_tasklet);
	unregister_netdev(netdev);
	dev_set_drvdata(&pdev->dev, NULL);
	free_netdev(netdev);

	return 0;
}

static void octeon_mgmt_probe_common(struct net_device *netdev,
				     struct platform_device *pdev)
{
	struct octeon_mgmt *p = netdev_priv(netdev);

	SET_NETDEV_DEV(netdev, &pdev->dev);
	dev_set_drvdata(&pdev->dev, netdev);

	netif_napi_add_weight(netdev, &p->napi, octeon_mgmt_napi_poll,
		       OCTEON_MGMT_NAPI_WEIGHT);

	p->netdev = netdev;
	p->dev = &pdev->dev;
	p->has_rx_tstamp = false;

	spin_lock_init(&p->lock);

	skb_queue_head_init(&p->tx_list);
	skb_queue_head_init(&p->rx_list);
	tasklet_setup(&p->tx_clean_tasklet,
		      octeon_mgmt_clean_tx_tasklet);

	netdev->priv_flags |= IFF_UNICAST_FLT | IFF_LIVE_ADDR_CHANGE;

	pdev->dev.coherent_dma_mask = DMA_BIT_MASK(64);
	pdev->dev.dma_mask = &pdev->dev.coherent_dma_mask;

	netif_carrier_off(netdev);
}

static int octeon_mgmt_o3_probe(struct platform_device *pdev)
{
	struct net_device *netdev;
	struct octeon_mgmt *p;
	struct mac_platform_data *pd = dev_get_platdata(&pdev->dev);
	int result = -EINVAL;

	dev_notice(&pdev->dev, "Probed %d:%d:%d\n", pd->numa_node, pd->interface, pd->port);
	netdev = alloc_etherdev(sizeof(struct octeon_mgmt));
	if (netdev == NULL)
		return -ENOMEM;

	octeon_mgmt_probe_common(netdev, pdev);

	bgx_port_set_netdev(pdev->dev.parent, netdev);

	p = netdev_priv(netdev);
	p->has_o3_structs = true;
	p->re_len_shift = 50;
	p->re_code_shift = 44;
	p->re_ts_bit = 49;
	/* 42-bits */
	p->re_addr_mask = 0x3ffffffffffull;
	p->port = pd->port;
	p->numa_node = pd->numa_node;
	netdev->netdev_ops = &octeon_mgmt_o3_ops;
	netdev->ethtool_ops = &octeon_mgmt_o3_ethtool_ops;

	p->mix_phys = 0x1070000100000ull + 0x800 * p->port + (1ull << 36) * p->numa_node;
	p->mix_size = 0x100;

	snprintf(netdev->name, IFNAMSIZ, "mgmt%d%d", p->numa_node, p->port);

	if (!devm_request_mem_region(&pdev->dev, p->mix_phys, p->mix_size,
				     "MIX")) {
		p->mix_phys = 0;
		dev_err(&pdev->dev, "ERROR: request_mem_region MIX failed\n");
		goto err;
	}

	p->mix = (u64)devm_ioremap(&pdev->dev, p->mix_phys, p->mix_size);

	result = register_netdev(netdev);
	if (result)
		goto err;

	return 0;
err:
	octeon_mgmt_remove(pdev);
	return result;
}

static int octeon_mgmt_probe(struct platform_device *pdev)
{
	struct net_device *netdev;
	struct octeon_mgmt *p;
	const __be32 *data;
	struct resource *res_mix;
	struct resource *res_agl;
	struct resource *res_agl_prt_ctl;
	int len;
	int result;

	/* If it isn't the old MIX, it must be the new one */
	if (!of_device_is_compatible(pdev->dev.of_node, "cavium,octeon-5750-mix"))
		return octeon_mgmt_o3_probe(pdev);

	netdev = alloc_etherdev(sizeof(struct octeon_mgmt));
	if (netdev == NULL)
		return -ENOMEM;

	octeon_mgmt_probe_common(netdev, pdev);

	p = netdev_priv(netdev);
	p->has_o3_structs = false;
	p->re_len_shift = 48;
	p->re_code_shift = 40;
	p->re_ts_bit = 47;
	/* 40-bits */
	p->re_addr_mask = 0xffffffffffull;

	data = of_get_property(pdev->dev.of_node, "cell-index", &len);
	if (data && len == sizeof(*data)) {
		p->port = be32_to_cpup(data);
	} else {
		dev_err(&pdev->dev, "no 'cell-index' property\n");
		result = -ENXIO;
		goto err;
	}

	snprintf(netdev->name, IFNAMSIZ, "mgmt%d", p->port);

	result = platform_get_irq(pdev, 0);
	if (result < 0)
		goto err;

	p->irq = result;
	result = -ENXIO; /* default err from here down */

	res_mix = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res_mix == NULL) {
		dev_err(&pdev->dev, "no 'reg' resource\n");
		goto err;
	}

	res_agl = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if (res_agl == NULL) {
		dev_err(&pdev->dev, "no 'reg' resource\n");
		goto err;
	}

	res_agl_prt_ctl = platform_get_resource(pdev, IORESOURCE_MEM, 3);
	if (res_agl_prt_ctl == NULL) {
		dev_err(&pdev->dev, "no 'reg' resource\n");
		goto err;
	}

	p->mix_phys = res_mix->start;
	p->mix_size = resource_size(res_mix);

	if (!devm_request_mem_region(&pdev->dev, p->mix_phys, p->mix_size,
				     res_mix->name)) {
		p->mix_phys = 0;
		dev_err(&pdev->dev, "request_mem_region (%s) failed\n",
			res_mix->name);
		goto err;
	}

	p->agl_phys = res_agl->start;
	p->agl_size = resource_size(res_agl);

	if (!devm_request_mem_region(&pdev->dev, p->agl_phys, p->agl_size,
				     res_agl->name)) {
		p->agl_phys = 0;
		dev_err(&pdev->dev, "request_mem_region (%s) failed\n",
			res_agl->name);
		goto err;
	}

	p->agl_prt_ctl_phys = res_agl_prt_ctl->start;
	p->agl_prt_ctl_size = resource_size(res_agl_prt_ctl);

	if (!devm_request_mem_region(&pdev->dev, p->agl_prt_ctl_phys,
				     p->agl_prt_ctl_size, res_agl_prt_ctl->name)) {
		p->agl_prt_ctl_phys = 0;
		dev_err(&pdev->dev, "request_mem_region (%s) failed\n",
			res_agl_prt_ctl->name);
		goto err;
	}

	p->mix = (u64)devm_ioremap(&pdev->dev, p->mix_phys, p->mix_size);
	p->agl = (u64)devm_ioremap(&pdev->dev, p->agl_phys, p->agl_size);
	p->agl_prt_ctl = (u64)devm_ioremap(&pdev->dev, p->agl_prt_ctl_phys,
					   p->agl_prt_ctl_size);
	netdev->netdev_ops = &octeon_mgmt_ops;
	netdev->ethtool_ops = &octeon_mgmt_ethtool_ops;

	if (of_get_ethdev_address(pdev->dev.of_node, netdev))
		eth_hw_addr_random(netdev);

	p->phy_np = of_parse_phandle(pdev->dev.of_node, "phy-handle", 0);

	result = register_netdev(netdev);
	if (result)
		goto err;

	dev_info(&pdev->dev, "Version " DRV_VERSION "\n");
	return 0;

err:
	of_node_put(p->phy_np);
	octeon_mgmt_remove(pdev);
	return result;
}

static const struct of_device_id octeon_mgmt_match[] = {
	{
		.compatible = "cavium,octeon-5750-mix",
	},
	{},
};
MODULE_DEVICE_TABLE(of, octeon_mgmt_match);

static struct platform_driver octeon_mgmt_driver = {
	.driver = {
		.name		= "octeon_mgmt",
		.of_match_table = octeon_mgmt_match,
	},
	.probe		= octeon_mgmt_probe,
	.remove		= octeon_mgmt_remove,
};

static int __init octeon_mgmt_mod_init(void)
{
	/* Force our mdiobus driver module to be loaded first. */
	return platform_driver_register(&octeon_mgmt_driver);
}

static void __exit octeon_mgmt_mod_exit(void)
{
	platform_driver_unregister(&octeon_mgmt_driver);
}

module_init(octeon_mgmt_mod_init);
module_exit(octeon_mgmt_mod_exit);

MODULE_DESCRIPTION(DRV_DESCRIPTION);
MODULE_AUTHOR("David Daney");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);
