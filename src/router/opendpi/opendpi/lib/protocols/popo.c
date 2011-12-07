/*
 * popo.c
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
#ifdef IPOQUE_PROTOCOL_POPO

static void ipoque_int_popo_add_connection(struct ipoque_detection_module_struct
										   *ipoque_struct)
{
	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_POPO, IPOQUE_REAL_PROTOCOL);
}

void ipoque_search_popo_tcp_udp(struct ipoque_detection_module_struct
								*ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
	struct ipoque_id_struct *src = ipoque_struct->src;
	struct ipoque_id_struct *dst = ipoque_struct->dst;

	if (packet->tcp != NULL) {
		if ((packet->payload_packet_len == 20)
			&& get_u32(packet->payload, 0) == htonl(0x0c000000)
			&& get_u32(packet->payload, 4) == htonl(0x01010000)
			&& get_u32(packet->payload, 8) == htonl(0x06000000)
			&& get_u32(packet->payload, 12) == 0 && get_u32(packet->payload, 16) == 0) {
			IPQ_LOG(IPOQUE_PROTOCOL_POPO, ipoque_struct, IPQ_LOG_DEBUG, "POPO detected\n");
			ipoque_int_popo_add_connection(ipoque_struct);
			return;
		}

		if (IPQ_SRC_OR_DST_HAS_PROTOCOL(src, dst, IPOQUE_PROTOCOL_POPO) != 0) {
#define IPOQUE_POPO_IP_SUBNET_START ( (220 << 24) + (181 << 16) + (28 << 8) + 220)
#define IPOQUE_POPO_IP_SUBNET_END ( (220 << 24) + (181 << 16) + (28 << 8) + 238)

			/* may match the first payload ip packet only ... */

			if (ntohl(packet->iph->daddr) >= IPOQUE_POPO_IP_SUBNET_START
				&& ntohl(packet->iph->daddr) <= IPOQUE_POPO_IP_SUBNET_END) {
				IPQ_LOG(IPOQUE_PROTOCOL_POPO, ipoque_struct, IPQ_LOG_DEBUG, "POPO ip subnet detected\n");
				ipoque_int_popo_add_connection(ipoque_struct);
				return;
			}
		}
	}

	if (packet->payload_packet_len > 13 && packet->payload_packet_len == get_l32(packet->payload, 0)
		&& !get_l16(packet->payload, 12)) {
		register u16 ii;
		for (ii = 14; ii < 50 && ii < packet->payload_packet_len - 8; ++ii) {
			if (packet->payload[ii] == '@')
				if (!memcmp(&packet->payload[ii + 1], "163.com", 7)
					|| (ii <= packet->payload_packet_len - 13 && !memcmp(&packet->payload[ii + 1], "popo.163.com", 12))) {
					IPQ_LOG(IPOQUE_PROTOCOL_POPO, ipoque_struct, IPQ_LOG_DEBUG, "POPO  detected.\n");
					ipoque_int_popo_add_connection(ipoque_struct);
					return;
				}
		}
	}

	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_POPO);
}

#endif
