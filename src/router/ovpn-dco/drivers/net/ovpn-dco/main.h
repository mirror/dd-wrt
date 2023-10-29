/* SPDX-License-Identifier: GPL-2.0-only */
/*  OpenVPN data channel accelerator
 *
 *  Copyright (C) 2019-2023 OpenVPN, Inc.
 *
 *  Author:	James Yonan <james@openvpn.net>
 *		Antonio Quartulli <antonio@openvpn.net>
 */

#ifndef _NET_OVPN_DCO_MAIN_H_
#define _NET_OVPN_DCO_MAIN_H_

#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/printk.h>
#include <linux/udp.h>

#ifndef OVPN_DCO_VERSION
#define OVPN_DCO_VERSION "2.0.0"
#endif

/* Our UDP encapsulation types, must be unique
 * (other values in include/uapi/linux/udp.h)
 */
#define UDP_ENCAP_OVPNINUDP 100  /* transport layer */

struct net_device;
bool ovpn_dev_is_valid(const struct net_device *dev);

#define SKB_HEADER_LEN                                       \
	(max(sizeof(struct iphdr), sizeof(struct ipv6hdr)) + \
	 sizeof(struct udphdr) + NET_SKB_PAD)

#define OVPN_HEAD_ROOM ALIGN(16 + SKB_HEADER_LEN, 4)
#define OVPN_MAX_PADDING 16
#define OVPN_QUEUE_LEN 1024
#define OVPN_MAX_TUN_QUEUE_LEN 0x10000

#endif /* _NET_OVPN_DCO_OVPN_DCO_H_ */
