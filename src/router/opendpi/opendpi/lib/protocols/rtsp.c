/*
 * rtsp.c
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

#ifdef IPOQUE_PROTOCOL_RTSP
#ifndef IPOQUE_PROTOCOL_RTP
#error RTSP requires RTP detection to work correctly
#endif
#ifndef IPOQUE_PROTOCOL_RTSP
#error RTSP requires RTSP detection to work correctly
#endif
#ifndef IPOQUE_PROTOCOL_RDP
#error RTSP requires RDP detection to work correctly
#endif

static void ipoque_int_rtsp_add_connection(struct ipoque_detection_module_struct
										   *ipoque_struct, ipoque_protocol_type_t protocol_type)
{
	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_RTSP, protocol_type);
}

/* this function searches for a rtsp-"handshake" over tcp or udp. */
void ipoque_search_rtsp_tcp_udp(struct ipoque_detection_module_struct
								*ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
	struct ipoque_id_struct *src = ipoque_struct->src;
	struct ipoque_id_struct *dst = ipoque_struct->dst;

	IPQ_LOG(IPOQUE_PROTOCOL_RTSP, ipoque_struct, IPQ_LOG_DEBUG, "calling ipoque_search_rtsp_tcp_udp.\n");


	if (flow->rtsprdt_stage == 0
#ifdef IPOQUE_PROTOCOL_RTCP
		&& !(packet->detected_protocol_stack[0] == IPOQUE_PROTOCOL_RTCP)
#endif
		) {
		flow->rtsprdt_stage = 1 + packet->packet_direction;

		IPQ_LOG(IPOQUE_PROTOCOL_RTSP, ipoque_struct, IPQ_LOG_DEBUG, "maybe handshake 1; need next packet, return.\n");
		return;
	}

	if (flow->packet_counter < 3 && flow->rtsprdt_stage == 1 + packet->packet_direction) {

		IPQ_LOG(IPOQUE_PROTOCOL_RTSP, ipoque_struct, IPQ_LOG_DEBUG, "maybe handshake 2; need next packet.\n");
		return;
	}

	if (packet->payload_packet_len > 20 && flow->rtsprdt_stage == 2 - packet->packet_direction) {

		// RTSP Server Message
		if (memcmp(packet->payload, "RTSP/1.0 ", 9) == 0) {


			IPQ_LOG(IPOQUE_PROTOCOL_RTSP, ipoque_struct, IPQ_LOG_DEBUG, "found RTSP/1.0 .\n");

			if (dst != NULL) {
				IPQ_LOG(IPOQUE_PROTOCOL_RTSP, ipoque_struct, IPQ_LOG_DEBUG, "found dst.\n");
				ipq_packet_src_ip_get(packet, &dst->rtsp_ip_address);
				dst->rtsp_timer = packet->tick_timestamp;
				dst->rtsp_ts_set = 1;
			}
			if (src != NULL) {
				IPQ_LOG(IPOQUE_PROTOCOL_RTSP, ipoque_struct, IPQ_LOG_DEBUG, "found src.\n");
				ipq_packet_dst_ip_get(packet, &src->rtsp_ip_address);
				src->rtsp_timer = packet->tick_timestamp;
				src->rtsp_ts_set = 1;
			}
			IPQ_LOG(IPOQUE_PROTOCOL_RTSP, ipoque_struct, IPQ_LOG_DEBUG, "found RTSP.\n");
			flow->rtsp_control_flow = 1;
			ipoque_int_rtsp_add_connection(ipoque_struct, IPOQUE_REAL_PROTOCOL);
			return;
		}
	}
	if (packet->udp != NULL && packet->detected_protocol_stack[0] == IPOQUE_PROTOCOL_UNKNOWN
		&& ((IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_RTP) == 0)
#ifdef IPOQUE_PROTOCOL_RTCP
			|| (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_RTCP) == 0)
#endif
		)) {
		IPQ_LOG(IPOQUE_PROTOCOL_RTSP, ipoque_struct, IPQ_LOG_DEBUG,
				"maybe RTSP RTP, RTSP RTCP, RDT; need next packet.\n");
		return;
	}


	IPQ_LOG(IPOQUE_PROTOCOL_RTSP, ipoque_struct, IPQ_LOG_DEBUG, "didn't find handshake, exclude.\n");
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_RTSP);
	return;
}


#endif
