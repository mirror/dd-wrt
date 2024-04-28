/*
 **************************************************************************
 * Copyright (c) 2014-2017, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */
/*
 * nss_gmac_api_if.h
 *	nss-gmac exported structure/apis.
 */

#ifndef __GMAC_API_IF_H
#define __GMAC_API_IF_H

#define NSS_GMAC_NORMAL_FRAME_MTU 1500
#define NSS_GMAC_MINI_JUMBO_FRAME_MTU 1978
#define NSS_GMAC_FULL_JUMBO_FRAME_MTU 9600

/*
 * NSS GMAC status
 */
#define NSS_GMAC_SUCCESS	0
#define NSS_GMAC_FAILURE	1

/*
 * NSS GMAC mode
 */
#define NSS_GMAC_MODE0	0	/* gmac mode 0 */
#define NSS_GMAC_MODE1	1	/* gmac mode 1 */
#define NSS_GMAC_MODE2	2	/* gmac mode 2 */

/*
 * struct nss_gmac_stats
 * The NA per-GMAC statistics statistics structure.
 */
struct nss_gmac_stats {
	uint64_t rx_bytes;		/**< Number of RX bytes */
	uint64_t rx_packets;		/**< Number of RX packets */
	uint64_t rx_errors;		/**< Number of RX errors */
	uint64_t rx_receive_errors;	/**< Number of RX receive errors */
	uint64_t rx_overflow_errors;	/**< Number of RX overflow errors */
	uint64_t rx_descriptor_errors;	/**< Number of RX descriptor errors */
	uint64_t rx_watchdog_timeout_errors;
					/**< Number of RX watchdog timeout errors */
	uint64_t rx_crc_errors;		/**< Number of RX CRC errors */
	uint64_t rx_late_collision_errors;
					/**< Number of RX late collision errors */
	uint64_t rx_dribble_bit_errors;	/**< Number of RX dribble bit errors */
	uint64_t rx_length_errors;	/**< Number of RX length errors */
	uint64_t rx_ip_header_errors;	/**< Number of RX IP header errors */
	uint64_t rx_ip_payload_errors;	/**< Number of RX IP payload errors */
	uint64_t rx_no_buffer_errors;	/**< Number of RX no-buffer errors */
	uint64_t rx_transport_csum_bypassed;
					/**< Number of RX packets where the transport checksum was bypassed */
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
	uint64_t hw_errs[10];		/**< GMAC DMA error counters */
	uint64_t rx_missed;		/**< Number of RX packets missed by the DMA */
	uint64_t fifo_overflows;	/**< Number of RX FIFO overflows signalled by the DMA */
	uint64_t rx_scatter_errors;	/**< Number of scattered frames received by the DMA */
	uint64_t tx_ts_create_errors;	/**< Number of tx timestamp creation errors */
	uint64_t gmac_total_ticks;	/**< Total clock ticks spend inside the GMAC */
	uint64_t gmac_worst_case_ticks;	/**< Worst case iteration of the GMAC in ticks */
	uint64_t gmac_iterations;	/**< Number of iterations around the GMAC */
	uint64_t tx_pause_frames;	/**< Number of pause frames sent by the GMAC */
	uint64_t rx_octets_g;		/* Number of good octets received */
	uint64_t rx_ucast_frames;	/* Number of Unicast frames received */
	uint64_t rx_bcast_frames;	/* Number of Bcast frames received */
	uint64_t rx_mcast_frames;	/* Number of Mcast frames received */
	uint64_t rx_undersize;		/* Number of RX undersize frames */
	uint64_t rx_oversize;		/* Number of RX oversize frames */
	uint64_t rx_jabber;		/* Number of jabber frames */
	uint64_t rx_octets_gb;		/* Number of good/bad octets */
	uint64_t rx_frag_frames_g;	/* Number of good ipv4 frag frames */
	uint64_t tx_octets_g;		/* Number of good octets sent */
	uint64_t tx_ucast_frames;	/* Number of Unicast frames sent*/
	uint64_t tx_bcast_frames;	/* Number of Broadcast frames sent */
	uint64_t tx_mcast_frames;	/* Number of Multicast frames sent */
	uint64_t tx_deferred;		/* Number of Deferred frames sent */
	uint64_t tx_single_col;		/* Number of single collisions */
	uint64_t tx_multiple_col;	/* Number of multiple collisions */
	uint64_t tx_octets_gb;		/* Number of good/bad octets sent*/
};

/*
 * NSS GMAC data plane ops, default would be slowpath and can be overridden by
 * nss-drv
 */
struct nss_gmac_data_plane_ops {
	int (*open)(void *ctx, uint32_t tx_desc_ring, uint32_t rx_desc_ring,
							uint32_t mode);
	int (*close)(void *ctx);
	int (*link_state)(void *ctx, uint32_t link_state);
	int (*mac_addr)(void *ctx, uint8_t *addr);
	int (*change_mtu)(void *ctx, uint32_t mtu);
	int (*xmit)(void *ctx, struct sk_buff *os_buf);
	void (*set_features)(struct net_device *netdev);
	int (*pause_on_off)(void *ctx, uint32_t pause_on);
	void (*get_stats)(void *ctx, struct nss_gmac_stats *stats);
};

extern void nss_gmac_receive(struct net_device *netdev, struct sk_buff *skb,
						struct napi_struct *napi);
void nss_gmac_start_data_plane(struct net_device *netdev, void *ctx);
extern int nss_gmac_override_data_plane(struct net_device *netdev,
			struct nss_gmac_data_plane_ops *dp_ops, void *ctx);
extern void nss_gmac_restore_data_plane(struct net_device *netdev);
extern struct net_device *nss_gmac_get_netdev_by_macid(int macid);
extern bool nss_gmac_is_in_open_state(struct net_device *netdev);
#endif
