/*
 * feidian.c
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

#ifdef IPOQUE_PROTOCOL_FEIDIAN

static void ipoque_int_feidian_add_connection(struct ipoque_detection_module_struct
											  *ipoque_struct, ipoque_protocol_type_t protocol_type)
{
	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_FEIDIAN, protocol_type);
}


void ipoque_search_feidian(struct ipoque_detection_module_struct *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;

//      struct ipoque_id_struct         *src=ipoque_struct->src;
//      struct ipoque_id_struct         *dst=ipoque_struct->dst;


	if (packet->tcp != NULL) {
		if (packet->tcp->dest == htons(8080) && packet->payload_packet_len == 4
			&& packet->payload[0] == 0x29 && packet->payload[1] == 0x1c
			&& packet->payload[2] == 0x32 && packet->payload[3] == 0x01) {
			IPQ_LOG(IPOQUE_PROTOCOL_FEIDIAN, ipoque_struct, IPQ_LOG_DEBUG,
					"Feidian: found the flow (TCP): packet_size: %u; Flowstage: %u\n",
					packet->payload_packet_len, flow->l4.udp.feidian_stage);
			ipoque_int_feidian_add_connection(ipoque_struct, IPOQUE_REAL_PROTOCOL);
			return;
		} else if (packet->payload_packet_len > 50 && memcmp(packet->payload, "GET /", 5) == 0) {
			ipq_parse_packet_line_info(ipoque_struct);
			if (packet->host_line.ptr != NULL && packet->host_line.len == 18
				&& memcmp(packet->host_line.ptr, "config.feidian.com", 18) == 0) {
				ipoque_int_feidian_add_connection(ipoque_struct, IPOQUE_CORRELATED_PROTOCOL);
				return;
			}
		}
		IPQ_LOG(IPOQUE_PROTOCOL_FEIDIAN, ipoque_struct, IPQ_LOG_DEBUG,
				"Feidian: discarted the flow (TCP): packet_size: %u; Flowstage: %u\n",
				packet->payload_packet_len, flow->l4.udp.feidian_stage);
	} else if (packet->udp != NULL) {
		if (ntohs(packet->udp->source) == 53124 || ntohs(packet->udp->dest) == 53124) {
			if (flow->l4.udp.feidian_stage == 0 && (packet->payload_packet_len == 112)
				&& packet->payload[0] == 0x1c && packet->payload[1] == 0x1c
				&& packet->payload[2] == 0x32 && packet->payload[3] == 0x01) {
				flow->l4.udp.feidian_stage = 1;
				return;
			} else if (flow->l4.udp.feidian_stage == 1
					   && (packet->payload_packet_len == 116 || packet->payload_packet_len == 112)
					   && packet->payload[0] == 0x1c
					   && packet->payload[1] == 0x1c && packet->payload[2] == 0x32 && packet->payload[3] == 0x01) {
				ipoque_int_feidian_add_connection(ipoque_struct, IPOQUE_REAL_PROTOCOL);
				return;
			}
		}
		IPQ_LOG(IPOQUE_PROTOCOL_FEIDIAN, ipoque_struct, IPQ_LOG_DEBUG,
				"Feidian: discarted the flow (UDP): packet_size: %u; Flowstage: %u\n",
				packet->payload_packet_len, flow->l4.udp.feidian_stage);
	}
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_FEIDIAN);
}
#endif
