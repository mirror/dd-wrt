/*
 * sopcast.c
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

#ifdef IPOQUE_PROTOCOL_SOPCAST


static void ipoque_int_sopcast_add_connection(struct ipoque_detection_module_struct
											  *ipoque_struct)
{

	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
	struct ipoque_id_struct *src = ipoque_struct->src;
	struct ipoque_id_struct *dst = ipoque_struct->dst;

	flow->detected_protocol = IPOQUE_PROTOCOL_SOPCAST;
	packet->detected_protocol = IPOQUE_PROTOCOL_SOPCAST;

	if (src != NULL) {
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(src->detected_protocol_bitmask, IPOQUE_PROTOCOL_SOPCAST);
	}
	if (dst != NULL) {
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(dst->detected_protocol_bitmask, IPOQUE_PROTOCOL_SOPCAST);
	}
}


static void ipoque_search_sopcast_tcp(struct ipoque_detection_module_struct
									  *ipoque_struct)
{

	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
	if (flow->packet_counter == 1 && packet->payload_packet_len == 54 && get_u16(packet->payload, 0) == ntohs(0x0036)) {
		if ((packet->payload[2] == packet->payload[3] - 4 || packet->payload[2] == packet->payload[3] + 4)
			&& (packet->payload[2] == packet->payload[4] - 1 || packet->payload[2] == packet->payload[4] + 1)
			&& packet->payload[4] == packet->payload[5]
			&& packet->payload[3] == packet->payload[25]

			&& packet->payload[4] == packet->payload[28]
			&& packet->payload[28] == packet->payload[30]
			&& packet->payload[30] == packet->payload[31]
			&& get_u16(packet->payload, 30) == get_u16(packet->payload, 32)
			&& get_u16(packet->payload, 32) == get_u16(packet->payload, 34)

			&& packet->payload[42] == packet->payload[53]
			&& (packet->payload[45] == packet->payload[46] + 1 || packet->payload[45] == packet->payload[46] - 1)
			&& packet->payload[45] == packet->payload[49] && packet->payload[46] == packet->payload[50]
			&& packet->payload[47] == packet->payload[51]
			) {
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
