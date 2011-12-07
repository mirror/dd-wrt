/*
 * maplestory.c
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

#ifdef IPOQUE_PROTOCOL_MAPLESTORY

static void ipoque_int_maplestory_add_connection(struct ipoque_detection_module_struct
												 *ipoque_struct, ipoque_protocol_type_t protocol_type)
{
	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_MAPLESTORY, protocol_type);
}


void ipoque_search_maplestory(struct ipoque_detection_module_struct *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
//      struct ipoque_id_struct         *src=ipoque_struct->src;
//      struct ipoque_id_struct         *dst=ipoque_struct->dst;



	if (packet->payload_packet_len == 16
		&& (ntohl(get_u32(packet->payload, 0)) == 0x0e003a00 || ntohl(get_u32(packet->payload, 0)) == 0x0e003b00
			|| ntohl(get_u32(packet->payload, 0)) == 0x0e004200)
		&& ntohs(get_u16(packet->payload, 4)) == 0x0100 && (packet->payload[6] == 0x32 || packet->payload[6] == 0x33)) {
		IPQ_LOG(IPOQUE_PROTOCOL_MAPLESTORY, ipoque_struct, IPQ_LOG_DEBUG, "found maplestory.\n");
		ipoque_int_maplestory_add_connection(ipoque_struct, IPOQUE_REAL_PROTOCOL);
		return;
	}

	if (packet->payload_packet_len > IPQ_STATICSTRING_LEN("GET /maple")
		&& memcmp(packet->payload, "GET /maple", IPQ_STATICSTRING_LEN("GET /maple")) == 0) {
		ipq_parse_packet_line_info(ipoque_struct);
		/* Maplestory update */
		if (packet->payload_packet_len > IPQ_STATICSTRING_LEN("GET /maple/patch")
			&& packet->payload[IPQ_STATICSTRING_LEN("GET /maple")] == '/') {
			if (packet->user_agent_line.ptr != NULL && packet->host_line.ptr != NULL
				&& packet->user_agent_line.len == IPQ_STATICSTRING_LEN("Patcher")
				&& packet->host_line.len > IPQ_STATICSTRING_LEN("patch.")
				&& memcmp(&packet->payload[IPQ_STATICSTRING_LEN("GET /maple/")], "patch",
						  IPQ_STATICSTRING_LEN("patch")) == 0
				&& memcmp(packet->user_agent_line.ptr, "Patcher", IPQ_STATICSTRING_LEN("Patcher")) == 0
				&& memcmp(packet->host_line.ptr, "patch.", IPQ_STATICSTRING_LEN("patch.")) == 0) {
				IPQ_LOG(IPOQUE_PROTOCOL_MAPLESTORY, ipoque_struct, IPQ_LOG_DEBUG, "found maplestory update.\n");
				ipoque_int_maplestory_add_connection(ipoque_struct, IPOQUE_CORRELATED_PROTOCOL);
				return;
			}
		} else if (packet->user_agent_line.ptr != NULL && packet->user_agent_line.len == IPQ_STATICSTRING_LEN("AspINet")
				   && memcmp(&packet->payload[IPQ_STATICSTRING_LEN("GET /maple")], "story/",
							 IPQ_STATICSTRING_LEN("story/")) == 0
				   && memcmp(packet->user_agent_line.ptr, "AspINet", IPQ_STATICSTRING_LEN("AspINet")) == 0) {
			IPQ_LOG(IPOQUE_PROTOCOL_MAPLESTORY, ipoque_struct, IPQ_LOG_DEBUG, "found maplestory update.\n");
			ipoque_int_maplestory_add_connection(ipoque_struct, IPOQUE_CORRELATED_PROTOCOL);
			return;
		}
	}

	IPQ_LOG(IPOQUE_PROTOCOL_MAPLESTORY, ipoque_struct, IPQ_LOG_DEBUG, "exclude maplestory.\n");
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_MAPLESTORY);

}
#endif
