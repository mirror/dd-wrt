/*
 * ppstream.c
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
#ifdef IPOQUE_PROTOCOL_PPSTREAM

static void ipoque_int_ppstream_add_connection(struct ipoque_detection_module_struct
											   *ipoque_struct)
{
	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_PPSTREAM, IPOQUE_REAL_PROTOCOL);
}

void ipoque_search_ppstream(struct ipoque_detection_module_struct
							*ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;

	// struct ipoque_id_struct *src=ipoque_struct->src;
	// struct ipoque_id_struct *dst=ipoque_struct->dst;



	/* check TCP Connections -> Videodata */
	if (packet->tcp != NULL) {
		if (packet->payload_packet_len >= 60 && get_u32(packet->payload, 52) == 0
			&& memcmp(packet->payload, "PSProtocol\x0", 11) == 0) {
			IPQ_LOG(IPOQUE_PROTOCOL_PPSTREAM, ipoque_struct, IPQ_LOG_DEBUG, "found ppstream over tcp.\n");
			ipoque_int_ppstream_add_connection(ipoque_struct);
			return;
		}
	}

	if (packet->udp != NULL) {
		if (packet->payload_packet_len > 2 && packet->payload[2] == 0x43
			&& ((packet->payload_packet_len - 4 == get_l16(packet->payload, 0))
				|| (packet->payload_packet_len == get_l16(packet->payload, 0))
				|| (packet->payload_packet_len >= 6 && packet->payload_packet_len - 6 == get_l16(packet->payload, 0)))) {
			flow->l4.udp.ppstream_stage++;
			if (flow->l4.udp.ppstream_stage == 5) {
				IPQ_LOG(IPOQUE_PROTOCOL_PPSTREAM, ipoque_struct, IPQ_LOG_DEBUG,
						"found ppstream over udp pattern len, 43.\n");
				ipoque_int_ppstream_add_connection(ipoque_struct);
				return;
			}
			return;
		}

		if (flow->l4.udp.ppstream_stage == 0
			&& packet->payload_packet_len > 4 && ((packet->payload_packet_len - 4 == get_l16(packet->payload, 0))
												  || (packet->payload_packet_len == get_l16(packet->payload, 0))
												  || (packet->payload_packet_len >= 6
													  && packet->payload_packet_len - 6 == get_l16(packet->payload,
																								   0)))) {

			if (packet->payload[2] == 0x00 && packet->payload[3] == 0x00 && packet->payload[4] == 0x03) {
				flow->l4.udp.ppstream_stage = 7;
				IPQ_LOG(IPOQUE_PROTOCOL_PPSTREAM, ipoque_struct, IPQ_LOG_DEBUG, "need next packet I.\n");
				return;
			}
		}

		if (flow->l4.udp.ppstream_stage == 7
			&& packet->payload_packet_len > 4 && packet->payload[3] == 0x00
			&& ((packet->payload_packet_len - 4 == get_l16(packet->payload, 0))
				|| (packet->payload_packet_len == get_l16(packet->payload, 0))
				|| (packet->payload_packet_len >= 6 && packet->payload_packet_len - 6 == get_l16(packet->payload, 0)))
			&& (packet->payload[2] == 0x00 && packet->payload[4] == 0x03)) {
			IPQ_LOG(IPOQUE_PROTOCOL_PPSTREAM, ipoque_struct, IPQ_LOG_DEBUG,
					"found ppstream over udp with pattern Vb.\n");
			ipoque_int_ppstream_add_connection(ipoque_struct);
			return;
		}




	}

	IPQ_LOG(IPOQUE_PROTOCOL_PPSTREAM, ipoque_struct, IPQ_LOG_DEBUG, "exclude ppstream.\n");
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_PPSTREAM);
}
#endif
