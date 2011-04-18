/*
 * meebo.c
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
#ifdef IPOQUE_PROTOCOL_MEEBO

static void ipoque_search_meebo(struct ipoque_detection_module_struct
						 *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
	const u8 *p, *end, *line;
	int len;

	IPQ_LOG(IPOQUE_PROTOCOL_MEEBO, ipoque_struct, IPQ_LOG_DEBUG, "search meebo.\n");

	if ((
#ifdef	IPOQUE_PROTOCOL_HTTP
			packet->detected_protocol == IPOQUE_PROTOCOL_HTTP
#endif
			|| ((packet->payload_packet_len > 3 && memcmp(packet->payload, "GET ", 4) == 0)
				|| (packet->payload_packet_len > 4 && memcmp(packet->payload, "POST ", 5) == 0))
		) && flow->packet_counter == 1) {

		for (p = packet->payload, end = p + packet->payload_packet_len;
		     get_next_line(&p, end, &line, &len);) {
			if (LINE_HEADER_MATCH_I(line, len, "Host:") &&
				len >= 9 && memcmp(&line[len - 9], "meebo.com", 9) == 0) {
				IPQ_LOG(IPOQUE_PROTOCOL_MEEBO, ipoque_struct, IPQ_LOG_DEBUG, "found meebo by host_line .meebo.com.\n");
				ipq_connection_detected(ipoque_struct, IPOQUE_PROTOCOL_MEEBO);
				return;
			}
			if (len > 29 && memcmp(line, "Referer: http://www.meebo.com/", 30) == 0) {

				IPQ_LOG(IPOQUE_PROTOCOL_MEEBO, ipoque_struct, IPQ_LOG_DEBUG,
						"found meebo by Referer: http://www.meebo.com/.\n");
				ipq_connection_detected(ipoque_struct, IPOQUE_PROTOCOL_MEEBO);
				return;
			}
		}

	}

	if (packet->detected_protocol == IPOQUE_PROTOCOL_MEEBO) {
		IPQ_LOG(IPOQUE_PROTOCOL_MEEBO, ipoque_struct, IPQ_LOG_DEBUG,
				"in case that ssl meebo has been detected return.\n");
		return;
	}

	if (flow->packet_counter < 5 && packet->detected_protocol == IPOQUE_PROTOCOL_UNKNOWN
		&& IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_SSL) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_MEEBO, ipoque_struct, IPQ_LOG_DEBUG, "ssl not yet excluded. need next packet.\n");
		return;
	}

	IPQ_LOG(IPOQUE_PROTOCOL_MEEBO, ipoque_struct, IPQ_LOG_DEBUG, "exclude meebo.\n");
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_MEEBO);
}
#endif
