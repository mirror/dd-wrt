/*
 * filetopia.c
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
#ifdef IPOQUE_PROTOCOL_FILETOPIA


static void ipoque_int_filetopia_add_connection(struct ipoque_detection_module_struct
												*ipoque_struct)
{
	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_FILETOPIA, IPOQUE_REAL_PROTOCOL);
}

void ipoque_search_filetopia_tcp(struct ipoque_detection_module_struct
								 *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
//      struct ipoque_id_struct         *src=ipoque_struct->src;
//      struct ipoque_id_struct         *dst=ipoque_struct->dst;

	if (flow->l4.tcp.filetopia_stage == 0) {
		if (packet->payload_packet_len >= 50 && packet->payload_packet_len <= 70
			&& packet->payload[0] == 0x03 && packet->payload[1] == 0x9a
			&& packet->payload[3] == 0x22 && packet->payload[packet->payload_packet_len - 1] == 0x2b) {
			IPQ_LOG(IPOQUE_PROTOCOL_FILETOPIA, ipoque_struct, IPQ_LOG_DEBUG, "Filetopia stage 1 detected\n");
			flow->l4.tcp.filetopia_stage = 1;
			return;
		}

	} else if (flow->l4.tcp.filetopia_stage == 1) {
		if (packet->payload_packet_len >= 100 && packet->payload[0] == 0x03
			&& packet->payload[1] == 0x9a && (packet->payload[3] == 0x22 || packet->payload[3] == 0x23)) {

			int i;
			for (i = 0; i < 10; i++) {	// check 10 bytes for valid ASCII printable characters
				if (!(packet->payload[5 + i] >= 0x20 && packet->payload[5 + i] <= 0x7e)) {
					goto end_filetopia_nothing_found;
				}
			}

			IPQ_LOG(IPOQUE_PROTOCOL_FILETOPIA, ipoque_struct, IPQ_LOG_DEBUG, "Filetopia stage 2 detected\n");
			flow->l4.tcp.filetopia_stage = 2;
			return;
		}


	} else if (flow->l4.tcp.filetopia_stage == 2) {
		if (packet->payload_packet_len >= 4 && packet->payload_packet_len <= 100
			&& packet->payload[0] == 0x03 && packet->payload[1] == 0x9a
			&& (packet->payload[3] == 0x22 || packet->payload[3] == 0x23)) {
			IPQ_LOG(IPOQUE_PROTOCOL_FILETOPIA, ipoque_struct, IPQ_LOG_DEBUG, "Filetopia detected\n");
			ipoque_int_filetopia_add_connection(ipoque_struct);
			return;
		}

	}

  end_filetopia_nothing_found:
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_FILETOPIA);
}

#endif
