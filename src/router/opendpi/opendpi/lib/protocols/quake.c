/*
 * quake.c
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


#include "ipq_utils.h"
#ifdef IPOQUE_PROTOCOL_QUAKE

static void ipoque_int_quake_add_connection(struct ipoque_detection_module_struct
											*ipoque_struct)
{
	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_QUAKE, IPOQUE_REAL_PROTOCOL);
}

void ipoque_search_quake(struct ipoque_detection_module_struct *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
//      struct ipoque_id_struct         *src=ipoque_struct->src;
//      struct ipoque_id_struct         *dst=ipoque_struct->dst;

	if ((packet->payload_packet_len == 14
		 && get_u16(packet->payload, 0) == 0xffff && ipq_mem_cmp(&packet->payload[2], "getInfo", 7) == 0)
		|| (packet->payload_packet_len == 17
			&& get_u16(packet->payload, 0) == 0xffff && ipq_mem_cmp(&packet->payload[2], "challenge", 9) == 0)
		|| (packet->payload_packet_len > 20
			&& packet->payload_packet_len < 30
			&& get_u16(packet->payload, 0) == 0xffff && ipq_mem_cmp(&packet->payload[2], "getServers", 10) == 0)) {
		IPQ_LOG(IPOQUE_PROTOCOL_QUAKE, ipoque_struct, IPQ_LOG_DEBUG, "Quake IV detected.\n");
		ipoque_int_quake_add_connection(ipoque_struct);
		return;
	}

	/* Quake III/Quake Live */
	if (packet->payload_packet_len == 15 && get_u32(packet->payload, 0) == 0xffffffff
		&& memcmp(&packet->payload[4], "getinfo", IPQ_STATICSTRING_LEN("getinfo")) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_QUAKE, ipoque_struct, IPQ_LOG_DEBUG, "Quake III Arena/Quake Live detected.\n");
		ipoque_int_quake_add_connection(ipoque_struct);
		return;
	}
	if (packet->payload_packet_len == 16 && get_u32(packet->payload, 0) == 0xffffffff
		&& memcmp(&packet->payload[4], "getchallenge", IPQ_STATICSTRING_LEN("getchallenge")) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_QUAKE, ipoque_struct, IPQ_LOG_DEBUG, "Quake III Arena/Quake Live detected.\n");
		ipoque_int_quake_add_connection(ipoque_struct);
		return;
	}
	if (packet->payload_packet_len > 20 && packet->payload_packet_len < 30
		&& get_u32(packet->payload, 0) == 0xffffffff
		&& memcmp(&packet->payload[4], "getservers", IPQ_STATICSTRING_LEN("getservers")) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_QUAKE, ipoque_struct, IPQ_LOG_DEBUG, "Quake III Arena/Quake Live detected.\n");
		ipoque_int_quake_add_connection(ipoque_struct);
		return;
	}



	/* ports for startup packet:
	   Quake I        26000 (starts with 0x8000)
	   Quake II       27910
	   Quake III      27960 (increases with each player)
	   Quake IV       27650
	   Quake World    27500
	   Quake Wars     ?????
	 */

	IPQ_LOG(IPOQUE_PROTOCOL_QUAKE, ipoque_struct, IPQ_LOG_DEBUG, "Quake excluded.\n");
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_QUAKE);
}

#endif
