/*
 * guildwars.c
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
#ifdef IPOQUE_PROTOCOL_GUILDWARS


static void ipoque_int_guildwars_add_connection(struct ipoque_detection_module_struct
												*ipoque_struct)
{
	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_GUILDWARS, IPOQUE_REAL_PROTOCOL);
}

void ipoque_search_guildwars_tcp(struct ipoque_detection_module_struct
								 *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
//      struct ipoque_id_struct         *src=ipoque_struct->src;
//      struct ipoque_id_struct         *dst=ipoque_struct->dst;

	IPQ_LOG(IPOQUE_PROTOCOL_GUILDWARS, ipoque_struct, IPQ_LOG_DEBUG, "search guildwars.\n");

	if (packet->payload_packet_len == 64 && get_u16(packet->payload, 1) == ntohs(0x050c)
		&& memcmp(&packet->payload[50], "@2&P", 4) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_GUILDWARS, ipoque_struct, IPQ_LOG_DEBUG, "GuildWars version 29.350: found.\n");
		ipoque_int_guildwars_add_connection(ipoque_struct);
		return;
	}
	if (packet->payload_packet_len == 16 && get_u16(packet->payload, 1) == ntohs(0x040c)
		&& get_u16(packet->payload, 4) == ntohs(0xa672)
		&& packet->payload[8] == 0x01 && packet->payload[12] == 0x04) {
		IPQ_LOG(IPOQUE_PROTOCOL_GUILDWARS, ipoque_struct, IPQ_LOG_DEBUG, "GuildWars version 29.350: found.\n");
		ipoque_int_guildwars_add_connection(ipoque_struct);
		return;
	}
	if (packet->payload_packet_len == 21 && get_u16(packet->payload, 0) == ntohs(0x0100)
		&& get_u32(packet->payload, 5) == ntohl(0xf1001000)
		&& packet->payload[9] == 0x01) {
		IPQ_LOG(IPOQUE_PROTOCOL_GUILDWARS, ipoque_struct, IPQ_LOG_DEBUG, "GuildWars version 216.107.245.50: found.\n");
		ipoque_int_guildwars_add_connection(ipoque_struct);
		return;
	}

	IPQ_LOG(IPOQUE_PROTOCOL_GUILDWARS, ipoque_struct, IPQ_LOG_DEBUG, "exclude guildwars.\n");
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_GUILDWARS);
}

#endif
