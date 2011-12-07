/*
 * mysql.c
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
#ifdef IPOQUE_PROTOCOL_MYSQL

static void ipoque_int_mysql_add_connection(struct ipoque_detection_module_struct
											*ipoque_struct)
{
	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_MYSQL, IPOQUE_REAL_PROTOCOL);
}

void ipoque_search_mysql_tcp(struct ipoque_detection_module_struct *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
//      struct ipoque_id_struct         *src=ipoque_struct->src;
//      struct ipoque_id_struct         *dst=ipoque_struct->dst;

	if (packet->payload_packet_len > 37	//min length
		&& get_u16(packet->payload, 0) == packet->payload_packet_len - 4	//first 3 bytes are length
		&& get_u8(packet->payload, 2) == 0x00	//3rd byte of packet length
		&& get_u8(packet->payload, 3) == 0x00	//packet sequence number is 0 for startup packet
		&& get_u8(packet->payload, 5) > 0x30	//server version > 0
		&& get_u8(packet->payload, 5) < 0x37	//server version < 7
		&& get_u8(packet->payload, 6) == 0x2e	//dot
		) {
		u32 a;
		for (a = 7; a + 31 < packet->payload_packet_len; a++) {
			if (packet->payload[a] == 0x00) {
				if (get_u8(packet->payload, a + 13) == 0x00	//filler byte
					&& get_u64(packet->payload, a + 19) == 0x0ULL	//13 more
					&& get_u32(packet->payload, a + 27) == 0x0	//filler bytes
					&& get_u8(packet->payload, a + 31) == 0x0) {
					IPQ_LOG(IPOQUE_PROTOCOL_MYSQL, ipoque_struct, IPQ_LOG_DEBUG, "MySQL detected.\n");
					ipoque_int_mysql_add_connection(ipoque_struct);
					return;
				}
				break;
			}
		}
	}

	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_MYSQL);

}

#endif
