/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2015-2018 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
 */

#ifndef _WG_ALLOWEDIPS_H
#define _WG_ALLOWEDIPS_H

#include <linux/mutex.h>
#include <linux/ip.h>
#include <linux/ipv6.h>

static struct wg_peer;
static struct allowedips_node;

struct allowedips {
	struct allowedips_node __rcu *root4;
	struct allowedips_node __rcu *root6;
	u64 seq;
};

struct allowedips_cursor {
	u64 seq;
	struct allowedips_node *stack[128];
	unsigned int len;
	bool second_half;
};

static void wg_allowedips_init(struct allowedips *table);
static void wg_allowedips_free(struct allowedips *table, struct mutex *mutex);
static int wg_allowedips_insert_v4(struct allowedips *table, const struct in_addr *ip,
			    u8 cidr, struct wg_peer *peer, struct mutex *lock);
static int wg_allowedips_insert_v6(struct allowedips *table, const struct in6_addr *ip,
			    u8 cidr, struct wg_peer *peer, struct mutex *lock);
static void wg_allowedips_remove_by_peer(struct allowedips *table,
				  struct wg_peer *peer, struct mutex *lock);
static int wg_allowedips_walk_by_peer(struct allowedips *table,
			       struct allowedips_cursor *cursor,
			       struct wg_peer *peer,
			       int (*func)(void *ctx, const u8 *ip, u8 cidr,
					   int family),
			       void *ctx, struct mutex *lock);

/* These return a strong reference to a peer: */
static struct wg_peer *wg_allowedips_lookup_dst(struct allowedips *table,
						struct sk_buff *skb);
static struct wg_peer *wg_allowedips_lookup_src(struct allowedips *table,
						struct sk_buff *skb);

#ifdef DEBUG
static bool wg_allowedips_selftest(void);
#endif

#endif /* _WG_ALLOWEDIPS_H */
