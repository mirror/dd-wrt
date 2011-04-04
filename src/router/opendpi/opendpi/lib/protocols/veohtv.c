/*
 * veohtv.c
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
#ifdef IPOQUE_PROTOCOL_VEOHTV

static void ipoque_int_veohtv_add_connection(struct ipoque_detection_module_struct
											 *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
	struct ipoque_id_struct *src = ipoque_struct->src;
	struct ipoque_id_struct *dst = ipoque_struct->dst;

	flow->detected_protocol = IPOQUE_PROTOCOL_VEOHTV;
	packet->detected_protocol = IPOQUE_PROTOCOL_VEOHTV;

	if (src != NULL) {
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(src->detected_protocol_bitmask, IPOQUE_PROTOCOL_VEOHTV);
	}
	if (dst != NULL) {
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(dst->detected_protocol_bitmask, IPOQUE_PROTOCOL_VEOHTV);
	}
}

void ipoque_search_veohtv_tcp(struct ipoque_detection_module_struct
							  *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
//      struct ipoque_id_struct         *src=ipoque_struct->src;
//      struct ipoque_id_struct         *dst=ipoque_struct->dst;

	if (packet->payload_packet_len > 4 && memcmp(packet->payload, "GET /", 4) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_VEOHTV, ipoque_struct, IPQ_LOG_DEBUG, "HTTP packet detected.\n");
		ipq_parse_packet_line_info(ipoque_struct);
		if (packet->host_line.ptr != NULL && packet->host_line.len > 9
			&& ipq_mem_cmp(&packet->host_line.ptr[packet->host_line.len - 9],
						   ".veoh.com", 9) == 0
			&& packet->referer_line.ptr != NULL
			&& packet->referer_line.len > 20
			&& ipq_mem_cmp(&packet->referer_line.ptr[packet->referer_line.len - 21], "fullscreen_client.swf",
						   21) == 0) {
			IPQ_LOG(IPOQUE_PROTOCOL_VEOHTV, ipoque_struct, IPQ_LOG_DEBUG, "VeohTV detected.\n");
			ipoque_int_veohtv_add_connection(ipoque_struct);
			return;
		}
	}


	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_VEOHTV);
}
#endif
