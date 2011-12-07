/*
 * iax.c
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
#ifdef IPOQUE_PROTOCOL_IAX

#define IPQ_IAX_MAX_INFORMATION_ELEMENTS 15

static void ipoque_int_iax_add_connection(struct ipoque_detection_module_struct
										  *ipoque_struct)
{
	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_IAX, IPOQUE_REAL_PROTOCOL);
}

static void ipoque_search_setup_iax(struct ipoque_detection_module_struct
									*ipoque_struct);

static void ipoque_search_setup_iax(struct ipoque_detection_module_struct *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;


	u8 i;
	u16 packet_len;

	if (						/* 1. iax is udp based, port 4569 */
		   (packet->udp->source == htons(4569) || packet->udp->dest == htons(4569))
		   /* check for iax new packet */
		   && packet->payload_packet_len >= 12
		   /* check for dst call id == 0, do not check for highest bit (packet retransmission) */
		   // && (ntohs(get_u16(packet->payload, 2)) & 0x7FFF) == 0
		   /* check full IAX packet  */
		   && (packet->payload[0] & 0x80) != 0
		   /* outbound seq == 0 */
		   && packet->payload[8] == 0
		   /* inbound seq == 0 || 1  */
		   && (packet->payload[9] == 0 || packet->payload[9] == 0x01)
		   /*  */
		   && packet->payload[10] == 0x06
		   /* IAX type: 0-15 */
		   && packet->payload[11] <= 15) {

		if (packet->payload_packet_len == 12) {
			IPQ_LOG(IPOQUE_PROTOCOL_IAX, ipoque_struct, IPQ_LOG_DEBUG, "found IAX.\n");
			ipoque_int_iax_add_connection(ipoque_struct);
			return;
		}
		packet_len = 12;
		for (i = 0; i < IPQ_IAX_MAX_INFORMATION_ELEMENTS; i++) {
			packet_len = packet_len + 2 + packet->payload[packet_len + 1];
			if (packet_len == packet->payload_packet_len) {
				IPQ_LOG(IPOQUE_PROTOCOL_IAX, ipoque_struct, IPQ_LOG_DEBUG, "found IAX.\n");
				ipoque_int_iax_add_connection(ipoque_struct);
				return;
			}
			if (packet_len > packet->payload_packet_len) {
				break;
			}
		}

	}

	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_IAX);

}

void ipoque_search_iax(struct ipoque_detection_module_struct *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
//      struct ipoque_flow_struct       *flow=ipoque_struct->flow;
//      struct ipoque_id_struct         *src=ipoque_struct->src;
//      struct ipoque_id_struct         *dst=ipoque_struct->dst;

	if (packet->detected_protocol_stack[0] == IPOQUE_PROTOCOL_UNKNOWN)
		ipoque_search_setup_iax(ipoque_struct);
}
#endif
