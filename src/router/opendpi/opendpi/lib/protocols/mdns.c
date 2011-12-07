/*
 * mdns.c
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

#ifdef IPOQUE_PROTOCOL_MDNS

#define IPOQUE_MAX_MDNS_REQUESTS                        128

/*
This module should detect MDNS
*/

static void ipoque_int_mdns_add_connection(struct ipoque_detection_module_struct
										   *ipoque_struct)
{
	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_MDNS, IPOQUE_REAL_PROTOCOL);
}

static int ipoque_int_check_mdns_payload(struct ipoque_detection_module_struct
										 *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;

	if ((packet->payload[2] & 0x80) == 0 &&
		ntohs(get_u16(packet->payload, 4)) <= IPOQUE_MAX_MDNS_REQUESTS &&
		ntohs(get_u16(packet->payload, 6)) <= IPOQUE_MAX_MDNS_REQUESTS) {

		IPQ_LOG(IPOQUE_PROTOCOL_MDNS, ipoque_struct, IPQ_LOG_DEBUG, "found MDNS with question query.\n");

		return 1;
	} else if ((packet->payload[2] & 0x80) != 0 &&
			   ntohs(get_u16(packet->payload, 4)) == 0 &&
			   ntohs(get_u16(packet->payload, 6)) <= IPOQUE_MAX_MDNS_REQUESTS &&
			   ntohs(get_u16(packet->payload, 6)) != 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_MDNS, ipoque_struct, IPQ_LOG_DEBUG, "found MDNS with answer query.\n");

		return 1;
	}

	return 0;
}

void ipoque_search_mdns(struct ipoque_detection_module_struct *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
//      struct ipoque_id_struct         *src=ipoque_struct->src;
//      struct ipoque_id_struct         *dst=ipoque_struct->dst;

	u16 dport;
//      const u16 sport=ntohs(packet->udp->source);

	/* check if UDP and */
	if (packet->udp != NULL) {
		/*read destination port */
		dport = ntohs(packet->udp->dest);

		IPQ_LOG(IPOQUE_PROTOCOL_MDNS, ipoque_struct, IPQ_LOG_DEBUG, "MDNS udp start \n");



		/*check standard MDNS to port 5353 */
		/*took this information from http://www.it-administrator.de/lexikon/multicast-dns.html */

		if (dport == 5353 && packet->payload_packet_len >= 12) {

			IPQ_LOG(IPOQUE_PROTOCOL_MDNS, ipoque_struct, IPQ_LOG_DEBUG, "found MDNS with destination port 5353\n");

			/* MDNS header is similar to dns header */
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
			   * dns query check: query: QR set, ancount = 0, nscount = 0, QDCOUNT < MAX_MDNS, ARCOUNT < MAX_MDNS
			   *
			 */

			/* mdns protocol must have destination address  224.0.0.251 */
			/* took this information from http://www.it-administrator.de/lexikon/multicast-dns.html */

			if (packet->iph != NULL && ntohl(packet->iph->daddr) == 0xe00000fb) {

				IPQ_LOG(IPOQUE_PROTOCOL_MDNS, ipoque_struct,
						IPQ_LOG_DEBUG, "found MDNS with destination address 224.0.0.251 (=0xe00000fb)\n");

				if (ipoque_int_check_mdns_payload(ipoque_struct) == 1) {
					ipoque_int_mdns_add_connection(ipoque_struct);
					return;
				}
			}
#ifdef IPOQUE_DETECTION_SUPPORT_IPV6
			if (packet->iphv6 != NULL) {
				const u32 *daddr = packet->iphv6->daddr.ipq_v6_u.u6_addr32;
				if (daddr[0] == htonl(0xff020000) && daddr[1] == 0 && daddr[2] == 0 && daddr[3] == htonl(0xfb)) {

					IPQ_LOG(IPOQUE_PROTOCOL_MDNS, ipoque_struct,
							IPQ_LOG_DEBUG, "found MDNS with destination address ff02::fb\n");

					if (ipoque_int_check_mdns_payload(ipoque_struct) == 1) {
						ipoque_int_mdns_add_connection(ipoque_struct);
						return;
					}
				}
			}
#endif

		}
	}
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_MDNS);
}
#endif
