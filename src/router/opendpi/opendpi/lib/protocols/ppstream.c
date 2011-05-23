/*
 * ppstream.c
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
#ifdef IPOQUE_PROTOCOL_PPSTREAM

static void ipoque_search_ppstream_tcp(struct ipoque_detection_module_struct
								*ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;

	/* check TCP Connections -> Videodata */
	if (packet->tcp != NULL) {
		if (packet->payload_packet_len >= 60 && get_u32(packet->payload, 52) == 0
			&& memcmp(packet->payload, "PSProtocol\x0", 11) == 0) {
			IPQ_LOG(IPOQUE_PROTOCOL_PPSTREAM, ipoque_struct, IPQ_LOG_DEBUG, "found ppstream over tcp.\n");
			ipq_connection_detected(ipoque_struct, IPOQUE_PROTOCOL_PPSTREAM);
			return;
		}
	}

	if (packet->udp != NULL) {
		if (packet->payload_packet_len > 2 && packet->payload[2] == 0x43
			&& ((packet->payload_packet_len - 4 == get_l16(packet->payload, 0))
				|| (packet->payload_packet_len == get_l16(packet->payload, 0))
				|| (packet->payload_packet_len >= 6 && packet->payload_packet_len - 6 == get_l16(packet->payload, 0)))) {
			flow->ppstream_stage++;
			if (flow->ppstream_stage == 5) {
				IPQ_LOG(IPOQUE_PROTOCOL_PPSTREAM, ipoque_struct, IPQ_LOG_DEBUG,
						"found ppstream over udp pattern len, 43.\n");
				ipq_connection_detected(ipoque_struct, IPOQUE_PROTOCOL_PPSTREAM);
				return;
			}
			return;
		}

		if (flow->ppstream_stage == 0
			&& packet->payload_packet_len > 4 && ((packet->payload_packet_len - 4 == get_l16(packet->payload, 0))
												  || (packet->payload_packet_len == get_l16(packet->payload, 0))
												  || (packet->payload_packet_len >= 6
													  && packet->payload_packet_len - 6 == get_l16(packet->payload,
																								   0)))) {

			if (packet->payload[2] == 0x00 && packet->payload[3] == 0x00 && packet->payload[4] == 0x03) {
				flow->ppstream_stage = 7;
				IPQ_LOG(IPOQUE_PROTOCOL_PPSTREAM, ipoque_struct, IPQ_LOG_DEBUG, "need next packet I.\n");
				return;
			}
		}

		if (flow->ppstream_stage == 7
			&& packet->payload_packet_len > 4 && packet->payload[3] == 0x00
			&& ((packet->payload_packet_len - 4 == get_l16(packet->payload, 0))
				|| (packet->payload_packet_len == get_l16(packet->payload, 0))
				|| (packet->payload_packet_len >= 6 && packet->payload_packet_len - 6 == get_l16(packet->payload, 0)))
			&& (packet->payload[2] == 0x00 && packet->payload[4] == 0x03)) {
			IPQ_LOG(IPOQUE_PROTOCOL_PPSTREAM, ipoque_struct, IPQ_LOG_DEBUG,
					"found ppstream over udp with pattern Vb.\n");
			ipq_connection_detected(ipoque_struct, IPOQUE_PROTOCOL_PPSTREAM);
			return;
		}




	}

	IPQ_LOG(IPOQUE_PROTOCOL_PPSTREAM, ipoque_struct, IPQ_LOG_DEBUG, "exclude ppstream.\n");
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_PPSTREAM);
}
#endif
