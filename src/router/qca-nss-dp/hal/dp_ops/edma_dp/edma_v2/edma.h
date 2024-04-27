/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 *
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef __EDMA_H__
#define __EDMA_H__

#include <fal/fal_qm.h>
#include <fal/fal_qos.h>
#include <linux/netdevice.h>
#include <nss_dp_arch.h>
#include <nss_dp_api_if.h>
#include <nss_dp_hal_if.h>
#include <ppe_drv.h>
#include "edma_rx.h"
#include "edma_tx.h"
#ifdef NSS_DP_PPEDS_SUPPORT
#include "edma_ppeds_priv.h"
#endif

/*
 * The driver uses kernel DMA constructs that assume an architecture
 * where the view of physical addresses is consistent between SoC and
 * IO device(EDMA).
 * Note that this may not be compatible for platforms where this
 * assumption is not true, for example IO devices with IOMMU support.
 */
#if defined(CONFIG_ARM_SMMU) || \
	defined(CONFIG_IOMMU_SUPPORT)
#error "Build Error: Platform is enabled with IOMMU/SMMU support."
#endif

#define EDMA_HW_RESET_ID		"edma_rst"
#define EDMA_DEVICE_NODE_NAME		"edma"
#define EDMA_START_GMACS		NSS_DP_HAL_START_IFNUM
#define EDMA_MAX_GMACS			NSS_DP_HAL_MAX_PORTS
#define EDMA_MAX_PORTS			NSS_DP_MAX_PORTS
#define EDMA_IRQ_NAME_SIZE		32
#define EDMA_SC_BYPASS			1
#define EDMA_NETDEV_FEATURES		NETIF_F_FRAGLIST \
					| NETIF_F_SG \
					| NETIF_F_RXCSUM \
					| NETIF_F_HW_CSUM \
					| NETIF_F_TSO \
					| NETIF_F_TSO6

#define EDMA_SWITCH_DEV_ID	0
#define EDMA_PPE_QUEUE_LEVEL	0
#define EDMA_BITS_IN_WORD	32

/*
 * Maximum queue priority
 */
#define EDMA_PRI_MAX		16

/*
 * Maximum queue priority supported per core
 */
#define EDMA_MAX_PRI_PER_CORE	8

/*
 * Bitmap for ring to PPE queue's mapping.
 *
 * A bitmap for 300 PPE queues requires 10 32bit integers
 */
#define EDMA_RING_MAPPED_QUEUE_BM_WORD_COUNT	10

/*
 * QID to RID Table
 */
#define EDMA_QID2RID_TABLE_MEM(q)	(0xb9000 + (0x4 * (q)))

/*
 * edma_port_ucast_queues
 * 	EDMA unicast queue number
 * To-do: read queue start from dtsi
 */
enum edma_port_ucast_queues {
	EDMA_CPU_PORT_QUEUE_START = 0,
	EDMA_CPU_PORT_QUEUE_MAX = 31,
};

/*
 * edma_cpu_port_mcast_queues
 *	EDMA multicast queue number
 */
enum edma_cpu_port_mcast_queues {
	EDMA_CPU_PORT_MCAST_QUEUE_START = 256,
	EDMA_CPU_PORT_MCAST_QUEUE_END = 271,
};

/*
 * EDMA profile ID
 *
 * To-do: Use enum once ppe-drv introduces profile id enum.
 */
#define EDMA_CPU_PORT_PROFILE_ID	0

/*
 * EDMA maximum RSS hash
 */
#define EDMA_RSS_HASH_MAX	256

/*
 * EDMA QID2RID configuration
 */
#define EDMA_QID2RID_NUM_PER_REG	4

/*
 * EDMA clock frequency: 352 MHZ
 * So, one clock cycle = (1/352) micro seconds
 *
 * One timer unit is 128 clock cycles.
 *
 * So, therefore the microsecond to timer unit calculation is:
 * Timer unit	= time in microseconds / (one clock cycle in microsecond * cycles in 1 timer unit)
 * 		= ('x' microsecond * 352 / 128)
 */
#define EDMA_CLK_FREQ		352
#define CYCLE_PER_TIMER_UNIT	128
#define MICROSEC_TO_TIMER_UNIT(x)	(((x) * EDMA_CLK_FREQ) / CYCLE_PER_TIMER_UNIT)

#define EDMA_DESC_AVAIL_COUNT(head, tail, max) (((head) - (tail)) + (max)) & ((max) - 1)

/*
 * EDMA MISC status get macros
 */
#define EDMA_MISC_AXI_RD_ERR_STATUS_GET(x)	((x) & EDMA_MISC_AXI_RD_ERR_MASK)
#define EDMA_MISC_AXI_WR_ERR_STATUS_GET(x)	(((x) & EDMA_MISC_AXI_WR_ERR_MASK) >> 1)
#define EDMA_MISC_RX_DESC_FIFO_FULL_STATUS_GET(x)	(((x) & EDMA_MISC_RX_DESC_FIFO_FULL_MASK) >> 2)
#define EDMA_MISC_RX_ERR_BUF_SIZE_STATUS_GET(x)		(((x) & EDMA_MISC_RX_ERR_BUF_SIZE_MASK) >> 3)
#define EDMA_MISC_TX_SRAM_FULL_STATUS_GET(x)		(((x) & EDMA_MISC_TX_SRAM_FULL_MASK) >> 4)
#define EDMA_MISC_TX_CMPL_BUF_FULL_STATUS_GET(x)		(((x) & EDMA_MISC_TX_CMPL_BUF_FULL_MASK) >> 5)
#define EDMA_MISC_DATA_LEN_ERR_STATUS_GET(x)		(((x) & EDMA_MISC_DATA_LEN_ERR_MASK) >> 6)
#define EDMA_MISC_TX_TIMEOUT_STATUS_GET(x)		(((x) & EDMA_MISC_TX_TIMEOUT_MASK) >> 7)

/*
 * edma_misc_stats
 *	EDMA miscellaneous stats
 */
struct edma_misc_stats {
	uint64_t edma_misc_axi_read_err;		/* AXI read error */
	uint64_t edma_misc_axi_write_err;		/* AXI write error */
	uint64_t edma_misc_rx_desc_fifo_full;		/* Rx descriptor FIFO full error */
	uint64_t edma_misc_rx_buf_size_err;		/* Rx buffer size too small error */
	uint64_t edma_misc_tx_sram_full;		/* Tx packet SRAM buffer full error */
	uint64_t edma_misc_tx_data_len_err;		/* Tx data length error */
	uint64_t edma_misc_tx_timeout;			/* Tx timeout error */
	uint64_t edma_misc_tx_cmpl_buf_full;		/* Tx completion buffer full error */
	struct u64_stats_sync syncp;			/* Synchronization pointer */
};

/*
 * edma_sawf_sc_stats
 *	EDMA per-service code stats
 */
struct edma_sawf_sc_stats {
	uint64_t rx_packets;		/* Per service code counter for packets recieved on queues from PPE */
	uint64_t rx_bytes;		/* Per service code counter for bytes recieved on queues from PPE */
	struct u64_stats_sync syncp;	/* Synchronization pointer */
};

/*
 * edma_pcpu_stats
 *	EDMA per cpu stats data structure
 */
struct edma_pcpu_stats {
	struct edma_rx_stats __percpu *rx_stats;
			/* Per CPU Rx statistics */
	struct edma_tx_stats __percpu *tx_stats;
			/* Per CPU Tx statistics */
};

/*
 * EDMA private data structure
 */
struct edma_gbl_ctx {
	struct net_device *netdev_arr[EDMA_MAX_PORTS];
			/* Net device for each GMAC port */
	struct device_node *device_node;
			/* Device tree node */
	struct platform_device *pdev;
			/* Platform device */
	void __iomem *reg_base;
			/* EDMA base register mapped address */
	struct resource *reg_resource;
			/* Memory resource */
	atomic_t active_port_count;
			/* Count of active number of ports */
	bool napi_added;
			/* NAPI flag */

	struct ctl_table_header *ctl_table_hdr;
			/* sysctl table entry */

	struct edma_rxfill_ring *rxfill_rings;
			/* Rx Fill Rings, SW is producer */
	struct edma_rxdesc_ring *rxdesc_rings;
			/* Rx Descriptor Rings, SW is consumer */
	struct edma_txdesc_ring *txdesc_rings;
			/* Tx Descriptor Ring, SW is producer */
	struct edma_txcmpl_ring *txcmpl_rings;
			/* Tx complete Ring, SW is consumer */

	uint32_t rxfill_ring_map[EDMA_RXFILL_RING_PER_CORE_MAX][NR_CPUS];
			/* Rx Fill ring per-core mapping from device tree */
	uint32_t rxdesc_ring_map[EDMA_RXDESC_RING_PER_CORE_MAX][NR_CPUS];
			/* Rx Descriptor ring per-core mapping from device tree */
	uint32_t (*rxdesc_ring_to_queue_bm)[EDMA_RING_MAPPED_QUEUE_BM_WORD_COUNT];
			/* Bitmap of mapped PPE queue ids of the Rx descriptor rings */
	int32_t tx_to_txcmpl_map[EDMA_MAX_TXDESC_RINGS];
			/* Tx ring to Tx complete ring mapping */
	int32_t tx_map[EDMA_TX_RING_PER_CORE_MAX][NR_CPUS];
			/* Per core Tx ring to core mapping */
	int32_t tx_fc_grp_map[EDMA_MAX_GMACS];
			/* Per GMAC TxDesc ring to flow control group mapping */
	int32_t txcmpl_map[EDMA_TXCMPL_RING_PER_CORE_MAX][NR_CPUS];
			/* Tx complete ring to core mapping */

	struct dentry *root_dentry;	/* Root debugfs entry */
	struct dentry *stats_dentry;	/* Statistics debugfs entry */

	struct edma_misc_stats __percpu *misc_stats;
			/* Per CPU miscellaneous statistics */
	struct edma_sawf_sc_stats sawf_sc_stats[PPE_DRV_SAWF_SC_MAX];
			/* Per service class stats */

	uint32_t tx_priority_level;
			/* Tx priority level per port */
	uint32_t rx_priority_level;
			/* Rx priority level per core */
	uint32_t num_txdesc_rings;
			/* Number of TxDesc rings */
	uint32_t txdesc_ring_start;
			/* Id of first TXDESC ring */
	uint32_t txdesc_ring_end;
			/* Id of the last TXDESC ring */
	uint32_t num_txcmpl_rings;
			/* Number of TxCmpl rings */
	uint32_t txcmpl_ring_start;
			/* Id of first TXCMPL ring */
	uint32_t txcmpl_ring_end;
			/* Id of last TXCMPL ring */
	uint32_t num_rxfill_rings;
			/* Number of RxFill rings */
	uint32_t rxfill_ring_start;
			/* Id of first RxFill ring */
	uint32_t rxfill_ring_end;
			/* Id of last RxFill ring */
	uint32_t num_rxdesc_rings;
			/* Number of RxDesc rings */
	uint32_t rxdesc_ring_start;
			/* Id of first RxDesc ring */
	uint32_t rxdesc_ring_end;
			/* Id of last RxDesc ring */
	uint32_t txcmpl_intr[EDMA_MAX_TXCMPL_RINGS];
			/* TxCmpl ring IRQ numbers */
	uint32_t rxfill_intr[EDMA_MAX_RXFILL_RINGS];
			/* Rx fill ring IRQ numbers */
	uint32_t rxdesc_intr[EDMA_MAX_RXDESC_RINGS];
			/* Rx desc ring IRQ numbers */
	uint32_t misc_intr;
			/* Misc IRQ number */

	uint32_t rxfill_intr_mask;
			/* Rx fill ring interrupt mask */
	uint32_t rxdesc_intr_mask;
			/* Rx Desc ring interrupt mask */
	uint32_t txcmpl_intr_mask;
			/* Tx Cmpl ring interrupt mask */
	uint32_t misc_intr_mask;
			/* Misc interrupt interrupt mask */
	uint32_t dp_override_cnt;
			/* Number of interfaces overriden */
	uint32_t rx_page_mode;
			/* Page mode enabled or disabled */
	uint32_t rx_jumbo_mru;
			/* Jumbo MRU value */
#if defined(NSS_DP_POINT_OFFLOAD)
	uint32_t txdesc_point_offload_ring;
			/* TX desc ring for point offlaod */
	uint32_t txcmpl_point_offload_ring;
			/* TX completion ring for point offlaod */
	uint32_t rxfill_point_offload_ring;
			/* RX fill ring for point offload */
	uint32_t rxdesc_point_offload_ring;
			/* RX desc ring for point offload */
	uint32_t rxdesc_point_offload_ring_to_queue_bm[EDMA_RING_MAPPED_QUEUE_BM_WORD_COUNT];
			/* PPE queue ids of the Rx descriptor point offload rings */
	uint16_t point_offload_queue;
			/* Point offload base queue id */
#endif
	bool edma_initialized;
			/* Flag to check initialization status */
	uint32_t rx_ring_queue_map[EDMA_MAX_PRI_PER_CORE][NR_CPUS];
			/* Rx ring to queue mapping */
#ifdef NSS_DP_PPEDS_SUPPORT
	uint32_t ppeds_node_map[EDMA_PPEDS_MAX_NODES][EDMA_PPEDS_NUM_ENTRY];
	struct edma_ppeds_drv ppeds_drv;
			/* PPE-DS nodes information */
#endif
	uint8_t rx_queue_start;
			/* Rx queue start */
};

extern struct edma_gbl_ctx edma_gbl_ctx;

int edma_irq_init(void);
irqreturn_t edma_misc_handle_irq(int irq, void *ctx);
int32_t edma_misc_stats_alloc(void);
void edma_misc_stats_free(void);
void edma_enable_interrupts(struct edma_gbl_ctx *egc);
void edma_disable_interrupts(struct edma_gbl_ctx *egc);
void edma_configure_rps_hash_map(struct edma_gbl_ctx *egc);

/*
 * edma_reg_read()
 *	Read EDMA register
 */
static inline uint32_t edma_reg_read(uint32_t reg_off)
{
	return hal_read_reg(edma_gbl_ctx.reg_base, reg_off);
}

/*
 * edma_reg_write()
 *	Write EDMA register
 */
static inline void edma_reg_write(uint32_t reg_off, uint32_t val)
{
	hal_write_reg(edma_gbl_ctx.reg_base, reg_off, val);
}

#endif	/* __EDMA_H__ */
