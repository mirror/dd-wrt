/*
 * ssdp.c
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
#ifdef IPOQUE_PROTOCOL_SSDP


static void ipoque_int_ssdp_add_connection(struct ipoque_detection_module_struct
										   *ipoque_struct)
{
	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_SSDP, IPOQUE_REAL_PROTOCOL);
}

/* this detection also works asymmetrically */
void ipoque_search_ssdp(struct ipoque_detection_module_struct *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
//      struct ipoque_id_struct         *src=ipoque_struct->src;
//      struct ipoque_id_struct         *dst=ipoque_struct->dst;

	IPQ_LOG(IPOQUE_PROTOCOL_SSDP, ipoque_struct, IPQ_LOG_DEBUG, "search ssdp.\n");
	if (packet->udp != NULL) {

		if (packet->payload_packet_len > 100) {
			if ((memcmp(packet->payload, "M-SEARCH * HTTP/1.1", 19) == 0)
				|| memcmp(packet->payload, "NOTIFY * HTTP/1.1", 17) == 0) {


				IPQ_LOG(IPOQUE_PROTOCOL_SSDP, ipoque_struct, IPQ_LOG_DEBUG, "found ssdp.\n");
				ipoque_int_ssdp_add_connection(ipoque_struct);
				return;
			}

#ifdef HAVE_NTOP
#define SSDP_HTTP "HTTP/1.1 200 OK\r\n"
			if(memcmp(packet->payload, SSDP_HTTP, strlen(SSDP_HTTP)) == 0) {
			  IPQ_LOG(IPOQUE_PROTOCOL_SSDP, ipoque_struct, IPQ_LOG_DEBUG, "found ssdp.\n");
			  ipoque_int_ssdp_add_connection(ipoque_struct);
			  return;
			}
#endif
		}
	}

	IPQ_LOG(IPOQUE_PROTOCOL_SSDP, ipoque_struct, IPQ_LOG_DEBUG, "ssdp excluded.\n");
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_SSDP);
}

#endif
