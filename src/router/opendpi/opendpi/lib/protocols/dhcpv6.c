/*
 * dhcpv6.c
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



/* include files */

#include "ndpi_protocols.h"
#ifdef NDPI_PROTOCOL_DHCPV6

static void ndpi_int_dhcpv6_add_connection(struct ndpi_detection_module_struct *ndpi_struct,
					   struct ndpi_flow_struct *flow)
{

	ndpi_int_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_DHCPV6, NDPI_REAL_PROTOCOL);
}

static void ndpi_search_dhcpv6_udp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{
	struct ndpi_packet_struct *packet = &flow->packet;
	
//  struct ndpi_id_struct         *src=ndpi_struct->src;
//  struct ndpi_id_struct         *dst=ndpi_struct->dst;

	if (packet->payload_packet_len >= 4 &&
		(packet->udp->source == htons(546) || packet->udp->source == htons(547)) &&
		(packet->udp->dest == htons(546) || packet->udp->dest == htons(547)) &&
		packet->payload[0] >= 1 && packet->payload[0] <= 13) {

		NDPI_LOG(NDPI_PROTOCOL_DHCPV6, ndpi_struct, NDPI_LOG_DEBUG, "DHCPv6 detected.\n");
		ndpi_int_dhcpv6_add_connection(ndpi_struct, flow);
		return;
	}

	NDPI_LOG(NDPI_PROTOCOL_DHCPV6, ndpi_struct, NDPI_LOG_DEBUG, "DHCPv6 excluded.\n");
	NDPI_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, NDPI_PROTOCOL_DHCPV6);
}

#endif
