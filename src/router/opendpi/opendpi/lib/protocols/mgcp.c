/*
 * mgcp.c
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

#ifdef IPOQUE_PROTOCOL_MGCP

static void ipoque_int_mgcp_add_connection(struct ipoque_detection_module_struct
										   *ipoque_struct)
{
	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_MGCP, IPOQUE_REAL_PROTOCOL);
}


	
#if !(defined(HAVE_NTOP) && defined(WIN32))
 static inline
#else
__forceinline static
#endif
	 void ipoque_search_mgcp_connection(struct ipoque_detection_module_struct
												 *ipoque_struct)
{

	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
//      struct ipoque_id_struct         *src=ipoque_struct->src;
//      struct ipoque_id_struct         *dst=ipoque_struct->dst;

	/* information about MGCP taken from http://en.wikipedia.org/wiki/MGCP */

	u16 pos = 4;

	if (packet->payload_packet_len < 8) {
		goto mgcp_excluded;
	}

	/* packet must end with 0x0d0a or with 0x0a */
	if (packet->payload[packet->payload_packet_len - 1] != 0x0a
		&& get_u16(packet->payload, packet->payload_packet_len - 2) != htons(0x0d0a)) {
		goto mgcp_excluded;
	}



	if (packet->payload[0] != 'A' && packet->payload[0] != 'C' && packet->payload[0] != 'D' &&
		packet->payload[0] != 'E' && packet->payload[0] != 'M' && packet->payload[0] != 'N' &&
		packet->payload[0] != 'R') {
		goto mgcp_excluded;
	}
	if (memcmp(packet->payload, "AUEP ", 5) != 0 && memcmp(packet->payload, "AUCX ", 5) != 0 &&
		memcmp(packet->payload, "CRCX ", 5) != 0 && memcmp(packet->payload, "DLCX ", 5) != 0 &&
		memcmp(packet->payload, "EPCF ", 5) != 0 && memcmp(packet->payload, "MDCX ", 5) != 0 &&
		memcmp(packet->payload, "NTFY ", 5) != 0 && memcmp(packet->payload, "RQNT ", 5) != 0 &&
		memcmp(packet->payload, "RSIP ", 5) != 0) {
		goto mgcp_excluded;
	}
	// now search for string "MGCP " in the rest of the message
	while ((pos + 5) < packet->payload_packet_len) {
		if (memcmp(&packet->payload[pos], "MGCP ", 5) == 0) {
			IPQ_LOG(IPOQUE_PROTOCOL_MGCP, ipoque_struct, IPQ_LOG_DEBUG, "MGCP match.\n");
			ipoque_int_mgcp_add_connection(ipoque_struct);
			return;
		}
		pos++;
	}

  mgcp_excluded:
	IPQ_LOG(IPOQUE_PROTOCOL_MGCP, ipoque_struct, IPQ_LOG_DEBUG, "exclude MGCP.\n");
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_MGCP);
}


void ipoque_search_mgcp(struct ipoque_detection_module_struct *ipoque_struct)
{

	ipoque_search_mgcp_connection(ipoque_struct);

}
#endif
