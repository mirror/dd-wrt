/*
 * icecast.c
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
#include "ipq_utils.h"

#ifdef IPOQUE_PROTOCOL_ICECAST

static void ipoque_int_icecast_add_connection(struct ipoque_detection_module_struct
											  *ipoque_struct)
{
	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_ICECAST, IPOQUE_CORRELATED_PROTOCOL);
}

void ipoque_search_icecast_tcp(struct ipoque_detection_module_struct
							   *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;

	u8 i;

	IPQ_LOG(IPOQUE_PROTOCOL_ICECAST, ipoque_struct, IPQ_LOG_DEBUG, "search icecast.\n");

	if ((packet->payload_packet_len < 500 &&
		 packet->payload_packet_len >= 7 && ipq_mem_cmp(packet->payload, "SOURCE ", 7) == 0)
		|| flow->l4.tcp.icecast_stage) {
		ipq_parse_packet_line_info_unix(ipoque_struct);
		IPQ_LOG(IPOQUE_PROTOCOL_ICECAST, ipoque_struct, IPQ_LOG_DEBUG, "Icecast lines=%d\n", packet->parsed_unix_lines);
		for (i = 0; i < packet->parsed_unix_lines; i++) {
			if (packet->unix_line[i].ptr != NULL && packet->unix_line[i].len > 4
				&& ipq_mem_cmp(packet->unix_line[i].ptr, "ice-", 4) == 0) {
				IPQ_LOG(IPOQUE_PROTOCOL_ICECAST, ipoque_struct, IPQ_LOG_DEBUG, "Icecast detected.\n");
				ipoque_int_icecast_add_connection(ipoque_struct);
				return;
			}
		}

		if (packet->parsed_unix_lines < 1 && !flow->l4.tcp.icecast_stage) {
			flow->l4.tcp.icecast_stage = 1;
			return;
		}
	}
#ifdef IPOQUE_PROTOCOL_HTTP
	if (IPQ_FLOW_PROTOCOL_EXCLUDED(ipoque_struct, flow, IPOQUE_PROTOCOL_HTTP)) {
		goto icecast_exclude;
	}
#endif

	if (packet->packet_direction == flow->setup_packet_direction && flow->packet_counter < 10) {
		return;
	}

	if (packet->packet_direction != flow->setup_packet_direction) {
		/* server answer, now test Server for Icecast */


		ipq_parse_packet_line_info(ipoque_struct);

		if (packet->server_line.ptr != NULL && packet->server_line.len > IPQ_STATICSTRING_LEN("Icecast") &&
			memcmp(packet->server_line.ptr, "Icecast", IPQ_STATICSTRING_LEN("Icecast")) == 0) {
			IPQ_LOG(IPOQUE_PROTOCOL_ICECAST, ipoque_struct, IPQ_LOG_DEBUG, "Icecast detected.\n");
			/* TODO maybe store the previous protocol type as subtype?
			 *      e.g. ogg or mpeg
			 */
			ipoque_int_icecast_add_connection(ipoque_struct);
			return;
		}
	}

  icecast_exclude:
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_ICECAST);
	IPQ_LOG(IPOQUE_PROTOCOL_ICECAST, ipoque_struct, IPQ_LOG_DEBUG, "Icecast excluded.\n");
}
#endif
