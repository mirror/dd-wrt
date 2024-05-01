/*
 * Copyright (c) 2012, 2015, 2017, 2019 The Linux Foundation. All rights reserved.
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

#ifndef _MC_PRIVATE_H
#define _MC_PRIVATE_H

#include <linux/version.h>
#include <linux/device.h>
#include <linux/rcupdate.h>
#include <linux/types.h>
#include <linux/spinlock_types.h>
#include <net/ip.h>

#include <br_private.h>
#include "mc_api.h"
#include "mc_ecm.h"

#ifdef MC_SUPPORT_MLD
#include <linux/in6.h>
#include <linux/ipv6.h>
#include <net/addrconf.h>
#include <net/ip6_checksum.h>

/* MLDv1 Query/Report/Done */
struct mld_msg {
	struct icmp6hdr mld_hdr;
	struct in6_addr mld_mca;
};

#define mld_type	mld_hdr.icmp6_type
#define mld_code	mld_hdr.icmp6_code
#define mld_cksum	mld_hdr.icmp6_cksum
#define mld_maxdelay	mld_hdr.icmp6_maxdelay
#define mld_reserved	mld_hdr.icmp6_dataun.un_data16[1]

/* Multicast Listener Discovery version 2 headers */
/* MLDv2 Report */
struct mld2_grec {
	__u8 grec_type;
	__u8 grec_auxwords;
	__be16 grec_nsrcs;
	struct in6_addr grec_mca;
	struct in6_addr grec_src[0];
};

struct mld2_report {
	struct icmp6hdr mld2r_hdr;
	struct mld2_grec mld2r_grec[0];
};

#define mld2r_type	mld2r_hdr.icmp6_type
#define mld2r_resv1	mld2r_hdr.icmp6_code
#define mld2r_cksum	mld2r_hdr.icmp6_cksum
#define mld2r_resv2	mld2r_hdr.icmp6_dataun.un_data16[0]
#define mld2r_ngrec	mld2r_hdr.icmp6_dataun.un_data16[1]

/* MLDv2 Query */
struct mld2_query {
	struct icmp6hdr mld2q_hdr;
	struct in6_addr mld2q_mca;
#if defined(__LITTLE_ENDIAN_BITFIELD)
	__u8	mld2q_qrv:3,
		mld2q_suppress:1,
		mld2q_resv2:4;
#elif defined(__BIG_ENDIAN_BITFIELD)
	__u8	mld2q_resv2:4,
		mld2q_suppress:1,
		mld2q_qrv:3;
#else
#error "Please fix <asm/byteorder.h>"
#endif
	__u8 mld2q_qqic;
	__be16 mld2q_nsrcs;
	struct in6_addr mld2q_srcs[0];
};

#define mld2q_type		mld2q_hdr.icmp6_type
#define mld2q_code		mld2q_hdr.icmp6_code
#define mld2q_cksum		mld2q_hdr.icmp6_cksum
#define mld2q_mrc		mld2q_hdr.icmp6_maxdelay
#define mld2q_resv1		mld2q_hdr.icmp6_dataun.un_data16[1]

#define MLDV2_MASK(value, nb) ((nb) >= 32 ? (value) : ((1<<(nb))-1) & (value))
#define MLDV2_EXP(thresh, nbmant, nbexp, value) \
			((value) < (thresh) ? (value) : \
			((MLDV2_MASK(value, nbmant) | (1<<(nbmant))) << \
			(MLDV2_MASK((value) >> (nbmant), nbexp) + (nbexp))))

#define MLDV2_MRC(value) MLDV2_EXP(0x8000, 12, 3, value)

#endif

#define MC_HASH_SIZE		512
#define MC_DEFAULT_DSCP		0x28
#define MC_DSCP(x)			(x << 2)
#define MC_GROUP_MAX		MC_DEF_GROUP_MAX
#define MC_SRC_GROUP_MAX	MC_DEF_SRCS_MAX
#define MC_RT_SRC_MAX		MC_DEF_RT_SRCS_MAX
#define MC_ENCAP_DEV_MAX	MC_DEF_DEV_MAX
#define MC_FLOOD_IF_MAX		MC_DEF_IF_MAX
#define MC_EVENT_DELAY_MS	3	/* ms */
#define MC_CHECK_TBIT(ip6)	(((ip6)->s6_addr[1] & 0x10) &&	((ip6)->s6_addr[12] & 0x80))
#define MC_INVALID_PID		(-1)

#define MC_DEV(dev) (mc_dev_get(dev))

#define MC_DEBUG
#ifdef MC_DEBUG
#define MC_PRINT(fmt, ...)	do {if (mc->debug) printk(fmt, ##__VA_ARGS__); } while (0)
#else
#define MC_PRINT(fmt, ...)
#endif

struct mc_ip {
	union {
		__be32 ip4;
#ifdef MC_SUPPORT_MLD
		struct in6_addr ip6;
#endif
	} u;
	__be16 pro;
};

struct mc_glist_entry {
	struct mc_ip group;
	struct mc_glist_entry *next;
};

struct mc_querier_entry {
	struct hlist_node rlist;	/* attach to router port list */
	struct rcu_head rcu;
	unsigned long ageing_timer;
	const void *dev;
	__be32 ifindex;
	struct mc_ip sip;
	unsigned long max_resp_time;	/* Max Resp Code */
	unsigned long qqic;	/* Querier's Query Interval Code */
	unsigned long qrv;	/* Querier's Robustness Variable */
};

struct mc_router_port {
	__be32 type;
	__be32 ifindex;
	struct hlist_head igmp_rlist;	/* for ipv4 router port list */
	struct mc_querier_entry *igmp_root_qe;
#ifdef MC_SUPPORT_MLD
	struct hlist_head mld_rlist;	/* for ipv6 router port list */
	struct mc_querier_entry *mld_root_qe;
#endif
};

struct mc_param_pattern {
	__be32 rule;
	__u8 mac[ETH_ALEN];
	__u8 mac_mask[ETH_ALEN];
	union {
		__be32 ip4;
#ifdef MC_SUPPORT_MLD
		struct in6_addr ip6;
#endif
	} ip;
	union {
		__be32 ip4_mask;
#ifdef MC_SUPPORT_MLD
		struct in6_addr ip6_mask;
#endif
	} ip_mask;
};

struct mc_acl_rule_table {
	__be32 pattern_count;
	struct mc_param_pattern patterns[MC_ACL_RULE_MAX_COUNT];
};

struct mc_struct {
	struct rcu_head rcu;
	spinlock_t lock;
	__be32 salt;		/* salt for hash */
	struct net_device *dev;	/* bridge device */
	__be32 enable;
	__be32 started;
	__be32 active_group_count;
	struct hlist_head hash[MC_HASH_SIZE];
	__be32 debug;
	__be32 forward_policy;
	__be32 last_member_count;
	__be32 startup_queries_sent;
	__be32 startup_query_count;
	unsigned long last_member_interval;
	unsigned long membership_interval;
	unsigned long querier_interval;
	unsigned long query_interval;
	unsigned long query_response_interval;
	unsigned long local_query_interval;
	__be32 enable_retag;
	__be32 dscp;
	struct mc_acl_rule_table igmp_acl;
#ifdef MC_SUPPORT_MLD
	struct mc_acl_rule_table mld_acl;
#endif
	__be32 convert_all;	/* zero indicate UDP only */
	__be32 timeout_gsq_enable;	/* enable timeout from group sepecific query */
	__be32 timeout_asq_enable;	/* enable timeout from all system query */
	__be32 timeout_gmi_enable;	/* enable timeout from group membership interval */
	__be32 m2i3_filter_enable;	/* enable mldv2/igmpv3 leave filter */
	__be32 ignore_tbit;	/* When this option is enabled, we will allow IPv6 Multicast
				   Groups, that don’t have the T-Bit enabled, to be snooped */

	struct mc_router_port rp;	/* router port */
	struct timer_list qtimer;	/* for sending query packet */
	struct timer_list agingtimer;	/* for time aging */
	struct timer_list rtimer;	/* for router port time aging */
	struct timer_list evtimer;	/* for event delay timer */
	unsigned long ageing_query;

	unsigned char multicast_router;
};

struct mc_rt_src_list {
	int nsrcs;
#ifdef MC_SUPPORT_MLD
	unsigned char srcs[MC_RT_SRC_MAX * sizeof(struct in6_addr)];
#else
	unsigned char srcs[MC_RT_SRC_MAX * sizeof(__be32)];
#endif
};

struct mc_mdb_entry {
	struct hlist_node hlist;	/* mdb hash table */
	atomic_t users;
	struct rcu_head rcu;
	struct mc_ip group;
	unsigned long ageing_query;
	struct hlist_head pslist;	/* head of port groups */
	struct mc_struct *mc;	/* pointed back to mc */
	unsigned long timer_base;
	struct timer_list etimer;	/* expire timer */
	int filter_mode;	/* for IGMPv3 and MLDv2 */
	struct mc_rt_src_list x;	/* Sources to be forwarded */
	struct mc_rt_src_list y;	/* Sources not to forward */
	rwlock_t rwlock;
	int encap_dev_cnt;
	struct __mc_encaptbl_dev encap_dev[MC_ENCAP_DEV_MAX];
	int flood_ifcnt;
	int flood_ifindex[MC_FLOOD_IF_MAX];
};

struct mc_port_group {
	void *port;		/* pointed to port */
	struct hlist_node pslist;	/* attach to mdb */
	struct rcu_head rcu;
	struct mc_mdb_entry *mdb;	/* pointed back to mdb */
	unsigned long ageing_timer;
	struct hlist_head fslist;	/* head of fdb groups */
};

struct mc_src_list {
	int nsrcs;
#ifdef MC_SUPPORT_MLD
	unsigned char srcs[MC_SRC_GROUP_MAX * sizeof(struct in6_addr)];
#else
	unsigned char srcs[MC_SRC_GROUP_MAX * sizeof(__be32)];
#endif
};

struct mc_fdb_group {
	struct mc_port_group *pg;	/* pointed back to port group  */
	struct hlist_node fslist;	/* attach to port group */
	struct rcu_head rcu;
	unsigned long ageing_timer;
	int filter_mode;	/* filter mode of host */
	struct mc_src_list a;	/* sources list of host */
	__u8 mac[ETH_ALEN];
	int fdb_age_out;
};

enum {
	MC_REPORT = 1,
	MC_LEAVE
};

struct mc_skb_cb {
	unsigned char igmp;		/* nonzero indicator the skb is an IGMP packet */
	unsigned char type;		/* MC_REPORT or MC_LEAVE */
	unsigned char non_snoop;	/* nonzero indicator the skb is an snooping packet */
	unsigned char reserveed;	/* reserved for furtue using aligned to 32 */
	void *mdb;		/* pointed to mdb entry */
};

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 3, 0))
#define MC_SKB_CB(__skb)  ((struct mc_skb_cb *)((__skb)->cb + sizeof(struct br_input_skb_cb)))
#else
#define MC_SKB_CB(__skb)  ((struct mc_skb_cb *)(__skb)->cb)
#endif
#define MC_GREC_NSRCS(grec_nsrcs) ((grec_nsrcs) > MC_SRC_GROUP_MAX ? MC_SRC_GROUP_MAX : (grec_nsrcs))

static inline void mc_acl_mac_mask(__u8 *ret, __u8 *mac, __u8 *mac_mask)
{
	memset(ret, 0, ETH_ALEN);
	ret[0] = mac_mask[0] & mac[0];
	ret[1] = mac_mask[1] & mac[1];
	ret[2] = mac_mask[2] & mac[2];
	ret[3] = mac_mask[3] & mac[3];
	ret[4] = mac_mask[4] & mac[4];
	ret[5] = mac_mask[5] & mac[5];
}

static inline __u8 *mc_fdb_mac_get(struct mc_fdb_group *fg)
{
	return fg->mac;
}

static inline int mc_device_is_router(struct mc_struct *mc)
{
	return mc->multicast_router > 0 ? 1 : 0;
}
#endif
