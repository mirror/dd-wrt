/*
 * mail_smtp.c
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

#ifdef IPOQUE_PROTOCOL_MAIL_SMTP

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

static void ipoque_search_mail_smtp_tcp(struct ipoque_detection_module_struct
								 *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
	const u8 *p, *end, *line;
	int len;


	IPQ_LOG(IPOQUE_PROTOCOL_MAIL_SMTP, ipoque_struct, IPQ_LOG_DEBUG, "search mail_smtp.\n");


	if (packet->payload_packet_len > 2 && ntohs(get_u16(packet->payload, packet->payload_packet_len - 2)) == 0x0d0a) {

		u8 a;
		u8 bit_count = 0;

		for (p = packet->payload, end = p + packet->payload_packet_len;
		     get_next_line(&p, end, &line, &len);) {

			// expected server responses
			if (len >= 3) {
				if (memcmp(line, "220", 3) == 0) {
					flow->smtp_command_bitmask |= SMTP_BIT_220;
				} else if (memcmp(line, "250", 3) == 0) {
					flow->smtp_command_bitmask |= SMTP_BIT_250;
				} else if (memcmp(line, "235", 3) == 0) {
					flow->smtp_command_bitmask |= SMTP_BIT_235;
				} else if (memcmp(line, "334", 3) == 0) {
					flow->smtp_command_bitmask |= SMTP_BIT_334;
				} else if (memcmp(line, "354", 3) == 0) {
					flow->smtp_command_bitmask |= SMTP_BIT_354;
				}
			}
			// expected client requests
			if (len >= 5) {
				if ((((line[0] == 'H' || line[0] == 'h')
					  && (line[1] == 'E' || line[1] == 'e'))
					 || ((line[0] == 'E' || line[0] == 'e')
						 && (line[1] == 'H' || line[1] == 'h')))
					&& (line[2] == 'L' || line[2] == 'l')
					&& (line[3] == 'O' || line[3] == 'o')
					&& line[4] == ' ') {
					flow->smtp_command_bitmask |= SMTP_BIT_HELO_EHLO;
				} else if ((line[0] == 'M' || line[0] == 'm')
						   && (line[1] == 'A' || line[1] == 'a')
						   && (line[2] == 'I' || line[2] == 'i')
						   && (line[3] == 'L' || line[3] == 'l')
						   && line[4] == ' ') {
					flow->smtp_command_bitmask |= SMTP_BIT_MAIL;
				} else if ((line[0] == 'R' || line[0] == 'r')
						   && (line[1] == 'C' || line[1] == 'c')
						   && (line[2] == 'P' || line[2] == 'p')
						   && (line[3] == 'T' || line[3] == 't')
						   && line[4] == ' ') {
					flow->smtp_command_bitmask |= SMTP_BIT_RCPT;
				} else if ((line[0] == 'A' || line[0] == 'a')
						   && (line[1] == 'U' || line[1] == 'u')
						   && (line[2] == 'T' || line[2] == 't')
						   && (line[3] == 'H' || line[3] == 'h')
						   && line[4] == ' ') {
					flow->smtp_command_bitmask |= SMTP_BIT_AUTH;
				}
			}

			if (len >= 8) {
				if ((line[0] == 'S' || line[0] == 's')
					&& (line[1] == 'T' || line[1] == 't')
					&& (line[2] == 'A' || line[2] == 'a')
					&& (line[3] == 'R' || line[3] == 'r')
					&& (line[4] == 'T' || line[0] == 't')
					&& (line[5] == 'T' || line[1] == 't')
					&& (line[6] == 'L' || line[2] == 'l')
					&& (line[7] == 'S' || line[3] == 's')) {
					flow->smtp_command_bitmask |= SMTP_BIT_STARTTLS;
				}
			}

			if (len >= 4) {
				if ((line[0] == 'D' || line[0] == 'd')
					&& (line[1] == 'A' || line[1] == 'a')
					&& (line[2] == 'T' || line[2] == 't')
					&& (line[3] == 'A' || line[3] == 'a')) {
					flow->smtp_command_bitmask |= SMTP_BIT_DATA;
				} else if ((line[0] == 'N' || line[0] == 'n')
						   && (line[1] == 'O' || line[1] == 'o')
						   && (line[2] == 'O' || line[2] == 'o')
						   && (line[3] == 'P' || line[3] == 'p')) {
					flow->smtp_command_bitmask |= SMTP_BIT_NOOP;
				} else if ((line[0] == 'R' || line[0] == 'r')
						   && (line[1] == 'S' || line[1] == 's')
						   && (line[2] == 'E' || line[2] == 'e')
						   && (line[3] == 'T' || line[3] == 't')) {
					flow->smtp_command_bitmask |= SMTP_BIT_RSET;
				}
			}

		}

		// now count the bits set in the bitmask
		if (flow->smtp_command_bitmask != 0) {
			for (a = 0; a < 16; a++) {
				bit_count += (flow->smtp_command_bitmask >> a) & 0x01;
			}
		}
		IPQ_LOG(IPOQUE_PROTOCOL_MAIL_SMTP, ipoque_struct, IPQ_LOG_DEBUG, "seen smtp commands and responses: %u.\n",
				bit_count);

		if (bit_count >= 3) {
			IPQ_LOG(IPOQUE_PROTOCOL_MAIL_SMTP, ipoque_struct, IPQ_LOG_DEBUG, "mail smtp identified\n");
			ipq_connection_detected(ipoque_struct, IPOQUE_PROTOCOL_MAIL_SMTP);
			return;
		}
		if (bit_count >= 1 && flow->packet_counter < 12) {
			return;
		}
	}
	IPQ_LOG(IPOQUE_PROTOCOL_MAIL_SMTP, ipoque_struct, IPQ_LOG_DEBUG, "exclude smtp\n");
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_MAIL_SMTP);

}
#endif
