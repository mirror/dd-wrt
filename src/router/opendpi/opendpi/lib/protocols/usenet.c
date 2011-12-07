/*
 * usenet.c
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

#ifdef IPOQUE_PROTOCOL_USENET


static void ipoque_int_usenet_add_connection(struct ipoque_detection_module_struct
											 *ipoque_struct)
{
	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_USENET, IPOQUE_REAL_PROTOCOL);
}



void ipoque_search_usenet_tcp(struct ipoque_detection_module_struct
							  *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
//      struct ipoque_id_struct         *src=ipoque_struct->src;
//      struct ipoque_id_struct         *dst=ipoque_struct->dst;

	IPQ_LOG(IPOQUE_PROTOCOL_USENET, ipoque_struct, IPQ_LOG_DEBUG, "USENET: search usenet.\n");





	IPQ_LOG(IPOQUE_PROTOCOL_USENET, ipoque_struct, IPQ_LOG_DEBUG, "USENET: STAGE IS %u.\n", flow->l4.tcp.usenet_stage);


	// check for the first server replay
	/*
	   200    Service available, posting allowed
	   201    Service available, posting prohibited
	 */
	if (flow->l4.tcp.usenet_stage == 0 && packet->payload_packet_len > 10
		&& ((ipq_mem_cmp(packet->payload, "200 ", 4) == 0)
			|| (ipq_mem_cmp(packet->payload, "201 ", 4) == 0))) {

		IPQ_LOG(IPOQUE_PROTOCOL_USENET, ipoque_struct, IPQ_LOG_DEBUG, "USENET: found 200 or 201.\n");
		flow->l4.tcp.usenet_stage = 1 + packet->packet_direction;

		IPQ_LOG(IPOQUE_PROTOCOL_USENET, ipoque_struct, IPQ_LOG_DEBUG, "USENET: maybe hit.\n");
		return;
	}

	/*
	   [C] AUTHINFO USER fred
	   [S] 381 Enter passphrase
	   [C] AUTHINFO PASS flintstone
	   [S] 281 Authentication accepted
	 */
	// check for client username
	if (flow->l4.tcp.usenet_stage == 2 - packet->packet_direction) {
		if (packet->payload_packet_len > 20 && (ipq_mem_cmp(packet->payload, "AUTHINFO USER ", 14) == 0)) {
			IPQ_LOG(IPOQUE_PROTOCOL_USENET, ipoque_struct, IPQ_LOG_DEBUG, "USENET: username found\n");
			flow->l4.tcp.usenet_stage = 3 + packet->packet_direction;

			IPQ_LOG(IPOQUE_PROTOCOL_USENET, ipoque_struct, IPQ_LOG_DEBUG, "USENET: found usenet.\n");
			ipoque_int_usenet_add_connection(ipoque_struct);
			return;
		} else if (packet->payload_packet_len == 13 && (ipq_mem_cmp(packet->payload, "MODE READER\r\n", 13) == 0)) {
			IPQ_LOG(IPOQUE_PROTOCOL_USENET, ipoque_struct, IPQ_LOG_DEBUG,
					"USENET: no login necessary but we are a client.\n");

			IPQ_LOG(IPOQUE_PROTOCOL_USENET, ipoque_struct, IPQ_LOG_DEBUG, "USENET: found usenet.\n");
			ipoque_int_usenet_add_connection(ipoque_struct);
			return;
		}
	}



	IPQ_LOG(IPOQUE_PROTOCOL_USENET, ipoque_struct, IPQ_LOG_DEBUG, "USENET: exclude usenet.\n");

	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_USENET);

}

#endif
