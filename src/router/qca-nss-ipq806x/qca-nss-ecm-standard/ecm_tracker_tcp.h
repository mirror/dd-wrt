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

struct ecm_tracker_tcp_instance;

#ifdef ECM_TRACKER_DPI_SUPPORT_ENABLE
typedef uint32_t (*ecm_tracker_tcp_bytes_avail_get_method_t)(struct ecm_tracker_tcp_instance *tti, ecm_tracker_sender_type_t sender);
									/* Return number of bytes available to read */
typedef int (*ecm_tracker_tcp_bytes_read_method_t)(struct ecm_tracker_tcp_instance *tti, ecm_tracker_sender_type_t sender, uint32_t offset, uint32_t size, void *buffer);
									/* Read a number of bytes */
typedef void (*ecm_tracker_tcp_bytes_discard_method_t)(struct ecm_tracker_tcp_instance *tti, ecm_tracker_sender_type_t sender, uint32_t n);
									/* Discard n bytes from the available stream bytes */
typedef bool (*ecm_tracker_tcp_mss_get_method_t)(struct ecm_tracker_tcp_instance *tti, ecm_tracker_sender_type_t sender, uint16_t *mss);
									/* Get the MSS as sent BY the given target i.e. the MSS the other party is allowed to send */
typedef bool (*ecm_tracker_tcp_segment_add_method_t)(struct ecm_tracker_tcp_instance *tti, ecm_tracker_sender_type_t sender,
								struct ecm_tracker_ip_header *ip_hdr, struct ecm_tracker_ip_protocol_header *ecm_tcp_header, struct tcphdr *tcp_hdr, struct sk_buff *skb);
									/* Add a pre-checked segment */
#endif

struct ecm_tracker_tcp_instance {
	struct ecm_tracker_instance base;				/* MUST BE FIRST FIELD */

#ifdef ECM_TRACKER_DPI_SUPPORT_ENABLE
	ecm_tracker_tcp_bytes_avail_get_method_t bytes_avail_get;	/* Return number of bytes available to read */
	ecm_tracker_tcp_bytes_read_method_t bytes_read;			/* Read a number of bytes */
	ecm_tracker_tcp_bytes_discard_method_t bytes_discard;		/* Discard n number of bytes from the beginning of the stream */
	ecm_tracker_tcp_mss_get_method_t mss_get;			/* Get the MSS as sent BY the given target i.e. the maximum number of */
	ecm_tracker_tcp_segment_add_method_t segment_add;		/* Add a prechecked MSS segment */
#endif
};

/*
 * TCP tracker
 *	Records the two streams of bytes sent in either direction over a TCP connection
 */
struct tcphdr *ecm_tracker_tcp_check_header_and_read(struct sk_buff *skb, struct ecm_tracker_ip_header *ip_hdr, struct tcphdr *port_buffer);
void ecm_tracker_tcp_init(struct ecm_tracker_tcp_instance *tti, int32_t data_limit, uint16_t src_mss_default, uint16_t dest_mss_default);
struct ecm_tracker_tcp_instance *ecm_tracker_tcp_alloc(void);

#ifdef ECM_TRACKER_DPI_SUPPORT_ENABLE
/*
 * TCP Reader
 *	Faster reading of a stream of bytes.
 * The stream MUST NOT BE EMPTY and the stream MUST NOT have any data discarded from it for the duration of the reader.
 * It is fine for additional data to be added to the stream during reader use.
 * You must operate the reader within the byte space you know to be available for reading.
 *
 * When reading blocks of data it is better to use the tracker bytes_read() method instead.
 */
struct ecm_tracker_tcp_reader_instance;
void ecm_tracker_tcp_reader_discard_preceding(struct ecm_tracker_tcp_reader_instance *tri);
void ecm_tracker_tcp_reader_advance(struct ecm_tracker_tcp_reader_instance *tri, uint32_t advancement);
void ecm_tracker_tcp_reader_position_set(struct ecm_tracker_tcp_reader_instance *tri, uint32_t offset);
void ecm_tracker_tcp_reader_init(struct ecm_tracker_tcp_reader_instance *tri, struct ecm_tracker_tcp_instance *tti, ecm_tracker_sender_type_t sender);
uint8_t ecm_tracker_tcp_reader_fwd_read_u8(struct ecm_tracker_tcp_reader_instance *tri);
void ecm_tracker_tcp_reader_ref(struct ecm_tracker_tcp_reader_instance *tri);
int ecm_tracker_tcp_reader_deref(struct ecm_tracker_tcp_reader_instance *tri);
struct ecm_tracker_tcp_reader_instance *ecm_tracker_tcp_reader_alloc(void);
uint32_t ecm_tracker_tcp_reader_remain_get(struct ecm_tracker_tcp_reader_instance *tri);
uint32_t ecm_tracker_tcp_reader_position_get(struct ecm_tracker_tcp_reader_instance *tri);
void ecm_tracker_tcp_reader_retreat(struct ecm_tracker_tcp_reader_instance *tri, uint32_t retreatment);
#endif

