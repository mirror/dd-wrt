/*
 * world_of_warcraft.c
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
#ifdef IPOQUE_PROTOCOL_WORLDOFWARCRAFT

static void ipoque_search_worldofwarcraft(struct ipoque_detection_module_struct
								   *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
	struct ipoque_id_struct *src = ipoque_struct->src;
	struct ipoque_id_struct *dst = ipoque_struct->dst;

	if (packet->tcp != NULL) {
		if (packet->tcp->dest == htons(3724) && packet->payload_packet_len < 70
			&& packet->payload_packet_len > 40 && (memcmp(&packet->payload[4], "WoW", 3) == 0
												   || memcmp(&packet->payload[5], "WoW", 3) == 0)) {
			ipq_connection_detected(ipoque_struct, IPOQUE_PROTOCOL_WORLDOFWARCRAFT);
			IPQ_LOG(IPOQUE_PROTOCOL_WORLDOFWARCRAFT, ipoque_struct, IPQ_LOG_DEBUG, "World of Warcraft: Login found\n");
			return;
		}

		if (IPQ_SRC_OR_DST_HAS_PROTOCOL(src, dst, IPOQUE_PROTOCOL_WORLDOFWARCRAFT) != 0) {
			if (packet->tcp->source == htons(3724)
				&& packet->payload_packet_len == 8 && get_u32(packet->payload, 0) == htonl(0x0006ec01)) {
				ipq_connection_detected(ipoque_struct, IPOQUE_PROTOCOL_WORLDOFWARCRAFT);
				IPQ_LOG(IPOQUE_PROTOCOL_WORLDOFWARCRAFT, ipoque_struct,
						IPQ_LOG_DEBUG, "World of Warcraft: connection detected\n");
				return;
			}
		}

	}

	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_WORLDOFWARCRAFT);
}

#endif
