/*
 * kerberos.c
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



/* include files */

#include "ipq_protocols.h"
#ifdef IPOQUE_PROTOCOL_KERBEROS

static void ipoque_int_kerberos_add_connection(struct ipoque_detection_module_struct
											   *ipoque_struct)
{
	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_KERBEROS, IPOQUE_REAL_PROTOCOL);
}


void ipoque_search_kerberos(struct ipoque_detection_module_struct
							*ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
//      struct ipoque_id_struct         *src=ipoque_struct->src;
//      struct ipoque_id_struct         *dst=ipoque_struct->dst;


	/* I have observed 0a,0c,0d,0e at packet->payload[19/21], maybe there are other possibilities */
	if (packet->payload_packet_len >= 4 && ntohl(get_u32(packet->payload, 0)) == packet->payload_packet_len - 4) {
		if (packet->payload_packet_len > 19 &&
			packet->payload[14] == 0x05 &&
			(packet->payload[19] == 0x0a ||
			 packet->payload[19] == 0x0c || packet->payload[19] == 0x0d || packet->payload[19] == 0x0e)) {
			IPQ_LOG(IPOQUE_PROTOCOL_KERBEROS, ipoque_struct, IPQ_LOG_DEBUG, "found KERBEROS\n");
			ipoque_int_kerberos_add_connection(ipoque_struct);
			return;

		}
		if (packet->payload_packet_len > 21 &&
			packet->payload[16] == 0x05 &&
			(packet->payload[21] == 0x0a ||
			 packet->payload[21] == 0x0c || packet->payload[21] == 0x0d || packet->payload[21] == 0x0e)) {
			IPQ_LOG(IPOQUE_PROTOCOL_KERBEROS, ipoque_struct, IPQ_LOG_DEBUG, "found KERBEROS\n");
			ipoque_int_kerberos_add_connection(ipoque_struct);
			return;

		}



	}








	IPQ_LOG(IPOQUE_PROTOCOL_KERBEROS, ipoque_struct, IPQ_LOG_DEBUG, "no KERBEROS detected.\n");
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_KERBEROS);
}

#endif
