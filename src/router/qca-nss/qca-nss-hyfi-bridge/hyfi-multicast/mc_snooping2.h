/*
 * Copyright (c) 2012, 2018 The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2, as published by the Free Software Foundation.
*/

#ifndef _MC_SNOOPING_H_
#define _MC_SNOOPING_H_

#include "mc_private2.h"
#include "hyfi_bridge.h"

int __init mc_snooping_init(void);
void mc_snooping_exit(void);
int mc_open(struct hyfi_net_bridge *hyfi_br, struct mc_struct *mc);
int mc_stop(struct mc_struct *mc);
int mc_rcv(struct mc_struct *mc, struct sk_buff *skb,
        struct net_bridge_fdb_entry *fdb, const struct net_bridge_port *port);
int mc_find_acl_rule(struct mc_acl_rule_table *acl, __be32 in4, void *in6, __u8 *mac, __be32 rule);
int mc_group_hash(__be32 mdb_salt, __be32 group);
struct mc_mdb_entry *mc_mdb_find(struct hlist_head *head, struct mc_ip *group);
int mc_attach(struct hyfi_net_bridge *hyfi_br);
void mc_detach(struct hyfi_net_bridge *hyfi_br);

extern void mc_nbp_change(struct hyfi_net_bridge *hyfi_br, struct net_bridge_port *p, int event);
extern void mc_fdb_change(struct hyfi_net_bridge *hyfi_br, __u8 *mac, int change);

#endif


