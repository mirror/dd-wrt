/*
 * tvants.c
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

#ifdef IPOQUE_PROTOCOL_TVANTS

static void ipoque_int_tvants_add_connection(struct ipoque_detection_module_struct
											 *ipoque_struct)
{
	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_TVANTS, IPOQUE_REAL_PROTOCOL);
}




void ipoque_search_tvants_udp(struct ipoque_detection_module_struct
							  *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;

//      struct ipoque_id_struct         *src=ipoque_struct->src;
//      struct ipoque_id_struct         *dst=ipoque_struct->dst;


	IPQ_LOG(IPOQUE_PROTOCOL_TVANTS, ipoque_struct, IPQ_LOG_DEBUG, "search tvants.  \n");

	if (packet->udp != NULL && packet->payload_packet_len > 57
		&& packet->payload[0] == 0x04 && packet->payload[1] == 0x00
		&& (packet->payload[2] == 0x05 || packet->payload[2] == 0x06
			|| packet->payload[2] == 0x07) && packet->payload[3] == 0x00
		&& packet->payload_packet_len == (packet->payload[5] << 8) + packet->payload[4]
		&& packet->payload[6] == 0x00 && packet->payload[7] == 0x00
		&& (memcmp(&packet->payload[48], "TVANTS", 6) == 0
			|| memcmp(&packet->payload[49], "TVANTS", 6) == 0 || memcmp(&packet->payload[51], "TVANTS", 6) == 0)) {

		IPQ_LOG(IPOQUE_PROTOCOL_TVANTS, ipoque_struct, IPQ_LOG_DEBUG, "found tvants over udp.  \n");
		ipoque_int_tvants_add_connection(ipoque_struct);

	} else if (packet->tcp != NULL && packet->payload_packet_len > 15
			   && packet->payload[0] == 0x04 && packet->payload[1] == 0x00
			   && packet->payload[2] == 0x07 && packet->payload[3] == 0x00
			   && packet->payload_packet_len == (packet->payload[5] << 8) + packet->payload[4]
			   && packet->payload[6] == 0x00 && packet->payload[7] == 0x00
			   && memcmp(&packet->payload[8], "TVANTS", 6) == 0) {

		IPQ_LOG(IPOQUE_PROTOCOL_TVANTS, ipoque_struct, IPQ_LOG_DEBUG, "found tvants over tcp.  \n");
		ipoque_int_tvants_add_connection(ipoque_struct);

	}
	IPQ_LOG(IPOQUE_PROTOCOL_TVANTS, ipoque_struct, IPQ_LOG_DEBUG, "exclude tvants.  \n");
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_TVANTS);

}
#endif
