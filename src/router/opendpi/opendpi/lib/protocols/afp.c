/*
 * afp.c
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
#ifdef IPOQUE_PROTOCOL_AFP

static void ipoque_int_afp_add_connection(struct ipoque_detection_module_struct
										  *ipoque_struct)
{
	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_AFP, IPOQUE_REAL_PROTOCOL);
}


void ipoque_search_afp(struct ipoque_detection_module_struct *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
//  struct ipoque_id_struct *src = ipoque_struct->src;
//  struct ipoque_id_struct *dst = ipoque_struct->dst;


	/*
	 * this will detect the OpenSession command of the Data Stream Interface (DSI) protocol
	 * which is exclusively used by the Apple Filing Protocol (AFP) on TCP/IP networks
	 */
	if (packet->payload_packet_len >= 22 && get_u16(packet->payload, 0) == htons(0x0004) &&
		get_u16(packet->payload, 2) == htons(0x0001) && get_u32(packet->payload, 4) == 0 &&
		get_u32(packet->payload, 8) == htonl(packet->payload_packet_len - 16) &&
		get_u32(packet->payload, 12) == 0 && get_u16(packet->payload, 16) == htons(0x0104)) {

		IPQ_LOG(IPOQUE_PROTOCOL_AFP, ipoque_struct, IPQ_LOG_DEBUG, "AFP: DSI OpenSession detected.\n");
		ipoque_int_afp_add_connection(ipoque_struct);
		return;
	}

	/*
	 * detection of GetStatus command of DSI protocl
	 */
	if (packet->payload_packet_len >= 18 && get_u16(packet->payload, 0) == htons(0x0003) &&
		get_u16(packet->payload, 2) == htons(0x0001) && get_u32(packet->payload, 4) == 0 &&
		get_u32(packet->payload, 8) == htonl(packet->payload_packet_len - 16) &&
		get_u32(packet->payload, 12) == 0 && get_u16(packet->payload, 16) == htons(0x0f00)) {

		IPQ_LOG(IPOQUE_PROTOCOL_AFP, ipoque_struct, IPQ_LOG_DEBUG, "AFP: DSI GetStatus detected.\n");
		ipoque_int_afp_add_connection(ipoque_struct);
		return;
	}


	IPQ_LOG(IPOQUE_PROTOCOL_AFP, ipoque_struct, IPQ_LOG_DEBUG, "AFP excluded.\n");
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_AFP);
}

#endif
