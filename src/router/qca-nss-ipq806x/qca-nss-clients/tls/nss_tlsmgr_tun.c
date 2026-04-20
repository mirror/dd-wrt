/*
 **************************************************************************
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE
 **************************************************************************
 */

/*
 * nss_tlsmgr_tun.c
 *	NSS TLS Manager tunnel onbject
 */

#include <linux/version.h>
#include <linux/types.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/debugfs.h>
#include <linux/rtnetlink.h>
#include <net/ipv6.h>
#include <linux/if_arp.h>
#include <linux/etherdevice.h>
#include <linux/atomic.h>
#include <linux/tlshdr.h>
#include <crypto/aes.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0)
#include <crypto/sha.h>
#else
#include <crypto/sha1.h>
#include <crypto/sha2.h>
#endif

#include <nss_api_if.h>
#include <nss_dynamic_interface.h>

#include <nss_tls.h>
#include <nss_tlsmgr.h>
#include "nss_tlsmgr_priv.h"
#include "nss_tlsmgr_ctx.h"
#include "nss_tlsmgr_crypto.h"
#include "nss_tlsmgr_tun.h"

/*
 * nss_tlsmgr_tun_get_headroom()
 *	Get headroom required for encapsulation.
 */
uint16_t nss_tlsmgr_tun_get_headroom(struct net_device *dev)
{
	return dev->needed_headroom;
}
EXPORT_SYMBOL(nss_tlsmgr_tun_get_headroom);

/*
 * nss_tlsmgr_tun_get_tailroom()
 *	Get tailroom required for encapsulation.
 */
uint16_t nss_tlsmgr_tun_get_tailroom(struct net_device *dev)
{
	return dev->needed_tailroom;
}
EXPORT_SYMBOL(nss_tlsmgr_tun_get_tailroom);

/*
 * nss_tlsmgr_tun_tx()
 *	Transmit packet to TLS node in NSS firmware.
 */
static netdev_tx_t nss_tlsmgr_tun_tx(struct sk_buff *skb, struct net_device *dev)
{
	nss_tlsmgr_warn("%px: TLS device xmit function invoked", dev);
	dev_kfree_skb_any(skb);
	return NETDEV_TX_OK;
}

/*
 * nss_tlsmgr_tun_close()
 *	Stop packet transmission on the TLS network device.
 */
static int nss_tlsmgr_tun_close(struct net_device *dev)
{
	netif_stop_queue(dev);
	return 0;
}

/*
 * nss_tlsmgr_tun_open()
 *	Start processing packets on the TLS network device.
 */
static int nss_tlsmgr_tun_open(struct net_device *dev)
{
	netif_start_queue(dev);
	return 0;
}

/*
 * nss_tlsmgr_tun_stats64()
 *	TLS manager tunnel device
 */
static struct rtnl_link_stats64 *nss_tlsmgr_get_tun_stats64(struct net_device *dev, struct rtnl_link_stats64 *stats)
{
	struct nss_tlsmgr_tun *tun = netdev_priv(dev);

	memset(stats, 0, sizeof(*stats));

	read_lock_bh(&tun->lock);
	nss_tlsmgr_ctx_stats_copy(&tun->ctx_enc, stats);
	nss_tlsmgr_ctx_stats_copy(&tun->ctx_dec, stats);
	read_unlock_bh(&tun->lock);

	return stats;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 6, 0))
/*
 * nss_tlsmgr_tun_stats64()
 * 	Netdev ops function to retrieve stats for kernel version < 4.6
 */
static struct rtnl_link_stats64 *nss_tlsmgr_tun_stats64(struct net_device *dev, struct rtnl_link_stats64 *stats)
{
	return nss_tlsmgr_get_tun_stats64(dev, stats);
}
#else
/*
 * nss_tlsmgr_tun_stats64()
 * 	Netdev ops function to retrieve stats for kernel version >= 4.6
 */
static void nss_tlsmgr_tun_stats64(struct net_device *dev, struct rtnl_link_stats64 *stats)
{
	nss_tlsmgr_get_tun_stats64(dev, stats);
}
#endif

/*
 * nss_tlsmgr_tun_change_mtu()
 *	Change MTU size of TLS context device.
 */
static int32_t nss_tlsmgr_tun_change_mtu(struct net_device *dev, int32_t mtu)
{
	/*
	 * TODO: Send a ETH_NODE message to firmware for updating the MTU
	 */
	dev->mtu = mtu;
	return 0;
}

/*
 * TLS netdev ops
 */
static const struct net_device_ops nss_tlsmgr_tun_ops = {
	.ndo_start_xmit = nss_tlsmgr_tun_tx,
	.ndo_open = nss_tlsmgr_tun_open,
	.ndo_stop = nss_tlsmgr_tun_close,
	.ndo_get_stats64 = nss_tlsmgr_tun_stats64,
	.ndo_change_mtu = nss_tlsmgr_tun_change_mtu,
};

/*
 * nss_tlsmgr_tun_setup()
 *	setup the TLS tunnel
 */
static void nss_tlsmgr_tun_setup(struct net_device *dev)
{
	dev->addr_len = ETH_ALEN;
	dev->mtu = ETH_DATA_LEN;
	dev->hard_header_len = 0;

	dev->type = ARPHRD_TUNNEL;
	dev->ethtool_ops = NULL;
	dev->header_ops = NULL;

	dev->netdev_ops = &nss_tlsmgr_tun_ops;

	/*
	 * Get the MAC address from the ethernet device
	 */
	eth_random_addr((u8 *) dev->dev_addr);

	memset(dev->broadcast, 0xff, dev->addr_len);
	memcpy(dev->perm_addr, dev->dev_addr, dev->addr_len);
}

/*
 * nss_tlsmgr_tun_delayed_free()
 *	Delayed worker function to free TLS session.
 */
static void nss_tlsmgr_tun_free_work(struct work_struct *work)
{
	struct nss_tlsmgr_tun *tun = container_of(work, struct nss_tlsmgr_tun, free_work);
	struct nss_tlsmgr_crypto *cur, *tmp;
	struct list_head tmp_head;

	write_lock_bh(&tun->lock);
	INIT_LIST_HEAD(&tmp_head);
	list_splice_tail_init(&tun->free_list, &tmp_head);
	write_unlock_bh(&tun->lock);

	list_for_each_entry_safe(cur, tmp, &tmp_head, list) {
		list_del(&cur->list);
		nss_tlsmgr_crypto_free(cur);
	}

	read_lock_bh(&tun->lock);
	if (!list_empty(&tun->free_list))
		schedule_work(&tun->free_work);
	read_unlock_bh(&tun->lock);
}


/*
 * nss_tlsmgr_notify_event()
 *	TLS manager notification timer handler
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0))
static void nss_tlsmgr_notify_event(unsigned long data)
#else
static void nss_tlsmgr_notify_event(struct timer_list *tm)
#endif
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0))
	struct nss_tlsmgr_tun *tun = (struct nss_tlsmgr_tun *)data;
#else
	struct nss_tlsmgr_tun *tun = from_timer(tun, tm, notify.timer);
#endif
	nss_tlsmgr_notify_callback_t cb;
	struct nss_tlsmgr_stats stats;
	void *app_data;

	read_lock_bh(&tun->lock);
	stats.encap = tun->ctx_enc.host_stats;
	stats.decap = tun->ctx_dec.host_stats;
	cb = tun->notify.cb;
	app_data = tun->notify.app_data;
	read_unlock_bh(&tun->lock);

	if (cb) {
		cb(app_data, tun->dev, &stats);
		mod_timer(&tun->notify.timer, jiffies + tun->notify.ticks);
	}
}

/*
 * nss_tlsmgr_notify_decongestion()
 *	TLS manager decongestion notification
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0))
static void nss_tlsmgr_notify_decongestion(unsigned long data)
#else
static void nss_tlsmgr_notify_decongestion(struct timer_list *tm)
#endif
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0))
	struct nss_tlsmgr_tun *tun = (struct nss_tlsmgr_tun *)data;
#else
	struct nss_tlsmgr_tun *tun = from_timer(tun, tm, notify.timer);
#endif
	nss_tlsmgr_decongest_callback_t cb;
	void *app_data;

	if (atomic_read(&tun->pkt_pending)) {
		mod_timer(&tun->decongest.timer, jiffies + tun->decongest.ticks);
		return;
	}

	BUG_ON(tun->dev->flags & IFF_RUNNING);

	read_lock_bh(&tun->lock);
	cb = tun->decongest.cb;
	app_data = tun->decongest.app_data;
	read_unlock_bh(&tun->lock);

	/*
	 * Mark device as RUNNING again before notifying the application
	 * to start transmitting again
	 */
	tun->dev->flags |= IFF_RUNNING;

	if (cb) {
		cb(app_data, tun->dev);
	}
}

/*
 * nss_tlsmgr_register_notify()
 *	TLS Manager notification register
 */
bool nss_tlsmgr_register_notify(struct net_device *dev, nss_tlsmgr_notify_callback_t cb,
					void *app_data, uint32_t msecs)
{
	struct nss_tlsmgr_tun *tun = netdev_priv(dev);

	if (!cb || !msecs) {
		nss_tlsmgr_warn("%px: NULL notification parameters %px %d\n", tun, cb, msecs);
		return false;
	};

	write_lock_bh(&tun->lock);
	tun->notify.ticks = msecs_to_jiffies(msecs);
	tun->notify.app_data = app_data;
	tun->notify.cb = cb;
	write_unlock_bh(&tun->lock);

	mod_timer(&tun->notify.timer, jiffies + tun->notify.ticks);
	return true;
}
EXPORT_SYMBOL(nss_tlsmgr_register_notify);

/*
 * nss_tlsmgr_unregister_notify()
 *	TLS Manager notification unregister
 */
void nss_tlsmgr_unregister_notify(struct net_device *dev)
{
	struct nss_tlsmgr_tun *tun = netdev_priv(dev);

	write_lock_bh(&tun->lock);
	tun->notify.app_data = NULL;
	tun->notify.cb = NULL;
	tun->notify.ticks = 0;
	write_unlock_bh(&tun->lock);

	BUG_ON(in_atomic());
	del_timer_sync(&tun->notify.timer);
}

/*
 * file operation structure instance
 */
const struct file_operations tlsmgr_ctx_file_ops = {
	.open = simple_open,
	.llseek = default_llseek,
	.read = nss_tlsmgr_ctx_read_stats,
};

/*
 * nss_tlsmgr_tun_add()
 *	Create TLS tunnel device and dynamic interface
 */
struct net_device *nss_tlsmgr_tun_add(nss_tlsmgr_decongest_callback_t cb, void *app_data)
{
	struct nss_tlsmgr_tun *tun;
	struct net_device *dev;
	int error;

	if (!atomic_read(&tlsmgr_drv->is_configured)) {
		nss_tlsmgr_warn("%px: tls firmware not ready", tlsmgr_drv);
		return NULL;
	}

	dev = alloc_netdev(sizeof(*tun), "tls%d", NET_NAME_ENUM, nss_tlsmgr_tun_setup);
	if (!dev) {
		nss_tlsmgr_warn("%px: unable to allocate tls device", tun);
		return NULL;
	}

	tun = netdev_priv(dev);
	tun->dev = dev;
	rwlock_init(&tun->lock);

	error = nss_tlsmgr_ctx_config_inner(&tun->ctx_enc, dev);
	if (error < 0) {
		nss_tlsmgr_warn("%px: unable to create encap context, error(%d)", tun, error);
		goto free_dev;
	}

	error = nss_tlsmgr_ctx_config_outer(&tun->ctx_dec, dev);
	if (error < 0) {
		nss_tlsmgr_warn("%px: unable to create decap context, error(%d)", tun, error);
		goto deconfig_inner;
	}

	error = rtnl_is_locked() ? register_netdevice(dev) : register_netdev(dev);
	if (error < 0) {
		nss_tlsmgr_warn("%px: unable register net_device(%s)", tun, dev->name);
		goto deconfig_outer;
	}

	nss_tlsmgr_trace("%px: tls tunnel(%s) created, encap(%u), decap(%u)",
			  tun, dev->name, tun->ctx_enc.ifnum, tun->ctx_dec.ifnum);

	/*
	 * Initialize tunnel decongestion
	 */
	tun->decongest.ticks = msecs_to_jiffies(NSS_TLSMGR_TUN_DECONGEST_TICKS);
	tun->decongest.app_data = app_data;
	tun->decongest.cb = cb;

	/*
	 * Initialize Event notification and Decongestion timer
	 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0))
	init_timer(&tun->notify.timer);
	tun->notify.timer.function = nss_tlsmgr_notify_event;
	tun->notify.timer.data = (unsigned long)tun;

	init_timer(&tun->decongest.timer);
	tun->decongest.timer.function = nss_tlsmgr_notify_decongestion;
	tun->decongest.timer.data = (unsigned long)tun;
#else
	timer_setup(&tun->notify.timer, nss_tlsmgr_notify_event, 0);
	timer_setup(&tun->decongest.timer, nss_tlsmgr_notify_decongestion, 0);
#endif

	INIT_LIST_HEAD(&tun->free_list);
	INIT_WORK(&tun->free_work, nss_tlsmgr_tun_free_work);

	/*
	 * Create debugfs entry for tunnel and its child context(s)
	 */
	tun->dentry = debugfs_create_dir(dev->name, tlsmgr_drv->dentry);
	if (tun->dentry) {
		debugfs_create_file("inner", S_IRUGO, tun->dentry, &tun->ctx_enc, &tlsmgr_ctx_file_ops);
		debugfs_create_file("outer", S_IRUGO, tun->dentry, &tun->ctx_dec, &tlsmgr_ctx_file_ops);
	}

	/*
	 * Mark device as UP and Running
	 */
	dev->flags = IFF_UP | IFF_RUNNING;

	return dev;

deconfig_outer:
	nss_tlsmgr_ctx_deconfig(&tun->ctx_dec);

deconfig_inner:
	nss_tlsmgr_ctx_deconfig(&tun->ctx_enc);

free_dev:
	free_netdev(dev);
	return NULL;
}
EXPORT_SYMBOL(nss_tlsmgr_tun_add);

/*
 * nss_tlsmgr_tun_del()
 *	Delete TLS tunnel
 */
void nss_tlsmgr_tun_del(struct net_device *dev)
{
	struct nss_tlsmgr_tun *tun = netdev_priv(dev);

	nss_tlsmgr_trace("%px: destroying encap(%u) and decap(%u) context",
			  tun, tun->ctx_enc.ifnum, tun->ctx_dec.ifnum);

	debugfs_remove_recursive(tun->dentry);

	nss_tlsmgr_unregister_notify(dev);

	del_timer_sync(&tun->decongest.timer);
	tun->decongest.app_data = NULL;
	tun->decongest.cb = NULL;
	tun->decongest.ticks = 0;

	nss_tlsmgr_ctx_deconfig(&tun->ctx_enc);
	nss_tlsmgr_ctx_deconfig(&tun->ctx_dec);

	schedule_work(&tun->free_work);
	flush_work(&tun->free_work);

	rtnl_is_locked() ? unregister_netdevice(dev) : unregister_netdev(dev);
}
EXPORT_SYMBOL(nss_tlsmgr_tun_del);
