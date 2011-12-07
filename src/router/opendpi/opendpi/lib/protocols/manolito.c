/*
 * manolito.c
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
#ifdef IPOQUE_PROTOCOL_MANOLITO

static void ipoque_int_manolito_add_connection(struct
											   ipoque_detection_module_struct
											   *ipoque_struct)
{

	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_id_struct *src = ipoque_struct->src;
	struct ipoque_id_struct *dst = ipoque_struct->dst;

	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_MANOLITO, IPOQUE_REAL_PROTOCOL);


	if (src != NULL) {
		if (packet->udp != NULL) {
			src->manolito_last_pkt_arrival_time = packet->tick_timestamp;
		}
	}
	if (dst != NULL) {
		if (packet->udp != NULL) {
			dst->manolito_last_pkt_arrival_time = packet->tick_timestamp;
		}
	}
}

/*
  return 0 if nothing has been detected
  return 1 if it is a megaupload packet
*/
u8 search_manolito_tcp(struct ipoque_detection_module_struct *ipoque_struct);
u8 search_manolito_tcp(struct ipoque_detection_module_struct *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
//  struct ipoque_id_struct *src = ipoque_struct->src;
//  struct ipoque_id_struct *dst = ipoque_struct->dst;

	IPQ_LOG(IPOQUE_PROTOCOL_MANOLITO, ipoque_struct, IPQ_LOG_DEBUG, "MANOLITO TCP DETECTION\n");

	if (flow->l4.tcp.manolito_stage == 0 && packet->payload_packet_len > 6) {
		if (ipq_mem_cmp(packet->payload, "SIZ ", 4) != 0)
			goto end_manolito_nothing_found;

		flow->l4.tcp.manolito_stage = 1 + packet->packet_direction;
		IPQ_LOG(IPOQUE_PROTOCOL_MANOLITO, ipoque_struct, IPQ_LOG_DEBUG, "MANOLITO Stage 1.\n");
		goto end_manolito_maybe_hit;

	} else if ((flow->l4.tcp.manolito_stage == 2 - packet->packet_direction)
			   && packet->payload_packet_len > 4) {
		if (ipq_mem_cmp(packet->payload, "STR ", 4) != 0)
			goto end_manolito_nothing_found;
		IPQ_LOG(IPOQUE_PROTOCOL_MANOLITO, ipoque_struct, IPQ_LOG_DEBUG, "MANOLITO Stage 2.\n");
		flow->l4.tcp.manolito_stage = 3 + packet->packet_direction;
		goto end_manolito_maybe_hit;

	} else if ((flow->l4.tcp.manolito_stage == 4 - packet->packet_direction) && packet->payload_packet_len > 5) {
		if (ipq_mem_cmp(packet->payload, "MD5 ", 4) != 0)
			goto end_manolito_nothing_found;
		IPQ_LOG(IPOQUE_PROTOCOL_MANOLITO, ipoque_struct, IPQ_LOG_DEBUG, "MANOLITO Stage 3.\n");
		flow->l4.tcp.manolito_stage = 5 + packet->packet_direction;
		goto end_manolito_maybe_hit;

	} else if ((flow->l4.tcp.manolito_stage == 6 - packet->packet_direction) && packet->payload_packet_len == 4) {

		if (ipq_mem_cmp(packet->payload, "GO!!", 4) != 0)
			goto end_manolito_nothing_found;
		IPQ_LOG(IPOQUE_PROTOCOL_MANOLITO, ipoque_struct, IPQ_LOG_DEBUG, "MANOLITO Stage 4.\n");
		goto end_manolito_found;
	}
	//IPQ_LOG(IPOQUE_PROTOCOL_MANOLITO,ipoque_struct, IPQ_LOG_DEBUG, "MANOLITO FLOW STAGE %d\n", flow->l4.tcp.manolito_stage);
	goto end_manolito_nothing_found;

  end_manolito_found:
	IPQ_LOG(IPOQUE_PROTOCOL_MANOLITO, ipoque_struct, IPQ_LOG_DEBUG, "MANOLITO FOUND\n");
	ipoque_int_manolito_add_connection(ipoque_struct);
	return 1;

  end_manolito_maybe_hit:
	IPQ_LOG(IPOQUE_PROTOCOL_MANOLITO, ipoque_struct, IPQ_LOG_DEBUG, "MANOLITO maybe hit.\n");
	return 2;

  end_manolito_nothing_found:
	IPQ_LOG(IPOQUE_PROTOCOL_MANOLITO, ipoque_struct, IPQ_LOG_DEBUG, "MANOLITO NOTHING FOUND\n");
	return 0;
}

void ipoque_search_manolito_tcp_udp(struct
									ipoque_detection_module_struct
									*ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
	struct ipoque_id_struct *src = ipoque_struct->src;
	struct ipoque_id_struct *dst = ipoque_struct->dst;


	if (packet->tcp != NULL) {
		if (search_manolito_tcp(ipoque_struct) != 0)
			return;
	} else if (packet->udp != NULL) {
		if (flow->detected_protocol_stack[0] == IPOQUE_PROTOCOL_MANOLITO) {
			if (src != NULL) {
				src->manolito_last_pkt_arrival_time = packet->tick_timestamp;
			}
			if (dst != NULL) {
				dst->manolito_last_pkt_arrival_time = packet->tick_timestamp;
			}
			return;
		} else if (packet->udp->source == htons(41170)
				   || packet->udp->dest == htons(41170)) {
			if (src != NULL && src->manolito_last_pkt_arrival_time != 0
				&& (packet->tick_timestamp - src->manolito_last_pkt_arrival_time <
					ipoque_struct->manolito_subscriber_timeout)) {
				IPQ_LOG(IPOQUE_PROTOCOL_MANOLITO, ipoque_struct, IPQ_LOG_DEBUG, "MANOLITO: UDP detected \n");
				ipoque_int_manolito_add_connection(ipoque_struct);
				return;
			} else if (src != NULL
					   && (packet->tick_timestamp - src->manolito_last_pkt_arrival_time) >=
					   ipoque_struct->manolito_subscriber_timeout) {
				src->manolito_last_pkt_arrival_time = 0;
			}

			if (dst != NULL && dst->manolito_last_pkt_arrival_time != 0
				&& (packet->tick_timestamp - dst->manolito_last_pkt_arrival_time <
					ipoque_struct->manolito_subscriber_timeout)) {
				IPQ_LOG(IPOQUE_PROTOCOL_MANOLITO, ipoque_struct, IPQ_LOG_DEBUG, "MANOLITO: UDP detected \n");
				ipoque_int_manolito_add_connection(ipoque_struct);
				return;
			} else if (dst != NULL
					   && (packet->tick_timestamp - dst->manolito_last_pkt_arrival_time) >=
					   ipoque_struct->manolito_subscriber_timeout) {
				dst->manolito_last_pkt_arrival_time = 0;
			}

			if ((packet->payload_packet_len == 20 && htons(0x3d4b) == get_u16(packet->payload, 0)
				 && packet->payload[2] == 0xd9 && htons(0xedbb) == get_u16(packet->payload, 16))
				|| (packet->payload_packet_len == 25 && htons(0x3e4a) == get_u16(packet->payload, 0)
					&& htons(0x092f) == get_u16(packet->payload, 20) && packet->payload[22] == 0x20)
				|| (packet->payload_packet_len == 20 && !get_u16(packet->payload, 2) && !get_u32(packet->payload, 8)
					&& !get_u16(packet->payload, 18) && get_u16(packet->payload, 0))
				) {				//20B pkt is For PING
				IPQ_LOG(IPOQUE_PROTOCOL_MANOLITO, ipoque_struct, IPQ_LOG_DEBUG, "MANOLITO: UDP detected \n");
				ipoque_int_manolito_add_connection(ipoque_struct);
				return;
			} else if (flow->packet_counter < 7) {
				return;
			}
		}
	}

	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_MANOLITO);
}
#endif
