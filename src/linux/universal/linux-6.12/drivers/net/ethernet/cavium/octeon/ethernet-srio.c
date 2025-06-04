/*************************************************************************
 *
 * Author: Cavium Inc.
 *
 * Contact: support@cavium.com
 * This file is part of the OCTEON SDK
 *
 * Copyright (c) 2010 - 2012 Cavium, Inc.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, Version 2, as
 * published by the Free Software Foundation.
 *
 * This file is distributed in the hope that it will be useful, but
 * AS-IS and WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, TITLE, or
 * NONINFRINGEMENT.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this file; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 * or visit http://www.gnu.org/licenses/.
 *
 * This file may also be available under a different license from Cavium.
 * Contact Cavium, Inc. for more information
 *************************************************************************/
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <net/dst.h>
#include <net/sock.h>
#include <linux/rio.h>
#include <linux/rio_drv.h>
#include <linux/rio_ids.h>
#include <linux/if_vlan.h>

#include <asm/octeon/octeon.h>
#include <asm/octeon/cvmx-srio.h>
#include <asm/octeon/cvmx-pip-defs.h>
#include <asm/octeon/cvmx-sriox-defs.h>
#include <asm/octeon/cvmx-sriomaintx-defs.h>

#include "ethernet-defines.h"
#include "octeon-ethernet.h"

/* SRIO packets' length must be a multiple of 8 */
#define SRIO_PAD		8

/* SRIO header length */
#define SRIO_HDR_LEN		8

struct net_device_stats *cvm_oct_srio_get_stats(struct net_device *dev)
{
	return &dev->stats;
}

int cvm_oct_srio_set_mac_address(struct net_device *dev, void *addr)
{
	/* FIXME: Should this be allowed? Should it change our device ID? */
	eth_hw_addr_set(dev, addr + 2);
	return 0;
}

int cvm_oct_srio_change_mtu(struct net_device *dev, int new_mtu)
{
	union cvmx_pip_frm_len_chkx pip_frm_len_chkx;
#if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)
	int vlan_bytes = VLAN_HLEN;
#else
	int vlan_bytes = 0;
#endif
	unsigned int max_mtu;

	/*
	 * Limit the MTU to make sure the ethernet packets are between
	 * 68 bytes and 4096. The MTU does not include the following:
	 *	SRIO header
	 *	Ethernet header
	 *	VLAN tags (optional)
	 *	SRIO padding
	 *	FCS
	 *
	 * Note: we could use "SRIO_PAD - 1" in this calculation, but that would
	 * make the MTU and odd number, which doesn't look right to me.
	 */
	max_mtu = RIO_MAX_MSG_SIZE - SRIO_HDR_LEN - ETH_HLEN - vlan_bytes - 
		SRIO_PAD - ETH_FCS_LEN;
	if ((new_mtu < 68) || (new_mtu > max_mtu)) {
		netdev_warn(dev, "MTU must be between %d and %d.\n",
			    68, max_mtu);
		return -EINVAL;
	}
	dev->mtu = new_mtu;

	/* set up pip. other interfaces prefer to disable the pip check. */
	pip_frm_len_chkx.u64 = cvmx_read_csr(CVMX_PIP_FRM_LEN_CHKX(0));
	pip_frm_len_chkx.s.maxlen = (new_mtu + 256) & ~0xff;
	cvmx_write_csr(CVMX_PIP_FRM_LEN_CHKX(0), pip_frm_len_chkx.u64);

	return 0;
}

int cvm_oct_xmit_srio(struct sk_buff *skb, struct net_device *dev)
{
	struct octeon_ethernet *priv = netdev_priv(dev);
	union cvmx_srio_tx_message_header tx_header;
	u64 dest_mac;

	if (unlikely(skb->len > 4096)) {
		dev_kfree_skb(skb);
		netdev_dbg(dev, "TX packet larger than 4096 bytes. Dropped.\n");
		return 0;
	}

	/*
	 * Since we are emulating ethernet over srio, small packets must be
	 * padded or the receiver will discard them for being too short.
	 */
	if (unlikely(skb->len < 64)) {
		if (likely(skb_tailroom(skb) >= 64 - skb->len)) {
			skb_put(skb, 64 - skb->len);
		}
	}

	/* srio message length needs to be a multiple of 8 */
	if (skb->len & (SRIO_PAD - 1)) {
		if (likely(skb_tailroom(skb) > SRIO_PAD - 1))
			skb_put(skb, SRIO_PAD - (skb->len & (SRIO_PAD - 1)));
	}

	tx_header.u64 = priv->srio_tx_header;
	/* Use the socket priority if it is available */
	if (skb->sk) {
		if (skb->sk->sk_priority < 0)
			tx_header.s.prio = 0;
		else if (skb->sk->sk_priority > 3)
			tx_header.s.prio = 3;
		else
			tx_header.s.prio = skb->sk->sk_priority;
	}

	/* Extract the destination MAC address from the packet */
	dest_mac = cpu_to_be64(*(u64 *)skb->data >> 16);

	/* If this is a broadcast/multicast we must manually send to everyone */
	if (dest_mac>>40) {
		struct list_head *pos;
		struct sk_buff *new_skb;

		list_for_each(pos, &priv->srio_bcast) {
			struct octeon_ethernet_srio_bcast_target *t;

			t = container_of(pos, struct octeon_ethernet_srio_bcast_target, list);
			/* Create a new SKB since each packet will have different data */
			if (skb_headroom(skb) < 8) {
				new_skb = skb_realloc_headroom(skb,
					8 - skb_headroom(skb));
			}
			else
				new_skb = skb_copy(skb, GFP_ATOMIC);

			if (new_skb) {
				tx_header.s.did = t->destid;
				*(u64 *)__skb_push(new_skb, 8) = cpu_to_be64(tx_header.u64);
				cvm_oct_xmit(new_skb, dev);
			} else {
				netdev_dbg(dev, "SKB allocation failed\n");
				break;
			}
		}

		dev->stats.tx_packets++;
		dev->stats.tx_bytes += skb->len;
		dev_kfree_skb(skb);
		return NETDEV_TX_OK;
	} else {
		/* Use the low two bytes of the destination MAC as the SRIO
		 * destination */
		/* tx_header.s.did = *(u16 *)(skb->data + 4); */
		tx_header.s.did = *(u8 *)(skb->data + 5);
		if (unlikely(skb_headroom(skb) < 8)) {
			int	nhead = 8 - skb_headroom(skb);

			if (pskb_expand_head(skb, nhead, 0, GFP_ATOMIC)) {
				netdev_dbg(dev,
					   "SKB didn't have room for SRIO "
					   "header and allocation failed\n");
				return NETDEV_TX_OK;
			}
		}

		dev->stats.tx_packets++;
		dev->stats.tx_bytes += skb->len;
		*(u64 *)__skb_push(skb, 8) = cpu_to_be64(tx_header.u64);
		return cvm_oct_xmit(skb, dev);
	}
}

int cvm_oct_srio_init(struct net_device *dev)
{
	struct octeon_ethernet *priv = netdev_priv(dev);
	int srio_port = (priv->ipd_port - 40) >> 1;
	union cvmx_sriox_status_reg srio_status_reg;

	dev->features |= NETIF_F_LLTX; /* We do our own locking, Linux doesn't need to */

	dev->ethtool_ops = &cvm_oct_ethtool_ops;

	/* Make sure register access is allowed */
	srio_status_reg.u64 = cvmx_read_csr(CVMX_SRIOX_STATUS_REG(srio_port));
	if (!srio_status_reg.s.access)
		return 0;

	netif_carrier_on(dev);

	dev->netdev_ops->ndo_change_mtu(dev, dev->mtu);

	return 0;
}

int cvm_oct_srio_open(struct net_device *dev)
{
	struct octeon_ethernet *priv = netdev_priv(dev);
	int srio_port = (priv->ipd_port - 40) >> 1;
	struct rio_dev *rdev;
	struct sockaddr sa;
	u32 devid;

	cvmx_srio_config_read32(srio_port, 0, -1, 1, 0, CVMX_SRIOMAINTX_PRI_DEV_ID(srio_port), &devid);

	sa.sa_data[0] = 0;
	sa.sa_data[1] = 0;
	sa.sa_data[2] = 0;
	sa.sa_data[3] = 0;
	if (devid >> 16) {
		sa.sa_data[4] = 0;
		sa.sa_data[5] = (devid >> 16) & 0xff;
	} else {
		sa.sa_data[4] = (devid >> 8) & 0xff;
		sa.sa_data[5] = devid & 0xff;
	}

	dev->netdev_ops->ndo_set_mac_address(dev, &sa);

	rdev = NULL;
	for (;;) {
		struct octeon_ethernet_srio_bcast_target *target;
		rdev = rio_get_device(RIO_ANY_ID, RIO_ANY_ID, rdev);
		if (!rdev)
			break;
		/* Skip devices not on my rio port */
		if (rdev->net->hport->id != srio_port)
			continue;
		/* Skip switches */
		if (rdev->destid == 0xffff)
			continue;
		target = kmalloc(sizeof(*target), GFP_KERNEL);
		if (!target) {
			WARN(1, "No memory");
			return -ENOMEM;
		}
		target->destid = rdev->destid;
		list_add(&target->list, &priv->srio_bcast);
	}
	return 0;
}

int cvm_oct_srio_stop(struct net_device *dev)
{
	struct octeon_ethernet *priv = netdev_priv(dev);
	struct list_head *pos;
	struct list_head *n;

	list_for_each_safe(pos, n, &priv->srio_bcast) {
		struct octeon_ethernet_srio_bcast_target *t;
		list_del(pos);
		t = container_of(pos, struct octeon_ethernet_srio_bcast_target,
				 list);
		kfree(t);
	}
	return 0;
}
