/*
 * battlefield.c
 * Copyright (C) 2009-2010 by ipoque GmbH
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

static void ipoque_search_battlefield(struct ipoque_detection_module_struct
							   *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
	struct ipoque_id_struct *src;
	struct ipoque_id_struct *dst;
	ipq_lookup_flow_addr(ipoque_struct, IPOQUE_PROTOCOL_BATTLEFIELD, &src, &dst);

	if ((ntohs(packet->udp->source) >= 27000 || ntohs(packet->udp->dest) >= 27000)
		&& IPQ_SRC_OR_DST_HAS_PROTOCOL(src, dst, IPOQUE_PROTOCOL_BATTLEFIELD)) {
		if (flow->battlefield_stage == 0 || flow->battlefield_stage == 1 + packet->packet_direction) {
			if (packet->payload_packet_len > 8 && get_u16(packet->payload, 0) == htons(0xfefd)) {
				flow->battlefield_msg_id = get_u32(packet->payload, 2);
				flow->battlefield_stage = 1 + packet->packet_direction;
				return;
			}
		} else if (flow->battlefield_stage == 2 - packet->packet_direction) {
			if (packet->payload_packet_len > 8 && get_u32(packet->payload, 0) == flow->battlefield_msg_id) {
				IPQ_LOG(IPOQUE_PROTOCOL_BATTLEFIELD, ipoque_struct,
						IPQ_LOG_DEBUG, "Battlefield message and reply detected.\n");
				ipq_connection_detected(ipoque_struct, IPOQUE_PROTOCOL_BATTLEFIELD);
				return;
			}
		}
	}

	if (packet->payload_packet_len == 18 && ipq_mem_cmp(&packet->payload[5], "battlefield2\x00", 13) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_BATTLEFIELD, ipoque_struct, IPQ_LOG_DEBUG, "Battlefield 2 hello packet detected.\n");
		ipq_connection_detected(ipoque_struct, IPOQUE_PROTOCOL_BATTLEFIELD);
		return;
	} else if (packet->payload_packet_len > 10
			   &&
			   (ipq_mem_cmp
				(packet->payload, "\x11\x20\x00\x01\x00\x00\x50\xb9\x10\x11",
				 10) == 0
				|| ipq_mem_cmp(packet->payload,
							   "\x11\x20\x00\x01\x00\x00\x30\xb9\x10\x11",
							   10) == 0
				|| ipq_mem_cmp(packet->payload, "\x11\x20\x00\x01\x00\x00\xa0\x98\x00\x11", 10) == 0)) {
		IPQ_LOG(IPOQUE_PROTOCOL_BATTLEFIELD, ipoque_struct, IPQ_LOG_DEBUG, "Battlefield safe pattern detected.\n");
		ipq_connection_detected(ipoque_struct, IPOQUE_PROTOCOL_BATTLEFIELD);
		return;
	}
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_BATTLEFIELD);
	return;
}

#endif
