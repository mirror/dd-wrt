/*
 *  QCA HyFi Bridge
 *
 * Copyright (c) 2012, 2015-2016, The Linux Foundation. All rights reserved.
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
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#define DEBUG_LEVEL HYFI_NF_DEBUG_LEVEL

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/etherdevice.h>
#include "mc_private2.h"
#include "mc_snooping2.h"
#include "hyfi_netfilter.h"
#include "hyfi_filters.h"
#include "hyfi_hatbl.h"
#include "hyfi_hdtbl.h"
#include "hyfi_api.h"
#include "hyfi_hash.h"
#include "hyfi_seamless.h"
#include "hyfi_aggr.h"
#include "queue.h"
#include <linux/version.h>

/* Default Linux bridge */
static char hyfi_linux_bridge[IFNAMSIZ] = "";

/* This parameter can be set from the insmod command line */
MODULE_PARM_DESC(hyfi_linux_bridge, "Default Hy-Fi managed bridge");

static struct hyfi_net_bridge hyfi_bridges[HYFI_BRIDGE_MAX] __read_mostly;

static int hyfi_bridge_ports_init(struct hyfi_net_bridge *hyfi_br, struct net_device *br_dev);
static int hyfi_bridge_init_bridge_device(struct hyfi_net_bridge *hyfi_br, const char *br_name);
static int hyfi_bridge_deinit_bridge_device(struct hyfi_net_bridge *hyfi_br);
static int hyfi_bridge_del_ports(struct hyfi_net_bridge *hyfi_br);
static void hyfi_destroy_port_rcu(struct rcu_head *head);

/**
 * @brief Synchronize with RCU and release the bridge device
 *
 * Note this function must call synchronize_rcu before putting
 * the device.  Other users rely on hyfi_bridge holding the
 * device to ensure it is valid while they are using it.  These
 * users get a reference to br_dev under rcu_read_lock, so this
 * function must ensure they have all exited the rcu_read_lock
 * before releasing the device.
 *
 * @pre Must not hold any locks
 * @pre dev_hold has been previously called on br_dev
 *
 * @param [in] br_dev  device to release hold on
 */
static void hyfi_sync_and_free_bridge_device(struct net_device *br_dev) {
	synchronize_rcu();
	dev_put(br_dev);
}

struct hyfi_net_bridge * hyfi_bridge_get_hyfi_bridge(const char *br_name)
{
	int i;
	struct net_device *br_dev;

	br_dev = dev_get_by_name(&init_net, br_name);
	if (br_dev == NULL)
	  return NULL;

	for (i = 0; i < HYFI_BRIDGE_MAX; i++) {
		if (hyfi_bridges[i].dev == br_dev) {
			dev_put(br_dev);
			return &hyfi_bridges[i];
		}
	}

	dev_put(br_dev);
	return NULL;
}

struct hyfi_net_bridge * hyfi_bridge_alloc_hyfi_bridge(const char *br_name)
{
	int i;
	struct hyfi_net_bridge *hf_br;

	hf_br = hyfi_bridge_get_hyfi_bridge(br_name);
	if (hf_br)
		return hf_br;

	for (i = 0; i < HYFI_BRIDGE_MAX; i++) {
		if (hyfi_bridges[i].dev == NULL)
			return &hyfi_bridges[i];
	}
	return NULL;
}

int hyfi_bridge_set_bridge_name(struct hyfi_net_bridge *hyfi_br, const char *br_name)
{
	int retval = 0;
	struct net_device *br_dev;

	spin_lock_bh(&hyfi_br->lock);

	br_dev = hyfi_br->dev;

	if (!br_name) {
		if (br_dev) {
			/* Detach from existing bridge */
			hyfi_bridge_deinit_bridge_device(hyfi_br);
		}

		spin_unlock_bh(&hyfi_br->lock);
		if (br_dev) {
			hyfi_sync_and_free_bridge_device(br_dev);
		}
		return 0;
	}

	if (hyfi_br->dev && !strcmp(hyfi_br->dev->name, br_name)) {
		/* Bridge already attached */
		spin_unlock_bh(&hyfi_br->lock);
		return 0;
	}

	if (hyfi_br->dev) {
		/* Detach from existing bridge */
		hyfi_bridge_deinit_bridge_device(hyfi_br);
	}

	/* Update new bridge */
	retval = hyfi_bridge_init_bridge_device(hyfi_br, br_name);

	spin_unlock_bh(&hyfi_br->lock);

	if (br_dev)
		hyfi_sync_and_free_bridge_device(br_dev);
	return retval;
}

int hyfi_bridge_dev_event(struct hyfi_net_bridge *hyfi_br,
                          unsigned long event, struct net_device *dev)
{
	int sync_and_free = 0;

	spin_lock_bh(&hyfi_br->lock);

	if (hyfi_br->dev && dev != hyfi_br->dev) {
		spin_unlock_bh(&hyfi_br->lock);
		return -1;
	}

	switch (event) {
	case NETDEV_DOWN:
		if (!hyfi_br->dev)
			break;

		DEBUG_TRACE("Interface %s is down, ptr = %p\n", dev->name, dev);

		/* Free the hold of the device */
		hyfi_bridge_deinit_bridge_device(hyfi_br);
		sync_and_free = 1;
		break;

	case NETDEV_UP:
		if (dev == hyfi_br->dev)
			break;

		if (!strcmp(dev->name, hyfi_linux_bridge)) {
			DEBUG_TRACE("Interface %s is up, ptr = %p\n", dev->name, dev);

			if (dev->priv_flags != IFF_EBRIDGE) {
				DEBUG_ERROR("hyfi-bridging: Device %s is NOT a bridge!\n", dev->name);
			} else {
				if (hyfi_bridge_init_bridge_device(hyfi_br, hyfi_linux_bridge)) {
					DEBUG_ERROR("hyfi-bridging: Failed to initialize device %s\n", dev->name);
					hyfi_br->dev = NULL;
					sync_and_free = 1;
				}
			}
		}
		break;

	case NETDEV_CHANGE:
	    break;

	case NETDEV_UNREGISTER:
		if (!hyfi_br->dev)
			break;
		if (!strcmp(dev->name, hyfi_br->linux_bridge)) {
			if (dev->priv_flags & IFF_EBRIDGE) {
				hyfi_bridge_deinit_bridge_device(hyfi_br);
				sync_and_free = 1;
			}
		}
		break;

	default:
		break;
	}

	spin_unlock_bh(&hyfi_br->lock);

	if (sync_and_free)
		hyfi_sync_and_free_bridge_device(dev);

	return 0;
}

static int hyfi_bridge_del_ports(struct hyfi_net_bridge *hyfi_br)
{
	struct hyfi_net_bridge_port *hyfi_p;

	list_for_each_entry_rcu(hyfi_p, &hyfi_br->port_list, list) {
		list_del_rcu(&hyfi_p->list);
		call_rcu(&hyfi_p->rcu, hyfi_destroy_port_rcu);
	}

	return 0;
}

int hyfi_bridge_init_port(struct hyfi_net_bridge *hyfi_br, struct net_bridge_port *p)
{
	struct hyfi_net_bridge_port *hyfi_p;

	/* First, look up this port in our list */
	hyfi_p = hyfi_bridge_get_port(p);

	if (hyfi_p) {
		/* Update the device pointer */
		hyfi_p->dev = p->dev;
		return 0;
	}

	/* Not found - create a new Hy-Fi port entry */
	hyfi_p = kzalloc(sizeof(struct hyfi_net_bridge_port), GFP_ATOMIC);

	if (!hyfi_p) {
		DEBUG_ERROR("hyfi: Failed to allocate memory for port\n");
		return -1;
	}

	hyfi_p->bcast_enable = 0;
	hyfi_p->group_num = HYFI_PORTGRP_INVALID;
	hyfi_p->group_type = !HYFI_PORTGRP_TYPE_RELAY;
	hyfi_p->port_type = HYFI_PORT_INVALID_TYPE;
	hyfi_p->dev = p->dev;

	list_add_rcu(&hyfi_p->list, &hyfi_br->port_list);
	DEBUG_INFO("hyfi: Added interface %s\n", p->dev->name);

	return 0;
}

static void hyfi_destroy_port_rcu(struct rcu_head *head)
{
	struct hyfi_net_bridge_port *hyfi_p =
			container_of(head, struct hyfi_net_bridge_port, rcu);

	DEBUG_INFO("hyfi: Removed interface %s\n", hyfi_p->dev->name);
	kfree(hyfi_p);
}

int hyfi_bridge_get_WdsExt_iface_list(struct hyfi_net_bridge *hyfi_br, struct WdsExt_iflist *wdsExtlist)
{
	struct hyfi_net_bridge_port *hyfi_p;
	int i=0;

	list_for_each_entry_rcu(hyfi_p, &hyfi_br->port_list, list) {
		if( strstr(hyfi_p->dev->name, ".sta") != NULL ) {
			memcpy(wdsExtlist->iflist[i].ifname, hyfi_p->dev->name, IFNAMSIZ);
			i++;
			wdsExtlist->num_entries=i;
		}
	}

	return 0;
}

int hyfi_bridge_delete_port(struct hyfi_net_bridge *hyfi_br, struct net_bridge_port *p)
{
	struct hyfi_net_bridge_port *hyfi_p;

	if (unlikely(!p))
		return 0;

	list_for_each_entry_rcu(hyfi_p, &hyfi_br->port_list, list) {
		if (hyfi_p->dev == p->dev) {
			list_del_rcu(&hyfi_p->list);
			call_rcu(&hyfi_p->rcu, hyfi_destroy_port_rcu);
			return 0;
		}
	}

	return 0;
}

struct hyfi_net_bridge_port *hyfi_bridge_get_port(const struct net_bridge_port *p)
{
	if (unlikely(!p))
		return NULL;

	return hyfi_bridge_get_port_by_dev(p->dev);
}

struct hyfi_net_bridge_port *hyfi_bridge_get_port_by_dev(const struct net_device *dev)
{
	struct hyfi_net_bridge_port *hyfi_p;
	struct hyfi_net_bridge *hyfi_br;

	if (unlikely(!dev))
		return NULL;

	if ( strlen(dev->name) == 0 ) {
		return NULL;
	}

	if ( dev->priv_flags ==  0) {
		return NULL;
	}

	hyfi_br = hyfi_bridge_get_by_dev(dev);

	if (unlikely(!hyfi_br))
		return NULL;

	list_for_each_entry_rcu(hyfi_p, &hyfi_br->port_list, list) {
		if (hyfi_p && hyfi_p->dev == dev) {
			return hyfi_p;
		}
	}

	return NULL;
}

static int hyfi_bridge_ports_init(struct hyfi_net_bridge *hyfi_br, struct net_device *br_dev)
{
	struct net_device *dev;

	read_lock(&dev_base_lock);

	dev = first_net_device(&init_net);
	while (dev) {
		struct net_bridge_port *br_port = hyfi_br_port_get(dev);
		if (br_port && br_port->br) {
			if (br_port->br->dev == br_dev) {
				/* Add to bridge port extended member */
				hyfi_bridge_init_port(hyfi_br, br_port);
			}
		}

		dev = next_net_device(dev);
	}

	read_unlock(&dev_base_lock);

	return 0;
}

struct hyfi_net_bridge *hyfi_bridge_get_first_br(void)
{
	return &hyfi_bridges[0];
}

struct hyfi_net_bridge *hyfi_bridge_get(const struct net_bridge *br)
{
	if (br == NULL)
		return NULL;

	return hyfi_bridge_get_by_dev(br->dev);
}

struct net_device *hyfi_bridge_dev_get_rcu(const struct hyfi_net_bridge *br)
{
	struct net_device *dev = rcu_dereference(br->dev);
	return dev;
}

struct hyfi_net_bridge *hyfi_bridge_get_by_dev(const struct net_device *dev)
{
	struct net_bridge_port *br_port;
	const struct net_device *br_dev;
	int i;

	if (!dev) {
	  return NULL;
	}

	if (dev->priv_flags & IFF_EBRIDGE) {
		br_dev = dev;
	} else {
		br_port = hyfi_br_port_get(dev);

		if (!br_port) {
			return NULL;
		}

		br_dev = br_port->br->dev;
	}

	for (i = 0; i < HYFI_BRIDGE_MAX; i++) {
		if (hyfi_bridges[i].dev && (br_dev == hyfi_bridges[i].dev)) {
			return &hyfi_bridges[i];
		}
	}

	return NULL;
}

struct hyfi_net_bridge *hyfi_bridge_get_by_port(const struct net_bridge_port *port)
{
	return (port ? hyfi_bridge_get_by_dev(port->dev): NULL) ;
}

static inline struct net_bridge_port *hyfi_bridge_handle_ha(struct net_hatbl_entry *ha,
		struct sk_buff **skb)
{
	struct net_bridge_port *dst;
	struct hyfi_net_bridge * hyfi_br = ha->hyfi_br;

	if (likely(ha->dst->dev)) {
		if ( hyfi_ha_has_flag(ha,
				HYFI_HACTIVE_TBL_AGGR_TX_ENTRY)) {
			dst = hyfi_aggr_handle_tx_path(ha, skb);
		} else {
			dst = ha->dst;
		}
		ha->num_packets++;
		ha->num_bytes += (*skb)->len;
		hyfi_ha_clear_flag(ha, HYFI_HACTIVE_TBL_ACCL_ENTRY);

		return dst;
	}

	hyfi_hatbl_delete_by_port(hyfi_br, ha->dst);
	return NULL;
}

#ifndef DISABLE_APS_HOOKS
static inline struct net_bridge_port *hyfi_bridge_handle_hd(struct net_hdtbl_entry *hd,
		struct sk_buff **skb, u_int32_t hash, u_int32_t traffic_class, u_int32_t priority)
{
	struct net_hatbl_entry *ha = hyfi_hatbl_insert(hd->hyfi_br, hash,
			traffic_class, hd, priority,
			eth_hdr(*skb)->h_source);

	if (unlikely(!ha))
		return NULL;

	ha->num_packets++;
	ha->num_bytes += (*skb)->len;
	return ha->dst;
}

static struct net_bridge_port *hyfi_bridge_handle_aggr(struct net_hatbl_entry *ha,
		struct sk_buff **skb, u_int16_t seq)
{
	/* Untag the packet */
	hyfi_aggr_untag_packet(*skb);

	if (unlikely(seq == (u_int16_t)~0)) {
		if (!ha->aggr_rx_entry->aggr_new_flow) {
			hyfi_aggr_end(ha);
		}

		/* We already received tagged packets on this flow,
		 * but still receiving older packets from the other
		 * interface. Need to forward them.
		 */
		return ha->dst;
	}

	if (unlikely(!hyfi_ha_has_flag(ha, HYFI_HACTIVE_TBL_AGGR_RX_ENTRY))) {
		/* New entry:
		 * Always start an entry with seq = 0 because we could receive
		 * older packets later due to different interface latency.
		 */
		if(hyfi_aggr_init_entry(ha, seq) == 0) {
			hyfi_ha_clear_flag(ha, HYFI_HACTIVE_TBL_TRACKED_ENTRY);
		} else {
			return ha->dst;
		}
	}

	/* Process the packet and return destination.
	 */
	return hyfi_aggr_process_pkt(ha, skb, seq);
}

/**
 * @brief Lookup the destination port for a skb (will check
 *        H-Active, H-Default and FDB)
 *
 * @param [in] br  bridge
 * @param [in] hash  hash calculated from skb
 * @param [in] traffic_class  traffic class of the skb (UDP or
 *                            other)
 * @param [in] priority  priority of the skb
 * @param [in] skb  skb to determine destination port for
 * @param [in] dest_addr  MAC address of the destination
 * @param [in] src_addr  MAC address of the source
 * @param [out] ha_out  H-Active entry (if found).  Note is not
 *                      filled in if created in this function.
 *
 * @return destination port if found, NULL if not
 */
static struct net_bridge_port *hyfi_bridge_get_dst_port(
	struct hyfi_net_bridge * hyfi_br,
	const struct net_bridge *br,
	u_int32_t hash, u_int32_t traffic_class,
	u_int32_t priority, struct sk_buff *skb,
	const unsigned char *dest_addr, const unsigned char *src_addr,
	struct net_hatbl_entry **ha_out)
{
	struct net_hatbl_entry *ha = NULL;
	struct net_hdtbl_entry *hd;
	struct net_bridge_fdb_entry *dst;
	bool ret;

	/* First, look up in the H-Active table. If not exists, look up in
	 * the H-Default table. Finally, if not in there, look up in the FDB. */
	ha = __hyfi_hatbl_get(hyfi_br, hash, dest_addr,
			traffic_class, priority);

	if (ha) {
		u_int32_t ha_flag = hyfi_ha_has_flag(ha, HYFI_HDTBL_STATIC_ENTRY);
		if (ha_flag) {
			hd = __hyfi_hdtbl_get(hyfi_br, dest_addr);
			if (hd) {
				u_int32_t hd_flag = hyfi_hd_has_flag(hd, HYFI_HDTBL_STATIC_ENTRY);
				if (hd_flag != ha_flag) {
					if (hd_flag == 1) {
						hyfi_ha_set_flag(ha, HYFI_HDTBL_STATIC_ENTRY);
					} else {
						hyfi_ha_clear_flag(ha, HYFI_HDTBL_STATIC_ENTRY);
					}
				}
			}
		}

		if (ha_out) {
			*ha_out = ha;
		}
		return ha->dst;
	} else if ((hd = __hyfi_hdtbl_get(hyfi_br, dest_addr))) {
		/* Create a new entry based on H-Default table */
		return hyfi_bridge_handle_hd(hd, &skb, hash, traffic_class, priority);
	} else if ((dst = os_br_fdb_get((struct net_bridge *)br, dest_addr))) {
		if(!dst) {
			return NULL;
		}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6,1,0))
		ret = test_bit(BR_FDB_LOCAL, &dst->flags);
#else
		ret = dst->is_local;
#endif
		if (!ret){
		hyfi_hatbl_insert_from_fdb(hyfi_br, hash, dst->dst, src_addr,
			dest_addr, br->dev->dev_addr,
			traffic_class, priority, false /* keep_lock */);

		return dst->dst;
		}
	}
	return NULL;
}

/**
 * @brief Lookup the port to use to reach a destination MAC in
 *        H-Default and FDB only
 *
 * Will not create a H-Active entry
 *
 * @param [in] br  bridge
 * @param [in] traffic_class  traffic class (UDP or other)
 * @param [in] addr  MAC address of the destination
 *
 * @return destination port if found, NULL if not
 */
static struct net_bridge_port *hyfi_bridge_get_dst_port_no_hash(
	struct hyfi_net_bridge * hyfi_br,
	const struct net_bridge *br, u_int32_t traffic_class,
	const unsigned char *addr)
{
	struct net_hdtbl_entry *hd;
	struct net_bridge_fdb_entry *dst;
	bool is_local;

	hd = __hyfi_hdtbl_get(hyfi_br, addr);
	if (hd) {
		if (traffic_class == HYFI_TRAFFIC_CLASS_UDP) {
			DEBUG_TRACE("%02x:%02x:%02x:%02x:%02x:%02x: Match in "
				"H-Default (UDP), sending on port %s\n",
				addr[0], addr[1], addr[2],
				addr[3], addr[4], addr[5], hd->dst_udp->dev->name);
			return hd->dst_udp;
		} else {
			DEBUG_TRACE("%02x:%02x:%02x:%02x:%02x:%02x: Match in "
				"H-Default (Other), sending on port %s\n",
				addr[0], addr[1], addr[2],
				addr[3], addr[4], addr[5],
				hd->dst_other->dev->name);
			return hd->dst_other;
		}
	} else {
		dst = os_br_fdb_get((struct net_bridge *)br, addr);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6,1,0))
	if(dst)
	is_local = test_bit(BR_FDB_LOCAL, &dst->flags);
#else
	if(dst)
	is_local = dst->is_local;
#endif

		if (dst && !(is_local)) {
			DEBUG_TRACE("%02x:%02x:%02x:%02x:%02x:%02x: Match in "
				"FDB, sending on port %s\n",
				addr[0], addr[1], addr[2],
				addr[3], addr[4], addr[5], dst->dst->dev->name);
			return dst->dst;
		} else {
			DEBUG_TRACE("%02x:%02x:%02x:%02x:%02x:%02x: No match found\n",
				addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
			return NULL;
		}
	}
}
#endif

struct net_bridge_port *hyfi_bridge_get_dst(const struct net_bridge_port *src,
		struct sk_buff **skb)
{
	/* hybrid look up first */
	u_int32_t flag, priority;
	u_int32_t hash;
#ifndef DISABLE_APS_HOOKS
	u_int32_t traffic_class = 0;
	struct net_hatbl_entry *ha = NULL;
	struct net_hdtbl_entry *hd;
	struct net_bridge_port *port;
#endif
	u_int16_t seq = ~0;
	const struct net_bridge *br;
	struct hyfi_net_bridge *hyfi_br;
	const unsigned char *dest_addr, *src_addr;
	struct net_bridge_fdb_entry *dst, *hsrc;
	struct sk_buff *skb2;

	if (src) {
		/* Bridged interface */
		br = src->br;
	} else {
		/* Routed interface */
		br = netdev_priv(BR_INPUT_SKB_CB(*skb)->brdev);
	}


	hyfi_br = hyfi_bridge_get(br);

	if (unlikely(!br || !hyfi_br || !hyfi_br->dev || br->dev != hyfi_br->dev))
		return NULL;

	if (hyfi_br->isController) {
		if (hyfi_is_ieee1905_pkt(*skb)) {
			/* Need to check for 1905 and src interface to be self
			Modify the SKB to be received by the other hyd instance
			1. From dest mac derive FDB
			2. From FDB, check local or noa
			3. Retrieve the dev from FDB
			4. Modify skb dev with the retrieved interface dev
			5. Call netif_rx_skb with the modified skb
			*/
			src_addr = eth_hdr(*skb)->h_source;
			dest_addr = eth_hdr(*skb)->h_dest;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6,1,0))
			if ((dst = os_br_fdb_get((struct net_bridge *)br, dest_addr)) &&
				test_bit(BR_FDB_LOCAL, &dst->flags)) {
				if ((hsrc = os_br_fdb_get((struct net_bridge *)br, src_addr)) &&
					test_bit(BR_FDB_LOCAL, &hsrc->flags)) {
					hyfi_ieee1905_frame_filter(*skb, (*skb)->dev);
					skb2 = skb_clone(*skb, GFP_ATOMIC);
					if (skb2) {
						skb2->dev = hyfi_br->dev;
						netif_receive_skb(skb2);
					}
					return NULL;
				}
			}
#else
			if ((dst = os_br_fdb_get((struct net_bridge *)br, dest_addr)) &&
				dst->is_local) {
				if ((hsrc = os_br_fdb_get((struct net_bridge *)br, src_addr)) &&
					hsrc->is_local) {
					hyfi_ieee1905_frame_filter(*skb, (*skb)->dev);
					skb2 = skb_clone(*skb, GFP_ATOMIC);
					if (skb2) {
						skb2->dev = hyfi_br->dev;
						netif_receive_skb(skb2);
					}
					return NULL;
				}
			}
#endif
		}
	}

	/* If not operating in APS mode, no hybrid tables are consulted. */
	if (unlikely(!hyfi_bridge_is_fwmode_aps(hyfi_br))) {
		return NULL;
	}

	if (unlikely(hyfi_hash_skbuf(*skb, &hash, &flag, &priority, &seq))) {
		return NULL;
	}

#ifndef DISABLE_APS_HOOKS
	traffic_class = (flag & IS_IPPROTO_UDP) ?
			HYFI_TRAFFIC_CLASS_UDP : HYFI_TRAFFIC_CLASS_OTHER;
	/* If incoming packet is a TCP stream, make sure that the TCP-ACK
	 * stream will be transmitted back on the same medium (if hyfi_tcp_sp
	 * is enabled).
	 */
	if (src && (flag & IS_IPPROTO_TCP) && hyfi_tcp_sp(hyfi_br) &&
			!hyfi_portgrp_relay(hyfi_bridge_get_port(src)) &&
			(hd = __hyfi_hdtbl_get(hyfi_br, eth_hdr(*skb)->h_source))) {
				/* Calculate reverse Hash for the Data so that its corresponding
				 * TCP ACK will have this same hash value
				 */
				if (unlikely(hyfi_hash_skbuf_reverse(*skb, &hash))) {
					return NULL;
				}
				hyfi_hatbl_update_local(hyfi_br, hash,eth_hdr(*skb)->h_source,
						eth_hdr(*skb)->h_dest, hd, (struct net_bridge_port *)src,
						HYFI_TRAFFIC_CLASS_OTHER, priority, flag);
	}

	port = hyfi_bridge_get_dst_port(hyfi_br, br, hash, traffic_class,
		priority, *skb, eth_hdr(*skb)->h_dest, eth_hdr(*skb)->h_source, &ha);

 	if (ha) {
		/* Entry found, update stats, and return destination port */
		if (!hyfi_ha_has_flag(ha, HYFI_HACTIVE_TBL_RX_ENTRY)) {
			return hyfi_bridge_handle_ha(ha, skb);
		}

		if (flag & IS_HYFI_AGGR_FLOW) {
			return hyfi_bridge_handle_aggr(ha, skb, seq);
		}

		if(hyfi_ha_has_flag(ha, HYFI_HACTIVE_TBL_TRACKED_ENTRY)) {
			if(!hyfi_psw_process_pkt(ha, skb, hyfi_br))
				return ha->dst;

			return (struct net_bridge_port *) -1;
		}
	}

	if (port) {
		return port;
	}
#endif
	if(unlikely(flag & (IS_HYFI_PKT | IS_HYFI_IP_PKT))) {
		hyfi_psw_process_hyfi_pkt(hyfi_br, *skb, flag);

		if(hyfi_br->path_switch_param.drop_markers) {
			kfree_skb(*skb);
			*skb = NULL;
			return (struct net_bridge_port *) -1;
		}

		return NULL;
	}

	if(unlikely(flag & IS_HYFI_AGGR_FLOW)) {
		hyfi_aggr_untag_packet(*skb);

		if(seq == (u_int16_t)~0) {
			return NULL;
		}
#ifndef DISABLE_APS_HOOKS
		ha = hyfi_hatbl_create_aggr_entry(hyfi_br, hash, eth_hdr(*skb)->h_source,
				eth_hdr(*skb)->h_dest, traffic_class, priority, seq);

		if(!ha) {
			DEBUG_ERROR("hyfi: Cannot create an entry for aggregated flow\n");

			return NULL;
		}

		/* If sequence == 0, then it means we received the first transmitted
		 * packet from the other side in the correct order.
		 */
		return hyfi_aggr_process_pkt(ha, skb, seq);
#endif
	}

	return NULL;
}
#ifndef DISABLE_APS_HOOKS
struct net_bridge_port *hyfi_bridge_port_dev_get(struct net_device *dev,
	struct sk_buff *skb, unsigned char *addr, unsigned int ecm_serial)
{
	/* hybrid look up first */
	u_int32_t flag, priority;
	u_int32_t hash;
	u_int32_t traffic_class;
	u_int16_t seq = ~0;
	const struct net_bridge *br = netdev_priv(dev);
	const unsigned char *dest_addr, *src_addr;
	struct net_hatbl_entry *ha;
	struct net_bridge_port *port = NULL;
	struct hyfi_net_bridge * hyfi_br = NULL;

	hyfi_br = hyfi_bridge_get_by_dev(dev);
	if (unlikely(!hyfi_br))
		return NULL;

	if (unlikely(!br || !hyfi_br->dev || dev != hyfi_br->dev))
		return NULL;

	/* If not operating in APS mode, no hybrid tables are consulted. */
	if (unlikely(!hyfi_bridge_is_fwmode_aps(hyfi_br)))
		return NULL;

	if (unlikely(hyfi_hash_skbuf(skb, &hash, &flag, &priority, &seq)))
		return NULL;

	traffic_class = (flag & IS_IPPROTO_UDP) ?
		HYFI_TRAFFIC_CLASS_UDP : HYFI_TRAFFIC_CLASS_OTHER;

	/* Determine which hash to use */
	if (memcmp(eth_hdr(skb)->h_dest, addr, ETH_ALEN) &&
		memcmp(eth_hdr(skb)->h_source, addr, ETH_ALEN)) {

		dest_addr = eth_hdr(skb)->h_dest;
		src_addr = eth_hdr(skb)->h_source;

		DEBUG_TRACE("0x%x: Addr %x:%x:%x:%x:%x:%x doesn't match dest_addr "
			"%x:%x:%x:%x:%x:%x or src_addr %x:%x:%x:%x:%x:%x\n", hash,
			addr[0], addr[1], addr[2], addr[3], addr[4], addr[5],
			dest_addr[0], dest_addr[1], dest_addr[2], dest_addr[3],
			dest_addr[4], dest_addr[5],
			src_addr[0], src_addr[1], src_addr[2], src_addr[3],
			src_addr[4], src_addr[5]);

		/*
		 * Mismatch on source and destination, but we have been given
		 * the next-hop MAC address. Can't use the skb to determine
		 * the egress port, so can't use or create a H-Active entry.
		 * However, can determine the egress port from addr by looking up
		 * in H-Default and FDB
		 */
		return hyfi_bridge_get_dst_port_no_hash(hyfi_br, br, traffic_class, addr);
	} else if (memcmp(eth_hdr(skb)->h_dest, addr, ETH_ALEN)) {
		/* Should be using reverse hash */
		if (unlikely(hyfi_hash_skbuf_reverse(skb, &hash)))
			return NULL;
		dest_addr = eth_hdr(skb)->h_source;
		src_addr = eth_hdr(skb)->h_dest;

		DEBUG_TRACE("0x%x: Using reverse hash, "
			"dest_addr %x:%x:%x:%x:%x:%x src_addr %x:%x:%x:%x:%x:%x\n",
			hash,
			dest_addr[0], dest_addr[1], dest_addr[2], dest_addr[3],
			dest_addr[4], dest_addr[5],
			src_addr[0], src_addr[1], src_addr[2], src_addr[3],
			src_addr[4], src_addr[5]);
	} else {
		/* Should be using forward hash */
		dest_addr = eth_hdr(skb)->h_dest;
		src_addr = eth_hdr(skb)->h_source;
		DEBUG_TRACE("0x%x: Using forward hash, "
			"dest_addr %x:%x:%x:%x:%x:%x src_addr %x:%x:%x:%x:%x:%x\n",
			hash,
			dest_addr[0], dest_addr[1], dest_addr[2], dest_addr[3],
			dest_addr[4], dest_addr[5],
			src_addr[0], src_addr[1], src_addr[2], src_addr[3],
			src_addr[4], src_addr[5]);
	}

	/*
	 * Try looking up via ECM serial (passed in via the cookie) first
	 */
	spin_lock_bh(&hyfi_br->hash_ha_lock);
	ha = hatbl_find_ecm(hyfi_br, hash, ecm_serial, dest_addr);
	if (ha) {
		/* Update priority if needed*/
		if (ha->priority != priority) {
			DEBUG_INFO("0x%x: Priority changed to 0x%x (from 0x%x), "
				"dest %x:%x:%x:%x:%x:%x, serial %u\n",
				hash, priority, ha->priority,
				dest_addr[0], dest_addr[1], dest_addr[2], dest_addr[3],
				dest_addr[4], dest_addr[5], ecm_serial);
			ha->priority = priority;
		}

		port = ha->dst;
	}

	spin_unlock_bh(&hyfi_br->hash_ha_lock);

	if (port) {
		return port;
	}

	/* No H-Active match - new flow? */
	return hyfi_bridge_get_dst_port(hyfi_br, br, hash, traffic_class,
		priority, skb, dest_addr, src_addr, NULL);
}
#endif
int hyfi_bridge_should_deliver(const struct hyfi_net_bridge_port *src,
		const struct hyfi_net_bridge_port *dst, const struct sk_buff *skb)
{
	if (likely(src && dst)) {
		if (!hyfi_portgrp_relay(src)) {
			/* For non-relay group, allow forwarding only to other groups */
			return ((src->group_num != dst->group_num) &&
					(dst->group_num != HYFI_PORTGRP_INVALID));
		}
	}

	/* Allow forwarding otherwise */
	return 1;
}

static int hyfi_bridge_deinit_bridge_device(struct hyfi_net_bridge *hf_br)
{
	struct net_device *br_dev;
#ifndef DISABLE_APS_HOOKS
	int i, del_hooks = 1;
#endif

	/* Detach from existing bridge */
	br_dev = hf_br->dev;

	if (!br_dev) {
		return -1;
	}

	if (!strncmp(br_dev->name, hyfi_linux_bridge, strlen(hyfi_linux_bridge))) {
		hyfi_linux_bridge[0] = 0;
	}
	hf_br->linux_bridge[0] = 0;
	br_dev->needed_headroom -= 80;


#ifdef HYFI_MULTICAST_SUPPORT
	/* Multicast module detach to the bridge */
	mc_detach(hf_br);
#endif

	hyfi_bridge_del_ports(hf_br);
#ifndef DISABLE_APS_HOOKS
	hyfi_hatbl_flush(hf_br);
	hyfi_hdtbl_flush(hf_br);
#endif
	rcu_assign_pointer(hf_br->dev, NULL);

#ifndef DISABLE_APS_HOOKS
	for (i = 0; i < HYFI_BRIDGE_MAX; i++) {
		if (hyfi_bridges[i].dev != NULL)
			del_hooks = 0;
	}
	if (del_hooks) {
		rcu_assign_pointer(br_get_dst_hook, NULL);
		rcu_assign_pointer(br_port_dev_get_hook, NULL);
	}
#endif
	/*
	 * Note: Can't put the device until RCU is synchronized, which can't
	 * be done under lock.
	 */

	DEBUG_INFO("hyfi: Bridge %s is now detached\n", br_dev->name);
	return 0;
}

static int hyfi_bridge_init_bridge_device(struct hyfi_net_bridge *hyfi_br, const char *br_name)
{
	struct net_device *br_dev;

	if (!br_name) {
		return -1;
	}

	br_dev = dev_get_by_name(&init_net, br_name);

	if (!br_dev) {
		strlcpy(hyfi_br->linux_bridge, br_name, IFNAMSIZ);
		return 0;
	}

	strlcpy(hyfi_br->linux_bridge, br_name, IFNAMSIZ);

	/* Default bridge configuration */
	hyfi_br->flags = HYFI_BRIDGE_FLAG_MODE_RELAY_OVERRIDE
			| HYFI_BRIDGE_FLAG_MODE_TCP_SP
			| HYFI_BRIDGE_FLAG_FWMODE_NO_HYBRID_TABLES;

	br_dev->needed_headroom += 80;

	/* Init ports */
	hyfi_bridge_ports_init(hyfi_br, br_dev);
	rcu_assign_pointer(hyfi_br->dev, br_dev);
#ifndef DISABLE_APS_HOOKS
	/* see br_input.c */
	rcu_assign_pointer(br_get_dst_hook, hyfi_bridge_get_dst);

	/* see br_if.c */
	rcu_assign_pointer(br_port_dev_get_hook, hyfi_bridge_port_dev_get);
#endif
#ifdef HYFI_MULTICAST_SUPPORT
	/* Multicast module attach to the bridge */
	if (mc_attach(hyfi_br)<0)
		return -1;
#endif

	DEBUG_INFO("hyfi: Bridge %s is now attached\n", br_dev->name);

	return 0;
}

int __init hyfi_bridge_init(void)
{
	int i;
	memset(&hyfi_bridges, 0, sizeof(hyfi_bridges));
	strlcpy(hyfi_bridges[0].linux_bridge, hyfi_linux_bridge, IFNAMSIZ );
#ifndef DISABLE_APS_HOOKS
	hyfi_hatbl_init();
	if (hyfi_hdtbl_init()) {
		hyfi_hatbl_free();
		return -1;
	}
#endif
	for (i = 0; i < HYFI_BRIDGE_MAX; i++) {
		spin_lock_init(&hyfi_bridges[i].lock);
		spin_lock_init(&hyfi_bridges[i].hash_ha_lock);
		spin_lock_init(&hyfi_bridges[i].hash_hd_lock);

		hyfi_bridges[i].event_pid = NLEVENT_INVALID_PID;
		INIT_LIST_HEAD(&hyfi_bridges[i].port_list);
#ifndef DISABLE_APS_HOOKS
		/* Init tables */
		if (hyfi_hatbl_setup(&hyfi_bridges[i]))
			return -1;
#endif
		/* Init seamless path switching */
		hyfi_psw_init(&hyfi_bridges[i]);
	}

	return 0;
}

void __exit hyfi_bridge_fini(void)
{
	int i;

	for (i = 0; i < HYFI_BRIDGE_MAX; i++) {
		hyfi_bridge_set_bridge_name(&hyfi_bridges[i], NULL);
#ifndef DISABLE_APS_HOOKS
		hyfi_hatbl_fini(&hyfi_bridges[i]);
		hyfi_hdtbl_fini(&hyfi_bridges[i]);
#endif
	}
#ifndef DISABLE_APS_HOOKS
	hyfi_hatbl_free();
	hyfi_hdtbl_free();
#endif
}
