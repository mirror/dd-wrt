/*
 * qq.c
 * Copyright (C) 2009-2010 by ipoque GmbH
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

#ifdef IPOQUE_PROTOCOL_QQ

static void ipoque_int_qq_add_connection(struct ipoque_detection_module_struct
										 *ipoque_struct)
{

	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
	struct ipoque_id_struct *src = ipoque_struct->src;
	struct ipoque_id_struct *dst = ipoque_struct->dst;

	flow->detected_protocol = IPOQUE_PROTOCOL_QQ;
	packet->detected_protocol = IPOQUE_PROTOCOL_QQ;

	if (src != NULL) {
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(src->detected_protocol_bitmask, IPOQUE_PROTOCOL_QQ);
	}
	if (dst != NULL) {
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(dst->detected_protocol_bitmask, IPOQUE_PROTOCOL_QQ);
	}
}

static inline void ipoque_search_qq_udp(struct ipoque_detection_module_struct *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
//  struct ipoque_id_struct *src = ipoque_struct->src;
//  struct ipoque_id_struct *dst = ipoque_struct->dst;
	static const u16 p8000_patt_02[10] = { 0x1549, 0x1801, 0x0961, 0x01501, 0x0e35, 0x113f, 0x0b37, 0x1131, 0x163a };
	u16 no_of_patterns = 9, index = 0;


	IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG, "search qq udp.\n");


	if (flow->qq_stage <= 3) {
		if ((packet->payload_packet_len == 27 && ntohs(get_u16(packet->payload, 0)) == 0x0300
			 && packet->payload[2] == 0x01)
			|| (packet->payload_packet_len == 84 && ((ntohs(get_u16(packet->payload, 0)) == 0x000e
													  && packet->payload[2] == 0x35)
													 || (ntohs(get_u16(packet->payload, 0)) == 0x0015
														 && packet->payload[2] == 0x01)
													 || (ntohs(get_u16(packet->payload, 0)) == 0x000b
														 && packet->payload[2] == 0x37)
													 || (ntohs(get_u16(packet->payload, 0)) == 0x0015
														 && packet->payload[2] == 0x49)))
			|| (packet->payload_packet_len > 10
				&& ((ntohs(get_u16(packet->payload, 0)) == 0x000b && packet->payload[2] == 0x37)
					|| (ntohl(get_u32(packet->payload, 0)) == 0x04163a00
						&& packet->payload[packet->payload_packet_len - 1] == 0x03
						&& packet->payload[4] == packet->payload_packet_len)))) {
			flow->qq_stage++;
			if (flow->qq_stage == 3) {
				IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG,
						"found qq udp pattern 030001 or 000e35 four times.\n");
				ipoque_int_qq_add_connection(ipoque_struct);
				return;
			}
			return;
		}
		if (packet->payload_packet_len > 2 && (packet->payload[0] == 0x02)) {
			u16 pat = ntohs(get_u16(packet->payload, 1));
			for (index = 0; index < no_of_patterns; index++) {
				if (pat == p8000_patt_02[index] && packet->payload[packet->payload_packet_len - 1] == 0x03) {
					flow->qq_stage++;
					if (flow->qq_stage == 3) {
						IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG,
								"found qq udp pattern 02 ... 03 four times.\n");
						ipoque_int_qq_add_connection(ipoque_struct);
						return;
					}
					return;
				}
			}
		}
		if (packet->payload_packet_len > 2 && packet->payload[0] == 0x04
			&& (ntohs(get_u16(packet->payload, 1)) == 0x1549
				|| ntohs(get_u16(packet->payload, 1)) == 0x1801 || ntohs(get_u16(packet->payload, 1)) == 0x0961)
			&& packet->payload[packet->payload_packet_len - 1] == 0x03) {
			flow->qq_stage++;
			if (flow->qq_stage == 3) {
				IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG,
						"found qq udp pattern 04 1159 ... 03 four times.\n");
				ipoque_int_qq_add_connection(ipoque_struct);
				return;
			}
			return;
		}
		if (packet->payload_packet_len > 2 && (packet->payload[0] == 0x06 || packet->payload[0] == 0x02)
			&& ntohs(get_u16(packet->payload, 1)) == 0x0100
			&& (packet->payload[packet->payload_packet_len - 1] == 0x00
				|| packet->payload[packet->payload_packet_len - 1] == 0x03)) {
			flow->qq_stage++;
			if (flow->qq_stage == 3) {
				IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG,
						"found qq udp pattern 02/06 0100 ... 03/00 four times.\n");
				ipoque_int_qq_add_connection(ipoque_struct);
				return;
			}
			return;
		}

		if (packet->payload_packet_len > 2 && (packet->payload[0] == 0x02)
			&& ntohs(get_u16(packet->payload, 1)) == 0x1131 && packet->payload[packet->payload_packet_len - 1] == 0x03) {
			flow->qq_stage++;
			if (flow->qq_stage == 3) {
				IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG,
						"found qq udp pattern 02 1131 ... 03 four times.\n");
				ipoque_int_qq_add_connection(ipoque_struct);
				return;
			}
			return;
		}

		if (packet->udp->dest == htons(9000) || packet->udp->source == htons(9000)) {
			if (packet->payload_packet_len > 3
				&& ntohs(get_u16(packet->payload, 0)) == 0x0202
				&& ntohs(get_u16(packet->payload, 2)) == packet->payload_packet_len) {
				flow->qq_stage++;
				if (flow->qq_stage == 3) {
					IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG,
							"found qq udp pattern 02 02 <length> four times.\n");
					ipoque_int_qq_add_connection(ipoque_struct);
					return;
				}
				return;
			}

		}
	}

	if (flow->qq_stage && flow->packet_counter <= 5) {
		return;
	}

	IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG, "QQ excluded\n");
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_QQ);
}


static inline void ipoque_search_qq_tcp(struct ipoque_detection_module_struct *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
//      struct ipoque_id_struct         *src=ipoque_struct->src;
//      struct ipoque_id_struct         *dst=ipoque_struct->dst;



	u16 i = 0;
//  u16 a = 0;

	IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG, "search qq tcp.\n");


	if ((packet->payload_packet_len > 4 && ntohs(get_u16(packet->payload, 0)) == packet->payload_packet_len
		 && ntohs(get_u16(packet->payload, 2)) == 0x0212 && packet->payload[4] == 0x0b)
		|| (packet->payload_packet_len > 9 && ntohl(get_u32(packet->payload, 0)) == 0x04154900
			&& get_l16(packet->payload, 4) == packet->payload_packet_len
			&& packet->payload[packet->payload_packet_len - 1] == 0x03)
		|| (packet->payload_packet_len > 9 && ntohl(get_u32(packet->payload, 0)) == 0x040e3500
			&& get_l16(packet->payload, 4) == packet->payload_packet_len
			&& packet->payload[9] == 0x33 && packet->payload[packet->payload_packet_len - 1] == 0x03)
		|| (packet->payload_packet_len > 9 && ntohl(get_u32(packet->payload, 0)) == 0x040e0215
			&& get_l16(packet->payload, 4) == packet->payload_packet_len
			&& packet->payload[9] == 0x33 && packet->payload[packet->payload_packet_len - 1] == 0x03)
		|| (packet->payload_packet_len > 6 && ntohl(get_u32(packet->payload, 2)) == 0x020d5500
			&& ntohs(get_u16(packet->payload, 0)) == packet->payload_packet_len
			&& packet->payload[packet->payload_packet_len - 1] == 0x03)
		|| (packet->payload_packet_len > 6 && ntohs(get_u16(packet->payload, 0)) == 0x0418
			&& packet->payload[2] == 0x01
			&& ntohs(get_u16(packet->payload, 3)) == packet->payload_packet_len
			&& packet->payload[packet->payload_packet_len - 1] == 0x03)
		|| (packet->payload_packet_len > 6 && ntohs(get_u16(packet->payload, 0)) == 0x0411
			&& packet->payload[2] == 0x31
			&& ntohs(get_u16(packet->payload, 3)) == packet->payload_packet_len
			&& packet->payload[packet->payload_packet_len - 1] == 0x03)
		|| (packet->payload_packet_len > 6 && ntohs(get_u16(packet->payload, 0)) == packet->payload_packet_len
			&& ntohs(get_u16(packet->payload, 2)) == 0x0211 && packet->payload[4] == 0x31
			&& packet->payload[packet->payload_packet_len - 1] == 0x03)
		|| (packet->payload_packet_len > 6 && ntohs(get_u16(packet->payload, 0)) == packet->payload_packet_len
			&& ntohs(get_u16(packet->payload, 2)) == 0x0218 && packet->payload[4] == 0x01
			&& packet->payload[packet->payload_packet_len - 1] == 0x03)
		|| (packet->payload_packet_len > 10 && ntohl(get_u32(packet->payload, 0)) == 0x04163a00
			&& packet->payload[packet->payload_packet_len - 1] == 0x03
			&& packet->payload[4] == packet->payload_packet_len)
		) {
		flow->qq_stage++;
		if (flow->qq_stage == 3) {
			IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG, "found qq over tcp.\n");
			ipoque_int_qq_add_connection(ipoque_struct);
			return;
		}
		return;
	}
	if (packet->payload_packet_len == 39
		&& ntohl(get_u32(packet->payload, 0)) == 0x27000000 && ntohs(get_u16(packet->payload, 4)) == 0x0014
		&& get_u16(packet->payload, packet->payload_packet_len - 2) == htons(0x0000)) {
		if (flow->qq_stage == 4) {
			IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG, "found qq over tcp.\n");
			ipoque_int_qq_add_connection(ipoque_struct);
			return;
		}
		flow->qq_stage = 4;
		return;
	}
	if (packet->payload_packet_len > 100
		&& ((ipq_mem_cmp(packet->payload, "GET", 3) == 0) || (ipq_mem_cmp(packet->payload, "POST", 4) == 0))) {
		IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG, "found GET or POST.\n");
		if (memcmp(packet->payload, "GET /qqfile/qq", 14) == 0) {
			IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG, "found qq over tcp GET /qqfile/qq.\n");
			ipoque_int_qq_add_connection(ipoque_struct);
			return;
		}
		ipq_parse_packet_line_info(ipoque_struct);

		if (packet->user_agent_line.ptr != NULL
			&& (packet->user_agent_line.len > 7 && memcmp(packet->user_agent_line.ptr, "QQClient", 8) == 0)) {
			IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG, "found qq over tcp GET...QQClient\n");
			ipoque_int_qq_add_connection(ipoque_struct);
			return;
		}
		for (i = 0; i < packet->parsed_lines; i++) {
			if (packet->line[i].len > 3 && memcmp(packet->line[i].ptr, "QQ: ", 4) == 0) {
				IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG, "found qq over tcp GET...QQ: \n");
				ipoque_int_qq_add_connection(ipoque_struct);
				return;
			}
		}
		if (packet->host_line.ptr != NULL) {
			IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG, "host line ptr\n");
			if (packet->host_line.len > 11 && memcmp(&packet->host_line.ptr[0], "www.qq.co.za", 12) == 0) {
				IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG, "found qq over tcp Host: www.qq.co.za\n");
				ipoque_int_qq_add_connection(ipoque_struct);
				return;
			}
		}
	}
	if (flow->qq_stage == 0 && packet->payload_packet_len == 82
		&& ntohl(get_u32(packet->payload, 0)) == 0x0000004e && ntohl(get_u32(packet->payload, 4)) == 0x01010000) {
		for (i = 8; i < 82; i++) {
			if (packet->payload[i] != 0x00) {
				break;
			}
			if (i == 81) {
				IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG, "found qq Mail.\n");
				ipoque_int_qq_add_connection(ipoque_struct);
				return;
			}
		}
	}
	if (flow->qq_stage == 0 && packet->payload_packet_len == 182
		&& ntohl(get_u32(packet->payload, 0)) == 0x000000b2
		&& ntohl(get_u32(packet->payload, 4)) == 0x01020000
		&& ntohl(get_u32(packet->payload, 8)) == 0x04015151 && ntohl(get_u32(packet->payload, 12)) == 0x4d61696c) {
		IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG, "found qq Mail.\n");
		ipoque_int_qq_add_connection(ipoque_struct);
		return;
	}
	if (packet->payload_packet_len == 204 && flow->qq_stage == 0 && ntohl(get_u32(packet->payload, 200)) == 0xfbffffff) {
		for (i = 0; i < 200; i++) {
			IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG, "i = %u\n", i);
			if (packet->payload[i] != 0) {
				break;
			}
			if (i == 199) {
				IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG, "found qq chat or file transfer\n");
				ipoque_int_qq_add_connection(ipoque_struct);
				return;
			}
		}
	}
#ifdef IPOQUE_PROTOCOL_HTTP
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_HTTP) != 0) {
#endif							/* IPOQUE_PROTOCOL_HTTP */

		IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_QQ);
		IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG, "QQ tcp excluded; len %u\n",
				packet->payload_packet_len);

#ifdef IPOQUE_PROTOCOL_HTTP
	}
#endif							/* IPOQUE_PROTOCOL_HTTP */

}

void ipoque_search_qq(struct ipoque_detection_module_struct *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;

	if (packet->udp != NULL)
		ipoque_search_qq_udp(ipoque_struct);

	if (packet->tcp != NULL)
		ipoque_search_qq_tcp(ipoque_struct);
}

#endif
