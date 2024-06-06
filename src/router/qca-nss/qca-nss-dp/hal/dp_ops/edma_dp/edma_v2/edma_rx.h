/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef __EDMA_RX_H__
#define __EDMA_RX_H__

#define EDMA_RXFILL_RING_PER_CORE_MAX	1
#define EDMA_RXDESC_RING_PER_CORE_MAX	1

#define EDMA_RX_MAX_PROCESS		32	/* Max Rx processing without
						   replenishing RxFill ring */
#define EDMA_RX_SKB_HEADROOM		128

/*
 * Helper function for generating mask for bit field in a word. This will generate a mask which will
 * enable bits from start to end(both inclusive) of the bit field in a word.
 * For ex: field A extends from 15:8 of a word. here end=15, start=8. This macro generates the mask
 * as 1111111100000000
 */
#define EDMA_RXDESC_GENMASK(end, start)	(uint32_t)((((uint64_t)1 << ((end) - (start) + 1)) - 1) << (start))

#define EDMA_GET_DESC(R, i, type)	(&(((type *)((R)->desc))[(i)]))
#define EDMA_GET_PDESC(R, i, type)	(&(((type *)((R)->pdesc))[(i)]))
#define EDMA_GET_SDESC(R, i, type)	(&(((type *)((R)->sdesc))[(i)]))
#define EDMA_RXFILL_DESC(R, i)		EDMA_GET_DESC(R, i, struct edma_rxfill_desc)
#define EDMA_RXDESC_PRI_DESC(R, i)	EDMA_GET_PDESC(R, i, struct edma_rxdesc_desc)
#define EDMA_RXDESC_SEC_DESC(R, i)	EDMA_GET_SDESC(R, i, struct edma_rxdesc_sec_desc)

/*
 * TODO - Make this a tunable parameter using module-param.
 */
#if defined(NSS_DP_MEM_PROFILE_LOW)
#define EDMA_RX_RING_SIZE		512
#elif defined(NSS_DP_MEM_PROFILE_MEDIUM)
#define EDMA_RX_RING_SIZE		1024
#else
#define EDMA_RX_RING_SIZE		4096
#endif

#define EDMA_RX_RING_SIZE_MASK		(EDMA_RX_RING_SIZE - 1)
#define EDMA_RX_RING_ID_MASK		0x1F
#define EDMA_MAX_RXDESC_RINGS		NSS_DP_EDMA_MAX_RXDESC_RINGS
#define EDMA_MAX_RXFILL_RINGS		NSS_DP_EDMA_MAX_RXFILL_RINGS
#define EDMA_RX_MAX_PRIORITY_LEVEL	1

#define EDMA_RX_PID_IPV4_MAX		0x3
#define EDMA_RX_PID_IPV6		0x4
#define EDMA_RX_PID_IS_IPV4(pid)	(!((pid) & (~EDMA_RX_PID_IPV4_MAX)))
#define EDMA_RX_PID_IS_IPV6(pid)	(!(!((pid) & EDMA_RX_PID_IPV6)))

#define EDMA_RXDESC_BUFFER_ADDR_GET(desc)	((uint32_t)(le32_to_cpu((desc)->word0)))
#define EDMA_RXDESC_OPAQUE_GET(desc)		((uintptr_t)((uint64_t)((desc)->word2) | \
						((uint64_t)((desc)->word3) << 0x20)))
#define EDMA_RXDESC_SRCINFO_TYPE_PORTID		0x2000
#define EDMA_RXDESC_SRCINFO_TYPE_SHIFT		8
#define EDMA_RXDESC_SRCINFO_TYPE_MASK		0xF000
#define EDMA_RXDESC_L3CSUM_STATUS_MASK		0x2000
#define EDMA_RXDESC_L4CSUM_STATUS_MASK		0x1000
#define EDMA_RXDESC_PORTNUM_BITS		0x0FFF

#define EDMA_RXDESC_PACKET_LEN_MASK		0x3FFFF
#define EDMA_RXDESC_PACKET_LEN_GET(desc)	((le32_to_cpu((desc)->word5)) & \
						EDMA_RXDESC_PACKET_LEN_MASK)
#define EDMA_RXDESC_MORE_BIT_MASK		0x40000000
#define EDMA_RXDESC_MORE_BIT_GET(desc)		((le32_to_cpu((desc)->word1)) & \
						EDMA_RXDESC_MORE_BIT_MASK)
#define EDMA_RXDESC_SRC_DST_INFO_GET(desc)	((uint32_t)((le32_to_cpu((desc)->word4))))

#define EDMA_RXDESC_L3_OFFSET_SHIFT 	16
#define EDMA_RXDESC_L3_OFFSET_MASK  	EDMA_RXDESC_GENMASK(23, 16)
#define EDMA_RXDESC_L3_OFFSET_GET(desc)	((le32_to_cpu(((desc)->word7)) & EDMA_RXDESC_L3_OFFSET_MASK) >> EDMA_RXDESC_L3_OFFSET_SHIFT)

#define EDMA_RXDESC_PID_SHIFT		12
#define EDMA_RXDESC_PID_MASK		EDMA_RXDESC_GENMASK(15, 12)
#define EDMA_RXDESC_PID_GET(desc)	((le32_to_cpu(((desc)->word7)) & EDMA_RXDESC_PID_MASK) >> EDMA_RXDESC_PID_SHIFT)

#define EDMA_RXDESC_DST_INFO_SHIFT	16
#define EDMA_RXDESC_DST_INFO_MASK	EDMA_RXDESC_GENMASK(31, 16)
#define EDMA_RXDESC_DST_INFO_GET(desc)	((le32_to_cpu(((desc)->word4)) & EDMA_RXDESC_DST_INFO_MASK) >> EDMA_RXDESC_DST_INFO_SHIFT)

#define EDMA_RXDESC_SRC_INFO_SHIFT	0
#define EDMA_RXDESC_SRC_INFO_MASK	EDMA_RXDESC_GENMASK(15, 0)
#define EDMA_RXDESC_SRC_INFO_GET(desc)	((le32_to_cpu(((desc)->word4)) & EDMA_RXDESC_SRC_INFO_MASK) >> EDMA_RXDESC_SRC_INFO_SHIFT)

#define EDMA_RXDESC_PORT_ID_SHIFT	0
#define EDMA_RXDESC_PORT_ID_MASK	EDMA_RXDESC_GENMASK(11, 0)
#define EDMA_RXDESC_PORT_ID_GET(x)	(((x) & EDMA_RXDESC_PORT_ID_MASK) >> EDMA_RXDESC_PORT_ID_SHIFT)

#define EDMA_RXDESC_SRC_PORT_ID_GET(desc)	EDMA_RXDESC_PORT_ID_GET(EDMA_RXDESC_SRC_INFO_GET(desc))
#define EDMA_RXDESC_DST_PORT_ID_GET(desc)	EDMA_RXDESC_PORT_ID_GET(EDMA_RXDESC_DST_INFO_GET(desc))

#define EDMA_RXDESC_SRC_PORT_ID_MASK	EDMA_RXDESC_PORT_ID_MASK
#define EDMA_RXDESC_DST_PORT_ID_MASK	EDMA_RXDESC_PORT_ID_MASK

#define EDMA_RXDESC_DST_PORT		0x2 << EDMA_RXDESC_PID_SHIFT

/*
 * Mask for checking source or destination virtual port.
 * Note: PPE virtual port start from 64 onwards
 */
#define EDMA_RXDESC_VP_PORT_MASK	0x00c0
#define EDMA_RXDESC_SRC_DST_VP_MASK	(EDMA_RXDESC_VP_PORT_MASK | (EDMA_RXDESC_VP_PORT_MASK << 16))

#define EDMA_RXDESC_L3CSUM_STATUS_GET(desc)	(le32_to_cpu(((desc)->word6)) & \
						EDMA_RXDESC_L3CSUM_STATUS_MASK)
#define EDMA_RXDESC_L4CSUM_STATUS_GET(desc)	((le32_to_cpu((desc)->word6)) & \
						EDMA_RXDESC_L4CSUM_STATUS_MASK)

#define EDMA_RXDESC_DATA_OFFSET_MASK		EDMA_RXDESC_GENMASK(11, 0)
#define EDMA_RXDESC_DATA_OFFSET_GET(desc)	((le32_to_cpu((desc)->word6)) & \
						EDMA_RXDESC_DATA_OFFSET_MASK)

#define EDMA_RXFILL_BUF_SIZE_MASK		0xFFFF
#define EDMA_RXFILL_BUF_SIZE_SHIFT		16

/*
 * Opaque values are not accessed by the EDMA HW, so endianness conversion is not needed
 */
#define EDMA_RXFILL_OPAQUE_LO_SET(desc, ptr)	(((desc)->word2) = (uint32_t)(uintptr_t)(ptr))
#define EDMA_RXFILL_OPAQUE_HI_SET(desc, ptr)	(((desc)->word3) = (uint32_t)((uint64_t)(ptr) >> 0x20))
#define EDMA_RXFILL_OPAQUE_GET(desc)		((uintptr_t)((uint64_t)((desc)->word2) | \
						((uint64_t)((desc)->word3) << 0x20)))

#define EDMA_RXFILL_PACKET_LEN_SET(desc, len)	{ \
	(((desc)->word1) = (uint32_t)((((uint32_t)len) << EDMA_RXFILL_BUF_SIZE_SHIFT) & 0xFFFF0000)); \
}
#define EDMA_RXFILL_BUFFER_ADDR_SET(desc, addr)	(((desc)->word0) = (uint32_t)(addr))
#define EDMA_RXDESC_SC_CC_VALID_GET(desc)	((le32_to_cpu((desc)->word1)) & 0x01FF1000)
#define EDMA_RXDESC_CPU_CODE_VALID_GET(desc)	(((le32_to_cpu((desc)->word1)) & 0x00001000) >> 12)
#define EDMA_RXDESC_SERVICE_CODE_GET(desc)	(((le32_to_cpu((desc)->word1)) & 0x01FF0000) >> 16)
#define EDMA_RXDESC_CPU_CODE_GET(desc)		(((le32_to_cpu((desc)->word5)) & 0x03FF0000) >> 16)
#define EDMA_RXDESC_ACL_IDX_VALID_GET(desc)	(((le32_to_cpu((desc)->word1)) & 0x80000000) >> 31)
#define EDMA_RXDESC_ACL_IDX_GET(desc)		(((le32_to_cpu((desc)->word1)) & 0x3FFF0000) >> 16)

/*
 * Extracting Tree ID and WiFi-QoS from descriptor.
 */
#define EDMA_RXDESC_WIFI_QOS_MASK		0xFF000000
#define EDMA_RXDESC_WIFI_QOS_SHIFT		0x18
#define EDMA_RXDESC_TREE_ID_GET(desc)		(le32_to_cpu(((desc)->word2)))
#define EDMA_RXDESC_WIFI_QOS_GET(desc)		((le32_to_cpu(((desc)->word5)) & \
						EDMA_RXDESC_WIFI_QOS_MASK) >> \
						EDMA_RXDESC_WIFI_QOS_SHIFT)

/*
 * Check if WiFi-QoS flag is valid.
 */
#define EDMA_RXDESC_WIFI_QOS_FLAG_VALID_GET(desc)	((le32_to_cpu((desc)->word1)) & 0x00008000)

/*
 * Tree_id related Macros.
 *	---------------------------------------------------------------------------------
 *	|Tree_ID Type (4 bits) | 		Tree_ID Metadata(20 bits)		|
 *	---------------------------------------------------------------------------------
 */
#define EDMA_RXDESC_TREE_ID_TYPE_SHIFT			20
#define EDMA_RXDESC_TREE_ID_TYPE_MASK			0x00F00000
#define EDMA_RXDESC_TREE_ID_TYPE_GET(desc)		((EDMA_RXDESC_TREE_ID_GET(desc) & EDMA_RXDESC_TREE_ID_TYPE_MASK) \
								>> EDMA_RXDESC_TREE_ID_TYPE_SHIFT)
/*
 * SAWF related macros
 */
#define EDMA_RXDESC_SERVICE_CLASS_SHIFT			10
#define EDMA_RXDESC_SERVICE_CLASS_MASK			0x0003FC00
#define EDMA_RXDESC_SERVICE_CLASS_GET(desc)		((EDMA_RXDESC_TREE_ID_GET(desc) & EDMA_RXDESC_SERVICE_CLASS_MASK) \
								>> EDMA_RXDESC_SERVICE_CLASS_SHIFT)
#define EDMA_RXDESC_PEER_ID_MASK			0x000003FF
#define EDMA_RXDESC_PEER_ID_GET(desc)			(EDMA_RXDESC_TREE_ID_GET(desc) & EDMA_RXDESC_PEER_ID_MASK)

/*
 * Service class TAG to be added for SAWF metadata.
 */
#define EDMA_RX_SAWF_SERVICE_CLASS_TAG			0xAA000000

/*
 * Construct the SAWF metadata
 *	----------------------------------------------------------------------------
 *	|TAG (8 bits) | service_class (8 bits) | peerid (10 bits) | MSDUQ (6 bits))|
 *	----------------------------------------------------------------------------
 */
#define EDMA_RX_SAWF_METADATA_SERVICE_CLASS_SHIFT		16
#define EDMA_RX_SAWF_METADATA_PEER_ID_SHIFT			6
#define EDMA_RX_SAWF_METADATA_CONSTRUCT(sc, pi, msduq)		(EDMA_RX_SAWF_SERVICE_CLASS_TAG | \
								(sc << EDMA_RX_SAWF_METADATA_SERVICE_CLASS_SHIFT) | \
								(pi << EDMA_RX_SAWF_METADATA_PEER_ID_SHIFT) | \
								msduq)
/*
 * Opaque values are set in word2 and word3, they are not accessed by the EDMA HW,
 * so endianness conversion is not needed.
*/
#define EDMA_RXFILL_ENDIAN_SET(desc)	{ \
	cpu_to_le32s(&((desc)->word0)); \
	cpu_to_le32s(&((desc)->word1)); \
}

/*
 * RX DESC size shift to obtain index from descriptor pointer
 */
#define EDMA_RXDESC_SIZE_SHIFT		5

/*
 * edma_ring_usage
 *	Indices for stats
 */
enum edma_ring_usage {
	EDMA_RING_USAGE_100_FULL = 0,
	EDMA_RING_USAGE_90_TO_100_FULL,
	EDMA_RING_USAGE_70_TO_90_FULL,
	EDMA_RING_USAGE_50_TO_70_FULL,
	EDMA_RING_USAGE_LESS_50_FULL,
	EDMA_RING_USAGE_MAX_FULL,
};

/*
 * edma_ring_util_stats
 *	Structure for tracking ring utilization
 */
struct edma_ring_util_stats {
	uint32_t util[EDMA_RING_USAGE_MAX_FULL];
};

/*
 * edma_rx_stats
 *	EDMA RX per cpu stats
 */
struct edma_rx_stats {
	uint64_t rx_pkts;
	uint64_t rx_bytes;
	uint64_t rx_drops;
	uint64_t rx_nr_frag_pkts;
	uint64_t rx_fraglist_pkts;
	uint64_t rx_nr_frag_headroom_err;
	uint64_t rx_vp_uninitialized;
	struct u64_stats_sync syncp;
};

/*
 * edma_rx_desc_stats
 *	RX descriptor ring stats data structure
 */
struct edma_rx_desc_stats {
	uint64_t src_port_inval;		/* Invalid source port number */
	uint64_t src_port_inval_type;		/* Source type is not PORT ID */
	uint64_t src_port_inval_netdev;		/* Invalid net device for the source port */
	struct edma_ring_util_stats ring_stats;	/* Tracking EDMA Rx Desc ring utilization */
	struct u64_stats_sync syncp;		/* Synchronization pointer */
};

/*
 * edma_rx_fill_stats
 *	Rx fill descriptor ring stats data structure
 */
struct edma_rx_fill_stats {
	uint64_t alloc_failed;			/* Buffer allocation failure count */
	uint64_t page_alloc_failed;		/* Page allocation failure count for page mode */
	struct edma_ring_util_stats ring_stats;    /* Tracking EDMA Rx Fill ring utilization */
	struct u64_stats_sync syncp;		/* Synchronization pointer */
};

/*
 * Rx descriptor
 */
struct edma_rxdesc_desc {
	uint32_t word0;		/* Contains buffer address */
	uint32_t word1;		/* Contains more bit, priority bit, service code */
	uint32_t word2;		/* Contains opaque */
	uint32_t word3;		/* Contains opaque high bits */
	uint32_t word4;		/* Contains destination and source information */
	uint32_t word5;		/* Contains WiFi QoS, data length */
	uint32_t word6;		/* Contains hash value, check sum status */
	uint32_t word7;		/* Contains DSCP, packet offsets */
};

/*
 * Rx secondary descriptor
 */
struct edma_rxdesc_sec_desc {
	uint32_t word0;		/* Contains timestamp */
	uint32_t word1;		/* Contains secondary checksum status */
	uint32_t word2;		/* Contains QoS tag */
	uint32_t word3;		/* Contains flow index details */
	uint32_t word4;		/* Contains secondary packet offsets */
	uint32_t word5;		/* Contains multicast bit, checksum */
	uint32_t word6;		/* Contains SVLAN, CVLAN */
	uint32_t word7;		/* Contains secondary SVLAN, CVLAN */
};

/*
 * RxFill descriptor
 */
struct edma_rxfill_desc {
	uint32_t word0;		/* Contains buffer address */
	uint32_t word1;		/* Contains buffer size */
	uint32_t word2;		/* Contains opaque */
	uint32_t word3;		/* Contains opaque high bits */
};

/*
 * RxFill ring
 */
struct edma_rxfill_ring {
#ifdef NSS_DP_PPEDS_SUPPORT
	struct napi_struct napi;	/* Napi structure */
#endif
	uint32_t ring_id;		/* RXFILL ring number */
	uint32_t count;			/* number of descriptors in the ring */
	uint32_t prod_idx;		/* Ring producer index */
	uint32_t alloc_size;		/* Buffer size to allocate */
	struct edma_rxfill_desc *desc;	/* descriptor ring virtual address */
	dma_addr_t dma;			/* descriptor ring physical address */
	uint32_t buf_len;		/* Buffer length for rxfill descriptor */
	bool page_mode;			/* Page mode for Rx processing */
	struct edma_rx_fill_stats rx_fill_stats;
					/* Rx fill ring statistics */
};

struct nss_dp_vp_skb_list;

/*
 * RxDesc ring
 */
struct edma_rxdesc_ring {
	struct napi_struct napi;	/* Napi structure */
	uint32_t ring_id;		/* RXDESC ring number */
	uint32_t count;			/* number of descriptors in the ring */
	uint32_t work_leftover;		/* Leftover descriptors to be processed */
	uint32_t cons_idx;		/* Ring consumer index */
	struct edma_rxdesc_desc *pdesc;
					/* Primary descriptor ring virtual address */
	struct edma_rxdesc_desc *pdesc_head;
					/* Primary descriptor head in case of scatter-gather frame */
	struct edma_rxdesc_sec_desc *sdesc;
					/* Secondary descriptor ring virtual address */
	struct edma_rx_desc_stats rx_desc_stats;
					/* Rx descriptor ring statistics */
	struct edma_rxfill_ring *rxfill;
					/* RXFILL ring used */
	bool napi_added;		/* Flag to indicate NAPI add status */
	dma_addr_t pdma;		/* Primary descriptor ring physical address */
	dma_addr_t sdma;		/* Secondary descriptor ring physical address */
	struct sk_buff *head;		/* Head of the skb list in case of scatter-gather frame */
	struct sk_buff *last;		/* Last skb of the skb list in case of scatter-gather frame */
	struct nss_dp_vp_skb_list *vp_head;
					/* Last skb of the skb list in case of scatter-gather frame */
};

irqreturn_t edma_rx_handle_irq(int irq, void *ctx);
#ifdef NSS_DP_PPEDS_SUPPORT
irqreturn_t edma_rxfill_handle_irq(int irq, void *ctx);
#endif
int edma_rx_alloc_buffer(struct edma_rxfill_ring *rxfill_ring, int alloc_count);
int edma_rx_napi_poll(struct napi_struct *napi, int budget);
bool edma_rx_phy_tstamp_buf(__attribute__((unused))void *app_data, struct sk_buff *skb, void *sc_data);
int edma_rx_napi_capwap_poll(struct napi_struct *napi, int budget);

#endif	/* __EDMA_RX_H__ */
