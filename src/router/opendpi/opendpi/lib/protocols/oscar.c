/*
 * oscar.c
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

#ifdef IPOQUE_PROTOCOL_OSCAR

static void ipoque_int_oscar_add_connection(struct ipoque_detection_module_struct
											*ipoque_struct)
{

	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
	struct ipoque_id_struct *src = ipoque_struct->src;
	struct ipoque_id_struct *dst = ipoque_struct->dst;

	flow->detected_protocol = IPOQUE_PROTOCOL_OSCAR;
	packet->detected_protocol = IPOQUE_PROTOCOL_OSCAR;

	if (src != NULL) {
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(src->detected_protocol_bitmask, IPOQUE_PROTOCOL_OSCAR);
		src->oscar_last_safe_access_time = packet->tick_timestamp;
	}
	if (dst != NULL) {
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(dst->detected_protocol_bitmask, IPOQUE_PROTOCOL_OSCAR);
		dst->oscar_last_safe_access_time = packet->tick_timestamp;
	}
}

static void ipoque_search_oscar_tcp_connect(struct ipoque_detection_module_struct
											*ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
	struct ipoque_id_struct *src = ipoque_struct->src;
	struct ipoque_id_struct *dst = ipoque_struct->dst;
	if (packet->payload_packet_len >= 10 && packet->payload[0] == 0x2a) {

		/* if is a oscar connection, 10 bytes long */

		/* OSCAR Connection :: Connection detected at initial packets only
		 * +----+----+------+------+---------------+
		 * |0x2a|Code|SeqNum|PktLen|ProtcolVersion |
		 * +----+----+------+------+---------------+
		 * Code 1 Byte : 0x01 Oscar Connection
		 * SeqNum and PktLen are 2 Bytes each and ProtcolVersion: 0x00000001
		 * */
		if (get_u8(packet->payload, 1) == 0x01 && get_u16(packet->payload, 4) == htons(packet->payload_packet_len - 6)
			&& get_u32(packet->payload, 6) == htonl(0x0000000001)) {
			IPQ_LOG(IPOQUE_PROTOCOL_OSCAR, ipoque_struct, IPQ_LOG_DEBUG, "OSCAR Connection FOUND \n");
			ipoque_int_oscar_add_connection(ipoque_struct);
			return;
		}

		/* OSCAR IM
		 * +----+----+------+------+----------+-----------+
		 * |0x2a|Code|SeqNum|PktLen|FNACfamily|FNACsubtype|
		 * +----+----+------+------+----------+-----------+
		 * Code 1 Byte : 0x02 SNAC Header Code;
		 * SeqNum and PktLen are 2 Bytes each
		 * FNACfamily   2 Byte : 0x0004 IM Messaging
		 * FNACEsubtype 2 Byte : 0x0006 IM Outgoing Message, 0x000c IM Message Acknowledgment
		 * */
		if (packet->payload[1] == 0x02
			&& ntohs(get_u16(packet->payload, 4)) >=
			packet->payload_packet_len - 6 && get_u16(packet->payload, 6) == htons(0x0004)
			&& (get_u16(packet->payload, 8) == htons(0x0006)
				|| get_u16(packet->payload, 8) == htons(0x000c))) {
			IPQ_LOG(IPOQUE_PROTOCOL_OSCAR, ipoque_struct, IPQ_LOG_DEBUG, "OSCAR IM Detected \n");
			ipoque_int_oscar_add_connection(ipoque_struct);
			return;
		}
	}


	/* detect http connections */
	if (packet->payload_packet_len > 40
		&& ((memcmp(packet->payload, "GET /aim", 8) == 0) || (memcmp(packet->payload, "GET /im", 7) == 0))) {
		IPQ_PARSE_PACKET_LINE_INFO(ipoque_struct, packet);
		if (packet->user_agent_line.len > 15 && packet->user_agent_line.ptr != NULL &&
			((memcmp(packet->user_agent_line.ptr, "mobileAIM/", 10) == 0) ||
			 memcmp(packet->user_agent_line.ptr, "mobileICQ/", 10) == 0)) {
			ipoque_int_oscar_add_connection(ipoque_struct);
			return;
		}
	}
	if (packet->payload_packet_len > 40 && memcmp(packet->payload, "CONNECT ", 8) == 0) {
		if (memcmp(packet->payload, "CONNECT login.icq.com:443 HTTP/1.", 33) == 0) {
			IPQ_LOG(IPOQUE_PROTOCOL_OSCAR, ipoque_struct, IPQ_LOG_DEBUG, "OSCAR ICQ-HTTP FOUND\n");
			ipoque_int_oscar_add_connection(ipoque_struct);
			return;
		}
		if (memcmp(packet->payload, "CONNECT login.oscar.aol.com:5190 HTTP/1.", 40) == 0) {
			IPQ_LOG(IPOQUE_PROTOCOL_OSCAR, ipoque_struct, IPQ_LOG_DEBUG, "OSCAR AIM-HTTP FOUND\n");
			ipoque_int_oscar_add_connection(ipoque_struct);
			return;
		}

	}

	if (packet->payload_packet_len > 43
		&& memcmp(packet->payload, "GET http://http.proxy.icq.com/hello HTTP/1.", 43) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_OSCAR, ipoque_struct, IPQ_LOG_DEBUG, "OSCAR ICQ-HTTP PROXY FOUND\n");
		ipoque_int_oscar_add_connection(ipoque_struct);
		return;
	}

	if (packet->payload_packet_len > 46
		&& memcmp(packet->payload, "GET http://aimhttp.oscar.aol.com/hello HTTP/1.", 46) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_OSCAR, ipoque_struct, IPQ_LOG_DEBUG, "OSCAR AIM-HTTP PROXY FOUND\n");
		ipoque_int_oscar_add_connection(ipoque_struct);
		return;
	}

	if (IPQ_SRC_OR_DST_HAS_PROTOCOL(src, dst, IPOQUE_PROTOCOL_OSCAR) != 0) {

		if (flow->packet_counter == 1
			&&
			((packet->payload_packet_len == 9
			  && memcmp(packet->payload, "\x00\x09\x00\x00\x83\x01\xc0\x00\x00", 9) == 0)
			 || (packet->payload_packet_len == 13
				 && (memcmp(packet->payload, "\x00\x0d\x00\x87\x01\xc0", 6) == 0
					 || memcmp(packet->payload, "\x00\x0d\x00\x87\x01\xc1", 6) == 0)))) {
			flow->oscar_video_voice = 1;
		}
		if (flow->oscar_video_voice && ntohs(get_u16(packet->payload, 0)) == packet->payload_packet_len
			&& packet->payload[2] == 0x00 && packet->payload[3] == 0x00) {
			ipoque_int_oscar_add_connection(ipoque_struct);
		}

		if (packet->payload_packet_len >= 70 && ntohs(get_u16(packet->payload, 4)) == packet->payload_packet_len) {
			if (memcmp(packet->payload, "OFT", 3) == 0 &&
				((packet->payload[3] == '3' && ((memcmp(&packet->payload[4], "\x01\x00\x01\x01", 4) == 0)
												|| (memcmp(&packet->payload[6], "\x01\x01\x00", 3) == 0)))
				 || (packet->payload[3] == '2' && ((memcmp(&packet->payload[6], "\x01\x01", 2)
													== 0)
					 )))) {
				// FILE TRANSFER PATTERN:: OFT3 or OFT2
				IPQ_LOG(IPOQUE_PROTOCOL_OSCAR, ipoque_struct, IPQ_LOG_DEBUG, "OSCAR FILE TRANSFER\n");
				ipoque_int_oscar_add_connection(ipoque_struct);
				return;
			}

			if (memcmp(packet->payload, "ODC2", 4) == 0 && memcmp(&packet->payload[6], "\x00\x01\x00\x06", 4) == 0) {
				//PICTURE TRANSFER PATTERN EXMAPLE::
				//4f 44 43 32 00 4c 00 01 00 06 00 00 00 00 00 00  ODC2.L..........
				IPQ_LOG(IPOQUE_PROTOCOL_OSCAR, ipoque_struct, IPQ_LOG_DEBUG, "OSCAR PICTURE TRANSFER\n");
				ipoque_int_oscar_add_connection(ipoque_struct);
				return;
			}
		}
		if (packet->payload_packet_len > 40 && (memcmp(&packet->payload[2], "\x04\x4a\x00", 3) == 0)
			&& (memcmp(&packet->payload[6], "\x00\x00", 2) == 0)
			&& packet->payload[packet->payload_packet_len - 15] == 'F'
			&& packet->payload[packet->payload_packet_len - 12] == 'L'
			&& (memcmp(&packet->payload[packet->payload_packet_len - 6], "DEST", 4) == 0)
			&& (memcmp(&packet->payload[packet->payload_packet_len - 2], "\x00\x00", 2) == 0)) {
			IPQ_LOG(IPOQUE_PROTOCOL_OSCAR, ipoque_struct, IPQ_LOG_DEBUG, "OSCAR PICTURE TRANSFER\n");
			ipoque_int_oscar_add_connection(ipoque_struct);
			if (ntohs(packet->tcp->dest) == 443 || ntohs(packet->tcp->source) == 443) {
				flow->oscar_ssl_voice_stage = 1;
			}
			return;

		}
	}
	if (flow->packet_counter < 3 && packet->payload_packet_len > 11 && (memcmp(packet->payload, "\x00\x37\x04\x4a", 4)
																		|| memcmp(packet->payload, "\x00\x0a\x04\x4a",
																				  4))) {
		return;
	}
	if (packet->detected_protocol != IPOQUE_PROTOCOL_OSCAR) {
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_OSCAR);
	}
}

void ipoque_search_oscar(struct ipoque_detection_module_struct *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	if (packet->tcp != NULL) {
		IPQ_LOG(IPOQUE_PROTOCOL_OSCAR, ipoque_struct, IPQ_LOG_DEBUG, "OSCAR :: TCP\n");
		ipoque_search_oscar_tcp_connect(ipoque_struct);
	}
}
#endif
