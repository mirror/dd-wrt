/*
 * steam.c
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
#ifdef IPOQUE_PROTOCOL_STEAM


static void ipoque_int_steam_add_connection(struct ipoque_detection_module_struct
											*ipoque_struct)
{
	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_STEAM, IPOQUE_REAL_PROTOCOL);
}

void ipoque_search_steam(struct ipoque_detection_module_struct *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
//      struct ipoque_id_struct         *src=ipoque_struct->src;
//      struct ipoque_id_struct         *dst=ipoque_struct->dst;

	if (flow->l4.tcp.steam_stage == 0) {
		if (packet->payload_packet_len == 4
			&& ntohl(get_u32(packet->payload, 0)) <= 0x07
			&& ntohs(packet->tcp->dest) >= 27030 && ntohs(packet->tcp->dest) <= 27040) {
			flow->l4.tcp.steam_stage = 1 + packet->packet_direction;
			IPQ_LOG(IPOQUE_PROTOCOL_STEAM, ipoque_struct, IPQ_LOG_DEBUG, "steam stage 1\n");
			return;
		}

	} else if (flow->l4.tcp.steam_stage == 2 - packet->packet_direction) {
		if ((packet->payload_packet_len == 1 || packet->payload_packet_len == 5)
			&& packet->payload[0] == 0x01) {
			ipoque_int_steam_add_connection(ipoque_struct);
			IPQ_LOG(IPOQUE_PROTOCOL_STEAM, ipoque_struct, IPQ_LOG_DEBUG, "steam detected\n");
			return;
		}
	}

	IPQ_LOG(IPOQUE_PROTOCOL_STEAM, ipoque_struct, IPQ_LOG_DEBUG, "steam excluded.\n");
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_STEAM);
}

#endif
