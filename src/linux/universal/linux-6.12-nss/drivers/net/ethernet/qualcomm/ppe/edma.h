/* SPDX-License-Identifier: GPL-2.0-only
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#ifndef __EDMA_MAIN__
#define __EDMA_MAIN__

#include "ppe_config.h"
#include "edma_rx.h"
#include "edma_tx.h"

/* One clock cycle = 1/(EDMA clock frequency in Mhz) micro seconds.
 *
 * One timer unit is 128 clock cycles.
 *
 * So, therefore the microsecond to timer unit calculation is:
 * Timer unit = time in microseconds / (one clock cycle in microsecond * cycles in 1 timer unit)
 *            = ('x' microsecond * EDMA clock frequency in MHz ('y') / 128).
 *
 */
#define EDMA_CYCLE_PER_TIMER_UNIT	128
#define EDMA_MICROSEC_TO_TIMER_UNIT(x, y)	((x) * (y) / EDMA_CYCLE_PER_TIMER_UNIT)
#define MHZ			1000000UL

/* EDMA profile ID. */
#define EDMA_CPU_PORT_PROFILE_ID  0

/* Number of PPE queue priorities supported per ARM core. */
#define EDMA_PRI_MAX_PER_CORE	8

/* Interface ID start. */
#define EDMA_START_IFNUM   1

#define EDMA_DESC_AVAIL_COUNT(head, tail, _max) ({ \
			typeof(_max) (max) = (_max); \
			((((head) - (tail)) + \
			(max)) & ((max) - 1)); })

/**
 * enum ppe_queue_class_type - PPE queue class type
 * @PPE_QUEUE_CLASS_PRIORITY: Queue offset configured from internal priority
 * @PPE_QUEUE_CLASS_HASH: Queue offset configured from RSS hash.
 */
enum ppe_queue_class_type {
	PPE_QUEUE_CLASS_PRIORITY,
	PPE_QUEUE_CLASS_HASH,
};

/**
 * struct edma_err_stats - EDMA error stats
 * @edma_axi_read_err: AXI read error
 * @edma_axi_write_err: AXI write error
 * @edma_rxdesc_fifo_full: Rx desc FIFO full error
 * @edma_rx_buf_size_err: Rx buffer size too small error
 * @edma_tx_sram_full: Tx packet SRAM buffer full error
 * @edma_tx_data_len_err: Tx data length error
 * @edma_tx_timeout: Tx timeout error
 * @edma_txcmpl_buf_full: Tx completion buffer full error
 * @syncp: Synchronization pointer
 */
struct edma_err_stats {
	u64 edma_axi_read_err;
	u64 edma_axi_write_err;
	u64 edma_rxdesc_fifo_full;
	u64 edma_rx_buf_size_err;
	u64 edma_tx_sram_full;
	u64 edma_tx_data_len_err;
	u64 edma_tx_timeout;
	u64 edma_txcmpl_buf_full;
	struct u64_stats_sync syncp;
};

/**
 * struct edma_ring_info - EDMA ring data structure.
 * @max_rings: Maximum number of rings
 * @ring_start: Ring start ID
 * @num_rings: Number of rings
 */
struct edma_ring_info {
	u32 max_rings;
	u32 ring_start;
	u32 num_rings;
};

/**
 * struct edma_hw_info - EDMA hardware data structure.
 * @rxfill: Rx Fill ring information
 * @rx: Rx Desc ring information
 * @tx: Tx Desc ring information
 * @txcmpl: Tx complete ring information
 * @max_ports: Maximum number of ports
 * @napi_budget_rx: Rx NAPI budget
 * @napi_budget_tx: Tx NAPI budget
 */
struct edma_hw_info {
	struct edma_ring_info *rxfill;
	struct edma_ring_info *rx;
	struct edma_ring_info *tx;
	struct edma_ring_info *txcmpl;
	u32 max_ports;
	u32 napi_budget_rx;
	u32 napi_budget_tx;
};

/**
 * struct edma_intr_info - EDMA interrupt data structure.
 * @intr_mask_rx: RX interrupt mask
 * @intr_rx: Rx interrupts
 * @intr_mask_txcmpl: Tx completion interrupt mask
 * @intr_txcmpl: Tx completion interrupts
 * @intr_mask_misc: Miscellaneous interrupt mask
 * @intr_misc: Miscellaneous interrupts
 */
struct edma_intr_info {
	u32 intr_mask_rx;
	u32 *intr_rx;
	u32 intr_mask_txcmpl;
	u32 *intr_txcmpl;
	u32 intr_mask_misc;
	u32 intr_misc;
};

/**
 * struct edma_context - EDMA context.
 * @netdev_arr: Net device for each EDMA port
 * @dummy_dev: Dummy netdevice for RX DMA
 * @ppe_dev: PPE device
 * @hw_info: EDMA Hardware info
 * @intr_info: EDMA Interrupt info
 * @rxfill_rings: Rx fill Rings, SW is producer
 * @rx_rings: Rx Desc Rings, SW is consumer
 * @tx_rings: Tx Descriptor Ring, SW is producer
 * @txcmpl_rings: Tx complete Ring, SW is consumer
 * @err_stats: Per CPU error statistics
 * @rx_rps_ctl_table_hdr: Rx RPS sysctl table
 * @rx_page_mode: Page mode enabled or disabled
 * @rx_buf_size: Rx buffer size for Jumbo MRU
 * @tx_requeue_stop: Tx requeue stop enabled or disabled
 */
struct edma_context {
	struct net_device **netdev_arr;
	struct net_device *dummy_dev;
	struct ppe_device *ppe_dev;
	struct edma_hw_info *hw_info;
	struct edma_intr_info intr_info;
	struct edma_rxfill_ring *rxfill_rings;
	struct edma_rxdesc_ring *rx_rings;
	struct edma_txdesc_ring *tx_rings;
	struct edma_txcmpl_ring *txcmpl_rings;
	struct edma_err_stats __percpu *err_stats;
	struct ctl_table_header *rx_rps_ctl_table_hdr;
	u32 rx_page_mode;
	u32 rx_buf_size;
	bool tx_requeue_stop;
};

/* Global EDMA context */
extern struct edma_context *edma_ctx;

int edma_err_stats_alloc(void);
void edma_err_stats_free(void);
void edma_destroy(struct ppe_device *ppe_dev);
int edma_setup(struct ppe_device *ppe_dev);
void edma_debugfs_teardown(void);
int edma_debugfs_setup(struct ppe_device *ppe_dev);
void edma_set_ethtool_ops(struct net_device *netdev);
int ppe_edma_queue_offset_config(struct ppe_device *ppe_dev,
				 enum ppe_queue_class_type class,
				 int index, int queue_offset);
int ppe_edma_queue_resource_get(struct ppe_device *ppe_dev, int type,
				int *res_start, int *res_end);
int ppe_edma_ring_to_queues_config(struct ppe_device *ppe_dev, int ring_id,
				   int num, int queues[]);


#endif
