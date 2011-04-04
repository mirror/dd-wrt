/*
 * secondlife.c
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
#ifdef IPOQUE_PROTOCOL_SECONDLIFE

static void ipoque_int_secondlife_add_connection(struct ipoque_detection_module_struct
												 *ipoque_struct)
{

	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
	struct ipoque_id_struct *src = ipoque_struct->src;
	struct ipoque_id_struct *dst = ipoque_struct->dst;

	flow->detected_protocol = IPOQUE_PROTOCOL_SECONDLIFE;
	packet->detected_protocol = IPOQUE_PROTOCOL_SECONDLIFE;

	if (src != NULL) {
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(src->detected_protocol_bitmask, IPOQUE_PROTOCOL_SECONDLIFE);
	}
	if (dst != NULL) {
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(dst->detected_protocol_bitmask, IPOQUE_PROTOCOL_SECONDLIFE);
	}
}

void ipoque_search_secondlife(struct ipoque_detection_module_struct
							  *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
//      struct ipoque_id_struct         *src=ipoque_struct->src;
//      struct ipoque_id_struct         *dst=ipoque_struct->dst;

//  if ((ntohs(packet->udp->dest) == 12035 || ntohs(packet->udp->dest) == 12036 || (ntohs(packet->udp->dest) >= 13000 && ntohs(packet->udp->dest) <= 13050))    //port
//      && packet->payload_packet_len > 6   // min length with no extra header, high frequency and 1 byte message body
//      && get_u8(packet->payload, 0) == 0x40   // reliable packet
//      && ntohl(get_u32(packet->payload, 1)) == 0x00000001 // sequence number equals 1
//      //ntohl (get_u32 (packet->payload, 5)) == 0x00FFFF00      // no extra header, low frequency message - can't use, message may have higher frequency
//      ) {
//      IPQ_LOG(IPOQUE_PROTOCOL_SECONDLIFE, ipoque_struct, IPQ_LOG_DEBUG, "Second Life detected.\n");
//      ipoque_int_secondlife_add_connection(ipoque_struct);
//      return;
//  }

	if (packet->payload_packet_len == 46
		&& memcmp(packet->payload, "\x40\x00\x00\x00\x01\x00\xff\xff\x00\x03", 10) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_SECONDLIFE, ipoque_struct, IPQ_LOG_DEBUG, "Second Life 0xffff0003 detected.\n");
		ipoque_int_secondlife_add_connection(ipoque_struct);
		return;
	}
	if (packet->payload_packet_len == 54
		&& memcmp(packet->payload, "\x40\x00\x00\x00\x01\x00\xff\xff\x00\x52", 10) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_SECONDLIFE, ipoque_struct, IPQ_LOG_DEBUG, "Second Life 0xffff0052 detected.\n");
		ipoque_int_secondlife_add_connection(ipoque_struct);
		return;
	}
	if (packet->payload_packet_len > 54 && memcmp(packet->payload, "\x40\x00\x00\x00\x01\x00\x08", 7) == 0 &&
		get_u32(packet->payload, packet->payload_packet_len - 4) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_SECONDLIFE, ipoque_struct, IPQ_LOG_DEBUG, "Second Life 0x08 detected.\n");
		ipoque_int_secondlife_add_connection(ipoque_struct);
		return;
	}


	IPQ_LOG(IPOQUE_PROTOCOL_SECONDLIFE, ipoque_struct, IPQ_LOG_DEBUG, "Second Life excluded.\n");
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_SECONDLIFE);
}

#endif
