/*
 *  ddtb.h - shameless reimplementation of broadcom ctf nat acceleration driver
 *
 *  Copyright (C) 2013-2014 <to be filled in>
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License version
 *	2 as published by the Free Software Foundation.
 *
 *  debugfs is for people to use instead of /proc or /sys.
 *  See Documentation/DocBook/filesystems for more details.
 */

#ifndef _DDTB_H
#define _DDTB_H

#include <linux/if.h>
#include <linux/if_vlan.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>

#define DDTB_F_SNAT		BIT(0)
#define DDTB_F_DNAT		BIT(1)
#define DDTB_F_REQUEST	BIT(2)
#define DDTB_F_REPLY	BIT(3)

#ifdef CONFIG_IPV6_1
#define IPADDR_U32_SZ		(16 / sizeof(u32))
#else
#define IPADDR_U32_SZ		1
#endif

#define pr_ddtb(fmt, ...)	do { \
		pr_info("ddtb: "fmt, ## __VA_ARGS__); \
	} while (0)

struct ddtb_tuple {
	struct net_device	*dev;
	u16			proto;
	u32			sip[IPADDR_U32_SZ];
	u32			dip[IPADDR_U32_SZ];
	u16			sp;
	u16			dp;
	s16			vid;
	u8			mac[6];
} __packed __aligned(4);

struct ddtb_dir {
	struct hlist_node	h;
	struct ddtb_tuple	tuple;
	u16			flags;
};

struct ddtb_conn {
	struct ddtb_dir		request;
	struct ddtb_dir		reply;

	u32			rx_pkts;
	u32			rx_bytes;
	u32			tx_pkts;
	u32			tx_bytes;

	struct rcu_head		rcu;
};

#ifdef CONFIG_DDTB

int ddtb_intercept(struct sk_buff *skb);
struct ddtb_conn *ddtb_ip_add(struct ddtb_conn *_c, struct net_device *orig, int v6);
int ddtb_ip_delete(struct ddtb_conn *_c, int v6);

#else

static inline int ddtb_intercept(struct sk_buff *skb)
{
	return -1;
}

static inline struct ddtb_conn *
ddtb_ip_add(struct ddtb_conn *_c, struct net_device *orig, int v6)
{
	return NULL;
}

static inline int ddtb_ip_delete(struct ddtb_conn *_c, int v6)
{
	return 0;
}

#endif

#endif
/*
 * Copyright (C) 2013, Broadcom Corporation. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * $Id: hndctf.h 418247 2013-08-14 11:16:42Z $
 */

#ifndef _DDTB_H
#define _DDTB_H

#include <linux/if.h>
#include <linux/if_vlan.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>

#define DDTB_F_SNAT		BIT(0)
#define DDTB_F_DNAT		BIT(1)
#define DDTB_F_REQUEST	BIT(2)
#define DDTB_F_REPLY	BIT(3)

#ifdef CONFIG_IPV6_1
#define IPADDR_U32_SZ		(16 / sizeof(u32))
#else
#define IPADDR_U32_SZ		1
#endif

#define pr_ddtb(fmt, ...)	do { \
		pr_info("ddtb: "fmt, ## __VA_ARGS__); \
	} while (0)

struct ddtb_tuple {
	struct net_device	*dev;
	u16			proto;
	u32			sip[IPADDR_U32_SZ];
	u32			dip[IPADDR_U32_SZ];
	u16			sp;
	u16			dp;
	s16			vid;
	u8			mac[6];
} __packed __aligned(4);

struct ddtb_dir {
	struct hlist_node	h;
	struct ddtb_tuple	tuple;
	u16			flags;
};

struct ddtb_conn {
	struct ddtb_dir		request;
	struct ddtb_dir		reply;

	u32			rx_pkts;
	u32			rx_bytes;
	u32			tx_pkts;
	u32			tx_bytes;

	struct rcu_head		rcu;
};

#ifdef CONFIG_DDTB

int ddtb_intercept(struct sk_buff *skb);
struct ddtb_conn *ddtb_ip_add(struct ddtb_conn *_c, struct net_device *orig, int v6);
int ddtb_ip_delete(struct ddtb_conn *_c, int v6);

#else

static inline int ddtb_intercept(struct sk_buff *skb)
{
	return -1;
}

static inline struct ddtb_conn *
ddtb_ip_add(struct ddtb_conn *_c, struct net_device *orig, int v6)
{
	return NULL;
}

static inline int ddtb_ip_delete(struct ddtb_conn *_c, int v6)
{
	return 0;
}

#endif

#endif
