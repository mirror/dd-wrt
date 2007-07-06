/*
 * netlink/route/link.h		libnl link module
 *
 * Copyright (c) 2003-2005 Thomas Graf <tgraf@suug.ch>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 */

#ifndef NETLINK_LINK_H_
#define NETLINK_LINK_H_

#include <netlink/netlink.h>
#include <netlink/cache.h>
#include <netlink/addr.h>

/**
 * @name Available Attributes Flags
 * @ingroup link
 * @anchor link_avail_attrs
 * Flags used in rtnl_link::l_mask to indicate which attributes are
 * present in a link.
 *
 * The corresponding flag is automatically set if a attribute is set
 * via a \c rtnl_link_set_* function or if the link was created by
 * the internal message parser, e.g. if it comes from a cache.
 *
 * @par Example: Check if link has a broadcast address and weight attribute set
 * @code
 * if (link->l_mask & (LINK_HAS_BRD | LINK_HAS_QDISC))
 *         // action goes here
 * @endcode
 * @{
 */
#define LINK_HAS_MTU     0x0001		/**< Has MTU */
#define LINK_HAS_LINK    0x0002		/**< Has parent device */
#define LINK_HAS_TXQLEN  0x0004		/**< Has transmission queue length */
#define LINK_HAS_WEIGHT  0x0008		/**< Has weight */
#define LINK_HAS_MASTER  0x0010		/**< Has master device */
#define LINK_HAS_QDISC   0x0020		/**< Has qdisc kind */
#define LINK_HAS_MAP     0x0040		/**< Has interface device mapping */
#define LINK_HAS_ADDR    0x0080		/**< Has link layer address */
#define LINK_HAS_BRD     0x0100		/**< Has link layer broadcast addr */
#define LINK_HAS_FLAGS   0x0200		/**< Has flags */
#define LINK_HAS_IFNAME  0x0400		/**< Has interface name */
#define LINK_HAS_IFINDEX 0x0800		/**< Has interface index */
#define LINK_HAS_FAMILY  0x1000		/**< Has address family */
#define LINK_HAS_ARPTYPE 0x2000		/**< Has ARP type */
#define LINK_HAS_STATS   0x4000		/**< Has statistics */
#define LINK_HAS_CHANGE  0x8000		/**< Has change mask */
/** @} */

/**
 * Generic Link statistics
 * @ingroup link
 *
 * These statistics are available for both transmitter
 * and receiver.
 * 
 * @note Although the counters are 64bit sized their kernel counterparts
 *       may not and thus overflows can still happen at 32bit.
 */
struct rtnl_gen_lstats
{
	uint64_t packets;	/**< Packets transmitted/received */
	uint64_t bytes;		/**< Bytes transmitted/received */
	uint64_t errors;	/**< Errors in transmitter/receiver */
	uint64_t dropped;	/**< Packets dropped */
	uint64_t compressed;	/**< Compressed packets (cslip etc) */
	uint64_t multicast;	/**< Multicast packets received */
};

/**
 * Link statistics
 * @ingroup link
 * @note Although the counters are 64bit sized their kernel counterparts
 *       may not and thus overflows can still happen at 32bit.
 */
struct rtnl_lstats
{
	struct rtnl_gen_lstats	ls_rx;	/**< Generic receiving statistics */
	struct rtnl_gen_lstats	ls_tx;	/**< Generic transmission statistics */

	/* detailed RX errors */
	uint64_t ls_rx_length_errors;	/**< Packets either too small or too big */
	uint64_t ls_rx_over_errors;	/**< Receiver ring buffer overflows */
	uint64_t ls_rx_crc_errors;	/**< Received packets with CRC errors */
	uint64_t ls_rx_frame_errors;	/**< Received packets with frame align errors */
	uint64_t ls_rx_fifo_errors;	/**< Receiver FIFO overruns */
	uint64_t ls_rx_missed_errors;	/**< Packets missed by receiver */

	/* detailed TX errors */
	uint64_t ls_tx_aborted_errors;	/**< Aborted transsmisions (e.g. timeout) */
	uint64_t ls_tx_carrier_errors;	/**< Aborted transmission due to missing carrier */
	uint64_t ls_tx_fifo_errors;	/**< Transmitter FIFO underflows */
	uint64_t ls_tx_heartbeat_errors; /**< Heartbeat errors (tx lifetime exceeded) */
	uint64_t ls_tx_window_errors;	/**< Transmitted packets out of window */
	uint64_t ls_tx_collisions;	/**< Collisions while transmitting */
};

/**
 * Link interface device mapping
 * @ingroup link
 */
struct rtnl_lifmap
{
	uint64_t lim_mem_start;		/**< mem_start */
	uint64_t lim_mem_end;		/**< mem_end */
	uint64_t lim_base_addr;		/**< base address */
	uint16_t lim_irq;		/**< interrupt number */
	uint8_t  lim_dma;		/**< DMA */
	uint8_t  lim_port;		/**< interface port */
};

/**
 * Link (interface)
 * @ingroup link
 */
struct rtnl_link
{
	/** Common header required by cache */
	NLHDR_COMMON

	char	l_name[IFNAMSIZ+1];	/**< Name of link */

	uint32_t	l_family;	/**< Address family of link layer adddresses */
	uint32_t	l_arptype;	/**< ARPHDR_ type */
	uint32_t	l_index;	/**< Interface index */
	uint32_t	l_flags;	/**< Link Flags */
	uint32_t	l_change;	/**< Change mask */
	uint32_t 	l_mtu;		/**< Maximum Transmission Unit */
	uint32_t	l_link;		/**< Interface index of parent link (tunnels) */
	uint32_t	l_txqlen;	/**< Transmission queue length */
	uint32_t	l_weight;	/**< Weight */
	uint32_t	l_master;	/**< Interface index of master link */
	struct nl_addr	l_addr;		/**< Link layer address */
	struct nl_addr	l_bcast;	/**< Link layer broadcast address */
	char		l_qdisc[32];	/**< Name of queueing discipline */

	struct rtnl_lifmap l_map;	/**< Interface device mapping */
	struct rtnl_lstats l_stats;	/**< Link statistics */ 

	/**
	 * Available attribtues (LINK_HAS_*)
	 * @see \ref link_avail_attrs "Available Attributes Flags"
	 */
	uint32_t	l_mask;	
	uint32_t	l_flag_mask;
};

extern struct nl_cache_ops rtnl_link_ops;

/**
 * Initialize a link cache structure.
 * @ingroup link
 *
 * @code
 * struct nl_cache cache = RTNL_INIT_LINK_CACHE();
 * @endcode
 */
#define RTNL_INIT_LINK_CACHE() {               \
    .c_type = RTNL_LINK,                       \
    .c_type_size = sizeof(struct rtnl_link),   \
    .c_ops = &rtnl_link_ops,                   \
}

/**
 * Initialize a link strcuture.
 * @ingroup link
 *
 * @code
 * struct rtnl_link l = RTNL_INIT_LINK();
 * @endcode
 */
#define RTNL_INIT_LINK() {                     \
    .ce_type = RTNL_LINK,                      \
    .ce_size = sizeof(struct rtnl_link),       \
}

/**
 * Special interface index stating the link was not found.
 * @ingroup link
 */
#define RTNL_LINK_NOT_FOUND -1

extern struct nl_cache * rtnl_link_build_cache(struct nl_handle *);
extern struct rtnl_link * rtnl_link_get(struct nl_cache *c, int);
extern struct rtnl_link * rtnl_link_get_by_name(struct nl_cache *,
						const char *);
extern void rtnl_link_dump(struct rtnl_link *, FILE *, struct nl_dump_params *);

extern int  rtnl_link_name2i(struct nl_cache *, const char *);
extern const char * rtnl_link_i2name(struct nl_cache *, int);

extern struct nl_msg * rtnl_link_build_change_request(struct rtnl_link *,
                                                      struct rtnl_link *);
extern int  rtnl_link_change(struct nl_handle *, struct rtnl_link *,
                             struct rtnl_link *);

extern char * rtnl_link_flags2str_r(int, char *, size_t);
extern char * rtnl_link_flags2str(int);
extern int rtnl_link_str2flags(const char *);

extern void rtnl_link_set_qdisc(struct rtnl_link *, const char *);
extern void rtnl_link_set_name(struct rtnl_link *, const char *);
extern void rtnl_link_set_addr(struct rtnl_link *, struct nl_addr *);
extern int  rtnl_link_set_addr_str(struct rtnl_link *, const char *);
extern void rtnl_link_set_broadcast(struct rtnl_link *, struct nl_addr *);
extern int  rtnl_link_set_broadcast_str(struct rtnl_link *, const char *);
extern void rtnl_link_set_flags(struct rtnl_link *, int);
extern void rtnl_link_unset_flags(struct rtnl_link *, int);
extern void rtnl_link_set_family(struct rtnl_link *l, int);
extern void rtnl_link_set_mtu(struct rtnl_link *, uint32_t);
extern void rtnl_link_set_txqlen(struct rtnl_link *, uint32_t);
extern void rtnl_link_set_weight(struct rtnl_link *, uint32_t);
extern void rtnl_link_set_ifindex(struct rtnl_link *, int);
extern int  rtnl_link_set_ifindex_name(struct rtnl_link *, struct nl_cache *,
                                       const char *);
extern void rtnl_link_set_link(struct rtnl_link *, uint32_t);
extern int  rtnl_link_set_link_name(struct rtnl_link *, struct nl_cache *,
                                    const char *);
extern void rtnl_link_set_master(struct rtnl_link *, uint32_t);
extern int  rtnl_link_set_master_name(struct rtnl_link *, struct nl_cache *,
                                      const char *);
#endif
