/*
 **************************************************************************
 * Copyright (c) 2016, 2020, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

/*
 * nss_connmgr_dtls_netdev.c
 *	NSS DTLS Manager netdev module
 */

#include <linux/version.h>
#include <linux/types.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <net/ipv6.h>
#include <linux/if_arp.h>
#include <linux/etherdevice.h>
#include <linux/udp.h>
#include <linux/ipv6.h>

#include <nss_api_if.h>
#include <nss_dynamic_interface.h>

#include "nss_connmgr_dtls.h"

/*
 * Maximum tailroom required by crypto
 */
#define NSS_DTLSMGR_TROOM (128 + (2 * NSS_CRYPTO_MAX_HASHLEN_SHA256))

/*
 * Maximum headroom for encapsulating headers
 */
#define NSS_DTLSMGR_MAX_HDR_LEN ((NSS_DTLSMGR_HDR_LEN + 3)	\
				 + NSS_DTLSMGR_CAPWAPHDR_LEN	\
				 + (2 * NSS_CRYPTO_MAX_IVLEN_AES)	\
				 + sizeof(struct ipv6hdr)	\
				 + sizeof(struct udphdr))

/*
 * nss_dtlsmgr_session_xmit()
 */
static netdev_tx_t nss_dtlsmgr_session_xmit(struct sk_buff *skb,
					    struct net_device *dev)
{
	struct nss_dtlsmgr_netdev_priv *priv;
	struct nss_dtlsmgr_session *s;
	int32_t  nhead, ntail;

	priv = netdev_priv(dev);
	s = priv->s;

	switch (skb->protocol) {
	case htons(ETH_P_IP):
		if (s->flags & NSS_DTLSMGR_HDR_IPV6) {
			nss_dtlsmgr_info("%px: NSS DTLS I/F %d: skb(%px) invalid L3 protocol 0x%x\n", dev, s->nss_dtls_if, skb, ETH_P_IP);
			return NETDEV_TX_BUSY;
		}
		break;

	case htons(ETH_P_IPV6):
		if (!(s->flags & NSS_DTLSMGR_HDR_IPV6)) {
			nss_dtlsmgr_info("%px: NSS DTLS I/F %d: skb(%px) invalid L3 protocol 0x%x\n", dev, s->nss_dtls_if, skb, ETH_P_IPV6);
			return NETDEV_TX_BUSY;
		}
		break;

	default:
		nss_dtlsmgr_info("%px: NSS DTLS I/F %d: skb(%px) unsupported IP protocol 0x%x\n", dev, s->nss_dtls_if, skb, ntohs(skb->protocol));
		return NETDEV_TX_BUSY;
	}

	nhead = dev->needed_headroom;
	ntail = dev->needed_tailroom;

	if (skb_is_nonlinear(skb)) {
		nss_dtlsmgr_info("%px: NSS DTLS does not support non-linear skb %px\n", dev, skb);
		return NETDEV_TX_BUSY;
	}

	if (unlikely(skb_shared(skb))) {
		nss_dtlsmgr_info("%px: Shared skb:%px is not supported\n",
				 dev, skb);
		return NETDEV_TX_BUSY;
	}

	if (skb_cloned(skb) || (skb_headroom(skb) < nhead)
	    || (skb_tailroom(skb) < ntail)) {
		if (pskb_expand_head(skb, nhead, ntail, GFP_KERNEL)) {
			nss_dtlsmgr_info("%px: skb:%px unable to expand buffer\n",
					 dev, skb);
			return NETDEV_TX_BUSY;
		}
	}

	if (skb->data != skb_network_header(skb)) {
		skb_pull(skb, skb_network_offset(skb));
	}

	if (nss_dtls_tx_buf(skb, s->nss_dtls_if, s->nss_ctx) != NSS_TX_SUCCESS) {
		return NETDEV_TX_BUSY;
	}

	return NETDEV_TX_OK;
}

/*
 * nss_dtlsmgr_session_stop()
 */
static int nss_dtlsmgr_session_stop(struct net_device *dev)
{
	netif_stop_queue(dev);
	return 0;
}

/*
 * nss_dtlsmgr_session_open()
 */
static int nss_dtlsmgr_session_open(struct net_device *dev)
{
	netif_start_queue(dev);
	return 0;
}

/*
 * DTLS netdev ops
 */
static const struct net_device_ops nss_dtlsmgr_session_ops = {
	.ndo_start_xmit = nss_dtlsmgr_session_xmit,
	.ndo_open = nss_dtlsmgr_session_open,
	.ndo_stop = nss_dtlsmgr_session_stop,
	.ndo_set_mac_address = eth_mac_addr,
};

/*
 * nss_dtlsmgr_dev_setup()
 */
static void nss_dtlsmgr_dev_setup(struct net_device *dev)
{
	dev->addr_len = ETH_ALEN;
	dev->mtu = ETH_DATA_LEN;
	dev->hard_header_len = NSS_DTLSMGR_MAX_HDR_LEN;
	dev->needed_headroom = 0;
	dev->needed_tailroom = NSS_DTLSMGR_TROOM;

	dev->type = ARPHRD_ETHER;
	dev->ethtool_ops = NULL;
	dev->header_ops = NULL;
	dev->netdev_ops = &nss_dtlsmgr_session_ops;
	dev->priv_destructor = NULL;

	memcpy(dev->dev_addr, "\xaa\xbb\xcc\xdd\xee\xff", dev->addr_len);
	memset(dev->broadcast, 0xff, dev->addr_len);
	memcpy(dev->perm_addr, dev->dev_addr, dev->addr_len);
}

/*
 * nss_dtlsmgr_netdev_create()
 */
nss_dtlsmgr_status_t nss_dtlsmgr_netdev_create(struct nss_dtlsmgr_session *ds)
{
	struct net_device *dev;
	struct nss_dtlsmgr_netdev_priv *priv;
	int32_t err = 0;

#if (LINUX_VERSION_CODE <= KERNEL_VERSION(3, 16, 0))
	dev = alloc_netdev(sizeof(struct nss_dtlsmgr_netdev_priv),
			   "qca-nss-dtls%d", nss_dtlsmgr_dev_setup);
#else
	dev = alloc_netdev(sizeof(struct nss_dtlsmgr_netdev_priv),
			   "qca-nss-dtls%d", NET_NAME_UNKNOWN,
			   nss_dtlsmgr_dev_setup);
#endif

	if (!dev) {
		nss_dtlsmgr_info("DTLS netdev alloc failed\n");
		return NSS_DTLSMGR_FAIL;
	}

	priv = netdev_priv(dev);
	priv->s = ds;

	err = rtnl_is_locked() ? register_netdevice(dev) : register_netdev(dev);
	if (err < 0) {
		nss_dtlsmgr_info("DTLS netdev register failed\n");
		free_netdev(dev);
		return NSS_DTLSMGR_FAIL;
	}

	ds->netdev = dev;
	return NSS_DTLSMGR_OK;
}

/*
 * nss_dtlsmgr_netdev_destroy()
 */
nss_dtlsmgr_status_t nss_dtlsmgr_netdev_destroy(struct nss_dtlsmgr_session *ds)
{
	if (!ds || !ds->netdev) {
		return NSS_DTLSMGR_FAIL;
	}

	rtnl_is_locked() ? unregister_netdevice(ds->netdev)
			 : unregister_netdev(ds->netdev);

	free_netdev(ds->netdev);
	ds->netdev = NULL;
	return NSS_DTLSMGR_OK;
}
