/*
 * ipp.c
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
#ifdef IPOQUE_PROTOCOL_IPP

static void ipoque_int_ipp_add_connection(struct ipoque_detection_module_struct
										  *ipoque_struct, ipoque_protocol_type_t protocol_type)
{
	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_IPP, protocol_type);
}

void ipoque_search_ipp(struct ipoque_detection_module_struct
					   *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
//      struct ipoque_id_struct         *src=ipoque_struct->src;
//      struct ipoque_id_struct         *dst=ipoque_struct->dst;

	u8 i;

	IPQ_LOG(IPOQUE_PROTOCOL_IPP, ipoque_struct, IPQ_LOG_DEBUG, "search ipp\n");
	if (packet->payload_packet_len > 20) {

		IPQ_LOG(IPOQUE_PROTOCOL_IPP, ipoque_struct, IPQ_LOG_DEBUG,
				"searching for a payload with a pattern like 'number(1to8)blanknumber(1to3)ipp://.\n");
		/* this pattern means that there is a printer saying that his state is idle,
		 * means that he is not printing anything at the moment */
		i = 0;

		if (packet->payload[i] < '0' || packet->payload[i] > '9') {
			IPQ_LOG(IPOQUE_PROTOCOL_IPP, ipoque_struct, IPQ_LOG_DEBUG, "payload does not begin with a number.\n");
			goto search_for_next_pattern;
		}

		for (;;) {
			i++;
			if (!((packet->payload[i] >= '0' && packet->payload[i] <= '9') ||
				  (packet->payload[i] >= 'a' && packet->payload[i] <= 'f') ||
				  (packet->payload[i] >= 'A' && packet->payload[i] <= 'F')) || i > 8) {
				IPQ_LOG(IPOQUE_PROTOCOL_IPP, ipoque_struct, IPQ_LOG_DEBUG,
						"read symbols while the symbol is a number.\n");
				break;
			}
		}

		if (packet->payload[i++] != ' ') {
			IPQ_LOG(IPOQUE_PROTOCOL_IPP, ipoque_struct, IPQ_LOG_DEBUG, "there is no blank following the number.\n");
			goto search_for_next_pattern;
		}

		if (packet->payload[i] < '0' || packet->payload[i] > '9') {
			IPQ_LOG(IPOQUE_PROTOCOL_IPP, ipoque_struct, IPQ_LOG_DEBUG, "no number following the blank.\n");
			goto search_for_next_pattern;
		}

		for (;;) {
			i++;
			if (packet->payload[i] < '0' || packet->payload[i] > '9' || i > 12) {
				IPQ_LOG(IPOQUE_PROTOCOL_IPP, ipoque_struct, IPQ_LOG_DEBUG,
						"read symbols while the symbol is a number.\n");
				break;
			}
		}

		if (ipq_mem_cmp(&packet->payload[i], " ipp://", 7) != 0) {
			IPQ_LOG(IPOQUE_PROTOCOL_IPP, ipoque_struct, IPQ_LOG_DEBUG, "the string ' ipp://' does not follow.\n");
			goto search_for_next_pattern;
		}

		IPQ_LOG(IPOQUE_PROTOCOL_IPP, ipoque_struct, IPQ_LOG_DEBUG, "found ipp\n");
		ipoque_int_ipp_add_connection(ipoque_struct, IPOQUE_REAL_PROTOCOL);
		return;
	}

  search_for_next_pattern:

	if (packet->payload_packet_len > 3 && memcmp(packet->payload, "POST", 4) == 0) {
		ipq_parse_packet_line_info(ipoque_struct);
		if (packet->content_line.ptr != NULL && packet->content_line.len > 14
			&& memcmp(packet->content_line.ptr, "application/ipp", 15) == 0) {
			IPQ_LOG(IPOQUE_PROTOCOL_IPP, ipoque_struct, IPQ_LOG_DEBUG, "found ipp via POST ... application/ipp.\n");
			ipoque_int_ipp_add_connection(ipoque_struct, IPOQUE_CORRELATED_PROTOCOL);
			return;
		}
	}
	IPQ_LOG(IPOQUE_PROTOCOL_IPP, ipoque_struct, IPQ_LOG_DEBUG, "no ipp detected.\n");
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_IPP);
}

#endif
