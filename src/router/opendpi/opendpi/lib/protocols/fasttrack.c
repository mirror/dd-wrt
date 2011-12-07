/*
 * fasttrack.c
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

#ifdef IPOQUE_PROTOCOL_FASTTRACK



static void ipoque_int_fasttrack_add_connection(struct ipoque_detection_module_struct
												*ipoque_struct)
{
	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_FASTTRACK, IPOQUE_CORRELATED_PROTOCOL);
}


void ipoque_search_fasttrack_tcp(struct ipoque_detection_module_struct
								 *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
//      struct ipoque_id_struct         *src=ipoque_struct->src;
//      struct ipoque_id_struct         *dst=ipoque_struct->dst;

	if (packet->payload_packet_len > 6 && ntohs(get_u16(packet->payload, packet->payload_packet_len - 2)) == 0x0d0a) {
		IPQ_LOG(IPOQUE_PROTOCOL_FASTTRACK, ipoque_struct, IPQ_LOG_TRACE, "detected 0d0a at the end of the packet.\n");

		if (memcmp(packet->payload, "GIVE ", 5) == 0 && packet->payload_packet_len >= 8) {
			u16 i;
			for (i = 5; i < (packet->payload_packet_len - 2); i++) {
				// make shure that the argument to GIVE is numeric
				if (!(packet->payload[i] >= '0' && packet->payload[i] <= '9')) {
					goto exclude_fasttrack;
				}
			}

			IPQ_LOG(IPOQUE_PROTOCOL_FASTTRACK, ipoque_struct, IPQ_LOG_TRACE, "FASTTRACK GIVE DETECTED\n");
			ipoque_int_fasttrack_add_connection(ipoque_struct);
			return;
		}

		if (packet->payload_packet_len > 50 && memcmp(packet->payload, "GET /", 5) == 0) {
			u8 a = 0;
			IPQ_LOG(IPOQUE_PROTOCOL_FASTTRACK, ipoque_struct, IPQ_LOG_TRACE, "detected GET /. \n");
			ipq_parse_packet_line_info(ipoque_struct);
			for (a = 0; a < packet->parsed_lines; a++) {
				if ((packet->line[a].len > 17 && memcmp(packet->line[a].ptr, "X-Kazaa-Username: ", 18) == 0)
					|| (packet->line[a].len > 23 && memcmp(packet->line[a].ptr, "User-Agent: PeerEnabler/", 24) == 0)) {
					IPQ_LOG(IPOQUE_PROTOCOL_FASTTRACK, ipoque_struct, IPQ_LOG_TRACE,
							"detected X-Kazaa-Username: || User-Agent: PeerEnabler/\n");
					ipoque_int_fasttrack_add_connection(ipoque_struct);
					return;
				}
			}
		}
	}

  exclude_fasttrack:
	IPQ_LOG(IPOQUE_PROTOCOL_FASTTRACK, ipoque_struct, IPQ_LOG_TRACE, "fasttrack/kazaa excluded.\n");
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_FASTTRACK);
}
#endif
