/*
 * ldap.c
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
#ifdef IPOQUE_PROTOCOL_LDAP

static void ipoque_int_ldap_add_connection(struct ipoque_detection_module_struct
										   *ipoque_struct)
{
	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_LDAP, IPOQUE_REAL_PROTOCOL);
}

void ipoque_search_ldap(struct ipoque_detection_module_struct
						*ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
//      struct ipoque_id_struct         *src=ipoque_struct->src;
//      struct ipoque_id_struct         *dst=ipoque_struct->dst;

//  u16 dport;



	IPQ_LOG(IPOQUE_PROTOCOL_LDAP, ipoque_struct, IPQ_LOG_DEBUG, "search ldap\n");


	if (packet->payload_packet_len >= 14 && packet->payload[0] == 0x30) {

		// simple type
		if (packet->payload[1] == 0x0c && packet->payload_packet_len == 14 &&
			packet->payload[packet->payload_packet_len - 1] == 0x00 && packet->payload[2] == 0x02) {

			if (packet->payload[3] == 0x01 &&
				(packet->payload[5] == 0x60 || packet->payload[5] == 0x61) && packet->payload[6] == 0x07) {
				IPQ_LOG(IPOQUE_PROTOCOL_LDAP, ipoque_struct, IPQ_LOG_DEBUG, "found ldap simple type 1\n");
				ipoque_int_ldap_add_connection(ipoque_struct);
				return;
			}

			if (packet->payload[3] == 0x02 &&
				(packet->payload[6] == 0x60 || packet->payload[6] == 0x61) && packet->payload[7] == 0x07) {
				IPQ_LOG(IPOQUE_PROTOCOL_LDAP, ipoque_struct, IPQ_LOG_DEBUG, "found ldap simple type 2\n");
				ipoque_int_ldap_add_connection(ipoque_struct);
				return;
			}
		}
		// normal type
		if (packet->payload[1] == 0x84 && packet->payload_packet_len >= 0x84 &&
			packet->payload[2] == 0x00 && packet->payload[3] == 0x00 && packet->payload[6] == 0x02) {

			if (packet->payload[7] == 0x01 &&
				(packet->payload[9] == 0x60 || packet->payload[9] == 0x61 || packet->payload[9] == 0x63 ||
				 packet->payload[9] == 0x64) && packet->payload[10] == 0x84) {

				IPQ_LOG(IPOQUE_PROTOCOL_LDAP, ipoque_struct, IPQ_LOG_DEBUG, "found ldap type 1\n");
				ipoque_int_ldap_add_connection(ipoque_struct);
				return;
			}

			if (packet->payload[7] == 0x02 &&
				(packet->payload[10] == 0x60 || packet->payload[10] == 0x61 || packet->payload[10] == 0x63 ||
				 packet->payload[10] == 0x64) && packet->payload[11] == 0x84) {

				IPQ_LOG(IPOQUE_PROTOCOL_LDAP, ipoque_struct, IPQ_LOG_DEBUG, "found ldap type 2\n");
				ipoque_int_ldap_add_connection(ipoque_struct);
				return;
			}
		}
	}


	IPQ_LOG(IPOQUE_PROTOCOL_LDAP, ipoque_struct, IPQ_LOG_DEBUG, "ldap excluded.\n");
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_LDAP);
}

#endif
