/* SPDX-License-Identifier: GPL-2.0-only */
/*  OpenVPN data channel accelerator
 *
 *  Copyright (C) 2019-2023 OpenVPN, Inc.
 *
 *  Author:	James Yonan <james@openvpn.net>
 *		Antonio Quartulli <antonio@openvpn.net>
 */

#ifndef _NET_OVPN_DCO_OVPNSTRUCT_H_
#define _NET_OVPN_DCO_OVPNSTRUCT_H_

#include "peer.h"

#include <uapi/linux/ovpn_dco.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>

/* Our state per ovpn interface */
struct ovpn_struct {
	/* read-mostly objects in this section */
	struct net_device *dev;

	/* device operation mode (i.e. P2P, MP) */
	enum ovpn_mode mode;

	/* protect writing to the ovpn_struct object */
	spinlock_t lock;

	/* workqueue used to schedule crypto work that may sleep */
	struct workqueue_struct *crypto_wq;
	/* workqueue used to schedule generic event that may sleep or that need
	 * to be performed out of softirq context
	 */
	struct workqueue_struct *events_wq;

	/* list of known peers */
	struct {
		DECLARE_HASHTABLE(by_id, 12);
		DECLARE_HASHTABLE(by_transp_addr, 12);
		DECLARE_HASHTABLE(by_vpn_addr, 12);
		/* protects write access to any of the hashtables above */
		spinlock_t lock;
	} peers;

	/* for p2p mode */
	struct ovpn_peer __rcu *peer;

	unsigned int max_tun_queue_len;

	netdev_features_t set_features;

	void *security;

	u32 registered_nl_portid;
	bool registered_nl_portid_set;
};

#endif /* _NET_OVPN_DCO_OVPNSTRUCT_H_ */
