/*
 * rtp.c
 * Copyright (C) 2009-2011 by ipoque GmbH
 * 
 * This file is part of OpenDPI, an open source deep packet inspection
 * library based on the PACE technology by ipoque GmbH
 * 
 * OpenDPI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * OpenDPI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with OpenDPI.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */


#include "ipq_utils.h"
#ifdef IPOQUE_PROTOCOL_RTP

#define RTP_MAX_OUT_OF_ORDER 11

static void ipoque_int_rtp_add_connection(struct ipoque_detection_module_struct
										  *ipoque_struct)
{
	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_RTP, IPOQUE_REAL_PROTOCOL);
}

/*
 * maintenance of current highest sequence number, cycle count, packet counter
 * adapted from RFC3550 Appendix A.1
 *
 * In their formulation, it is not possible to represent "no packets sent yet". This is fixed here by defining
 * baseseq to be the sequence number of the first packet minus 1 (in other words, the sequence number of the
 * zeroth packet).
 *
 * Note: As described in the RFC, the number of packets received includes retransmitted packets.
 * This means the "packets lost" count (seq_num-isn+1)-received can become negative.
 *
 * include_current_packet should be
 *   1, if the current packet should count towards the total, or
 *   0, if it it regarded as belonging to the previous reporting interval
 */
	
#if !(defined(HAVE_NTOP) && defined(WIN32))
 static inline
#else
__forceinline static
#endif
	 void init_seq(struct ipoque_detection_module_struct *ipoque_struct, struct ipoque_flow_struct *flow,
							u8 direction, u16 seq, u8 include_current_packet)
{
	flow->rtp_seqnum[direction] = seq;
	IPQ_LOG(IPOQUE_PROTOCOL_RTP, ipoque_struct, IPQ_LOG_DEBUG, "rtp_seqnum[%u] = %u\n", direction, seq);
}

/* returns difference between old and new highest sequence number */
	
#if !(defined(HAVE_NTOP) && defined(WIN32))
 static inline
#else
__forceinline static
#endif
	 u16 update_seq(struct ipoque_detection_module_struct *ipoque_struct, struct ipoque_flow_struct *flow,
							 u8 direction, u16 seq)
{
	u16 delta = seq - flow->rtp_seqnum[direction];


	if (delta < RTP_MAX_OUT_OF_ORDER) {	/* in order, with permissible gap */
		flow->rtp_seqnum[direction] = seq;
		IPQ_LOG(IPOQUE_PROTOCOL_RTP, ipoque_struct, IPQ_LOG_DEBUG, "rtp_seqnum[%u] = %u (increased by %u)\n",
				direction, seq, delta);
		return delta;
	} else {
		IPQ_LOG(IPOQUE_PROTOCOL_RTP, ipoque_struct, IPQ_LOG_DEBUG, "retransmission (dir %u, seqnum %u)\n",
				direction, seq);
		return 0;
	}
}


static void ipoque_rtp_search(struct ipoque_detection_module_struct *ipoque_struct,
							  const u8 * payload, const u16 payload_len)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
	u8 stage;
	u16 seqnum = ntohs(get_u16(payload, 2));

	IPQ_LOG(IPOQUE_PROTOCOL_RTP, ipoque_struct, IPQ_LOG_DEBUG, "search rtp.\n");

	if (payload_len == 4 && get_u32(packet->payload, 0) == 0 && flow->packet_counter < 8) {
		IPQ_LOG(IPOQUE_PROTOCOL_RTP, ipoque_struct, IPQ_LOG_DEBUG, "need next packet, maybe ClearSea out calls.\n");
		return;
	}

	if (payload_len == 5 && memcmp(payload, "hello", 5) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_RTP, ipoque_struct, IPQ_LOG_DEBUG,
				"need next packet, initial hello packet of SIP out calls.\n");
		return;
	}

	if (payload_len == 1 && payload[0] == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_RTP, ipoque_struct, IPQ_LOG_DEBUG,
				"need next packet, payload_packet_len == 1 && payload[0] == 0.\n");
		return;
	}

	if (payload_len == 3 && memcmp(payload, "png", 3) == 0) {
		/* weird packet found in Ninja GlobalIP trace */
		IPQ_LOG(IPOQUE_PROTOCOL_RTP, ipoque_struct, IPQ_LOG_DEBUG, "skipping packet with len = 3 and png payload.\n");
		return;
	}

	if (payload_len < 12) {
		IPQ_LOG(IPOQUE_PROTOCOL_RTP, ipoque_struct, IPQ_LOG_DEBUG, "minimal packet size for rtp packets: 12.\n");
		goto exclude_rtp;
	}

	if (payload_len == 12 && get_u32(payload, 0) == 0 && get_u32(payload, 4) == 0 && get_u32(payload, 8) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_RTP, ipoque_struct, IPQ_LOG_DEBUG, "skipping packet with len = 12 and only 0-bytes.\n");
		return;
	}

	if ((payload[0] & 0xc0) == 0xc0 || (payload[0] & 0xc0) == 0x40 || (payload[0] & 0xc0) == 0x00) {
		IPQ_LOG(IPOQUE_PROTOCOL_RTP, ipoque_struct, IPQ_LOG_DEBUG, "version = 3 || 1 || 0, maybe first rtp packet.\n");
		return;
	}

	if ((payload[0] & 0xc0) != 0x80) {
		IPQ_LOG(IPOQUE_PROTOCOL_RTP, ipoque_struct,
				IPQ_LOG_DEBUG, "rtp version must be 2, first two bits of a packets must be 10.\n");
		goto exclude_rtp;
	}

	/* rtp_payload_type are the last seven bits of the second byte */
	if (flow->rtp_payload_type[packet->packet_direction] != (payload[1] & 0x7F)) {
		IPQ_LOG(IPOQUE_PROTOCOL_RTP, ipoque_struct, IPQ_LOG_DEBUG, "payload_type has changed, reset stages.\n");
		packet->packet_direction == 0 ? (flow->rtp_stage1 = 0) : (flow->rtp_stage2 = 0);
	}
	/* first bit of first byte is not part of payload_type */
	flow->rtp_payload_type[packet->packet_direction] = payload[1] & 0x7F;

	stage = (packet->packet_direction == 0 ? flow->rtp_stage1 : flow->rtp_stage2);

	if (stage > 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_RTP, ipoque_struct,
				IPQ_LOG_DEBUG, "stage = %u.\n", packet->packet_direction == 0 ? flow->rtp_stage1 : flow->rtp_stage2);
		if (flow->rtp_ssid[packet->packet_direction] != get_u32(payload, 8)) {
			IPQ_LOG(IPOQUE_PROTOCOL_RTP, ipoque_struct, IPQ_LOG_DEBUG, "ssid has changed, goto exclude rtp.\n");
			goto exclude_rtp;
		}

		if (seqnum == flow->rtp_seqnum[packet->packet_direction]) {
			IPQ_LOG(IPOQUE_PROTOCOL_RTP, ipoque_struct, IPQ_LOG_DEBUG, "maybe \"retransmission\", need next packet.\n");
			return;
		} else if ((u16) (seqnum - flow->rtp_seqnum[packet->packet_direction]) < RTP_MAX_OUT_OF_ORDER) {
			IPQ_LOG(IPOQUE_PROTOCOL_RTP, ipoque_struct, IPQ_LOG_DEBUG,
					"new packet has larger sequence number (within valid range)\n");
			update_seq(ipoque_struct, flow, packet->packet_direction, seqnum);
		} else if ((u16) (flow->rtp_seqnum[packet->packet_direction] - seqnum) < RTP_MAX_OUT_OF_ORDER) {
			IPQ_LOG(IPOQUE_PROTOCOL_RTP, ipoque_struct, IPQ_LOG_DEBUG,
					"new packet has smaller sequence number (within valid range)\n");
			init_seq(ipoque_struct, flow, packet->packet_direction, seqnum, 1);
		} else {
			IPQ_LOG(IPOQUE_PROTOCOL_RTP, ipoque_struct, IPQ_LOG_DEBUG,
					"sequence number diff is too big, goto exclude rtp.\n");
			goto exclude_rtp;
		}
	} else {
		IPQ_LOG(IPOQUE_PROTOCOL_RTP, ipoque_struct,
				IPQ_LOG_DEBUG, "rtp_ssid[%u] = %u.\n", packet->packet_direction,
				flow->rtp_ssid[packet->packet_direction]);
		flow->rtp_ssid[packet->packet_direction] = get_u32(payload, 8);
		if (flow->packet_counter < 3) {
			IPQ_LOG(IPOQUE_PROTOCOL_RTP, ipoque_struct, IPQ_LOG_DEBUG, "packet_counter < 3, need next packet.\n");
		}
		init_seq(ipoque_struct, flow, packet->packet_direction, seqnum, 1);
	}
	if (seqnum <= 3) {
		IPQ_LOG(IPOQUE_PROTOCOL_RTP, ipoque_struct,
				IPQ_LOG_DEBUG, "sequence_number = %u, too small, need next packet, return.\n", seqnum);
		return;
	}

	if (stage == 3) {
		IPQ_LOG(IPOQUE_PROTOCOL_RTP, ipoque_struct, IPQ_LOG_DEBUG, "add connection I.\n");
		ipoque_int_rtp_add_connection(ipoque_struct);
	} else {
		packet->packet_direction == 0 ? flow->rtp_stage1++ : flow->rtp_stage2++;
		IPQ_LOG(IPOQUE_PROTOCOL_RTP, ipoque_struct, IPQ_LOG_DEBUG, "stage[%u]++; need next packet.\n",
				packet->packet_direction);
	}
	return;

  exclude_rtp:
#ifdef IPOQUE_PROTOCOL_STUN
	if (packet->detected_protocol_stack[0] == IPOQUE_PROTOCOL_STUN
		|| packet->real_protocol_read_only == IPOQUE_PROTOCOL_STUN) {
		IPQ_LOG(IPOQUE_PROTOCOL_RTP, ipoque_struct, IPQ_LOG_DEBUG, "STUN: is detected, need next packet.\n");
		return;
	}
#endif							/*  IPOQUE_PROTOCOL_STUN */
	IPQ_LOG(IPOQUE_PROTOCOL_RTP, ipoque_struct, IPQ_LOG_DEBUG, "exclude rtp.\n");
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_RTP);
}


void ipoque_search_rtp(struct ipoque_detection_module_struct *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;

	if (packet->udp) {
		ipoque_rtp_search(ipoque_struct, packet->payload, packet->payload_packet_len);
	} else if (packet->tcp) {

		/* skip special packets seen at yahoo traces */
		if (packet->payload_packet_len >= 20 && ntohs(get_u16(packet->payload, 2)) + 20 == packet->payload_packet_len &&
			packet->payload[0] == 0x90 && packet->payload[1] >= 0x01 && packet->payload[1] <= 0x07) {
			if (flow->packet_counter == 2)
				flow->l4.tcp.rtp_special_packets_seen = 1;
			IPQ_LOG(IPOQUE_PROTOCOL_RTP, ipoque_struct, IPQ_LOG_DEBUG,
					"skipping STUN-like, special yahoo packets with payload[0] == 0x90.\n");
			return;
		}
#ifdef IPOQUE_PROTOCOL_STUN
		/* TODO the rtp detection sometimes doesn't exclude rtp
		 * so for TCP flows only run the detection if STUN has been
		 * detected (or RTP is already detected)
		 * If flows will be seen which start directly with RTP
		 * we can remove this restriction
		 */

		if (packet->detected_protocol_stack[0] == IPOQUE_PROTOCOL_STUN
			|| packet->detected_protocol_stack[0] == IPOQUE_PROTOCOL_RTP) {

			/* RTP may be encapsulated in TCP packets */

			if (packet->payload_packet_len >= 2 && ntohs(get_u16(packet->payload, 0)) + 2 == packet->payload_packet_len) {

				/* TODO there could be several RTP packets in a single TCP packet so maybe the detection could be
				 * improved by checking only the RTP packet of given length */

				ipoque_rtp_search(ipoque_struct, packet->payload + 2, packet->payload_packet_len - 2);

				return;
			}
		}
		if (packet->detected_protocol_stack[0] == IPOQUE_PROTOCOL_UNKNOWN && flow->l4.tcp.rtp_special_packets_seen == 1) {

			if (packet->payload_packet_len >= 4 && ntohl(get_u32(packet->payload, 0)) + 4 == packet->payload_packet_len) {

				/* TODO there could be several RTP packets in a single TCP packet so maybe the detection could be
				 * improved by checking only the RTP packet of given length */

				ipoque_rtp_search(ipoque_struct, packet->payload + 4, packet->payload_packet_len - 4);

				return;
			}
		}

		if (IPQ_FLOW_PROTOCOL_EXCLUDED(ipoque_struct, flow, IPOQUE_PROTOCOL_STUN)) {
			IPQ_LOG(IPOQUE_PROTOCOL_RTP, ipoque_struct, IPQ_LOG_DEBUG, "exclude rtp.\n");
			IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_RTP);
		} else {
			IPQ_LOG(IPOQUE_PROTOCOL_RTP, ipoque_struct, IPQ_LOG_DEBUG, "STUN not yet excluded, need next packet.\n");
		}
#else
		IPQ_LOG(IPOQUE_PROTOCOL_RTP, ipoque_struct, IPQ_LOG_DEBUG, "exclude rtp.\n");
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_RTP);
#endif
	}
}

#endif							/* IPOQUE_PROTOCOL_RTP */
