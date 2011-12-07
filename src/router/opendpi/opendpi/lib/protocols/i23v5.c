/*
 * i23v5.c
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
#ifdef IPOQUE_PROTOCOL_I23V5


static void ipoque_i23v5_add_connection(struct ipoque_detection_module_struct
										*ipoque_struct)
{

	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_I23V5, IPOQUE_REAL_PROTOCOL);
}

void ipoque_search_i23v5(struct ipoque_detection_module_struct
						 *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
//      struct ipoque_id_struct         *src=ipoque_struct->src;
//      struct ipoque_id_struct         *dst=ipoque_struct->dst;

	u32 i;
	u32 sum;

	IPQ_LOG(IPOQUE_PROTOCOL_I23V5, ipoque_struct, IPQ_LOG_DEBUG, "search i23v5.\n");

	/*
	 * encryption of i23v5 is tricky:  the 7th bit of the first byte and the sescond bit of the second byte must be set.
	 * Three lengths are written in three packets after 0x0d58, 0x0e58 and 0x0f58 but without a certain order.
	 * The sum of the three packets is in another packet at any place.
	 */

	if (packet->payload_packet_len > 7 && ((packet->payload[0] & 0x04) == 0x04 && (packet->payload[2] & 0x80) == 0x80)) {
		IPQ_LOG(IPOQUE_PROTOCOL_I23V5, ipoque_struct, IPQ_LOG_DEBUG, "found i23v5 handshake bits.\n");

		for (i = 3; i < packet->payload_packet_len - 5; i++) {
			if (packet->payload[i] == 0x0d && packet->payload[i + 1] == 0x58) {
				IPQ_LOG(IPOQUE_PROTOCOL_I23V5, ipoque_struct, IPQ_LOG_DEBUG, "found first i23v5 key len.\n");
				flow->i23v5_len1 = get_u32(packet->payload, i + 2);
				return;
			}
		}
		for (i = 3; i < packet->payload_packet_len - 5; i++) {
			if (packet->payload[i] == 0x0e && packet->payload[i + 1] == 0x58) {
				IPQ_LOG(IPOQUE_PROTOCOL_I23V5, ipoque_struct, IPQ_LOG_DEBUG, "found second i23v5 key len.\n");
				flow->i23v5_len2 = get_u32(packet->payload, i + 2);
				return;
			}
		}
		for (i = 3; i < packet->payload_packet_len - 5; i++) {
			if (packet->payload[i] == 0x0f && packet->payload[i + 1] == 0x58) {
				IPQ_LOG(IPOQUE_PROTOCOL_I23V5, ipoque_struct, IPQ_LOG_DEBUG, "found third i23v5 key len.\n");
				flow->i23v5_len3 = get_u32(packet->payload, i + 2);
				return;
			}
		}
		if (flow->i23v5_len1 != 0 && flow->i23v5_len2 != 0 && flow->i23v5_len3 != 0) {
			for (i = 3; i < packet->payload_packet_len - 5; i++) {
				sum = flow->i23v5_len1 + flow->i23v5_len2 + flow->i23v5_len3;
				if (get_u32(packet->payload, i) == sum) {
					IPQ_LOG(IPOQUE_PROTOCOL_I23V5, ipoque_struct, IPQ_LOG_DEBUG, "key sum matches.\n");
					ipoque_i23v5_add_connection(ipoque_struct);
				}

			}
		}
	}

	IPQ_LOG(IPOQUE_PROTOCOL_I23V5, ipoque_struct, IPQ_LOG_DEBUG, "exclude i23v5.\n");
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_I23V5);
}

#endif
