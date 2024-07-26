/*
 * Copyright (c) 2022, 2024 Qualcomm Innovation Center, Inc. All rights reserved.
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
 */
#include <linux/version.h>

struct ppe_vp;
struct ppe_vp_base;

/*
 * ppe_vp_base_stats
 *	PPE VP base Statistics
 */
struct ppe_vp_base_stats {
	atomic64_t vp_allocation_fails;		/* Total VP allocation failures */
	atomic64_t vp_table_full;		/* VP allocation fails due to table full */
	atomic64_t mtu_assign_fails;		/* MTU assign fails */
	atomic64_t mac_assign_fails;		/* MAC assign fails */
	atomic64_t rx_dvp_inactive;		/* Packet received from PPE with inactive destinaton VP */
	atomic64_t rx_svp_inactive;		/* Packet received from PPE with inactive Source VP */
	atomic64_t rx_dvp_invalid;		/* Packet received from PPE without valid DVP */
	atomic64_t rx_svp_invalid;		/* Packet received from PPE without valid SVP */
	atomic64_t tx_vp_inactive;		/* VP of Packet forwarded by VP user is inactive */
	atomic64_t rx_fastxmit_fails;		/* Rx packet fast transmit failed */
	atomic64_t rx_dvp_no_listcb;		/* list handler not registered for list destination VP */
};

/*
 * ppe_vp_misc_info
 *	PPE VP misc Statistics
 */
struct ppe_vp_misc_info {
	uint32_t netdev_if_num;         	/* Net device interface number */
	uint8_t ppe_port_num;          	/* PPE Port number */
};

/*
 * ppe_vp_rx_stats
 *	PPE VP Rx Statistics
 */
struct ppe_vp_rx_stats {
	uint64_t rx_pkts;			/* Total rx packets */
	uint64_t rx_bytes;			/* Total rx bytes */
	uint64_t rx_excp_pkts;		/* Total exceptioned VP packets */
	uint64_t rx_excp_bytes;		/* Total exceptioned VP bytes */
	uint64_t rx_errors;			/* Total rx errors */
	uint64_t rx_drops;			/* Total rx drops */
	uint64_t rx_dev_not_up;			/* Received packets before dev IFF_UP */
	struct u64_stats_sync syncp;		/* Stats sync status */
};

/*
 * ppe_vp_tx_stats
 *	PPE VP Tx Statistics
 */
struct ppe_vp_tx_stats {
	uint64_t tx_pkts;			/* Total tx packets */
	uint64_t tx_bytes;			/* Total tx bytes */
	uint64_t tx_errors;			/* Total tx errors */
	uint64_t tx_drops;			/* Total tx drops */
	struct u64_stats_sync syncp;		/* Stats sync status */
};

/*
 * ppe_vp_stats
 *	Structure for VP Per CPU stats
 */
struct ppe_vp_stats {
	ppe_vp_hw_stats_t vp_hw_stats;	/* HW port statistics */
	struct ppe_vp_misc_info misc_info;	/* Misc statistics */
	struct ppe_vp_rx_stats __percpu *rx_stats;
						/* VP Rx statistics */
	struct ppe_vp_tx_stats __percpu *tx_stats;
						/* VP Tx statistics */
};

/*
 * ppe_vp_stats_fetch_begin()
 *	Fetch vp stats
 */
static inline unsigned int ppe_vp_stats_fetch_begin(const struct u64_stats_sync *syncp)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
	return u64_stats_fetch_begin_irq(syncp);
#else
	return u64_stats_fetch_begin(syncp);
#endif
}

/*
 * ppe_vp_stats_fetch_retry()
 *	Retry fetching vp stats
 */
static inline bool ppe_vp_stats_fetch_retry(const struct u64_stats_sync *syncp, unsigned int start)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
	return u64_stats_fetch_retry_irq(syncp, start);
#else
	return u64_stats_fetch_retry(syncp, start);
#endif

}

extern void ppe_vp_stats_reset_vp_stats(struct ppe_vp_stats *vp_stats);
extern ppe_vp_status_t ppe_vp_stats_deinit(struct ppe_vp *vp);
extern ppe_vp_status_t ppe_vp_stats_init(struct ppe_vp *vp);
extern ppe_vp_status_t ppe_vp_base_stats_init(struct ppe_vp_base *pvb);
