/*
 * battlefield.c
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


#include "ipq_protocols.h"
#ifdef IPOQUE_PROTOCOL_BATTLEFIELD


static void ipoque_int_battlefield_add_connection(struct ipoque_detection_module_struct
												  *ipoque_struct)
{

	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_id_struct *src = ipoque_struct->src;
	struct ipoque_id_struct *dst = ipoque_struct->dst;

	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_BATTLEFIELD, IPOQUE_REAL_PROTOCOL);

	if (src != NULL) {
		src->battlefield_ts = packet->tick_timestamp;
	}
	if (dst != NULL) {
		dst->battlefield_ts = packet->tick_timestamp;
	}
}

void ipoque_search_battlefield(struct ipoque_detection_module_struct
							   *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
	struct ipoque_id_struct *src = ipoque_struct->src;
	struct ipoque_id_struct *dst = ipoque_struct->dst;

	if (packet->detected_protocol_stack[0] == IPOQUE_PROTOCOL_BATTLEFIELD) {
		if (src != NULL && ((IPOQUE_TIMESTAMP_COUNTER_SIZE)
							(packet->tick_timestamp - src->battlefield_ts) < ipoque_struct->battlefield_timeout)) {
			IPQ_LOG(IPOQUE_PROTOCOL_BATTLEFIELD, ipoque_struct, IPQ_LOG_DEBUG,
					"battlefield : save src connection packet detected\n");
			src->battlefield_ts = packet->tick_timestamp;
		} else if (dst != NULL && ((IPOQUE_TIMESTAMP_COUNTER_SIZE)
								   (packet->tick_timestamp - dst->battlefield_ts) < ipoque_struct->battlefield_timeout)) {
			IPQ_LOG(IPOQUE_PROTOCOL_BATTLEFIELD, ipoque_struct, IPQ_LOG_DEBUG,
					"battlefield : save dst connection packet detected\n");
			dst->battlefield_ts = packet->tick_timestamp;
		}
		return;
	}

	if (IPQ_SRC_OR_DST_HAS_PROTOCOL(src, dst, IPOQUE_PROTOCOL_BATTLEFIELD)) {
		if (flow->l4.udp.battlefield_stage == 0 || flow->l4.udp.battlefield_stage == 1 + packet->packet_direction) {
			if (packet->payload_packet_len > 8 && get_u16(packet->payload, 0) == htons(0xfefd)) {
				flow->l4.udp.battlefield_msg_id = get_u32(packet->payload, 2);
				flow->l4.udp.battlefield_stage = 1 + packet->packet_direction;
				return;
			}
		} else if (flow->l4.udp.battlefield_stage == 2 - packet->packet_direction) {
			if (packet->payload_packet_len > 8 && get_u32(packet->payload, 0) == flow->l4.udp.battlefield_msg_id) {
				IPQ_LOG(IPOQUE_PROTOCOL_BATTLEFIELD, ipoque_struct,
						IPQ_LOG_DEBUG, "Battlefield message and reply detected.\n");
				ipoque_int_battlefield_add_connection(ipoque_struct);
				return;
			}
		}
	}

	if (flow->l4.udp.battlefield_stage == 0) {
		if (packet->payload_packet_len == 46 && packet->payload[2] == 0 && packet->payload[4] == 0
			&& get_u32(packet->payload, 7) == htonl(0x98001100)) {
			flow->l4.udp.battlefield_stage = 3 + packet->packet_direction;
			return;
		}
	} else if (flow->l4.udp.battlefield_stage == 4 - packet->packet_direction) {
		if (packet->payload_packet_len == 7
			&& (packet->payload[0] == 0x02 || packet->payload[packet->payload_packet_len - 1] == 0xe0)) {
			IPQ_LOG(IPOQUE_PROTOCOL_BATTLEFIELD, ipoque_struct, IPQ_LOG_DEBUG,
					"Battlefield message and reply detected.\n");
			ipoque_int_battlefield_add_connection(ipoque_struct);
			return;
		}
	}

	if (packet->payload_packet_len == 18 && ipq_mem_cmp(&packet->payload[5], "battlefield2\x00", 13) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_BATTLEFIELD, ipoque_struct, IPQ_LOG_DEBUG, "Battlefield 2 hello packet detected.\n");
		ipoque_int_battlefield_add_connection(ipoque_struct);
		return;
	} else if (packet->payload_packet_len > 10 &&
			   (ipq_mem_cmp(packet->payload, "\x11\x20\x00\x01\x00\x00\x50\xb9\x10\x11", 10) == 0
				|| ipq_mem_cmp(packet->payload, "\x11\x20\x00\x01\x00\x00\x30\xb9\x10\x11", 10) == 0
				|| ipq_mem_cmp(packet->payload, "\x11\x20\x00\x01\x00\x00\xa0\x98\x00\x11", 10) == 0)) {
		IPQ_LOG(IPOQUE_PROTOCOL_BATTLEFIELD, ipoque_struct, IPQ_LOG_DEBUG, "Battlefield safe pattern detected.\n");
		ipoque_int_battlefield_add_connection(ipoque_struct);
		return;
	}

	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_BATTLEFIELD);
	return;
}

#endif
