/*
 **************************************************************************
 * Copyright (c) 2014-2015, The Linux Foundation.  All rights reserved.
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

struct ecm_tracker_instance;

/*
 * Data tracking data limits - system global and per-connection - defaults.
 */
#define ECM_TRACKER_GLOBAL_DATA_LIMIT_DEFAULT (1024 * 1024 * 8)
#define ECM_TRACKER_GLOBAL_DATA_BUFFER_LIMIT_DEFAULT (1024 * 1024 * 64)
#define ECM_TRACKER_CONNECTION_TRACKING_LIMIT_DEFAULT (1024 * 1024)
#define ECM_TRACKER_CONNECTION_TRACKING_LIMIT_MAX ECM_TRACKER_GLOBAL_DATA_LIMIT_DEFAULT

/*
 * Definitions for IPv6 version-class-flow_label field.
 */
#define ECM_TRACKER_IPV6_FLOW_LBL_PRIORITY_MASK 0xC0
#define ECM_TRACKER_IPV6_FLOW_LBL_PRIORITY_SHIFT 6
#define ECM_TRACKER_IPV6_PRIORITY_SHIFT 2

enum ecm_tracker_sender_types {
	ECM_TRACKER_SENDER_TYPE_SRC = 0,		/* Sender of tracked data is the source of the connection (who established the connection) */
	ECM_TRACKER_SENDER_TYPE_DEST = 1,	/* Sender of tracked data is the destination of the connection (to whom connection was established) */
	ECM_TRACKER_SENDER_MAX,			/* MUST BE LAST */
};
typedef enum ecm_tracker_sender_types ecm_tracker_sender_type_t;

/*
 * enum ecm_tracker_sender_states
 *	Notional states of senders of a tracker
 *
 * Order is important here - don't change them as logic depends on their numerical value.
 */
enum ecm_tracker_sender_states {
	ECM_TRACKER_SENDER_STATE_UNKNOWN = 0,		/* Endpoint has not sent any packets yet */
	ECM_TRACKER_SENDER_STATE_ESTABLISHING,		/* Endpoint has not yet given any indication it is established */
	ECM_TRACKER_SENDER_STATE_ESTABLISHED,		/* Endpoint has indicated that it is established */
	ECM_TRACKER_SENDER_STATE_CLOSING,		/* Endpoint has indicated that it wants to close down its side of the connection */
	ECM_TRACKER_SENDER_STATE_CLOSED,		/* Endpoint has closed, connection remains to service any late packets */
	ECM_TRACKER_SENDER_STATE_FAULT,			/* Endpoint experienced a fault */
	ECM_TRACKER_SENDER_STATE_MAX,			/* MUST BE LAST */
};
typedef enum ecm_tracker_sender_states ecm_tracker_sender_state_t;

/*
 * ecm_tracker_sender_state_to_string[]
 *	Convert a sender state to a string
 */
const char *
ecm_tracker_sender_state_to_string(enum ecm_tracker_sender_states);

/*
 * enum ecm_tracker_connection_states
 *	Notional states of connection being monitored by the tracker
 *
 * Order is important here - don't change them as logic depends on their numerical value.
 */
enum ecm_tracker_connection_states {
	ECM_TRACKER_CONNECTION_STATE_ESTABLISHING = 0,	/* Not yet given any indication it is established */
	ECM_TRACKER_CONNECTION_STATE_ESTABLISHED,	/* It is established */
	ECM_TRACKER_CONNECTION_STATE_CLOSING,		/* Connection has begun process of closing */
	ECM_TRACKER_CONNECTION_STATE_CLOSED,		/* Has closed, connection remains to service any late packets */
	ECM_TRACKER_CONNECTION_STATE_FAULT,		/* Experienced a fault */
	ECM_TRACKER_CONNECTION_STATE_MAX,		/* MUST BE LAST */
};
typedef enum ecm_tracker_connection_states ecm_tracker_connection_state_t;

/*
 * ecm_tracker_connection_state_to_string
 *	Convert a connection state to a string
 */
const char *ecm_tracker_connection_state_to_string(enum ecm_tracker_connection_states);

/*
 * enum ecm_tracker_ip_protocol_types
 *	A list of protocol types that can be recorded in the ecm_ip_header
 *
 * This is especially useful for IPv6 where the ip header can contain many sub headers.
 * But it is also useful for IPv4 where you might have IP following a GRE header, for example.
 * An ECM IP header may record only ONE of each type of header, if more are found the header is considered invalid.
 *
 * These constants are used to index into the ecm_tracker_ip_header.headers[]
 */
enum ecm_tracker_ip_protocol_types {
	ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN,		/* A protocol that is unrecognised */
	ECM_TRACKER_IP_PROTOCOL_TYPE_ICMP,
	ECM_TRACKER_IP_PROTOCOL_TYPE_UDP,
	ECM_TRACKER_IP_PROTOCOL_TYPE_TCP,
	ECM_TRACKER_IP_PROTOCOL_TYPE_GRE,
#ifdef ECM_IPV6_ENABLE
	ECM_TRACKER_IP_PROTOCOL_TYPE_IPV6_ROUTING,
	ECM_TRACKER_IP_PROTOCOL_TYPE_IPV6_FRAGMENT,
	ECM_TRACKER_IP_PROTOCOL_TYPE_AH,
	ECM_TRACKER_IP_PROTOCOL_TYPE_IPV6_ICMP,
	ECM_TRACKER_IP_PROTOCOL_TYPE_IPV6_DO,
	ECM_TRACKER_IP_PROTOCOL_TYPE_IPV6_HBH,		/* IPv6 hop-by-hop header */
#endif
	ECM_TRACKER_IP_PROTOCOL_TYPE_COUNT		/* Must be last, do not use */
};
typedef enum ecm_tracker_ip_protocol_types ecm_tracker_ip_protocol_type_t;

/*
 * struct ecm_tracker_ip_protocol_header
 *	Records a protocol header as stored within an IP datagram
 */
struct ecm_tracker_ip_protocol_header {
	uint8_t protocol_number;		/* IP protocol number */
	uint16_t header_size;			/* Size of the protocol header */
	uint16_t size;				/* Size of the header_size + its payload */
	uint16_t offset;			/* Offset from the start of the skb where this header is located */
};

/*
 * struct ecm_tracker_ip_header
 *	An IP header in the ECM can deal with V4 and V6 headers.
 *
 * WARNING: An ecm_ip_header is ONLY VALID while the skb from which it was initialised remains untouched.
 */
struct ecm_tracker_ip_header {
	/*
	 * h is a union of IP version headers.
	 * These are ONLY used as buffers where skb_header_pointer() needs them to perform a skb_copy_bits() operation.
	 * WARNING: You should NOT rely on the content of these structures because skb_header_pointer() may not have used them!
	 * Use the actual fields below instead.
	 */
	union {
		struct iphdr v4_hdr;
#ifdef ECM_IPV6_ENABLE
		struct ipv6hdr v6_hdr;
#endif
	} h;

	struct sk_buff *skb;		/* COPY of POINTER to the skb this header relates to.  This ecm_tracker_ip_header is ONLY VALID for as long as the skb it relates to remains UNTOUCHED */
	bool is_v4;			/* True when v4, else v6 */
	ip_addr_t src_addr;		/* ECM ip address equivalent */
	ip_addr_t dest_addr;		/* ECM ip address equivalent */
	int protocol;			/* The upper layer transport protocol */
	bool fragmented;		/* True when fragmented */
	uint8_t dscp;			/* DSCP field from the packet */
	uint8_t ds;			/* DS field from packet */
	uint8_t ttl;			/* v4 TTL or v6 hop limit */
	uint32_t ip_header_length;	/* Length of the IP header plus any variable sized intrinsically attached options */
	uint32_t total_length;		/* total length of IP header including all extensions and payload.  For v4 this is total_len, for v6 this is payload_len + size of the IP 6 header */
	uint32_t payload_length;	/* total_length - ip_header_length */
	struct ecm_tracker_ip_protocol_header headers[ECM_TRACKER_IP_PROTOCOL_TYPE_COUNT];
					/* Use one of the ECM_TRACKER_IP_PROTOCOL_TYPE_XYZ constants to index into this to locate the header you want to inspect.  If the size is zero then the header was not found. */
};

#ifdef ECM_TRACKER_DPI_SUPPORT_ENABLE
typedef int32_t (*ecm_tracker_datagram_count_get_method_t)(struct ecm_tracker_instance *ti, ecm_tracker_sender_type_t sender);
													/* Return number of available datagrams sent by the sender */
typedef void (*ecm_tracker_datagram_discard_method_t)(struct ecm_tracker_instance *ti, ecm_tracker_sender_type_t sender, int32_t n);
													/* Discard n number of datagrams at the head of the datagram list that were sent by the sender */
typedef int32_t (*ecm_tracker_datagram_size_get_method_t)(struct ecm_tracker_instance *ti, ecm_tracker_sender_type_t sender, int32_t i);
													/* Return size in bytes of datagram at index i that was sent by the sender  */
typedef int (*ecm_tracker_datagram_read_method_t)(struct ecm_tracker_instance *ti, ecm_tracker_sender_type_t sender, int32_t i, int32_t offset, int32_t size, void *buffer);
													/* Read size bytes from datagram at index i into the buffer */
typedef bool (*ecm_tracker_datagram_add_method_t)(struct ecm_tracker_instance *ti, ecm_tracker_sender_type_t sender, struct sk_buff *skb);
													/* Add (append) the datagram into the tracker */
typedef void (*ecm_tracker_discard_all_method_t)(struct ecm_tracker_instance *ti);
													/* Discard all tracked data */
typedef int32_t (*ecm_tracker_data_total_get_method_t)(struct ecm_tracker_instance *ti);
													/* Return number of bytes of tracked data in total */
typedef int32_t (*ecm_tracker_data_limit_get_method_t)(struct ecm_tracker_instance *ti);
													/* Return the limit on the number of bytes we can track */
typedef void (*ecm_tracker_data_limit_set_method_t)(struct ecm_tracker_instance *ti, int32_t data_limit);
													/* Set the limit on the number of bytes we can track */
#endif
typedef void (*ecm_tracker_ref_method_t)(struct ecm_tracker_instance *ti);
typedef int (*ecm_tracker_deref_method_t)(struct ecm_tracker_instance *ti);

typedef void (*ecm_tracker_state_update_method_t)(struct ecm_tracker_instance *ti, ecm_tracker_sender_type_t sender, struct ecm_tracker_ip_header *ip_hdr, struct sk_buff *skb);
													/* Update state of the sender */
typedef void (*ecm_tracker_state_get_method_t)(struct ecm_tracker_instance *ti, ecm_tracker_sender_state_t *src_state, ecm_tracker_sender_state_t *dest_state, ecm_tracker_connection_state_t *state, ecm_db_timer_group_t *tg);
													/* State of the connection */
#ifdef ECM_STATE_OUTPUT_ENABLE
typedef int (*ecm_tracker_state_get_callback_t)(struct ecm_tracker_instance *ti, struct ecm_state_file_instance *sfi);
											/*
											 * Get state output.
											 * Returns zero on success.
											 */
#endif

/*
 * struct ecm_tracker_instance
 *	Base class of all trackers
 *
 * ALL trackers must implement these features in addition to their own.
 * ALL trackers must be castable to a type of this, i.e. this structure must be the first element of their own data type.
 */
struct ecm_tracker_instance {
#ifdef ECM_TRACKER_DPI_SUPPORT_ENABLE
	ecm_tracker_data_total_get_method_t data_total_get;
	ecm_tracker_data_limit_get_method_t data_limit_get;
	ecm_tracker_data_limit_set_method_t data_limit_set;
	ecm_tracker_datagram_count_get_method_t datagram_count_get;
	ecm_tracker_datagram_discard_method_t datagram_discard;
	ecm_tracker_datagram_size_get_method_t datagram_size_get;
	ecm_tracker_datagram_read_method_t datagram_read;
	ecm_tracker_datagram_add_method_t datagram_add;
	ecm_tracker_discard_all_method_t discard_all;
#endif
	ecm_tracker_state_update_method_t state_update;
	ecm_tracker_state_get_method_t state_get;
#ifdef ECM_STATE_OUTPUT_ENABLE
	ecm_tracker_state_get_callback_t state_text_get;		/* Return text containing its state */
#endif
	ecm_tracker_ref_method_t ref;
	ecm_tracker_deref_method_t deref;
};

bool ecm_tracker_ip_check_header_and_read(struct ecm_tracker_ip_header *ip_hdr, struct sk_buff *skb);
#ifdef ECM_TRACKER_DPI_SUPPORT_ENABLE
uint32_t ecm_tracker_data_limit_get(void);
void ecm_tracker_data_limit_set(uint32_t limit);
uint32_t ecm_tracker_data_total_get(void);
uint32_t ecm_tracker_data_buffer_total_get(void);
bool ecm_tracker_data_total_increase(uint32_t n, uint32_t data_bufer_size);
void ecm_tracker_data_total_decrease(uint32_t n, uint32_t data_bufer_size);
#endif

