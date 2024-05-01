/*
 * sfe_ipv4.h
 *	Shortcut forwarding engine header file for IPv4.
 *
 * Copyright (c) 2013-2016, 2019-2020, The Linux Foundation. All rights reserved.
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

#ifndef __SFE_IPV4_H
#define __SFE_IPV4_H

#define SFE_IPV4_DSCP_MASK 0x3
#define SFE_IPV4_DSCP_SHIFT 2
#include <linux/version.h>

/*
 * Size of single sfe_dump msg buffer
 */
#define SFE_IPV4_DEBUG_MSG_SIZE 2048

/*
 * Specifies the lower bound on ACK numbers carried in the TCP header
 */
#define SFE_IPV4_TCP_MAX_ACK_WINDOW 65520

/*
 * IPv4 TCP connection match additional data.
 */
struct sfe_ipv4_tcp_connection_match {
	u8 win_scale;		/* Window scale */
	u32 max_win;		/* Maximum window size seen */
	u32 end;			/* Sequence number of the next byte to send (seq + segment length) */
	u32 max_end;		/* Sequence number of the last byte to ack */
};

#ifdef SFE_BRIDGE_VLAN_FILTERING_ENABLE
/*
 * Bridge VLAN Filter connection match rule structure.
 */
struct sfe_ipv4_vlan_filter_connection_match {
	u32 ingress_vlan_tag;	/**< VLAN tag for ingress packets. */
	u8 ingress_flags;	/**< VLAN flags at ingress. */
	u8 reserved[3];		/**< reserved. */
};
#endif

/*
 * Bit flags for IPv4 connection matching entry.
 */
#define SFE_IPV4_CONNECTION_MATCH_FLAG_XLATE_SRC (1<<0)
					/* Perform source translation */
#define SFE_IPV4_CONNECTION_MATCH_FLAG_XLATE_DEST (1<<1)
					/* Perform destination translation */
#define SFE_IPV4_CONNECTION_MATCH_FLAG_NO_SEQ_CHECK (1<<2)
					/* Ignore TCP sequence numbers */
#define SFE_IPV4_CONNECTION_MATCH_FLAG_WRITE_FAST_ETH_HDR (1<<3)
					/* Fast Ethernet header write */
#define SFE_IPV4_CONNECTION_MATCH_FLAG_WRITE_L2_HDR (1<<4)
					/* Fast Ethernet header write */
#define SFE_IPV4_CONNECTION_MATCH_FLAG_PRIORITY_REMARK (1<<5)
					/* remark priority of SKB */
#define SFE_IPV4_CONNECTION_MATCH_FLAG_DSCP_REMARK (1<<6)
					/* remark DSCP of packet */
#define SFE_IPV4_CONNECTION_MATCH_FLAG_CSUM_OFFLOAD (1<<7)
					/* checksum offload.*/
#define SFE_IPV4_CONNECTION_MATCH_FLAG_PPPOE_DECAP (1<<8)
					/* Indicates that PPPoE should be decapsulated */
#define SFE_IPV4_CONNECTION_MATCH_FLAG_PPPOE_ENCAP (1<<9)
					/* Indicates that PPPoE should be encapsulated */
#define SFE_IPV4_CONNECTION_MATCH_FLAG_BRIDGE_FLOW (1<<10)
					/* Bridge flow */
#define SFE_IPV4_CONNECTION_MATCH_FLAG_MARK (1<<11)
					/* skb mark of the packet */
#define SFE_IPV4_CONNECTION_MATCH_FLAG_INSERT_EGRESS_VLAN_TAG (1<<12)
					/* Insert VLAN tag */
#define SFE_IPV4_CONNECTION_MATCH_FLAG_SRC_INTERFACE_CHECK (1<<13)
					/* Source interface check */
#define SFE_IPV4_CONNECTION_MATCH_FLAG_PASSTHROUGH (1<<14)
					/* passthrough flow: encap/decap to be skipped for this flow */
#define SFE_IPV4_CONNECTION_MATCH_FLAG_FAST_XMIT (1<<15)
					/* skb go fast xmit */
#define SFE_IPV4_CONNECTION_MATCH_FLAG_FAST_XMIT_FLOW_CHECKED (1<<16)
					/* Fast xmit flow checked or not */
#define SFE_IPV4_CONNECTION_MATCH_FLAG_FAST_XMIT_DEV_ADMISSION (1<<17)
					/* Fast xmit may be possible for this flow, if SFE check passes */
#define SFE_IPV4_CONNECTION_MATCH_FLAG_SRC_INTERFACE_CHECK_NO_FLUSH (1<<18)
					/* Source interface check but do not flush the connection */
#define SFE_IPV4_CONNECTION_MATCH_FLAG_INSERT_EGRESS_TRUSTSEC_SGT (1<<19)
					/* Insert Trustsec SGT */
#define SFE_IPV4_CONNECTION_MATCH_FLAG_FAST_QDISC_XMIT (1<<20)
					/* Fast Qdisc transmit enabled */
#define SFE_IPV4_CONNECTION_MATCH_FLAG_MULTICAST (1<<21)
					/* Multicast flow*/
#define SFE_IPV4_CONNECTION_MATCH_FLAG_MULTICAST_CHANGED (1<<22)
					/* Multicast flow changed the data */
#define SFE_IPV4_CONNECTION_MATCH_FLAG_TSO_ENABLE (1<<23)
					/* TSO enabled for dest dev */
#define SFE_IPV4_CONNECTION_MATCH_FLAG_PACKET_HOST (1<<24)
					/* Packet Host Type Set */
#define SFE_IPV4_CONNECTION_MATCH_FLAG_FLS_DISABLED (1<<25)
					/* Don't send packets to FLS */
#define SFE_IPV4_CONNECTION_MATCH_FLAG_BRIDGE_VLAN_PASSTHROUGH (1<<26)
					/* Bridge Vlan passthrough enable */

/*
 * IPv4 multicast destination structure
 */
struct sfe_ipv4_mc_dest {
	struct list_head list;		/* Link in the list*/
	struct rcu_head rcu;            /* delay rcu free */
	uint32_t flags;				/* Bit flags associated with the rule */
	u16 xmit_dest_mac[ETH_ALEN / 2];	/* Destination MAC address to use when forwarding */
	u16 xmit_src_mac[ETH_ALEN / 2];		/* Src MAC address to use when forwarding */
	struct net_device *xmit_dev;		/* Transmit interface */
	netdev_features_t features;		/* Device features */

	uint32_t xlate_src_ip;			/* Translated flow IP address */
	uint32_t xlate_src_ident;		/* Translated flow ident (e.g. port) */
	uint32_t xlate_src_csum_adjustment;	/* Transport layer checksum adjustment after source translation */
	u16 xlate_src_partial_csum_adjustment;	/* Increase compute checksum adjustment */
	u16 l2_hdr_size;			/* l2 hdr length */

	uint16_t if_mac[3];			/* Interface MAC address */
	u16 pppoe_session_id;			/* PPPOE header session id */
	u8 pppoe_remote_mac[ETH_ALEN];		/* PPPOE remote mac address */
	u8 egress_vlan_hdr_cnt;			/* Active egress vlan headers count */
	u8 reserved[1];				/* Reserved for alignment */
	struct sfe_vlan_hdr egress_vlan_hdr[SFE_MAX_VLAN_DEPTH];
						/* VLAN Tag stack for the egress packets */
};

/*
 * IPv4 connection matching structure.
 */
struct sfe_ipv4_connection_match {
	/*
	 * References to other objects.
	 */
	struct hlist_node hnode;

	/*
	 * Characteristics that identify flows that match this rule.
	 */
	__be32 match_src_ip;		/* Source IP address */
	__be32 match_dest_ip;		/* Destination IP address */
	__be16 match_src_port;		/* Source port/connection ident */
	__be16 match_dest_port;		/* Destination port/connection ident */
	u8 match_protocol;		/* Protocol */
	u32 flags;			/* Bit flags */
	u16 xmit_dest_mac[ETH_ALEN / 2];
					/* Destination MAC address to use when forwarding */
	u16 xmit_src_mac[ETH_ALEN / 2];
					/* Source MAC address to use when forwarding */
	struct net_device *xmit_dev;	/* Network device on which to transmit */
	struct net_device *qdisc_xmit_dev;      /* Interface used for fast qdisc xmit */

	/*
	 * xmit device's feature
	 */
	netdev_features_t features;
	/*
	 * Packet transmit information.
	 */
	unsigned short int xmit_dev_mtu;
					/* Interface MTU */

	/*
	 * Size of all needed L2 headers
	 */
	u16 l2_hdr_size;

	/*
	 * Stats recorded in a sync period. These stats will be added to
	 * rx_packet_count64/rx_byte_count64 after a sync period.
	 */
	atomic_t rx_packet_count;
	atomic_t rx_byte_count;

	struct net_device *match_dev;	/* Network device */

	/*
	 * Control the operations of the match.
	 */
#ifdef CONFIG_NF_FLOW_COOKIE
	u32 flow_cookie;		/* used flow cookie, for debug */
#endif
#ifdef CONFIG_XFRM
	u32 flow_accel;             /* The flow accelerated or not */
#endif

	struct sfe_ipv4_connection *connection;
	struct sfe_ipv4_connection_match *counter_match;
					/* Matches the flow in the opposite direction as the one in *connection */

	/*
	 * Connection state that we track once we match.
	 */
	union {				/* Protocol-specific state */
		struct sfe_ipv4_tcp_connection_match tcp;
	} protocol_state;

	struct udp_sock *up;		/* Stores UDP sock information; valid only in decap path */

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 13, 0))
	const struct net_protocol *proto;	/* stores protocol handler; valid only in decap path */
#else
	struct net_protocol *proto;	/* stores protocol handler; valid only in decap path */
#endif

	/*
	 * VLAN headers
	 */
	struct sfe_vlan_hdr ingress_vlan_hdr[SFE_MAX_VLAN_DEPTH];
	struct sfe_vlan_hdr egress_vlan_hdr[SFE_MAX_VLAN_DEPTH];

#ifdef SFE_BRIDGE_VLAN_FILTERING_ENABLE
	/*
	 * Bridge VLAN Filter information per connection match entry.
	 */
	struct sfe_ipv4_vlan_filter_connection_match vlan_filter_rule;
#endif

	/*
	 * trustsec headers
	 */
	struct sfe_trustsec_hdr ingress_trustsec_hdr;
	struct sfe_trustsec_hdr egress_trustsec_hdr;

	/*
	 * Packet translation information.
	 */
	__be32 xlate_src_ip;		/* Address after source translation */
	__be16 xlate_src_port;	/* Port/connection ident after source translation */
	u16 xlate_src_csum_adjustment;
					/* Transport layer checksum adjustment after source translation */
	u16 xlate_src_partial_csum_adjustment;
					/* Transport layer pseudo header checksum adjustment after source translation */

	__be32 xlate_dest_ip;		/* Address after destination translation */
	__be16 xlate_dest_port;	/* Port/connection ident after destination translation */
	u16 xlate_dest_csum_adjustment;
					/* Transport layer checksum adjustment after destination translation */
	u16 xlate_dest_partial_csum_adjustment;
					/* Transport layer pseudo header checksum adjustment after destination translation */
	/*
	 * QoS information
	 */
	u32 priority;
	u32 dscp;
	u32 mark;			/* mark for outgoing packet */
	u8 svc_id;			/* service_class for the flow */
#if defined(SFE_PPE_QOS_SUPPORTED)
	u8 int_pri;			/* INT_PRI value of PPE QDISC */
#endif

	u8 ingress_vlan_hdr_cnt;        /* Ingress active vlan headers count */
	u8 egress_vlan_hdr_cnt;         /* Egress active vlan headers count */

	/*
	 * Summary stats.
	 */
	u64 rx_packet_count64;
	u64 rx_byte_count64;

	/*
	 * PPPoE information
	 */
	u16 pppoe_session_id;
	u8 pppoe_remote_mac[ETH_ALEN];

	/*
	 * Multicast destination device list
	 */
	struct list_head mc_list;

	struct net_device *top_interface_dev;	/* Used by tun6rd to store decap VLAN netdevice.*/

	void *fls_conn;

	bool sawf_valid;		/* Indicates mark has valid SAWF information */
	bool ingress_trustsec_valid;	/* Indicates trustsec header is valid */
};

/*
 * Per-connection data structure.
 */
struct sfe_ipv4_connection {
	struct sfe_ipv4_connection *next;
					/* Pointer to the next entry in a hash chain */
	struct sfe_ipv4_connection *prev;
					/* Pointer to the previous entry in a hash chain */
	int protocol;			/* IP protocol number */
	__be32 src_ip;			/* Src IP addr pre-translation */
	__be32 src_ip_xlate;		/* Src IP addr post-translation */
	__be32 dest_ip;			/* Dest IP addr pre-translation */
	__be32 dest_ip_xlate;		/* Dest IP addr post-translation */
	__be16 src_port;		/* Src port pre-translation */
	__be16 src_port_xlate;		/* Src port post-translation */
	__be16 dest_port;		/* Dest port pre-translation */
	__be16 dest_port_xlate;		/* Dest port post-translation */
	struct sfe_ipv4_connection_match *original_match;
					/* Original direction matching structure */
	struct net_device *original_dev;
					/* Original direction source device */
	struct sfe_ipv4_connection_match *reply_match;
					/* Reply direction matching structure */
	struct net_device *reply_dev;	/* Reply direction source device */
	u64 last_sync_jiffies;		/* Jiffies count for the last sync */
	struct sfe_ipv4_connection *all_connections_next;
					/* Pointer to the next entry in the list of all connections */
	struct sfe_ipv4_connection *all_connections_prev;
					/* Pointer to the previous entry in the list of all connections */
	u32 debug_read_seq;		/* sequence number for debug dump */
	bool removed;			/* Indicates the connection is removed */
	struct rcu_head rcu;		/* delay rcu free */
};

/*
 * IPv4 connections and hash table size information.
 */
#define SFE_IPV4_CONNECTION_HASH_SHIFT 12
#define SFE_IPV4_CONNECTION_HASH_SIZE (1 << SFE_IPV4_CONNECTION_HASH_SHIFT)
#define SFE_IPV4_CONNECTION_HASH_MASK (SFE_IPV4_CONNECTION_HASH_SIZE - 1)

enum sfe_ipv4_exception_events {
	SFE_IPV4_EXCEPTION_EVENT_UDP_HEADER_INCOMPLETE,
	SFE_IPV4_EXCEPTION_EVENT_UDP_NO_CONNECTION,
	SFE_IPV4_EXCEPTION_EVENT_UDP_IP_OPTIONS_OR_INITIAL_FRAGMENT,
	SFE_IPV4_EXCEPTION_EVENT_UDP_SMALL_TTL,
	SFE_IPV4_EXCEPTION_EVENT_UDP_NEEDS_FRAGMENTATION,
	SFE_IPV4_EXCEPTION_EVENT_TCP_HEADER_INCOMPLETE,
	SFE_IPV4_EXCEPTION_EVENT_TCP_NO_CONNECTION_SLOW_FLAGS,
	SFE_IPV4_EXCEPTION_EVENT_TCP_NO_CONNECTION_FAST_FLAGS,
	SFE_IPV4_EXCEPTION_EVENT_TCP_IP_OPTIONS_OR_INITIAL_FRAGMENT,
	SFE_IPV4_EXCEPTION_EVENT_TCP_SMALL_TTL,
	SFE_IPV4_EXCEPTION_EVENT_TCP_NEEDS_FRAGMENTATION,
	SFE_IPV4_EXCEPTION_EVENT_TCP_FLAGS,
	SFE_IPV4_EXCEPTION_EVENT_TCP_SEQ_EXCEEDS_RIGHT_EDGE,
	SFE_IPV4_EXCEPTION_EVENT_TCP_SMALL_DATA_OFFS,
	SFE_IPV4_EXCEPTION_EVENT_TCP_BAD_SACK,
	SFE_IPV4_EXCEPTION_EVENT_TCP_BIG_DATA_OFFS,
	SFE_IPV4_EXCEPTION_EVENT_TCP_SEQ_BEFORE_LEFT_EDGE,
	SFE_IPV4_EXCEPTION_EVENT_TCP_ACK_EXCEEDS_RIGHT_EDGE,
	SFE_IPV4_EXCEPTION_EVENT_TCP_ACK_BEFORE_LEFT_EDGE,
	SFE_IPV4_EXCEPTION_EVENT_ICMP_HEADER_INCOMPLETE,
	SFE_IPV4_EXCEPTION_EVENT_ICMP_UNHANDLED_TYPE,
	SFE_IPV4_EXCEPTION_EVENT_ICMP_IPV4_HEADER_INCOMPLETE,
	SFE_IPV4_EXCEPTION_EVENT_ICMP_IPV4_NON_V4,
	SFE_IPV4_EXCEPTION_EVENT_ICMP_IPV4_IP_OPTIONS_INCOMPLETE,
	SFE_IPV4_EXCEPTION_EVENT_ICMP_IPV4_UDP_HEADER_INCOMPLETE,
	SFE_IPV4_EXCEPTION_EVENT_ICMP_IPV4_TCP_HEADER_INCOMPLETE,
	SFE_IPV4_EXCEPTION_EVENT_ICMP_IPV4_UNHANDLED_PROTOCOL,
	SFE_IPV4_EXCEPTION_EVENT_ICMP_NO_CONNECTION,
	SFE_IPV4_EXCEPTION_EVENT_ICMP_FLUSHED_CONNECTION,
	SFE_IPV4_EXCEPTION_EVENT_HEADER_INCOMPLETE,
	SFE_IPV4_EXCEPTION_EVENT_HEADER_CSUM_BAD,
	SFE_IPV4_EXCEPTION_EVENT_BAD_TOTAL_LENGTH,
	SFE_IPV4_EXCEPTION_EVENT_NON_V4,
	SFE_IPV4_EXCEPTION_EVENT_NON_INITIAL_FRAGMENT,
	SFE_IPV4_EXCEPTION_EVENT_DATAGRAM_INCOMPLETE,
	SFE_IPV4_EXCEPTION_EVENT_IP_OPTIONS_INCOMPLETE,
	SFE_IPV4_EXCEPTION_EVENT_UNHANDLED_PROTOCOL,
	SFE_IPV4_EXCEPTION_EVENT_NO_HEADROOM,
	SFE_IPV4_EXCEPTION_EVENT_UNCLONE_FAILED,
	SFE_IPV4_EXCEPTION_EVENT_INVALID_PPPOE_SESSION,
	SFE_IPV4_EXCEPTION_EVENT_INCORRECT_PPPOE_PARSING,
	SFE_IPV4_EXCEPTION_EVENT_PPPOE_NOT_SET_IN_CME,
	SFE_IPV4_EXCEPTION_EVENT_PPPOE_BR_NOT_IN_CME,
	SFE_IPV4_EXCEPTION_EVENT_INGRESS_VLAN_TAG_MISMATCH,
	SFE_IPV4_EXCEPTION_EVENT_INVALID_SRC_IFACE,
	SFE_IPV4_EXCEPTION_EVENT_TUN6RD_NO_CONNECTION,
	SFE_IPV4_EXCEPTION_EVENT_TUN6RD_NEEDS_FRAGMENTATION,
	SFE_IPV4_EXCEPTION_EVENT_TUN6RD_SYNC_ON_FIND,
	SFE_IPV4_EXCEPTION_EVENT_TUN6RD_SMALL_TTL,
	SFE_IPV4_EXCEPTION_EVENT_GRE_HEADER_INCOMPLETE,
	SFE_IPV4_EXCEPTION_EVENT_GRE_NO_CONNECTION,
	SFE_IPV4_EXCEPTION_EVENT_GRE_IP_OPTIONS_OR_INITIAL_FRAGMENT,
	SFE_IPV4_EXCEPTION_EVENT_GRE_SMALL_TTL,
	SFE_IPV4_EXCEPTION_EVENT_GRE_NEEDS_FRAGMENTATION,
	SFE_IPV4_EXCEPTION_EVENT_ESP_NO_CONNECTION,
	SFE_IPV4_EXCEPTION_EVENT_ESP_IP_OPTIONS_OR_INITIAL_FRAGMENT,
	SFE_IPV4_EXCEPTION_EVENT_ESP_NEEDS_FRAGMENTATION,
	SFE_IPV4_EXCEPTION_EVENT_ESP_SMALL_TTL,
	SFE_IPV4_EXCEPTION_EVENT_INGRESS_TRUSTSEC_SGT_MISMATCH,
	SFE_IPV4_EXCEPTION_EVENT_GSO_NOT_SUPPORTED,
	SFE_IPV4_EXCEPTION_EVENT_TSO_SEG_MAX_NOT_SUPPORTED,
	SFE_IPV4_EXCEPTION_EVENT_L2TPV3_NO_CONNECTION,
	SFE_IPV4_EXCEPTION_EVENT_L2TPV3_IP_OPTIONS_OR_INITIAL_FRAGMENT,
	SFE_IPV4_EXCEPTION_EVENT_L2TPV3_SMALL_TTL,
	SFE_IPV4_EXCEPTION_EVENT_L2TPV3_NEEDS_FRAGMENTATION,
	SFE_IPV4_EXCEPTION_EVENT_LAST
};

/*
 * per CPU stats
 */
struct sfe_ipv4_stats {
	/*
	 * Stats recorded in a sync period. These stats will be added to
	 * connection_xxx64 after a sync period.
	 */
	u64 connection_create_requests64;
					/* Number of IPv4 connection create requests */
	u64 connection_create_collisions64;
					/* Number of IPv4 connection create requests that collided with existing hash table entries */
	u64 connection_create_failures64;
					/* Number of IPv4 connection create requests that failed */
	u64 connection_destroy_requests64;
					/* Number of IPv4 connection destroy requests */
	u64 connection_destroy_misses64;
					/* Number of IPv4 connection destroy requests that missed our hash table */
	u64 connection_match_hash_hits64;
					/* Number of IPv4 connection match hash hits */
	u64 connection_match_hash_reorders64;
					/* Number of IPv4 connection match hash reorders */
	u64 connection_flushes64;		/* Number of IPv4 connection flushes */
	u64 packets_dropped64;			/* Number of IPv4 packets dropped */
	u64 packets_forwarded64;		/* Number of IPv4 packets forwarded */
	u64 packets_fast_xmited64;		/* Number of IPv4 packets fast transmited */
	u64 packets_fast_qdisc_xmited64;	/* Number of IPv4 packets that used fast qdisc xmit */
	u64 packets_not_forwarded64;	/* Number of IPv4 packets not forwarded */
	u64 exception_events64[SFE_IPV4_EXCEPTION_EVENT_LAST];
	u64 pppoe_encap_packets_forwarded64;	/* Number of IPv4 PPPoE encap packets forwarded */
	u64 pppoe_decap_packets_forwarded64;	/* Number of IPv4 PPPoE decap packets forwarded */
	u64 pppoe_bridge_packets_forwarded64;	/* Number of IPv4 PPPoE bridge packets forwarded */
	u64 pppoe_bridge_packets_3tuple_forwarded64;    /* Number of IPv4 PPPoE bridge packets forwarded based on 3-tuple info */
	u64 connection_create_requests_overflow64;	/* Number of IPV4 connection create requests after reaching max limit */
	u64 bridge_vlan_passthorugh_forwarded64;	/* Number of IPV4 bridge vlan passthrough packets forwarded */
};

/*
 * sfe_ipv4_per_service_class_stats
 *	Per service class stats
 */
struct sfe_ipv4_per_service_class_stats {
	u64 tx_bytes;		/* Byte count */
	u64 tx_packets;		/* Packet count */
	seqcount_t seq;		/* seq lock for read/write protection */
	/*
	 * TODO : add entries to be collected later.
	 */
};

/*
 * sfe_ipv4_service_class_stats_db
 *	stat entries for each service class.
 */
struct sfe_ipv4_service_class_stats_db {
	struct sfe_ipv4_per_service_class_stats psc_stats[SFE_MAX_SERVICE_CLASS_ID + 1];
				/*  Per service class stats */
};

/*
 * Per-module structure.
 */
struct sfe_ipv4 {
	spinlock_t lock;		/* Lock for SMP correctness */
	struct sfe_ipv4_connection *all_connections_head;
					/* Head of the list of all connections */
	struct sfe_ipv4_connection *all_connections_tail;
					/* Tail of the list of all connections */
	unsigned int num_connections;	/* Number of connections */
	struct delayed_work sync_dwork;	/* Work to sync the statistics */
	unsigned int work_cpu;		/* The core to run stats sync on */

	sfe_sync_rule_callback_t __rcu sync_rule_callback;
	sfe_ipv4_many_sync_callback_t __rcu many_sync_callback;
					/* Callback function registered by a connection manager for stats syncing */
	struct sfe_ipv4_connection *conn_hash[SFE_IPV4_CONNECTION_HASH_SIZE];
					/* Connection hash table */

	struct hlist_head hlist_conn_match_hash_head[SFE_IPV4_CONNECTION_HASH_SIZE];
					/* Connection match hash table */

#ifdef CONFIG_NF_FLOW_COOKIE
	struct sfe_flow_cookie_entry sfe_flow_cookie_table[SFE_FLOW_COOKIE_SIZE];
					/* flow cookie table*/
	flow_cookie_set_func_t flow_cookie_set_func;
					/* function used to configure flow cookie in hardware*/
	int flow_cookie_enable;
					/* Enable/disable flow cookie at runtime */
#endif
	struct sfe_ipv4_service_class_stats_db __percpu *stats_pcpu_psc;
					/* Database to maintain per cpu per service class statistics */

	struct sfe_ipv4_stats __percpu *stats_pcpu;
					/* Per CPU statistics. */

	struct sfe_ipv4_connection *wc_next; /* Connection list walk pointer for stats sync */

	/*
	 * Control state.
	 */
	struct kobject *sys_ipv4;	/* sysfs linkage */
	int debug_dev;			/* Major number of the debug char device */
	u32 debug_read_seq;	/* sequence number for debug dump */
};

/*
 * Enumeration of the XML output.
 */
enum sfe_ipv4_debug_xml_states {
	SFE_IPV4_DEBUG_XML_STATE_START,
	SFE_IPV4_DEBUG_XML_STATE_CONNECTIONS_START,
	SFE_IPV4_DEBUG_XML_STATE_CONNECTIONS_CONNECTION,
	SFE_IPV4_DEBUG_XML_STATE_CONNECTIONS_END,
	SFE_IPV4_DEBUG_XML_STATE_EXCEPTIONS_START,
	SFE_IPV4_DEBUG_XML_STATE_EXCEPTIONS_EXCEPTION,
	SFE_IPV4_DEBUG_XML_STATE_EXCEPTIONS_END,
	SFE_IPV4_DEBUG_XML_STATE_STATS,
	SFE_IPV4_DEBUG_XML_STATE_END,
	SFE_IPV4_DEBUG_XML_STATE_DONE
};

/*
 * XML write state.
 */
struct sfe_ipv4_debug_xml_write_state {
	enum sfe_ipv4_debug_xml_states state;
					/* XML output file state machine state */
	int iter_exception;		/* Next exception iterator */
        char *msg;			/* The message written / being returned to the reader */
        char *msgp;                     /* Points into the msg buffer as we output it to the reader piece by piece */
        int msg_len;                    /* Length of the msg buffer still to be written out */
};

typedef int (*sfe_ipv4_debug_xml_write_method_t)(struct sfe_ipv4 *si, struct sfe_ipv4_debug_xml_write_state *ws, char *buffer, size_t length,
						  int *total_read);

void sfe_ipv4_fls_clear(void);
u16 sfe_ipv4_gen_ip_csum(struct iphdr *iph);
bool sfe_ipv4_service_class_stats_get(uint8_t sid, uint64_t *bytes, uint64_t *packets);
void sfe_ipv4_service_class_stats_inc(struct sfe_ipv4 *si, uint8_t sid, uint64_t bytes);
void sfe_ipv4_exception_stats_inc(struct sfe_ipv4 *si, enum sfe_ipv4_exception_events reason);
bool sfe_ipv4_remove_connection(struct sfe_ipv4 *si, struct sfe_ipv4_connection *c);
void sfe_ipv4_flush_connection(struct sfe_ipv4 *si, struct sfe_ipv4_connection *c, sfe_sync_reason_t reason);
void sfe_ipv4_sync_status(struct sfe_ipv4 *si, struct sfe_ipv4_connection *c, sfe_sync_reason_t reason);
#if defined(SFE_RFS_SUPPORTED)
void sfe_ipv4_fill_connection_dev(struct sfe_ipv4_rule_destroy_msg *msg, struct net_device **original_dev, struct net_device **reply_dev);
#endif
struct sfe_ipv4_connection_match *
sfe_ipv4_find_connection_match_rcu(struct sfe_ipv4 *si, struct net_device *dev, u8 protocol,
					__be32 src_ip, __be16 src_port,
					__be32 dest_ip, __be16 dest_port);

void sfe_ipv4_exit(void);
int sfe_ipv4_init(void);
bool sfe_ipv4_cancel_delayed_work_sync(void);

#endif /* __SFE_IPV4_H */
