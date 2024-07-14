/*
 *  HyFi forwarding database
 *  QCA HyFi Bridge
 *
 * Copyright (c) 2012-2016, The Linux Foundation. All rights reserved.
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
#include <linux/rculist.h>
#include <linux/spinlock.h>
#include <linux/etherdevice.h>
#include <mc_private.h>
#include "hyfi_netfilter.h"
#include "hyfi_netlink.h"
#include "hyfi_bridge.h"
#include "hyfi_api.h"
#include "hyfi_fdb.h"
/* ref_port_ctrl.h and ref_fdb.h header file is  platform dependent code and this
   is not required for 3rd party platform. So avoided this header file inclusion
   by the flag HYFI_DISABLE_SSDK_SUPPORT
 */
#ifndef HYFI_DISABLE_SSDK_SUPPORT
#include "ref/ref_port_ctrl.h"
#include "ref/ref_fdb.h"
#endif

static inline unsigned long hold_time(const struct net_bridge *br)
{
	return br->topology_change ? br->forward_delay : br->ageing_time;
}

static inline int has_expired(const struct net_bridge *br,
		const struct net_bridge_fdb_entry *fdb)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6,1,0))
	return !test_bit(BR_FDB_STATIC,&fdb->flags)
			&& time_before_eq(hyfi_updated_time_get(fdb) + hold_time( br ), jiffies );
#else
	return !fdb->is_static
			&& time_before_eq(hyfi_updated_time_get(fdb) + hold_time( br ), jiffies );
#endif
}

void hyfi_fdb_perport(struct hyfi_net_bridge *hyfi_br,struct __switchport_index *pid)
{
	struct __ssdkport_entry portEntry;
	memcpy((char *)portEntry.addr, (char *)pid->mac_addr, sizeof(portEntry.addr));
	portEntry.vlanid = pid->vlanid;
	/* if MAC not found on any one of switch port then ssdk returns with 0xffffffff */
	if (!(~pid->portid)) {
		portEntry.portid = 0xFF;
		portEntry.portlink = 0;
	}
	else {
		portEntry.portid = pid->portid;
		portEntry.portlink = 1;
	}
	hyfi_netlink_event_send(hyfi_br, HYFI_EVENT_MAC_LEARN_ON_PORT, sizeof(portEntry), (void *)&portEntry);
}
/*
 * Fill buffer with forwarding table records in
 * the API format.
 */
int hyfi_fdb_fillbuf(struct net_bridge *br, void *buf, u_int32_t buf_len,
		u_int32_t skip, u_int32_t *bytes_written, u_int32_t *bytes_needed)
{
	struct __hfdb_entry *fe = buf;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	u_int32_t total = 0, num = 0, num_entries;
#else
	u_int32_t i, total = 0, num = 0, num_entries;
#endif
	int ret = 0;
	struct hlist_node *h;
	struct net_bridge_fdb_entry *f;
	bool fdb_flag;

	memset(buf, 0, buf_len);
	num_entries = buf_len / sizeof(struct __hfdb_entry);

	rcu_read_lock();
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	os_hlist_for_each_entry_rcu(f, h, &br->fdb_list, fdb_node) {
#else
	for (i = 0; i < BR_HASH_SIZE; i++) {
		os_hlist_for_each_entry_rcu(f, h, &br->hash[i], hlist)	{
#endif
			if (has_expired(br, f))
				continue;

			total++;
			if (num >= num_entries) {
				ret = -EAGAIN;
				continue;
			}

			/* Ignore any local entries that do not have a valid
			 * port
			 */
			if (!f->dst)
				continue;

			if (skip) {
				skip--;
				continue;
			}

			/* convert from internal format to API */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
			memcpy(fe->mac_addr, f->key.addr.addr, ETH_ALEN);
#else
			memcpy(fe->mac_addr, f->addr.addr, ETH_ALEN);
#endif

			/* due to ABI compat need to split into hi/lo */
			fe->ifindex = f->dst->dev->ifindex & 0xff;
			fe->ifindex_hi = (f->dst->dev->ifindex >> 8) & 0xff;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6,1,0))
			fe->is_local = test_bit(BR_FDB_LOCAL, &f->flags);
			fdb_flag = test_bit(BR_FDB_STATIC, &f->flags);
#else
			fe->is_local = f->is_local;
			fdb_flag = f->is_static;
#endif

			if (!fdb_flag)
				fe->ageing_timer_value = jiffies_to_clock_t(
						jiffies - hyfi_updated_time_get(f));
			++fe;
			++num;
		}
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
        }
#endif
	rcu_read_unlock();
	if (bytes_written)
		*bytes_written = num * sizeof(struct __hfdb_entry);

	if (bytes_needed) {
		if (ret == -EAGAIN)
			*bytes_needed = total * sizeof(struct __hfdb_entry);
		else
			*bytes_needed = 0;
	}

	return ret;
}

