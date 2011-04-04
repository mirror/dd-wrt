/*
 * tftp.c
 * Copyright (C) 2009-2010 by ipoque GmbH
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
#ifdef IPOQUE_PROTOCOL_TFTP

static void ipoque_int_tftp_add_connection(struct ipoque_detection_module_struct
										   *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
	struct ipoque_id_struct *src = ipoque_struct->src;
	struct ipoque_id_struct *dst = ipoque_struct->dst;

	flow->detected_protocol = IPOQUE_PROTOCOL_TFTP;
	packet->detected_protocol = IPOQUE_PROTOCOL_TFTP;

	if (src != NULL) {
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(src->detected_protocol_bitmask, IPOQUE_PROTOCOL_TFTP);
	}
	if (dst != NULL) {
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(dst->detected_protocol_bitmask, IPOQUE_PROTOCOL_TFTP);
	}
}

void ipoque_search_tftp(struct ipoque_detection_module_struct
						*ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;



	IPQ_LOG(IPOQUE_PROTOCOL_TFTP, ipoque_struct, IPQ_LOG_DEBUG, "search TFTP.\n");



	if (packet->payload_packet_len > 3 && flow->tftp_stage == 0 && ntohl(get_u32(packet->payload, 0)) == 0x00030001) {
		IPQ_LOG(IPOQUE_PROTOCOL_TFTP, ipoque_struct, IPQ_LOG_DEBUG, "maybe tftp. need next packet.\n");
		flow->tftp_stage = 1;
		return;
	}
	if (packet->payload_packet_len > 3 && (flow->tftp_stage == 1)
		&& ntohl(get_u32(packet->payload, 0)) == 0x00040001) {

		IPQ_LOG(IPOQUE_PROTOCOL_TFTP, ipoque_struct, IPQ_LOG_DEBUG, "found tftp.\n");
		ipoque_int_tftp_add_connection(ipoque_struct);
		return;
	}
	if (packet->payload_packet_len > 1
		&& ((packet->payload[0] == 0 && packet->payload[packet->payload_packet_len - 1] == 0)
			|| (packet->payload_packet_len == 4 && ntohl(get_u32(packet->payload, 0)) == 0x00040000))) {
		IPQ_LOG(IPOQUE_PROTOCOL_TFTP, ipoque_struct, IPQ_LOG_DEBUG, "skip initial packet.\n");
		return;
	}

	IPQ_LOG(IPOQUE_PROTOCOL_TFTP, ipoque_struct, IPQ_LOG_DEBUG, "exclude TFTP.\n");
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_TFTP);
}
#endif
