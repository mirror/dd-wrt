/*
 * tvuplayer.c
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
#ifdef IPOQUE_PROTOCOL_TVUPLAYER


static void ipoque_int_tvuplayer_add_connection(struct ipoque_detection_module_struct
												*ipoque_struct, ipoque_protocol_type_t protocol_type)
{
	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_TVUPLAYER, protocol_type);
}

void ipoque_search_tvuplayer(struct ipoque_detection_module_struct
							 *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;

//      struct ipoque_id_struct         *src=ipoque_struct->src;
//      struct ipoque_id_struct         *dst=ipoque_struct->dst;

	IPQ_LOG(IPOQUE_PROTOCOL_TVUPLAYER, ipoque_struct, IPQ_LOG_DEBUG, "search tvuplayer.  \n");



	if (packet->tcp != NULL) {
		if ((packet->payload_packet_len == 36 || packet->payload_packet_len == 24)
			&& packet->payload[0] == 0x00
			&& ntohl(get_u32(packet->payload, 2)) == 0x31323334
			&& ntohl(get_u32(packet->payload, 6)) == 0x35363837 && packet->payload[10] == 0x01) {
			IPQ_LOG(IPOQUE_PROTOCOL_TVUPLAYER, ipoque_struct, IPQ_LOG_DEBUG, "found tvuplayer over tcp.  \n");
			ipoque_int_tvuplayer_add_connection(ipoque_struct, IPOQUE_REAL_PROTOCOL);
			return;
		}

		if (packet->payload_packet_len >= 50) {

			if (memcmp(packet->payload, "POST", 4) || memcmp(packet->payload, "GET", 3)) {
				IPQ_PARSE_PACKET_LINE_INFO(ipoque_struct, packet);
				if (packet->user_agent_line.ptr != NULL &&
					packet->user_agent_line.len >= 8 && (memcmp(packet->user_agent_line.ptr, "MacTVUP", 7) == 0)) {
					IPQ_LOG(IPOQUE_PROTOCOL_TVUPLAYER, ipoque_struct, IPQ_LOG_DEBUG, "Found user agent as MacTVUP.\n");
					ipoque_int_tvuplayer_add_connection(ipoque_struct, IPOQUE_CORRELATED_PROTOCOL);
					return;
				}
			}
		}
	}

	if (packet->udp != NULL) {

		if (packet->payload_packet_len == 56 &&
			packet->payload[0] == 0xff
			&& packet->payload[1] == 0xff && packet->payload[2] == 0x00
			&& packet->payload[3] == 0x01
			&& packet->payload[12] == 0x02 && packet->payload[13] == 0xff
			&& packet->payload[19] == 0x2c && ((packet->payload[26] == 0x05 && packet->payload[27] == 0x14)
											   || (packet->payload[26] == 0x14 && packet->payload[27] == 0x05))) {
			IPQ_LOG(IPOQUE_PROTOCOL_TVUPLAYER, ipoque_struct, IPQ_LOG_DEBUG, "found tvuplayer pattern type I.  \n");
			ipoque_int_tvuplayer_add_connection(ipoque_struct, IPOQUE_REAL_PROTOCOL);
			return;
		}
		if (packet->payload_packet_len == 82
			&& packet->payload[0] == 0x00 && packet->payload[2] == 0x00
			&& packet->payload[10] == 0x00 && packet->payload[11] == 0x00
			&& packet->payload[12] == 0x01 && packet->payload[13] == 0xff
			&& packet->payload[19] == 0x14 && packet->payload[32] == 0x03
			&& packet->payload[33] == 0xff && packet->payload[34] == 0x01
			&& packet->payload[39] == 0x32 && ((packet->payload[46] == 0x05 && packet->payload[47] == 0x14)
											   || (packet->payload[46] == 0x14 && packet->payload[47] == 0x05))) {
			IPQ_LOG(IPOQUE_PROTOCOL_TVUPLAYER, ipoque_struct, IPQ_LOG_DEBUG, "found tvuplayer pattern type II.  \n");
			ipoque_int_tvuplayer_add_connection(ipoque_struct, IPOQUE_REAL_PROTOCOL);
			return;
		}
		if (packet->payload_packet_len == 32
			&& packet->payload[0] == 0x00 && packet->payload[2] == 0x00
			&& (packet->payload[10] == 0x00 || packet->payload[10] == 0x65
				|| packet->payload[10] == 0x7e || packet->payload[10] == 0x49)
			&& (packet->payload[11] == 0x00 || packet->payload[11] == 0x57
				|| packet->payload[11] == 0x06 || packet->payload[11] == 0x22)
			&& packet->payload[12] == 0x01 && (packet->payload[13] == 0xff || packet->payload[13] == 0x01)
			&& packet->payload[19] == 0x14) {
			IPQ_LOG(IPOQUE_PROTOCOL_TVUPLAYER, ipoque_struct, IPQ_LOG_DEBUG, "found tvuplayer pattern type III.  \n");
			ipoque_int_tvuplayer_add_connection(ipoque_struct, IPOQUE_REAL_PROTOCOL);
			return;
		}
		if (packet->payload_packet_len == 84
			&& packet->payload[0] == 0x00 && packet->payload[2] == 0x00
			&& packet->payload[10] == 0x00 && packet->payload[11] == 0x00
			&& packet->payload[12] == 0x01 && packet->payload[13] == 0xff
			&& packet->payload[19] == 0x14 && packet->payload[32] == 0x03
			&& packet->payload[33] == 0xff && packet->payload[34] == 0x01 && packet->payload[39] == 0x34) {
			IPQ_LOG(IPOQUE_PROTOCOL_TVUPLAYER, ipoque_struct, IPQ_LOG_DEBUG, "found tvuplayer pattern type IV.  \n");
			ipoque_int_tvuplayer_add_connection(ipoque_struct, IPOQUE_REAL_PROTOCOL);
			return;
		}
		if (packet->payload_packet_len == 102
			&& packet->payload[0] == 0x00 && packet->payload[2] == 0x00
			&& packet->payload[10] == 0x00 && packet->payload[11] == 0x00
			&& packet->payload[12] == 0x01 && packet->payload[13] == 0xff
			&& packet->payload[19] == 0x14 && packet->payload[33] == 0xff && packet->payload[39] == 0x14) {
			IPQ_LOG(IPOQUE_PROTOCOL_TVUPLAYER, ipoque_struct, IPQ_LOG_DEBUG, "found tvuplayer pattern type V.  \n");
			ipoque_int_tvuplayer_add_connection(ipoque_struct, IPOQUE_REAL_PROTOCOL);
			return;
		}
		if (packet->payload_packet_len == 62 && packet->payload[0] == 0x00 && packet->payload[2] == 0x00
			//&& packet->payload[10] == 0x00 && packet->payload[11] == 0x00
			&& packet->payload[12] == 0x03 && packet->payload[13] == 0xff
			&& packet->payload[19] == 0x32 && ((packet->payload[26] == 0x05 && packet->payload[27] == 0x14)
											   || (packet->payload[26] == 0x14 && packet->payload[27] == 0x05))) {
			IPQ_LOG(IPOQUE_PROTOCOL_TVUPLAYER, ipoque_struct, IPQ_LOG_DEBUG, "found tvuplayer pattern type VI.  \n");
			ipoque_int_tvuplayer_add_connection(ipoque_struct, IPOQUE_REAL_PROTOCOL);
			return;
		}
		// to check, if byte 26, 27, 33,39 match
		if (packet->payload_packet_len == 60
			&& packet->payload[0] == 0x00 && packet->payload[2] == 0x00
			&& packet->payload[10] == 0x00 && packet->payload[11] == 0x00
			&& packet->payload[12] == 0x06 && packet->payload[13] == 0x00 && packet->payload[19] == 0x30) {
			IPQ_LOG(IPOQUE_PROTOCOL_TVUPLAYER, ipoque_struct, IPQ_LOG_DEBUG, "found tvuplayer pattern type VII.  \n");
			ipoque_int_tvuplayer_add_connection(ipoque_struct, IPOQUE_REAL_PROTOCOL);
			return;
		}
	}



	IPQ_LOG(IPOQUE_PROTOCOL_TVUPLAYER, ipoque_struct, IPQ_LOG_DEBUG, "exclude tvuplayer.  \n");
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_TVUPLAYER);

}
#endif
