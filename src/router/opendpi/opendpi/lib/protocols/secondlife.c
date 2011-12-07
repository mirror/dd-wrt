/*
 * secondlife.c
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


#include "ipq_utils.h"
#ifdef IPOQUE_PROTOCOL_SECONDLIFE

static void ipoque_int_secondlife_add_connection(struct ipoque_detection_module_struct
												 *ipoque_struct, ipoque_protocol_type_t protocol_type)
{
	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_SECONDLIFE, protocol_type);
}

void ipoque_search_secondlife(struct ipoque_detection_module_struct
							  *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
//      struct ipoque_id_struct         *src=ipoque_struct->src;
//      struct ipoque_id_struct         *dst=ipoque_struct->dst;

//  if ((ntohs(packet->udp->dest) == 12035 || ntohs(packet->udp->dest) == 12036 || (ntohs(packet->udp->dest) >= 13000 && ntohs(packet->udp->dest) <= 13050))    //port
//      && packet->payload_packet_len > 6   // min length with no extra header, high frequency and 1 byte message body
//      && get_u8(packet->payload, 0) == 0x40   // reliable packet
//      && ntohl(get_u32(packet->payload, 1)) == 0x00000001 // sequence number equals 1
//      //ntohl (get_u32 (packet->payload, 5)) == 0x00FFFF00      // no extra header, low frequency message - can't use, message may have higher frequency
//      ) {
//      IPQ_LOG(IPOQUE_PROTOCOL_SECONDLIFE, ipoque_struct, IPQ_LOG_DEBUG, "Second Life detected.\n");
//      ipoque_int_secondlife_add_connection(ipoque_struct);
//      return;
//  }

	if (packet->tcp != NULL) {
		if (packet->payload_packet_len > IPQ_STATICSTRING_LEN("GET /")
			&& memcmp(packet->payload, "GET /", IPQ_STATICSTRING_LEN("GET /")) == 0) {
			IPQ_LOG(IPOQUE_PROTOCOL_SECONDLIFE, ipoque_struct, IPQ_LOG_DEBUG, "Second Life HTTP 'GET /'' found.\n");
			ipq_parse_packet_line_info(ipoque_struct);
			if (packet->user_agent_line.ptr != NULL
				&& packet->user_agent_line.len >
				IPQ_STATICSTRING_LEN
				("Mozilla/5.0 (Windows; U; Windows NT 6.1; de-DE) AppleWebKit/532.4 (KHTML, like Gecko) SecondLife/")
				&& memcmp(&packet->user_agent_line.ptr[IPQ_STATICSTRING_LEN
													   ("Mozilla/5.0 (Windows; U; Windows NT 6.1; de-DE) AppleWebKit/532.4 (KHTML, like Gecko) ")],
						  "SecondLife/", IPQ_STATICSTRING_LEN("SecondLife/")) == 0) {
				IPQ_LOG(IPOQUE_PROTOCOL_SECONDLIFE, ipoque_struct, IPQ_LOG_DEBUG,
						"Second Life TCP HTTP User Agent detected.\n");
				ipoque_int_secondlife_add_connection(ipoque_struct, IPOQUE_CORRELATED_PROTOCOL);
				return;
			}
			if (packet->host_line.ptr != NULL && packet->host_line.len > IPQ_STATICSTRING_LEN(".agni.lindenlab.com:")) {
				u8 x;
				for (x = 2; x < 6; x++) {
					if (packet->host_line.ptr[packet->host_line.len - (1 + x)] == ':') {
						if ((1 + x + IPQ_STATICSTRING_LEN(".agni.lindenlab.com")) < packet->host_line.len
							&& memcmp(&packet->host_line.ptr[packet->host_line.len -
															 (1 + x + IPQ_STATICSTRING_LEN(".agni.lindenlab.com"))],
									  ".agni.lindenlab.com", IPQ_STATICSTRING_LEN(".agni.lindenlab.com")) == 0) {
							IPQ_LOG(IPOQUE_PROTOCOL_SECONDLIFE, ipoque_struct, IPQ_LOG_DEBUG,
									"Second Life TCP HTTP Host detected.\n");
							ipoque_int_secondlife_add_connection(ipoque_struct, IPOQUE_CORRELATED_PROTOCOL);
							return;
						}
						break;
					}
				}
			}
		}
	}
	if (packet->udp != NULL) {
		if (packet->payload_packet_len == 46
			&& memcmp(packet->payload, "\x40\x00\x00\x00\x01\x00\xff\xff\x00\x03", 10) == 0) {
			IPQ_LOG(IPOQUE_PROTOCOL_SECONDLIFE, ipoque_struct, IPQ_LOG_DEBUG, "Second Life 0xffff0003 detected.\n");
			ipoque_int_secondlife_add_connection(ipoque_struct, IPOQUE_REAL_PROTOCOL);
			return;
		}
		if (packet->payload_packet_len == 54
			&& memcmp(packet->payload, "\x40\x00\x00\x00\x01\x00\xff\xff\x00\x52", 10) == 0) {
			IPQ_LOG(IPOQUE_PROTOCOL_SECONDLIFE, ipoque_struct, IPQ_LOG_DEBUG, "Second Life 0xffff0052 detected.\n");
			ipoque_int_secondlife_add_connection(ipoque_struct, IPOQUE_REAL_PROTOCOL);
			return;
		}
		if (packet->payload_packet_len == 58
			&& memcmp(packet->payload, "\x40\x00\x00\x00\x01\x00\xff\xff\x00\xa9", 10) == 0) {
			IPQ_LOG(IPOQUE_PROTOCOL_SECONDLIFE, ipoque_struct, IPQ_LOG_DEBUG, "Second Life 0xffff00a9 detected.\n");
			ipoque_int_secondlife_add_connection(ipoque_struct, IPOQUE_REAL_PROTOCOL);
			return;
		}
		if (packet->payload_packet_len > 54 && memcmp(packet->payload, "\x40\x00\x00\x00\x01\x00\x08", 7) == 0 &&
			get_u32(packet->payload, packet->payload_packet_len - 4) == 0) {
			IPQ_LOG(IPOQUE_PROTOCOL_SECONDLIFE, ipoque_struct, IPQ_LOG_DEBUG, "Second Life 0x08 detected.\n");
			ipoque_int_secondlife_add_connection(ipoque_struct, IPOQUE_REAL_PROTOCOL);
			return;
		}
	}


	IPQ_LOG(IPOQUE_PROTOCOL_SECONDLIFE, ipoque_struct, IPQ_LOG_DEBUG, "Second Life excluded.\n");
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_SECONDLIFE);
}

#endif
