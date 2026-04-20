/*
 ******************************************************************************
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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
 * ****************************************************************************
 */

#ifndef _NSS_CONNMGR_TUNIPIP6_STATS_H_
#define _NSS_CONNMGR_TUNIPIP6_STATS_H_

struct nss_tunipip6_instance;

/*
 * tunipip6 statistic counters
 */
enum nss_tunipip6_stats_type {
	NSS_TUNIPIP6_STATS_RX_PKTS,
	NSS_TUNIPIP6_STATS_RX_BYTES,
	NSS_TUNIPIP6_STATS_TX_PKTS,
	NSS_TUNIPIP6_STATS_TX_BYTES,
	NSS_TUNIPIP6_STATS_RX_QUEUE_0_DROPPED,
	NSS_TUNIPIP6_STATS_RX_QUEUE_1_DROPPED,
	NSS_TUNIPIP6_STATS_RX_QUEUE_2_DROPPED,
	NSS_TUNIPIP6_STATS_RX_QUEUE_3_DROPPED,
	NSS_TUNIPIP6_STATS_EXCEP_ENCAP_LOW_HEADROOM,
	NSS_TUNIPIP6_STATS_EXCEP_ENCAP_UNHANDLED_PROTOCOL,
	NSS_TUNIPIP6_STATS_DROP_ENCAP_ENQUEUE_FAIL,
	NSS_TUNIPIP6_STATS_CONFIG_ERR_TUNNEL,
	NSS_TUNIPIP6_STATS_CONFIG_ENCAP_TOTAL_FMR,
	NSS_TUNIPIP6_STATS_CONFIG_ENCAP_FMR_ADD,
	NSS_TUNIPIP6_STATS_CONFIG_ENCAP_FMR_DEL,
	NSS_TUNIPIP6_STATS_CONFIG_ENCAP_FMR_FLUSH,
	NSS_TUNIPIP6_STATS_CONFIG_ENCAP_FMR_UPDATE,
	NSS_TUNIPIP6_STATS_CONFIG_ENCAP_FMR_ADD_FAIL,
	NSS_TUNIPIP6_STATS_CONFIG_ENCAP_FMR_DEL_FAIL,
	NSS_TUNIPIP6_STATS_CONFIG_ENCAP_ERR_NO_FMR,
	NSS_TUNIPIP6_STATS_CONFIG_ENCAP_BMR_ADD,
	NSS_TUNIPIP6_STATS_CONFIG_ENCAP_BMR_DEL,
	NSS_TUNIPIP6_STATS_CONFIG_ENCAP_ERR_BMR_EXIST,
	NSS_TUNIPIP6_STATS_CONFIG_ENCAP_ERR_NO_BMR,
	NSS_TUNIPIP6_STATS_DROP_DECAP_ENQUEUE_FAIL,
	NSS_TUNIPIP6_STATS_MAX,
};

/*
 * tunipip6 statistics
 */
struct nss_tunipip6_stats {
	uint64_t inner_stats[NSS_TUNIPIP6_STATS_MAX];
	uint64_t outer_stats[NSS_TUNIPIP6_STATS_MAX];
};

/*
 * tunipip6 statistics API
 */
extern void nss_tunipip6_stats_sync(struct net_device *dev, struct nss_tunipip6_msg *ntm);
extern void nss_tunipip6_stats_dentry_deinit(void);
extern bool nss_tunipip6_stats_dentry_init(void);
extern void nss_tunipip6_stats_dentry_destroy(struct nss_tunipip6_instance *tun_inst);
extern bool nss_tunipip6_stats_dentry_create(struct nss_tunipip6_instance *tun_inst);

#endif /* _NSS_CONNMGR_TUNIPIP6_STATS_H_ */
