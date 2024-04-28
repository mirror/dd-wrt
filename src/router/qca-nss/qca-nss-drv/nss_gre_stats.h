/*
 **************************************************************************
 * Copyright (c) 2017, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
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
 * nss_gre_stats.h
 *	NSS GRE statistics header file.
 */

#ifndef __NSS_GRE_STATS_H
#define __NSS_GRE_STATS_H

/*
 * GRE base debug statistics types
 */
enum nss_gre_stats_base_debug_types {
	NSS_GRE_STATS_BASE_RX_PACKETS,			/**< Rx packet count. */
	NSS_GRE_STATS_BASE_RX_DROPPED,			/**< Rx dropped count. */
	NSS_GRE_STATS_BASE_EXP_ETH_HDR_MISSING,		/**< Ethernet header missing. */
	NSS_GRE_STATS_BASE_EXP_ETH_TYPE_NON_IP,		/**< Not IPV4 or IPV6 packet. */
	NSS_GRE_STATS_BASE_EXP_IP_UNKNOWN_PROTOCOL,	/**< Unknown protocol. */
	NSS_GRE_STATS_BASE_EXP_IP_HEADER_INCOMPLETE,	/**< Bad IP header. */
	NSS_GRE_STATS_BASE_EXP_IP_BAD_TOTAL_LENGTH,	/**< Invalid IP packet length. */
	NSS_GRE_STATS_BASE_EXP_IP_BAD_CHECKSUM,		/**< Bad packet checksum. */
	NSS_GRE_STATS_BASE_EXP_IP_DATAGRAM_INCOMPLETE,	/**< Bad packet. */
	NSS_GRE_STATS_BASE_EXP_IP_FRAGMENT,		/**< IP fragment. */
	NSS_GRE_STATS_BASE_EXP_IP_OPTIONS_INCOMPLETE,	/**< Invalid IP options. */
	NSS_GRE_STATS_BASE_EXP_IP_WITH_OPTIONS,		/**< IP packet with options. */
	NSS_GRE_STATS_BASE_EXP_IPV6_UNKNOWN_PROTOCOL,	/**< Unknown protocol. */
	NSS_GRE_STATS_BASE_EXP_IPV6_HEADER_INCOMPLETE,	/**< Incomplete IPV6 header. */
	NSS_GRE_STATS_BASE_EXP_GRE_UNKNOWN_SESSION,	/**< Unknown GRE session. */
	NSS_GRE_STATS_BASE_EXP_GRE_NODE_INACTIVE,	/**< GRE node inactive. */
	NSS_GRE_STATS_BASE_DEBUG_MAX,			/**< GRE base error max. */
};

/*
 *  GRE base debug statistics
 */
struct nss_gre_stats_base_debug	{
	uint64_t stats[NSS_GRE_STATS_BASE_DEBUG_MAX];	/**< GRE debug statistics. */
};

/*
 * GRE session debug statistics types
 */
enum nss_gre_stats_session_debug_types {
	NSS_GRE_STATS_SESSION_PBUF_ALLOC_FAIL,			/**< Pbuf alloc failure. */
	NSS_GRE_STATS_SESSION_DECAP_FORWARD_ENQUEUE_FAIL,	/**< Rx forward enqueue failure. */
	NSS_GRE_STATS_SESSION_ENCAP_FORWARD_ENQUEUE_FAIL,	/**< Tx forward enqueue failure. */
	NSS_GRE_STATS_SESSION_DECAP_TX_FORWARDED,		/**< Packets forwarded after decap. */
	NSS_GRE_STATS_SESSION_ENCAP_RX_RECEIVED,		/**< Packets received for encap. */
	NSS_GRE_STATS_SESSION_ENCAP_RX_DROPPED,			/**< Packets dropped while enqueue for encap. */
	NSS_GRE_STATS_SESSION_ENCAP_RX_LINEAR_FAIL,		/**< Packets dropped during encap linearization. */
	NSS_GRE_STATS_SESSION_EXP_RX_KEY_ERROR,			/**< Rx KEY error. */
	NSS_GRE_STATS_SESSION_EXP_RX_SEQ_ERROR,			/**< Rx sequence number error. */
	NSS_GRE_STATS_SESSION_EXP_RX_CS_ERROR,			/**< Rx checksum error. */
	NSS_GRE_STATS_SESSION_EXP_RX_FLAG_MISMATCH,		/**< Rx flag mismatch. */
	NSS_GRE_STATS_SESSION_EXP_RX_MALFORMED,			/**< Rx malformed packet. */
	NSS_GRE_STATS_SESSION_EXP_RX_INVALID_PROTOCOL,		/**< Rx invalid protocol. */
	NSS_GRE_STATS_SESSION_EXP_RX_NO_HEADROOM,		/**< Rx no headroom. */
	NSS_GRE_STATS_SESSION_DEBUG_MAX,			/**< Session debug max. */
};

/*
 *  GRE session debug statistics
 */
struct nss_gre_stats_session_debug {
	uint64_t stats[NSS_GRE_STATS_SESSION_DEBUG_MAX];	/**< Session debug statistics. */
	int32_t if_index;					/**< Netdevice's ifindex. */
	uint32_t if_num;					/**< NSS interface number. */
	bool valid;						/**< Is node valid ? */
};

/*
 * GRE statistics APIs
 */
extern void nss_gre_stats_session_debug_sync(struct nss_ctx_instance *nss_ctx, struct nss_gre_session_stats_msg *sstats, uint16_t if_num);
extern void nss_gre_stats_base_debug_sync(struct nss_ctx_instance *nss_ctx, struct nss_gre_base_stats_msg *bstats);
extern void nss_gre_stats_session_register(uint32_t if_num, struct net_device *netdev);
extern void nss_gre_stats_session_unregister(uint32_t if_num);
extern void nss_gre_stats_dentry_create(void);

#endif /* __NSS_GRE_STATS_H */
