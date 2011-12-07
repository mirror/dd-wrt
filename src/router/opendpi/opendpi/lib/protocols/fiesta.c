/*
 * fiesta.c
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



/* include files */
#include "ipq_protocols.h"
#ifdef IPOQUE_PROTOCOL_FIESTA


static void ipoque_int_fiesta_add_connection(struct ipoque_detection_module_struct
											 *ipoque_struct)
{
	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_FIESTA, IPOQUE_REAL_PROTOCOL);
}

void ipoque_search_fiesta(struct ipoque_detection_module_struct
						  *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
//      struct ipoque_id_struct         *src=ipoque_struct->src;
//      struct ipoque_id_struct         *dst=ipoque_struct->dst;

	IPQ_LOG(IPOQUE_PROTOCOL_FIESTA, ipoque_struct, IPQ_LOG_DEBUG, "search fiesta.\n");

	if (flow->l4.tcp.fiesta_stage == 0 && packet->payload_packet_len == 5
		&& get_u16(packet->payload, 0) == ntohs(0x0407)
		&& (packet->payload[2] == 0x08)
		&& (packet->payload[4] == 0x00 || packet->payload[4] == 0x01)) {

		IPQ_LOG(IPOQUE_PROTOCOL_FIESTA, ipoque_struct, IPQ_LOG_DEBUG, "maybe fiesta symmetric, first packet.\n");
		flow->l4.tcp.fiesta_stage = 1 + packet->packet_direction;
		goto maybe_fiesta;
	}
	if (flow->l4.tcp.fiesta_stage == (2 - packet->packet_direction)
		&& ((packet->payload_packet_len > 1 && packet->payload_packet_len - 1 == packet->payload[0])
			|| (packet->payload_packet_len > 3 && packet->payload[0] == 0
				&& get_l16(packet->payload, 1) == packet->payload_packet_len - 3))) {
		IPQ_LOG(IPOQUE_PROTOCOL_FIESTA, ipoque_struct, IPQ_LOG_DEBUG, "Maybe fiesta.\n");
		goto maybe_fiesta;
	}
	if (flow->l4.tcp.fiesta_stage == (1 + packet->packet_direction)) {
		if (packet->payload_packet_len == 4 && get_u32(packet->payload, 0) == htonl(0x03050c01)) {
			goto add_fiesta;
		}
		if (packet->payload_packet_len == 5 && get_u32(packet->payload, 0) == htonl(0x04030c01)
			&& packet->payload[4] == 0) {
			goto add_fiesta;
		}
		if (packet->payload_packet_len == 6 && get_u32(packet->payload, 0) == htonl(0x050e080b)) {
			goto add_fiesta;
		}
		if (packet->payload_packet_len == 100 && packet->payload[0] == 0x63 && packet->payload[61] == 0x52
			&& packet->payload[81] == 0x5a && get_u16(packet->payload, 1) == htons(0x3810)
			&& get_u16(packet->payload, 62) == htons(0x6f75)) {
			goto add_fiesta;
		}
		if (packet->payload_packet_len > 3 && packet->payload_packet_len - 1 == packet->payload[0]
			&& get_u16(packet->payload, 1) == htons(0x140c)) {
			goto add_fiesta;
		}
	}

	IPQ_LOG(IPOQUE_PROTOCOL_FIESTA, ipoque_struct, IPQ_LOG_DEBUG, "exclude fiesta.\n");
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_FIESTA);
	return;

  maybe_fiesta:
	IPQ_LOG(IPOQUE_PROTOCOL_FIESTA, ipoque_struct, IPQ_LOG_DEBUG, "Stage is set to %d.\n", flow->l4.tcp.fiesta_stage);
	return;

  add_fiesta:
	IPQ_LOG(IPOQUE_PROTOCOL_FIESTA, ipoque_struct, IPQ_LOG_DEBUG, "detected fiesta.\n");
	ipoque_int_fiesta_add_connection(ipoque_struct);
	return;
}
#endif
