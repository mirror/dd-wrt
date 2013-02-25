/*
 * steam.c
 *
 * Copyright (C) 2009-2011 by ipoque GmbH
 * Copyright (C) 2011-13 - ntop.org
 *
 * This file is part of nDPI, an open source deep packet inspection
 * library based on the OpenDPI and PACE technology by ipoque GmbH
 *
 * nDPI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * nDPI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with nDPI.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */


#include "ndpi_protocols.h"
#ifdef NDPI_PROTOCOL_STEAM


static void ndpi_int_steam_add_connection(struct ndpi_detection_module_struct
											*ndpi_struct, struct ndpi_flow_struct *flow)
{
	ndpi_int_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_STEAM, NDPI_REAL_PROTOCOL);
}

static void ndpi_search_steam(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{
	struct ndpi_packet_struct *packet = &flow->packet;
	
//      struct ndpi_id_struct         *src=ndpi_struct->src;
//      struct ndpi_id_struct         *dst=ndpi_struct->dst;

	if (flow->l4.tcp.steam_stage == 0) {
		if (packet->payload_packet_len == 4
			&& ntohl(get_u_int32_t(packet->payload, 0)) <= 0x07
			&& ntohs(packet->tcp->dest) >= 27030 && ntohs(packet->tcp->dest) <= 27040) {
			flow->l4.tcp.steam_stage = 1 + packet->packet_direction;
			NDPI_LOG(NDPI_PROTOCOL_STEAM, ndpi_struct, NDPI_LOG_DEBUG, "steam stage 1\n");
			return;
		}

	} else if (flow->l4.tcp.steam_stage == 2 - packet->packet_direction) {
		if ((packet->payload_packet_len == 1 || packet->payload_packet_len == 5)
			&& packet->payload[0] == 0x01) {
			ndpi_int_steam_add_connection(ndpi_struct, flow);
			NDPI_LOG(NDPI_PROTOCOL_STEAM, ndpi_struct, NDPI_LOG_DEBUG, "steam detected\n");
			return;
		}
	}

	NDPI_LOG(NDPI_PROTOCOL_STEAM, ndpi_struct, NDPI_LOG_DEBUG, "steam excluded.\n");
	NDPI_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, NDPI_PROTOCOL_STEAM);
}

#endif
