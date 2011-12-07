/*
 * crossfire.c
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
#ifdef IPOQUE_PROTOCOL_CROSSFIRE


static void ipoque_int_crossfire_add_connection(struct ipoque_detection_module_struct
												*ipoque_struct, ipoque_protocol_type_t protocol_type)
{

	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_CROSSFIRE, protocol_type);
}

void ipoque_search_crossfire_tcp_udp(struct ipoque_detection_module_struct
									 *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
//      struct ipoque_id_struct         *src=ipoque_struct->src;
//      struct ipoque_id_struct         *dst=ipoque_struct->dst;

	IPQ_LOG(IPOQUE_PROTOCOL_CROSSFIRE, ipoque_struct, IPQ_LOG_DEBUG, "search crossfire.\n");


	if (packet->udp != 0) {
		if (packet->payload_packet_len == 25 && get_u32(packet->payload, 0) == ntohl(0xc7d91999)
			&& get_u16(packet->payload, 4) == ntohs(0x0200)
			&& get_u16(packet->payload, 22) == ntohs(0x7d00)
			) {
			IPQ_LOG(IPOQUE_PROTOCOL_CROSSFIRE, ipoque_struct, IPQ_LOG_DEBUG, "Crossfire: found udp packet.\n");
			ipoque_int_crossfire_add_connection(ipoque_struct, IPOQUE_REAL_PROTOCOL);
			return;
		}

	} else if (packet->tcp != 0) {

		if (packet->payload_packet_len > 4 && memcmp(packet->payload, "GET /", 5) == 0) {
			ipq_parse_packet_line_info(ipoque_struct);
			if (packet->parsed_lines == 8
				&& (packet->line[0].ptr != NULL && packet->line[0].len >= 30
					&& (memcmp(&packet->payload[5], "notice/login_big", 16) == 0
						|| memcmp(&packet->payload[5], "notice/login_small", 18) == 0))
				&& memcmp(&packet->payload[packet->line[0].len - 19], "/index.asp HTTP/1.", 18) == 0
				&& (packet->host_line.ptr != NULL && packet->host_line.len >= 13
					&& (memcmp(packet->host_line.ptr, "crossfire", 9) == 0
						|| memcmp(packet->host_line.ptr, "www.crossfire", 13) == 0))
				) {
				IPQ_LOG(IPOQUE_PROTOCOL_CROSSFIRE, ipoque_struct, IPQ_LOG_DEBUG, "Crossfire: found HTTP request.\n");
				ipoque_int_crossfire_add_connection(ipoque_struct, IPOQUE_CORRELATED_PROTOCOL);
				return;
			}
		}

	}

	IPQ_LOG(IPOQUE_PROTOCOL_CROSSFIRE, ipoque_struct, IPQ_LOG_DEBUG, "exclude crossfire.\n");
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_CROSSFIRE);
}



#endif
