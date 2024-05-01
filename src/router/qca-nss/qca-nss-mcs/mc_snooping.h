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

#ifndef _MC_SNOOPING_H_
#define _MC_SNOOPING_H_

#include "mc_private.h"

int __init mc_snooping_init(void);

void mc_snooping_exit(void);

struct mc_struct *mc_dev_get(const struct net_device *dev);

void mc_dev_put(struct mc_struct *mc);

int mc_rcv(struct mc_struct *mc, struct sk_buff *skb, __u8 *from_mac, const struct net_device *dev);

int mc_find_acl_rule(struct mc_acl_rule_table *acl, __be32 in4, void *in6, __u8 *mac, __be32 rule);

int mc_group_hash(__be32 mdb_salt, __be32 group);

struct mc_mdb_entry *mc_mdb_find(struct hlist_head *head, struct mc_ip *group);

int mc_attach(struct net_device *dev);

void mc_detach(struct net_device *dev);

int mc_has_more_instance(void);

extern void mc_nbp_change(struct mc_struct *mc, struct net_device *dev, int event);

extern void mc_fdb_change(__u8 *mac, int change);

#endif


