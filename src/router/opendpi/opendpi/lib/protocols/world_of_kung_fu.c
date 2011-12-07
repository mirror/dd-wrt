/*
 * world_of_kung_fu.c
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
#ifdef IPOQUE_PROTOCOL_WORLD_OF_KUNG_FU

static void ipoque_int_world_of_kung_fu_add_connection(struct ipoque_detection_module_struct *ipoque_struct)
{
	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_WORLD_OF_KUNG_FU, IPOQUE_REAL_PROTOCOL);
}

void ipoque_search_world_of_kung_fu(struct ipoque_detection_module_struct *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
	//      struct ipoque_id_struct         *src=ipoque_struct->src;
	//      struct ipoque_id_struct         *dst=ipoque_struct->dst;

	IPQ_LOG(IPOQUE_PROTOCOL_WORLD_OF_KUNG_FU, ipoque_struct, IPQ_LOG_DEBUG, "search world_of_kung_fu.\n");

	if ((packet->payload_packet_len == 16)
		&& ntohl(get_u32(packet->payload, 0)) == 0x0c000000 && ntohl(get_u32(packet->payload, 4)) == 0xd2000c00
		&& (packet->payload[9]
			== 0x16) && ntohs(get_u16(packet->payload, 10)) == 0x0000 && ntohs(get_u16(packet->payload, 14)) == 0x0000) {
		IPQ_LOG(IPOQUE_PROTOCOL_WORLD_OF_KUNG_FU, ipoque_struct, IPQ_LOG_DEBUG, "detected world_of_kung_fu.\n");
		ipoque_int_world_of_kung_fu_add_connection(ipoque_struct);
		return;
	}

	IPQ_LOG(IPOQUE_PROTOCOL_WORLD_OF_KUNG_FU, ipoque_struct, IPQ_LOG_DEBUG, "exclude world_of_kung_fu.\n");
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_WORLD_OF_KUNG_FU);
}

#endif
