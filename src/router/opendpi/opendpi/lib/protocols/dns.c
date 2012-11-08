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


#include "ndpi_protocols.h"

#ifdef NDPI_PROTOCOL_DNS

/*
This module should detect DNS
*/

static void ndpi_int_dns_add_connection(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{
  ndpi_int_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_DNS, NDPI_REAL_PROTOCOL);
}


static void ndpi_search_dns(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{
	struct ndpi_packet_struct *packet = &flow->packet;
	
//      struct ndpi_id_struct         *src=ndpi_struct->src;
//      struct ndpi_id_struct         *dst=ndpi_struct->dst;


	u_int16_t dport = 0;

#define NDPI_MAX_DNS_REQUESTS			16

	NDPI_LOG(NDPI_PROTOCOL_DNS, ndpi_struct, NDPI_LOG_DEBUG, "search DNS.\n");


	if (packet->udp != NULL) {
		//      const u_int16_t sport=ntohs(packet->udp->source);
		dport = ntohs(packet->udp->dest);
		NDPI_LOG(NDPI_PROTOCOL_DNS, ndpi_struct, NDPI_LOG_DEBUG, "calculated dport over UDP.\n");
	}
	if (packet->tcp != NULL) {
		//      const u_int16_t sport=ntohs(packet->tcp->source);
		dport = ntohs(packet->tcp->dest);
		NDPI_LOG(NDPI_PROTOCOL_DNS, ndpi_struct, NDPI_LOG_DEBUG, "calculated dport over tcp.\n");
	}

	/*check standard DNS to port 53 */
	if (dport == 53 && packet->payload_packet_len >= 12) {

		NDPI_LOG(NDPI_PROTOCOL_DNS, ndpi_struct, NDPI_LOG_DEBUG, "dport==53, packet-payload-packet-len>=12.\n");

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
			 ntohs(get_u_int16_t(packet->payload, 4)) <= NDPI_MAX_DNS_REQUESTS &&
			 ntohs(get_u_int16_t(packet->payload, 4)) != 0 &&
			 ntohs(get_u_int16_t(packet->payload, 6)) == 0 &&
			 ntohs(get_u_int16_t(packet->payload, 8)) == 0 && ntohs(get_u_int16_t(packet->payload, 10)) <= NDPI_MAX_DNS_REQUESTS)
			||
			((ntohs(get_u_int16_t(packet->payload, 0)) == packet->payload_packet_len - 2) &&
			 (packet->payload[4] & 0x80) == 0 &&
			 ntohs(get_u_int16_t(packet->payload, 6)) <= NDPI_MAX_DNS_REQUESTS &&
			 ntohs(get_u_int16_t(packet->payload, 6)) != 0 &&
			 ntohs(get_u_int16_t(packet->payload, 8)) == 0 &&
			 ntohs(get_u_int16_t(packet->payload, 10)) == 0 &&
			 packet->payload_packet_len >= 14 && ntohs(get_u_int16_t(packet->payload, 12)) <= NDPI_MAX_DNS_REQUESTS)) {

			NDPI_LOG(NDPI_PROTOCOL_DNS, ndpi_struct, NDPI_LOG_DEBUG, "found DNS.\n");

			ndpi_int_dns_add_connection(ndpi_struct, flow);
			return;
		}
	}

	NDPI_LOG(NDPI_PROTOCOL_DNS, ndpi_struct, NDPI_LOG_DEBUG, "exclude DNS.\n");
	NDPI_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, NDPI_PROTOCOL_DNS);

}
#endif
