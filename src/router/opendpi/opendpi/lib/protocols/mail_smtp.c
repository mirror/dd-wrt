/*
 * mail_smtp.c
 *
 * Copyright (C) 2009-2011 by ipoque GmbH
 * Copyright (C) 2011-15 - ntop.org
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

#ifdef NDPI_PROTOCOL_MAIL_SMTP

#define SMTP_BIT_220		0x01
#define SMTP_BIT_250		0x02
#define SMTP_BIT_235		0x04
#define SMTP_BIT_334		0x08
#define SMTP_BIT_354		0x10
#define SMTP_BIT_HELO_EHLO	0x20
#define SMTP_BIT_MAIL		0x40
#define SMTP_BIT_RCPT		0x80
#define SMTP_BIT_AUTH		0x100
#define SMTP_BIT_STARTTLS	0x200
#define SMTP_BIT_DATA		0x400
#define SMTP_BIT_NOOP		0x800
#define SMTP_BIT_RSET		0x1000
#define SMTP_BIT_TlRM		0x2000

static void ndpi_int_mail_smtp_add_connection(struct ndpi_detection_module_struct
					      *ndpi_struct, struct ndpi_flow_struct *flow)
{
	ndpi_set_detected_protocol(ndpi_struct, flow, NDPI_PROTOCOL_MAIL_SMTP, NDPI_PROTOCOL_UNKNOWN);
}

static void ndpi_search_mail_smtp_tcp(struct ndpi_detection_module_struct
				      *ndpi_struct, struct ndpi_flow_struct *flow)
{
	struct ndpi_packet_struct *packet = &flow->packet;

	//  struct ndpi_id_struct         *src=ndpi_struct->src;
	//  struct ndpi_id_struct         *dst=ndpi_struct->dst;

	NDPI_LOG(NDPI_PROTOCOL_MAIL_SMTP, ndpi_struct, NDPI_LOG_DEBUG, "search mail_smtp.\n");

	if (packet->payload_packet_len > 2 && ntohs(get_u_int16_t(packet->payload, packet->payload_packet_len - 2)) == 0x0d0a) {
		u_int8_t a;
		u_int8_t bit_count = 0;

		NDPI_PARSE_PACKET_LINE_INFO(ndpi_struct, flow, packet);
		for (a = 0; a < packet->parsed_lines; a++) {
			const char *s = packet_line(a);

			// expected server responses
			if (packet->line[a].len >= 3) {
				if (memcmp(s, "220", 3) == 0) {
					flow->l4.tcp.smtp_command_bitmask |= SMTP_BIT_220;
				} else if (memcmp(s, "250", 3) == 0) {
					flow->l4.tcp.smtp_command_bitmask |= SMTP_BIT_250;
				} else if (memcmp(s, "235", 3) == 0) {
					flow->l4.tcp.smtp_command_bitmask |= SMTP_BIT_235;
				} else if (memcmp(s, "334", 3) == 0) {
					flow->l4.tcp.smtp_command_bitmask |= SMTP_BIT_334;
				} else if (memcmp(s, "354", 3) == 0) {
					flow->l4.tcp.smtp_command_bitmask |= SMTP_BIT_354;
				}
			}
			// expected client requests
			if (packet->line[a].len >= 5) {
				if ((((s[0] == 'H' || s[0] == 'h')
				      && (s[1] == 'E' || s[1] == 'e'))
				     || ((s[0] == 'E' || s[0] == 'e')
					 && (s[1] == 'H' || s[1] == 'h')))
				    && (s[2] == 'L' || s[2] == 'l')
				    && (s[3] == 'O' || s[3] == 'o')
				    && s[4] == ' ') {
					flow->l4.tcp.smtp_command_bitmask |= SMTP_BIT_HELO_EHLO;
				} else if ((s[0] == 'M' || s[0] == 'm')
					   && (s[1] == 'A' || s[1] == 'a')
					   && (s[2] == 'I' || s[2] == 'i')
					   && (s[3] == 'L' || s[3] == 'l')
					   && s[4] == ' ') {
					flow->l4.tcp.smtp_command_bitmask |= SMTP_BIT_MAIL;
				} else if ((s[0] == 'R' || s[0] == 'r')
					   && (s[1] == 'C' || s[1] == 'c')
					   && (s[2] == 'P' || s[2] == 'p')
					   && (s[3] == 'T' || s[3] == 't')
					   && s[4] == ' ') {
					flow->l4.tcp.smtp_command_bitmask |= SMTP_BIT_RCPT;
				} else if ((s[0] == 'A' || s[0] == 'a')
					   && (s[1] == 'U' || s[1] == 'u')
					   && (s[2] == 'T' || s[2] == 't')
					   && (s[3] == 'H' || s[3] == 'h')
					   && s[4] == ' ') {
					flow->l4.tcp.smtp_command_bitmask |= SMTP_BIT_AUTH;
				}
			}

			if (packet->line[a].len >= 8) {
				if ((s[0] == 'S' || s[0] == 's')
				    && (s[1] == 'T' || s[1] == 't')
				    && (s[2] == 'A' || s[2] == 'a')
				    && (s[3] == 'R' || s[3] == 'r')
				    && (s[4] == 'T' || s[4] == 't')
				    && (s[5] == 'T' || s[5] == 't')
				    && (s[6] == 'L' || s[6] == 'l')
				    && (s[7] == 'S' || s[7] == 's')) {
					flow->l4.tcp.smtp_command_bitmask |= SMTP_BIT_STARTTLS;
				}
			}

			if (packet->line[a].len >= 4) {
				if ((s[0] == 'D' || s[0] == 'd')
				    && (s[1] == 'A' || s[1] == 'a')
				    && (s[2] == 'T' || s[2] == 't')
				    && (s[3] == 'A' || s[3] == 'a')) {
					flow->l4.tcp.smtp_command_bitmask |= SMTP_BIT_DATA;
				} else if ((s[0] == 'N' || s[0] == 'n')
					   && (s[1] == 'O' || s[1] == 'o')
					   && (s[2] == 'O' || s[2] == 'o')
					   && (s[3] == 'P' || s[3] == 'p')) {
					flow->l4.tcp.smtp_command_bitmask |= SMTP_BIT_NOOP;
				} else if ((s[0] == 'R' || s[0] == 'r')
					   && (s[1] == 'S' || s[1] == 's')
					   && (s[2] == 'E' || s[2] == 'e')
					   && (s[3] == 'T' || s[3] == 't')) {
					flow->l4.tcp.smtp_command_bitmask |= SMTP_BIT_RSET;
				}
			}

		}

		// now count the bits set in the bitmask
		if (flow->l4.tcp.smtp_command_bitmask != 0) {
			for (a = 0; a < 16; a++) {
				bit_count += (flow->l4.tcp.smtp_command_bitmask >> a) & 0x01;
			}
		}
		NDPI_LOG(NDPI_PROTOCOL_MAIL_SMTP, ndpi_struct, NDPI_LOG_DEBUG, "seen smtp commands and responses: %u.\n", bit_count);

		if (bit_count >= 3) {
			NDPI_LOG(NDPI_PROTOCOL_MAIL_SMTP, ndpi_struct, NDPI_LOG_DEBUG, "mail smtp identified\n");
			ndpi_int_mail_smtp_add_connection(ndpi_struct, flow);
			return;
		}
		if (bit_count >= 1 && flow->packet_counter < 12) {
			return;
		}
	}
	/* when the first or second packets are split into two packets, those packets are ignored. */
	if (flow->packet_counter <= 4 &&
	    packet->payload_packet_len >= 4 && (ntohs(get_u_int16_t(packet->payload, packet->payload_packet_len - 2)) == 0x0d0a || memcmp(packet->payload, "220", 3) == 0 || memcmp(packet->payload, "EHLO", 4) == 0)) {
		NDPI_LOG(NDPI_PROTOCOL_MAIL_SMTP, ndpi_struct, NDPI_LOG_DEBUG, "maybe SMTP, need next packet.\n");
		return;
	}

	NDPI_LOG(NDPI_PROTOCOL_MAIL_SMTP, ndpi_struct, NDPI_LOG_DEBUG, "exclude smtp\n");
	NDPI_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, NDPI_PROTOCOL_MAIL_SMTP);

}

static void init_mail_smtp_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id, NDPI_PROTOCOL_BITMASK * detection_bitmask)
{
	ndpi_set_bitmask_protocol_detection("MAIL_SMTP", ndpi_struct, detection_bitmask, *id,
					    NDPI_PROTOCOL_MAIL_SMTP,
					    ndpi_search_mail_smtp_tcp, NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION, SAVE_DETECTION_BITMASK_AS_UNKNOWN, ADD_TO_DETECTION_BITMASK);

	*id += 1;
}

#endif
