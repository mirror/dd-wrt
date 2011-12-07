/*
 * veohtv.c
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
#include "ipq_utils.h"
#ifdef IPOQUE_PROTOCOL_HTTP_APPLICATION_VEOHTV

static void ipoque_int_veohtv_add_connection(struct ipoque_detection_module_struct
											 *ipoque_struct, ipoque_protocol_type_t protocol_type)
{
	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_HTTP_APPLICATION_VEOHTV, protocol_type);
}

void ipoque_search_veohtv_tcp(struct ipoque_detection_module_struct
							  *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
//      struct ipoque_id_struct         *src=ipoque_struct->src;
//      struct ipoque_id_struct         *dst=ipoque_struct->dst;

	if (packet->detected_protocol_stack[0] == IPOQUE_PROTOCOL_HTTP_APPLICATION_VEOHTV)
		return;

	if (flow->l4.tcp.veoh_tv_stage == 1 || flow->l4.tcp.veoh_tv_stage == 2) {
		if (packet->packet_direction != flow->setup_packet_direction &&
			packet->payload_packet_len > IPQ_STATICSTRING_LEN("HTTP/1.1 20")
			&& memcmp(packet->payload, "HTTP/1.1 ", IPQ_STATICSTRING_LEN("HTTP/1.1 ")) == 0 &&
			(packet->payload[IPQ_STATICSTRING_LEN("HTTP/1.1 ")] == '2' ||
			 packet->payload[IPQ_STATICSTRING_LEN("HTTP/1.1 ")] == '3' ||
			 packet->payload[IPQ_STATICSTRING_LEN("HTTP/1.1 ")] == '4' ||
			 packet->payload[IPQ_STATICSTRING_LEN("HTTP/1.1 ")] == '5')) {
#ifdef IPOQUE_PROTOCOL_FLASH
			ipq_parse_packet_line_info(ipoque_struct);
			if (packet->detected_protocol_stack[0] == IPOQUE_PROTOCOL_FLASH &&
				packet->server_line.ptr != NULL &&
				packet->server_line.len > IPQ_STATICSTRING_LEN("Veoh-") &&
				memcmp(packet->server_line.ptr, "Veoh-", IPQ_STATICSTRING_LEN("Veoh-")) == 0) {
				IPQ_LOG(IPOQUE_PROTOCOL_HTTP_APPLICATION_VEOHTV, ipoque_struct, IPQ_LOG_DEBUG, "VeohTV detected.\n");
				ipoque_int_veohtv_add_connection(ipoque_struct, IPOQUE_CORRELATED_PROTOCOL);
				return;
			}
#endif
			if (flow->l4.tcp.veoh_tv_stage == 2) {
				IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask,
											   IPOQUE_PROTOCOL_HTTP_APPLICATION_VEOHTV);
				return;
			}
			IPQ_LOG(IPOQUE_PROTOCOL_HTTP_APPLICATION_VEOHTV, ipoque_struct, IPQ_LOG_DEBUG, "VeohTV detected.\n");
			ipoque_int_veohtv_add_connection(ipoque_struct, IPOQUE_CORRELATED_PROTOCOL);
			return;
		} else if (flow->packet_direction_counter[(flow->setup_packet_direction == 1) ? 0 : 1] > 3) {
			if (flow->l4.tcp.veoh_tv_stage == 2) {
				IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask,
											   IPOQUE_PROTOCOL_HTTP_APPLICATION_VEOHTV);
				return;
			}
			IPQ_LOG(IPOQUE_PROTOCOL_HTTP_APPLICATION_VEOHTV, ipoque_struct, IPQ_LOG_DEBUG, "VeohTV detected.\n");
			ipoque_int_veohtv_add_connection(ipoque_struct, IPOQUE_CORRELATED_PROTOCOL);
			return;
		} else {
			if (flow->packet_counter > 10) {
				if (flow->l4.tcp.veoh_tv_stage == 2) {
					IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask,
												   IPOQUE_PROTOCOL_HTTP_APPLICATION_VEOHTV);
					return;
				}
				IPQ_LOG(IPOQUE_PROTOCOL_HTTP_APPLICATION_VEOHTV, ipoque_struct, IPQ_LOG_DEBUG, "VeohTV detected.\n");
				ipoque_int_veohtv_add_connection(ipoque_struct, IPOQUE_CORRELATED_PROTOCOL);
				return;
			}
			return;
		}
	} else if (packet->udp) {
		/* UDP packets from Veoh Client Player
		 *
		 * packet starts with 16 byte random? value
		 * then a 4 byte mode value
		 *   values between 21 and 26 has been seen 
		 * then a 4 byte counter */

		if (packet->payload_packet_len == 28 &&
			get_u32(packet->payload, 16) == htonl(0x00000021) &&
			get_u32(packet->payload, 20) == htonl(0x00000000) && get_u32(packet->payload, 24) == htonl(0x01040000)) {
			IPQ_LOG(IPOQUE_PROTOCOL_HTTP_APPLICATION_VEOHTV, ipoque_struct, IPQ_LOG_DEBUG, "UDP VeohTV found.\n");
			ipoque_int_veohtv_add_connection(ipoque_struct, IPOQUE_REAL_PROTOCOL);
			return;
		}
	}


	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_HTTP_APPLICATION_VEOHTV);
}
#endif
