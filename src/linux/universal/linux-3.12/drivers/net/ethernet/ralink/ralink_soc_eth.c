/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; version 2 of the License
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 *
 *   Copyright (C) 2009-2013 John Crispin <blogic@openwrt.org>
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/dma-mapping.h>
#include <linux/init.h>
#include <linux/skbuff.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/clk.h>
#include <linux/of_net.h>
#include <linux/of_mdio.h>
#include <linux/if_vlan.h>

#include <asm/mach-ralink-openwrt/ralink_regs.h>

#include "ralink_soc_eth.h"
#include "esw_rt3052.h"
#include "mdio.h"

#define TX_TIMEOUT		(20 * HZ / 100)
#define	MAX_RX_LENGTH		1536

static const u32 fe_reg_table_default[FE_REG_COUNT] = {
	[FE_REG_PDMA_GLO_CFG] = FE_PDMA_GLO_CFG,
	[FE_REG_PDMA_RST_CFG] = FE_PDMA_RST_CFG,
	[FE_REG_DLY_INT_CFG] = FE_DLY_INT_CFG,
	[FE_REG_TX_BASE_PTR0] = FE_TX_BASE_PTR0,
	[FE_REG_TX_MAX_CNT0] = FE_TX_MAX_CNT0,
	[FE_REG_TX_CTX_IDX0] = FE_TX_CTX_IDX0,
	[FE_REG_RX_BASE_PTR0] = FE_RX_BASE_PTR0,
	[FE_REG_RX_MAX_CNT0] = FE_RX_MAX_CNT0,
	[FE_REG_RX_CALC_IDX0] = FE_RX_CALC_IDX0,
	[FE_REG_FE_INT_ENABLE] = FE_FE_INT_ENABLE,
	[FE_REG_FE_INT_STATUS] = FE_FE_INT_STATUS,
};

static const u32 *fe_reg_table = fe_reg_table_default;

static void __iomem *fe_base = 0;

void fe_w32(u32 val, unsigned reg)
{
	__raw_writel(val, fe_base + reg);
}

u32 fe_r32(unsigned reg)
{
	return __raw_readl(fe_base + reg);
}

static inline void fe_reg_w32(u32 val, enum fe_reg reg)
{
	fe_w32(val, fe_reg_table[reg]);
}

static inline u32 fe_reg_r32(enum fe_reg reg)
{
	return fe_r32(fe_reg_table[reg]);
}

static inline void fe_int_disable(u32 mask)
{
	fe_reg_w32(fe_reg_r32(FE_REG_FE_INT_ENABLE) & ~mask,
		     FE_REG_FE_INT_ENABLE);
	/* flush write */
	fe_reg_r32(FE_REG_FE_INT_ENABLE);
}

static inline void fe_int_enable(u32 mask)
{
	fe_reg_w32(fe_reg_r32(FE_REG_FE_INT_ENABLE) | mask,
		     FE_REG_FE_INT_ENABLE);
	/* flush write */
	fe_reg_r32(FE_REG_FE_INT_ENABLE);
}

static inline void fe_hw_set_macaddr(struct fe_priv *priv, unsigned char *mac)
{
	unsigned long flags;

	spin_lock_irqsave(&priv->page_lock, flags);
	fe_w32((mac[0] << 8) | mac[1], FE_GDMA1_MAC_ADRH);
	fe_w32((mac[2] << 24) | (mac[3] << 16) | (mac[4] << 8) | mac[5],
		     FE_GDMA1_MAC_ADRL);
	spin_unlock_irqrestore(&priv->page_lock, flags);
}

static int fe_set_mac_address(struct net_device *dev, void *p)
{
	int ret = eth_mac_addr(dev, p);

	if (!ret) {
		struct fe_priv *priv = netdev_priv(dev);

		if (priv->soc->set_mac)
			priv->soc->set_mac(priv, dev->dev_addr);
		else
			fe_hw_set_macaddr(priv, p);
	}

	return ret;
}

static struct sk_buff* fe_alloc_skb(struct fe_priv *priv)
{
	struct sk_buff *skb;

	skb = netdev_alloc_skb(priv->netdev, MAX_RX_LENGTH + NET_IP_ALIGN);
	if (!skb)
		return NULL;

	skb_reserve(skb, NET_IP_ALIGN);

	return skb;
}

static int fe_alloc_rx(struct fe_priv *priv)
{
	int size = NUM_DMA_DESC * sizeof(struct fe_rx_dma);
	int i;

	priv->rx_dma = dma_alloc_coherent(&priv->netdev->dev, size,
					&priv->rx_phys, GFP_ATOMIC);
	if (!priv->rx_dma)
		return -ENOMEM;

	memset(priv->rx_dma, 0, size);

	for (i = 0; i < NUM_DMA_DESC; i++) {
		priv->rx_skb[i] = fe_alloc_skb(priv);
		if (!priv->rx_skb[i])
			return -ENOMEM;
	}

	for (i = 0; i < NUM_DMA_DESC; i++) {
		dma_addr_t dma_addr = dma_map_single(&priv->netdev->dev,
						priv->rx_skb[i]->data,
						MAX_RX_LENGTH,
						DMA_FROM_DEVICE);
		priv->rx_dma[i].rxd1 = (unsigned int) dma_addr;

		if (priv->soc->rx_dma)
			priv->soc->rx_dma(priv, i, MAX_RX_LENGTH);
		else
			priv->rx_dma[i].rxd2 = RX_DMA_LSO;
	}
	wmb();

	fe_reg_w32(priv->rx_phys, FE_REG_RX_BASE_PTR0);
	fe_reg_w32(NUM_DMA_DESC, FE_REG_RX_MAX_CNT0);
	fe_reg_w32((NUM_DMA_DESC - 1), FE_REG_RX_CALC_IDX0);
	fe_reg_w32(FE_PST_DRX_IDX0, FE_REG_PDMA_RST_CFG);

	return 0;
}

static int fe_alloc_tx(struct fe_priv *priv)
{
	int size = NUM_DMA_DESC * sizeof(struct fe_tx_dma);
	int i;

	priv->tx_free_idx = 0;

	priv->tx_dma = dma_alloc_coherent(&priv->netdev->dev, size,
					&priv->tx_phys, GFP_ATOMIC);
	if (!priv->tx_dma)
		return -ENOMEM;

	memset(priv->tx_dma, 0, size);

	for (i = 0; i < NUM_DMA_DESC; i++) {
		if (priv->soc->tx_dma) {
			priv->soc->tx_dma(priv, i, 0);
			continue;
		}

		priv->tx_dma[i].txd2 = TX_DMA_LSO | TX_DMA_DONE;
		priv->tx_dma[i].txd4 = TX_DMA_QN(3) | TX_DMA_PN(1);
	}

	fe_reg_w32(priv->tx_phys, FE_REG_TX_BASE_PTR0);
	fe_reg_w32(NUM_DMA_DESC, FE_REG_TX_MAX_CNT0);
	fe_reg_w32(0, FE_REG_TX_CTX_IDX0);
	fe_reg_w32(FE_PST_DTX_IDX0, FE_REG_PDMA_RST_CFG);

	return 0;
}

static void fe_free_dma(struct fe_priv *priv)
{
	int i;

	for (i = 0; i < NUM_DMA_DESC; i++) {
		if (priv->rx_skb[i]) {
			dma_unmap_single(&priv->netdev->dev, priv->rx_dma[i].rxd1,
						MAX_RX_LENGTH, DMA_FROM_DEVICE);
			dev_kfree_skb_any(priv->rx_skb[i]);
			priv->rx_skb[i] = NULL;
		}

		if (priv->tx_skb[i]) {
			dev_kfree_skb_any(priv->tx_skb[i]);
			priv->tx_skb[i] = NULL;
		}
	}

	if (priv->rx_dma) {
		int size = NUM_DMA_DESC * sizeof(struct fe_rx_dma);
		dma_free_coherent(&priv->netdev->dev, size, priv->rx_dma,
					priv->rx_phys);
	}

	if (priv->tx_dma) {
		int size = NUM_DMA_DESC * sizeof(struct fe_tx_dma);
		dma_free_coherent(&priv->netdev->dev, size, priv->tx_dma,
					priv->tx_phys);
	}

	netdev_reset_queue(priv->netdev);
}

static int fe_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct fe_priv *priv = netdev_priv(dev);
	dma_addr_t mapped_addr;
	u32 tx_next;
	u32 tx;

	if (priv->soc->min_pkt_len) {
		if (skb->len < priv->soc->min_pkt_len) {
			if (skb_padto(skb, priv->soc->min_pkt_len)) {
				printk(KERN_ERR
				       "fe_eth: skb_padto failed\n");
				kfree_skb(skb);
				return 0;
			}
			skb_put(skb, priv->soc->min_pkt_len - skb->len);
		}
	}

	dev->trans_start = jiffies;
	mapped_addr = dma_map_single(&priv->netdev->dev, skb->data,
				skb->len, DMA_TO_DEVICE);

	spin_lock(&priv->page_lock);

	tx = fe_reg_r32(FE_REG_TX_CTX_IDX0);
	tx_next = (tx + 1) % NUM_DMA_DESC;

	if ((priv->tx_skb[tx]) || (priv->tx_skb[tx_next]) ||
			!(priv->tx_dma[tx].txd2 & TX_DMA_DONE) ||
			!(priv->tx_dma[tx_next].txd2 & TX_DMA_DONE))
	{
		spin_unlock(&priv->page_lock);
		dev->stats.tx_dropped++;
		kfree_skb(skb);

		return NETDEV_TX_OK;
	}

	priv->tx_skb[tx] = skb;
	priv->tx_dma[tx].txd1 = (unsigned int) mapped_addr;
	wmb();
	if (priv->soc->tx_dma)
		priv->soc->tx_dma(priv, tx, skb->len);
	else
		priv->tx_dma[tx].txd2 = TX_DMA_LSO | TX_DMA_PLEN0(skb->len);

	if (skb->ip_summed == CHECKSUM_PARTIAL)
		priv->tx_dma[tx].txd4 |= TX_DMA_CHKSUM;
	else
		priv->tx_dma[tx].txd4 &= ~TX_DMA_CHKSUM;

	priv->tx_dma[tx].txd4 &= ~0x80;

	dev->stats.tx_packets++;
	dev->stats.tx_bytes += skb->len;

	fe_reg_w32(tx_next, FE_REG_TX_CTX_IDX0);
	netdev_sent_queue(dev, skb->len);

	spin_unlock(&priv->page_lock);

	return NETDEV_TX_OK;
}

static int fe_poll_rx(struct napi_struct *napi, int budget)
{
	struct fe_priv *priv = container_of(napi, struct fe_priv, rx_napi);
	int idx = fe_reg_r32(FE_REG_RX_CALC_IDX0);
	int complete = 0;
	int rx = 0;

	while ((rx < budget) && !complete) {
		idx = (idx + 1) % NUM_DMA_DESC;

		if (priv->rx_dma[idx].rxd2 & RX_DMA_DONE) {
			struct sk_buff *new_skb = fe_alloc_skb(priv);

			if (new_skb) {
				int pktlen = RX_DMA_PLEN0(priv->rx_dma[idx].rxd2);
				dma_addr_t dma_addr;

				dma_unmap_single(&priv->netdev->dev, priv->rx_dma[idx].rxd1,
						MAX_RX_LENGTH, DMA_FROM_DEVICE);

				skb_put(priv->rx_skb[idx], pktlen);
				priv->rx_skb[idx]->dev = priv->netdev;
				priv->rx_skb[idx]->protocol = eth_type_trans(priv->rx_skb[idx], priv->netdev);
				if (priv->rx_dma[idx].rxd4 & priv->soc->checksum_bit)
					priv->rx_skb[idx]->ip_summed = CHECKSUM_UNNECESSARY;
				else
					priv->rx_skb[idx]->ip_summed = CHECKSUM_NONE;
				priv->netdev->stats.rx_packets++;
				priv->netdev->stats.rx_bytes += pktlen;
				netif_receive_skb(priv->rx_skb[idx]);

				priv->rx_skb[idx] = new_skb;

				dma_addr = dma_map_single(&priv->netdev->dev,
						  new_skb->data,
						  MAX_RX_LENGTH,
						  DMA_FROM_DEVICE);
				priv->rx_dma[idx].rxd1 = (unsigned int) dma_addr;
				wmb();
			} else {
				priv->netdev->stats.rx_dropped++;
			}

			if (priv->soc->rx_dma)
				priv->soc->rx_dma(priv, idx, MAX_RX_LENGTH);
			else
				priv->rx_dma[idx].rxd2 = RX_DMA_LSO;
			fe_reg_w32(idx, FE_REG_RX_CALC_IDX0);

			rx++;
		} else {
			complete = 1;
		}
	}

	if (complete) {
		napi_complete(&priv->rx_napi);
		fe_int_enable(priv->soc->rx_dly_int);
	}

	return rx;
}

static void fe_tx_housekeeping(unsigned long ptr)
{
	struct net_device *dev = (struct net_device*)ptr;
	struct fe_priv *priv = netdev_priv(dev);
	unsigned int bytes_compl = 0;
	unsigned int pkts_compl = 0;

	spin_lock(&priv->page_lock);
	while (1) {
		struct fe_tx_dma *txd;

		txd = &priv->tx_dma[priv->tx_free_idx];

		if (!(txd->txd2 & TX_DMA_DONE) || !(priv->tx_skb[priv->tx_free_idx]))
			break;

		bytes_compl += priv->tx_skb[priv->tx_free_idx]->len;
		pkts_compl++;

		dev_kfree_skb_irq(priv->tx_skb[priv->tx_free_idx]);
		priv->tx_skb[priv->tx_free_idx] = NULL;
		priv->tx_free_idx++;
		if (priv->tx_free_idx >= NUM_DMA_DESC)
			priv->tx_free_idx = 0;
	}

	netdev_completed_queue(priv->netdev, pkts_compl, bytes_compl);
        spin_unlock(&priv->page_lock);

	fe_int_enable(priv->soc->tx_dly_int);
}

static void fe_tx_timeout(struct net_device *dev)
{
	struct fe_priv *priv = netdev_priv(dev);

        tasklet_schedule(&priv->tx_tasklet);
	priv->netdev->stats.tx_errors++;
	netdev_err(dev, "transmit timed out, waking up the queue\n");
	netif_wake_queue(dev);
}

static irqreturn_t fe_handle_irq(int irq, void *dev)
{
	struct fe_priv *priv = netdev_priv(dev);
	unsigned int status;
	unsigned int mask;

	status = fe_reg_r32(FE_REG_FE_INT_STATUS);
	mask = fe_reg_r32(FE_REG_FE_INT_ENABLE);

	if (!(status & mask))
		return IRQ_NONE;

	if (status & priv->soc->rx_dly_int) {
		fe_int_disable(priv->soc->rx_dly_int);
		napi_schedule(&priv->rx_napi);
	}

	if (status & priv->soc->tx_dly_int) {
		fe_int_disable(priv->soc->tx_dly_int);
		tasklet_schedule(&priv->tx_tasklet);
	}

	fe_reg_w32(status, FE_REG_FE_INT_STATUS);

	return IRQ_HANDLED;
}

static int fe_hw_init(struct net_device *dev)
{
	struct fe_priv *priv = netdev_priv(dev);
	int err;

	err = devm_request_irq(priv->device, dev->irq, fe_handle_irq, 0,
				dev_name(priv->device), dev);
	if (err)
		return err;

	err = fe_alloc_rx(priv);
	if (!err)
		err = fe_alloc_tx(priv);
	if (err)
		return err;

	if (priv->soc->set_mac)
		priv->soc->set_mac(priv, dev->dev_addr);
	else
		fe_hw_set_macaddr(priv, dev->dev_addr);

	fe_reg_w32(FE_DELAY_INIT, FE_REG_DLY_INT_CFG);

	fe_int_disable(priv->soc->tx_dly_int | priv->soc->rx_dly_int);

	tasklet_init(&priv->tx_tasklet, fe_tx_housekeeping, (unsigned long)dev);

	if (priv->soc->fwd_config) {
		priv->soc->fwd_config(priv);
	} else {
		unsigned long sysclk = priv->sysclk;

		if (!sysclk) {
			netdev_err(dev, "unable to get clock\n");
			return -EINVAL;
		}

		sysclk /= FE_US_CYC_CNT_DIVISOR;
		sysclk <<= FE_US_CYC_CNT_SHIFT;

		fe_w32((fe_r32(FE_FE_GLO_CFG) &
			~(FE_US_CYC_CNT_MASK << FE_US_CYC_CNT_SHIFT)) | sysclk,
			FE_FE_GLO_CFG);

		fe_w32(fe_r32(FE_GDMA1_FWD_CFG) & ~0xffff, FE_GDMA1_FWD_CFG);
		fe_w32(fe_r32(FE_GDMA1_FWD_CFG) | (FE_GDM1_ICS_EN | FE_GDM1_TCS_EN | FE_GDM1_UCS_EN),
			FE_GDMA1_FWD_CFG);
		fe_w32(fe_r32(FE_CDMA_CSG_CFG) | (FE_ICS_GEN_EN | FE_TCS_GEN_EN | FE_UCS_GEN_EN),
			FE_CDMA_CSG_CFG);
		fe_w32(FE_PSE_FQFC_CFG_INIT, FE_PSE_FQ_CFG);
	}

	fe_w32(1, FE_FE_RST_GL);
	fe_w32(0, FE_FE_RST_GL);

	return 0;
}

static int fe_open(struct net_device *dev)
{
	struct fe_priv *priv = netdev_priv(dev);
	unsigned long flags;
	u32 val;

	spin_lock_irqsave(&priv->page_lock, flags);
	napi_enable(&priv->rx_napi);

	val = FE_TX_WB_DDONE | FE_RX_DMA_EN | FE_TX_DMA_EN;
	val |= priv->soc->pdma_glo_cfg;
	fe_reg_w32(val, FE_REG_PDMA_GLO_CFG);

	spin_unlock_irqrestore(&priv->page_lock, flags);

	if (priv->phy)
		priv->phy->start(priv);

	if (priv->soc->has_carrier && priv->soc->has_carrier(priv))
		netif_carrier_on(dev);

	netif_start_queue(dev);
	fe_int_enable(priv->soc->tx_dly_int | priv->soc->rx_dly_int);

	return 0;
}

static int fe_stop(struct net_device *dev)
{
	struct fe_priv *priv = netdev_priv(dev);
	unsigned long flags;

	fe_int_disable(priv->soc->tx_dly_int | priv->soc->rx_dly_int);

	netif_stop_queue(dev);

	if (priv->phy)
		priv->phy->stop(priv);

	spin_lock_irqsave(&priv->page_lock, flags);
	napi_disable(&priv->rx_napi);

	fe_reg_w32(fe_reg_r32(FE_REG_PDMA_GLO_CFG) &
		     ~(FE_TX_WB_DDONE | FE_RX_DMA_EN | FE_TX_DMA_EN),
		     FE_REG_PDMA_GLO_CFG);
	spin_unlock_irqrestore(&priv->page_lock, flags);

	return 0;
}

static int __init fe_init(struct net_device *dev)
{
	struct fe_priv *priv = netdev_priv(dev);
	struct device_node *port;
	int err;

	BUG_ON(!priv->soc->reset_fe);
	priv->soc->reset_fe();

	if (priv->soc->switch_init)
		priv->soc->switch_init(priv);

	net_srandom(jiffies);
	memcpy(dev->dev_addr, priv->soc->mac, ETH_ALEN);
	of_get_mac_address_mtd(priv->device->of_node, dev->dev_addr);

	err = fe_mdio_init(priv);
	if (err)
		return err;

	if (priv->phy) {
		err = priv->phy->connect(priv);
		if (err)
			goto err_mdio_cleanup;
	}

	if (priv->soc->port_init)
		for_each_child_of_node(priv->device->of_node, port)
			if (of_device_is_compatible(port, "ralink,eth-port"))
				priv->soc->port_init(priv, port);

	err = fe_hw_init(dev);
	if (err)
		goto err_phy_disconnect;

	return 0;

err_phy_disconnect:
	if (priv->phy)
		priv->phy->disconnect(priv);
err_mdio_cleanup:
	fe_mdio_cleanup(priv);

	return err;
}

static void fe_uninit(struct net_device *dev)
{
	struct fe_priv *priv = netdev_priv(dev);

	tasklet_kill(&priv->tx_tasklet);

	if (priv->phy)
		priv->phy->disconnect(priv);
	fe_mdio_cleanup(priv);

	fe_reg_w32(0, FE_REG_FE_INT_ENABLE);
	free_irq(dev->irq, dev);

	fe_free_dma(priv);
}


typedef struct rt3052_esw_reg {
	unsigned int off;
	unsigned int val;
} esw_reg;


int fe_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	esw_reg reg;

#if defined(CONFIG_SOC_RT305X_OPENWRT) 
#define REG_ESW_MAX			0x16C
#elif defined (CONFIG_SOC_MT7620_OPENWRT)
#define REG_ESW_MAX			0x7FFFF
#else //RT305x, RT3350
#define REG_ESW_MAX			0xFC
#endif


#define RAETH_ESW_REG_READ		0x89F1
#define RAETH_ESW_REG_WRITE		0x89F2
#define RAETH_MII_READ			0x89F3
#define RAETH_MII_WRITE			0x89F4
#define RAETH_ESW_INGRESS_RATE		0x89F5
#define RAETH_ESW_EGRESS_RATE		0x89F6
#define RAETH_ESW_PHY_DUMP		0x89F7
#define RALINK_ETH_SW_BASE		0xB0110000
#define _ESW_REG(x)	(*((volatile u32 *)(RALINK_ETH_SW_BASE + x)))
	switch (cmd) {
		case RAETH_ESW_REG_READ:
			copy_from_user(&reg, ifr->ifr_data, sizeof(reg));
			if (reg.off > REG_ESW_MAX)
				return -EINVAL;
			reg.val = _ESW_REG(reg.off);
			copy_to_user(ifr->ifr_data, &reg, sizeof(reg));
			break;
		case RAETH_ESW_REG_WRITE:
			copy_from_user(&reg, ifr->ifr_data, sizeof(reg));
			if (reg.off > REG_ESW_MAX)
				return -EINVAL;
			_ESW_REG(reg.off) = reg.val;
			break;
		default:
			return -EOPNOTSUPP;

	}

	return 0;
}


static const struct net_device_ops fe_netdev_ops = {
	.ndo_init		= fe_init,
	.ndo_uninit		= fe_uninit,
	.ndo_open		= fe_open,
	.ndo_stop		= fe_stop,
	.ndo_start_xmit		= fe_start_xmit,
	.ndo_tx_timeout		= fe_tx_timeout,
	.ndo_set_mac_address	= fe_set_mac_address,
	.ndo_change_mtu		= eth_change_mtu,
        .ndo_do_ioctl           = fe_ioctl,
	.ndo_validate_addr	= eth_validate_addr,
};

static int fe_probe(struct platform_device *pdev)
{
	struct resource *res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	const struct of_device_id *match;
	struct fe_soc_data *soc = NULL;
	struct net_device *netdev;
	struct fe_priv *priv;
	struct clk *sysclk;
	int err;

	match = of_match_device(of_fe_match, &pdev->dev);
	soc = (struct fe_soc_data *) match->data;
	if (soc->reg_table)
		fe_reg_table = soc->reg_table;

	fe_base = devm_request_and_ioremap(&pdev->dev, res);
	if (!fe_base)
		return -ENOMEM;

	netdev = alloc_etherdev(sizeof(struct fe_priv));
	if (!netdev) {
		dev_err(&pdev->dev, "alloc_etherdev failed\n");
		return -ENOMEM;
	}

	strcpy(netdev->name, "eth%d");
	netdev->netdev_ops = &fe_netdev_ops;
	netdev->base_addr = (unsigned long) fe_base;
	netdev->watchdog_timeo = TX_TIMEOUT;
	netdev->features |= NETIF_F_IP_CSUM | NETIF_F_RXCSUM;

	netdev->irq = platform_get_irq(pdev, 0);
	if (netdev->irq < 0) {
		dev_err(&pdev->dev, "no IRQ resource found\n");
		kfree(netdev);
		return -ENXIO;
	}

	priv = netdev_priv(netdev);
	memset(priv, 0, sizeof(struct fe_priv));
	spin_lock_init(&priv->page_lock);

	sysclk = devm_clk_get(&pdev->dev, NULL);
	if (!IS_ERR(sysclk))
		priv->sysclk = clk_get_rate(sysclk);

	priv->netdev = netdev;
	priv->device = &pdev->dev;
	priv->soc = soc;

	err = register_netdev(netdev);
	if (err) {
		dev_err(&pdev->dev, "error bringing up device\n");
		kfree(netdev);
		return err;
	}
	netif_napi_add(netdev, &priv->rx_napi, fe_poll_rx, 32);

	platform_set_drvdata(pdev, netdev);

	netdev_info(netdev, "done loading\n");

	return 0;
}

static int fe_remove(struct platform_device *pdev)
{
        struct net_device *dev = platform_get_drvdata(pdev);
	struct fe_priv *priv = netdev_priv(dev);

	netif_stop_queue(dev);
	netif_napi_del(&priv->rx_napi);

	unregister_netdev(dev);
	free_netdev(dev);

	return 0;
}

static struct platform_driver fe_driver = {
	.probe = fe_probe,
	.remove = fe_remove,
	.driver = {
		.name = "ralink_soc_eth",
		.owner = THIS_MODULE,
		.of_match_table = of_fe_match,
	},
};

static int __init init_rtfe(void)
{
	int ret;

	ret = rtesw_init();
	if (ret)
		return ret;

	ret = platform_driver_register(&fe_driver);
	if (ret)
		rtesw_exit();

	return ret;
}

static void __exit exit_rtfe(void)
{
	platform_driver_unregister(&fe_driver);
	rtesw_exit();
}

module_init(init_rtfe);
module_exit(exit_rtfe);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("John Crispin <blogic@openwrt.org>");
MODULE_DESCRIPTION("Ethernet driver for Ralink SoC");
