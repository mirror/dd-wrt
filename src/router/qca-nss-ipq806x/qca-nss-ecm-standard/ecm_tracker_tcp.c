/*
 **************************************************************************
 * Copyright (c) 2014-2015, 2020, The Linux Foundation.  All rights reserved.
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

#include <linux/version.h>
#include <linux/types.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/icmp.h>
#include <linux/sysctl.h>
#include <linux/kthread.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/pkt_sched.h>
#include <linux/string.h>
#include <net/route.h>
#include <net/ip.h>
#include <net/tcp.h>
#include <asm/unaligned.h>
#include <asm/uaccess.h>	/* for put_user */
#include <net/ipv6.h>
#include <linux/inet.h>
#include <linux/in.h>
#include <linux/udp.h>
#include <linux/tcp.h>

#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_bridge.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack_l4proto.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/ipv4/nf_conntrack_ipv4.h>
#include <net/netfilter/ipv4/nf_defrag_ipv4.h>

/*
 * Debug output levels
 * 0 = OFF
 * 1 = ASSERTS / ERRORS
 * 2 = 1 + WARN
 * 3 = 2 + INFO
 * 4 = 3 + TRACE
 */
#define DEBUG_LEVEL ECM_TRACKER_TCP_DEBUG_LEVEL

#include "ecm_types.h"
#include "ecm_db_types.h"
#include "ecm_state.h"
#include "ecm_tracker.h"
#include "ecm_tracker_tcp.h"

/*
 * TCP control flags.
 */
#define TCF_FIN 0x01
#define TCF_SYN 0x02
#define TCF_RST 0x04
#define TCF_PSH 0x08
#define TCF_ACK 0x10
#define TCF_URG 0x20

/*
 * Extract a byte tcp flags field from a union tcp_word_hdr.
 */
#define ECM_TRACKER_TCP_WORD_HDR_TO_FLAGS(word_hdr) ((ntohl(word_hdr->words[3]) >> 16) & 0xff)

/*
 * Valid TCP flag combinations.
 */
#define ECM_TRACKER_TCP_VALID_FLAGS_MAX ((TCF_URG | TCF_ACK | TCF_PSH | TCF_RST | TCF_SYN | TCF_FIN) + 1)

static uint32_t ecm_tracker_tcp_valid_flags[ECM_TRACKER_TCP_VALID_FLAGS_MAX] = {
	[TCF_FIN | TCF_RST | TCF_ACK] = 1,
	[TCF_FIN | TCF_PSH | TCF_ACK] = 1,
	[TCF_FIN | TCF_PSH | TCF_ACK | TCF_URG] = 1,
	[TCF_FIN | TCF_ACK] = 1,
	[TCF_FIN | TCF_ACK | TCF_URG] = 1,
	[TCF_SYN] = 1,
	[TCF_SYN | TCF_ACK] = 1,
	[TCF_RST] = 1,
	[TCF_RST | TCF_PSH | TCF_ACK] = 1,
	[TCF_RST | TCF_ACK] = 1,
	[TCF_PSH | TCF_ACK] = 1,
	[TCF_PSH | TCF_ACK | TCF_URG] = 1,
	[TCF_ACK] = 1,
	[TCF_ACK | TCF_URG] = 1,
};

/*
 * Magic numbers
 */
#define ECM_TRACKER_TCP_INSTANCE_MAGIC 0x5129
#define ECM_TRACKER_TCP_SKB_CB_MAGIC 0x4398
#define ECM_TRACKER_TCP_READER_INSTANCE_MAGIC 0x1123

#ifdef ECM_TRACKER_DPI_SUPPORT_ENABLE
/*
 * struct ecm_tracker_tcp_skb_cb_format
 *	Map of the cb[] array within our cached buffers - we use that area for our tracking
 */
struct ecm_tracker_tcp_skb_cb_format {
	struct sk_buff *next;		/* Next sk buff in the recvd_order list */
	struct sk_buff *prev;		/* Previous sk buff in the recvd_order  */
	uint32_t seq_no;		/* Sequence number of the first readable DATA BYTE in this buffer.  There may be others but they may be old. */
	uint32_t num_seqs;		/* Number of DATA BYTES/sequences, i.e. the number of sequences from seq_no. */
	uint32_t offset;		/* Offset from skb->data to seq_no (i.e. header size we must skip to get to data) */
#if (DEBUG_LEVEL > 0)
	uint16_t magic;
#endif
};

/*
 * struct ecm_tracker_tcp_host_data
 *	data tracking for a host
 */
struct ecm_tracker_tcp_host_data {
	uint16_t mss;					/* MSS as sent by that host in a SYN packet */
	bool mss_seen;					/* true if the mss was from a SYN packet rather than an assigned default */

	uint32_t seq_no;				/* Sequence number of the first data byte we are tracking */
	uint32_t num_seqs;				/* Number of sequences from first */
	bool seq_no_valid;				/* True when our sequencing is valid */

	/*
	 * All skbuffs are added into the recvd order list in addition to POSSIBLY being added into the in_order or future lists as well.
	 * NOTE: Buffers in recv_order list make use of the [cb]->next and [cb]->prev.
	 */
	struct sk_buff *recvd_order;			/* skbuffs inserted in order of reception */
	struct sk_buff *recvd_order_last;		/* skbuffs inserted in order of reception - tracks last insertion point for speed of appending */
	int32_t recvd_bytes_total;			/* Current total of bytes of all buffers received */
	int32_t recvd_count;				/* Current total buffers received */

	/*
	 * Buffers that have useful in-sequence-order data are added into this list
	 * NOTE: These lists make use of sk_buff->next and sk_buff->prev because:
	 * 1. An skb cannot be in in_order and future at the same time.
	 * 2. It's faster to work with these pointers - which is important when sliding over lists for reading etc..
	 */
	struct sk_buff *in_order;			/* A list of skb's tracked in our recvd_order list in sequence space order that we can read from.
							 * NOTE: An skb is added into both the recvd_order AND, if applicable, this list.
							 */
	struct sk_buff *in_order_last;			/* Fast appending of in-order data buffers */
	int32_t in_order_count;				/* The number of skb's in the in order list */

	struct sk_buff *future;				/* A list of skb's that are in the "future" (sequence space wise) and need more sequence space
							 * received to join the 'gap' allowing their use.
							 */

};
#endif

/*
 * struct ecm_tracker_tcp_sender_state
 *	Connection state of each sender
 */
struct ecm_tracker_tcp_sender_state {
	ecm_tracker_sender_state_t state;		/* State of the sender */
	uint32_t syn_seq;				/* Sequence number of the syn (ISN): used to detect when peer has ack'd the syn, transitioned this sender to established. */
	uint32_t fin_seq;				/* Sequence number of the fin: used to detect when peer has ack'd the fin, transitioned this sender to closed. */
};

/*
 * struct ecm_tracker_tcp_internal_instance
 */
struct ecm_tracker_tcp_internal_instance  {
	struct ecm_tracker_tcp_instance tcp_base;					/* MUST BE FIRST FIELD */
#ifdef ECM_TRACKER_DPI_SUPPORT_ENABLE
	struct ecm_tracker_tcp_host_data sender_data[ECM_TRACKER_SENDER_MAX];		/* Data tracked sent by the src of the connection */
#endif
	struct ecm_tracker_tcp_sender_state sender_states[ECM_TRACKER_SENDER_MAX];	/* Sender states */
#ifdef ECM_TRACKER_DPI_SUPPORT_ENABLE
	int32_t data_limit;								/* Limit for tracked data */
#endif
	int refs;									/* Integer to trap we never go negative */
	spinlock_t lock;
#if (DEBUG_LEVEL > 0)
	uint16_t magic;
#endif
};

#ifdef ECM_TRACKER_DPI_SUPPORT_ENABLE
/*
 * struct ecm_tracker_tcp_reader_instance
 *	Reader.
 */
struct ecm_tracker_tcp_reader_instance {
	struct ecm_tracker_tcp_internal_instance *ttii;	/* Reference to the internal tracker instance */
	struct ecm_tracker_tcp_host_data *data;		/* Host data this reader is using */

	/*
	 * Tracking current position
	 */
	uint32_t offset;				/* Logical offset from a start of 0 - i.e. from start of tracker data.
							 * NOTE: When bytes are discarded the offset is NOT adjusted to reflect discarded bytes, i.e. they are not shifted left.
							 * It remains as it is so that external notions of positions remain correct - especially when setting positions.
							 */

	uint32_t discarded;				/* Tracks the amount of data that has been discarded so far so that we don't try to discard them again.
							 * This is important because offset is never modified when bytes are discarded.
							 */
	/*
	 * The real work is done by the segment tracking variables that follow.
	 * These 'slide' over the available segments and always represent actual data and positions within a segment.
	 */
	uint32_t segment_offset;			/* Offset from the start of the segment at which the next read will occur (header offset + data already advanced over in this segment) */
	uint32_t segment_remain;			/* Data bytes remaining to be read in this segment given the read pointer */
	struct sk_buff *segment;			/* Current segment skb */
	struct ecm_tracker_tcp_skb_cb_format *segment_dcb;
							/* The private cb area of the current segment, cached for convenience */

	int refs;					/* Integer to trap we never go negative */
	spinlock_t lock;
#if (DEBUG_LEVEL > 0)
	uint16_t magic;
#endif
};
#endif

/*
 * ecm_tracker_tcp_timer_group_from_state[]
 *	Given a prevailing connection state return the appropriate timer group.
 *
 * Different states have different timeouts, for example, established state is the longest timeout.
 * Using these timeouts ensures efficient resource uses and avoids connections hanging around when it is unnecessary.
 */
static ecm_db_timer_group_t ecm_tracker_tcp_timer_group_from_state[] = {
							ECM_DB_TIMER_GROUPS_CONNECTION_TCP_SHORT_TIMEOUT,	/* ECM_TRACKER_CONNECTION_STATE_ESTABLISHING */
							ECM_DB_TIMER_GROUPS_CONNECTION_TCP_LONG_TIMEOUT,	/* ECM_TRACKER_CONNECTION_STATE_ESTABLISHED */
							ECM_DB_TIMER_GROUPS_CONNECTION_TCP_SHORT_TIMEOUT,	/* ECM_TRACKER_CONNECTION_STATE_CLOSING */
							ECM_DB_TIMER_GROUPS_CONNECTION_TCP_CLOSED_TIMEOUT,	/* ECM_TRACKER_CONNECTION_STATE_CLOSED */
							ECM_DB_TIMER_GROUPS_CONNECTION_TCP_RESET_TIMEOUT,	/* ECM_TRACKER_CONNECTION_STATE_FAULT */
							};
int ecm_tracker_tcp_count = 0;		/* Counts the number of TCP data trackers right now */
#ifdef ECM_TRACKER_DPI_SUPPORT_ENABLE
int ecm_tracker_tcp_reader_count = 0;	/* Counts the number of TCP readers right now */
#endif
static  DEFINE_SPINLOCK(ecm_tracker_tcp_lock);	/* Global lock for the tracker globals */

/*
 * ecm_tracker_tcp_connection_state_matrix[][]
 *	Matrix to convert from/to states to connection state
 */
static ecm_tracker_connection_state_t ecm_tracker_tcp_connection_state_matrix[ECM_TRACKER_SENDER_STATE_MAX][ECM_TRACKER_SENDER_STATE_MAX] =
{	/* 			Unknown						Establishing					Established					Closing					Closed					Fault */
	/* Unknown */		{ECM_TRACKER_CONNECTION_STATE_ESTABLISHING,	ECM_TRACKER_CONNECTION_STATE_ESTABLISHING,	ECM_TRACKER_CONNECTION_STATE_ESTABLISHED,	ECM_TRACKER_CONNECTION_STATE_FAULT,	ECM_TRACKER_CONNECTION_STATE_FAULT,	ECM_TRACKER_CONNECTION_STATE_FAULT},
	/* Establishing */	{ECM_TRACKER_CONNECTION_STATE_ESTABLISHING,	ECM_TRACKER_CONNECTION_STATE_ESTABLISHING,	ECM_TRACKER_CONNECTION_STATE_ESTABLISHING,	ECM_TRACKER_CONNECTION_STATE_FAULT,	ECM_TRACKER_CONNECTION_STATE_FAULT,	ECM_TRACKER_CONNECTION_STATE_FAULT},
	/* Established */	{ECM_TRACKER_CONNECTION_STATE_ESTABLISHED,	ECM_TRACKER_CONNECTION_STATE_ESTABLISHING,	ECM_TRACKER_CONNECTION_STATE_ESTABLISHED,	ECM_TRACKER_CONNECTION_STATE_CLOSING,	ECM_TRACKER_CONNECTION_STATE_CLOSING,	ECM_TRACKER_CONNECTION_STATE_FAULT},
	/* Closing */		{ECM_TRACKER_CONNECTION_STATE_FAULT,		ECM_TRACKER_CONNECTION_STATE_FAULT,		ECM_TRACKER_CONNECTION_STATE_CLOSING,		ECM_TRACKER_CONNECTION_STATE_CLOSING,	ECM_TRACKER_CONNECTION_STATE_CLOSING,	ECM_TRACKER_CONNECTION_STATE_FAULT},
	/* Closed */		{ECM_TRACKER_CONNECTION_STATE_FAULT,		ECM_TRACKER_CONNECTION_STATE_FAULT,		ECM_TRACKER_CONNECTION_STATE_CLOSING,		ECM_TRACKER_CONNECTION_STATE_CLOSING,	ECM_TRACKER_CONNECTION_STATE_CLOSED, 	ECM_TRACKER_CONNECTION_STATE_FAULT},
	/* Fault */		{ECM_TRACKER_CONNECTION_STATE_FAULT,		ECM_TRACKER_CONNECTION_STATE_FAULT,		ECM_TRACKER_CONNECTION_STATE_FAULT,		ECM_TRACKER_CONNECTION_STATE_FAULT,	ECM_TRACKER_CONNECTION_STATE_FAULT,	ECM_TRACKER_CONNECTION_STATE_FAULT},
};

/*
 * ecm_tracker_tcp_check_header_and_read()
 *	Check that we have a complete transport-level header, check it and return it if it's okay.
 */
struct tcphdr *ecm_tracker_tcp_check_header_and_read(struct sk_buff *skb, struct ecm_tracker_ip_header *ip_hdr, struct tcphdr *port_buffer)
{
	struct tcphdr *hdr;
	union tcp_word_hdr *word_hdr;
	uint16_t tcp_hdr_len;
	uint32_t tcp_flags;
	struct ecm_tracker_ip_protocol_header *header;

	/*
	 * Is there a TCP header?
	 */
	header = &ip_hdr->headers[ECM_TRACKER_IP_PROTOCOL_TYPE_TCP];
	if (header->size == 0) {
		DEBUG_WARN("No TCP header %px\n", skb);
		return NULL;
	}
	hdr = (struct tcphdr *)skb_header_pointer(skb, header->offset, sizeof(*port_buffer), port_buffer);
	if (unlikely(!hdr)) {
		DEBUG_WARN("Cant read TCP header %px\n", skb);
		return NULL;
	}
	word_hdr = (union tcp_word_hdr *)hdr;

	/*
	 * Check that we have all of the TCP data we're supposed to have
	 */
	tcp_hdr_len = (uint16_t)hdr->doff;
	tcp_hdr_len <<= 2;
	if (unlikely(tcp_hdr_len < 20)) {
		DEBUG_WARN("Packet %px TCP header to short %u\n", skb, tcp_hdr_len);
		return NULL;
	}

	if (unlikely(ip_hdr->total_length < (header->offset + header->size))) {
		DEBUG_WARN("TCP packet %px too short (ip_total_len = %u, size needed=%u)\n", skb, ip_hdr->total_length, header->offset + header->size);
		return NULL;
	}

	/*
	 * Validate flags
	 */
	tcp_flags = ECM_TRACKER_TCP_WORD_HDR_TO_FLAGS(word_hdr);
	DEBUG_TRACE("%px: TCP flags = 0x%04x\n", skb, (unsigned)tcp_flags);
	if (unlikely(!ecm_tracker_tcp_valid_flags[tcp_flags & 0x3f])) {
		DEBUG_WARN("%px: Invalid flags 0x%x", skb, (unsigned)tcp_flags);
		return NULL;
	}

	return hdr;
}
EXPORT_SYMBOL(ecm_tracker_tcp_check_header_and_read);

#ifdef ECM_TRACKER_DPI_SUPPORT_ENABLE
/*
 * ecm_tracker_tcp_bytes_discard()
 *	Discard n bytes from the available in_order bytes
 */
static void ecm_tracker_tcp_bytes_discard(struct ecm_tracker_tcp_internal_instance *ttii, struct ecm_tracker_tcp_host_data *data, uint32_t n)
{
	DEBUG_CHECK_MAGIC(ttii, ECM_TRACKER_TCP_INSTANCE_MAGIC, "%px: magic failed", ttii);

	/*
	 * Advance sequence space by 'n'.  This may allow throwing of datagrams.
	 */
	spin_lock_bh(&ttii->lock);
	DEBUG_ASSERT(n <= data->num_seqs, "%px: discard invalid %u, available %u\n", ttii, n, data->num_seqs);
	data->num_seqs -= n;
	data->seq_no += n;
	DEBUG_TRACE("%px: Discard %u, New seq space for %px, seq_no: %u, num_seqs: %u\n", ttii, n, data, data->seq_no, data->num_seqs);
	spin_unlock_bh(&ttii->lock);

	while (n) {
		struct ecm_tracker_tcp_skb_cb_format *skb_cb;
		struct sk_buff *skb;

		/*
		 * Examine the buffer at the head of the in order data list
		 */
		spin_lock_bh(&ttii->lock);
		skb = data->in_order;
		DEBUG_ASSERT(skb, "%px: data list invalid\n", ttii);

		skb_cb = (struct ecm_tracker_tcp_skb_cb_format *)skb->cb;
		DEBUG_CHECK_MAGIC(skb_cb, ECM_TRACKER_TCP_SKB_CB_MAGIC, "%px: invalid cb magic %px\n", ttii, skb_cb);

		if (n < skb_cb->num_seqs) {
			/*
			 * skb has more sequences in it than we are discarding.  Simply advance the sequence space by n
			 */
			skb_cb->seq_no += n;
			skb_cb->offset += n;
			skb_cb->num_seqs -= n;
			DEBUG_TRACE("%px: PARTIAL BUFFER DISCARD SEQ: %u, NUM SEQ: %u, OFFSET: %u\n", ttii, skb_cb->seq_no, skb_cb->num_seqs, skb_cb->offset);
			spin_unlock_bh(&ttii->lock);
			return;
		}

		/*
		 * Remove skb from the recvd_order list (it may be anywhere in this list)
		 */
		if (!skb_cb->next) {
			DEBUG_ASSERT(data->recvd_order_last == skb, "%px: bad list\n", ttii);
			data->recvd_order_last = skb_cb->prev;
		} else {
			struct ecm_tracker_tcp_skb_cb_format *next_cb;
			next_cb = (struct ecm_tracker_tcp_skb_cb_format *)skb_cb->next->cb;
			DEBUG_CHECK_MAGIC(next_cb, ECM_TRACKER_TCP_SKB_CB_MAGIC, "%px: invalid cb magic %px\n", ttii, next_cb);
			next_cb->prev = skb_cb->prev;
		}
		if (!skb_cb->prev) {
			DEBUG_ASSERT(data->recvd_order == skb, "%px: bad list\n", ttii);
			data->recvd_order = skb_cb->next;
		} else {
			struct ecm_tracker_tcp_skb_cb_format *prev_cb;
			prev_cb = (struct ecm_tracker_tcp_skb_cb_format *)skb_cb->prev->cb;
			DEBUG_CHECK_MAGIC(prev_cb, ECM_TRACKER_TCP_SKB_CB_MAGIC, "%px: invalid cb magic %px\n", ttii, prev_cb);
			prev_cb->next = skb_cb->next;
		}

		/*
		 * Buffer is to be discarded in its entirety
		 * Link it out of the in_order list - this datagram will be at the head of the list
		 */
		data->in_order = skb->next;
		if (data->in_order) {
			data->in_order->prev = NULL;
		} else {
			DEBUG_ASSERT(data->in_order_last == skb, "%px: bad list\n", ttii);
			data->in_order_last = NULL;
		}

		/*
		 * Update our stats
		 */
		data->in_order_count--;
		DEBUG_ASSERT(data->in_order_count >= 0, "%px: bad in order count\n", ttii);
		data->recvd_count--;
		DEBUG_ASSERT(data->recvd_count >= 0, "%px: bad count\n", ttii);
		data->recvd_bytes_total -= skb->truesize;
		DEBUG_ASSERT(data->recvd_bytes_total >= 0, "%px: bad bytes count\n", ttii);
		ecm_tracker_data_total_decrease(skb->len, skb->truesize);

		/*
		 * Decrement n by the number of segments we are discarding in this segment
		 */
		n -= skb_cb->num_seqs;
		DEBUG_TRACE("%px: BUFFER DISCARD %px, size: %u, new head: %px end: %px, discard remain: %u, in order count: %d\n", ttii, skb, skb_cb->num_seqs, data->in_order, data->in_order_last, n, data->in_order_count);
		spin_unlock_bh(&ttii->lock);

		dev_kfree_skb_any(skb);
	}
}

/*
 * ecm_tracker_tcp_bytes_discard_callback()
 *	Discard n bytes from the available stream bytes
 */
static void ecm_tracker_tcp_bytes_discard_callback(struct ecm_tracker_tcp_instance *tti, ecm_tracker_sender_type_t sender, uint32_t n)
{
	struct ecm_tracker_tcp_internal_instance *ttii = (struct ecm_tracker_tcp_internal_instance *)tti;
	struct ecm_tracker_tcp_host_data *data;

	DEBUG_CHECK_MAGIC(ttii, ECM_TRACKER_TCP_INSTANCE_MAGIC, "%px: magic failed", ttii);
	DEBUG_ASSERT((sender >= 0) && (sender <= 1), "%px: invalid sender %d\n", ttii, sender);

	/*
	 * Which list?
	 */
	data = &ttii->sender_data[sender];
	ecm_tracker_tcp_bytes_discard(ttii, data, n);
}

/*
 * ecm_tracker_tcp_datagram_discard_callback()
 *	Discard n number of datagrams from the in_order list
 *
 * NOTE: This discards from the in_order list - this means that any data in the future list cannot be destroyed by this method.
 */
static void ecm_tracker_tcp_datagram_discard_callback(struct ecm_tracker_instance *ti, ecm_tracker_sender_type_t sender, int32_t n)
{
	struct ecm_tracker_tcp_internal_instance *ttii = (struct ecm_tracker_tcp_internal_instance *)ti;
	struct ecm_tracker_tcp_host_data *data;
	struct sk_buff *skb;
	uint32_t seqs_discard;

	DEBUG_CHECK_MAGIC(ttii, ECM_TRACKER_TCP_INSTANCE_MAGIC, "%px: magic failed", ttii);
	DEBUG_ASSERT((sender >= 0) && (sender <= 1), "%px: invalid sender %d\n", ttii, sender);

	/*
	 * Which list?
	 */
	data = &ttii->sender_data[sender];

	/*
	 * Get the number of sequences in n datagrams
	 */
	seqs_discard = 0;
	spin_lock_bh(&ttii->lock);
	skb = data->in_order;
	while (n) {
		struct ecm_tracker_tcp_skb_cb_format *skb_cb;

		skb_cb = (struct ecm_tracker_tcp_skb_cb_format *)skb->cb;
		DEBUG_CHECK_MAGIC(skb_cb, ECM_TRACKER_TCP_SKB_CB_MAGIC, "%px: invalid cb magic %px\n", ttii, skb);
		seqs_discard += skb_cb->num_seqs;
		skb = skb->next;
		n--;
	}
	spin_unlock_bh(&ttii->lock);

	/*
	 * Now simply discard bytes to the tune of seqs_discard!
	 */
	ecm_tracker_tcp_bytes_discard(ttii, data, seqs_discard);
}

/*
 * _ecm_tracker_tcp_discard_all()
 *	Discard all tracked data
 *
 * NOTE: Calling this will erase all data within the tracker - including any future unsequenced data.  FOR BOTH SENDERS.
 * Sequence space tracking shall pick up again on the next tracked data packets.
 * Don't call this function often (slow) - usually you call it when your classifier has determined responsibility for a connection and
 * it no longer needs any data - and will not, typically, track any further data either.
 */
static void _ecm_tracker_tcp_discard_all(struct ecm_tracker_tcp_internal_instance *ttii)
{
	int s;

	DEBUG_TRACE("%px: destroy all\n", ttii);
	for (s = 0; s < ECM_TRACKER_SENDER_MAX; ++s) {
		struct ecm_tracker_tcp_host_data *data;

		/*
		 * Get sender data
		 */
		data = &ttii->sender_data[s];

		/*
		 * Discard all in the recvd_order list
		 */
		while (data->recvd_count) {
			struct ecm_tracker_tcp_skb_cb_format *skb_cb;
			struct sk_buff *skb;

			/*
			 * Get private region of skb that maintains the recvd_order list pointers
			 */
			skb = data->recvd_order;
			skb_cb = (struct ecm_tracker_tcp_skb_cb_format *)skb->cb;
			DEBUG_CHECK_MAGIC(skb_cb, ECM_TRACKER_TCP_SKB_CB_MAGIC, "%px: invalid cb magic %px\n", ttii, skb);

			/*
			 * Remove the skb from the recvd_order list
			 * NOTE: We don't have to maintain recvd_order_last or any list prev pointers its because we are discarding the lot!
			 */
			data->recvd_order = skb_cb->next;
			data->recvd_count--;

			/*
			 * Destroy
			 */
			ecm_tracker_data_total_decrease(skb->len, skb->truesize);
			dev_kfree_skb_any(skb);
		}
		DEBUG_ASSERT(data->recvd_order == NULL, "%px: recvd_count not match recvd_order length: %px\n", ttii, data->recvd_order);
		data->recvd_order_last = NULL;
		data->recvd_bytes_total = 0;
		data->in_order = NULL;
		data->in_order_last = NULL;
		data->in_order_count = 0;
		data->future = NULL;

		/*
		 * Invalidate the sequence space
		 */
		data->seq_no_valid = false;
	}
}

/*
 * ecm_tracker_tcp_discard_all_callback()
 *	Discard all tracked data
 *
 * NOTE: Calling this will erase all data within the tracker - including any future unsequenced data.  FOR BOTH SENDERS.
 * Sequence space tracking shall pick up again on the next tracked data packets.
 * Don't call this function often (slow) - usually you call it when your classifier has determined responsibility for a connection and
 * it no longer needs any data - and will not, typically, track any further data either.
 */
static void ecm_tracker_tcp_discard_all_callback(struct ecm_tracker_instance *ti)
{
	struct ecm_tracker_tcp_internal_instance *ttii = (struct ecm_tracker_tcp_internal_instance *)ti;

	DEBUG_CHECK_MAGIC(ttii, ECM_TRACKER_TCP_INSTANCE_MAGIC, "%px: magic failed", ttii);
	spin_lock_bh(&ttii->lock);
	_ecm_tracker_tcp_discard_all(ttii);
	spin_unlock_bh(&ttii->lock);
}
#endif

/*
 * ecm_tracker_tcp_ref_callback()
 */
void ecm_tracker_tcp_ref_callback(struct ecm_tracker_instance *ti)
{
	struct ecm_tracker_tcp_internal_instance *ttii = (struct ecm_tracker_tcp_internal_instance *)ti;
	DEBUG_CHECK_MAGIC(ttii, ECM_TRACKER_TCP_INSTANCE_MAGIC, "%px: magic failed", ttii);

	spin_lock_bh(&ttii->lock);

	ttii->refs++;
	DEBUG_ASSERT(ttii->refs > 0, "%px: ref wrap", ttii);
	DEBUG_TRACE("%px: ref %d\n", ttii, ttii->refs);

	spin_unlock_bh(&ttii->lock);
}

/*
 * ecm_tracker_tcp_deref_callback()
 */
int ecm_tracker_tcp_deref_callback(struct ecm_tracker_instance *ti)
{
	struct ecm_tracker_tcp_internal_instance *ttii = (struct ecm_tracker_tcp_internal_instance *)ti;
	int refs;
	DEBUG_CHECK_MAGIC(ttii, ECM_TRACKER_TCP_INSTANCE_MAGIC, "%px: magic failed", ttii);

	spin_lock_bh(&ttii->lock);
	ttii->refs--;
	refs = ttii->refs;
	DEBUG_ASSERT(ttii->refs >= 0, "%px: ref wrap", ttii);
	DEBUG_TRACE("%px: deref %d\n", ttii, ttii->refs);

	if (ttii->refs > 0) {
		spin_unlock_bh(&ttii->lock);
		return refs;
	}

	DEBUG_TRACE("%px: final\n", ttii);
#ifdef ECM_TRACKER_DPI_SUPPORT_ENABLE
	_ecm_tracker_tcp_discard_all(ttii);
#endif
	spin_unlock_bh(&ttii->lock);

	spin_lock_bh(&ecm_tracker_tcp_lock);
	ecm_tracker_tcp_count--;
	DEBUG_ASSERT(ecm_tracker_tcp_count >= 0, "%px: tracker count wrap", ttii);
	spin_unlock_bh(&ecm_tracker_tcp_lock);

	DEBUG_INFO("%px: TCP tracker final\n", ttii);
	DEBUG_CLEAR_MAGIC(ttii);
	kfree(ttii);

	return 0;
}

#ifdef ECM_TRACKER_DPI_SUPPORT_ENABLE
/*
 * ecm_tracker_tcp_datagram_count_get_callback()
 *	Return number of available datagrams sent to the specified target in the in_order list
 */
static int32_t ecm_tracker_tcp_datagram_count_get_callback(struct ecm_tracker_instance *ti, ecm_tracker_sender_type_t sender)
{
	struct ecm_tracker_tcp_internal_instance *ttii = (struct ecm_tracker_tcp_internal_instance *)ti;
	int32_t count;

	DEBUG_CHECK_MAGIC(ttii, ECM_TRACKER_TCP_INSTANCE_MAGIC, "%px: magic failed", ttii);
	DEBUG_ASSERT((sender >= 0) && (sender <= 1), "%px: invalid sender %d\n", ttii, sender);

	/*
	 * Which list?
	 */
	spin_lock_bh(&ttii->lock);
	count = ttii->sender_data[sender].in_order_count;
	spin_unlock_bh(&ttii->lock);
	DEBUG_TRACE("%px: datagram count get for %d is %d\n", ttii, sender, count);

	return count;
}

/*
 * ecm_tracker_tcp_datagram_size_get_callback()
 *	Return size in bytes of in_order datagram at index i that was sent by the sender
 */
static int32_t ecm_tracker_tcp_datagram_size_get_callback(struct ecm_tracker_instance *ti, ecm_tracker_sender_type_t sender, int32_t i)
{
	struct ecm_tracker_tcp_internal_instance *ttii = (struct ecm_tracker_tcp_internal_instance *)ti;
	int32_t size;
	struct sk_buff *skb;
	DEBUG_CHECK_MAGIC(ttii, ECM_TRACKER_TCP_INSTANCE_MAGIC, "%px: magic failed", ttii);
	DEBUG_ASSERT((sender >= 0) && (sender <= 1), "%px: invalid sender %d\n", ttii, sender);

	DEBUG_TRACE("%px: get datagram size at %u for %d\n", ttii, i, sender);

	/*
	 * Which list?
	 */
	spin_lock_bh(&ttii->lock);
	skb = ttii->sender_data[sender].in_order;

	/*
	 * Iterate to the i'th datagram
	 */
	while (i) {
		DEBUG_ASSERT(skb, "%px: index bad\n", ttii);
		skb = skb->next;
		i--;
	}
	DEBUG_ASSERT(skb, "%px: index bad\n", ttii);

	/*
	 * Get size
	 */
	size = skb->len;
	spin_unlock_bh(&ttii->lock);

	DEBUG_TRACE("%px: datagram size is %d\n", ttii, size);
	return size;
}

/*
 * ecm_tracker_tcp_datagram_read_callback()
 *	Read size bytes from the i'th in_order datagram
 */
static int ecm_tracker_tcp_datagram_read_callback(struct ecm_tracker_instance *ti, ecm_tracker_sender_type_t sender, int32_t i, int32_t offset, int32_t size, void *buffer)
{
	struct ecm_tracker_tcp_internal_instance *ttii = (struct ecm_tracker_tcp_internal_instance *)ti;
	int res;
	struct sk_buff *skb;

	DEBUG_CHECK_MAGIC(ttii, ECM_TRACKER_TCP_INSTANCE_MAGIC, "%px: magic failed", ttii);
	DEBUG_TRACE("%px: datagram %d read at offset %d for %d bytes for %d\n", ttii, i, offset, size, sender);
	DEBUG_ASSERT((sender >= 0) && (sender <= 1), "%px: invalid sender %d\n", ttii, sender);

	/*
	 * Which list?
	 */
	spin_lock_bh(&ttii->lock);
	skb = ttii->sender_data[sender].in_order;

	/*
	 * Iterate to the i'th datagram
	 */
	while (i) {
		DEBUG_ASSERT(skb, "%px: index bad\n", ttii);
		skb = skb->next;
		i--;
	}
	DEBUG_ASSERT(skb, "%px: index bad\n", ttii);

	/*
	 * Perform read
	 */
	res = skb_copy_bits(skb, offset, buffer, (unsigned int)size);
	spin_unlock_bh(&ttii->lock);

	return res;
}

/*
 * _ecm_tracker_tcp_data_future_replay()
 *	Replay packets in the future data list to see if any are now in sequence.
 *
 * NOTE: 'Future' is a misnomer here because the arrival of data may have actually cause some future data
 * to now be old, example:
 * SPACE:    --------------CCCCCCCC-------
 * FUTURE:   ------------------------FFF--
 * NEW:      ----------------------NNNNNN
 * As you can see the future data is now rendered old by the arrival of sequences
 *
 * NOTE: The future data list may also contain duplicate future data - we weren't that fussy when we inserted the segment into the future list.
 */
static void _ecm_tracker_tcp_data_future_replay(struct ecm_tracker_tcp_internal_instance *ttii,
									struct ecm_tracker_tcp_host_data *data)
{
	while (data->future) {
		struct ecm_tracker_tcp_skb_cb_format *skb_cb;
		struct sk_buff *skb;
		int32_t in_sequence;
		int32_t new_seqs;
		int32_t offset;

		/*
		 * Get our private area of future buffer
		 */
		skb = data->future;
		skb_cb = (struct ecm_tracker_tcp_skb_cb_format *)skb->cb;
		DEBUG_CHECK_MAGIC(skb_cb, ECM_TRACKER_TCP_SKB_CB_MAGIC, "%px: invalid cb magic %px\n", ttii, skb_cb);

		/*
		 * Identify the sequence space
		 */
		in_sequence = (int32_t)(skb_cb->seq_no - (data->seq_no + data->num_seqs));
		new_seqs = (int32_t)(skb_cb->num_seqs + in_sequence);
		offset = -in_sequence;
		DEBUG_TRACE("%px: FUTURE REPLAY %px: in_sequence: %d, new_seqs: %d, offset: %d\n", ttii, skb, in_sequence, new_seqs, offset);

		/*
		 * Is the segment still in the future?
		 */
		if (in_sequence > 0) {
			/*
			 * Still awaiting sequences until this segment becomes valid.
			 * We are done
		 	 */
			DEBUG_TRACE("%px: FUTURE REPLAY FUTURE SEQ: %u LEN:%u\n", ttii, skb_cb->seq_no, skb_cb->num_seqs);
			break;
		}

		/*
		 * Whatever happens now we are definately linking this skb out of the futures list
		 */
		data->future = skb->next;
		if (data->future) {
			data->future->prev = NULL;
		}

		/*
		 * Does the segment contain only old data?
		 */
		if (new_seqs <= 0) {
			/*
			 * all old - skb needs to be discarded
			 */
			DEBUG_TRACE("%px: FUTURE REPLAY OLD SEQ: %u LEN:%u, new future head: %px\n", ttii, skb_cb->seq_no, skb_cb->num_seqs, data->future);

			/*
			 * Remove skb from the recvd_order list (it may be anywhere in this list)
			 */
			if (!skb_cb->next) {
				DEBUG_ASSERT(data->recvd_order_last == skb, "%px: bad list\n", ttii);
				data->recvd_order_last = skb_cb->prev;
			} else {
				struct ecm_tracker_tcp_skb_cb_format *next_cb;
				next_cb = (struct ecm_tracker_tcp_skb_cb_format *)skb_cb->next->cb;
				DEBUG_CHECK_MAGIC(next_cb, ECM_TRACKER_TCP_SKB_CB_MAGIC, "%px: invalid cb magic %px\n", ttii, next_cb);
				next_cb->prev = skb_cb->prev;
			}
			if (!skb_cb->prev) {
				DEBUG_ASSERT(data->recvd_order == skb, "%px: bad list\n", ttii);
				data->recvd_order = skb_cb->next;
			} else {
				struct ecm_tracker_tcp_skb_cb_format *prev_cb;
				prev_cb = (struct ecm_tracker_tcp_skb_cb_format *)skb_cb->prev->cb;
				DEBUG_CHECK_MAGIC(prev_cb, ECM_TRACKER_TCP_SKB_CB_MAGIC, "%px: invalid cb magic %px\n", ttii, prev_cb);
				prev_cb->next = skb_cb->next;
			}

			/*
			 * Update stats
			 */
			data->recvd_count--;
			DEBUG_ASSERT(data->recvd_count >= 0, "%px: bad count\n", ttii);
			data->recvd_bytes_total -= skb->truesize;
			ecm_tracker_data_total_decrease(skb->len, skb->truesize);
			DEBUG_ASSERT(data->recvd_bytes_total >= 0, "%px: bad bytes count\n", ttii);

			/*
			 * Release skb containing obsoleted data
			 */
			dev_kfree_skb_any(skb);
			continue;
		}

		/*
		 * We are dealing with new or some new/old data.
		 */
		DEBUG_ASSERT(offset >= 0, "%px: alg bad, offset negative at %d\n", ttii, offset);

		/*
		 * Initialise the private cb space
		 * The sequence space recorded here is READABLE space i.e. any old/dup is removed
		 */
		skb_cb->seq_no += offset;				/* Advance sequence space to new data sequences */
		skb_cb->num_seqs = (uint32_t)new_seqs;
		skb_cb->offset += offset;				/* Add new offset to existing offset */

		/*
		 * Insert the skb onto the end of the data list - it must be the end because the data here is considered in order
		 */
		skb->next = NULL;
		skb->prev = data->in_order_last;
		data->in_order_last = skb;
		if (skb->prev) {
			skb->prev->next = skb;
		} else {
			/*
			 * If there is no data_end then skb is at the head of the list
			 */
			DEBUG_ASSERT(data->in_order == NULL, "%px: bad data_end, data points to %px\n", ttii, data->in_order);
			data->in_order = skb;
		}
		data->in_order_count++;
		DEBUG_ASSERT(data->in_order_count > 0, "%px: invalid in_order count %d for data %px\n", ttii, data->in_order_count, data);

		/*
		 * Update our contiguous space pointer
		 */
		data->num_seqs += (uint32_t)new_seqs;

		DEBUG_TRACE("%px: FUTURE REPLAY IN SEQ:%u LEN:%u.  Inserted between %px and %px.  SEQ SPACE: %u, len %u, in_order_count %d\n", ttii, skb_cb->seq_no, skb_cb->num_seqs, skb_cb->prev,
				skb_cb->next, data->seq_no, data->num_seqs, data->in_order_count);
	}

	DEBUG_TRACE("%px: FUTURE REPLAY COMPLETE\n", ttii);
}

/*
 * _ecm_tracker_tcp_stream_segment_add()
 *	Add skb buff into our stream tracking lists if appropriate.
 *
 * Must pass a validly formed tcp segment to this function.
 * Returns true when the segment is to be recorded in the recvd_order list.
 */
static bool _ecm_tracker_tcp_stream_segment_add(struct ecm_tracker_tcp_internal_instance *ttii,
									struct ecm_tracker_tcp_host_data *data,
									struct ecm_tracker_ip_header *ip_hdr, struct ecm_tracker_ip_protocol_header *ecm_tcp_header,
									struct tcphdr *tcp_hdr, struct sk_buff *skb,
									struct ecm_tracker_tcp_skb_cb_format *skb_cb)
{
	uint16_t data_offset;
	uint16_t data_len;
	uint32_t seq;
	uint32_t num_seqs;
	int32_t in_sequence;
	int32_t new_seqs;
	int32_t offset;

	/*
	 * We should have been passed a valid cb area
	 */
	DEBUG_CHECK_MAGIC(skb_cb, ECM_TRACKER_TCP_SKB_CB_MAGIC, "%px: skb bad cb magic\n", ttii);

	/*
	 * Reset? ignore
	 */
	if (unlikely(tcp_hdr->rst)) {
		DEBUG_TRACE("%px: reset seen - ignoring\n", ttii);
		return false;
	}

	/*
	 * Figure out the data length in this TCP segment.
	 */
	DEBUG_ASSERT((ecm_tcp_header->offset + ecm_tcp_header->size) <= ip_hdr->total_length, "%px: invalid hdrs\n", ttii);
	data_offset = ecm_tcp_header->offset + ecm_tcp_header->header_size;
	data_len = ecm_tcp_header->size - ecm_tcp_header->header_size;
	DEBUG_TRACE("%px: data offset: %u, data_len = %u\n", ttii, data_offset, data_len);

	/*
	 * Set sequence number of first byte of data in the header.
	 * NOTE: If the syn flag is set then this is sequenced like a data byte
	 */
	seq = ntohl(tcp_hdr->seq);

	/*
	 * If SYN or FIN are set then we have one sequence
	 */
	if (unlikely(tcp_hdr->syn)) {
		/*
		 * First actual byte of data has the next sequence number
		 */
		seq++;
	}

	/*
	 * If our sequence space is not yet validated then we take the sequence number of this packet
	 * to validate our sequence numbering
	 */
	if (unlikely(!data->seq_no_valid)) {
		DEBUG_ASSERT(data->in_order == NULL, "%px: in_order not null %px\n", ttii, data->in_order);
		DEBUG_ASSERT(data->in_order_last == NULL, "%px: in_order_last not null %px\n", ttii, data->in_order_last);
		DEBUG_ASSERT(data->in_order_count == 0, "%px: in_order_count not 0: %d\n", ttii, data->in_order_count);

		/*
		 * Sequence numbers are not valid at the moment so this segment is where we start from
		 */
		data->seq_no = seq;
		data->num_seqs = 0;
		data->seq_no_valid = true;

		DEBUG_TRACE("%px: FIRST SEQ:%u\n", ttii, seq);
	}

	/*
	 * Do we have any data?
	 */
	if (data_len == 0) {
		return false;
	}

	/*
	 * Examine the data sequence space:
	 * It contains in sequence new data - USE NEW.
	 * It is old data - IGNORE
	 * It is duplicate - IGNORE
	 * It contains some old, duplicate, new - IGNORE OLD and DUPLICATE.  USE NEW.
	 * It contains some future - ADD TO FUTURES LIST
	 * Sequence space possibilities:
	 * SEQ SPACE:     0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz
	 * CURRENT SPACE: -----------------CCCCCCCCCCCCCCCCCC---------------------------
	 * NEW:           -----------------------------------NNN------------------------
	 * ALL OLD:       ----------OOOOOOO---------------------------------------------
	 * DUPLICATE:     -----------------DDDDDDD--------------------------------------
	 * OLD/DUP/NEW:   --------------OOODDDDDDDDDDDDDDDDDDNNNNNNNNNNN----------------
	 * FUTURE:        ------------------------------------F-------------------------
	 */

	/*
	 * Number of sequences is now the data length
	 */
	num_seqs = data_len;

	/*
	 * Is the segment in sequence?
	 * Sequence space scenarios:
	 * data->seq	data->num_seqs		seq	num_seqs	in_sequence	new_seqs	Offset	Result?
	 * 100		20			120	30		0		30		0	In sequence 30 bytes, new data is from the first data byte in skb
	 * 100		20			121	30		1		31		-1	Future, new_seqs is irrelevant here, offset irrelevant
	 * 100		20			119	30		-1		29		1	1 DUP, 29 new, offset to new data is 1
	 * 100		20			100	30		-20		10		20	20 DUP, 10 new, offset to new data is 20
	 * 100		20			99	30		-21		9		21	21 DUP, 9 new, offset to new data is 21
	 * 100		20			90	30		-30		0		30	Nothing new, offset irrelevant
	 * 100		20			70	30		-50		-20		50	Nothing new, offset irrelevant
	 */
	in_sequence = (int32_t)(seq - (data->seq_no + data->num_seqs));
	new_seqs = (int32_t)(num_seqs + in_sequence);
	offset = -in_sequence;
	DEBUG_TRACE("%px: in_sequence: %d, new_seqs: %d, offset: %d\n", ttii, in_sequence, new_seqs, offset);

	/*
	 * Does the segment contain only old data?
	 */
	if (new_seqs <= 0) {
		/*
		 * all old
		 */
		DEBUG_TRACE("%px: OLD SEQ: %u LEN:%u\n", ttii, seq, num_seqs);
		return false;
	}

	/*
	 * Is the segment in the future?
	 */
	if (unlikely(in_sequence > 0)) {
		/*
		 * Packet contains sequences in the future
		 */
		struct sk_buff *nskb;
		struct sk_buff *pskb;

		/*
		 * Initialise the private cb space
		 * Note that sequence space here may duplicate other future data but that will be corrected when future data is sorted out.
		 */
		skb_cb->seq_no = seq;
		skb_cb->num_seqs = num_seqs;
		skb_cb->offset = data_offset;
		DEBUG_INFO("%px: FUTURE SEQ: %u LEN:%u\n", ttii, seq, num_seqs);

		/*
		 * Insert the segment into the future list in order of sequence number
		 */
		nskb = data->future;
		pskb = NULL;
		while (nskb) {
			struct ecm_tracker_tcp_skb_cb_format *nskb_cb;
			nskb_cb = (struct ecm_tracker_tcp_skb_cb_format *)nskb->cb;
			DEBUG_CHECK_MAGIC(nskb_cb, ECM_TRACKER_TCP_SKB_CB_MAGIC, "%px: invalid cb magic %px\n", ttii, nskb_cb);
			DEBUG_TRACE("%px: seq: %u, nskb_cb->seq: %u\n", ttii, seq, nskb_cb->seq_no);
			if (seq <= nskb_cb->seq_no) {
				break;
			}
			pskb = nskb;
			nskb = nskb->next;
		}
		DEBUG_TRACE("%px: Insert after %px and before %px\n", ttii, pskb, nskb);

		/*
		 * Insert skb between pskb and nskb in the futures list
		 */
		skb->next = nskb;
		skb->prev = pskb;
		if (nskb) {
			nskb->prev = skb;
		}
		if (pskb) {
			pskb->next = skb;
		} else {
			/*
			 * Head of the list
			 */
			DEBUG_ASSERT(data->future == nskb, "%px: invalid future list %px != %px\n", ttii, data->future, nskb);
			data->future = skb;
		}

		DEBUG_TRACE("%px: FUTURE SEQ:%u LEN:%u.  Inserted after %px and before %px\n", ttii, seq, num_seqs, pskb, nskb);
		return true;
	}

	/*
	 * We are dealing with new or some new/old data.
	 */
	DEBUG_ASSERT(offset >= 0, "%px: alg bad, offset negative at %d\n", ttii, offset);

	/*
	 * Initialise the private cb space
	 * The sequence space recorded here is READABLE space i.e. any old/dup is removed
	 */
	skb_cb->seq_no = seq + offset;
	skb_cb->num_seqs = (uint32_t)new_seqs;
	skb_cb->offset = data_offset + offset;

	/*
	 * Insert the skb onto the end of the data list - it must be the end because the data here is considered in order
	 */
	skb->next = NULL;
	skb->prev = data->in_order_last;
	data->in_order_last = skb;
	if (skb->prev) {
		skb->prev->next = skb;
	} else {
		/*
		 * If there is no data_end then skb is at the head of the list
		 */
		DEBUG_ASSERT(data->in_order == NULL, "%px: bad data_end, data points to %px\n", ttii, data->in_order);
		data->in_order = skb;
	}
	data->in_order_count++;
	DEBUG_ASSERT(data->in_order_count > 0, "%px: invalid in_order count %d for data %px\n", ttii, data->in_order_count, data);

	/*
	 * Update our contiguous space pointers
	 */
	data->num_seqs += (uint32_t)new_seqs;

	DEBUG_TRACE("%px: IN SEQ:%u LEN:%u.  Inserted between %px and %px.  SEQ SPACE starts at: %u, len %u, in_order_count %d\n",
			ttii, skb_cb->seq_no, skb_cb->num_seqs, skb_cb->prev, skb_cb->next, data->seq_no, data->num_seqs, data->in_order_count);

	/*
	 * Do we have any future data?  It's likely we won't as it is rare for packets to arrive out of sequence in this way.
	 */
	if (likely(!data->future)) {
		return true;
	}

	/*
	 * We now need to process future data - the reception of seqences here may now release some
	 */
	_ecm_tracker_tcp_data_future_replay(ttii, data);
	return true;
}

/*
 * ecm_tracker_tcp_extract_mss()
 *	Read the MSS from the options block of the tcp header
 */
static bool ecm_tracker_tcp_extract_mss(struct sk_buff *skb, uint16_t *mss, struct ecm_tracker_ip_protocol_header *ecm_tcp_header)
{
	struct tcp_options_received opt_rx;

	/*
	 * Set he transport header offset (tcp) - unlikely not to be set BUT we don't take the chance
	 */
	skb_set_transport_header(skb, ecm_tcp_header->offset);

	/*
	 * Parse the TCP header options
	 */
	memset(&opt_rx, 0, sizeof(opt_rx));
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 13, 0))
	tcp_parse_options(skb, &opt_rx, 0, NULL);
#else
	tcp_parse_options(dev_net(skb->dev), skb, &opt_rx, 0, NULL);
#endif
	/*
	 * Was there an MSS?
	 */
	if (!opt_rx.mss_clamp) {
		DEBUG_WARN("No mss clamp in skb %px\n", skb);
		return false;
	}

	DEBUG_INFO("MSS clamp seen in %px as %u\n", skb, opt_rx.mss_clamp);
	*mss = opt_rx.mss_clamp;

	return true;
}

/*
 * ecm_tracker_tcp_segment_add_callback()
 *	Append the segment onto the tracker queue for the given target
 */
static bool ecm_tracker_tcp_segment_add_callback(struct ecm_tracker_tcp_instance *tti, ecm_tracker_sender_type_t sender,
								struct ecm_tracker_ip_header *ip_hdr, struct ecm_tracker_ip_protocol_header *ecm_tcp_header,
								struct tcphdr *tcp_hdr, struct sk_buff *skb)
{
	struct ecm_tracker_tcp_internal_instance *ttii = (struct ecm_tracker_tcp_internal_instance *)tti;
	struct sk_buff *skbc;
	struct ecm_tracker_tcp_host_data *data;
	struct ecm_tracker_tcp_skb_cb_format *skbc_cb;

	DEBUG_CHECK_MAGIC(ttii, ECM_TRACKER_TCP_INSTANCE_MAGIC, "%px: magic failed", ttii);
	DEBUG_ASSERT((sender >= 0) && (sender <= 1), "%px: invalid sender %d\n", ttii, sender);
	DEBUG_ASSERT(ecm_tcp_header->protocol_number == IPPROTO_TCP, "%px: invalid sender %u\n", ttii, ecm_tcp_header->protocol_number);
	DEBUG_TRACE("%px: segment %px add for %d\n", ttii, skb, sender);

	/*
	 * Clone the packet
	 */
	skbc = skb_clone(skb, GFP_ATOMIC | __GFP_NOWARN);
	if (!skbc) {
		DEBUG_WARN("%px: Failed to clone packet %px\n", ttii, skb);
		return false;
	}

	DEBUG_TRACE("%px: cloned %px to %px\n", ttii, skb, skbc);

	/*
	 * Get the private cb area and initialise it.
	 * ALL SEGMENTS HAVE TO HAVE ONE.
	 */
	skbc_cb = (struct ecm_tracker_tcp_skb_cb_format *)skbc->cb;
	DEBUG_SET_MAGIC(skbc_cb, ECM_TRACKER_TCP_SKB_CB_MAGIC);

	spin_lock_bh(&ttii->lock);

	/*
	 * Are we within instance limit?
	 */
	DEBUG_ASSERT((ttii->sender_data[ECM_TRACKER_SENDER_TYPE_SRC].recvd_bytes_total + ttii->sender_data[ECM_TRACKER_SENDER_TYPE_DEST].recvd_bytes_total + skbc->truesize) > 0, "%px: bad total\n", ttii);
	if ((ttii->sender_data[ECM_TRACKER_SENDER_TYPE_SRC].recvd_bytes_total + ttii->sender_data[ECM_TRACKER_SENDER_TYPE_DEST].recvd_bytes_total + skbc->truesize) > ttii->data_limit) {
		DEBUG_TRACE("%px: over limit\n", ttii);
		spin_unlock_bh(&ttii->lock);
		dev_kfree_skb_any(skbc);
		return false;
	}

	/*
	 * Within global limit?
	 */
	if (!ecm_tracker_data_total_increase(skbc->len, skbc->truesize)) {
		DEBUG_TRACE("%px: over global limit\n", ttii);
		spin_unlock_bh(&ttii->lock);
		dev_kfree_skb_any(skbc);
		return false;
	}

	/*
	 * Which list?
	 */
	data = &ttii->sender_data[sender];

	/*
	 * Do we have the MSS for this host?  Unlikely as most streams we would have seen the SYN already and retrieved the MSS.
	 */
	if (unlikely(tcp_hdr->syn) && unlikely(!data->mss_seen)) {
		uint16_t mss;
		if (ecm_tracker_tcp_extract_mss(skbc, &mss, ecm_tcp_header)) {
			data->mss = mss;
			data->mss_seen = true;
			DEBUG_TRACE("%px: Seen mss as %u for %px\n", ttii, mss, data);
		}
	}

	/*
	 * If sequence tracking is enabled but the segment is not wanted then we simply discard it
	 */
	if (!_ecm_tracker_tcp_stream_segment_add(ttii, data, ip_hdr, ecm_tcp_header, tcp_hdr, skbc, skbc_cb)) {
		spin_unlock_bh(&ttii->lock);
		ecm_tracker_data_total_decrease(skbc->len, skbc->truesize);
		dev_kfree_skb_any(skbc);

		/*
		 * Return true - there is nothing wrong with the datagram we just don't want this datagram but we will still
		 * allow it to pass and still want to account for it in the connection stats
		 */
		return true;
	}

	/*
	 * Store the packet in the reception order list.
	 * NOTE: When sequence tracking is not enabled we want to store things as they are received no matter what.
	 * When we are sequence tracking we only store the skb when the segment has been added successfully
	 */
	skbc_cb->next = NULL;
	skbc_cb->prev = data->recvd_order_last;
	data->recvd_order_last = skbc;
	if (!skbc_cb->prev) {
		DEBUG_ASSERT(data->recvd_order == NULL, "%px: bad list\n", ttii);
		data->recvd_order = skbc;
	} else {
		struct ecm_tracker_tcp_skb_cb_format *prev_cb;
		prev_cb = (struct ecm_tracker_tcp_skb_cb_format *)skbc_cb->prev->cb;
		DEBUG_CHECK_MAGIC(prev_cb, ECM_TRACKER_TCP_SKB_CB_MAGIC, "%px: invalid cb magic %px\n", ttii, prev_cb);
		prev_cb->next = skbc;
	}

	/*
	 * Update stats
	 */
	data->recvd_count++;
	DEBUG_ASSERT(data->recvd_count > 0, "%px: bad count\n", ttii);
	data->recvd_bytes_total += skbc->truesize;
	DEBUG_TRACE("%px: Segment %px added between %px and %px. recvd total %u, bytes total %u\n",
			ttii, skbc, skbc->prev, skbc->next, data->recvd_count, data->recvd_bytes_total);
	spin_unlock_bh(&ttii->lock);
	return true;
}

/*
 * ecm_tracker_tcp_datagram_add_callback()
 *	Append the datagram onto the tracker queue for the given target
 */
static bool ecm_tracker_tcp_datagram_add_callback(struct ecm_tracker_instance *ti, ecm_tracker_sender_type_t sender, struct sk_buff *skb)
{
	struct ecm_tracker_tcp_internal_instance *ttii = (struct ecm_tracker_tcp_internal_instance *)ti;
	struct sk_buff *skbc;
	struct ecm_tracker_tcp_skb_cb_format *skbc_cb;
	struct ecm_tracker_ip_header ip_hdr;
	struct ecm_tracker_ip_protocol_header *ecm_tcp_header;
	struct tcphdr tcp_hdr_buff;
	struct tcphdr *tcp_hdr;
	struct ecm_tracker_tcp_host_data *data;

	DEBUG_CHECK_MAGIC(ttii, ECM_TRACKER_TCP_INSTANCE_MAGIC, "%px: magic failed", ttii);
	DEBUG_ASSERT((sender >= 0) && (sender <= 1), "%px: invalid sender %d\n", ttii, sender);
	DEBUG_TRACE("%px: datagram %px add for sender: %d\n", ttii, skb, sender);

	/*
	 * Clone the packet
	 */
	skbc = skb_clone(skb, GFP_ATOMIC | __GFP_NOWARN);
	if (!skbc) {
		DEBUG_WARN("%px: Failed to clone packet %px\n", ttii, skb);
		return false;
	}

	DEBUG_TRACE("%px: cloned %px to %px\n", ttii, skb, skbc);

	/*
	 * Get the private cb area and initialise it.
	 * ALL SEGMENTS HAVE TO HAVE ONE.
	 */
	skbc_cb = (struct ecm_tracker_tcp_skb_cb_format *)skbc->cb;
	DEBUG_SET_MAGIC(skbc_cb, ECM_TRACKER_TCP_SKB_CB_MAGIC);

	/*
	 * Obtain the IP header from the skb
	 */
	if (!ecm_tracker_ip_check_header_and_read(&ip_hdr, skb)) {
		DEBUG_WARN("%px: no ip_hdr for %px\n", ttii, skbc);
		dev_kfree_skb_any(skbc);
		return false;
	}

	/*
	 * Extract the TCP header
	 */
	tcp_hdr = ecm_tracker_tcp_check_header_and_read(skbc, &ip_hdr, &tcp_hdr_buff);
	if (!tcp_hdr) {
		DEBUG_WARN("%px: invalid tcp header %px\n", ttii, skbc);
		dev_kfree_skb_any(skbc);
		return false;
	}
	ecm_tcp_header = &ip_hdr.headers[ECM_TRACKER_IP_PROTOCOL_TYPE_TCP];

	spin_lock_bh(&ttii->lock);

	/*
	 * Are we within instance limit?
	 */
	DEBUG_ASSERT((ttii->sender_data[ECM_TRACKER_SENDER_TYPE_SRC].recvd_bytes_total + ttii->sender_data[ECM_TRACKER_SENDER_TYPE_DEST].recvd_bytes_total + skbc->truesize) > 0, "%px: bad total\n", ttii);
	if ((ttii->sender_data[ECM_TRACKER_SENDER_TYPE_SRC].recvd_bytes_total + ttii->sender_data[ECM_TRACKER_SENDER_TYPE_DEST].recvd_bytes_total + skbc->truesize) > ttii->data_limit) {
		DEBUG_TRACE("%px: over limit: SRC recvd_bytes_total: %u, DEST recvd_bytes_total: %u, skb true_size: %u, limit: %u\n",
				ttii,
				ttii->sender_data[ECM_TRACKER_SENDER_TYPE_SRC].recvd_bytes_total,
				ttii->sender_data[ECM_TRACKER_SENDER_TYPE_DEST].recvd_bytes_total,
				skbc->truesize,
				ttii->data_limit);
		spin_unlock_bh(&ttii->lock);
		dev_kfree_skb_any(skbc);
		return false;
	}

	/*
	 * Within global limit?
	 */
	if (!ecm_tracker_data_total_increase(skbc->len, skbc->truesize)) {
		DEBUG_TRACE("%px: over global limit\n", ttii);
		spin_unlock_bh(&ttii->lock);
		dev_kfree_skb_any(skbc);
		return false;
	}

	/*
	 * Which list?
	 */
	data = &ttii->sender_data[sender];

	/*
	 * Do we have the MSS for this host?  Unlikely as most streams we would have seen the SYN already and retrieved the MSS.
	 */
	if (unlikely(tcp_hdr->syn) && unlikely(!data->mss_seen)) {
		uint16_t mss;
		if (ecm_tracker_tcp_extract_mss(skbc, &mss, ecm_tcp_header)) {
			data->mss = mss;
			data->mss_seen = true;
			DEBUG_TRACE("%px: Seen mss as %u for %px\n", ttii, mss, data);
		}
	}

	/*
	 * If sequence tracking is enabled but the segment is not wanted then we simply discard it
	 */
	if (!_ecm_tracker_tcp_stream_segment_add(ttii, data, &ip_hdr, ecm_tcp_header, tcp_hdr, skbc, skbc_cb)) {
		spin_unlock_bh(&ttii->lock);
		ecm_tracker_data_total_decrease(skbc->len, skbc->truesize);
		dev_kfree_skb_any(skbc);

		/*
		 * Return true - there is nothing wrong with the datagram we just don't want this datagram but we will still
		 * allow it to pass and still want to account for it in the connection stats
		 */
		return true;
	}

	/*
	 * Store the packet in the reception order list.
	 * NOTE: When sequence tracking is not enabled we want to store things as they are received no matter what.
	 * When we are sequence tracking we only store the skb when the segment has been added successfully
	 */
	skbc_cb->next = NULL;
	skbc_cb->prev = data->recvd_order_last;
	data->recvd_order_last = skbc;
	if (!skbc_cb->prev) {
		DEBUG_ASSERT(data->recvd_order == NULL, "%px: bad list\n", ttii);
		data->recvd_order = skbc;
	} else {
		struct ecm_tracker_tcp_skb_cb_format *prev_cb;
		prev_cb = (struct ecm_tracker_tcp_skb_cb_format *)skbc_cb->prev->cb;
		DEBUG_CHECK_MAGIC(prev_cb, ECM_TRACKER_TCP_SKB_CB_MAGIC, "%px: invalid cb magic %px\n", ttii, prev_cb);
		prev_cb->next = skbc;
	}

	/*
	 * Update stats
	 */
	data->recvd_count++;
	DEBUG_ASSERT(data->recvd_count > 0, "%px: bad count\n", ttii);
	data->recvd_bytes_total += skbc->truesize;
	DEBUG_TRACE("%px: Segment %px added between %px and %px.  recvd total %u, bytes total %u\n",
			ttii, skbc, skbc->prev, skbc->next, data->recvd_count, data->recvd_bytes_total);
	spin_unlock_bh(&ttii->lock);
	return true;
}

/*
 * ecm_tracker_tcp_bytes_avail_get_callback()
 *	Return number of bytes available to read
 */
static uint32_t ecm_tracker_tcp_bytes_avail_get_callback(struct ecm_tracker_tcp_instance *tti, ecm_tracker_sender_type_t sender)
{
	struct ecm_tracker_tcp_internal_instance *ttii = (struct ecm_tracker_tcp_internal_instance *)tti;
	struct ecm_tracker_tcp_host_data *data;
	uint32_t bytes_avail;

	DEBUG_CHECK_MAGIC(ttii, ECM_TRACKER_TCP_INSTANCE_MAGIC, "%px: magic failed", ttii);
	DEBUG_ASSERT((sender >= 0) && (sender <= 1), "%px: invalid sender %d\n", ttii, sender);

	/*
	 * Which list?
	 */
	data = &ttii->sender_data[sender];

	/*
	 * The number of available bytes is our sequence space
	 */
	spin_lock_bh(&ttii->lock);
	bytes_avail = data->num_seqs;
	spin_unlock_bh(&ttii->lock);

	DEBUG_TRACE("%px: get bytes available for %d: %u\n", ttii, sender, bytes_avail);
	return bytes_avail;
}

/*
 * ecm_tracker_tcp_bytes_read_callback()
 *	Read a number of bytes
 */
static int ecm_tracker_tcp_bytes_read_callback(struct ecm_tracker_tcp_instance *tti, ecm_tracker_sender_type_t sender, uint32_t offset, uint32_t size, void *buffer)
{
	struct ecm_tracker_tcp_internal_instance *ttii = (struct ecm_tracker_tcp_internal_instance *)tti;
	struct ecm_tracker_tcp_host_data *data;
	uint32_t read_start_seq;
	uint32_t seq_offset;
	uint32_t seg_seq_avail;
	struct sk_buff *skb;
	int res;
	uint8_t *p;
	struct ecm_tracker_tcp_skb_cb_format *skb_cb;

	DEBUG_CHECK_MAGIC(ttii, ECM_TRACKER_TCP_INSTANCE_MAGIC, "%px: magic failed", ttii);
	DEBUG_ASSERT((sender >= 0) && (sender <= 1), "%px: invalid sender %d\n", ttii, sender);

	/*
	 * Which list?
	 */
	data = &ttii->sender_data[sender];

	/*
	 * Compute the sequence to start reading at given the offset
	 */
	spin_lock_bh(&ttii->lock);
	DEBUG_ASSERT((offset + size) <= data->num_seqs, "%px: attempt to read %u bytes from offset %u.  Available bytes is %u\n", ttii, size, offset, data->num_seqs);
	read_start_seq = data->seq_no + offset;
	DEBUG_TRACE("%px: seq: %u, num_seqs: %u, offset: %u, size: %u, read_start_seq: %u\n", ttii, data->seq_no, data->num_seqs, offset, size, read_start_seq);

	/*
	 * Identify which skb in our sequence space seq_start begins at
	 */
	skb = data->in_order;
	while (skb) {
		uint32_t next_seg_seq;

		skb_cb = (struct ecm_tracker_tcp_skb_cb_format *)skb->cb;
		DEBUG_CHECK_MAGIC(skb_cb, ECM_TRACKER_TCP_SKB_CB_MAGIC, "%px: invalid cb magic %px\n", ttii, skb_cb);

		/*
		 * Get the sequence number of the start of the next segment
		 */
		next_seg_seq = skb_cb->seq_no + skb_cb->num_seqs;

		/*
		 * If the point at which we start reading is at or greater then the next segment sequence then we need to advance onto that
		 * i.e. skb does not contain the sequences we need
		 */
		if (read_start_seq >= next_seg_seq) {
			DEBUG_TRACE("%px: skip skb %px, we start at %u and this segment ends at %u\n", ttii, skb, read_start_seq, next_seg_seq - 1);
			skb = skb->next;
			continue;
		}

		/*
		 * We start reading at this skb
		 */
		DEBUG_TRACE("%px: skb to start reading from is at %px, seq_no of this is %u\n", ttii, skb, skb_cb->seq_no);
		break;
	}

	DEBUG_ASSERT(skb, "%px: no read start could be found\n", ttii);

	/*
	 * Finally perform read
	 */

	/*
	 * We may need to apply an offset to the first skb to get to our seq_start point
	 */
	seq_offset = read_start_seq - skb_cb->seq_no;
	seg_seq_avail = skb_cb->num_seqs - seq_offset;
	DEBUG_TRACE("%px: seq_offset: %u, avail: %u, want %u bytes\n", ttii, seq_offset, seg_seq_avail, size);
	if (seg_seq_avail > size) {
		seg_seq_avail = size;
	}

	/*
	 * Read that data.
	 */
	p = (uint8_t *)buffer;
	res = skb_copy_bits(skb, skb_cb->offset + seq_offset, p, seg_seq_avail);
	if (res < 0) {
		spin_unlock_bh(&ttii->lock);
		return res;
	}
	p += seg_seq_avail;
	size -= seg_seq_avail;

	/*
	 * Read bytes
	 */
	while (size) {
		uint32_t read_seq;

		skb = skb->next;
		DEBUG_ASSERT(skb, "%px: size %u not zero but no skb\n", ttii, size);

		skb_cb = (struct ecm_tracker_tcp_skb_cb_format *)skb->cb;
		DEBUG_CHECK_MAGIC(skb_cb, ECM_TRACKER_TCP_SKB_CB_MAGIC, "%px: invalid cb magic %px\n", ttii, skb_cb);

		/*
		 * Read available bytes from this segment.
		 * No offset here as it's linear all the way from this point on.
		 */
		read_seq = skb_cb->num_seqs;
		if (read_seq > size) {
			read_seq = size;
		}

		DEBUG_TRACE("%px: Read %u sequences from seq_no %u\n", ttii, read_seq, skb_cb->seq_no);

		res = skb_copy_bits(skb, skb_cb->offset, p, read_seq);
		if (res < 0) {
			spin_unlock_bh(&ttii->lock);
			return res;
		}
		p += read_seq;
		size -= read_seq;
	}

	spin_unlock_bh(&ttii->lock);
	return 0;
}

/*
 * ecm_tracker_tcp_mss_get_callback()
 *	Get the MSS as sent BY the given target i.e. the MSS the other party is allowed to send
 *
 * Returns true if the MSS was seen in a SYN or false is system default
 */
static bool ecm_tracker_tcp_mss_get_callback(struct ecm_tracker_tcp_instance *tti, ecm_tracker_sender_type_t sender, uint16_t *mss)
{
	struct ecm_tracker_tcp_internal_instance *ttii = (struct ecm_tracker_tcp_internal_instance *)tti;
	struct ecm_tracker_tcp_host_data *data;
	bool res;

	DEBUG_CHECK_MAGIC(ttii, ECM_TRACKER_TCP_INSTANCE_MAGIC, "%px: magic failed", ttii);
	DEBUG_ASSERT((sender >= 0) && (sender <= 1), "%px: invalid sender %d\n", ttii, sender);

	/*
	 * Which list?
	 */
	data = &ttii->sender_data[sender];

	spin_lock_bh(&ttii->lock);
	*mss = data->mss;
	res = data->mss_seen;
	spin_unlock_bh(&ttii->lock);

	DEBUG_TRACE("%px: get mss for %d: %u, seen? %u\n", ttii, sender, *mss, res);

	return res;
}

/*
 * ecm_tracker_tcp_data_total_get_callback()
 *	Return total tracked data
 */
static int32_t ecm_tracker_tcp_data_total_get_callback(struct ecm_tracker_instance *ti)
{
	struct ecm_tracker_tcp_internal_instance *ttii = (struct ecm_tracker_tcp_internal_instance *)ti;
	int32_t data_total;

	DEBUG_CHECK_MAGIC(ttii, ECM_TRACKER_TCP_INSTANCE_MAGIC, "%px: magic failed", ttii);

	spin_lock_bh(&ttii->lock);
	data_total = ttii->sender_data[ECM_TRACKER_SENDER_TYPE_SRC].recvd_bytes_total + ttii->sender_data[ECM_TRACKER_SENDER_TYPE_DEST].recvd_bytes_total;
	spin_unlock_bh(&ttii->lock);

	return data_total;
}

/*
 * ecm_tracker_tcp_data_limit_get_callback()
 *	Get tracked data limit
 */
static int32_t ecm_tracker_tcp_data_limit_get_callback(struct ecm_tracker_instance *ti)
{
	struct ecm_tracker_tcp_internal_instance *ttii = (struct ecm_tracker_tcp_internal_instance *)ti;
	int32_t data_limit;

	DEBUG_CHECK_MAGIC(ttii, ECM_TRACKER_TCP_INSTANCE_MAGIC, "%px: magic failed", ttii);

	spin_lock_bh(&ttii->lock);
	data_limit = ttii->data_limit;
	spin_unlock_bh(&ttii->lock);

	return data_limit;
}

/*
 * ecm_tracker_tcp_data_limit_set_callback()
 *	Set tracked data limit
 */
static void ecm_tracker_tcp_data_limit_set_callback(struct ecm_tracker_instance *ti, int32_t data_limit)
{
	struct ecm_tracker_tcp_internal_instance *ttii = (struct ecm_tracker_tcp_internal_instance *)ti;

	DEBUG_CHECK_MAGIC(ttii, ECM_TRACKER_TCP_INSTANCE_MAGIC, "%px: magic failed", ttii);

	spin_lock_bh(&ttii->lock);
	ttii->data_limit = data_limit;
	spin_unlock_bh(&ttii->lock);
}
#endif

/*
 * ecm_tracker_tcp_state_update_callback()
 * 	Update connection state based on the knowledge we have and the skb given
 */
static void ecm_tracker_tcp_state_update_callback(struct ecm_tracker_instance *ti, ecm_tracker_sender_type_t sender, struct ecm_tracker_ip_header *ip_hdr, struct sk_buff *skb)
{
	struct ecm_tracker_tcp_internal_instance *ttii = (struct ecm_tracker_tcp_internal_instance *)ti;
	struct tcphdr tcp_hdr_buff;
	struct tcphdr *tcp_hdr;
	struct ecm_tracker_tcp_sender_state *sender_state;
	struct ecm_tracker_tcp_sender_state *peer_state;

	DEBUG_CHECK_MAGIC(ttii, ECM_TRACKER_TCP_INSTANCE_MAGIC, "%px: magic failed", ttii);

	/*
	 * Get refereces to states
	 */
	DEBUG_ASSERT((sender >= 0) && (sender <= 1), "%px: invalid sender %d\n", ttii, sender);
	sender_state = &ttii->sender_states[sender];
	peer_state = &ttii->sender_states[!sender];

	/*
	 * Get tcp header
	 */
	tcp_hdr = ecm_tracker_tcp_check_header_and_read(skb, ip_hdr, &tcp_hdr_buff);
	if (unlikely(!tcp_hdr)) {
		DEBUG_WARN("%px: no tcp_hdr for %px\n", ttii, skb);
		spin_lock_bh(&ttii->lock);
		sender_state->state = ECM_TRACKER_SENDER_STATE_FAULT;
		peer_state->state = ECM_TRACKER_SENDER_STATE_FAULT;
		spin_unlock_bh(&ttii->lock);
		return;
	}

	/*
	 * If either side reports a reset this is catastrophic for the connection
	 */
	if (unlikely(tcp_hdr->rst)) {
		DEBUG_INFO("%px: RESET\n", ttii);
		spin_lock_bh(&ttii->lock);
		sender_state->state = ECM_TRACKER_SENDER_STATE_FAULT;
		peer_state->state = ECM_TRACKER_SENDER_STATE_FAULT;
		spin_unlock_bh(&ttii->lock);
		return;
	}

	/*
	 * Likely ack is set - this constitutes the mainstay of a TCP connection
	 * The sending of an ack may put the other side of the connection into a different state
	 */
	spin_lock_bh(&ttii->lock);
	if (likely(tcp_hdr->ack)) {
		ecm_tracker_sender_state_t peer_state_current = peer_state->state;
		uint32_t ack_seq = ntohl(tcp_hdr->ack_seq);

		switch (peer_state_current) {
		case ECM_TRACKER_SENDER_STATE_UNKNOWN: {
			/*
			 * Looks like we came into this connection mid-flow.
			 * Flag that the peer is established which is all we can infer right now and
			 * initialise the peers SYN sequence for further analysis of sequence space.
			 */
			peer_state->state = ECM_TRACKER_SENDER_STATE_ESTABLISHED;
			peer_state->syn_seq = (ack_seq - 1);		/* -1 is because the ACK has ack'd our ficticious SYN */
			DEBUG_INFO("%px: From unkown to established, ack_seq: %u, syn_seq: %u\n", ttii, ack_seq, peer_state->syn_seq);
			break;
		}
		case ECM_TRACKER_SENDER_STATE_ESTABLISHING: {
			int32_t ackd;
			uint32_t syn_seq;

			syn_seq = peer_state->syn_seq;
			ackd = (int32_t)(ack_seq - syn_seq);
			DEBUG_TRACE("%px: ack %u for syn_seq %u? ackd = %d\n", ttii, ack_seq, syn_seq, ackd);

			if (ackd <= 0) {
				DEBUG_TRACE("%px: No change\n", ttii);
			} else {
				DEBUG_INFO("%px: Established\n", ttii);
				peer_state->state = ECM_TRACKER_SENDER_STATE_ESTABLISHED;
			}
			break;
		}
		case ECM_TRACKER_SENDER_STATE_CLOSING: {
			int32_t ackd;
			uint32_t fin_seq;

			fin_seq = peer_state->fin_seq;
			ackd = (int32_t)(ack_seq - fin_seq);
			DEBUG_TRACE("%px: ack %u for fin_seq %u? ackd = %d\n", ttii, ack_seq, fin_seq, ackd);

			if (ackd <= 0) {
				DEBUG_TRACE("%px: No change\n", ttii);
			} else {
				DEBUG_TRACE("%px: Closed\n", ttii);
				peer_state->state = ECM_TRACKER_SENDER_STATE_CLOSED;
			}
			break;
		}
		case ECM_TRACKER_SENDER_STATE_ESTABLISHED:
		case ECM_TRACKER_SENDER_STATE_CLOSED:
		case ECM_TRACKER_SENDER_STATE_FAULT:
			/*
			 * No change
			 */
			break;
		default:
			DEBUG_ASSERT(false, "%px: unhandled state: %d\n", ttii, peer_state_current);
		}
	}

	/*
	 * Handle control flags sent by the sender (SYN & FIN)
	 * Handle SYN first because, in sequence space, SYN is first.
	 */
	if (tcp_hdr->syn) {
		ecm_tracker_sender_state_t sender_state_current = sender_state->state;
		uint32_t seq = ntohl(tcp_hdr->seq);

		switch (sender_state_current) {
		case ECM_TRACKER_SENDER_STATE_UNKNOWN:
			sender_state->state = ECM_TRACKER_SENDER_STATE_ESTABLISHING;
			sender_state->syn_seq = seq;		/* Seq is the sequence number of the SYN */
			DEBUG_INFO("%px: From unkown to establishing, syn_seq: %u\n", ttii, sender_state->syn_seq);
			break;
		case ECM_TRACKER_SENDER_STATE_CLOSING:
		case ECM_TRACKER_SENDER_STATE_CLOSED:
			/*
			 * SYN after seeing a FIN?  FAULT!
			 */
			sender_state->state = ECM_TRACKER_SENDER_STATE_FAULT;
			DEBUG_INFO("%px: SYN after FIN - fault\n", ttii);
			break;
		case ECM_TRACKER_SENDER_STATE_ESTABLISHED:
			/*
			 * SYN when established is just a duplicate down to syn/ack timing subtleties
			 */
		case ECM_TRACKER_SENDER_STATE_ESTABLISHING:
		case ECM_TRACKER_SENDER_STATE_FAULT:
			/*
			 * No change
			 */
			break;
		default:
			DEBUG_ASSERT(false, "%px: unhandled state: %d\n", ttii, sender_state_current);
		}
	}

	if (tcp_hdr->fin) {
		ecm_tracker_sender_state_t sender_state_current = sender_state->state;
		uint32_t seq = ntohl(tcp_hdr->seq);

		switch (sender_state_current) {
		case ECM_TRACKER_SENDER_STATE_UNKNOWN:
			/*
			 * Looks like we joined mid-flow.
			 * Have to set up both SYN and FIN.
			 * NOTE: It's possible that SYN is in the same packet as FIN, account for that in the seq numbers
			 */
			sender_state->state = ECM_TRACKER_SENDER_STATE_CLOSING;
			if (tcp_hdr->syn) {
				sender_state->syn_seq = seq;
				sender_state->fin_seq = seq + 1;
			} else {
				sender_state->fin_seq = seq;		/* seq is the FIN sequence */
				sender_state->syn_seq = seq - 1;	/* Make a guess at what the SYN was */
			}
			DEBUG_INFO("%px: From unkown to closing, syn_seq: %u, fin_seq: %u\n", ttii, sender_state->syn_seq, sender_state->fin_seq);
			break;
		case ECM_TRACKER_SENDER_STATE_ESTABLISHED:
			/*
			 * Connection becomes closing.
			 */
			sender_state->state = ECM_TRACKER_SENDER_STATE_CLOSING;
			sender_state->fin_seq = seq;
			DEBUG_INFO("%px: From established to closing, fin_seq: %u\n", ttii, sender_state->fin_seq);
			break;
		case ECM_TRACKER_SENDER_STATE_ESTABLISHING: {
			int32_t newer;

			/*
			 * FIN while waiting for SYN to be acknowledged is possible but only if it
			 * it is in the same packet or later sequence space
			 */
			newer = (int32_t)(seq - sender_state->syn_seq);
			if (!tcp_hdr->syn || (newer <= 0)) {
				DEBUG_INFO("%px: From establishing to fault - odd FIN seen, syn: %u, syn_seq: %u, newer: %d\n",
						ttii, tcp_hdr->syn, sender_state->syn_seq, newer);
				sender_state->state = ECM_TRACKER_SENDER_STATE_FAULT;
			} else {
				uint32_t fin_seq = seq;
				if (tcp_hdr->syn) {
					fin_seq++;
				}
				sender_state->state = ECM_TRACKER_SENDER_STATE_CLOSING;
				DEBUG_INFO("%px: From establishing to closing, syn: %u, syn_seq: %u, fin_seq: %u\n",
						ttii, tcp_hdr->syn, sender_state->syn_seq, sender_state->fin_seq);
			}
			break;
		}
		case ECM_TRACKER_SENDER_STATE_CLOSING:
		case ECM_TRACKER_SENDER_STATE_CLOSED:
		case ECM_TRACKER_SENDER_STATE_FAULT:
			/*
			 * No change
			 */
			break;
		default:
			DEBUG_ASSERT(false, "%px: unhandled state: %d\n", ttii, sender_state_current);
		}
	}

	spin_unlock_bh(&ttii->lock);
}

/*
 * ecm_tracker_tcp_state_get_callback()
 * 	Get state
 */
static void ecm_tracker_tcp_state_get_callback(struct ecm_tracker_instance *ti, ecm_tracker_sender_state_t *src_state,
					ecm_tracker_sender_state_t *dest_state, ecm_tracker_connection_state_t *state, ecm_db_timer_group_t *tg)
{
	struct ecm_tracker_tcp_internal_instance *ttii = (struct ecm_tracker_tcp_internal_instance *)ti;
	DEBUG_CHECK_MAGIC(ttii, ECM_TRACKER_TCP_INSTANCE_MAGIC, "%px: magic failed", ttii);
	spin_lock_bh(&ttii->lock);
	*src_state = ttii->sender_states[ECM_TRACKER_SENDER_TYPE_SRC].state;
	*dest_state = ttii->sender_states[ECM_TRACKER_SENDER_TYPE_DEST].state;
	spin_unlock_bh(&ttii->lock);
	*state = ecm_tracker_tcp_connection_state_matrix[*src_state][*dest_state];
	*tg = ecm_tracker_tcp_timer_group_from_state[*state];
}

#ifdef ECM_STATE_OUTPUT_ENABLE
/*
 * ecm_tracker_tcp_sender_state_get()
 *	Return state
 */
static int ecm_tracker_tcp_sender_state_get(struct ecm_state_file_instance *sfi, ecm_tracker_sender_type_t sender,
#ifdef ECM_TRACKER_DPI_SUPPORT_ENABLE
						struct ecm_tracker_tcp_host_data *data,
#endif
						struct ecm_tracker_tcp_sender_state *state)
{
	int result;

	if ((result = ecm_state_write(sfi, "state", "%s", ecm_tracker_sender_state_to_string(state->state)))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "syn_seq", "%u", state->syn_seq))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "fin_seq", "%u", state->fin_seq))) {
		return result;
	}
#ifdef ECM_TRACKER_DPI_SUPPORT_ENABLE
	if ((result = ecm_state_write(sfi, "mss_seen", "%s", (data->mss_seen)? "yes" : "no"))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "mss", "%u", data->mss))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "seq_no", "%u", data->seq_no))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "num_seqs", "%u", data->num_seqs))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "seq_valid", "%s", (data->seq_no_valid)? "yes" : "no"))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "bytes_total", "%d", data->recvd_bytes_total))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "buffers_total", "%d", data->recvd_count))) {
		return result;
	}
#endif
	return 0;
}

/*
 * ecm_tracker_tcp_state_text_get_callback()
 *	Return state
 */
static int ecm_tracker_tcp_state_text_get_callback(struct ecm_tracker_instance *ti, struct ecm_state_file_instance *sfi)
{
	int result;
	struct ecm_tracker_tcp_internal_instance *ttii = (struct ecm_tracker_tcp_internal_instance *)ti;
	struct ecm_tracker_tcp_sender_state sender_states[ECM_TRACKER_SENDER_MAX];
#ifdef ECM_TRACKER_DPI_SUPPORT_ENABLE
	struct ecm_tracker_tcp_host_data sender_data[ECM_TRACKER_SENDER_MAX];
	int32_t data_limit;
#endif
	ecm_tracker_connection_state_t connection_state;
	DEBUG_CHECK_MAGIC(ttii, ECM_TRACKER_TCP_INSTANCE_MAGIC, "%px: magic failed", ttii);

	if ((result = ecm_state_prefix_add(sfi, "tracker_tcp"))) {
		return result;
	}

	/*
	 * Capture state
	 */
	spin_lock_bh(&ttii->lock);
#ifdef ECM_TRACKER_DPI_SUPPORT_ENABLE
	data_limit = ttii->data_limit;
	sender_data[ECM_TRACKER_SENDER_TYPE_SRC] = ttii->sender_data[ECM_TRACKER_SENDER_TYPE_SRC];
	sender_data[ECM_TRACKER_SENDER_TYPE_DEST] = ttii->sender_data[ECM_TRACKER_SENDER_TYPE_DEST];
#endif
	sender_states[ECM_TRACKER_SENDER_TYPE_SRC] = ttii->sender_states[ECM_TRACKER_SENDER_TYPE_SRC];
	sender_states[ECM_TRACKER_SENDER_TYPE_DEST] = ttii->sender_states[ECM_TRACKER_SENDER_TYPE_DEST];
	spin_unlock_bh(&ttii->lock);
	connection_state = ecm_tracker_tcp_connection_state_matrix[sender_states[ECM_TRACKER_SENDER_TYPE_SRC].state][sender_states[ECM_TRACKER_SENDER_TYPE_DEST].state];

	if ((result = ecm_state_write(sfi, "connection_state", "%s", ecm_tracker_connection_state_to_string(connection_state)))) {
		return result;
	}
#ifdef ECM_TRACKER_DPI_SUPPORT_ENABLE
	if ((result = ecm_state_write(sfi, "data_limit", "%d", data_limit))) {
		return result;
	}
#endif

	if ((result = ecm_state_prefix_add(sfi, "senders"))) {
		return result;
	}

	/*
	 * Output src sender
	 */
	if ((result = ecm_state_prefix_add(sfi, "src"))) {
		return result;
	}
	if ((result = ecm_tracker_tcp_sender_state_get(sfi,
			ECM_TRACKER_SENDER_TYPE_SRC,
#ifdef ECM_TRACKER_DPI_SUPPORT_ENABLE
			&sender_data[ECM_TRACKER_SENDER_TYPE_SRC],
#endif
			&sender_states[ECM_TRACKER_SENDER_TYPE_SRC]))) {
		return result;
	}
	if ((result = ecm_state_prefix_remove(sfi))) {
		return result;
	}

	/*
	 * Output dest sender
	 */
	if ((result = ecm_state_prefix_add(sfi, "dest"))) {
		return result;
	}
	if ((result = ecm_tracker_tcp_sender_state_get(sfi,
			ECM_TRACKER_SENDER_TYPE_DEST,
#ifdef ECM_TRACKER_DPI_SUPPORT_ENABLE
			&sender_data[ECM_TRACKER_SENDER_TYPE_DEST],
#endif
			&sender_states[ECM_TRACKER_SENDER_TYPE_DEST]))) {
		return result;
	}
	if ((result = ecm_state_prefix_remove(sfi))) {
		return result;
	}

	if ((result = ecm_state_prefix_remove(sfi))) {
		return result;
	}

 	return ecm_state_prefix_remove(sfi);
}
#endif

/*
 * ecm_tracker_tcp_init()
 *	Initialise the instance
 */
void ecm_tracker_tcp_init(struct ecm_tracker_tcp_instance *tti, int32_t data_limit, uint16_t mss_src_default, uint16_t mss_dest_default)
{
	struct ecm_tracker_tcp_internal_instance *ttii = (struct ecm_tracker_tcp_internal_instance *)tti;
	DEBUG_CHECK_MAGIC(ttii, ECM_TRACKER_TCP_INSTANCE_MAGIC, "%px: magic failed", ttii);
	DEBUG_TRACE("%px: init host addresses src mss: %d, dest mss: %d\n", ttii, mss_src_default, mss_dest_default);
#ifdef ECM_TRACKER_DPI_SUPPORT_ENABLE
	spin_lock_bh(&ttii->lock);
	ttii->data_limit = data_limit;
	ttii->sender_data[ECM_TRACKER_SENDER_TYPE_SRC].mss = mss_src_default;
	ttii->sender_data[ECM_TRACKER_SENDER_TYPE_DEST].mss = mss_dest_default;
	spin_unlock_bh(&ttii->lock);
#endif
}
EXPORT_SYMBOL(ecm_tracker_tcp_init);

/*
 * ecm_tracker_tcp_alloc()
 */
struct ecm_tracker_tcp_instance *ecm_tracker_tcp_alloc(void)
{
	struct ecm_tracker_tcp_internal_instance *ttii;

	ttii = (struct ecm_tracker_tcp_internal_instance *)kzalloc(sizeof(struct ecm_tracker_tcp_internal_instance), GFP_ATOMIC | __GFP_NOWARN);
	if (!ttii) {
		DEBUG_WARN("Failed to allocate tcp tracker instance\n");
		return NULL;
	}

	ttii->tcp_base.base.ref = ecm_tracker_tcp_ref_callback;
	ttii->tcp_base.base.deref = ecm_tracker_tcp_deref_callback;
	ttii->tcp_base.base.state_update = ecm_tracker_tcp_state_update_callback;
	ttii->tcp_base.base.state_get = ecm_tracker_tcp_state_get_callback;
#ifdef ECM_TRACKER_DPI_SUPPORT_ENABLE
	ttii->tcp_base.base.datagram_count_get = ecm_tracker_tcp_datagram_count_get_callback;
	ttii->tcp_base.base.datagram_discard = ecm_tracker_tcp_datagram_discard_callback;
	ttii->tcp_base.base.datagram_read = ecm_tracker_tcp_datagram_read_callback;
	ttii->tcp_base.base.datagram_size_get = ecm_tracker_tcp_datagram_size_get_callback;
	ttii->tcp_base.base.datagram_add = ecm_tracker_tcp_datagram_add_callback;
	ttii->tcp_base.base.discard_all = ecm_tracker_tcp_discard_all_callback;
	ttii->tcp_base.base.data_total_get = ecm_tracker_tcp_data_total_get_callback;
	ttii->tcp_base.base.data_limit_get = ecm_tracker_tcp_data_limit_get_callback;
	ttii->tcp_base.base.data_limit_set = ecm_tracker_tcp_data_limit_set_callback;

	ttii->tcp_base.bytes_avail_get = ecm_tracker_tcp_bytes_avail_get_callback;
	ttii->tcp_base.bytes_read = ecm_tracker_tcp_bytes_read_callback;
	ttii->tcp_base.bytes_discard = ecm_tracker_tcp_bytes_discard_callback;
	ttii->tcp_base.mss_get = ecm_tracker_tcp_mss_get_callback;
	ttii->tcp_base.segment_add = ecm_tracker_tcp_segment_add_callback;
#endif
#ifdef ECM_STATE_OUTPUT_ENABLE
	ttii->tcp_base.base.state_text_get = ecm_tracker_tcp_state_text_get_callback;
#endif

	spin_lock_init(&ttii->lock);

	ttii->refs = 1;
	DEBUG_SET_MAGIC(ttii, ECM_TRACKER_TCP_INSTANCE_MAGIC);

	spin_lock_bh(&ecm_tracker_tcp_lock);
	ecm_tracker_tcp_count++;
	DEBUG_ASSERT(ecm_tracker_tcp_count > 0, "%px: tcp tracker count wrap\n", ttii);
	spin_unlock_bh(&ecm_tracker_tcp_lock);

	DEBUG_TRACE("TCP tracker created %px\n", ttii);
	return (struct ecm_tracker_tcp_instance *)ttii;
}
EXPORT_SYMBOL(ecm_tracker_tcp_alloc);

#ifdef ECM_TRACKER_DPI_SUPPORT_ENABLE
/*
 * ecm_tracker_tcp_reader_fwd_read_u8()
 *	Read a byte, advancing the read position
 */
uint8_t ecm_tracker_tcp_reader_fwd_read_u8(struct ecm_tracker_tcp_reader_instance *tri)
{
	uint8_t b;

	DEBUG_CHECK_MAGIC(tri, ECM_TRACKER_TCP_READER_INSTANCE_MAGIC, "%px: magic failed", tri);

	spin_lock_bh(&tri->lock);
	spin_lock_bh(&tri->ttii->lock);

	/*
	 * Do we need to advance onto the next segment?  Unlikely as most segments contain a lot more than 1 byte of data.
	 */
	if (unlikely(tri->segment_remain == 0)) {
		if (likely(tri->segment)) {
			tri->segment = tri->segment->next;
		} else {
			/*
			 * Initialise segment to allow the walking of the in_order list
			 */
			tri->segment = tri->data->in_order;
		}
		DEBUG_ASSERT(tri->segment, "%px: attempt to read past end of stream\n", tri);

		tri->segment_dcb = (struct ecm_tracker_tcp_skb_cb_format *)tri->segment->cb;
		DEBUG_CHECK_MAGIC(tri->segment_dcb, ECM_TRACKER_TCP_SKB_CB_MAGIC, "%px: invalid magic for cb %px\n", tri, tri->segment_dcb);

		tri->segment_remain = tri->segment_dcb->num_seqs;
		tri->segment_offset = tri->segment_dcb->offset;

		DEBUG_TRACE("%px: Advance to next segment %px, segment remain: %u, segment offset: %u\n",
				tri, tri->segment, tri->segment_remain, tri->segment_offset);
		DEBUG_ASSERT(tri->segment_remain, "%px: Next segment has no data\n", tri);
	}

	/*
	 * Read byte
	 */
	skb_copy_bits(tri->segment, tri->segment_offset, &b, 1);
	tri->segment_offset++;
	tri->segment_remain--;
	tri->offset++;

	spin_unlock_bh(&tri->ttii->lock);
	spin_unlock_bh(&tri->lock);

	return b;
}
EXPORT_SYMBOL(ecm_tracker_tcp_reader_fwd_read_u8);

/*
 * _ecm_tracker_tcp_reader_retreat()
 *	Retreat reading position by number of bytes.
 *
 * This function CANNOT advance.
 */
static void _ecm_tracker_tcp_reader_retreat(struct ecm_tracker_tcp_reader_instance *tri, uint32_t retreatment)
{
	while (retreatment) {
		uint32_t seg_prior_data;

		/*
		 * Identify how much DATA precedes our current tri->segment_offset.
		 * This tells us how much data we can retreat over before having to move onto the prior segment
		 */
		seg_prior_data = tri->segment_offset - tri->segment_dcb->offset;
		DEBUG_TRACE("%px: Retreat %u, data_offs %u\n", tri, retreatment, seg_prior_data);

		/*
		 * Is there enough prior data in the current segment to satisfy the retreat?
		 */
		if (retreatment <= seg_prior_data) {
			tri->segment_remain += retreatment;
			tri->segment_offset -= retreatment;
			DEBUG_TRACE("%px: retreat completed seg_remain %u, seg offset %u\n", tri, tri->segment_remain, tri->segment_offset);
			return;
		}

		retreatment -= seg_prior_data;

		/*
		 * Move onto previous actual segment
		 */
		tri->segment = tri->segment->prev;
		DEBUG_ASSERT(tri->segment, "%px: attempt to read past start of stream\n", tri);

		tri->segment_dcb = (struct ecm_tracker_tcp_skb_cb_format *)tri->segment->cb;
		DEBUG_CHECK_MAGIC(tri->segment_dcb, ECM_TRACKER_TCP_SKB_CB_MAGIC, "%px: invalid magic for cb %px\n", tri, tri->segment_dcb);

		/*
		 * Position at the end of the previous segment
		 */
		tri->segment_remain = 0;
		tri->segment_offset = tri->segment_dcb->num_seqs + tri->segment_dcb->offset;

		DEBUG_TRACE("%px: advanvcement now %u this seg_remain %u, this seg_offset %u\n",
				tri, retreatment, tri->segment_remain, tri->segment_offset);
	}
}

/*
 * ecm_tracker_tcp_reader_retreat()
 *	Retreat reading position by number of bytes
 */
void ecm_tracker_tcp_reader_retreat(struct ecm_tracker_tcp_reader_instance *tri, uint32_t retreatment)
{
	DEBUG_CHECK_MAGIC(tri, ECM_TRACKER_TCP_READER_INSTANCE_MAGIC, "%px: magic failed", tri);

	spin_lock_bh(&tri->lock);
	spin_lock_bh(&tri->ttii->lock);

	DEBUG_ASSERT(tri->segment, "%px: segment pointer invalid\n", tri);
	_ecm_tracker_tcp_reader_retreat(tri, retreatment);
	tri->offset -= retreatment;

	spin_unlock_bh(&tri->ttii->lock);
	spin_unlock_bh(&tri->lock);
}
EXPORT_SYMBOL(ecm_tracker_tcp_reader_retreat);

/*
 * _ecm_tracker_tcp_reader_advance()
 *	Advance reading position by number of bytes.
 *
 * This function CANNOT retreat.
 */
static void _ecm_tracker_tcp_reader_advance(struct ecm_tracker_tcp_reader_instance *tri, uint32_t advancement)
{
	while (advancement) {
		DEBUG_TRACE("%px: Advance %u\n", tri, advancement);
		if (advancement <= tri->segment_remain) {
			tri->segment_remain -= advancement;
			tri->segment_offset += advancement;
			DEBUG_TRACE("%px: advanvce completed seg_remain %u, seg offset %u\n", tri, tri->segment_remain, tri->segment_offset);
			return;
		}

		advancement -= tri->segment_remain;

		/*
		 * Move onto next actual segment
		 */
		if (likely(tri->segment)) {
			tri->segment = tri->segment->next;
		} else {
			/*
			 * Initialise segment to allow the walking of the in_order list
			 */
			tri->segment = tri->data->in_order;
		}
		DEBUG_ASSERT(tri->segment, "%px: attempt to read past end of stream\n", tri);

		tri->segment_dcb = (struct ecm_tracker_tcp_skb_cb_format *)tri->segment->cb;
		DEBUG_CHECK_MAGIC(tri->segment_dcb, ECM_TRACKER_TCP_SKB_CB_MAGIC, "%px: invalid magic for cb %px\n", tri, tri->segment_dcb);

		tri->segment_remain = tri->segment_dcb->num_seqs;
		tri->segment_offset = tri->segment_dcb->offset;

		DEBUG_TRACE("%px: advanvcement now %u this seg_remain %u, this seg_offset %u\n",
				tri, advancement, tri->segment_remain, tri->segment_offset);
	}
}

/*
 * ecm_tracker_tcp_reader_advance()
 *	Advance reading position by number of bytes
 */
void ecm_tracker_tcp_reader_advance(struct ecm_tracker_tcp_reader_instance *tri, uint32_t advancement)
{
	DEBUG_CHECK_MAGIC(tri, ECM_TRACKER_TCP_READER_INSTANCE_MAGIC, "%px: magic failed", tri);

	spin_lock_bh(&tri->lock);
	spin_lock_bh(&tri->ttii->lock);

	_ecm_tracker_tcp_reader_advance(tri, advancement);
	tri->offset += advancement;

	spin_unlock_bh(&tri->ttii->lock);
	spin_unlock_bh(&tri->lock);
}
EXPORT_SYMBOL(ecm_tracker_tcp_reader_advance);

/*
 * ecm_tracker_tcp_reader_position_get()
 *	Return the current reading position
 *
 * NOTE: position is always referenced from the ORIGINAL 0, EVEN AFTER discarding bytes.
 * For example, calling:
 * _position_set(50)
 * _discard_preceding()
 * _position_get() WILL RETURN 50
 * I.e. positions are not shifted left by a discarded operation.
 * This is so that once a stream is parsed and positions known, data can be discarded without
 * having to recalculate all notions of positions.
 */
uint32_t ecm_tracker_tcp_reader_position_get(struct ecm_tracker_tcp_reader_instance *tri)
{
	uint32_t pos;

	DEBUG_CHECK_MAGIC(tri, ECM_TRACKER_TCP_READER_INSTANCE_MAGIC, "%px: magic failed", tri);

	spin_lock_bh(&tri->lock);
	pos = tri->offset;
	spin_unlock_bh(&tri->lock);
	return pos;
}
EXPORT_SYMBOL(ecm_tracker_tcp_reader_position_get);

/*
 * ecm_tracker_tcp_reader_remain_get()
 *	Return the available bytes to be read from the current position
 */
uint32_t ecm_tracker_tcp_reader_remain_get(struct ecm_tracker_tcp_reader_instance *tri)
{
	uint32_t remain;

	DEBUG_CHECK_MAGIC(tri, ECM_TRACKER_TCP_READER_INSTANCE_MAGIC, "%px: magic failed", tri);

	spin_lock_bh(&tri->lock);
	spin_lock_bh(&tri->ttii->lock);
	remain = tri->data->num_seqs - (tri->offset - tri->discarded);
	spin_unlock_bh(&tri->ttii->lock);
	spin_unlock_bh(&tri->lock);
	return remain;
}
EXPORT_SYMBOL(ecm_tracker_tcp_reader_remain_get);

/*
 * ecm_tracker_tcp_reader_position_set()
 *	Set the reading position as an offset from the start
 */
void ecm_tracker_tcp_reader_position_set(struct ecm_tracker_tcp_reader_instance *tri, uint32_t offset)
{
	int32_t delta;

	DEBUG_CHECK_MAGIC(tri, ECM_TRACKER_TCP_READER_INSTANCE_MAGIC, "%px: magic failed", tri);

	DEBUG_TRACE("%px: Position set %u\n", tri, offset);

	spin_lock_bh(&tri->lock);
	spin_lock_bh(&tri->ttii->lock);

	/*
	 * Is this a retreat or an advance?
	 */
	delta = (int32_t)(offset - tri->offset);
	DEBUG_TRACE("%px: Position set delta %d\n", tri, delta);
	if (delta <= 0) {
		_ecm_tracker_tcp_reader_retreat(tri, -delta);
	} else {
		_ecm_tracker_tcp_reader_advance(tri, delta);
	}
	tri->offset = offset;

	spin_unlock_bh(&tri->ttii->lock);
	spin_unlock_bh(&tri->lock);
}
EXPORT_SYMBOL(ecm_tracker_tcp_reader_position_set);

/*
 * ecm_tracker_tcp_reader_discard_preceding()
 *	Discard bytes from beginning of available data before the reader position
 *
 * NOTE: After discarding you DO NOT have to modify your notions of positions i.e. position X is still position X to you.
 * Obviously deleted positions are no longer valid but positions that have not been discarded remain valid.
 */
void ecm_tracker_tcp_reader_discard_preceding(struct ecm_tracker_tcp_reader_instance *tri)
{
	uint32_t discard;

	DEBUG_CHECK_MAGIC(tri, ECM_TRACKER_TCP_READER_INSTANCE_MAGIC, "%px: magic failed", tri);

	/*
	 * Discard all bytes preceding our offset
	 */
	spin_lock_bh(&tri->lock);
	discard = tri->offset - tri->discarded;
	tri->discarded += discard;

	/*
	 * If the reader position is at the end of a segment then the tri->segment is going to be ripped from
	 * under the reader.
	 */
	if (!tri->segment_remain) {
		/*
		 * By setting the segment to null we effectively cause a re-init of the tri->segment on the next operation
		 */
		DEBUG_TRACE("%px: Discarding when segment_remain is 0 - causing re-init of tri->segment on next oper\n", tri);
		tri->segment = NULL;
	}

	spin_unlock_bh(&tri->lock);

	/*
	 * Discard the actual bytes in the tracker
	 */
	DEBUG_TRACE("%px: discard_preceding %u bytes\n", tri, discard);
	ecm_tracker_tcp_bytes_discard(tri->ttii, tri->data, discard);
}
EXPORT_SYMBOL(ecm_tracker_tcp_reader_discard_preceding);

/*
 * ecm_tracker_tcp_reader_init()
 */
void ecm_tracker_tcp_reader_init(struct ecm_tracker_tcp_reader_instance *tri, struct ecm_tracker_tcp_instance *tti, ecm_tracker_sender_type_t sender)
{
	struct ecm_tracker_tcp_internal_instance *ttii = (struct ecm_tracker_tcp_internal_instance *)tti;
	struct ecm_tracker_tcp_host_data *data;

	DEBUG_CHECK_MAGIC(tri, ECM_TRACKER_TCP_READER_INSTANCE_MAGIC, "%px: magic failed", tri);
	DEBUG_CHECK_MAGIC(ttii, ECM_TRACKER_TCP_INSTANCE_MAGIC, "%px: magic failed", ttii);
	DEBUG_ASSERT((sender >= 0) && (sender <= 1), "%px: invalid sender %d\n", tri, sender);

	/*
	 * Take a reference to the tcp tracker
	 */
	((struct ecm_tracker_instance *)tti)->ref((struct ecm_tracker_instance *)tti);

	/*
	 * Get a pointer to the host data
	 */
	data = &ttii->sender_data[sender];

	/*
	 * take a copy of the ttii and data pointers for quick reference later on during reading
	 */
	tri->ttii = ttii;
	tri->data = data;

	DEBUG_TRACE("%px: Init reader for %px (%d): data %px.  \n", tri, ttii, sender, data);
}
EXPORT_SYMBOL(ecm_tracker_tcp_reader_init);

/*
 * ecm_tracker_tcp_reader_ref()
 */
void ecm_tracker_tcp_reader_ref(struct ecm_tracker_tcp_reader_instance *tri)
{
	DEBUG_CHECK_MAGIC(tri, ECM_TRACKER_TCP_READER_INSTANCE_MAGIC, "%px: magic failed", tri);

	spin_lock_bh(&tri->lock);

	tri->refs++;
	DEBUG_ASSERT(tri->refs > 0, "%px: ref wrap", tri);
	DEBUG_TRACE("%px: ref %d\n", tri, tri->refs);

	spin_unlock_bh(&tri->lock);
}
EXPORT_SYMBOL(ecm_tracker_tcp_reader_ref);

/*
 * ecm_tracker_tcp_reader_deref()
 */
int ecm_tracker_tcp_reader_deref(struct ecm_tracker_tcp_reader_instance *tri)
{
	int refs;
	DEBUG_CHECK_MAGIC(tri, ECM_TRACKER_TCP_READER_INSTANCE_MAGIC, "%px: magic failed", tri);

	spin_lock_bh(&tri->lock);
	tri->refs--;
	refs = tri->refs;
	DEBUG_ASSERT(tri->refs >= 0, "%px: ref wrap", tri);
	DEBUG_TRACE("%px: deref %d\n", tri, tri->refs);

	if (tri->refs > 0) {
		spin_unlock_bh(&tri->lock);
		return refs;
	}
	spin_unlock_bh(&tri->lock);

	DEBUG_TRACE("%px: final\n", tri);

	/*
	 * Release the reference to the tcp tracker
	 */
	((struct ecm_tracker_instance *)tri->ttii)->deref((struct ecm_tracker_instance *)tri->ttii);

	spin_lock_bh(&ecm_tracker_tcp_lock);
	ecm_tracker_tcp_reader_count--;
	DEBUG_ASSERT(ecm_tracker_tcp_reader_count >= 0, "%px: tracker count wrap", tri);
	spin_unlock_bh(&ecm_tracker_tcp_lock);

	DEBUG_INFO("%px: TCP Reader final\n", tri);
	DEBUG_CLEAR_MAGIC(tri);
	kfree(tri);

	return 0;
}
EXPORT_SYMBOL(ecm_tracker_tcp_reader_deref);

/*
 * ecm_tracker_tcp_reader_alloc()
 */
struct ecm_tracker_tcp_reader_instance *ecm_tracker_tcp_reader_alloc(void)
{
	struct ecm_tracker_tcp_reader_instance *tri;

	tri = (struct ecm_tracker_tcp_reader_instance *)kzalloc(sizeof(struct ecm_tracker_tcp_reader_instance), GFP_ATOMIC | __GFP_NOWARN);
	if (!tri) {
		DEBUG_WARN("Failed to allocate tcp reader instance\n");
		return NULL;
	}

	spin_lock_init(&tri->lock);
	tri->refs = 1;
	DEBUG_SET_MAGIC(tri, ECM_TRACKER_TCP_READER_INSTANCE_MAGIC);

	spin_lock_bh(&ecm_tracker_tcp_lock);
	ecm_tracker_tcp_reader_count++;
	DEBUG_ASSERT(ecm_tracker_tcp_reader_count > 0, "%px: tcp tracker reader count wrap\n", tri);
	spin_unlock_bh(&ecm_tracker_tcp_lock);

	DEBUG_TRACE("TCP reader created %px\n", tri);
	return tri;
}
EXPORT_SYMBOL(ecm_tracker_tcp_reader_alloc);
#endif
