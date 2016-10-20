/*
 * gadu_gadu.c
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
#include "ndpi_api.h"

#ifdef NDPI_PROTOCOL_GADUGADU

static void ndpi_int_gadugadu_add_connection(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{
	ndpi_set_detected_protocol(ndpi_struct, flow, NDPI_PROTOCOL_GADUGADU, NDPI_PROTOCOL_UNKNOWN);
}

static void parse_gg_foneno(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{
	struct ndpi_packet_struct *packet = &flow->packet;
	struct ndpi_id_struct *src = flow->src;
	u_int16_t pos = 18;

	NDPI_LOG(NDPI_PROTOCOL_GADUGADU, ndpi_struct, NDPI_LOG_DEBUG, "Gadu-Gadu: parse_gg_foneno.\n");

	if (packet->payload_packet_len < 19) {
		return;
	}

	while (packet->payload[pos] != '?') {
		pos++;
		if ((pos + 18) > packet->payload_packet_len)
			return;
	}
	pos++;
	if (pos + 16 < packet->payload_packet_len) {
		char fmnumber[8];
		int i = 0;
		if (memcmp(&packet->payload[pos], "fmnumber=", 9) == 0) {
			NDPI_LOG(NDPI_PROTOCOL_GADUGADU, ndpi_struct, NDPI_LOG_DEBUG, "Gadu-Gadu: fmnumber found .\n");
		} else
			return;

		pos += 9;
		while (packet->payload[pos] != '&') {
			fmnumber[i] = packet->payload[pos];
			i++;
			pos++;
			if (pos > packet->payload_packet_len || i > 7)
				break;
		}
		if (i < 8) {
			fmnumber[i] = '\0';
			if (src != NULL) {
				memcpy(src->gg_fmnumber, fmnumber, i);
				NDPI_LOG(NDPI_PROTOCOL_GADUGADU, ndpi_struct, NDPI_LOG_DEBUG, "Gadu-Gadu: fmnumber %s\n", src->gg_fmnumber);
			}
		}
	}

}

static u_int8_t check_for_gadugadu_payload_pattern(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{
	struct ndpi_packet_struct *packet = &flow->packet;

	NDPI_LOG(NDPI_PROTOCOL_GADUGADU, ndpi_struct, NDPI_LOG_DEBUG, "Gadu-Gadu: check for 0xbebadec0 pattern in payload.\n");

	if (packet->payload_packet_len == 12) {
		if ((flow->packet_counter == 1) && (ntohl(get_u_int32_t(packet->payload, 0)) == 0xbebadec0)) {
			flow->l4.tcp.gadugadu_stage++;
			return 1;
		}
		if ((flow->l4.tcp.gadugadu_stage == 1) && (flow->packet_counter == 2)
		    && (ntohl(get_u_int32_t(packet->payload, 0)) == 0xbebadec0)) {
			NDPI_LOG(NDPI_PROTOCOL_GADUGADU, ndpi_struct, NDPI_LOG_DEBUG, "Gadu-Gadu: gadugadu pattern bebadec0 FOUND \n");

			ndpi_int_gadugadu_add_connection(ndpi_struct, flow);
			return 1;
		}
	}
	return 0;
}

static u_int8_t check_for_http(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{
	struct ndpi_packet_struct *packet = &flow->packet;

	NDPI_LOG(NDPI_PROTOCOL_GADUGADU, ndpi_struct, NDPI_LOG_DEBUG, "Gadu-Gadu: check for http.\n");

	if (packet->payload_packet_len < 50) {
		NDPI_LOG(NDPI_PROTOCOL_GADUGADU, ndpi_struct, NDPI_LOG_DEBUG, "Gadu-Gadu: Packet too small.\n");
		return 0;
	} else if (memcmp(packet->payload, "GET /appsvc/appmsg", 18) == 0) {
		NDPI_LOG(NDPI_PROTOCOL_GADUGADU, ndpi_struct, NDPI_LOG_DEBUG, "Gadu-Gadu: GET FOUND\n");
		parse_gg_foneno(ndpi_struct, flow);
		// parse packet
		ndpi_parse_packet_line_info(ndpi_struct, flow);
		if (packet->parsed_lines <= 1) {
			return 0;
		}
		if (packet->host_line.offs == 0xffff) {
			return 0;
		}
		if (!(packet->host_line.len >= 19 && memcmp(packet_hdr(host_line), "appmsg.gadu-gadu.pl", 19) == 0)) {
			return 0;
		}
		NDPI_LOG(NDPI_PROTOCOL_GADUGADU, ndpi_struct, NDPI_LOG_DEBUG, "Gadu-Gadu: Is gadugadu host FOUND %s\n", packet_hdr(host_line));

		ndpi_int_gadugadu_add_connection(ndpi_struct, flow);

	} else if (memcmp(packet->payload, "POST /send/message/", 15) == 0) {
		NDPI_LOG(NDPI_PROTOCOL_GADUGADU, ndpi_struct, NDPI_LOG_DEBUG, "Gadu-Gadu: GET FOUND\n");

		// parse packet
		ndpi_parse_packet_line_info(ndpi_struct, flow);
		if (packet->parsed_lines <= 1) {
			return 0;
		}
		if (packet->host_line.offs == 0xffff) {
			return 0;
		}
		if (!(packet->host_line.len >= 17 && memcmp(packet_hdr(host_line), "life.gadu-gadu.pl", 17) == 0)) {
			return 0;
		}
		NDPI_LOG(NDPI_PROTOCOL_GADUGADU, ndpi_struct, NDPI_LOG_DEBUG, "Gadu-Gadu: Is gadugadu post FOUND %s\n", packet_hdr(host_line));

		ndpi_int_gadugadu_add_connection(ndpi_struct, flow);

	} else if (memcmp(packet->payload, "GET /rotate_token", 17) == 0) {
		NDPI_LOG(NDPI_PROTOCOL_GADUGADU, ndpi_struct, NDPI_LOG_DEBUG, "Gadu-Gadu: GET FOUND\n");

		// parse packet
		ndpi_parse_packet_line_info(ndpi_struct, flow);
		if (packet->parsed_lines <= 1) {
			return 0;
		}
		if (packet->host_line.offs == 0xffff) {
			return 0;
		}
		if (!(packet->host_line.len >= 13 && memcmp(packet_hdr(host_line), "sms.orange.pl", 13) == 0)) {
			return 0;
		}
		NDPI_LOG(NDPI_PROTOCOL_GADUGADU, ndpi_struct, NDPI_LOG_DEBUG, "Gadu-Gadu:  gadugadu sms FOUND %s\n", packet_hdr(host_line));

		ndpi_int_gadugadu_add_connection(ndpi_struct, flow);

	} else if ((memcmp(packet->payload, "GET /nowosci.xml", NDPI_STATICSTRING_LEN("GET /nowosci.xml")) == 0) ||
		   (memcmp(packet->payload, "GET /gadu-gadu.xml", NDPI_STATICSTRING_LEN("GET /gadu-gadu.xml")) == 0) || (memcmp(packet->payload, "POST /access_token", NDPI_STATICSTRING_LEN("POST /access_token")) == 0)) {
		ndpi_parse_packet_line_info(ndpi_struct, flow);
		if (packet->user_agent_line.offs == 0xffff) {
			return 0;
		}
		if (!(packet->user_agent_line.len >= NDPI_STATICSTRING_LEN("Gadu-Gadu Client") && memcmp(packet_hdr(user_agent_line), "Gadu-Gadu Client", NDPI_STATICSTRING_LEN("Gadu-Gadu Client")) == 0)) {
			return 0;
		}
		NDPI_LOG(NDPI_PROTOCOL_GADUGADU, ndpi_struct, NDPI_LOG_DEBUG, "Gadu-Gadu:  gadugadu FOUND %s\n", packet_hdr(user_agent_line));

		ndpi_int_gadugadu_add_connection(ndpi_struct, flow);

	}

	return 1;

}

static void ndpi_search_gadugadu_tcp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{
	struct ndpi_packet_struct *packet = &flow->packet;

	struct ndpi_id_struct *src = flow->src;
	struct ndpi_id_struct *dst = flow->dst;

	if (packet->detected_protocol_stack[0] == NDPI_PROTOCOL_GADUGADU) {
		if (src != NULL)
			src->gg_timeout = packet->tick_timestamp;
		if (dst != NULL)
			dst->gg_timeout = packet->tick_timestamp;

		if (packet->payload_packet_len == 311) {
			if (packet->payload[28] != 0) {
				if (src != NULL) {
					src->gg_timeout = packet->tick_timestamp;
					if (ntohs(packet->tcp->dest) == 8074 || ntohs(packet->tcp->dest) == 443)
						src->gadu_gadu_ft_direction = 0;
					else
						src->gadu_gadu_ft_direction = 1;
					src->gadu_gadu_voice = 0;

				}
				if (dst != NULL) {
					dst->gg_timeout = packet->tick_timestamp;
					if (ntohs(packet->tcp->dest) == 8074 || ntohs(packet->tcp->dest) == 443)
						dst->gadu_gadu_ft_direction = 0;
					else
						dst->gadu_gadu_ft_direction = 1;
					dst->gadu_gadu_voice = 0;

				}

				NDPI_LOG(NDPI_PROTOCOL_GADUGADU, ndpi_struct, NDPI_LOG_DEBUG, "gg filetransfer setup detected\n");

			} else {
				if (src != NULL) {
					src->gadu_gadu_voice = 1;
					src->gg_timeout = packet->tick_timestamp;
				}
				if (dst != NULL) {
					dst->gadu_gadu_voice = 1;
					dst->gg_timeout = packet->tick_timestamp;
				}
				NDPI_LOG(NDPI_PROTOCOL_GADUGADU, ndpi_struct, NDPI_LOG_DEBUG, "gg voice setup detected \n");
			}
		}
		return;
	}
#ifdef NDPI_PROTOCOL_HTTP
	if (packet->detected_protocol_stack[0] == NDPI_PROTOCOL_HTTP) {
#endif
		NDPI_LOG(NDPI_PROTOCOL_GADUGADU, ndpi_struct, NDPI_LOG_DEBUG, "Gadu-Gadu: HTTP CHECK FOUND\n");
		if (packet->tcp != NULL && ntohs(packet->tcp->dest) == 80)
			if (check_for_http(ndpi_struct, flow))
				return;
#ifdef NDPI_PROTOCOL_HTTP
	}
#endif

	/* the following code is implemented asymmetrically. */
	if (packet->tcp != NULL && (ntohs(packet->tcp->dest) == 443 || ntohs(packet->tcp->dest) == 8074 || ntohs(packet->tcp->source) == 443 || ntohs(packet->tcp->source) == 8074)) {
		NDPI_LOG(NDPI_PROTOCOL_GADUGADU, ndpi_struct, NDPI_LOG_DEBUG, "Gadu-Gadu: found port 8074 or 443.\n");
		if (flow->packet_counter <= 6) {

			if ((packet->payload_packet_len == 9 || packet->payload_packet_len == 12 || packet->payload_packet_len == 100 || (packet->payload_packet_len > 190 && packet->payload_packet_len < 210)
			    )
			    && get_l32(packet->payload, 4) == packet->payload_packet_len - 8
			    && (ntohl(get_u_int32_t(packet->payload, 0)) == 0x01000000
				|| ntohl(get_u_int32_t(packet->payload, 0)) == 0x02000000
				|| ntohl(get_u_int32_t(packet->payload, 0)) == 0x03000000
				|| ntohl(get_u_int32_t(packet->payload, 0)) == 0x12000000
				|| ntohl(get_u_int32_t(packet->payload, 0)) == 0x19000000
				|| ntohl(get_u_int32_t(packet->payload, 0)) == 0x31000000
				|| ntohl(get_u_int32_t(packet->payload, 0)) == 0x35000000 || ntohl(get_u_int32_t(packet->payload, 0)) == 0x10000000 || ntohl(get_u_int32_t(packet->payload, 0)) == 0x15000000)) {
				flow->l4.tcp.gadugadu_stage++;
				NDPI_LOG(NDPI_PROTOCOL_GADUGADU, ndpi_struct, NDPI_LOG_DEBUG, "Gadu-Gadu: len=9,12,100,190-210, stage++.\n");
			}

			/*detection of mirinda client .this has a different way of communicating ports */
			if (packet->payload_packet_len == 114 && ntohl(get_u_int32_t(packet->payload, 0)) == 0x19000000 && get_l32(packet->payload, 4) == packet->payload_packet_len - 8) {
				flow->l4.tcp.gadugadu_stage++;
				NDPI_LOG(NDPI_PROTOCOL_GADUGADU, ndpi_struct, NDPI_LOG_DEBUG, "Gadu-Gadu: len=114, stage++.\n");
				/* here the asymmetric implementation ends */

				if (flow->l4.tcp.gadugadu_stage == 2) {
					if (src != NULL) {

						memcpy(src->gg_call_id[src->gg_next_id], &packet->payload[8], 4);
						NDPI_LOG(NDPI_PROTOCOL_GADUGADU, ndpi_struct, NDPI_LOG_DEBUG, "call id parsed %d\n", packet->payload[8]);

						src->gg_ft_ip_address = get_u_int32_t(packet->payload, 86);
						src->gg_ft_port = htons(get_u_int16_t(packet->payload, 90));
						NDPI_LOG(NDPI_PROTOCOL_GADUGADU, ndpi_struct, NDPI_LOG_DEBUG, "mirinda file transfer port %d \n", ntohs(src->gg_ft_port));
					}
					if (dst != NULL) {

						memcpy(dst->gg_call_id[dst->gg_next_id], &packet->payload[8], 4);
						NDPI_LOG(NDPI_PROTOCOL_GADUGADU, ndpi_struct, NDPI_LOG_DEBUG, "call id parsed %d\n", packet->payload[8]);

						dst->gg_ft_ip_address = get_u_int32_t(packet->payload, 86);
						dst->gg_ft_port = htons(get_u_int16_t(packet->payload, 90));

						NDPI_LOG(NDPI_PROTOCOL_GADUGADU, ndpi_struct, NDPI_LOG_DEBUG, "mirinda file transfer port %d \n", ntohs(dst->gg_ft_port));
					}
				}
			}

			if (flow->l4.tcp.gadugadu_stage == 2) {
				NDPI_LOG(NDPI_PROTOCOL_GADUGADU, ndpi_struct, NDPI_LOG_DEBUG, "Gadu-Gadu: add connection.\n");

				ndpi_int_gadugadu_add_connection(ndpi_struct, flow);
			}
			return;
		}

	}
	/*mirinda file detection */
	if (packet->tcp != NULL && src != NULL) {
		if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(src->detected_protocol_bitmask, NDPI_PROTOCOL_GADUGADU) != 0 && ((src->gg_ft_ip_address == packet->iph->saddr && src->gg_ft_port == packet->tcp->source)
														      || (src->gg_ft_ip_address == packet->iph->daddr && src->gg_ft_port == packet->tcp->dest))) {
			if ((packet->tick_timestamp - src->gg_timeout) < ndpi_struct->gadugadu_peer_connection_timeout) {

				ndpi_int_gadugadu_add_connection(ndpi_struct, flow);

				NDPI_LOG(NDPI_PROTOCOL_GADUGADU, ndpi_struct, NDPI_LOG_DEBUG, "file transfer detected %d\n", ntohs(packet->tcp->dest));
				return;
			} else {
				src->gg_ft_ip_address = 0;
				src->gg_ft_port = 0;
			}
		} else if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(src->detected_protocol_bitmask, NDPI_PROTOCOL_GADUGADU) != 0 && (packet->tcp->dest == htons(80)
															     || packet->tcp->source == htons(80))
			   && packet->payload_packet_len == 12 && (memcmp(src->gg_call_id[0], &packet->payload[5], 4) == 0 || (src->gg_call_id[1][0]
															       && (memcmp(src->gg_call_id[1], &packet->payload[5], 4)
																   == 0)))) {
			if ((packet->tick_timestamp - src->gg_timeout) < ndpi_struct->gadugadu_peer_connection_timeout) {

				ndpi_int_gadugadu_add_connection(ndpi_struct, flow);

				NDPI_LOG(NDPI_PROTOCOL_GADUGADU, ndpi_struct, NDPI_LOG_DEBUG, "http file transfer detetced \n");
				return;
			} else {
				NDPI_LOG(NDPI_PROTOCOL_GADUGADU, ndpi_struct, NDPI_LOG_DEBUG, "http file transfer timeout \n");

			}

		} else if (NDPI_COMPARE_PROTOCOL_TO_BITMASK
			   (src->detected_protocol_bitmask, NDPI_PROTOCOL_GADUGADU) != 0 && packet->payload_packet_len == 8 && (memcmp(src->gg_call_id[0], &packet->payload[0], 4) == 0 || (src->gg_call_id[1][0]
																							    &&
																							    (memcmp
																							     (src->gg_call_id[1],
																							      &packet->payload[0], 4)
																							     == 0)))) {
			if ((packet->tick_timestamp - src->gg_timeout) < ndpi_struct->gadugadu_peer_connection_timeout) {

				ndpi_int_gadugadu_add_connection(ndpi_struct, flow);

				NDPI_LOG(NDPI_PROTOCOL_GADUGADU, ndpi_struct, NDPI_LOG_DEBUG, "file transfer detetced %d\n", htons(packet->tcp->dest));
				return;
			} else {
				NDPI_LOG(NDPI_PROTOCOL_GADUGADU, ndpi_struct, NDPI_LOG_DEBUG, " file transfer timeout \n");
			}
		}
	}

	if (packet->tcp != NULL && dst != NULL) {
		if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(dst->detected_protocol_bitmask, NDPI_PROTOCOL_GADUGADU) != 0 && ((dst->gg_ft_ip_address == packet->iph->saddr && dst->gg_ft_port == packet->tcp->source)
														      || (dst->gg_ft_ip_address == packet->iph->daddr && dst->gg_ft_port == packet->tcp->dest))) {
			if ((packet->tick_timestamp - dst->gg_timeout) < ndpi_struct->gadugadu_peer_connection_timeout) {

				ndpi_int_gadugadu_add_connection(ndpi_struct, flow);

				NDPI_LOG(NDPI_PROTOCOL_GADUGADU, ndpi_struct, NDPI_LOG_DEBUG, "file transfer detected %d\n", ntohs(packet->tcp->dest));
				return;
			} else {
				dst->gg_ft_ip_address = 0;
				dst->gg_ft_port = 0;
			}
		} else if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(dst->detected_protocol_bitmask, NDPI_PROTOCOL_GADUGADU) != 0 && (packet->tcp->dest == htons(80)
															     || packet->tcp->source == htons(80))
			   && packet->payload_packet_len == 12 && (memcmp(dst->gg_call_id[0], &packet->payload[0], 4) == 0 || (dst->gg_call_id[1][0]
															       && (memcmp(dst->gg_call_id[1], &packet->payload[0], 4)
																   == 0)))) {
			if ((packet->tick_timestamp - dst->gg_timeout) < ndpi_struct->gadugadu_peer_connection_timeout) {

				ndpi_int_gadugadu_add_connection(ndpi_struct, flow);

				NDPI_LOG(NDPI_PROTOCOL_GADUGADU, ndpi_struct, NDPI_LOG_DEBUG, "http file transfer detetced \n");
				return;
			} else {
				NDPI_LOG(NDPI_PROTOCOL_GADUGADU, ndpi_struct, NDPI_LOG_DEBUG, "http file transfer timeout \n");

			}

		} else if (NDPI_COMPARE_PROTOCOL_TO_BITMASK
			   (dst->detected_protocol_bitmask, NDPI_PROTOCOL_GADUGADU) != 0 && packet->payload_packet_len == 8 && (memcmp(dst->gg_call_id[0], &packet->payload[0], 4) == 0 || (dst->gg_call_id[1][0]
																							    &&
																							    (memcmp
																							     (dst->gg_call_id[1],
																							      &packet->payload[0], 4)
																							     == 0)))) {
			if ((packet->tick_timestamp - dst->gg_timeout) < ndpi_struct->gadugadu_peer_connection_timeout) {

				ndpi_int_gadugadu_add_connection(ndpi_struct, flow);

				NDPI_LOG(NDPI_PROTOCOL_GADUGADU, ndpi_struct, NDPI_LOG_DEBUG, "file transfer detected %d\n", ntohs(packet->tcp->dest));
				return;
			} else {
				NDPI_LOG(NDPI_PROTOCOL_GADUGADU, ndpi_struct, NDPI_LOG_DEBUG, " file transfer timeout \n");
			}
		}
	}
  /** newly added start **/
	if (packet->tcp != NULL && ((ntohs(packet->tcp->dest) == 80) || (ntohs(packet->tcp->source) == 80))) {
		if (check_for_gadugadu_payload_pattern(ndpi_struct, flow)) {
			return;
		}
	}

	NDPI_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, NDPI_PROTOCOL_GADUGADU);

}

static void ndpi_search_gadugadu(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{
	ndpi_search_gadugadu_tcp(ndpi_struct, flow);
}

static void init_gadugadu_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id, NDPI_PROTOCOL_BITMASK * detection_bitmask)
{
	ndpi_set_bitmask_protocol_detection("GADUGADU", ndpi_struct, detection_bitmask, *id,
					    NDPI_PROTOCOL_GADUGADU, ndpi_search_gadugadu, NDPI_SELECTION_BITMASK_PROTOCOL_TCP_OR_UDP_WITH_PAYLOAD, SAVE_DETECTION_BITMASK_AS_UNKNOWN, ADD_TO_DETECTION_BITMASK);
	*id += 1;
#ifdef NDPI_PROTOCOL_HTTP
	NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[*id].detection_bitmask, NDPI_PROTOCOL_HTTP);
#endif
	NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[*id].excluded_protocol_bitmask, NDPI_PROTOCOL_GADUGADU);

	*id += 1;
}



#endif
