/*
 * meebo.c
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

#ifdef IPOQUE_PROTOCOL_MEEBO

static void ipoque_int_meebo_add_connection(struct ipoque_detection_module_struct
											*ipoque_struct)
{
	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_MEEBO, IPOQUE_CORRELATED_PROTOCOL);
}




void ipoque_search_meebo(struct ipoque_detection_module_struct
						 *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;

	// struct ipoque_id_struct *src=ipoque_struct->src;
	// struct ipoque_id_struct *dst=ipoque_struct->dst;


	IPQ_LOG(IPOQUE_PROTOCOL_MEEBO, ipoque_struct, IPQ_LOG_DEBUG, "search meebo.\n");

	/* catch audio/video flows which are flash (rtmp) */
	if (
#ifdef IPOQUE_PROTOCOL_FLASH
		   packet->detected_protocol_stack[0] == IPOQUE_PROTOCOL_FLASH
#else
		   (packet->tcp->source == htons(1935) || packet->tcp->dest == htons(1935))
#endif
		) {

		/* TODO: once we have an amf decoder we can more directly access the rtmp fields
		 *       if so, we may also exclude earlier */
		if (packet->payload_packet_len > 900) {
			if (memcmp(packet->payload + 116, "tokbox/", IPQ_STATICSTRING_LEN("tokbox/")) == 0 ||
				memcmp(packet->payload + 316, "tokbox/", IPQ_STATICSTRING_LEN("tokbox/")) == 0) {
				IPQ_LOG(IPOQUE_PROTOCOL_MEEBO, ipoque_struct, IPQ_LOG_DEBUG, "found meebo/tokbox flash flow.\n");
				ipoque_int_meebo_add_connection(ipoque_struct);
				return;
			}
		}

		if (flow->packet_counter < 16 && flow->packet_direction_counter[flow->setup_packet_direction] < 6) {
			IPQ_LOG(IPOQUE_PROTOCOL_MEEBO, ipoque_struct, IPQ_LOG_DEBUG, "need next packet.\n");
			return;
		}

		IPQ_LOG(IPOQUE_PROTOCOL_MEEBO, ipoque_struct, IPQ_LOG_DEBUG, "exclude meebo.\n");
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_MEEBO);
		return;
	}

	if ((
#ifdef	IPOQUE_PROTOCOL_HTTP
			packet->detected_protocol_stack[0] == IPOQUE_PROTOCOL_HTTP ||
#endif
			((packet->payload_packet_len > 3 && memcmp(packet->payload, "GET ", 4) == 0)
			 || (packet->payload_packet_len > 4 && memcmp(packet->payload, "POST ", 5) == 0))
		) && flow->packet_counter == 1) {
		u8 host_or_referer_match = 0;

		ipq_parse_packet_line_info(ipoque_struct);
		if (packet->host_line.ptr != NULL
			&& packet->host_line.len >= 9
			&& memcmp(&packet->host_line.ptr[packet->host_line.len - 9], "meebo.com", 9) == 0) {

			IPQ_LOG(IPOQUE_PROTOCOL_MEEBO, ipoque_struct, IPQ_LOG_DEBUG, "Found Meebo host\n");
			host_or_referer_match = 1;
		} else if (packet->host_line.ptr != NULL
				   && packet->host_line.len >= 10
				   && memcmp(&packet->host_line.ptr[packet->host_line.len - 10], "tokbox.com", 10) == 0) {

			IPQ_LOG(IPOQUE_PROTOCOL_MEEBO, ipoque_struct, IPQ_LOG_DEBUG, "Found tokbox host\n");
			/* set it to 2 to avoid having plain tokbox traffic detected as meebo */
			host_or_referer_match = 2;
		} else if (packet->host_line.ptr != NULL && packet->host_line.len >= IPQ_STATICSTRING_LEN("74.114.28.110")
				   && memcmp(&packet->host_line.ptr[packet->host_line.len - IPQ_STATICSTRING_LEN("74.114.28.110")],
							 "74.114.28.110", IPQ_STATICSTRING_LEN("74.114.28.110")) == 0) {

			IPQ_LOG(IPOQUE_PROTOCOL_MEEBO, ipoque_struct, IPQ_LOG_DEBUG, "Found meebo IP\n");
			host_or_referer_match = 1;
		} else if (packet->referer_line.ptr != NULL &&
				   packet->referer_line.len >= IPQ_STATICSTRING_LEN("http://www.meebo.com/") &&
				   memcmp(packet->referer_line.ptr, "http://www.meebo.com/",
						  IPQ_STATICSTRING_LEN("http://www.meebo.com/")) == 0) {

			IPQ_LOG(IPOQUE_PROTOCOL_MEEBO, ipoque_struct, IPQ_LOG_DEBUG, "Found meebo referer\n");
			host_or_referer_match = 1;
		} else if (packet->referer_line.ptr != NULL &&
				   packet->referer_line.len >= IPQ_STATICSTRING_LEN("http://mee.tokbox.com/") &&
				   memcmp(packet->referer_line.ptr, "http://mee.tokbox.com/",
						  IPQ_STATICSTRING_LEN("http://mee.tokbox.com/")) == 0) {

			IPQ_LOG(IPOQUE_PROTOCOL_MEEBO, ipoque_struct, IPQ_LOG_DEBUG, "Found tokbox referer\n");
			host_or_referer_match = 1;
		} else if (packet->referer_line.ptr != NULL &&
				   packet->referer_line.len >= IPQ_STATICSTRING_LEN("http://74.114.28.110/") &&
				   memcmp(packet->referer_line.ptr, "http://74.114.28.110/",
						  IPQ_STATICSTRING_LEN("http://74.114.28.110/")) == 0) {

			IPQ_LOG(IPOQUE_PROTOCOL_MEEBO, ipoque_struct, IPQ_LOG_DEBUG, "Found meebo IP referer\n");
			host_or_referer_match = 1;
		}

		if (host_or_referer_match) {
			if (host_or_referer_match == 1) {
				IPQ_LOG(IPOQUE_PROTOCOL_MEEBO, ipoque_struct, IPQ_LOG_DEBUG,
						"Found Meebo traffic based on host/referer\n");
				ipoque_int_meebo_add_connection(ipoque_struct);
				return;
			}
		}
	}

	if (packet->detected_protocol_stack[0] == IPOQUE_PROTOCOL_MEEBO) {
		IPQ_LOG(IPOQUE_PROTOCOL_MEEBO, ipoque_struct, IPQ_LOG_DEBUG,
				"in case that ssl meebo has been detected return.\n");
		return;
	}

	if (flow->packet_counter < 5 && packet->detected_protocol_stack[0] == IPOQUE_PROTOCOL_UNKNOWN
		&& IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_SSL) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_MEEBO, ipoque_struct, IPQ_LOG_DEBUG, "ssl not yet excluded. need next packet.\n");
		return;
	}
#ifdef IPOQUE_PROTOCOL_FLASH
	if (flow->packet_counter < 5 && packet->detected_protocol_stack[0] == IPOQUE_PROTOCOL_UNKNOWN &&
		!IPQ_FLOW_PROTOCOL_EXCLUDED(ipoque_struct, flow, IPOQUE_PROTOCOL_FLASH)) {
		IPQ_LOG(IPOQUE_PROTOCOL_MEEBO, ipoque_struct, IPQ_LOG_DEBUG, "flash not yet excluded. need next packet.\n");
		return;
	}
#endif

	IPQ_LOG(IPOQUE_PROTOCOL_MEEBO, ipoque_struct, IPQ_LOG_DEBUG, "exclude meebo.\n");
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_MEEBO);
}
#endif
