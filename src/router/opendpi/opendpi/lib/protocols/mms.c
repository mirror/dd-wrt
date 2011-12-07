/*
 * mms.c
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

#ifdef IPOQUE_PROTOCOL_MMS


static void ipoque_int_mms_add_connection(struct ipoque_detection_module_struct
										  *ipoque_struct)
{
	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_MMS, IPOQUE_REAL_PROTOCOL);
}


void ipoque_search_mms_tcp(struct ipoque_detection_module_struct *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
//      struct ipoque_id_struct         *src=ipoque_struct->src;
//      struct ipoque_id_struct         *dst=ipoque_struct->dst;


	/* search MSMMS packets */
	if (packet->payload_packet_len >= 20) {

		if (flow->l4.tcp.mms_stage == 0 && packet->payload[4] == 0xce
			&& packet->payload[5] == 0xfa && packet->payload[6] == 0x0b
			&& packet->payload[7] == 0xb0 && packet->payload[12] == 0x4d
			&& packet->payload[13] == 0x4d && packet->payload[14] == 0x53 && packet->payload[15] == 0x20) {
			IPQ_LOG(IPOQUE_PROTOCOL_MMS, ipoque_struct, IPQ_LOG_DEBUG, "MMS: MSMMS Request found \n");
			flow->l4.tcp.mms_stage = 1 + packet->packet_direction;
			return;
		}

		if (flow->l4.tcp.mms_stage == 2 - packet->packet_direction
			&& packet->payload[4] == 0xce && packet->payload[5] == 0xfa
			&& packet->payload[6] == 0x0b && packet->payload[7] == 0xb0
			&& packet->payload[12] == 0x4d && packet->payload[13] == 0x4d
			&& packet->payload[14] == 0x53 && packet->payload[15] == 0x20) {
			IPQ_LOG(IPOQUE_PROTOCOL_MMS, ipoque_struct, IPQ_LOG_DEBUG, "MMS: MSMMS Response found \n");
			ipoque_int_mms_add_connection(ipoque_struct);
			return;
		}
	}
#ifdef IPOQUE_PROTOCOL_HTTP
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_HTTP) != 0) {
#endif							/* IPOQUE_PROTOCOL_HTTP */
		IPQ_LOG(IPOQUE_PROTOCOL_MMS, ipoque_struct, IPQ_LOG_DEBUG, "MMS: exclude\n");
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_MMS);

#ifdef IPOQUE_PROTOCOL_HTTP
	} else {
		IPQ_LOG(IPOQUE_PROTOCOL_MMS, ipoque_struct, IPQ_LOG_DEBUG, "MMS avoid early exclude from http\n");
	}
#endif							/* IPOQUE_PROTOCOL_HTTP */

}
#endif
