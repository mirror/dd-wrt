/*
 * i23v5.c
 *
 * Copyright (C) 2009-2011 by ipoque GmbH
 * Copyright (C) 2011-13 - ntop.org
 *
 * This file is part of nDPI, an open source deep packet inspection
 * library based on the OpenDPI and PACE technology by ipoque GmbH
 *
 * nDPI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * nDPI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with nDPI.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#include "ndpi_protocols.h"
#ifdef NDPI_PROTOCOL_I23V5

static void ndpi_i23v5_add_connection(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{

	ndpi_set_detected_protocol(ndpi_struct, flow, NDPI_PROTOCOL_I23V5, NDPI_PROTOCOL_UNKNOWN);
}

static void ndpi_search_i23v5(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{
	struct ndpi_packet_struct *packet = &flow->packet;

//      struct ndpi_id_struct         *src=ndpi_struct->src;
//      struct ndpi_id_struct         *dst=ndpi_struct->dst;

	u_int32_t i;
	u_int32_t sum;

	NDPI_LOG(NDPI_PROTOCOL_I23V5, ndpi_struct, NDPI_LOG_DEBUG, "search i23v5.\n");

	/*
	 * encryption of i23v5 is tricky:  the 7th bit of the first byte and the sescond bit of the second byte must be set.
	 * Three lengths are written in three packets after 0x0d58, 0x0e58 and 0x0f58 but without a certain order.
	 * The sum of the three packets is in another packet at any place.
	 */

	if (packet->payload_packet_len > 7 && ((packet->payload[0] & 0x04) == 0x04 && (packet->payload[2] & 0x80) == 0x80)) {
		NDPI_LOG(NDPI_PROTOCOL_I23V5, ndpi_struct, NDPI_LOG_DEBUG, "found i23v5 handshake bits.\n");

		for (i = 3; i < packet->payload_packet_len - 5; i++) {
			if (packet->payload[i] == 0x0d && packet->payload[i + 1] == 0x58) {
				NDPI_LOG(NDPI_PROTOCOL_I23V5, ndpi_struct, NDPI_LOG_DEBUG, "found first i23v5 key len.\n");
				flow->i23v5_len1 = get_u_int32_t(packet->payload, i + 2);
				return;
			}
		}
		for (i = 3; i < packet->payload_packet_len - 5; i++) {
			if (packet->payload[i] == 0x0e && packet->payload[i + 1] == 0x58) {
				NDPI_LOG(NDPI_PROTOCOL_I23V5, ndpi_struct, NDPI_LOG_DEBUG, "found second i23v5 key len.\n");
				flow->i23v5_len2 = get_u_int32_t(packet->payload, i + 2);
				return;
			}
		}
		for (i = 3; i < packet->payload_packet_len - 5; i++) {
			if (packet->payload[i] == 0x0f && packet->payload[i + 1] == 0x58) {
				NDPI_LOG(NDPI_PROTOCOL_I23V5, ndpi_struct, NDPI_LOG_DEBUG, "found third i23v5 key len.\n");
				flow->i23v5_len3 = get_u_int32_t(packet->payload, i + 2);
				return;
			}
		}
		if (flow->i23v5_len1 != 0 && flow->i23v5_len2 != 0 && flow->i23v5_len3 != 0) {
			for (i = 3; i < packet->payload_packet_len - 5; i++) {
				sum = flow->i23v5_len1 + flow->i23v5_len2 + flow->i23v5_len3;
				if (get_u_int32_t(packet->payload, i) == sum) {
					NDPI_LOG(NDPI_PROTOCOL_I23V5, ndpi_struct, NDPI_LOG_DEBUG, "key sum matches.\n");
					ndpi_i23v5_add_connection(ndpi_struct, flow);
				}

			}
		}
	}

	NDPI_LOG(NDPI_PROTOCOL_I23V5, ndpi_struct, NDPI_LOG_DEBUG, "exclude i23v5.\n");
	NDPI_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, NDPI_PROTOCOL_I23V5);
}


static void init_i23v5_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id, NDPI_PROTOCOL_BITMASK * detection_bitmask)
{
	ndpi_set_bitmask_protocol_detection("I23V5", ndpi_struct, detection_bitmask, *id,
					    NDPI_PROTOCOL_I23V5,
					    ndpi_search_i23v5, NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION, SAVE_DETECTION_BITMASK_AS_UNKNOWN, ADD_TO_DETECTION_BITMASK);

	*id += 1;
}


#endif
