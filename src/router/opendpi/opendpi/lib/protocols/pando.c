/*
 * pando.c
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

#ifdef IPOQUE_PROTOCOL_PANDO

static void ipoque_int_pando_add_connection(struct ipoque_detection_module_struct
											*ipoque_struct)
{
	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_PANDO, IPOQUE_REAL_PROTOCOL);
}

	
#if !(defined(HAVE_NTOP) && defined(WIN32))
 static inline
#else
__forceinline static
#endif
	 u8 search_pando(struct ipoque_detection_module_struct *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
//      struct ipoque_flow_struct       *flow=ipoque_struct->flow;
//      struct ipoque_id_struct         *src=ipoque_struct->src;
//      struct ipoque_id_struct         *dst=ipoque_struct->dst;

	if (packet->tcp != NULL) {

		if (packet->payload_packet_len == 63 && memcmp(&packet->payload[1], "Pando protocol", 14) == 0) {
			IPQ_LOG(IPOQUE_PROTOCOL_PANDO, ipoque_struct, IPQ_LOG_DEBUG, "Pando download detected\n");
			goto end_pando_found;
		}

	} else if (packet->udp != NULL) {
		if (packet->payload_packet_len > 20
			&& packet->payload_packet_len < 100
			&& packet->payload[0] == 0x00
			&& packet->payload[1] == 0x00
			&& packet->payload[2] == 0x00
			&& packet->payload[3] == 0x09 && packet->payload[4] == 0x00 && packet->payload[5] == 0x00) {
			// bypass the detection because one packet has at a specific place the word Pando in it
			if (packet->payload_packet_len == 87 && memcmp(&packet->payload[25], "Pando protocol", 14) == 0) {
				IPQ_LOG(IPOQUE_PROTOCOL_PANDO, ipoque_struct, IPQ_LOG_DEBUG,
						"Pando UDP packet detected --> Pando in payload\n");
				goto end_pando_found;
			} else if (packet->payload_packet_len == 92 && memcmp(&packet->payload[72], "Pando", 5) == 0) {
				IPQ_LOG(IPOQUE_PROTOCOL_PANDO, ipoque_struct, IPQ_LOG_DEBUG,
						"Pando UDP packet detected --> Pando in payload\n");
				goto end_pando_found;
			}
			goto end_pando_maybe_found;
		}
	}

	goto end_pando_nothing_found;

  end_pando_found:
	ipoque_int_pando_add_connection(ipoque_struct);
	return 1;

  end_pando_maybe_found:
	return 2;

  end_pando_nothing_found:
	return 0;
}

void ipoque_search_pando_tcp_udp(struct ipoque_detection_module_struct
								 *ipoque_struct)
{
//      struct ipoque_packet_struct     *packet=&ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
//      struct ipoque_id_struct         *src=ipoque_struct->src;
//      struct ipoque_id_struct         *dst=ipoque_struct->dst;

	if (search_pando(ipoque_struct) != 0)
		return;

	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_PANDO);

}
#endif
