/*
 * Copyright (c) 2012, 2015, 2018-2020 The Linux Foundation. All rights reserved.
 * Copyright (c) 2023-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <linux/kernel.h>
#include <net/net_namespace.h>
#include <net/sock.h>
#include <linux/etherdevice.h>

#include "mc_osdep.h"
#include "mc_api.h"
#include "mc_snooping.h"
#include "mc_netlink.h"

static struct sock *mc_nl_sk = NULL;
static __be32 event_pid = MC_INVALID_PID;

/*
 * mc_acltbl_update
 *	update the acl rules table
 */
static void mc_acltbl_update(struct mc_struct *mc, void *param)
{
	int i;
	struct mc_param_pattern pattern;
	struct __mc_param_acl_rule *ar = (struct __mc_param_acl_rule *)param;

	memset(&pattern, 0, sizeof pattern);
	if (ar->pattern_type == MC_ACL_PATTERN_IGMP) {
		pattern.rule = ar->pattern.rule;
		memcpy(pattern.mac, ar->pattern.mac, ETH_ALEN);
		memcpy(pattern.mac_mask, ar->pattern.mac_mask, ETH_ALEN);
		/* Input value in network byte order which be converted from the string by inet_pton() */
		pattern.ip.ip4 = *((__be32 *)ar->pattern.ip);
		pattern.ip_mask.ip4_mask = *((__be32 *)ar->pattern.ip_mask);

		if (pattern.ip.ip4) {
			for (i = 0; i < mc->igmp_acl.pattern_count; i++) {
				if (mc->igmp_acl.patterns[i].ip.ip4 == pattern.ip.ip4)
					break;
			}
		} else if (!is_zero_ether_addr(pattern.mac)) {
			for (i = 0; i < mc->igmp_acl.pattern_count; i++) {
				if (!memcmp(mc->igmp_acl.patterns[i].mac, pattern.mac,
						sizeof pattern - sizeof pattern.rule))
					break;
			}
		} else {
			goto out;
		}

		if (ar->pattern.rule == MC_ACL_RULE_DISABLE) {
			if (i != mc->igmp_acl.pattern_count) {
				if (i < mc->igmp_acl.pattern_count - 1)
					memcpy(&mc->igmp_acl.patterns[i], &mc->igmp_acl.patterns[i+1],
						(sizeof pattern) * (mc->igmp_acl.pattern_count - 1 - i));
				mc->igmp_acl.pattern_count--;
			}
			MC_PRINT(KERN_INFO "%s: Del IGMP acl rule, count=%d\n", __func__, mc->igmp_acl.pattern_count);
		} else {
			if (i != mc->igmp_acl.pattern_count) {
				memcpy(&mc->igmp_acl.patterns[i], &pattern, sizeof pattern);
				MC_PRINT(KERN_INFO "%s: Update acl rule\n", __func__);
				goto out;
			}

			if (mc->igmp_acl.pattern_count == MC_ACL_RULE_MAX_COUNT) {
				MC_PRINT(KERN_INFO "%s: Add IGMP acl rule failed, table is full\n", __func__);
				goto out;
			}

			memcpy(&mc->igmp_acl.patterns[mc->igmp_acl.pattern_count], &pattern, sizeof pattern);
			mc->igmp_acl.pattern_count++;
			MC_PRINT(KERN_INFO "%s: Add IGMP acl rule, count=%d\n", __func__, mc->igmp_acl.pattern_count);
		}
	}
#ifdef MC_SUPPORT_MLD
	else { /* MLD */
		pattern.rule = ar->pattern.rule;
		memcpy(pattern.mac, ar->pattern.mac, ETH_ALEN);
		memcpy(pattern.mac_mask, ar->pattern.mac_mask, ETH_ALEN);
		/* Input value in network byte order which be converted from the string by inet_pton() */
		ipv6_addr_set(&pattern.ip.ip6,
				((struct in6_addr *)ar->pattern.ip)->s6_addr32[0],
				((struct in6_addr *)ar->pattern.ip)->s6_addr32[1],
				((struct in6_addr *)ar->pattern.ip)->s6_addr32[2],
				((struct in6_addr *)ar->pattern.ip)->s6_addr32[3]);
		ipv6_addr_set(&pattern.ip_mask.ip6_mask,
				((struct in6_addr *)ar->pattern.ip_mask)->s6_addr32[0],
				((struct in6_addr *)ar->pattern.ip_mask)->s6_addr32[1],
				((struct in6_addr *)ar->pattern.ip_mask)->s6_addr32[2],
				((struct in6_addr *)ar->pattern.ip_mask)->s6_addr32[3]);

		if (!ipv6_addr_any(&pattern.ip.ip6)) {
			for (i = 0; i < mc->mld_acl.pattern_count; i++) {
				if (!ipv6_addr_cmp(&mc->mld_acl.patterns[i].ip.ip6, &pattern.ip.ip6))
					break;
			}
		} else if (!is_zero_ether_addr(pattern.mac)) {
			for (i = 0; i < mc->mld_acl.pattern_count; i++) {
				u_int32_t pattern_size = sizeof pattern - sizeof pattern.rule;

				if (pattern_size > ETH_ALEN)
					pattern_size = ETH_ALEN;
				if (!memcmp(mc->mld_acl.patterns[i].mac, pattern.mac, pattern_size))
					break;
			}
		} else {
			goto out;
		}

		if (ar->pattern.rule == MC_ACL_RULE_DISABLE) {
			if (i != mc->mld_acl.pattern_count) {
				if (i < mc->mld_acl.pattern_count - 1)
					memcpy(&mc->mld_acl.patterns[i], &mc->mld_acl.patterns[i+1],
						(sizeof pattern) * (mc->mld_acl.pattern_count - 1 - i));
				mc->mld_acl.pattern_count--;
			}
			MC_PRINT(KERN_INFO "%s: Del MLD acl rule, count=%d\n", __func__, mc->mld_acl.pattern_count);
		} else {
			if (i != mc->mld_acl.pattern_count) {
				memcpy(&mc->mld_acl.patterns[mc->mld_acl.pattern_count], &pattern, sizeof pattern);
				MC_PRINT(KERN_INFO "%s: Update acl rule\n", __func__);
				goto out;
			}

			if (mc->mld_acl.pattern_count == MC_ACL_RULE_MAX_COUNT) {
				MC_PRINT(KERN_INFO "%s: Add MLD acl rule failed, table is full\n", __func__);
				goto out;
			}

			memcpy(&mc->mld_acl.patterns[mc->mld_acl.pattern_count], &pattern, sizeof pattern);
			mc->mld_acl.pattern_count++;
			MC_PRINT(KERN_INFO "%s: Add MLD acl rule, count=%d\n", __func__, mc->mld_acl.pattern_count);
		}
	}
#endif
out:
	return;
}

/*
 * mc_acltbl_flush
 *	flush the acl rule table
 */
static void mc_acltbl_flush(struct mc_struct *mc, void *param)
{
	struct __mc_param_acl_rule *ar = (struct __mc_param_acl_rule *)param;

	if (ar->pattern_type == MC_ACL_PATTERN_IGMP) {
		memset(&mc->igmp_acl, 0, sizeof(mc->igmp_acl));
		MC_PRINT(KERN_INFO "%s: Flush IGMP acl rule table.\n", __func__);
	}
#ifdef MC_SUPPORT_MLD
	else { /* MLD */
		memset(&mc->mld_acl, 0, sizeof(mc->mld_acl));
		MC_PRINT(KERN_INFO "%s: Flush MLD acl rule table.\n", __func__);
	}
#endif
}

/*
 * mc_acltbl_fillbuf
 *	fill the acl rules table into the buf
 */
static int mc_acltbl_fillbuf(struct mc_struct *mc, void *buf,
			     __be32 buflen, __be32 *bytes_written, __be32 *bytes_needed)
{
	struct __mc_param_acl_rule *entry = buf;
	int i, total = 0, num = 0, num_entries, ret = 0;
	struct mc_param_pattern *p = &mc->igmp_acl.patterns[0];

	num_entries = buflen / sizeof(*entry);

	for (i = 0; i < mc->igmp_acl.pattern_count; i++) {
		total++;
		if (num >= num_entries) {
			ret = -EAGAIN;
			continue;
		}

		entry->pattern_type = MC_ACL_PATTERN_IGMP;
		entry->pattern.rule = p[i].rule;
		memcpy(entry->pattern.mac, p[i].mac, ETH_ALEN);
		memcpy(entry->pattern.mac_mask, p[i].mac_mask, ETH_ALEN);
		memcpy(entry->pattern.ip, &p[i].ip.ip4, sizeof(__be32));
		memcpy(entry->pattern.ip_mask, &p[i].ip_mask.ip4_mask, sizeof(entry->pattern.ip_mask));

		entry++;
		num++;
	}
	if (ret < 0)
		goto out;

#ifdef MC_SUPPORT_MLD
	p = &mc->mld_acl.patterns[0];
	for (i = 0; i < mc->mld_acl.pattern_count; i++) {
		total++;
		if (num >= num_entries) {
			ret = -EAGAIN;
			continue;
		}

		entry->pattern_type = MC_ACL_PATTERN_MLD;
		entry->pattern.rule = p[i].rule;
		memcpy(entry->pattern.mac, p[i].mac, ETH_ALEN);
		memcpy(entry->pattern.mac_mask, p[i].mac_mask, ETH_ALEN);
		memcpy(entry->pattern.ip, &p[i].ip.ip6, sizeof(entry->pattern.ip));
		memcpy(entry->pattern.ip_mask, &p[i].ip_mask.ip6_mask, sizeof(entry->pattern.ip_mask));

		entry++;
		num++;
	}
#endif
out:
	if (bytes_written)
		*bytes_written = num * sizeof(*entry);

	if (bytes_needed) {
		if (ret == -EAGAIN)
			*bytes_needed = total * sizeof(*entry);
		else
			*bytes_needed = 0;
	}
	return ret;
}

/*
 * mc_rtports_fillbuf
 *	fill the buf with the root port info
 *	called with rcu_read_lock()
 */
static int mc_rtports_fillbuf(struct mc_struct *mc, void *buf,
			     __be32 buflen, __be32 *bytes_written, __be32 *bytes_needed)
{
	struct __mc_rtport_entry *entry = buf;
	struct hlist_head *rhead = NULL;
	int num_entries, total = 0, num = 0, ret = 0;

	num_entries = buflen / sizeof(*entry);

	rhead = &mc->rp.igmp_rlist;
	if (!hlist_empty(rhead)) {
		struct mc_querier_entry *qe;
		struct hlist_node *h;
		os_hlist_for_each_entry_rcu(qe, h, rhead, rlist) {
			total++;
			if (num >= num_entries) {
				ret =  -EAGAIN;
				continue;

			}
			entry->ifindex = qe->ifindex;
			entry->ipv4 = 1;
			num++;
			entry++;
		}
	}

#ifdef MC_SUPPORT_MLD
	rhead = &mc->rp.mld_rlist;
	if (!hlist_empty(rhead)) {
		struct mc_querier_entry *qe;
		struct hlist_node *h;
		os_hlist_for_each_entry_rcu(qe, h, rhead, rlist) {
			total++;
			if (num >= num_entries) {
				ret =  -EAGAIN;
				continue;

			}
			entry->ifindex = qe->ifindex;
			entry->ipv4 = 0;
			num++;
			entry++;
		}
	}
#endif

	if (bytes_written)
		*bytes_written = num * sizeof(*entry);

	if (bytes_needed) {
		if (ret == -EAGAIN)
			*bytes_needed = total * sizeof(*entry);
		else
			*bytes_needed = 0;
	}

	return ret;
}

/*
 * mc_mdbtbl_fillbuf
 *	fill the mdbtbl into the netlink message data buf
 *	call with rcu_read_lock()
 */
static int mc_mdbtbl_fillbuf(struct mc_struct *mc, void *buf,
			     __be32 buflen, __be32 *bytes_written, __be32 *bytes_needed)
{
	unsigned long now = jiffies;
	struct __mc_mdb_entry *entry = buf;
	int i, total = 0, num = 0, num_entries, ret = 0;

	num_entries = buflen / sizeof(*entry);

	for (i = 0; i < MC_HASH_SIZE; i++) {
		struct mc_mdb_entry *mdb;
		struct hlist_node *mdbh;

		os_hlist_for_each_entry_rcu(mdb, mdbh, &mc->hash[i], hlist) {
			struct mc_port_group *pg;
			struct hlist_node *pgh;

			if (!atomic_read(&mdb->users) || hlist_empty(&mdb->pslist))
				continue;

			os_hlist_for_each_entry_rcu(pg, pgh, &mdb->pslist, pslist) {
				struct mc_fdb_group *fg;
				struct hlist_node *fgh;

				if (hlist_empty(&pg->fslist))
					continue;

				os_hlist_for_each_entry_rcu(fg, fgh, &pg->fslist, fslist) {
					total++;
					if (num >= num_entries) {
						ret = -EAGAIN;
						continue;
					}

					if (mdb->group.pro == htons(ETH_P_IP)) {
						entry->nsrcs = fg->a.nsrcs;
						entry->group.pro = mdb->group.pro;
						entry->group.u.ip4 = mdb->group.u.ip4;
						memcpy(entry->srcs, fg->a.srcs, fg->a.nsrcs * sizeof(int));
					}
#ifdef MC_SUPPORT_MLD
					else {
						entry->nsrcs = fg->a.nsrcs;
						entry->group.pro = mdb->group.pro;
						memcpy(entry->group.u.ip6, mdb->group.u.ip6.s6_addr, sizeof(struct in6_addr));
						memcpy(entry->srcs, fg->a.srcs, fg->a.nsrcs * sizeof(struct in6_addr));
					}
#endif
					entry->ifindex = ((struct net_device *)pg->port)->ifindex;
					entry->filter_mode = fg->filter_mode;
					entry->aging = jiffies_to_msecs(now - fg->ageing_timer) / 1000;
					entry->fdb_age_out = fg->fdb_age_out;
					memcpy(entry->mac, mc_fdb_mac_get(fg), ETH_ALEN);

					entry++;
					num++;
				}
			}
		}
	}

	if (bytes_written)
		*bytes_written = num * sizeof(*entry);

	if (bytes_needed) {
		if (ret == -EAGAIN)
			*bytes_needed = total * sizeof(*entry);
		else
			*bytes_needed = 0;
	}
	return ret;
}

/*
 * mc_find_entry_by_mdb
 *	find a entry with the same ip address in the mdb
 *	call with rcu_read_lock()
 */
static void *mc_find_entry_by_mdb(struct mc_struct *mc, struct mc_mdb_entry *mdb,
				  __be32 entry_size, void *param, __be32 param_len)
{
	struct mc_ip group;
	int i, entry_cnt = param_len / entry_size;
	__u8 *entry = param;

	for (i = 0; i < entry_cnt; i++, entry += entry_size) {
		struct __mc_group *entry_group = (struct __mc_group *)entry;

		memset(&group, 0, sizeof group);
		group.pro = entry_group->pro;
		if (group.pro == htons(ETH_P_IP))
			group.u.ip4 = entry_group->u.ip4;
#ifdef MC_SUPPORT_MLD
		else
			memcpy(group.u.ip6.s6_addr, entry_group->u.ip6, sizeof(struct in6_addr));
#endif

		if (!memcmp(&group, &mdb->group, sizeof(struct mc_ip)))
			return entry;
	}
	return NULL;
}

/*
 * mc_group_list_add
 *	add group into a list
 *	called with rcu_read_lock()
 */
static void mc_group_list_add(struct mc_ip *pgroup, struct mc_glist_entry **ghead)
{
	struct mc_glist_entry *pge;

	pge = kmalloc(sizeof(struct mc_glist_entry), GFP_ATOMIC);
	if (!pge) {
		printk("MC out-of-memory\n");
		return;
	}

	memcpy(&pge->group, pgroup, sizeof(struct mc_ip));
	pge->next = *ghead;
	*ghead = pge;
}

/*
 * mc_group_notify_one
 *	notify a group to the listeners
 */
void mc_group_notify_one(struct mc_struct *mc, struct mc_ip *pgroup)
{
	struct net_device *brdev;

	brdev = mc->dev;
	if (!brdev)
		return;

	if (pgroup->pro == htons(ETH_P_IP)) {
		mc_bridge_ipv4_update_callback_t ipv4_mc_event_cb;
		ipv4_mc_event_cb = mc_bridge_ipv4_update_callback_get();
		if (!ipv4_mc_event_cb)
			return;

		MC_PRINT("Group %pI4  changed\n", &pgroup->u.ip4);
		ipv4_mc_event_cb(brdev, pgroup->u.ip4);
	}
#ifdef MC_SUPPORT_MLD
	else {
		mc_bridge_ipv6_update_callback_t ipv6_mc_event_cb;
		ipv6_mc_event_cb = mc_bridge_ipv6_update_callback_get();
		if (!ipv6_mc_event_cb)
			return;

		MC_PRINT("Group %pI6  changed\n", &pgroup->u.ip6);
		ipv6_mc_event_cb(brdev, &pgroup->u.ip6);
	}
#endif
}


/*
 * mc_group_notify
 *	notify all group in the list to listener
 *	called with lock-free
 */
static void mc_group_notify(struct mc_struct *mc, struct mc_glist_entry **ghead)
{
	struct mc_glist_entry *pge;
	struct mc_glist_entry *prev;

	pge = *ghead;
	*ghead = NULL;

	while (pge) {
		mc_group_notify_one(mc, &pge->group);
		prev = pge;
		pge = pge->next;
		kfree(prev);
	}

}

/*
 * mc_set_psw_encap - set path selected way encapsulation
 */
static void mc_set_psw_encap(struct mc_struct *mc, void *param, __be32 param_len)
{
	int i, entry_cnt = param_len / sizeof(struct __mc_encaptbl_entry);
	struct __mc_encaptbl_entry *entry = param;

	MC_PRINT("%s: %s encap table\n", __func__, entry_cnt ? "Update" : "Clear");
	for (i = 0; i < MC_HASH_SIZE; i++) {
		struct mc_mdb_entry *mdb;
		struct hlist_node *mdbh;

		os_hlist_for_each_entry_rcu(mdb, mdbh, &mc->hash[i], hlist) {
			write_lock_bh(&mdb->rwlock);
			if (entry_cnt && ((entry = mc_find_entry_by_mdb(mc, mdb,
								sizeof(struct __mc_encaptbl_entry), param, param_len)) != NULL)) {
				mdb->encap_dev_cnt = entry->dev_cnt > MC_ENCAP_DEV_MAX ? MC_ENCAP_DEV_MAX : entry->dev_cnt;
				memcpy(mdb->encap_dev, entry->dev,	entry->dev_cnt * sizeof(struct __mc_encaptbl_dev));
			} else {
				mdb->encap_dev_cnt = 0;
				memset(mdb->encap_dev, 0,  sizeof(mdb->encap_dev));
			}
			write_unlock_bh(&mdb->rwlock);
		}
	}
}

/*
 * mc_set_psw_flood
 *	flood the path selected way in the whole mdb database
 */
static void mc_set_psw_flood(struct mc_struct *mc, void *param, __be32 param_len, struct mc_glist_entry **ghead)
{
	int i, entry_cnt = param_len / sizeof(struct __mc_floodtbl_entry);
	struct __mc_floodtbl_entry *entry = param;
	int    flood_ifcnt;
	int    entry_changed;

	MC_PRINT("%s: Update flood table\n", __func__);
	for (i = 0; i < MC_HASH_SIZE; i++) {
		struct mc_mdb_entry *mdb;
		struct hlist_node *mdbh;

		os_hlist_for_each_entry_rcu(mdb, mdbh, &mc->hash[i], hlist) {
			entry_changed = 0;

			write_lock_bh(&mdb->rwlock);
			if (entry_cnt && ((entry = mc_find_entry_by_mdb(mc, mdb,
								sizeof(struct __mc_floodtbl_entry), param, param_len)) != NULL)) {
				flood_ifcnt = mdb->flood_ifcnt;
				mdb->flood_ifcnt = entry->ifcnt > MC_FLOOD_IF_MAX ? MC_FLOOD_IF_MAX : entry->ifcnt;
				if (flood_ifcnt != mdb->flood_ifcnt ||
					memcmp(mdb->flood_ifindex, entry->ifindex, entry->ifcnt * sizeof(__be32)))
					entry_changed = 1;
				memcpy(mdb->flood_ifindex, entry->ifindex, entry->ifcnt * sizeof(__be32));
			} else {
				if (mdb->flood_ifcnt != 0)
					entry_changed = 1;
				mdb->flood_ifcnt = 0;
				memset(mdb->flood_ifindex, 0, sizeof(mdb->flood_ifindex));
			}
			write_unlock_bh(&mdb->rwlock);

			if (entry_changed) {
				mc_group_list_add(&mdb->group, ghead);
			}
		}
	}
}

/*
 * __mc_netlink_receive
 *	user space netlink message handler
 *	rcu read lock proteced
 */
static void __mc_netlink_receive(struct sk_buff *__skb)
{
	struct net_device *brdev = NULL;
	struct sk_buff *skb;
	struct nlmsghdr *nlh = NULL;
	void *msgdata = NULL;
	u32 pid, seq, msgtype;
	struct __mcctl_msg_header *msghdr;
	struct mc_struct *mc = NULL;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
	if ((skb = skb_clone(__skb, GFP_ATOMIC)) == NULL)
#else
	if ((skb = skb_get(__skb)) == NULL)
#endif
		return;

	/* process netlink message pointed by skb->data */
	nlh = nlmsg_hdr(skb);
	pid = nlh->nlmsg_pid;
	seq = nlh->nlmsg_seq;
	msghdr = NLMSG_DATA(nlh);
	msghdr->status = MC_STATUS_SUCCESS;
	msgdata = MC_MSG_DATA(nlh);
	msgtype = nlh->nlmsg_type;

	do {
		/*
		 * For global option to set, it needn't specified bridge
		 */
		if (msgtype == MC_MSG_SET_EVENT_PID) {
			struct __event_info *p = msgdata;

			event_pid = p->event_pid;
			break;
		}

		brdev = dev_get_by_name(&init_net, msghdr->if_name);
		if (!brdev) {
			msghdr->status = MC_STATUS_NOT_FOUND;
			break;
		}

		/*
		 * For enable/disable option, the handler make sure it won't
		 * have two instance with same device.
		 */
		if ( msgtype == MC_MSG_SET_ENABLE) {
			struct __mc_param_value *e = (struct __mc_param_value *)msgdata;

			if (e->val) {
				mc_attach(brdev);
			} else {
				mc_detach(brdev);
			}
			dev_put(brdev);
			break;
		}

		/*
		 * For mc instance option, it need find the instance before any setting
		 */
		mc = MC_DEV(brdev);
		if (!mc) {
			msghdr->status = MC_STATUS_FAILURE;
			dev_put(brdev);
			break;
		}

		switch (msgtype) {
		case MC_MSG_SET_DEBUG:
			{
				struct __mc_param_value *e = (struct __mc_param_value *)msgdata;
				mc->debug = e->val;
				MC_PRINT(KERN_INFO "%s: %s MC[%s] debug\n",
						__func__, e->val ? "Enable" : "Disable", brdev->name);
			}
			break;
		case MC_MSG_SET_POLICY:
			{
				struct __mc_param_value *p = (struct __mc_param_value *)msgdata;
				mc->forward_policy = p->val;
				MC_PRINT(KERN_INFO "%s: Set the forward policy %s\n", __func__,
						p->val == MC_POLICY_FLOOD ? "FLOOD" : "DROP");
			}
			break;
		case MC_MSG_SET_MEMBERSHIP_INTERVAL:
			{
				struct __mc_param_value *mi = (struct __mc_param_value *)msgdata;
				mc->membership_interval = mi->val * HZ;
				MC_PRINT(KERN_INFO "%s: Set membership interval to %ds\n", __func__, mi->val);
			}
			break;
		case MC_MSG_SET_RETAG:
			{
				struct __mc_param_retag *t = (struct __mc_param_retag *)msgdata;
				mc->enable_retag = t->enable;
				mc->dscp = t->dscp & 0xff;
				MC_PRINT(KERN_INFO "%s: %s retag, DSCP=%02x\n", __func__, t->enable ? "Enable" : "Disable", mc->dscp);
			}
			break;
		case MC_MSG_SET_ROUTER_PORT:
			{
				struct __mc_param_router_port *rp = (struct __mc_param_router_port *)msgdata;
				if (rp->type >= MC_RTPORT_MAX) {
					MC_PRINT(KERN_ERR "%s: Invalid router port type %d!\n", __func__, rp->type);
				} else {
					mc->rp.type = rp->type;
					mc->rp.ifindex = rp->ifindex;
					MC_PRINT(KERN_INFO "%s: Set router port type=%d, ifindex=%d\n", __func__, rp->type, rp->ifindex);
				}
			}
			break;
		case MC_MSG_SET_ADD_ACL_RULE:
			{
				spin_lock_bh(&mc->lock);
				mc_acltbl_update(mc, msgdata);
				spin_unlock_bh(&mc->lock);
			}
			break;
		case MC_MSG_SET_FLUSH_ACL_RULE:
			{
				spin_lock_bh(&mc->lock);
				mc_acltbl_flush(mc, msgdata);
				spin_unlock_bh(&mc->lock);
			}
			break;
		case MC_MSG_SET_CONVERT_ALL:
			{
				struct __mc_param_value *e = (struct __mc_param_value *)msgdata;
				mc->convert_all = e->val;
				MC_PRINT(KERN_INFO "%s: %s MC convert all.\n", __func__, e->val ? "Enable" : "Disable");
			}
			break;
		case MC_MSG_SET_TIMEOUT:
			{
				struct __mc_param_timeout *t = (struct __mc_param_timeout *)msgdata;
				switch (t->from) {
				case MC_TIMEOUT_FROM_GROUP_SPECIFIC_QUERIES:
					mc->timeout_gsq_enable = t->enable;
					MC_PRINT("%s: %s timeout from %s.\n", __func__,
						t->enable ? "Enable" : "Disable", "group specific queries");
					break;
				case MC_TIMEOUT_FROM_ALL_SYSTEM_QUERIES:
					mc->timeout_asq_enable = t->enable;
					MC_PRINT("%s: %s timeout from %s.\n", __func__,
							t->enable ? "Enable" : "Disable", "all system queries");
					break;
				case MC_TIMEOUT_FROM_GROUP_MEMBERSHIP_INTERVAL:
					mc->timeout_gmi_enable = t->enable;
					MC_PRINT("%s: %s timeout from %s.\n", __func__,
							t->enable ? "Enable" : "Disable", "group membership interval");
					break;
				default:
					MC_PRINT("%s: Set timeout failed, invalid value %d.\n", __func__, t->from);
				}
			}
			break;
		case MC_MSG_SET_M2I3_FILTER:
			{
				struct __mc_param_value *e = (struct __mc_param_value *)msgdata;
				mc->m2i3_filter_enable = e->val;
				MC_PRINT(KERN_INFO "%s: %s IGMPv3/MLDv2 Leave Filter.\n", __func__, e->val ? "Enable" : "Disable");
			}
			break;
		case MC_MSG_SET_TBIT:
			{
				struct __mc_param_value *e = (struct __mc_param_value *)msgdata;
				mc->ignore_tbit = e->val;
				MC_PRINT(KERN_INFO "%s: %s 'ignore T-bit'.\n", __func__, e->val ? "Enable" : "Disable");
			}
			break;
		case MC_MSG_SET_LOCAL_QUERY_INTERVAL:
			{
				struct __mc_param_value *i = (struct __mc_param_value *)msgdata;
				if (i->val) {
					spin_lock_bh(&mc->lock);
					mc->local_query_interval = i->val * HZ;
					if (timer_pending(&mc->qtimer) ?
						time_after(mc->qtimer.expires, jiffies + mc->local_query_interval) :
							try_to_del_timer_sync(&mc->qtimer) >= 0) {
						if (mc->started)
							mod_timer(&mc->qtimer, jiffies + mc->local_query_interval);
					}
					spin_unlock_bh(&mc->lock);
				}
				MC_PRINT(KERN_INFO "%s: Set local query interval to %u\n", __func__, i->val);
			}
			break;
		case MC_MSG_SET_PSW_ENCAP:
			{
				mc_set_psw_encap(mc, msgdata, msghdr->buf_len);
			}
			break;
		case MC_MSG_SET_PSW_FLOOD:
			{
				struct mc_glist_entry *ghead = NULL;
				mc_set_psw_flood(mc, msgdata, msghdr->buf_len, &ghead);
				mc_group_notify(mc, &ghead);
			}
			break;
		case MC_MSG_GET_ACL:
			{
				spin_lock_bh(&mc->lock);
				if (mc_acltbl_fillbuf(mc, msgdata, msghdr->buf_len,
						&msghdr->bytes_written, &msghdr->bytes_needed))
					msghdr->status = MC_STATUS_BUFFER_OVERFLOW;
				spin_unlock_bh(&mc->lock);
			}
			break;
		case MC_MSG_GET_MDB:
			{
				spin_lock_bh(&mc->lock);
				if (mc_mdbtbl_fillbuf(mc, msgdata, msghdr->buf_len,
							&msghdr->bytes_written, &msghdr->bytes_needed))
					msghdr->status = MC_STATUS_BUFFER_OVERFLOW;
				spin_unlock_bh(&mc->lock);
			}
			break;
		case MC_MSG_DEL_MDB:
			{
				struct __mc_mdb_entry *e = (struct __mc_mdb_entry *)msgdata;
				struct mc_ip mc_group;
				 memset(&mc_group, 0, sizeof(struct mc_ip));
				if (e->group.pro == htons(ETH_P_IP) ) {
					mc_group.u.ip4=e->group.u.ip4;
					mc_group.pro=e->group.pro;
					MC_PRINT("%s:del group=%pI4, ifindex=%d, mac=%pM\n",
						 __func__, &e->group.u.ip4,e->ifindex, e->mac);
				}
				else {
					mc_group.pro = e->group.pro;
					memcpy(&mc_group.u.ip6, &e->group.u.ip6, sizeof(struct in6_addr));
					MC_PRINT("%s:del group=%pI6, ifindex=%d, mac=%pM\n",
						 __func__, &e->group.u.ip6,e->ifindex, e->mac);
				}

				spin_lock_bh(&mc->lock);
				if(mc_mdb_destroy_by_port(mc, &mc_group, (__u8 *)e->mac, e->ifindex)>0)
				{
					MC_PRINT("%s(%d):Found group!\n", __func__, __LINE__);
					msghdr->status=MC_STATUS_SUCCESS;
				}
				else
				{
					MC_PRINT("%s(%d):Not found group!\n", __func__, __LINE__);
					msghdr->status=MC_STATUS_NOT_FOUND;
				}
				spin_unlock_bh(&mc->lock);
			}
			break;
		case MC_MSG_SET_ROUTER:
			{
				struct __mc_param_value *e = (struct __mc_param_value *)msgdata;
				mc->multicast_router = e->val;
				MC_PRINT(KERN_INFO "%s: %s multicast router.\n", __func__, e->val ? "Enable" : "Disable");
			}
			break;
		case MC_MSG_GET_ROUTER_PORT:
			{
				if (mc_rtports_fillbuf(mc, msgdata, msghdr->buf_len,
							&msghdr->bytes_written, &msghdr->bytes_needed))
					msghdr->status = MC_STATUS_BUFFER_OVERFLOW;
			}
			break;
		default:
			MC_PRINT("mc: Unknown message type 0x%x\n", msgtype);
			msghdr->status = MC_STATUS_INVALID_PARAMETER;
			break;
		} /* switch */

		dev_put(brdev);

	} while (0);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 7, 0))
	NETLINK_CB(skb).portid = 0; /* from kernel */
#else
	NETLINK_CB(skb).pid = 0; /* from kernel */
#endif
	NETLINK_CB(skb).dst_group = 0; /* unicast */
	netlink_unicast(mc_nl_sk, skb, pid, MSG_DONTWAIT);
}

/*
 * mc_netlink_receive
 *	netlink message receiver
 *	the entry of netlink message, rcu protected the further process
 */
static void mc_netlink_receive(struct sk_buff *__skb)
{
	rcu_read_lock();
	__mc_netlink_receive(__skb);
	rcu_read_unlock();
}

/*
 * mc_netlink_event_send - send a event to user space by netlink message
 */
void mc_netlink_event_send(struct mc_struct *mc, u32 event_type, u32 event_len, void *event_data)
{
	struct sk_buff *skb;
	struct nlmsghdr *nlh;
	struct __mcctl_msg_header *data;
	int send_msg = 1;

	if (!mc || event_pid == MC_INVALID_PID ||
			event_type >= MC_EVENT_MAX)
		return;

	skb = nlmsg_new(event_len + MC_MSG_HDRLEN, gfp_any());
	if (!skb) {
		MC_PRINT("nlmsg_new failed, event_type=%d\n", event_type);
		return;
	}

	nlh = nlmsg_put(skb, event_pid, 0, event_type, event_len + MC_MSG_HDRLEN, 0);
	if (!nlh) {
		MC_PRINT("nlmsg_put failed, event_type=%d\n", event_type);
		kfree_skb(skb);
		return;
	}

	switch (event_type) {
	case MC_EVENT_MDB_UPDATED:
		data = NLMSG_DATA(nlh);
		memcpy(data->if_name, mc->dev->name, IFNAMSIZ);
		break;

	default:
		MC_PRINT("event type %d is not supported\n", event_type);
		send_msg = 0;
		kfree_skb(skb);
		break;
	}

	if (send_msg) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 7, 0))
		NETLINK_CB(skb).portid = 0; /* from kernel */
#else
		NETLINK_CB(skb).pid = 0; /* from kernel */
#endif
		NETLINK_CB(skb).dst_group = 0; /* unicast */
		netlink_unicast(mc_nl_sk, skb, event_pid, MSG_DONTWAIT);
	}
}

/*
 * mc_netlink_init
 *	netlink process module init
 */
int __init mc_netlink_init(void)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 6, 0))
	struct netlink_kernel_cfg nlcfg;
	memset(&nlcfg, 0, sizeof(nlcfg));
	nlcfg.groups = 0;
	nlcfg.input = mc_netlink_receive;
	mc_nl_sk = netlink_kernel_create(&init_net,
			NETLINK_QCA_MC,
			&nlcfg);
#else
	mc_nl_sk = netlink_kernel_create(&init_net,
			NETLINK_QCA_MC,
			0,
			mc_netlink_receive,
			NULL,
			THIS_MODULE);
#endif
	if (mc_nl_sk == NULL)
		goto err;

	return 0;
err:
	printk("mc: Failed to create netlink socket\n");
	return -ENODEV;
}

/*
 * mc_netlink_exit
 *	netlink process module exit
 */
void mc_netlink_exit(void)
{
	if (mc_nl_sk) {
		sock_release(mc_nl_sk->sk_socket);
		mc_nl_sk = NULL;
	}
}

