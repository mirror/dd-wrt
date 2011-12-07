/*
 * xbox.c
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
#ifdef IPOQUE_PROTOCOL_XBOX

static void ipoque_int_xbox_add_connection(struct ipoque_detection_module_struct
										   *ipoque_struct)
{
	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_XBOX, IPOQUE_REAL_PROTOCOL);
}


void ipoque_search_xbox(struct ipoque_detection_module_struct *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
	//  struct ipoque_id_struct *src = ipoque_struct->src;
	//  struct ipoque_id_struct *dst = ipoque_struct->dst;

	/*
	 * THIS IS TH XBOX UDP DETCTION ONLY !!!
	 * the xbox tcp detection is done by http code
	 */


	/* this detection also works for asymmetric xbox udp traffic */
	if (packet->udp != NULL) {

		u16 dport = ntohs(packet->udp->dest);
		u16 sport = ntohs(packet->udp->source);

		IPQ_LOG(IPOQUE_PROTOCOL_XBOX, ipoque_struct, IPQ_LOG_DEBUG, "search xbox\n");

		if (packet->payload_packet_len > 12 &&
			get_u32(packet->payload, 0) == 0 && packet->payload[5] == 0x58 &&
			memcmp(&packet->payload[7], "\x00\x00\x00", 3) == 0) {

			if ((packet->payload[4] == 0x0c && packet->payload[6] == 0x76) ||
				(packet->payload[4] == 0x02 && packet->payload[6] == 0x18) ||
				(packet->payload[4] == 0x0b && packet->payload[6] == 0x80) ||
				(packet->payload[4] == 0x03 && packet->payload[6] == 0x40) ||
				(packet->payload[4] == 0x06 && packet->payload[6] == 0x4e)) {

				ipoque_int_xbox_add_connection(ipoque_struct);
				IPQ_LOG(IPOQUE_PROTOCOL_XBOX, ipoque_struct, IPQ_LOG_DEBUG, "xbox udp connection detected\n");
				return;
			}
		}
		if ((dport == 3074 || sport == 3074)
			&& ((packet->payload_packet_len == 24 && packet->payload[0] == 0x00)
				|| (packet->payload_packet_len == 42 && packet->payload[0] == 0x4f && packet->payload[2] == 0x0a)
				|| (packet->payload_packet_len == 80 && ntohs(get_u16(packet->payload, 0)) == 0x50bc
					&& packet->payload[2] == 0x45)
				|| (packet->payload_packet_len == 40 && ntohl(get_u32(packet->payload, 0)) == 0xcf5f3202)
				|| (packet->payload_packet_len == 38 && ntohl(get_u32(packet->payload, 0)) == 0xc1457f03)
				|| (packet->payload_packet_len == 28 && ntohl(get_u32(packet->payload, 0)) == 0x015f2c00))) {
			if (flow->l4.udp.xbox_stage == 1) {
				ipoque_int_xbox_add_connection(ipoque_struct);
				IPQ_LOG(IPOQUE_PROTOCOL_XBOX, ipoque_struct, IPQ_LOG_DEBUG, "xbox udp connection detected\n");
				return;
			}
			IPQ_LOG(IPOQUE_PROTOCOL_XBOX, ipoque_struct, IPQ_LOG_DEBUG, "maybe xbox.\n");
			flow->l4.udp.xbox_stage++;
			return;
		}

		/* exclude here all non matched udp traffic, exclude here tcp only if http has been excluded, because xbox could use http */
		if (packet->tcp == NULL
#ifdef IPOQUE_PROTOCOL_HTTP
			|| IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_HTTP) != 0
#endif
			) {
			IPQ_LOG(IPOQUE_PROTOCOL_XBOX, ipoque_struct, IPQ_LOG_DEBUG, "xbox udp excluded.\n");
			IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_XBOX);
		}
	}
	/* to not exclude tcp traffic here, done by http code... */
}

#endif
