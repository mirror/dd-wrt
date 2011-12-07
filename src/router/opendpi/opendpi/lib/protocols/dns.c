/*
 * dns.c
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

#ifdef IPOQUE_PROTOCOL_DNS

/*
This module should detect DNS
*/

static void ipoque_int_dns_add_connection(struct ipoque_detection_module_struct
										  *ipoque_struct)
{

	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_DNS, IPOQUE_REAL_PROTOCOL);
}


void ipoque_search_dns(struct ipoque_detection_module_struct *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
//      struct ipoque_id_struct         *src=ipoque_struct->src;
//      struct ipoque_id_struct         *dst=ipoque_struct->dst;


	u16 dport = 0;

#define IPOQUE_MAX_DNS_REQUESTS			16

	IPQ_LOG(IPOQUE_PROTOCOL_DNS, ipoque_struct, IPQ_LOG_DEBUG, "search DNS.\n");


	if (packet->udp != NULL) {
		//      const u16 sport=ntohs(packet->udp->source);
		dport = ntohs(packet->udp->dest);
		IPQ_LOG(IPOQUE_PROTOCOL_DNS, ipoque_struct, IPQ_LOG_DEBUG, "calculated dport over UDP.\n");
	}
	if (packet->tcp != NULL) {
		//      const u16 sport=ntohs(packet->tcp->source);
		dport = ntohs(packet->tcp->dest);
		IPQ_LOG(IPOQUE_PROTOCOL_DNS, ipoque_struct, IPQ_LOG_DEBUG, "calculated dport over tcp.\n");
	}

	/*check standard DNS to port 53 */
	if (dport == 53 && packet->payload_packet_len >= 12) {

		IPQ_LOG(IPOQUE_PROTOCOL_DNS, ipoque_struct, IPQ_LOG_DEBUG, "dport==53, packet-payload-packet-len>=12.\n");

		/* dns header
		   0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
		   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
		   |                      ID                       |
		   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
		   |QR|   Opcode  |AA|TC|RD|RA|   Z    |   RCODE   |
		   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
		   |                    QDCOUNT                    |
		   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
		   |                    ANCOUNT                    |
		   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
		   |                    NSCOUNT                    |
		   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
		   |                    ARCOUNT                    |
		   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
		   *
		   * dns query check: query: QR set, ancount = 0, nscount = 0, QDCOUNT < MAX_DNS, ARCOUNT < MAX_DNS
		   *
		 */

		if (((packet->payload[2] & 0x80) == 0 &&
			 ntohs(get_u16(packet->payload, 4)) <= IPOQUE_MAX_DNS_REQUESTS &&
			 ntohs(get_u16(packet->payload, 4)) != 0 &&
			 ntohs(get_u16(packet->payload, 6)) == 0 &&
			 ntohs(get_u16(packet->payload, 8)) == 0 && ntohs(get_u16(packet->payload, 10)) <= IPOQUE_MAX_DNS_REQUESTS)
			||
			((ntohs(get_u16(packet->payload, 0)) == packet->payload_packet_len - 2) &&
			 (packet->payload[4] & 0x80) == 0 &&
			 ntohs(get_u16(packet->payload, 6)) <= IPOQUE_MAX_DNS_REQUESTS &&
			 ntohs(get_u16(packet->payload, 6)) != 0 &&
			 ntohs(get_u16(packet->payload, 8)) == 0 &&
			 ntohs(get_u16(packet->payload, 10)) == 0 &&
			 packet->payload_packet_len >= 14 && ntohs(get_u16(packet->payload, 12)) <= IPOQUE_MAX_DNS_REQUESTS)) {

			IPQ_LOG(IPOQUE_PROTOCOL_DNS, ipoque_struct, IPQ_LOG_DEBUG, "found DNS.\n");

			ipoque_int_dns_add_connection(ipoque_struct);
			return;
		}
	}

	IPQ_LOG(IPOQUE_PROTOCOL_DNS, ipoque_struct, IPQ_LOG_DEBUG, "exclude DNS.\n");
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_DNS);

}
#endif
