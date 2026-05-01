/* SPDX-License-Identifier: GPL-2.0-only
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#ifndef __EDMA_PORTS__
#define __EDMA_PORTS__

#include "ppe_port.h"

#define EDMA_PORT_MAX_CORE		4

#define EDMA_NETDEV_FEATURES		(NETIF_F_FRAGLIST \
					| NETIF_F_SG \
					| NETIF_F_RXCSUM \
					| NETIF_F_HW_CSUM \
					| NETIF_F_TSO \
					| NETIF_F_TSO6)

/**
 * struct edma_port_rx_stats - EDMA RX per CPU stats for the port.
 * @rx_pkts: Number of Rx packets
 * @rx_bytes: Number of Rx bytes
 * @rx_drops: Number of Rx drops
 * @rx_nr_frag_pkts: Number of Rx nr_frags packets
 * @rx_fraglist_pkts: Number of Rx fraglist packets
 * @rx_nr_frag_headroom_err: nr_frags headroom error packets
 * @syncp: Synchronization pointer
 */
struct edma_port_rx_stats {
	u64 rx_pkts;
	u64 rx_bytes;
	u64 rx_drops;
	u64 rx_nr_frag_pkts;
	u64 rx_fraglist_pkts;
	u64 rx_nr_frag_headroom_err;
	struct u64_stats_sync syncp;
};

/**
 * struct edma_port_tx_stats - EDMA TX port per CPU stats for the port.
 * @tx_pkts: Number of Tx packets
 * @tx_bytes: Number of Tx bytes
 * @tx_drops: Number of Tx drops
 * @tx_nr_frag_pkts: Number of Tx nr_frag packets
 * @tx_fraglist_pkts: Number of Tx fraglist packets
 * @tx_fraglist_with_nr_frags_pkts:  Number of Tx packets with fraglist and nr_frags
 * @tx_tso_pkts: Number of Tx TSO packets
 * @tx_tso_drop_pkts: Number of Tx TSO drop packets
 * @tx_gso_pkts: Number of Tx GSO packets
 * @tx_gso_drop_pkts: Number of Tx GSO drop packets
 * @tx_queue_stopped: Number of Tx queue stopped packets
 * @syncp: Synchronization pointer
 */
struct edma_port_tx_stats {
	u64 tx_pkts;
	u64 tx_bytes;
	u64 tx_drops;
	u64 tx_nr_frag_pkts;
	u64 tx_fraglist_pkts;
	u64 tx_fraglist_with_nr_frags_pkts;
	u64 tx_tso_pkts;
	u64 tx_tso_drop_pkts;
	u64 tx_gso_pkts;
	u64 tx_gso_drop_pkts;
	u64 tx_queue_stopped[EDMA_PORT_MAX_CORE];
	struct u64_stats_sync syncp;
};

/**
 * struct edma_port_pcpu_stats - EDMA per cpu stats data structure for the port.
 * @rx_stats: Per CPU Rx statistics
 * @tx_stats: Per CPU Tx statistics
 */
struct edma_port_pcpu_stats {
	struct edma_port_rx_stats __percpu *rx_stats;
	struct edma_port_tx_stats __percpu *tx_stats;
};

/**
 * struct edma_port_priv - EDMA port priv structure.
 * @ppe_port: Pointer to PPE port
 * @netdev: Corresponding netdevice
 * @pcpu_stats: Per CPU netdev statistics
 * @txr_map: Tx ring per-core mapping
 * @flags: Feature flags
 */
struct edma_port_priv {
	struct ppe_port *ppe_port;
	struct net_device *netdev;
	struct edma_port_pcpu_stats pcpu_stats;
	struct edma_txdesc_ring *txr_map[EDMA_PORT_MAX_CORE];
	unsigned long flags;
};

void edma_port_destroy(struct ppe_port *port);
int edma_port_setup(struct ppe_port *port);
#endif
