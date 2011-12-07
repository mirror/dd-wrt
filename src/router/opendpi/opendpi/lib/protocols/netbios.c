/*
 * netbios.c
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

#ifdef IPOQUE_PROTOCOL_NETBIOS

static void ipoque_int_netbios_add_connection(struct ipoque_detection_module_struct
											  *ipoque_struct)
{
	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_NETBIOS, IPOQUE_REAL_PROTOCOL);
}


void ipoque_search_netbios(struct ipoque_detection_module_struct *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
//      struct ipoque_id_struct         *src=ipoque_struct->src;
//      struct ipoque_id_struct         *dst=ipoque_struct->dst;


	u16 dport;

	if (packet->udp != NULL) {
		dport = ntohs(packet->udp->dest);

		IPQ_LOG(IPOQUE_PROTOCOL_NETBIOS, ipoque_struct, IPQ_LOG_DEBUG, "netbios udp start\n");

		/*check standard NETBIOS over udp to port 137 */
		if ((dport == 137 || 0) && packet->payload_packet_len >= 50) {

			IPQ_LOG(IPOQUE_PROTOCOL_NETBIOS, ipoque_struct,
					IPQ_LOG_DEBUG, "found netbios port 137 and payload_packet_len 50\n");


			if (ntohs(get_u16(packet->payload, 2)) == 0 &&
				ntohs(get_u16(packet->payload, 4)) == 1 &&
				ntohs(get_u16(packet->payload, 6)) == 0 &&
				ntohs(get_u16(packet->payload, 8)) == 0 && ntohs(get_u16(packet->payload, 10)) == 0) {

				IPQ_LOG(IPOQUE_PROTOCOL_NETBIOS, ipoque_struct,
						IPQ_LOG_DEBUG, "found netbios with questions = 1 and answers = 0, authority = 0  \n");

				ipoque_int_netbios_add_connection(ipoque_struct);
				return;
			}
			if (packet->payload[2] == 0x80 &&
				ntohs(get_u16(packet->payload, 4)) == 1 &&
				ntohs(get_u16(packet->payload, 6)) == 0 &&
				ntohs(get_u16(packet->payload, 8)) == 0 && ntohs(get_u16(packet->payload, 10)) == 1) {

				IPQ_LOG(IPOQUE_PROTOCOL_NETBIOS, ipoque_struct,
						IPQ_LOG_DEBUG, "found netbios with questions = 1 and answers, authority, additional = 0  \n");

				ipoque_int_netbios_add_connection(ipoque_struct);
				return;
			}
			if (ntohs(get_u16(packet->payload, 2)) == 0x4000 &&
				ntohs(get_u16(packet->payload, 4)) == 1 &&
				ntohs(get_u16(packet->payload, 6)) == 0 &&
				ntohs(get_u16(packet->payload, 8)) == 0 && ntohs(get_u16(packet->payload, 10)) == 1) {

				IPQ_LOG(IPOQUE_PROTOCOL_NETBIOS, ipoque_struct,
						IPQ_LOG_DEBUG, "found netbios with questions = 1 and answers = 0, authority = 0  \n");

				ipoque_int_netbios_add_connection(ipoque_struct);
				return;
			}
			if (ntohs(get_u16(packet->payload, 2)) == 0x8400 &&
				ntohs(get_u16(packet->payload, 4)) == 0 &&
				ntohs(get_u16(packet->payload, 6)) == 1 &&
				ntohs(get_u16(packet->payload, 8)) == 0 && ntohs(get_u16(packet->payload, 10)) == 0) {

				IPQ_LOG(IPOQUE_PROTOCOL_NETBIOS, ipoque_struct,
						IPQ_LOG_DEBUG,
						"found netbios with flag 8400 questions = 0 and answers = 1, authority, additional = 0  \n");

				ipoque_int_netbios_add_connection(ipoque_struct);
				return;
			}
			if (ntohs(get_u16(packet->payload, 2)) == 0x8500 &&
				ntohs(get_u16(packet->payload, 4)) == 0 &&
				ntohs(get_u16(packet->payload, 6)) == 1 &&
				ntohs(get_u16(packet->payload, 8)) == 0 && ntohs(get_u16(packet->payload, 10)) == 0) {

				IPQ_LOG(IPOQUE_PROTOCOL_NETBIOS, ipoque_struct,
						IPQ_LOG_DEBUG,
						"found netbios with flag 8500 questions = 0 and answers = 1, authority, additional = 0  \n");

				ipoque_int_netbios_add_connection(ipoque_struct);
				return;
			}
			if (ntohs(get_u16(packet->payload, 2)) == 0x2910 &&
				ntohs(get_u16(packet->payload, 4)) == 1 &&
				ntohs(get_u16(packet->payload, 6)) == 0 &&
				ntohs(get_u16(packet->payload, 8)) == 0 && ntohs(get_u16(packet->payload, 10)) == 1) {

				IPQ_LOG(IPOQUE_PROTOCOL_NETBIOS, ipoque_struct,
						IPQ_LOG_DEBUG,
						"found netbios with flag 2910, questions = 1 and answers, authority=0, additional = 1  \n");

				ipoque_int_netbios_add_connection(ipoque_struct);
				return;
			}
			if (ntohs(get_u16(packet->payload, 2)) == 0xAD86 &&
				ntohs(get_u16(packet->payload, 4)) == 0 &&
				ntohs(get_u16(packet->payload, 6)) == 1 &&
				ntohs(get_u16(packet->payload, 8)) == 0 && ntohs(get_u16(packet->payload, 10)) == 0) {

				IPQ_LOG(IPOQUE_PROTOCOL_NETBIOS, ipoque_struct,
						IPQ_LOG_DEBUG,
						"found netbios with flag ad86 questions = 0 and answers = 1, authority, additional = 0  \n");

				ipoque_int_netbios_add_connection(ipoque_struct);
				return;
			}
			if (ntohs(get_u16(packet->payload, 2)) == 0x0110 &&
				ntohs(get_u16(packet->payload, 4)) == 1 &&
				ntohs(get_u16(packet->payload, 6)) == 0 &&
				ntohs(get_u16(packet->payload, 8)) == 0 && ntohs(get_u16(packet->payload, 10)) == 0) {

				IPQ_LOG(IPOQUE_PROTOCOL_NETBIOS, ipoque_struct,
						IPQ_LOG_DEBUG,
						"found netbios with flag 0110 questions = 1 and answers = 0, authority, additional = 0  \n");

				ipoque_int_netbios_add_connection(ipoque_struct);
				return;
			}

			if ((ntohs(get_u16(packet->payload, 2)) & 0xf800) == 0) {

				IPQ_LOG(IPOQUE_PROTOCOL_NETBIOS, ipoque_struct, IPQ_LOG_DEBUG, "possible netbios name query request\n");

				if (get_u16(packet->payload, 4) == htons(1) &&
					get_u16(packet->payload, 6) == 0 &&
					get_u16(packet->payload, 8) == 0 && get_u16(packet->payload, 10) == 0) {

					/* name is encoded as described in rfc883 */
					u8 name_length = packet->payload[12];

					IPQ_LOG(IPOQUE_PROTOCOL_NETBIOS, ipoque_struct, IPQ_LOG_DEBUG,
							"possible netbios name query request, one question\n");

					if (packet->payload_packet_len == 12 + 1 + name_length + 1 + 2 + 2) {

						IPQ_LOG(IPOQUE_PROTOCOL_NETBIOS, ipoque_struct, IPQ_LOG_DEBUG,
								"possible netbios name query request, length matches\n");

						/* null terminated? */
						if (packet->payload[12 + name_length + 1] == 0 &&
							get_u16(packet->payload, 12 + name_length + 2) == htons(0x0020) &&
							get_u16(packet->payload, 12 + name_length + 4) == htons(0x0001)) {

							IPQ_LOG(IPOQUE_PROTOCOL_NETBIOS, ipoque_struct, IPQ_LOG_DEBUG,
									"found netbios name query request\n");
							ipoque_int_netbios_add_connection(ipoque_struct);
							return;
						}
					}
				}
			} else if ((ntohs(get_u16(packet->payload, 2)) & 0xf800) == 0x8000) {

				IPQ_LOG(IPOQUE_PROTOCOL_NETBIOS, ipoque_struct, IPQ_LOG_DEBUG,
						"possible netbios name query response\n");

				if (get_u16(packet->payload, 4) == 0 &&
					get_u16(packet->payload, 6) == htons(1) &&
					get_u16(packet->payload, 8) == 0 && get_u16(packet->payload, 10) == 0) {

					/* name is encoded as described in rfc883 */
					u8 name_length = packet->payload[12];

					IPQ_LOG(IPOQUE_PROTOCOL_NETBIOS, ipoque_struct, IPQ_LOG_DEBUG,
							"possible netbios positive name query response, one answer\n");

					if (packet->payload_packet_len >= 12 + 1 + name_length + 1 + 2 + 2) {

						IPQ_LOG(IPOQUE_PROTOCOL_NETBIOS, ipoque_struct, IPQ_LOG_DEBUG,
								"possible netbios name query response, length matches\n");

						/* null terminated? */
						if (packet->payload[12 + name_length + 1] == 0 &&
							get_u16(packet->payload, 12 + name_length + 2) == htons(0x0020) &&
							get_u16(packet->payload, 12 + name_length + 4) == htons(0x0001)) {

							IPQ_LOG(IPOQUE_PROTOCOL_NETBIOS, ipoque_struct, IPQ_LOG_DEBUG,
									"found netbios name query response\n");
							ipoque_int_netbios_add_connection(ipoque_struct);
							return;
						}
					}
				} else if (get_u16(packet->payload, 4) == 0 &&
						   get_u16(packet->payload, 6) == 0 &&
						   get_u16(packet->payload, 8) == 0 && get_u16(packet->payload, 10) == 0) {

					/* name is encoded as described in rfc883 */
					u8 name_length = packet->payload[12];

					IPQ_LOG(IPOQUE_PROTOCOL_NETBIOS, ipoque_struct, IPQ_LOG_DEBUG,
							"possible netbios negative name query response, one answer\n");

					if (packet->payload_packet_len >= 12 + 1 + name_length + 1 + 2 + 2) {

						IPQ_LOG(IPOQUE_PROTOCOL_NETBIOS, ipoque_struct, IPQ_LOG_DEBUG,
								"possible netbios name query response, length matches\n");

						/* null terminated? */
						if (packet->payload[12 + name_length + 1] == 0 &&
							get_u16(packet->payload, 12 + name_length + 2) == htons(0x000A) &&
							get_u16(packet->payload, 12 + name_length + 4) == htons(0x0001)) {

							IPQ_LOG(IPOQUE_PROTOCOL_NETBIOS, ipoque_struct, IPQ_LOG_DEBUG,
									"found netbios name query response\n");
							ipoque_int_netbios_add_connection(ipoque_struct);
							return;
						}
					}
				} else if (get_u16(packet->payload, 4) == 0 &&
						   get_u16(packet->payload, 6) == 0 &&
						   get_u16(packet->payload, 8) == htons(1) && get_u16(packet->payload, 10) == htons(1)) {

					/* name is encoded as described in rfc883 */
					u8 name_length = packet->payload[12];

					IPQ_LOG(IPOQUE_PROTOCOL_NETBIOS, ipoque_struct, IPQ_LOG_DEBUG,
							"possible netbios redirect name query response, one answer\n");

					if (packet->payload_packet_len >= 12 + 1 + name_length + 1 + 2 + 2) {

						IPQ_LOG(IPOQUE_PROTOCOL_NETBIOS, ipoque_struct, IPQ_LOG_DEBUG,
								"possible netbios name query response, length matches\n");

						/* null terminated? */
						if (packet->payload[12 + name_length + 1] == 0 &&
							get_u16(packet->payload, 12 + name_length + 2) == htons(0x0002) &&
							get_u16(packet->payload, 12 + name_length + 4) == htons(0x0001)) {

							IPQ_LOG(IPOQUE_PROTOCOL_NETBIOS, ipoque_struct, IPQ_LOG_DEBUG,
									"found netbios name query response\n");
							ipoque_int_netbios_add_connection(ipoque_struct);
							return;
						}
					}
				}
			}
			/* TODO: extend according to rfc1002 */
		}



		/*check standard NETBIOS over udp to port 138 */

		/*netbios header token from http://www.protocolbase.net/protocols/protocol_NBDGM.php */

		if ((dport == 138 ||
			 0) &&
			packet->payload_packet_len >= 14 &&
			ntohs(get_u16(packet->payload, 10)) == packet->payload_packet_len - 14) {

			IPQ_LOG(IPOQUE_PROTOCOL_NETBIOS, ipoque_struct,
					IPQ_LOG_DEBUG, "found netbios port 138 and payload length >= 112 \n");


			if (packet->payload[0] >= 0x11 && packet->payload[0] <= 0x16) {

				IPQ_LOG(IPOQUE_PROTOCOL_NETBIOS, ipoque_struct,
						IPQ_LOG_DEBUG, "found netbios with MSG-type 0x11,0x12,0x13,0x14,0x15 or 0x16\n");




				if (ntohl(get_u32(packet->payload, 4)) == ntohl(packet->iph->saddr)) {
					IPQ_LOG(IPOQUE_PROTOCOL_NETBIOS, ipoque_struct,
							IPQ_LOG_DEBUG, "found netbios with checked ip-address.\n");

					ipoque_int_netbios_add_connection(ipoque_struct);
					return;
				}
			}
		}
	}

	if (packet->tcp != NULL) {
		dport = ntohs(packet->tcp->dest);

		IPQ_LOG(IPOQUE_PROTOCOL_NETBIOS, ipoque_struct, IPQ_LOG_DEBUG, "netbios tcp start\n");

		/* destination port must be 139 */
		if (dport == 139) {

			IPQ_LOG(IPOQUE_PROTOCOL_NETBIOS, ipoque_struct, IPQ_LOG_DEBUG, "found netbios with destination port 139\n");

			/* payload_packet_len must be 72 */
			if (packet->payload_packet_len == 72) {
				IPQ_LOG(IPOQUE_PROTOCOL_NETBIOS, ipoque_struct,
						IPQ_LOG_DEBUG, "found netbios with payload_packen_len = 72. \n");

				if (packet->payload[0] == 0x81 && packet->payload[1] == 0 && ntohs(get_u16(packet->payload, 2)) == 68) {
					IPQ_LOG(IPOQUE_PROTOCOL_NETBIOS, ipoque_struct,
							IPQ_LOG_DEBUG,
							"found netbios with session request = 81, flags=0 and length od following bytes = 68. \n");

					ipoque_int_netbios_add_connection(ipoque_struct);
					return;
				}
			}
		}

	}

	IPQ_LOG(IPOQUE_PROTOCOL_NETBIOS, ipoque_struct, IPQ_LOG_DEBUG, "exclude netbios\n");
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_NETBIOS);

}
#endif
