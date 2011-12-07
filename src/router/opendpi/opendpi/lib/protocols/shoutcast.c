/*
 * shoutcast.c
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

#ifdef IPOQUE_PROTOCOL_SHOUTCAST

static void ipoque_int_shoutcast_add_connection(struct ipoque_detection_module_struct
												*ipoque_struct)
{
	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_SHOUTCAST, IPOQUE_CORRELATED_PROTOCOL);
}

void ipoque_search_shoutcast_tcp(struct ipoque_detection_module_struct
								 *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;

	IPQ_LOG(IPOQUE_PROTOCOL_SHOUTCAST, ipoque_struct, IPQ_LOG_DEBUG, "search shoutcast.\n");

	if (flow->packet_counter == 1) {
/* this case in paul_upload_oddcast_002.pcap */
		if (packet->payload_packet_len >= 6
			&& packet->payload_packet_len < 80 && memcmp(packet->payload, "123456", 6) == 0) {
			IPQ_LOG(IPOQUE_PROTOCOL_SHOUTCAST, ipoque_struct, IPQ_LOG_DEBUG, "Shoutcast stage 1, \"123456\".\n");
			return;
		}
		if (flow->packet_counter < 3
#ifdef IPOQUE_PROTOCOL_HTTP
			&& packet->detected_protocol_stack[0] == IPOQUE_PROTOCOL_HTTP
#endif
			) {
			IPQ_LOG(IPOQUE_PROTOCOL_SHOUTCAST, ipoque_struct, IPQ_LOG_DEBUG,
					"http detected, need next packet for shoutcast detection.\n");
			if (packet->payload_packet_len > 4
				&& get_u32(packet->payload, packet->payload_packet_len - 4) != htonl(0x0d0a0d0a)) {
				IPQ_LOG(IPOQUE_PROTOCOL_SHOUTCAST, ipoque_struct, IPQ_LOG_DEBUG, "segmented packet found.\n");
				flow->l4.tcp.shoutcast_stage = 1 + packet->packet_direction;
			}
			return;
		}


		/*  else
		   goto exclude_shoutcast; */

	}
	/* evtl. für asym detection noch User-Agent:Winamp dazunehmen. */
	if (packet->payload_packet_len > 11 && memcmp(packet->payload, "ICY 200 OK\x0d\x0a", 12) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_SHOUTCAST, ipoque_struct, IPQ_LOG_DEBUG, "found shoutcast by ICY 200 OK.\n");
		ipoque_int_shoutcast_add_connection(ipoque_struct);
		return;
	}
	if (flow->l4.tcp.shoutcast_stage == 1 + packet->packet_direction
		&& flow->packet_direction_counter[packet->packet_direction] < 5) {
		return;
	}

	if (flow->packet_counter == 2) {
		if (packet->payload_packet_len == 2 && memcmp(packet->payload, "\x0d\x0a", 2) == 0) {
			IPQ_LOG(IPOQUE_PROTOCOL_SHOUTCAST, ipoque_struct, IPQ_LOG_DEBUG, "Shoutcast stage 1 continuation.\n");
			return;
		} else if (packet->payload_packet_len > 3 && ipq_mem_cmp(&packet->payload[0], "OK2", 3) == 0) {
			IPQ_LOG(IPOQUE_PROTOCOL_SHOUTCAST, ipoque_struct, IPQ_LOG_DEBUG, "Shoutcast stage 2, OK2 found.\n");
			return;
		} else
			goto exclude_shoutcast;
	} else if (flow->packet_counter == 3 || flow->packet_counter == 4) {
		if (packet->payload_packet_len > 3 && ipq_mem_cmp(&packet->payload[0], "OK2", 3) == 0) {
			IPQ_LOG(IPOQUE_PROTOCOL_SHOUTCAST, ipoque_struct, IPQ_LOG_DEBUG, "Shoutcast stage 2, OK2 found.\n");
			return;
		} else if (packet->payload_packet_len > 4 && ipq_mem_cmp(&packet->payload[0], "icy-", 4) == 0) {
			IPQ_LOG(IPOQUE_PROTOCOL_SHOUTCAST, ipoque_struct, IPQ_LOG_DEBUG, "Shoutcast detected.\n");
			ipoque_int_shoutcast_add_connection(ipoque_struct);
			return;
		} else
			goto exclude_shoutcast;
	}

  exclude_shoutcast:
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_SHOUTCAST);
	IPQ_LOG(IPOQUE_PROTOCOL_SHOUTCAST, ipoque_struct, IPQ_LOG_DEBUG, "Shoutcast excluded.\n");
}
#endif
