/*
 * bgp.c
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
#ifdef IPOQUE_PROTOCOL_BGP


static void ipoque_int_bgp_add_connection(struct ipoque_detection_module_struct
										  *ipoque_struct)
{

	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_BGP, IPOQUE_REAL_PROTOCOL);
}

/* this detection also works asymmetrically */
void ipoque_search_bgp(struct ipoque_detection_module_struct *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
//      struct ipoque_id_struct         *src=ipoque_struct->src;
//      struct ipoque_id_struct         *dst=ipoque_struct->dst;

	if (packet->payload_packet_len > 18 &&
		get_u64(packet->payload, 0) == 0xffffffffffffffffULL &&
		get_u64(packet->payload, 8) == 0xffffffffffffffffULL &&
		ntohs(get_u16(packet->payload, 16)) <= packet->payload_packet_len &&
		(packet->tcp->dest == htons(179) || packet->tcp->source == htons(179))
		&& packet->payload[18] < 5) {
		IPQ_LOG(IPOQUE_PROTOCOL_BGP, ipoque_struct, IPQ_LOG_DEBUG, "BGP detected.\n");
		ipoque_int_bgp_add_connection(ipoque_struct);
		return;
	}

	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_BGP);
}

#endif
