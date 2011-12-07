/*
 * florensia.c
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
#ifdef IPOQUE_PROTOCOL_FLORENSIA


static void ipoque_florensia_add_connection(struct ipoque_detection_module_struct
											*ipoque_struct)
{
	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_FLORENSIA, IPOQUE_REAL_PROTOCOL);
}

void ipoque_search_florensia(struct ipoque_detection_module_struct
							 *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
//      struct ipoque_id_struct         *src=ipoque_struct->src;
//      struct ipoque_id_struct         *dst=ipoque_struct->dst;


	IPQ_LOG(IPOQUE_PROTOCOL_FLORENSIA, ipoque_struct, IPQ_LOG_DEBUG, "search florensia.\n");

	if (packet->tcp != NULL) {
		if (packet->payload_packet_len == 5 && get_l16(packet->payload, 0) == packet->payload_packet_len
			&& packet->payload[2] == 0x65 && packet->payload[4] == 0xff) {
			if (flow->florensia_stage == 1) {
				IPQ_LOG(IPOQUE_PROTOCOL_FLORENSIA, ipoque_struct, IPQ_LOG_DEBUG, "found florensia.\n");
				ipoque_florensia_add_connection(ipoque_struct);
				return;
			}
			IPQ_LOG(IPOQUE_PROTOCOL_FLORENSIA, ipoque_struct, IPQ_LOG_DEBUG, "maybe florensia -> stage is set to 1.\n");
			flow->florensia_stage = 1;
			return;
		}
		if (packet->payload_packet_len > 8 && get_l16(packet->payload, 0) == packet->payload_packet_len
			&& get_u16(packet->payload, 2) == htons(0x0201) && get_u32(packet->payload, 4) == htonl(0xFFFFFFFF)) {
			IPQ_LOG(IPOQUE_PROTOCOL_FLORENSIA, ipoque_struct, IPQ_LOG_DEBUG, "maybe florensia -> stage is set to 1.\n");
			flow->florensia_stage = 1;
			return;
		}
		if (packet->payload_packet_len == 406 && get_l16(packet->payload, 0) == packet->payload_packet_len
			&& packet->payload[2] == 0x63) {
			IPQ_LOG(IPOQUE_PROTOCOL_FLORENSIA, ipoque_struct, IPQ_LOG_DEBUG, "maybe florensia -> stage is set to 1.\n");
			flow->florensia_stage = 1;
			return;
		}
		if (packet->payload_packet_len == 12 && get_l16(packet->payload, 0) == packet->payload_packet_len
			&& get_u16(packet->payload, 2) == htons(0x0301)) {
			if (flow->florensia_stage == 1) {
				IPQ_LOG(IPOQUE_PROTOCOL_FLORENSIA, ipoque_struct, IPQ_LOG_DEBUG, "found florensia.\n");
				ipoque_florensia_add_connection(ipoque_struct);
				return;
			}
			IPQ_LOG(IPOQUE_PROTOCOL_FLORENSIA, ipoque_struct, IPQ_LOG_DEBUG, "maybe florensia -> stage is set to 1.\n");
			flow->florensia_stage = 1;
			return;
		}

		if (flow->florensia_stage == 1) {
			if (packet->payload_packet_len == 8 && get_l16(packet->payload, 0) == packet->payload_packet_len
				&& get_u16(packet->payload, 2) == htons(0x0302) && get_u32(packet->payload, 4) == htonl(0xFFFFFFFF)) {
				IPQ_LOG(IPOQUE_PROTOCOL_FLORENSIA, ipoque_struct, IPQ_LOG_DEBUG, "found florensia asymmetrically.\n");
				ipoque_florensia_add_connection(ipoque_struct);
				return;
			}
			if (packet->payload_packet_len == 24 && get_l16(packet->payload, 0) == packet->payload_packet_len
				&& get_u16(packet->payload, 2) == htons(0x0202)
				&& get_u32(packet->payload, packet->payload_packet_len - 4) == htonl(0xFFFFFFFF)) {
				IPQ_LOG(IPOQUE_PROTOCOL_FLORENSIA, ipoque_struct, IPQ_LOG_DEBUG, "found florensia.\n");
				ipoque_florensia_add_connection(ipoque_struct);
				return;
			}
			if (flow->packet_counter < 10 && get_l16(packet->payload, 0) == packet->payload_packet_len) {
				IPQ_LOG(IPOQUE_PROTOCOL_FLORENSIA, ipoque_struct, IPQ_LOG_DEBUG, "maybe florensia.\n");
				return;
			}
		}
	}

	if (packet->udp != NULL) {
		if (flow->florensia_stage == 0 && packet->payload_packet_len == 6
			&& get_u16(packet->payload, 0) == ntohs(0x0503) && get_u32(packet->payload, 2) == htonl(0xFFFF0000)) {
			IPQ_LOG(IPOQUE_PROTOCOL_FLORENSIA, ipoque_struct, IPQ_LOG_DEBUG, "maybe florensia -> stage is set to 1.\n");
			flow->florensia_stage = 1;
			return;
		}
		if (flow->florensia_stage == 1 && packet->payload_packet_len == 8
			&& get_u16(packet->payload, 0) == ntohs(0x0500) && get_u16(packet->payload, 4) == htons(0x4191)) {
			IPQ_LOG(IPOQUE_PROTOCOL_FLORENSIA, ipoque_struct, IPQ_LOG_DEBUG, "found florensia.\n");
			ipoque_florensia_add_connection(ipoque_struct);
			return;
		}
	}

	IPQ_LOG(IPOQUE_PROTOCOL_FLORENSIA, ipoque_struct, IPQ_LOG_DEBUG, "exclude florensia.\n");
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_FLORENSIA);
}

#endif
