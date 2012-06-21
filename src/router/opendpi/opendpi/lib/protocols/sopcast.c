/*
 * sopcast.c
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

#ifdef IPOQUE_PROTOCOL_SOPCAST


static void ipoque_int_sopcast_add_connection(struct ipoque_detection_module_struct
											  *ipoque_struct)
{
	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_SOPCAST, IPOQUE_REAL_PROTOCOL);
}

/**
 * this function checks for sopcast tcp pattern
 *
 * NOTE: if you add more patterns please keep the number of if levels
 * low, it is already complex enough
 */
	
#if !(defined(HAVE_NTOP) && defined(WIN32))
 static inline
#else
__forceinline static
#endif
	 u8 ipoque_int_is_sopcast_tcp(const u8 * payload, const u16 payload_len)
{
	if (payload_len != 54)
		return 0;

	if (payload[2] != payload[3] - 4 && payload[2] != payload[3] + 4)
		return 0;

	if (payload[2] != payload[4] - 1 && payload[2] != payload[4] + 1)
		return 0;

	if (payload[25] != payload[25 + 16 - 1] + 1 && payload[25] != payload[25 + 16 - 1] - 1) {

		if (payload[3] != payload[25] &&
			payload[3] != payload[25] - 4 && payload[3] != payload[25] + 4 && payload[3] != payload[25] - 21) {
			return 0;
		}
	}

	if (payload[4] != payload[28] ||
		payload[28] != payload[30] ||
		payload[30] != payload[31] ||
		get_u16(payload, 30) != get_u16(payload, 32) || get_u16(payload, 32) != get_u16(payload, 34)) {

		if ((payload[2] != payload[5] - 1 && payload[2] != payload[5] + 1) ||
			payload[2] != payload[25] ||
			payload[4] != payload[28] ||
			payload[4] != payload[31] ||
			payload[4] != payload[32] ||
			payload[4] != payload[33] ||
			payload[4] != payload[34] ||
			payload[4] != payload[35] || payload[4] != payload[30] || payload[2] != payload[36]) {
			return 0;
		}
	}

	if (payload[42] != payload[53])
		return 0;

	if (payload[45] != payload[46] + 1 && payload[45] != payload[46] - 1)
		return 0;

	if (payload[45] != payload[49] || payload[46] != payload[50] || payload[47] != payload[51])
		return 0;

	return 1;
}

static void ipoque_search_sopcast_tcp(struct ipoque_detection_module_struct
									  *ipoque_struct)
{

	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
	if (flow->packet_counter == 1 && packet->payload_packet_len == 54 && get_u16(packet->payload, 0) == ntohs(0x0036)) {
		if (ipoque_int_is_sopcast_tcp(packet->payload, packet->payload_packet_len)) {
			IPQ_LOG(IPOQUE_PROTOCOL_SOPCAST, ipoque_struct, IPQ_LOG_DEBUG, "found sopcast TCP \n");
			ipoque_int_sopcast_add_connection(ipoque_struct);
			return;
		}
	}

	IPQ_LOG(IPOQUE_PROTOCOL_SOPCAST, ipoque_struct, IPQ_LOG_DEBUG, "exclude sopcast TCP.  \n");
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_SOPCAST);


}

static void ipoque_search_sopcast_udp(struct ipoque_detection_module_struct
									  *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;

//      struct ipoque_id_struct         *src=ipoque_struct->src;
//      struct ipoque_id_struct         *dst=ipoque_struct->dst;

	IPQ_LOG(IPOQUE_PROTOCOL_SOPCAST, ipoque_struct, IPQ_LOG_DEBUG, "search sopcast.  \n");


	if (packet->payload_packet_len == 52 && packet->payload[0] == 0xff
		&& packet->payload[1] == 0xff && packet->payload[2] == 0x01
		&& packet->payload[8] == 0x02 && packet->payload[9] == 0xff
		&& packet->payload[10] == 0x00 && packet->payload[11] == 0x2c
		&& packet->payload[12] == 0x00 && packet->payload[13] == 0x00 && packet->payload[14] == 0x00) {
		IPQ_LOG(IPOQUE_PROTOCOL_SOPCAST, ipoque_struct, IPQ_LOG_DEBUG, "found sopcast with if I.  \n");
		ipoque_int_sopcast_add_connection(ipoque_struct);
		return;
	}
	if ((packet->payload_packet_len == 80 || packet->payload_packet_len == 28 || packet->payload_packet_len == 94)
		&& packet->payload[0] == 0x00 && (packet->payload[2] == 0x02 || packet->payload[2] == 0x01)
		&& packet->payload[8] == 0x01 && packet->payload[9] == 0xff
		&& packet->payload[10] == 0x00 && packet->payload[11] == 0x14
		&& packet->payload[12] == 0x00 && packet->payload[13] == 0x00) {
		IPQ_LOG(IPOQUE_PROTOCOL_SOPCAST, ipoque_struct, IPQ_LOG_DEBUG, "found sopcast with if II.  \n");
		ipoque_int_sopcast_add_connection(ipoque_struct);
		return;
	}
	/* this case has been seen once. Please revome this comment, if you see it another time */
	if (packet->payload_packet_len == 60 && packet->payload[0] == 0x00
		&& packet->payload[2] == 0x01
		&& packet->payload[8] == 0x03 && packet->payload[9] == 0xff
		&& packet->payload[10] == 0x00 && packet->payload[11] == 0x34
		&& packet->payload[12] == 0x00 && packet->payload[13] == 0x00 && packet->payload[14] == 0x00) {
		IPQ_LOG(IPOQUE_PROTOCOL_SOPCAST, ipoque_struct, IPQ_LOG_DEBUG, "found sopcast with if III.  \n");
		ipoque_int_sopcast_add_connection(ipoque_struct);
		return;
	}
	if (packet->payload_packet_len == 42 && packet->payload[0] == 0x00
		&& packet->payload[1] == 0x02 && packet->payload[2] == 0x01
		&& packet->payload[3] == 0x07 && packet->payload[4] == 0x03
		&& packet->payload[8] == 0x06
		&& packet->payload[9] == 0x01 && packet->payload[10] == 0x00
		&& packet->payload[11] == 0x22 && packet->payload[12] == 0x00 && packet->payload[13] == 0x00) {
		IPQ_LOG(IPOQUE_PROTOCOL_SOPCAST, ipoque_struct, IPQ_LOG_DEBUG, "found sopcast with if IV.  \n");
		ipoque_int_sopcast_add_connection(ipoque_struct);
		return;
	}
	if (packet->payload_packet_len == 28 && packet->payload[0] == 0x00
		&& packet->payload[1] == 0x0c && packet->payload[2] == 0x01
		&& packet->payload[3] == 0x07 && packet->payload[4] == 0x00
		&& packet->payload[8] == 0x01
		&& packet->payload[9] == 0x01 && packet->payload[10] == 0x00
		&& packet->payload[11] == 0x14 && packet->payload[12] == 0x00 && packet->payload[13] == 0x00) {
		IPQ_LOG(IPOQUE_PROTOCOL_SOPCAST, ipoque_struct, IPQ_LOG_DEBUG, "found sopcast with if V.  \n");
		ipoque_int_sopcast_add_connection(ipoque_struct);
		return;
	}
	/* this case has been seen once. Please revome this comment, if you see it another time */
	if (packet->payload_packet_len == 286 && packet->payload[0] == 0x00
		&& packet->payload[1] == 0x02 && packet->payload[2] == 0x01
		&& packet->payload[3] == 0x07 && packet->payload[4] == 0x03
		&& packet->payload[8] == 0x06
		&& packet->payload[9] == 0x01 && packet->payload[10] == 0x01
		&& packet->payload[11] == 0x16 && packet->payload[12] == 0x00 && packet->payload[13] == 0x00) {
		IPQ_LOG(IPOQUE_PROTOCOL_SOPCAST, ipoque_struct, IPQ_LOG_DEBUG, "found sopcast with if VI.  \n");
		ipoque_int_sopcast_add_connection(ipoque_struct);
		return;
	}
	if (packet->payload_packet_len == 76 && packet->payload[0] == 0xff
		&& packet->payload[1] == 0xff && packet->payload[2] == 0x01
		&& packet->payload[8] == 0x0c && packet->payload[9] == 0xff
		&& packet->payload[10] == 0x00 && packet->payload[11] == 0x44
		&& packet->payload[16] == 0x01 && packet->payload[15] == 0x01
		&& packet->payload[12] == 0x00 && packet->payload[13] == 0x00 && packet->payload[14] == 0x00) {
		IPQ_LOG(IPOQUE_PROTOCOL_SOPCAST, ipoque_struct, IPQ_LOG_DEBUG, "found sopcast with if VII.  \n");
		ipoque_int_sopcast_add_connection(ipoque_struct);
		return;
	}

	/* Attention please: no asymmetric detection necessary. This detection works asymmetrically as well. */

	IPQ_LOG(IPOQUE_PROTOCOL_SOPCAST, ipoque_struct, IPQ_LOG_DEBUG, "exclude sopcast.  \n");
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_SOPCAST);



}

void ipoque_search_sopcast(struct ipoque_detection_module_struct
						   *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;

	if (packet->udp != NULL)
		ipoque_search_sopcast_udp(ipoque_struct);
	if (packet->tcp != NULL)
		ipoque_search_sopcast_tcp(ipoque_struct);

}
#endif
