/*
 * zattoo.c
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

#ifdef IPOQUE_PROTOCOL_ZATTOO

static void ipoque_int_zattoo_add_connection(struct ipoque_detection_module_struct
											 *ipoque_struct, ipoque_protocol_type_t protocol_type)
{

	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_id_struct *src = ipoque_struct->src;
	struct ipoque_id_struct *dst = ipoque_struct->dst;

	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_ZATTOO, protocol_type);

	if (src != NULL) {
		src->zattoo_ts = packet->tick_timestamp;
	}
	if (dst != NULL) {
		dst->zattoo_ts = packet->tick_timestamp;
	}
}


	
#if !(defined(HAVE_NTOP) && defined(WIN32))
 static inline
#else
__forceinline static
#endif
	 u8 ipoque_int_zattoo_user_agent_set(struct ipoque_detection_module_struct *ipoque_struct)
{
	if (ipoque_struct->packet.user_agent_line.ptr != NULL && ipoque_struct->packet.user_agent_line.len == 111) {
		if (memcmp(ipoque_struct->packet.user_agent_line.ptr +
				   ipoque_struct->packet.user_agent_line.len - 25, "Zattoo/4", sizeof("Zattoo/4") - 1) == 0) {
			IPQ_LOG(IPOQUE_PROTOCOL_ZATTOO, ipoque_struct, IPQ_LOG_DEBUG, "found zattoo useragent\n");
			return 1;
		}
	}
	return 0;
}

void ipoque_search_zattoo(struct ipoque_detection_module_struct *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;

	struct ipoque_id_struct *src = ipoque_struct->src;
	struct ipoque_id_struct *dst = ipoque_struct->dst;

	u16 i;

	if (packet->detected_protocol_stack[0] == IPOQUE_PROTOCOL_ZATTOO) {
		if (src != NULL && ((IPOQUE_TIMESTAMP_COUNTER_SIZE)
							(packet->tick_timestamp - src->zattoo_ts) < ipoque_struct->zattoo_connection_timeout)) {
			src->zattoo_ts = packet->tick_timestamp;
		}
		if (dst != NULL && ((IPOQUE_TIMESTAMP_COUNTER_SIZE)
							(packet->tick_timestamp - dst->zattoo_ts) < ipoque_struct->zattoo_connection_timeout)) {
			dst->zattoo_ts = packet->tick_timestamp;
		}
		return;
	}

	if (packet->tcp != NULL) {
		if (packet->payload_packet_len > 50 && memcmp(packet->payload, "GET /frontdoor/fd?brand=Zattoo&v=", 33) == 0) {
			IPQ_LOG(IPOQUE_PROTOCOL_ZATTOO, ipoque_struct,
					IPQ_LOG_DEBUG, "add connection over tcp with pattern GET /frontdoor/fd?brand=Zattoo&v=\n");
			ipoque_int_zattoo_add_connection(ipoque_struct, IPOQUE_CORRELATED_PROTOCOL);
			return;
		}
		if (packet->payload_packet_len > 50
			&& memcmp(packet->payload, "GET /ZattooAdRedirect/redirect.jsp?user=", 40) == 0) {
			IPQ_LOG(IPOQUE_PROTOCOL_ZATTOO, ipoque_struct,
					IPQ_LOG_DEBUG, "add connection over tcp with pattern GET /ZattooAdRedirect/redirect.jsp?user=\n");
			ipoque_int_zattoo_add_connection(ipoque_struct, IPOQUE_CORRELATED_PROTOCOL);
			return;
		}
		if (packet->payload_packet_len > 50
			&& (memcmp(packet->payload, "POST /channelserver/player/channel/update HTTP/1.1", 50) == 0
				|| memcmp(packet->payload, "GET /epg/query", 14) == 0)) {
			ipq_parse_packet_line_info(ipoque_struct);
			for (i = 0; i < packet->parsed_lines; i++) {
				if (packet->line[i].len >= 18 && (ipq_mem_cmp(packet->line[i].ptr, "User-Agent: Zattoo", 18) == 0)) {
					IPQ_LOG(IPOQUE_PROTOCOL_ZATTOO, ipoque_struct,
							IPQ_LOG_DEBUG,
							"add connection over tcp with pattern POST /channelserver/player/channel/update HTTP/1.1\n");
					ipoque_int_zattoo_add_connection(ipoque_struct, IPOQUE_CORRELATED_PROTOCOL);
					return;
				}
			}
		} else if (packet->payload_packet_len > 50
				   && (memcmp(packet->payload, "GET /", 5) == 0
					   || memcmp(packet->payload, "POST /", IPQ_STATICSTRING_LEN("POST /")) == 0)) {
			/* TODO to avoid searching currently only a specific length and offset is used
			 * that might be changed later */
			ipq_parse_packet_line_info(ipoque_struct);
			if (ipoque_int_zattoo_user_agent_set(ipoque_struct)) {
				ipoque_int_zattoo_add_connection(ipoque_struct, IPOQUE_CORRELATED_PROTOCOL);
				return;
			}
		} else if (packet->payload_packet_len > 50 && memcmp(packet->payload, "POST http://", 12) == 0) {
			ipq_parse_packet_line_info(ipoque_struct);
			// test for unique character of the zattoo header
			if (packet->parsed_lines == 4 && packet->host_line.ptr != NULL) {
				u32 ip;
				u16 bytes_read = 0;

				ip = ipq_bytestream_to_ipv4(&packet->payload[12], packet->payload_packet_len, &bytes_read);

				// and now test the firt 5 bytes of the payload for zattoo pattern
				if (ip == packet->iph->daddr
					&& packet->empty_line_position_set != 0
					&& ((packet->payload_packet_len - packet->empty_line_position) > 10)
					&& packet->payload[packet->empty_line_position + 2] ==
					0x03
					&& packet->payload[packet->empty_line_position + 3] ==
					0x04
					&& packet->payload[packet->empty_line_position + 4] ==
					0x00
					&& packet->payload[packet->empty_line_position + 5] ==
					0x04
					&& packet->payload[packet->empty_line_position + 6] ==
					0x0a && packet->payload[packet->empty_line_position + 7] == 0x00) {
					IPQ_LOG(IPOQUE_PROTOCOL_ZATTOO, ipoque_struct,
							IPQ_LOG_DEBUG, "add connection over tcp with pattern POST http://\n");
					ipoque_int_zattoo_add_connection(ipoque_struct, IPOQUE_CORRELATED_PROTOCOL);
					return;
				}
			}
		} else if (flow->zattoo_stage == 0) {

			if (packet->payload_packet_len > 50
				&& packet->payload[0] == 0x03
				&& packet->payload[1] == 0x04
				&& packet->payload[2] == 0x00
				&& packet->payload[3] == 0x04 && packet->payload[4] == 0x0a && packet->payload[5] == 0x00) {
				flow->zattoo_stage = 1 + packet->packet_direction;
				IPQ_LOG(IPOQUE_PROTOCOL_ZATTOO, ipoque_struct,
						IPQ_LOG_DEBUG, "need next packet, seen pattern 0x030400040a00\n");
				return;
			}
			/* the following is is searching for flash, not for zattoo. cust1 wants to do so. */
		} else if (flow->zattoo_stage == 2 - packet->packet_direction
				   && packet->payload_packet_len > 50 && packet->payload[0] == 0x03 && packet->payload[1] == 0x04) {
			IPQ_LOG(IPOQUE_PROTOCOL_ZATTOO, ipoque_struct, IPQ_LOG_DEBUG, "add connection over tcp with 0x0304.\n");
			ipoque_int_zattoo_add_connection(ipoque_struct, IPOQUE_CORRELATED_PROTOCOL);
			return;
		} else if (flow->zattoo_stage == 1 + packet->packet_direction) {
			if (packet->payload_packet_len > 500 && packet->payload[0] == 0x00 && packet->payload[1] == 0x00) {
				flow->zattoo_stage = 3 + packet->packet_direction;
				IPQ_LOG(IPOQUE_PROTOCOL_ZATTOO, ipoque_struct,
						IPQ_LOG_DEBUG, "need next packet, seen pattern 0x0000\n");
				return;
			}
			if (packet->payload_packet_len > 50
				&& packet->payload[0] == 0x03
				&& packet->payload[1] == 0x04
				&& packet->payload[2] == 0x00
				&& packet->payload[3] == 0x04 && packet->payload[4] == 0x0a && packet->payload[5] == 0x00) {
			}
			IPQ_LOG(IPOQUE_PROTOCOL_ZATTOO, ipoque_struct, IPQ_LOG_DEBUG,
					"need next packet, seen pattern 0x030400040a00\n");
			return;
		} else if (flow->zattoo_stage == 4 - packet->packet_direction
				   && packet->payload_packet_len > 50 && packet->payload[0] == 0x03 && packet->payload[1] == 0x04) {
			IPQ_LOG(IPOQUE_PROTOCOL_ZATTOO, ipoque_struct, IPQ_LOG_DEBUG, "add connection over tcp with 0x0304.\n");
			ipoque_int_zattoo_add_connection(ipoque_struct, IPOQUE_CORRELATED_PROTOCOL);
			return;
		} else if (flow->zattoo_stage == 5 + packet->packet_direction && (packet->payload_packet_len == 125)) {
			IPQ_LOG(IPOQUE_PROTOCOL_ZATTOO, ipoque_struct, IPQ_LOG_DEBUG, "detected zattoo.\n");
			ipoque_int_zattoo_add_connection(ipoque_struct, IPOQUE_CORRELATED_PROTOCOL);
			return;
		} else if (flow->zattoo_stage == 6 - packet->packet_direction && packet->payload_packet_len == 1412) {
			IPQ_LOG(IPOQUE_PROTOCOL_ZATTOO, ipoque_struct, IPQ_LOG_DEBUG, "found zattoo.\n");
			ipoque_int_zattoo_add_connection(ipoque_struct, IPOQUE_CORRELATED_PROTOCOL);
			return;
		}
		IPQ_LOG(IPOQUE_PROTOCOL_ZATTOO, ipoque_struct, IPQ_LOG_DEBUG,
				"ZATTOO: discarted the flow (TCP): packet_size: %u; Flowstage: %u\n",
				packet->payload_packet_len, flow->zattoo_stage);

	} else if (packet->udp != NULL) {

		if (packet->payload_packet_len > 20 && (packet->udp->dest == htons(5003)
												|| packet->udp->source == htons(5003))
			&& (get_u16(packet->payload, 0) == htons(0x037a)
				|| get_u16(packet->payload, 0) == htons(0x0378)
				|| get_u16(packet->payload, 0) == htons(0x0305)
				|| get_u32(packet->payload, 0) == htonl(0x03040004)
				|| get_u32(packet->payload, 0) == htonl(0x03010005))) {
			if (++flow->zattoo_stage == 2) {
				IPQ_LOG(IPOQUE_PROTOCOL_ZATTOO, ipoque_struct, IPQ_LOG_DEBUG, "add connection over udp.\n");
				ipoque_int_zattoo_add_connection(ipoque_struct, IPOQUE_REAL_PROTOCOL);
				return;
			}
			IPQ_LOG(IPOQUE_PROTOCOL_ZATTOO, ipoque_struct, IPQ_LOG_DEBUG, "need next packet udp.\n");
			return;
		}

		IPQ_LOG(IPOQUE_PROTOCOL_ZATTOO, ipoque_struct, IPQ_LOG_DEBUG,
				"ZATTOO: discarded the flow (UDP): packet_size: %u; Flowstage: %u\n",
				packet->payload_packet_len, flow->zattoo_stage);

	}

	IPQ_LOG(IPOQUE_PROTOCOL_ZATTOO, ipoque_struct, IPQ_LOG_DEBUG, "exclude zattoo.\n");
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_ZATTOO);
}
#endif
