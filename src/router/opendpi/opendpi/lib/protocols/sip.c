/*
 * sip.c
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

#ifdef IPOQUE_PROTOCOL_SIP
static void ipoque_int_sip_add_connection(struct ipoque_detection_module_struct
										  *ipoque_struct)
{

	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
	struct ipoque_id_struct *src = ipoque_struct->src;
	struct ipoque_id_struct *dst = ipoque_struct->dst;

	flow->detected_protocol = IPOQUE_PROTOCOL_SIP;
	packet->detected_protocol = IPOQUE_PROTOCOL_SIP;
	if (src != NULL) {
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(src->detected_protocol_bitmask, IPOQUE_PROTOCOL_SIP);
	}

	if (dst != NULL) {
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(dst->detected_protocol_bitmask, IPOQUE_PROTOCOL_SIP);
	}
}


static inline void ipoque_search_sip_handshake(struct ipoque_detection_module_struct
											   *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
//      struct ipoque_id_struct         *src=ipoque_struct->src;
//      struct ipoque_id_struct         *dst=ipoque_struct->dst;
#ifndef IPOQUE_PROTOCOL_YAHOO
	if (packet->payload_packet_len >= 14
		&& packet->payload[packet->payload_packet_len - 2] == 0x0d
		&& packet->payload[packet->payload_packet_len - 1] == 0x0a)
#endif
#ifdef IPOQUE_PROTOCOL_YAHOO
		if (packet->payload_packet_len >= 14)
#endif
		{
			if ((memcmp(packet->payload, "REGISTER ", 9) == 0 || memcmp(packet->payload, "register ", 9) == 0)
				&& (memcmp(&packet->payload[9], "SIP:", 4) == 0 || memcmp(&packet->payload[9], "sip:", 4) == 0)) {
				IPQ_LOG(IPOQUE_PROTOCOL_SIP, ipoque_struct, IPQ_LOG_DEBUG, "found sip REGISTER.\n");
				ipoque_int_sip_add_connection(ipoque_struct);
				return;
			}
			if ((memcmp(packet->payload, "INVITE ", 7) == 0 || memcmp(packet->payload, "invite ", 7) == 0)
				&& (memcmp(&packet->payload[7], "SIP:", 4) == 0 || memcmp(&packet->payload[7], "sip:", 4) == 0)) {
				IPQ_LOG(IPOQUE_PROTOCOL_SIP, ipoque_struct, IPQ_LOG_DEBUG, "found sip INVITE.\n");
				ipoque_int_sip_add_connection(ipoque_struct);
				return;
			}
			/* seen this in second direction on the third position,
			 * maybe it could be deleted, if somebody sees it in the first direction,
			 * please delete this comment.
			 */
			if (memcmp(packet->payload, "SIP/2.0 200 OK", 14) == 0
				|| memcmp(packet->payload, "sip/2.0 200 OK", 14) == 0) {
				IPQ_LOG(IPOQUE_PROTOCOL_SIP, ipoque_struct, IPQ_LOG_DEBUG, "found sip SIP/2.0 0K.\n");
				ipoque_int_sip_add_connection(ipoque_struct);
				return;
			}
		}

	/* add bitmask for tcp only, some stupid udp programs
	 * send a very few (< 10 ) packets before invite (mostly a 0x0a0x0d, but just search the first 3 payload_packets here */
	if (packet->udp != NULL && flow->packet_counter < 10) {
		IPQ_LOG(IPOQUE_PROTOCOL_SIP, ipoque_struct, IPQ_LOG_DEBUG, "need next packet.\n");
		return;
	}

	if (packet->payload_packet_len == 4 && get_u32(packet->payload, 0) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_SIP, ipoque_struct, IPQ_LOG_DEBUG, "maybe sip. need next packet.\n");
		return;
	}
#ifdef IPOQUE_PROTOCOL_YAHOO
	if (packet->payload_packet_len > 30 && packet->payload[0] == 0x90
		&& packet->payload[3] == packet->payload_packet_len - 20 && get_u32(packet->payload, 4) == 0
		&& get_u32(packet->payload, 8) == 0) {
		flow->sip_yahoo_voice = 1;
		IPQ_LOG(IPOQUE_PROTOCOL_SIP, ipoque_struct, IPQ_LOG_DEBUG, "maybe sip yahoo. need next packet.\n");
	}
	if (flow->sip_yahoo_voice && flow->packet_counter < 10) {
		return;
	}
#endif
	IPQ_LOG(IPOQUE_PROTOCOL_SIP, ipoque_struct, IPQ_LOG_DEBUG, "exclude sip.\n");
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_SIP);
	return;


}

void ipoque_search_sip(struct ipoque_detection_module_struct *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
//  struct ipoque_flow_struct   *flow = ipoque_struct->flow;
//      struct ipoque_id_struct         *src=ipoque_struct->src;
//      struct ipoque_id_struct         *dst=ipoque_struct->dst;

	IPQ_LOG(IPOQUE_PROTOCOL_SIP, ipoque_struct, IPQ_LOG_DEBUG, "sip detection...\n");

	/* skip marked packets */
	if (packet->detected_protocol != IPOQUE_PROTOCOL_SIP) {
		if (packet->tcp_retransmission == 0) {
			ipoque_search_sip_handshake(ipoque_struct);
		}
	}
}

#endif
