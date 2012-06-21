/*
 * telnet.c
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
#ifdef IPOQUE_PROTOCOL_TELNET



static void ipoque_int_telnet_add_connection(struct ipoque_detection_module_struct
											 *ipoque_struct)
{
	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_TELNET, IPOQUE_REAL_PROTOCOL);
}

	
#if !(defined(HAVE_NTOP) && defined(WIN32))
 static inline
#else
__forceinline static
#endif
	 u8 search_iac(struct ipoque_detection_module_struct *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;

	u16 a;

	if (packet->payload_packet_len < 3) {
		return 0;
	}

	if (!(packet->payload[0] == 0xff
		  && packet->payload[1] > 0xf9 && packet->payload[1] != 0xff && packet->payload[2] < 0x28)) {
		return 0;
	}

	a = 3;

	while (a < packet->payload_packet_len - 2) {
		// commands start with a 0xff byte followed by a command byte >= 0xf0 and < 0xff
		// command bytes 0xfb to 0xfe are followed by an option byte <= 0x28
		if (!(packet->payload[a] != 0xff ||
			  (packet->payload[a] == 0xff && (packet->payload[a + 1] >= 0xf0) && (packet->payload[a + 1] <= 0xfa)) ||
			  (packet->payload[a] == 0xff && (packet->payload[a + 1] >= 0xfb) && (packet->payload[a + 1] != 0xff)
			   && (packet->payload[a + 2] <= 0x28)))) {
			return 0;
		}
		a++;
	}

	return 1;
}

/* this detection also works asymmetrically */
void ipoque_search_telnet_tcp(struct ipoque_detection_module_struct
							  *ipoque_struct)
{
//  struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
//      struct ipoque_id_struct         *src=ipoque_struct->src;
//      struct ipoque_id_struct         *dst=ipoque_struct->dst;

	IPQ_LOG(IPOQUE_PROTOCOL_TELNET, ipoque_struct, IPQ_LOG_DEBUG, "search telnet.\n");

	if (search_iac(ipoque_struct) == 1) {

		if (flow->l4.tcp.telnet_stage == 2) {
			IPQ_LOG(IPOQUE_PROTOCOL_TELNET, ipoque_struct, IPQ_LOG_DEBUG, "telnet identified.\n");
			ipoque_int_telnet_add_connection(ipoque_struct);
			return;
		}
		flow->l4.tcp.telnet_stage++;
		IPQ_LOG(IPOQUE_PROTOCOL_TELNET, ipoque_struct, IPQ_LOG_DEBUG, "telnet stage %u.\n", flow->l4.tcp.telnet_stage);
		return;
	}

	if ((flow->packet_counter < 12 && flow->l4.tcp.telnet_stage > 0) || flow->packet_counter < 6) {
		return;
	} else {
		IPQ_LOG(IPOQUE_PROTOCOL_TELNET, ipoque_struct, IPQ_LOG_DEBUG, "telnet excluded.\n");
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_TELNET);
	}
	return;
}

#endif
