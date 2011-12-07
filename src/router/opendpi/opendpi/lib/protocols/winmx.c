/*
 * winmx.c
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

#ifdef IPOQUE_PROTOCOL_WINMX


static void ipoque_int_winmx_add_connection(struct ipoque_detection_module_struct
											*ipoque_struct);

static void ipoque_int_winmx_add_connection(struct ipoque_detection_module_struct
											*ipoque_struct)
{
	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_WINMX, IPOQUE_REAL_PROTOCOL);
}


void ipoque_search_winmx_tcp(struct ipoque_detection_module_struct *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
//      struct ipoque_id_struct         *src=ipoque_struct->src;
//      struct ipoque_id_struct         *dst=ipoque_struct->dst;


	if (flow->l4.tcp.winmx_stage == 0) {
		if (packet->payload_packet_len == 1 || (packet->payload_packet_len > 1 && packet->payload[0] == 0x31)) {
			return;
		}
		/* did not see this pattern in any trace that we have */
		if (((packet->payload_packet_len) == 4)
			&& (memcmp(packet->payload, "SEND", 4) == 0)) {

			IPQ_LOG(IPOQUE_PROTOCOL_WINMX, ipoque_struct, IPQ_LOG_DEBUG, "maybe WinMX Send\n");
			flow->l4.tcp.winmx_stage = 1;
			return;
		}

		if (((packet->payload_packet_len) == 3)
			&& (memcmp(packet->payload, "GET", 3) == 0)) {
			IPQ_LOG(IPOQUE_PROTOCOL_WINMX, ipoque_struct, IPQ_LOG_DEBUG, "found winmx by GET\n");
			ipoque_int_winmx_add_connection(ipoque_struct);
			return;
		}


		if (packet->payload_packet_len == 149 && packet->payload[0] == '8') {
			IPQ_LOG(IPOQUE_PROTOCOL_WINMX, ipoque_struct, IPQ_LOG_DEBUG, "maybe WinMX\n");
			if (get_u32(packet->payload, 17) == 0
				&& get_u32(packet->payload, 21) == 0
				&& get_u32(packet->payload, 25) == 0
				&& get_u16(packet->payload, 39) == 0 && get_u16(packet->payload, 135) == htons(0x7edf)
				&& get_u16(packet->payload, 147) == htons(0xf792)) {

				IPQ_LOG(IPOQUE_PROTOCOL_WINMX, ipoque_struct, IPQ_LOG_DEBUG,
						"found winmx by pattern in first packet\n");
				ipoque_int_winmx_add_connection(ipoque_struct);
				return;
			}
		}
		/* did not see this pattern in any trace that we have */
	} else if (flow->l4.tcp.winmx_stage == 1) {
		if (packet->payload_packet_len > 10 && packet->payload_packet_len < 1000) {
			u16 left = packet->payload_packet_len - 1;
			while (left > 0) {
				if (packet->payload[left] == ' ') {
					IPQ_LOG(IPOQUE_PROTOCOL_WINMX, ipoque_struct, IPQ_LOG_DEBUG, "found winmx in second packet\n");
					ipoque_int_winmx_add_connection(ipoque_struct);
					return;
				} else if (packet->payload[left] < '0' || packet->payload[left] > '9') {
					break;
				}
				left--;
			}
		}
	}

	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_WINMX);
}

#endif
