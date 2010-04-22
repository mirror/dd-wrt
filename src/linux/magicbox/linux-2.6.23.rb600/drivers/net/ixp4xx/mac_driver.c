/*
 * mac_driver.c - provide a network interface for each MAC
 *
 * Copyright (C) 2006 Christian Hohnstaedt <chohnstaedt@innominate.com>
 *
 * This file is released under the GPLv2
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/dma-mapping.h>
#include <linux/workqueue.h>
#include <asm/io.h>
#include <asm/irq.h>


#include <linux/ixp_qmgr.h>
#include <linux/ixp_npe.h>
#include "mac.h"

#define MDIO_INTERVAL (3*HZ)
#define RX_QUEUE_PREFILL 64
#define TX_QUEUE_PREFILL 16

#define IXMAC_NAME "ixp4xx_mac"
#define IXMAC_VERSION "0.3.1"

#define MAC_DEFAULT_REG(mac, name) \
	mac_write_reg(mac, MAC_ ## name, MAC_ ## name ## _DEFAULT)

#define TX_DONE_QID 31

#define DMA_ALLOC_SIZE 2048
#define DMA_HDR_SIZE   (sizeof(struct npe_cont))
#define DMA_BUF_SIZE (DMA_ALLOC_SIZE - DMA_HDR_SIZE)

/* Since the NPEs use 1 Return Q for sent frames, we need a device
 * independent return Q. We call it tx_doneq.
 * It will be initialized during module load and uninitialized
 * during module unload. Evil hack, but there is no choice :-(
 */

static struct qm_queue *tx_doneq = NULL;
static int debug = -1;
module_param(debug, int, 0);

static int init_buffer(struct qm_queue *queue, int count)
{
	int i;
	struct npe_cont *cont;

	for (i=0; i<count; i++) {
		cont = kmalloc(DMA_ALLOC_SIZE, GFP_KERNEL | GFP_DMA);
		if (!cont)
			goto err;

		cont->phys = dma_map_single(queue->dev, cont, DMA_ALLOC_SIZE,
				DMA_BIDIRECTIONAL);
		if (dma_mapping_error(cont->phys))
			goto err;

		cont->data = cont+1;
		/* now the buffer is on a 32 bit boundary.
		 * we add 2 bytes for good alignment to SKB */
		cont->data+=2;
		cont->eth.next = 0;
		cont->eth.buf_len = cpu_to_npe16(DMA_BUF_SIZE);
		cont->eth.pkt_len = 0;
		/* also add 2 alignment bytes from cont->data*/
		cont->eth.phys_addr = cpu_to_npe32(cont->phys+ DMA_HDR_SIZE+ 2);

		dma_sync_single(queue->dev, cont->phys, DMA_HDR_SIZE,
				DMA_TO_DEVICE);

		queue_put_entry(queue, cont->phys);
		if (queue_stat(queue) == 2) { /* overflow */
			dma_unmap_single(queue->dev, cont->phys, DMA_ALLOC_SIZE,
				DMA_BIDIRECTIONAL);
			goto err;
		}
	}
	return i;
err:
	if (cont)
		kfree(cont);
	return i;
}

static int destroy_buffer(struct qm_queue *queue, int count)
{
	u32 phys;
	int i;
	struct npe_cont *cont;

	for (i=0; i<count; i++) {
		phys = queue_get_entry(queue) & ~0xf;
		if (!phys)
			break;
		dma_unmap_single(queue->dev, phys, DMA_ALLOC_SIZE,
				DMA_BIDIRECTIONAL);
		cont = dma_to_virt(queue->dev, phys);
		kfree(cont);
	}
	return i;
}

static void mac_init(struct mac_info *mac)
{
	MAC_DEFAULT_REG(mac, TX_CNTRL2);
	MAC_DEFAULT_REG(mac, RANDOM_SEED);
	MAC_DEFAULT_REG(mac, THRESH_P_EMPTY);
	MAC_DEFAULT_REG(mac, THRESH_P_FULL);
	MAC_DEFAULT_REG(mac, TX_DEFER);
	MAC_DEFAULT_REG(mac, TX_TWO_DEFER_1);
	MAC_DEFAULT_REG(mac, TX_TWO_DEFER_2);
	MAC_DEFAULT_REG(mac, SLOT_TIME);
	MAC_DEFAULT_REG(mac, INT_CLK_THRESH);
	MAC_DEFAULT_REG(mac, BUF_SIZE_TX);
	MAC_DEFAULT_REG(mac, TX_CNTRL1);
	MAC_DEFAULT_REG(mac, RX_CNTRL1);
}

static void mac_set_uniaddr(struct net_device *dev)
{
	int i;
	struct mac_info *mac = netdev_priv(dev);
	struct npe_info *npe = dev_get_drvdata(mac->npe_dev);

	/* check for multicast */
	if (dev->dev_addr[0] & 1)
		return;

	npe_mh_setportaddr(npe, mac->plat, dev->dev_addr);
	npe_mh_disable_firewall(npe, mac->plat);
	for (i=0; i<dev->addr_len; i++)
		mac_write_reg(mac, MAC_UNI_ADDR + i, dev->dev_addr[i]);
}

static void update_duplex_mode(struct net_device *dev)
{
	struct mac_info *mac = netdev_priv(dev);
	if (netif_msg_link(mac)) {
		printk(KERN_DEBUG "Link of %s is %s-duplex\n", dev->name,
				mac->mii.full_duplex ? "full" : "half");
	}
	if (mac->mii.full_duplex) {
		mac_reset_regbit(mac, MAC_TX_CNTRL1, TX_CNTRL1_DUPLEX);
	} else {
		mac_set_regbit(mac, MAC_TX_CNTRL1, TX_CNTRL1_DUPLEX);
	}
}

static int media_check(struct net_device *dev, int init)
{
	struct mac_info *mac = netdev_priv(dev);

	if (mii_check_media(&mac->mii, netif_msg_link(mac), init)) {
		update_duplex_mode(dev);
		return 1;
	}
	return 0;
}

static void get_npe_stats(struct mac_info *mac, u32 *buf, int len, int reset)
{
	struct npe_info *npe = dev_get_drvdata(mac->npe_dev);
	u32 phys;

	memset(buf, len, 0);
	phys = dma_map_single(mac->npe_dev, buf, len, DMA_BIDIRECTIONAL);
	npe_mh_get_stats(npe, mac->plat, phys, reset);
	dma_unmap_single(mac->npe_dev, phys, len, DMA_BIDIRECTIONAL);
}

static void irqcb_recv(struct qm_queue *queue)
{
	struct net_device *dev = queue->cb_data;

	queue_ack_irq(queue);
	queue_disable_irq(queue);
	if (netif_running(dev))
		netif_rx_schedule(dev);
}

int ix_recv(struct net_device *dev, int *budget, struct qm_queue *queue)
{
	struct mac_info *mac = netdev_priv(dev);
	struct sk_buff *skb;
	u32 phys;
	struct npe_cont *cont;

	while (*budget > 0 && netif_running(dev) ) {
		int len;
		phys = queue_get_entry(queue) & ~0xf;
		if (!phys)
			break;
		dma_sync_single(queue->dev, phys, DMA_HDR_SIZE,
				DMA_FROM_DEVICE);
		cont = dma_to_virt(queue->dev, phys);
		len = npe_to_cpu16(cont->eth.pkt_len) -4; /* strip FCS */

		if (unlikely(netif_msg_rx_status(mac))) {
			printk(KERN_DEBUG "%s: RX packet size: %u\n",
				dev->name, len);
			queue_state(mac->rxq);
			queue_state(mac->rxdoneq);
		}
		skb = dev_alloc_skb(len + 2);
		if (likely(skb)) {
			skb->dev = dev;
			skb_reserve(skb, 2);
			dma_sync_single(queue->dev, cont->eth.phys_addr, len,
					DMA_FROM_DEVICE);
#ifdef CONFIG_NPE_ADDRESS_COHERENT
		        /* swap the payload of the SKB */
	        	{
				u32 *t = (u32*)(skb->data-2);
				u32 *s = (u32*)(cont->data-2);
				int i, j = (len+5)/4;
				for (i=0; i<j; i++)
					t[i] = cpu_to_be32(s[i]);
			}
#else			
			eth_copy_and_sum(skb, cont->data, len, 0);
#endif
			skb_put(skb, len);
			skb->protocol = eth_type_trans(skb, dev);
			dev->last_rx = jiffies;
			netif_receive_skb(skb);
			mac->stat.rx_packets++;
			mac->stat.rx_bytes += skb->len;
		} else {
			mac->stat.rx_dropped++;
		}
		cont->eth.buf_len = cpu_to_npe16(DMA_BUF_SIZE);
		cont->eth.pkt_len = 0;
		dma_sync_single(queue->dev, phys, DMA_HDR_SIZE, DMA_TO_DEVICE);
		queue_put_entry(mac->rxq, phys);
		dev->quota--;
		(*budget)--;
	}

	return !budget;
}

static int ix_poll(struct net_device *dev, int *budget)
{
	struct mac_info *mac = netdev_priv(dev);
	struct qm_queue *queue = mac->rxdoneq;

	for (;;) {
		if (ix_recv(dev, budget, queue))
			return 1;
		netif_rx_complete(dev);
		queue_enable_irq(queue);
		if (!queue_len(queue))
			break;
		queue_disable_irq(queue);
		if (netif_rx_reschedule(dev, 0))
			break;
	}
	return 0;
}

static void ixmac_set_rx_mode (struct net_device *dev)
{
	struct mac_info *mac = netdev_priv(dev);
	struct dev_mc_list *mclist;
	u8 aset[dev->addr_len], aclear[dev->addr_len];
	int i,j;

	if (dev->flags & IFF_PROMISC) {
		mac_reset_regbit(mac, MAC_RX_CNTRL1, RX_CNTRL1_ADDR_FLTR_EN);
	} else {
		mac_set_regbit(mac, MAC_RX_CNTRL1, RX_CNTRL1_ADDR_FLTR_EN);

		mclist = dev->mc_list;
		memset(aset, 0xff, dev->addr_len);
		memset(aclear, 0x00, dev->addr_len);
		for (i = 0; mclist && i < dev->mc_count; i++) {
			for (j=0; j< dev->addr_len; j++) {
				aset[j] &= mclist->dmi_addr[j];
				aclear[j] |= mclist->dmi_addr[j];
			}
			mclist = mclist->next;
		}
		for (j=0; j< dev->addr_len; j++) {
			aclear[j] = aset[j] | ~aclear[j];
		}
		for (i=0; i<dev->addr_len; i++) {
			mac_write_reg(mac, MAC_ADDR + i, aset[i]);
			mac_write_reg(mac, MAC_ADDR_MASK + i, aclear[i]);
		}
	}
}

static int ixmac_open (struct net_device *dev)
{
	struct mac_info *mac = netdev_priv(dev);
	struct npe_info *npe = dev_get_drvdata(mac->npe_dev);
	u32 buf[NPE_STAT_NUM];
	int i;
	u32 phys;

	/* first check if the NPE is up and running */
	if (!( npe_status(npe) & IX_NPEDL_EXCTL_STATUS_RUN)) {
		printk(KERN_ERR "%s: %s not running\n", dev->name,
				npe->plat->name);
		return -EIO;
	}
	if (npe_mh_status(npe)) {
		printk(KERN_ERR "%s: %s not responding\n", dev->name,
				npe->plat->name);
		return -EIO;
	}
	mac->txq_pkt += init_buffer(mac->txq, TX_QUEUE_PREFILL - mac->txq_pkt);
	mac->rxq_pkt += init_buffer(mac->rxq, RX_QUEUE_PREFILL - mac->rxq_pkt);

	queue_enable_irq(mac->rxdoneq);

	/* drain all buffers from then RX-done-q to make the IRQ happen */
	while ((phys = queue_get_entry(mac->rxdoneq) & ~0xf)) {
		struct npe_cont *cont;
		cont = dma_to_virt(mac->rxdoneq->dev, phys);
		cont->eth.buf_len = cpu_to_npe16(DMA_BUF_SIZE);
		cont->eth.pkt_len = 0;
		dma_sync_single(mac->rxdoneq->dev, phys, DMA_HDR_SIZE,
				DMA_TO_DEVICE);
		queue_put_entry(mac->rxq, phys);
	}
	mac_init(mac);
	npe_mh_set_rxqid(npe, mac->plat, mac->plat->rxdoneq_id);
	get_npe_stats(mac, buf, sizeof(buf), 1); /* reset stats */
	get_npe_stats(mac, buf, sizeof(buf), 0);
	/*
	 * if the extended stats contain random values
	 * the NPE image lacks extendet statistic counters
	 */
	for (i=NPE_STAT_NUM_BASE; i<NPE_STAT_NUM; i++) {
		if (buf[i] >10000)
			break;
	}
	mac->npe_stat_num = i<NPE_STAT_NUM ? NPE_STAT_NUM_BASE : NPE_STAT_NUM;
	mac->npe_stat_num += NPE_Q_STAT_NUM;

	mac_set_uniaddr(dev);
	media_check(dev, 1);
	ixmac_set_rx_mode(dev);
	netif_start_queue(dev);
	schedule_delayed_work(&mac->mdio_thread, MDIO_INTERVAL);
	if (netif_msg_ifup(mac)) {
		 printk(KERN_DEBUG "%s: open " IXMAC_NAME
		 	" RX queue %d bufs, TX queue %d bufs\n",
			 dev->name, mac->rxq_pkt, mac->txq_pkt);
	}
	return 0;
}

static int ixmac_start_xmit (struct sk_buff *skb, struct net_device *dev)
{
	struct mac_info *mac = netdev_priv(dev);
	struct npe_cont *cont;
	u32 phys;
	struct qm_queue *queue = mac->txq;

	if (unlikely(skb->len > DMA_BUF_SIZE)) {
		dev_kfree_skb(skb);
		mac->stat.tx_errors++;
		return NETDEV_TX_OK;
	}
	phys = queue_get_entry(tx_doneq) & ~0xf;
	if (!phys)
		goto busy;
	cont = dma_to_virt(queue->dev, phys);
#ifdef CONFIG_NPE_ADDRESS_COHERENT
	/* swap the payload of the SKB */
	{
		u32 *s = (u32*)(skb->data-2);
		u32 *t = (u32*)(cont->data-2);
		int i,j = (skb->len+5) / 4;
		for (i=0; i<j; i++)
			t[i] = cpu_to_be32(s[i]);
	}
#else			
	//skb_copy_and_csum_dev(skb, cont->data);
	memcpy(cont->data, skb->data, skb->len);
#endif
	cont->eth.buf_len = cpu_to_npe16(DMA_BUF_SIZE);
	cont->eth.pkt_len = cpu_to_npe16(skb->len);
	/* disable VLAN functions in NPE image for now */
	cont->eth.flags = 0;
	dma_sync_single(queue->dev, phys, skb->len + DMA_HDR_SIZE,
			DMA_TO_DEVICE);
	queue_put_entry(queue, phys);
	if (queue_stat(queue) == 2) { /* overflow */
		queue_put_entry(tx_doneq, phys);
		goto busy;
	}
        dev_kfree_skb(skb);

	mac->stat.tx_packets++;
	mac->stat.tx_bytes += skb->len;
	dev->trans_start = jiffies;
	if (netif_msg_tx_queued(mac)) {
		printk(KERN_DEBUG "%s: TX packet size %u\n",
				dev->name, skb->len);
		queue_state(mac->txq);
		queue_state(tx_doneq);
	}
	return NETDEV_TX_OK;
busy:
	return NETDEV_TX_BUSY;
}

static int ixmac_close (struct net_device *dev)
{
	struct mac_info *mac = netdev_priv(dev);

	netif_stop_queue (dev);
	queue_disable_irq(mac->rxdoneq);

	mac->txq_pkt -= destroy_buffer(tx_doneq, mac->txq_pkt);
	mac->rxq_pkt -= destroy_buffer(mac->rxq, mac->rxq_pkt);

	cancel_rearming_delayed_work(&(mac->mdio_thread));

	if (netif_msg_ifdown(mac)) {
		printk(KERN_DEBUG "%s: close " IXMAC_NAME
			" RX queue %d bufs, TX queue %d bufs\n",
			dev->name, mac->rxq_pkt, mac->txq_pkt);
	}
	return 0;
}

static int ixmac_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
	struct mac_info *mac = netdev_priv(dev);
	int rc, duplex_changed;

	if (!netif_running(dev))
		return -EINVAL;
	if (!try_module_get(THIS_MODULE))
		return -ENODEV;

	rc = generic_mii_ioctl(&mac->mii, if_mii(rq), cmd, &duplex_changed);
	module_put(THIS_MODULE);
	if (duplex_changed)
		update_duplex_mode(dev);
	return rc;
}

static struct net_device_stats *ixmac_stats (struct net_device *dev)
{
	struct mac_info *mac = netdev_priv(dev);
	return &mac->stat;
}

static void ixmac_get_drvinfo(struct net_device *dev, struct ethtool_drvinfo *info)
{
	struct mac_info *mac = netdev_priv(dev);
	struct npe_info *npe = dev_get_drvdata(mac->npe_dev);

	strcpy(info->driver, IXMAC_NAME);
	strcpy(info->version, IXMAC_VERSION);
	if  (npe_status(npe) & IX_NPEDL_EXCTL_STATUS_RUN) {
		snprintf(info->fw_version, 32, "%d.%d func [%d]",
			npe->img_info[2], npe->img_info[3], npe->img_info[1]);
	}
	strncpy(info->bus_info, npe->plat->name, ETHTOOL_BUSINFO_LEN);
}

static int ixmac_get_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
	struct mac_info *mac = netdev_priv(dev);
	mii_ethtool_gset(&mac->mii, cmd);
	return 0;
}

static int ixmac_set_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
	struct mac_info *mac = netdev_priv(dev);
	int rc;
	rc = mii_ethtool_sset(&mac->mii, cmd);
	return rc;
}

static int ixmac_nway_reset(struct net_device *dev)
{
	struct mac_info *mac = netdev_priv(dev);
	return mii_nway_restart(&mac->mii);
}

static u32 ixmac_get_link(struct net_device *dev)
{
	struct mac_info *mac = netdev_priv(dev);
	return mii_link_ok(&mac->mii);
}

static const int mac_reg_list[] = MAC_REG_LIST;

static int ixmac_get_regs_len(struct net_device *dev)
{
	return ARRAY_SIZE(mac_reg_list);
}

static void
ixmac_get_regs(struct net_device *dev, struct ethtool_regs *regs, void *regbuf)
{
	int i;
	struct mac_info *mac = netdev_priv(dev);
	u8 *buf = regbuf;

	for (i=0; i<regs->len; i++) {
		buf[i] = mac_read_reg(mac, mac_reg_list[i]);
	}
}

static struct {
	const char str[ETH_GSTRING_LEN];
} ethtool_stats_keys[NPE_STAT_NUM + NPE_Q_STAT_NUM] = {
	NPE_Q_STAT_STRINGS
	NPE_STAT_STRINGS
};

static void ixmac_get_strings(struct net_device *dev, u32 stringset, u8 *data)
{
	struct mac_info *mac = netdev_priv(dev);
	memcpy(data, ethtool_stats_keys, mac->npe_stat_num * ETH_GSTRING_LEN);
}

static int ixmac_get_stats_count(struct net_device *dev)
{
	struct mac_info *mac = netdev_priv(dev);
	return mac->npe_stat_num;
}

static u32 ixmac_get_msglevel(struct net_device *dev)
{
	struct mac_info *mac = netdev_priv(dev);
	return mac->msg_enable;
}

static void ixmac_set_msglevel(struct net_device *dev, u32 datum)
{
	struct mac_info *mac = netdev_priv(dev);
	mac->msg_enable = datum;
}

static void ixmac_get_ethtool_stats(struct net_device *dev,
		struct ethtool_stats *stats, u64 *data)
{
	int i;
	struct mac_info *mac = netdev_priv(dev);
	u32 buf[NPE_STAT_NUM];

	data[0] = queue_len(mac->rxq);
	data[1] = queue_len(mac->rxdoneq);
	data[2] = queue_len(mac->txq);
	data[3] = queue_len(tx_doneq);

	get_npe_stats(mac, buf, sizeof(buf), 0);

	for (i=0; i<stats->n_stats-4; i++) {
		data[i+4] = npe_to_cpu32(buf[i]);
	}
}

static struct ethtool_ops ixmac_ethtool_ops = {
	.get_drvinfo		= ixmac_get_drvinfo,
	.get_settings		= ixmac_get_settings,
	.set_settings		= ixmac_set_settings,
	.nway_reset		= ixmac_nway_reset,
	.get_link		= ixmac_get_link,
	.get_msglevel           = ixmac_get_msglevel,
	.set_msglevel           = ixmac_set_msglevel,
	.get_regs_len		= ixmac_get_regs_len,
	.get_regs		= ixmac_get_regs,
	.get_perm_addr		= ethtool_op_get_perm_addr,
	.get_strings		= ixmac_get_strings,
	.get_stats_count	= ixmac_get_stats_count,
	.get_ethtool_stats	= ixmac_get_ethtool_stats,
};

static void mac_mdio_thread(struct work_struct *work)
{
	struct mac_info *mac = container_of(work, struct mac_info,
			mdio_thread.work);
	struct net_device *dev = mac->netdev;

	media_check(dev, 0);
	schedule_delayed_work(&mac->mdio_thread, MDIO_INTERVAL);
}

static int mac_probe(struct platform_device *pdev)
{
	struct resource *res;
	struct mac_info *mac;
	struct net_device *dev;
	struct npe_info *npe;
	struct mac_plat_info *plat = pdev->dev.platform_data;
	int size, ret;

	if (!(res = platform_get_resource(pdev, IORESOURCE_MEM, 0))) {
		return -EIO;
	}
	if (!(dev = alloc_etherdev (sizeof(struct mac_info)))) {
		return -ENOMEM;
	}
	SET_MODULE_OWNER(dev);
	sprintf(dev->name, "ixp%d", pdev->id);
	SET_NETDEV_DEV(dev, &pdev->dev);
	mac = netdev_priv(dev);
	mac->netdev = dev;

	size = res->end - res->start +1;
	mac->res = request_mem_region(res->start, size, IXMAC_NAME);
	if (!mac->res) {
		ret = -EBUSY;
		goto out_free;
	}

	mac->addr = ioremap(res->start, size);
	if (!mac->addr) {
		ret = -ENOMEM;
		goto out_rel;
	}

	dev->open = ixmac_open;
	dev->hard_start_xmit = ixmac_start_xmit;
	dev->poll = ix_poll;
	dev->stop = ixmac_close;
	dev->get_stats = ixmac_stats;
	dev->do_ioctl = ixmac_ioctl;
	dev->set_multicast_list = ixmac_set_rx_mode;
	dev->ethtool_ops = &ixmac_ethtool_ops;

	dev->weight = 16;
	dev->tx_queue_len = 100;

	mac->npe_dev = get_npe_by_id(plat->npe_id);
	if (!mac->npe_dev) {
		ret = -EIO;
		goto out_unmap;
	}
	npe = dev_get_drvdata(mac->npe_dev);

	mac->rxq = request_queue(plat->rxq_id, 128);
	if (IS_ERR(mac->rxq)) {
		printk(KERN_ERR "Error requesting Q: %d\n", plat->rxq_id);
		ret = -EBUSY;
		goto out_putmod;
	}
	mac->txq = request_queue(plat->txq_id, 128);
	if (IS_ERR(mac->txq)) {
		printk(KERN_ERR "Error requesting Q: %d\n", plat->txq_id);
		ret = -EBUSY;
		goto out_putmod;
	}
	mac->rxdoneq = request_queue(plat->rxdoneq_id, 128);
	if (IS_ERR(mac->rxdoneq)) {
		printk(KERN_ERR "Error requesting Q: %d\n", plat->rxdoneq_id);
		ret = -EBUSY;
		goto out_putmod;
	}
	mac->rxdoneq->irq_cb = irqcb_recv;
	mac->rxdoneq->cb_data = dev;
	queue_set_watermarks(mac->rxdoneq, 0, 0);
	queue_set_irq_src(mac->rxdoneq, Q_IRQ_ID_NOT_E);

	mac->qmgr = dev_get_drvdata(mac->rxq->dev);
	if (register_netdev (dev)) {
		ret = -EIO;
		goto out_putmod;
	}

	mac->plat = plat;
	mac->npe_stat_num = NPE_STAT_NUM_BASE;
	mac->msg_enable = netif_msg_init(debug, MAC_DEF_MSG_ENABLE);

	platform_set_drvdata(pdev, dev);

	mac_write_reg(mac, MAC_CORE_CNTRL, CORE_RESET);
	udelay(500);
	mac_write_reg(mac, MAC_CORE_CNTRL, CORE_MDC_EN);

	if (plat->phy_id==0)
	    {
	    init_mdio(dev, plat->phy_id,0);
	    int reg1 = mdio_read_register(dev, 1, 0x02);
	    reg1=mdio_read_register(dev, 1, 0x02);
	    int reg2 = mdio_read_register(dev, 1, 0x03);
		printk(KERN_INFO IXMAC_NAME " PhyID 0x%04X:0x%04X\n",reg1,reg2);
	    	
	    if (reg1 == 0x22 && reg2 == 0x1450)
		{
		printk(KERN_INFO IXMAC_NAME " GW2345 Kendin Switch detected\n");
		plat->phy_id = 1;
	        init_mdio(dev, plat->phy_id,1);
		}
	    }
	else if (plat->phy_id==1)
	    {
	    init_mdio(dev, plat->phy_id,0);
	    int reg1 = mdio_read_register(dev, 1, 0x02);
	    reg1=mdio_read_register(dev, 1, 0x02);
	    int reg2 = mdio_read_register(dev, 1, 0x03);
		printk(KERN_INFO IXMAC_NAME " PhyID 0x%04X:0x%04X\n",reg1,reg2);
	    	
	    if (reg1 == 0x22 && reg2 == 0x1450)
		{
		printk(KERN_INFO IXMAC_NAME " GW2345 WAN Port detected\n");
		plat->phy_id = 5;
	        init_mdio(dev, plat->phy_id,0);
		}
	    }
	else 
	    init_mdio(dev, plat->phy_id,0);



	INIT_DELAYED_WORK(&mac->mdio_thread, mac_mdio_thread);

	/* The place of the MAC address is very system dependent.
	 * Here we use a random one to be replaced by one of the
	 * following commands:
	 * "ip link set address 02:03:04:04:04:01 dev eth0"
	 * "ifconfig eth0 hw ether 02:03:04:04:04:07"
	*/

	if (is_zero_ether_addr(plat->hwaddr)) {
		random_ether_addr(dev->dev_addr);
		dev->dev_addr[5] = plat->phy_id;
	}
	else
		memcpy(dev->dev_addr, plat->hwaddr, 6);

	printk(KERN_INFO IXMAC_NAME " driver " IXMAC_VERSION
			": %s on %s with PHY[%d] initialized\n",
			dev->name, npe->plat->name, plat->phy_id);

	return 0;

out_putmod:
	if (mac->rxq)
		release_queue(mac->rxq);
	if (mac->txq)
		release_queue(mac->txq);
	if (mac->rxdoneq)
		release_queue(mac->rxdoneq);
	module_put(mac->npe_dev->driver->owner);
out_unmap:
	iounmap(mac->addr);
out_rel:
	release_resource(mac->res);
out_free:
	kfree(mac);
	return ret;
}

static void drain_npe(struct mac_info *mac)
{
	struct npe_info *npe = dev_get_drvdata(mac->npe_dev);
	struct npe_cont *cont;
	u32 phys;
	int loop = 0;

	/* Now there are some skb hold by the NPE.
	 * We switch the MAC in loopback mode and send a pseudo packet
	 * that will be returned by the NPE in its last SKB.
	 * We will also try to isolate the PHY to keep the packets internal.
	 */

	if (mac->txq_pkt <2)
		mac->txq_pkt += init_buffer(tx_doneq, 5);

	if (npe_status(npe) & IX_NPEDL_EXCTL_STATUS_RUN) {
		mac_reset_regbit(mac, MAC_CORE_CNTRL, CORE_MDC_EN);
		mac_set_regbit(mac, MAC_RX_CNTRL1, RX_CNTRL1_LOOP_EN);

		npe_mh_npe_loopback_mode(npe, mac->plat, 1);
		mdelay(200);

		while (mac->rxq_pkt && loop++ < 2000 ) {
			phys = queue_get_entry(tx_doneq) & ~0xf;
			if (!phys)
				break;
			cont = dma_to_virt(queue->dev, phys);
			/* actually the packets should never leave the system,
			 * but if they do, they shall contain 0s instead of
			 * intresting random data....
			 */
			memset(cont->data, 0, 64);
			cont->eth.pkt_len = 64;
			dma_sync_single(mac->txq->dev, phys, 64 + DMA_HDR_SIZE,
					DMA_TO_DEVICE);
			queue_put_entry(mac->txq, phys);
			if (queue_stat(mac->txq) == 2) { /* overflow */
				queue_put_entry(tx_doneq, phys);
				break;
			}
			mdelay(1);
			mac->rxq_pkt -= destroy_buffer(mac->rxdoneq,
					mac->rxq_pkt);
		}
		npe_mh_npe_loopback_mode(npe, mac->plat, 0);
	}
	/* Flush MAC TX fifo to drain the bogus packages */
	mac_set_regbit(mac, MAC_CORE_CNTRL, CORE_TX_FIFO_FLUSH);
	mac_reset_regbit(mac, MAC_RX_CNTRL1, RX_CNTRL1_RX_EN);
	mac_reset_regbit(mac, MAC_TX_CNTRL1, TX_CNTRL1_TX_EN);
	mac_reset_regbit(mac, MAC_RX_CNTRL1, RX_CNTRL1_LOOP_EN);
	mac_reset_regbit(mac, MAC_CORE_CNTRL, CORE_TX_FIFO_FLUSH);
	mac_reset_regbit(mac, MAC_CORE_CNTRL, CORE_TX_FIFO_FLUSH);
}

static int mac_remove(struct platform_device *pdev)
{
	struct net_device* dev = platform_get_drvdata(pdev);
	struct mac_info *mac = netdev_priv(dev);

	unregister_netdev(dev);

	mac->rxq_pkt -= destroy_buffer(mac->rxq, mac->rxq_pkt);
	if (mac->rxq_pkt)
		drain_npe(mac);

	mac->txq_pkt -= destroy_buffer(mac->txq, mac->txq_pkt);
	mac->txq_pkt -= destroy_buffer(tx_doneq, mac->txq_pkt);

	if (mac->rxq_pkt || mac->txq_pkt)
		printk("Buffers lost in NPE: RX:%d, TX:%d\n",
				mac->rxq_pkt,  mac->txq_pkt);

	release_queue(mac->txq);
	release_queue(mac->rxq);
	release_queue(mac->rxdoneq);

	flush_scheduled_work();
	return_npe_dev(mac->npe_dev);

	iounmap(mac->addr);
	release_resource(mac->res);
	platform_set_drvdata(pdev, NULL);
	free_netdev(dev);
	return 0;
}

static struct platform_driver ixp4xx_mac = {
	.driver.name    = IXMAC_NAME,
	.probe          = mac_probe,
	.remove         = mac_remove,
};

static int __init init_mac(void)
{
	/* The TX done Queue handles skbs sent out by the NPE */
	tx_doneq = request_queue(TX_DONE_QID, 128);
	if (IS_ERR(tx_doneq)) {
		printk(KERN_ERR "Error requesting Q: %d\n", TX_DONE_QID);
		return -EBUSY;
	}
	return platform_driver_register(&ixp4xx_mac);
}

static void __exit finish_mac(void)
{
	platform_driver_unregister(&ixp4xx_mac);
	if (tx_doneq) {
		release_queue(tx_doneq);
	}
}

module_init(init_mac);
module_exit(finish_mac);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Christian Hohnstaedt <chohnstaedt@innominate.com>");

