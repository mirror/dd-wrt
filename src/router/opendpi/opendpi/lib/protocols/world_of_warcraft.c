/*
 * world_of_warcraft.c
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
#ifdef IPOQUE_PROTOCOL_WORLDOFWARCRAFT


static void ipoque_int_worldofwarcraft_add_connection(struct
													  ipoque_detection_module_struct
													  *ipoque_struct, ipoque_protocol_type_t protocol_type)
{
	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_WORLDOFWARCRAFT, protocol_type);
}

	
#if !(defined(HAVE_NTOP) && defined(WIN32))
 static inline
#else
__forceinline static
#endif
	 u8 ipoque_int_is_wow_port(const u16 port)
{
	if (port == htons(3724) || port == htons(6112) || port == htons(6113) ||
		port == htons(6114) || port == htons(4000) || port == htons(1119)) {
		return 1;
	}
	return 0;
}

void ipoque_search_worldofwarcraft(struct ipoque_detection_module_struct
								   *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
	struct ipoque_id_struct *src = ipoque_struct->src;
	struct ipoque_id_struct *dst = ipoque_struct->dst;

	IPQ_LOG(IPOQUE_PROTOCOL_WORLDOFWARCRAFT, ipoque_struct, IPQ_LOG_DEBUG, "Search World of Warcraft.\n");

	if (packet->tcp != NULL) {
		if ((packet->payload_packet_len > IPQ_STATICSTRING_LEN("POST /") &&
			 memcmp(packet->payload, "POST /", IPQ_STATICSTRING_LEN("POST /")) == 0) ||
			(packet->payload_packet_len > IPQ_STATICSTRING_LEN("GET /") &&
			 memcmp(packet->payload, "GET /", IPQ_STATICSTRING_LEN("GET /")) == 0)) {
			ipq_parse_packet_line_info(ipoque_struct);
			if (packet->user_agent_line.ptr != NULL &&
				packet->user_agent_line.len == IPQ_STATICSTRING_LEN("Blizzard Web Client") &&
				memcmp(packet->user_agent_line.ptr, "Blizzard Web Client",
					   IPQ_STATICSTRING_LEN("Blizzard Web Client")) == 0) {
				ipoque_int_worldofwarcraft_add_connection(ipoque_struct, IPOQUE_CORRELATED_PROTOCOL);
				IPQ_LOG(IPOQUE_PROTOCOL_WORLDOFWARCRAFT, ipoque_struct, IPQ_LOG_DEBUG,
						"World of Warcraft: Web Client found\n");
				return;
			}
		}
		if (packet->payload_packet_len > IPQ_STATICSTRING_LEN("GET /")
			&& memcmp(packet->payload, "GET /", IPQ_STATICSTRING_LEN("GET /")) == 0) {
			ipq_parse_packet_line_info(ipoque_struct);
			if (packet->user_agent_line.ptr != NULL && packet->host_line.ptr != NULL
				&& packet->user_agent_line.len > IPQ_STATICSTRING_LEN("Blizzard Downloader")
				&& packet->host_line.len > IPQ_STATICSTRING_LEN("worldofwarcraft.com")
				&& memcmp(packet->user_agent_line.ptr, "Blizzard Downloader",
						  IPQ_STATICSTRING_LEN("Blizzard Downloader")) == 0
				&& memcmp(&packet->host_line.ptr[packet->host_line.len - IPQ_STATICSTRING_LEN("worldofwarcraft.com")],
						  "worldofwarcraft.com", IPQ_STATICSTRING_LEN("worldofwarcraft.com")) == 0) {
				ipoque_int_worldofwarcraft_add_connection(ipoque_struct, IPOQUE_CORRELATED_PROTOCOL);
				IPQ_LOG(IPOQUE_PROTOCOL_WORLDOFWARCRAFT, ipoque_struct, IPQ_LOG_DEBUG,
						"World of Warcraft: Web Client found\n");
				return;
			}
		}
		if (packet->payload_packet_len == 50 && memcmp(&packet->payload[2], "WORLD OF WARCRAFT CONNECTION",
													   IPQ_STATICSTRING_LEN("WORLD OF WARCRAFT CONNECTION")) == 0) {
			ipoque_int_worldofwarcraft_add_connection(ipoque_struct, IPOQUE_REAL_PROTOCOL);
			IPQ_LOG(IPOQUE_PROTOCOL_WORLDOFWARCRAFT, ipoque_struct, IPQ_LOG_DEBUG, "World of Warcraft: Login found\n");
			return;
		}
		if (packet->tcp->dest == htons(3724) && packet->payload_packet_len < 70
			&& packet->payload_packet_len > 40 && (memcmp(&packet->payload[4], "WoW", 3) == 0
												   || memcmp(&packet->payload[5], "WoW", 3) == 0)) {
			ipoque_int_worldofwarcraft_add_connection(ipoque_struct, IPOQUE_REAL_PROTOCOL);
			IPQ_LOG(IPOQUE_PROTOCOL_WORLDOFWARCRAFT, ipoque_struct, IPQ_LOG_DEBUG, "World of Warcraft: Login found\n");
			return;
		}

		if (IPQ_SRC_OR_DST_HAS_PROTOCOL(src, dst, IPOQUE_PROTOCOL_WORLDOFWARCRAFT) != 0) {
			if (packet->tcp->source == htons(3724)
				&& packet->payload_packet_len == 8 && get_u32(packet->payload, 0) == htonl(0x0006ec01)) {
				ipoque_int_worldofwarcraft_add_connection(ipoque_struct, IPOQUE_REAL_PROTOCOL);
				IPQ_LOG(IPOQUE_PROTOCOL_WORLDOFWARCRAFT, ipoque_struct,
						IPQ_LOG_DEBUG, "World of Warcraft: connection detected\n");
				return;
			}

		}

		/* for some well known WoW ports
		   check another pattern */
		if (flow->l4.tcp.wow_stage == 0) {
			if (ipoque_int_is_wow_port(packet->tcp->source) &&
				packet->payload_packet_len >= 14 &&
				ntohs(get_u16(packet->payload, 0)) == (packet->payload_packet_len - 2)) {
				if (get_u32(packet->payload, 2) == htonl(0xec010100)) {

					IPQ_LOG(IPOQUE_PROTOCOL_WORLDOFWARCRAFT, ipoque_struct,
							IPQ_LOG_DEBUG, "probably World of Warcraft, waiting for final packet\n");
					flow->l4.tcp.wow_stage = 2;
					return;
				} else if (packet->payload_packet_len == 41 &&
						   (get_u16(packet->payload, 2) == htons(0x0085) ||
							get_u16(packet->payload, 2) == htons(0x0034) ||
							get_u16(packet->payload, 2) == htons(0x1960))) {
					IPQ_LOG(IPOQUE_PROTOCOL_WORLDOFWARCRAFT, ipoque_struct,
							IPQ_LOG_DEBUG, "maybe World of Warcraft, need next\n");
					flow->l4.tcp.wow_stage = 1;
					return;
				}
			}
		}

		if (flow->l4.tcp.wow_stage == 1) {
			if (packet->payload_packet_len == 325 &&
				ntohs(get_u16(packet->payload, 0)) == (packet->payload_packet_len - 2) &&
				get_u16(packet->payload, 4) == 0 &&
				(get_u16(packet->payload, packet->payload_packet_len - 3) == htons(0x2331) ||
				 get_u16(packet->payload, 67) == htons(0x2331)) &&
				(memcmp
				 (&packet->payload[packet->payload_packet_len - 18],
				  "\x94\xec\xff\xfd\x67\x62\xd4\x67\xfb\xf9\xdd\xbd\xfd\x01\xc0\x8f\xf9\x81", 18) == 0
				 || memcmp(&packet->payload[packet->payload_packet_len - 30],
						   "\x94\xec\xff\xfd\x67\x62\xd4\x67\xfb\xf9\xdd\xbd\xfd\x01\xc0\x8f\xf9\x81", 18) == 0)) {
				ipoque_int_worldofwarcraft_add_connection(ipoque_struct, IPOQUE_REAL_PROTOCOL);
				IPQ_LOG(IPOQUE_PROTOCOL_WORLDOFWARCRAFT, ipoque_struct,
						IPQ_LOG_DEBUG, "World of Warcraft: connection detected\n");
				return;
			}
			if (packet->payload_packet_len > 32 &&
				ntohs(get_u16(packet->payload, 0)) == (packet->payload_packet_len - 2)) {
				if (get_u16(packet->payload, 4) == 0) {

					IPQ_LOG(IPOQUE_PROTOCOL_WORLDOFWARCRAFT, ipoque_struct,
							IPQ_LOG_DEBUG, "probably World of Warcraft, waiting for final packet\n");
					flow->l4.tcp.wow_stage = 2;
					return;
				} else if (get_u32(packet->payload, 2) == htonl(0x12050000)) {
					IPQ_LOG(IPOQUE_PROTOCOL_WORLDOFWARCRAFT, ipoque_struct,
							IPQ_LOG_DEBUG, "probably World of Warcraft, waiting for final packet\n");
					flow->l4.tcp.wow_stage = 2;
					return;
				}
			}
		}

		if (flow->l4.tcp.wow_stage == 2) {
			if (packet->payload_packet_len == 4) {
				ipoque_int_worldofwarcraft_add_connection(ipoque_struct, IPOQUE_REAL_PROTOCOL);
				IPQ_LOG(IPOQUE_PROTOCOL_WORLDOFWARCRAFT, ipoque_struct,
						IPQ_LOG_DEBUG, "World of Warcraft: connection detected\n");
				return;
			} else if (packet->payload_packet_len > 4 && packet->payload_packet_len <= 16 && packet->payload[4] == 0x0c) {
				ipoque_int_worldofwarcraft_add_connection(ipoque_struct, IPOQUE_REAL_PROTOCOL);
				IPQ_LOG(IPOQUE_PROTOCOL_WORLDOFWARCRAFT, ipoque_struct,
						IPQ_LOG_DEBUG, "World of Warcraft: connection detected\n");
				return;
			} else if (flow->packet_counter < 3) {
				IPQ_LOG(IPOQUE_PROTOCOL_WORLDOFWARCRAFT, ipoque_struct, IPQ_LOG_DEBUG, "waiting for final packet\n");
				return;
			}
		}
		if (flow->l4.tcp.wow_stage == 0 && packet->tcp->dest == htons(1119)) {
			/* special log in port for battle.net/world of warcraft */

			if (packet->payload_packet_len >= 77 &&
				get_u32(packet->payload, 0) == htonl(0x40000aed) && get_u32(packet->payload, 4) == htonl(0xea070aed)) {

				ipoque_int_worldofwarcraft_add_connection(ipoque_struct, IPOQUE_REAL_PROTOCOL);
				IPQ_LOG(IPOQUE_PROTOCOL_WORLDOFWARCRAFT, ipoque_struct,
						IPQ_LOG_DEBUG, "World of Warcraft: connection detected\n");
				return;
			}
		}
	}

	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_WORLDOFWARCRAFT);
}

#endif
