/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 *
 * Copyright (c) 2021-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef __NSS_DP_ARCH_H__
#define __NSS_DP_ARCH_H__

#define NSS_DP_HAL_MAX_PORTS			2
#define NSS_DP_MAX_PORTS			NSS_DP_HAL_MAX_PORTS
#define NSS_DP_HAL_START_IFNUM			0

/*
 * Maximum supported GSO segments
 */
#define NSS_DP_HAL_GSO_MAX_SEGS			GSO_MAX_SEGS

/*
 * Number of TX/RX queue supported
 */
#define NSS_DP_QUEUE_NUM			1

/*
 * TX/RX NAPI budget
 */
#define NSS_DP_HAL_RX_NAPI_BUDGET		32
#define NSS_DP_HAL_TX_NAPI_BUDGET		32

/*
 * TCSR_GMAC_AXI_CACHE_OVERRIDE register size
 */
#define TCSR_GMAC_AXI_CACHE_OVERRIDE_REG_SIZE	4

/*
 * TCSR_GMAC_AXI_CACHE_OVERRIDE Register offset
 */
#define TCSR_GMAC_AXI_CACHE_OVERRIDE_OFFSET	0x6224

/*
 * Value for TCSR_GMAC_AXI_CACHE_OVERRIDE register
 */
#define TCSR_GMAC_AXI_CACHE_OVERRIDE_VALUE	0x05050505

/**
 * nss_dp_hal_gmac_stats_rx
 *	Per-GMAC Rx statistics
 */
struct nss_dp_hal_gmac_stats_rx {
	uint64_t rx_bytes;		/**< Number of RX bytes */
	uint64_t rx_packets;		/**< Number of RX packets */
	uint64_t rx_errors;		/**< Number of RX errors */
	uint64_t rx_missed;		/**< Number of RX packets missed by the DMA */
	uint64_t rx_descriptor_errors;	/**< Number of RX descriptor errors */
	uint64_t rx_late_collision_errors;
					/**< Number of RX late collision errors */
	uint64_t rx_dribble_bit_errors;	/**< Number of RX dribble bit errors */
	uint64_t rx_length_errors;	/**< Number of RX length errors */
	uint64_t rx_ip_header_errors;	/**< Number of RX IP header errors read from rxdec */
	uint64_t rx_ip_payload_errors;	/**< Number of RX IP payload errors */
	uint64_t rx_no_buffer_errors;	/**< Number of RX no-buffer errors */
	uint64_t rx_transport_csum_bypassed;
					/**< Number of RX packets where the transport checksum was bypassed */
	uint64_t rx_fifo_overflows;	/**< Number of RX FIFO overflows signalled by the DMA */
	uint64_t rx_overflow_errors;	/**< Number of Rx Overflow errors received from Rx descriptors */
	uint64_t rx_crc_errors;		/**< Number of Rx CRC errors */
	uint64_t rx_skb_alloc_errors;	/**< Number of Rx skb alocation errors */
	uint64_t rx_scatter_packets;	/**< Number of received scattered frames successful */
	uint64_t rx_scatter_bytes;	/**< Number of bytes received for scattered frames */
	uint64_t rx_scatter_errors;	/**< Number of RX scatter errors */
};

/**
 * nss_dp_hal_gmac_stats_tx
 *	Per-GMAC Tx statistics
 */
struct nss_dp_hal_gmac_stats_tx {
	uint64_t tx_bytes;		/**< Number of TX bytes */
	uint64_t tx_packets;		/**< Number of TX packets */
	uint64_t tx_collisions;		/**< Number of TX collisions */
	uint64_t tx_errors;		/**< Number of TX errors */
	uint64_t tx_jabber_timeout_errors;
					/**< Number of TX jabber timeout errors */
	uint64_t tx_frame_flushed_errors;
					/**< Number of TX frame flushed errors */
	uint64_t tx_loss_of_carrier_errors;
					/**< Number of TX loss of carrier errors */
	uint64_t tx_no_carrier_errors;	/**< Number of TX no carrier errors */
	uint64_t tx_late_collision_errors;
					/**< Number of TX late collision errors */
	uint64_t tx_excessive_collision_errors;
					/**< Number of TX excessive collision errors */
	uint64_t tx_excessive_deferral_errors;
					/**< Number of TX excessive deferral errors */
	uint64_t tx_underflow_errors;	/**< Number of TX underflow errors */
	uint64_t tx_ip_header_errors;	/**< Number of TX IP header errors */
	uint64_t tx_ip_payload_errors;	/**< Number of TX IP payload errors */
	uint64_t tx_dropped;		/**< Number of TX dropped packets */
	uint64_t tx_ts_create_errors;	/**< Number of tx timestamp creation errors */
	uint64_t tx_desc_not_avail;	/**< TX descriptor unavailable */
	uint64_t tx_nr_frags_pkts;	/**< Number of Tx scatter packets with nr_frags */
	uint64_t tx_fraglist_pkts;	/**< Number of Tx scatter packets with frag_list */
	uint64_t tx_packets_requeued;	/**< Number of Tx packets requeued */
};

/**
 * nss_dp_hal_gmac_stats
 *	The per-GMAC statistics structure.
 */
struct nss_dp_hal_gmac_stats {
	struct nss_dp_hal_gmac_stats_rx rx_stats;
					/**< GMAC Rx statistics */
	struct nss_dp_hal_gmac_stats_tx tx_stats;
					/**< GMAC Tx statistics */
	uint64_t hw_errs[10];		/**< GMAC DMA error counters */
};

/**
 * nss_dp_hal_nsm_sawf_sc_stats
 *	Per-service code stats to be send to NSM.
 */
struct nss_dp_hal_nsm_sawf_sc_stats {
};

extern struct nss_dp_data_plane_ops nss_dp_gmac_ops;
extern bool nss_dp_hal_nsm_sawf_sc_stats_read(struct nss_dp_hal_nsm_sawf_sc_stats *nsm_stats, uint8_t service_class);

#endif /* __NSS_DP_ARCH_H__ */
