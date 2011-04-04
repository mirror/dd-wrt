/*
 * aimini.c
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

#ifdef IPOQUE_PROTOCOL_AIMINI


static void ipoque_int_aimini_add_connection(struct ipoque_detection_module_struct
											 *ipoque_struct)
{

	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
	struct ipoque_id_struct *src = ipoque_struct->src;
	struct ipoque_id_struct *dst = ipoque_struct->dst;

	flow->detected_protocol = IPOQUE_PROTOCOL_AIMINI;
	packet->detected_protocol = IPOQUE_PROTOCOL_AIMINI;

	if (src != NULL) {
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(src->detected_protocol_bitmask, IPOQUE_PROTOCOL_AIMINI);
	}
	if (dst != NULL) {
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(dst->detected_protocol_bitmask, IPOQUE_PROTOCOL_AIMINI);
	}
}


void ipoque_search_aimini(struct ipoque_detection_module_struct *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
	//    struct ipoque_id_struct         *src=ipoque_struct->src;
	//    struct ipoque_id_struct         *dst=ipoque_struct->dst;


	IPQ_LOG(IPOQUE_PROTOCOL_AIMINI, ipoque_struct, IPQ_LOG_DEBUG, "search aimini.\n");

	if (flow->aimini_stage == 0) {
		if (packet->payload_packet_len == 64 && ntohs(get_u16(packet->payload, 0)) == 0x010b) {
			flow->aimini_stage = 1;
			IPQ_LOG(IPOQUE_PROTOCOL_AIMINI, ipoque_struct, IPQ_LOG_DEBUG, "stage = 1.\n");
			return;
		}
		if (packet->payload_packet_len == 136
			&& (ntohs(get_u16(packet->payload, 0)) == 0x01c9 || ntohs(get_u16(packet->payload, 0)) == 0x0165)) {
			flow->aimini_stage = 4;
			IPQ_LOG(IPOQUE_PROTOCOL_AIMINI, ipoque_struct, IPQ_LOG_DEBUG, "stage = 4.\n");
			return;
		}
		if (packet->payload_packet_len == 88 && ntohs(get_u16(packet->payload, 0)) == 0x0101) {
			flow->aimini_stage = 7;
			IPQ_LOG(IPOQUE_PROTOCOL_AIMINI, ipoque_struct, IPQ_LOG_DEBUG, "stage = 7.\n");
			return;
		}
		if (packet->payload_packet_len == 104 && ntohs(get_u16(packet->payload, 0)) == 0x0102) {
			flow->aimini_stage = 10;
			IPQ_LOG(IPOQUE_PROTOCOL_AIMINI, ipoque_struct, IPQ_LOG_DEBUG, "stage = 10.\n");
			return;
		}
		if (packet->payload_packet_len == 32 && ntohs(get_u16(packet->payload, 0)) == 0x01ca) {
			flow->aimini_stage = 13;
			IPQ_LOG(IPOQUE_PROTOCOL_AIMINI, ipoque_struct, IPQ_LOG_DEBUG, "stage = 13.\n");
			return;
		}
		if (packet->payload_packet_len == 16 && ntohs(get_u16(packet->payload, 0)) == 0x010c) {
			flow->aimini_stage = 16;
			IPQ_LOG(IPOQUE_PROTOCOL_AIMINI, ipoque_struct, IPQ_LOG_DEBUG, "stage = 16.\n");
			return;
		}
	}
	/* first packet chronology: (len, value): (64, 0x010b), (>100, 0x0115), (16, 0x010c || 64, 0x010b),
	 * (16, 0x010c || 64, 0x010b || >100, 0x0115)
	 */
	if (flow->aimini_stage == 1 && packet->payload_packet_len > 100 && ntohs(get_u16(packet->payload, 0)) == 0x0115) {
		flow->aimini_stage = 2;
		IPQ_LOG(IPOQUE_PROTOCOL_AIMINI, ipoque_struct, IPQ_LOG_DEBUG, "stage = 2.\n");
		return;
	}
	if (flow->aimini_stage == 2 && ((packet->payload_packet_len == 16 && ntohs(get_u16(packet->payload, 0)) == 0x010c)
									|| (packet->payload_packet_len == 64
										&& ntohs(get_u16(packet->payload, 0)) == 0x010b))) {
		flow->aimini_stage = 3;
		IPQ_LOG(IPOQUE_PROTOCOL_AIMINI, ipoque_struct, IPQ_LOG_DEBUG, "stage = 3.\n");
		return;
	}
	if (flow->aimini_stage == 3 && ((packet->payload_packet_len == 16 && ntohs(get_u16(packet->payload, 0)) == 0x010c)
									|| (packet->payload_packet_len == 64
										&& ntohs(get_u16(packet->payload, 0)) == 0x010b)
									|| (packet->payload_packet_len > 100
										&& ntohs(get_u16(packet->payload, 0)) == 0x0115))) {
		IPQ_LOG(IPOQUE_PROTOCOL_AIMINI, ipoque_struct, IPQ_LOG_DEBUG, "found aimini (64, 0x010b), (>300, 0x0115), "
				"(16, 0x010c || 64, 0x010b), (16, 0x010c || 64, 0x010b || >100, 0x0115).\n");
		ipoque_int_aimini_add_connection(ipoque_struct);
		return;
	}

	/* second packet chronology: (len, value): (136, 0x01c9), (136, 0x01c9),(136, 0x01c9),(136, 0x01c9 || 32, 0x01ca) */

	if (flow->aimini_stage == 4 && packet->payload_packet_len == 136
		&& (ntohs(get_u16(packet->payload, 0)) == 0x01c9 || ntohs(get_u16(packet->payload, 0)) == 0x0165)) {
		flow->aimini_stage = 5;
		IPQ_LOG(IPOQUE_PROTOCOL_AIMINI, ipoque_struct, IPQ_LOG_DEBUG, "stage = 5.\n");
		return;
	}
	if (flow->aimini_stage == 5 && (packet->payload_packet_len == 136
									&& (ntohs(get_u16(packet->payload, 0)) == 0x01c9
										|| ntohs(get_u16(packet->payload, 0)) == 0x0165))) {
		flow->aimini_stage = 6;
		IPQ_LOG(IPOQUE_PROTOCOL_AIMINI, ipoque_struct, IPQ_LOG_DEBUG, "stage = 6.\n");
		return;
	}
	if (flow->aimini_stage == 6 && ((packet->payload_packet_len == 136
									 && ((ntohs(get_u16(packet->payload, 0)) == 0x0165)
										 || ntohs(get_u16(packet->payload, 0)) == 0x01c9))
									|| (packet->payload_packet_len == 32
										&& ntohs(get_u16(packet->payload, 0)) == 0x01ca))) {
		IPQ_LOG(IPOQUE_PROTOCOL_AIMINI, ipoque_struct, IPQ_LOG_DEBUG,
				"found aimini (136, 0x01c9), (136, 0x01c9)," "(136, 0x01c9),(136, 0x01c9 || 32, 0x01ca).\n");
		ipoque_int_aimini_add_connection(ipoque_struct);
		return;
	}

	/* third packet chronology: (len, value): (88, 0x0101), (88, 0x0101),(88, 0x0101),(88, 0x0101) */

	if (flow->aimini_stage == 7 && packet->payload_packet_len == 88 && ntohs(get_u16(packet->payload, 0)) == 0x0101) {
		flow->aimini_stage = 8;
		IPQ_LOG(IPOQUE_PROTOCOL_AIMINI, ipoque_struct, IPQ_LOG_DEBUG, "stage = 8.\n");
		return;
	}
	if (flow->aimini_stage == 8 && (packet->payload_packet_len == 88 && ntohs(get_u16(packet->payload, 0)) == 0x0101)) {
		flow->aimini_stage = 9;
		IPQ_LOG(IPOQUE_PROTOCOL_AIMINI, ipoque_struct, IPQ_LOG_DEBUG, "stage = 9.\n");
		return;
	}
	if (flow->aimini_stage == 9 && (packet->payload_packet_len == 88 && ntohs(get_u16(packet->payload, 0)) == 0x0101)) {
		IPQ_LOG(IPOQUE_PROTOCOL_AIMINI, ipoque_struct, IPQ_LOG_DEBUG, "found aimini (88, 0x0101), (88, 0x0101),"
				"(88, 0x0101),(88, 0x0101).\n");
		ipoque_int_aimini_add_connection(ipoque_struct);
		return;
	}

	/* fourth packet chronology: (len, value): (104, 0x0102), (104, 0x0102), (104, 0x0102), (104, 0x0102) */

	if (flow->aimini_stage == 10 && packet->payload_packet_len == 104 && ntohs(get_u16(packet->payload, 0)) == 0x0102) {
		flow->aimini_stage = 11;
		IPQ_LOG(IPOQUE_PROTOCOL_AIMINI, ipoque_struct, IPQ_LOG_DEBUG, "stage = 11.\n");
		return;
	}
	if (flow->aimini_stage == 11 && (packet->payload_packet_len == 104 && ntohs(get_u16(packet->payload, 0)) == 0x0102)) {
		flow->aimini_stage = 12;
		IPQ_LOG(IPOQUE_PROTOCOL_AIMINI, ipoque_struct, IPQ_LOG_DEBUG, "stage = 12.\n");
		return;
	}
	if (flow->aimini_stage == 12 && ((packet->payload_packet_len == 104 && ntohs(get_u16(packet->payload, 0)) == 0x0102)
									 || (packet->payload_packet_len == 32
										 && ntohs(get_u16(packet->payload, 0)) == 0x01ca))) {
		IPQ_LOG(IPOQUE_PROTOCOL_AIMINI, ipoque_struct, IPQ_LOG_DEBUG,
				"found aimini (104, 0x0102), (104, 0x0102), " "(104, 0x0102), (104, 0x0102).\n");
		ipoque_int_aimini_add_connection(ipoque_struct);
		return;
	}

	/* fifth packet chronology (len, value): (32,0x01ca), (32,0x01ca), (32,0x01ca), ((136, 0x0166) || (32,0x01ca)) */

	if (flow->aimini_stage == 13 && packet->payload_packet_len == 32 && ntohs(get_u16(packet->payload, 0)) == 0x01ca) {
		flow->aimini_stage = 14;
		IPQ_LOG(IPOQUE_PROTOCOL_AIMINI, ipoque_struct, IPQ_LOG_DEBUG, "stage = 14.\n");
		return;
	}
	if (flow->aimini_stage == 14 && ((packet->payload_packet_len == 32 && ntohs(get_u16(packet->payload, 0)) == 0x01ca)
									 || (packet->payload_packet_len == 136
										 && ntohs(get_u16(packet->payload, 0)) == 0x0166))) {
		flow->aimini_stage = 15;
		IPQ_LOG(IPOQUE_PROTOCOL_AIMINI, ipoque_struct, IPQ_LOG_DEBUG, "stage = 15.\n");
		return;
	}
	if (flow->aimini_stage == 15 && ((packet->payload_packet_len == 136 && ntohs(get_u16(packet->payload, 0)) == 0x0166)
									 || (packet->payload_packet_len == 32
										 && ntohs(get_u16(packet->payload, 0)) == 0x01ca))) {
		IPQ_LOG(IPOQUE_PROTOCOL_AIMINI, ipoque_struct, IPQ_LOG_DEBUG,
				"found aimini (32,0x01ca), (32,0x01ca), (32,0x01ca), ((136, 0x0166)||(32,0x01ca)).\n");
		ipoque_int_aimini_add_connection(ipoque_struct);
		return;
	}

	/* sixth packet chronology (len, value): (16, 0x010c), (16, 0x010c), (16, 0x010c), (16, 0x010c) */

	if (flow->aimini_stage == 16 && packet->payload_packet_len == 16 && ntohs(get_u16(packet->payload, 0)) == 0x010c) {
		flow->aimini_stage = 17;
		IPQ_LOG(IPOQUE_PROTOCOL_AIMINI, ipoque_struct, IPQ_LOG_DEBUG, "stage = 17.\n");
		return;
	}
	if (flow->aimini_stage == 17 && (packet->payload_packet_len == 16 && ntohs(get_u16(packet->payload, 0)) == 0x010c)) {
		flow->aimini_stage = 18;
		IPQ_LOG(IPOQUE_PROTOCOL_AIMINI, ipoque_struct, IPQ_LOG_DEBUG, "stage = 18.\n");
		return;
	}
	if (flow->aimini_stage == 18 && (packet->payload_packet_len == 16 && ntohs(get_u16(packet->payload, 0)) == 0x010c)) {
		IPQ_LOG(IPOQUE_PROTOCOL_AIMINI, ipoque_struct, IPQ_LOG_DEBUG,
				"found aimini (16, 0x010c), (16, 0x010c), (16, 0x010c), (16, 0x010c).\n");
		ipoque_int_aimini_add_connection(ipoque_struct);
		return;
	}



	IPQ_LOG(IPOQUE_PROTOCOL_AIMINI, ipoque_struct, IPQ_LOG_DEBUG, "exclude aimini.\n");
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_AIMINI);

}
#endif
