/*
 * fasttrack.c
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

#ifdef IPOQUE_PROTOCOL_FASTTRACK

static void ipoque_search_fasttrack_tcp(struct ipoque_detection_module_struct
								 *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
	const u8 *p, *end, *line;
	int len;

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
			ipq_connection_detected(ipoque_struct, IPOQUE_PROTOCOL_FASTTRACK);
			return;
		}

		if (packet->payload_packet_len > 50 && memcmp(packet->payload, "GET /", 5) == 0) {
			IPQ_LOG(IPOQUE_PROTOCOL_FASTTRACK, ipoque_struct, IPQ_LOG_TRACE, "detected GET /. \n");
			for (p = packet->payload, end = p + packet->payload_packet_len;
			     get_next_line(&p, end, &line, &len);) {
				if ((len > 17 && memcmp(line, "X-Kazaa-Username: ", 18) == 0)
					|| (len > 23 && memcmp(line, "User-Agent: PeerEnabler/", 24) == 0)) {
					IPQ_LOG(IPOQUE_PROTOCOL_FASTTRACK, ipoque_struct, IPQ_LOG_TRACE,
							"detected X-Kazaa-Username: || User-Agent: PeerEnabler/\n");
					ipq_connection_detected(ipoque_struct, IPOQUE_PROTOCOL_FASTTRACK);
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
