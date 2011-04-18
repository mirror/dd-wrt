/*
 * zattoo.c
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

#ifdef IPOQUE_PROTOCOL_ZATTOO

static u32 ipq_bytestream_to_ipv4(const u8 * str, u16 max_chars_to_read, u16 * bytes_read)
{
	u32 val;
	u16 read = 0;
	u16 oldread;
	u32 c;
	/* ip address must be X.X.X.X with each X between 0 and 255 */
	oldread = read;
	c = ipq_bytestream_to_number(str, max_chars_to_read, &read);
	if (c > 255 || oldread == read || max_chars_to_read == read || str[read] != '.')
		return 0;
	read++;
	val = c << 24;
	oldread = read;
	c = ipq_bytestream_to_number(&str[read], max_chars_to_read - read, &read);
	if (c > 255 || oldread == read || max_chars_to_read == read || str[read] != '.')
		return 0;
	read++;
	val = val + (c << 16);
	oldread = read;
	c = ipq_bytestream_to_number(&str[read], max_chars_to_read - read, &read);
	if (c > 255 || oldread == read || max_chars_to_read == read || str[read] != '.')
		return 0;
	read++;
	val = val + (c << 8);
	oldread = read;
	c = ipq_bytestream_to_number(&str[read], max_chars_to_read - read, &read);
	if (c > 255 || oldread == read || max_chars_to_read == read)
		return 0;
	val = val + c;

	*bytes_read = *bytes_read + read;

	return htonl(val);
}


static void ipoque_search_zattoo_tcp(struct ipoque_detection_module_struct
							  *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;

	struct ipoque_id_struct *src = ipoque_struct->src;
	struct ipoque_id_struct *dst = ipoque_struct->dst;
	const u8 *p, *end, *line;
	int len;

	if (packet->detected_protocol == IPOQUE_PROTOCOL_ZATTOO) {
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
			ipq_connection_detected(ipoque_struct, IPOQUE_PROTOCOL_ZATTOO);
			return;
		}
		if (packet->payload_packet_len > 50
			&& memcmp(packet->payload, "GET /ZattooAdRedirect/redirect.jsp?user=", 40) == 0) {
			IPQ_LOG(IPOQUE_PROTOCOL_ZATTOO, ipoque_struct,
					IPQ_LOG_DEBUG, "add connection over tcp with pattern GET /ZattooAdRedirect/redirect.jsp?user=\n");
			ipq_connection_detected(ipoque_struct, IPOQUE_PROTOCOL_ZATTOO);
			return;
		}
		if (packet->payload_packet_len > 50
			&& (memcmp(packet->payload, "POST /channelserver/player/channel/update HTTP/1.1", 50) == 0
				|| memcmp(packet->payload, "GET /epg/query", 14) == 0)) {
			for (p = packet->payload, end = p + packet->payload_packet_len;
			     get_next_line(&p, end, &line, &len);) {
				if (len >= 18 && (ipq_mem_cmp(line, "User-Agent: Zattoo", 18) == 0)) {
					IPQ_LOG(IPOQUE_PROTOCOL_ZATTOO, ipoque_struct,
							IPQ_LOG_DEBUG,
							"add connection over tcp with pattern POST /channelserver/player/channel/update HTTP/1.1\n");
					ipq_connection_detected(ipoque_struct, IPOQUE_PROTOCOL_ZATTOO);
					return;
				}
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
					ipq_connection_detected(ipoque_struct, IPOQUE_PROTOCOL_ZATTOO);
					return;
				}
			}
		}

		else if (flow->zattoo_stage == 0) {

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
			ipq_connection_detected(ipoque_struct, IPOQUE_PROTOCOL_ZATTOO);
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
			ipq_connection_detected(ipoque_struct, IPOQUE_PROTOCOL_ZATTOO);
			return;
		} else if (flow->zattoo_stage == 5 + packet->packet_direction && (packet->payload_packet_len == 125)) {
			IPQ_LOG(IPOQUE_PROTOCOL_ZATTOO, ipoque_struct, IPQ_LOG_DEBUG, "detected zattoo.\n");
			ipq_connection_detected(ipoque_struct, IPOQUE_PROTOCOL_ZATTOO);
			return;
		} else if (flow->zattoo_stage == 6 - packet->packet_direction && packet->payload_packet_len == 1412) {
			IPQ_LOG(IPOQUE_PROTOCOL_ZATTOO, ipoque_struct, IPQ_LOG_DEBUG, "found zattoo.\n");
			ipq_connection_detected(ipoque_struct, IPOQUE_PROTOCOL_ZATTOO);
			return;
		}
		IPQ_LOG(IPOQUE_PROTOCOL_ZATTOO, ipoque_struct, IPQ_LOG_DEBUG,
				"ZATTOO: discarted the flow (TCP): packet_size: %u; Flowstage: %u\n",
				packet->payload_packet_len, flow->zattoo_stage);

#endif
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
				ipq_connection_detected(ipoque_struct, IPOQUE_PROTOCOL_ZATTOO);
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
