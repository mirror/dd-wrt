/* SPDX-License-Identifier: GPL-2.0-only */
/*  OpenVPN data channel accelerator
 *
 *  Copyright (C) 2020-2023 OpenVPN, Inc.
 *
 *  Author:	James Yonan <james@openvpn.net>
 *		Antonio Quartulli <antonio@openvpn.net>
 *		Lev Stipakov <lev@openvpn.net>
 */

#ifndef _NET_OVPN_DCO_OVPNSTATS_H_
#define _NET_OVPN_DCO_OVPNSTATS_H_

#include <linux/atomic.h>
#include <linux/jiffies.h>

struct ovpn_struct;

/* per-peer stats, measured on transport layer */

/* one stat */
struct ovpn_peer_stat {
	atomic64_t bytes;
	atomic_t packets;
};

/* rx and tx stats, enabled by notify_per != 0 or period != 0 */
struct ovpn_peer_stats {
	struct ovpn_peer_stat rx;
	struct ovpn_peer_stat tx;
};

/* struct for OVPN_ERR_STATS */

struct ovpn_err_stat {
	unsigned int category;
	int errcode;
	u64 count;
};

struct ovpn_err_stats {
	/* total stats, returned by kovpn */
	unsigned int total_stats;
	/* number of stats dimensioned below */
	unsigned int n_stats;
	struct ovpn_err_stat stats[];
};

void ovpn_peer_stats_init(struct ovpn_peer_stats *ps);

static inline void ovpn_peer_stats_increment(struct ovpn_peer_stat *stat, const unsigned int n)
{
	atomic64_add(n, &stat->bytes);
	atomic_inc(&stat->packets);
}

static inline void ovpn_peer_stats_increment_rx(struct ovpn_peer_stats *stats, const unsigned int n)
{
	ovpn_peer_stats_increment(&stats->rx, n);
}

static inline void ovpn_peer_stats_increment_tx(struct ovpn_peer_stats *stats, const unsigned int n)
{
	ovpn_peer_stats_increment(&stats->tx, n);
}

#endif /* _NET_OVPN_DCO_OVPNSTATS_H_ */
