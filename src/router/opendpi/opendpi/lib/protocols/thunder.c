/*
 * thunder.c
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
#ifdef IPOQUE_PROTOCOL_THUNDER


static inline void ipoque_int_search_thunder_udp(struct ipoque_detection_module_struct
												 *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
//      struct ipoque_id_struct         *src=ipoque_struct->src;
//      struct ipoque_id_struct         *dst=ipoque_struct->dst;

	if (packet->payload_packet_len > 8 && packet->payload[0] >= 0x30
		&& packet->payload[0] < 0x40 && packet->payload[1] == 0 && packet->payload[2] == 0 && packet->payload[3] == 0) {
		if (flow->thunder_stage == 3) {
			IPQ_LOG(IPOQUE_PROTOCOL_THUNDER, ipoque_struct, IPQ_LOG_DEBUG, "THUNDER udp detected\n");
			ipq_connection_detected(ipoque_struct, IPOQUE_PROTOCOL_THUNDER);
			return;
		}

		flow->thunder_stage++;
		IPQ_LOG(IPOQUE_PROTOCOL_THUNDER, ipoque_struct, IPQ_LOG_DEBUG,
				"maybe thunder udp packet detected, stage increased to %u\n", flow->thunder_stage);
		return;
	}

	IPQ_LOG(IPOQUE_PROTOCOL_THUNDER, ipoque_struct, IPQ_LOG_DEBUG,
			"excluding thunder udp at stage %u\n", flow->thunder_stage);

	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_THUNDER);
}

static inline void ipoque_int_search_thunder_tcp(struct ipoque_detection_module_struct
												 *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
//      struct ipoque_id_struct         *src=ipoque_struct->src;
//      struct ipoque_id_struct         *dst=ipoque_struct->dst;

	if (packet->payload_packet_len > 8 && packet->payload[0] >= 0x30
		&& packet->payload[0] < 0x40 && packet->payload[1] == 0 && packet->payload[2] == 0 && packet->payload[3] == 0) {
		if (flow->thunder_stage == 3) {
			IPQ_LOG(IPOQUE_PROTOCOL_THUNDER, ipoque_struct, IPQ_LOG_DEBUG, "THUNDER tcp detected\n");
			ipq_connection_detected(ipoque_struct, IPOQUE_PROTOCOL_THUNDER);
			return;
		}

		flow->thunder_stage++;
		IPQ_LOG(IPOQUE_PROTOCOL_THUNDER, ipoque_struct, IPQ_LOG_DEBUG,
				"maybe thunder tcp packet detected, stage increased to %u\n", flow->thunder_stage);
		return;
	}

	if (flow->thunder_stage == 0 && packet->payload_packet_len > 17
		&& ipq_mem_cmp(packet->payload, "POST / HTTP/1.1\r\n", 17) == 0) {
		ipq_parse_packet_line_info(ipoque_struct);

		IPQ_LOG(IPOQUE_PROTOCOL_THUNDER, ipoque_struct, IPQ_LOG_DEBUG,
				"maybe thunder http POST packet detected, parsed packet lines: %u, empty line set %u (at: %u)\n",
				packet->parsed_lines, packet->empty_line_position_set, packet->empty_line_position);

		if (packet->empty_line_position_set != 0 &&
			packet->content_line.ptr != NULL &&
			packet->content_line.len == 24 &&
			ipq_mem_cmp(packet->content_line.ptr, "application/octet-stream",
						24) == 0 && packet->empty_line_position_set < (packet->payload_packet_len - 8)
			&& packet->payload[packet->empty_line_position + 2] >= 0x30
			&& packet->payload[packet->empty_line_position + 2] < 0x40
			&& packet->payload[packet->empty_line_position + 3] == 0x00
			&& packet->payload[packet->empty_line_position + 4] == 0x00
			&& packet->payload[packet->empty_line_position + 5] == 0x00) {
			IPQ_LOG(IPOQUE_PROTOCOL_THUNDER, ipoque_struct, IPQ_LOG_DEBUG,
					"maybe thunder http POST packet application does match\n");
			ipq_connection_detected(ipoque_struct, IPOQUE_PROTOCOL_THUNDER);
			return;
		}
	}
	IPQ_LOG(IPOQUE_PROTOCOL_THUNDER, ipoque_struct, IPQ_LOG_DEBUG,
			"excluding thunder tcp at stage %u\n", flow->thunder_stage);

	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_THUNDER);
}

static void ipoque_int_search_thunder_http(struct ipoque_detection_module_struct *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_id_struct *src = ipoque_struct->src;
	struct ipoque_id_struct *dst = ipoque_struct->dst;
	const u8 *p, *end, *line;
	bool ua_found = false;
	int i, len;

	if (packet->payload_packet_len > 5
		&& memcmp(packet->payload, "GET /", 5) == 0 && IPQ_SRC_OR_DST_HAS_PROTOCOL(src, dst, IPOQUE_PROTOCOL_THUNDER)) {
		IPQ_LOG(IPOQUE_PROTOCOL_THUNDER, ipoque_struct, IPQ_LOG_DEBUG, "HTTP packet detected.\n");

		for (p = packet->payload, end = p + packet->payload_packet_len, i = 0;
		     get_next_line(&p, end, &line, &len); i++) {

			switch(i) {
			case 1:
				if (!LINE_HEADER_MATCH(line, len, "Accept: */*"))
					return;
				break;
			case 2:
				if (!LINE_HEADER_MATCH(line, len, "Cache-Control: no-cache"))
					return;
				break;
			case 3:
				if (!LINE_HEADER_MATCH(line, len, "Connection: close"))
					return;
				break;
			case 4:
				if (!LINE_HEADER_MATCH(line, len, "Host: "))
					return;
				break;
			case 5:
				if (!LINE_HEADER_MATCH(line, len, "Pragma: no-cache"))
					return;
				break;
			}

			if (LINE_HEADER_MATCH_I(line, len, "User-Agent: ")) {
				line += 12;
				len -= 12;
				if (!LINE_HEADER_MATCH(line, len, "Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.0)"))
					return;
				ua_found = true;
			}

			if (i > 10)
				return;
		}
		if (!ua_found)
			return;

		IPQ_LOG(IPOQUE_PROTOCOL_THUNDER, ipoque_struct, IPQ_LOG_DEBUG,
				"Thunder HTTP download detected, adding flow.\n");
		ipq_connection_detected(ipoque_struct, IPOQUE_PROTOCOL_THUNDER);
	}
}

static void ipoque_search_thunder(struct ipoque_detection_module_struct *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;

	if (packet->tcp != NULL) {
		ipoque_int_search_thunder_http(ipoque_struct);
		ipoque_int_search_thunder_tcp(ipoque_struct);
	} else if (packet->udp != NULL) {
		ipoque_int_search_thunder_udp(ipoque_struct);
	}
}

#endif
