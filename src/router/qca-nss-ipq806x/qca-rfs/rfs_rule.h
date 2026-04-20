/*
 * Copyright (c) 2014 - 2017, The Linux Foundation. All rights reserved.
 *
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
 */

/*
 * rfs_rule.h
 *	Receiving Flow Streering - Rules
 */

#ifndef __RFS_RULE_H
#define __RFS_RULE_H

#define MAX_VLAN_PORT 6

struct rfs_rule_entry {
        struct hlist_node hlist;
	struct rcu_head rcu;
	uint32_t type;
	uint8_t mac[ETH_ALEN];
	union {
		__be32  ip4addr;
		struct in6_addr ip6addr;
	} u;
	/*
	 * Interface(from which packets coming) VLAN IDs and MAC addresses
	 */
	uint32_t nvif;
	uint32_t ifvid[MAX_VLAN_PORT];
	uint8_t  ifmac[MAX_VLAN_PORT *6];
	uint32_t is_static;
	uint16_t cpu;
	/*
	 * Host bridge(the interface which host is connecting)
	 * Currently, it's only used for MAC rule.
	 */
	int brindex;
	struct net_device *to;
};

#define RFS_RULE_TYPE_MAC_RULE 1
#define RFS_RULE_TYPE_IP4_RULE 2
#define RFS_RULE_TYPE_IP6_RULE 3

int rfs_rule_create_mac_rule(uint8_t *addr, uint16_t cpu, struct net_device *to, uint32_t is_static);
int rfs_rule_destroy_mac_rule(uint8_t *addr,  struct net_device *to, uint32_t is_static);
int rfs_rule_find_mac_rule(uint8_t *addr, struct net_device *to, uint32_t is_static);
int rfs_rule_create_ip_rule(int family, uint8_t *ipaddr, uint8_t *maddr, uint16_t cpu, uint32_t is_static);
int rfs_rule_destroy_ip_rule(int family, uint8_t *addr, uint32_t is_static);
uint16_t rfs_rule_get_cpu_by_ipaddr(__be32 ipaddr);
void rfs_rule_reset_all(void);
void rfs_rule_destroy_all(void);

int rfs_rule_init(void);
void rfs_rule_exit(void);

#endif
