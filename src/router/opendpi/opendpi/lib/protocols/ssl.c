/*
 * ssl.c
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

#ifdef IPOQUE_PROTOCOL_SSL

#define IPOQUE_MAX_SSL_REQUEST_SIZE 10000

static void ipoque_int_ssl_add_connection(struct ipoque_detection_module_struct
										  *ipoque_struct, u32 protocol)
{

	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
	struct ipoque_id_struct *src = ipoque_struct->src;
	struct ipoque_id_struct *dst = ipoque_struct->dst;

	flow->detected_protocol = protocol;
	packet->detected_protocol = protocol;

	if (src != NULL) {
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(src->detected_protocol_bitmask, protocol);
	}
	if (dst != NULL) {
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(dst->detected_protocol_bitmask, protocol);
	}
}

static void ssl_mark_and_payload_search_for_other_protocols(struct
															ipoque_detection_module_struct
															*ipoque_struct)
{
#if defined(IPOQUE_PROTOCOL_SOFTETHER) || defined(IPOQUE_PROTOCOL_MEEBO)|| defined(IPOQUE_PROTOCOL_TOR) || defined(IPOQUE_PROTOCOL_VPN_X) || defined(IPOQUE_PROTOCOL_UNENCRYPED_JABBER) || defined (IPOQUE_PROTOCOL_OOVOO) || defined (IPOQUE_PROTOCOL_ISKOOT) || defined (IPOQUE_PROTOCOL_OSCAR)

	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
#ifdef IPOQUE_PROTOCOL_ISKOOT
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
#endif
//      struct ipoque_id_struct         *src=ipoque_struct->src;
//      struct ipoque_id_struct         *dst=ipoque_struct->dst;
	u32 a;
#if defined ( IPOQUE_PROTOCOL_TOR) || defined (IPOQUE_PROTOCOL_ISKOOT)
	u32 b;
#endif

	u32 end;
#if defined(IPOQUE_PROTOCOL_UNENCRYPED_JABBER)
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(ipoque_struct->detection_bitmask, IPOQUE_PROTOCOL_UNENCRYPED_JABBER) != 0)
		goto check_for_ssl_payload;
#endif
#if defined(IPOQUE_PROTOCOL_OSCAR)
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(ipoque_struct->detection_bitmask, IPOQUE_PROTOCOL_OSCAR) != 0)
		goto check_for_ssl_payload;
#endif

	goto no_check_for_ssl_payload;

  check_for_ssl_payload:
	end = packet->payload_packet_len - 20;
	for (a = 5; a < end; a++) {
#ifdef IPOQUE_PROTOCOL_UNENCRYPED_JABBER
		if (packet->payload[a] == 't') {
			if (memcmp(&packet->payload[a], "talk.google.com", 15) == 0) {
				IPQ_LOG(IPOQUE_PROTOCOL_UNENCRYPED_JABBER, ipoque_struct, IPQ_LOG_DEBUG, "ssl jabber packet match\n");
				if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK
					(ipoque_struct->detection_bitmask, IPOQUE_PROTOCOL_UNENCRYPED_JABBER) != 0) {
					ipoque_int_ssl_add_connection(ipoque_struct, IPOQUE_PROTOCOL_UNENCRYPED_JABBER);
					return;
				}
			}
		}
#endif
#ifdef IPOQUE_PROTOCOL_OSCAR
		if (packet->payload[a] == 'A' || packet->payload[a] == 'k' || packet->payload[a] == 'c'
			|| packet->payload[a] == 'h') {
			if (((a + 19) < packet->payload_packet_len && memcmp(&packet->payload[a], "America Online Inc.", 19) == 0)
				//                        || (end - c > 3 memcmp (&packet->payload[c],"AOL", 3) == 0 )
				//                        || (end - c > 7 && memcmp (&packet->payload[c], "AOL LLC", 7) == 0)
				|| ((a + 15) < packet->payload_packet_len && memcmp(&packet->payload[a], "kdc.uas.aol.com", 15) == 0)
				|| ((a + 14) < packet->payload_packet_len && memcmp(&packet->payload[a], "corehc@aol.net", 14) == 0)
				|| ((a + 41) < packet->payload_packet_len
					&& memcmp(&packet->payload[a], "http://crl.aol.com/AOLMSPKI/aolServerCert", 41) == 0)
				|| ((a + 28) < packet->payload_packet_len
					&& memcmp(&packet->payload[a], "http://ocsp.web.aol.com/ocsp", 28) == 0)
				|| ((a + 32) < packet->payload_packet_len
					&& memcmp(&packet->payload[a], "http://pki-info.aol.com/AOLMSPKI", 32) == 0)) {
				IPQ_LOG(IPOQUE_PROTOCOL_OSCAR, ipoque_struct, IPQ_LOG_DEBUG, "OSCAR SERVER SSL DETECTED\n");

				if (ipoque_struct->dst != NULL && packet->payload_packet_len > 75) {
					memcpy(ipoque_struct->dst->oscar_ssl_session_id, &packet->payload[44], 32);
					ipoque_struct->dst->oscar_ssl_session_id[32] = '\0';
					ipoque_struct->dst->oscar_last_safe_access_time = packet->tick_timestamp;
				}

				ipoque_int_ssl_add_connection(ipoque_struct, IPOQUE_PROTOCOL_OSCAR);
				return;
			}
		}

		if (packet->payload[a] == 'm' || packet->payload[a] == 's') {
			if ((a + 21) < packet->payload_packet_len &&
				(memcmp(&packet->payload[a], "my.screenname.aol.com", 21) == 0
				 || memcmp(&packet->payload[a], "sns-static.aolcdn.com", 21) == 0)) {
				IPQ_LOG(IPOQUE_PROTOCOL_OSCAR, ipoque_struct, IPQ_LOG_DEBUG, "OSCAR SERVER SSL DETECTED\n");
				ipoque_int_ssl_add_connection(ipoque_struct, IPOQUE_PROTOCOL_OSCAR);
				return;
			}
		}
#endif
	}
  no_check_for_ssl_payload:
#endif
	IPQ_LOG(IPOQUE_PROTOCOL_SSL, ipoque_struct, IPQ_LOG_DEBUG, "found ssl connection.\n");
	ipoque_int_ssl_add_connection(ipoque_struct, IPOQUE_PROTOCOL_SSL);
}


static u8 ipoque_search_sslv3_direction1(struct ipoque_detection_module_struct *ipoque_struct)
{

	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
//  struct ipoque_flow_struct *flow = ipoque_struct->flow;
//      struct ipoque_id_struct         *src=ipoque_struct->src;
//      struct ipoque_id_struct         *dst=ipoque_struct->dst;


	if (packet->payload_packet_len >= 5 && packet->payload[0] == 0x16 && packet->payload[1] == 0x03
		&& (packet->payload[2] == 0x00 || packet->payload[2] == 0x01 || packet->payload[2] == 0x02)) {
		u32 temp;
		IPQ_LOG(IPOQUE_PROTOCOL_SSL, ipoque_struct, IPQ_LOG_DEBUG, "search sslv3\n");
		// SSLv3 Record
		if (packet->payload_packet_len >= 1300) {
			return 1;
		}
		temp = ntohs(get_u16(packet->payload, 3)) + 5;
		IPQ_LOG(IPOQUE_PROTOCOL_SSL, ipoque_struct, IPQ_LOG_DEBUG, "temp = %u.\n", temp);
		if (packet->payload_packet_len == temp
			|| (temp < packet->payload_packet_len && packet->payload_packet_len > 500)) {
			return 1;
		}

		if (packet->payload_packet_len < temp && temp < 5000 && packet->payload_packet_len > 9) {
			/* the server hello maybe split into small packets */
			u32 cert_start;

			IPQ_LOG(IPOQUE_PROTOCOL_SSL, ipoque_struct, IPQ_LOG_DEBUG,
					"maybe SSLv3 server hello split into smaller packets\n", temp);

			/* lets hope at least the server hello and the start of the certificate block are in the first packet */
			cert_start = ntohs(get_u16(packet->payload, 7)) + 5 + 4;
			IPQ_LOG(IPOQUE_PROTOCOL_SSL, ipoque_struct, IPQ_LOG_DEBUG, "suspected start of certificate: %u\n",
					cert_start);

			if (cert_start < packet->payload_packet_len && packet->payload[cert_start] == 0x0b) {
				IPQ_LOG(IPOQUE_PROTOCOL_SSL, ipoque_struct, IPQ_LOG_DEBUG,
						"found 0x0b at suspected start of certificate block\n", cert_start);
				return 1;
			}
		}


		if (packet->payload_packet_len >= temp + 5 && (packet->payload[temp] == 0x14 || packet->payload[temp] == 0x16)
			&& packet->payload[temp + 1] == 0x03) {
			u32 temp2 = ntohs(get_u16(packet->payload, temp + 3)) + 5;
			if (temp + temp2 > IPOQUE_MAX_SSL_REQUEST_SIZE) {
				return 1;
			}
			temp += temp2;
			IPQ_LOG(IPOQUE_PROTOCOL_SSL, ipoque_struct, IPQ_LOG_DEBUG, "temp = %u.\n", temp);
			if (packet->payload_packet_len == temp) {
				return 1;
			}
			if (packet->payload_packet_len >= temp + 5 &&
				packet->payload[temp] == 0x16 && packet->payload[temp + 1] == 0x03) {
				temp2 = ntohs(get_u16(packet->payload, temp + 3)) + 5;
				if (temp + temp2 > IPOQUE_MAX_SSL_REQUEST_SIZE) {
					return 1;
				}
				temp += temp2;
				IPQ_LOG(IPOQUE_PROTOCOL_SSL, ipoque_struct, IPQ_LOG_DEBUG, "temp = %u.\n", temp);
				if (packet->payload_packet_len == temp) {
					return 1;
				}
				if (packet->payload_packet_len >= temp + 5 &&
					packet->payload[temp] == 0x16 && packet->payload[temp + 1] == 0x03) {
					temp2 = ntohs(get_u16(packet->payload, temp + 3)) + 5;
					if (temp + temp2 > IPOQUE_MAX_SSL_REQUEST_SIZE) {
						return 1;
					}
					temp += temp2;
					IPQ_LOG(IPOQUE_PROTOCOL_SSL, ipoque_struct, IPQ_LOG_DEBUG, "temp = %u.\n", temp);
					if (temp == packet->payload_packet_len) {
						return 1;
					}
				}

			}


		}

	}
	return 0;

}

void ipoque_search_ssl_tcp(struct ipoque_detection_module_struct *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
//      struct ipoque_id_struct         *src=ipoque_struct->src;
//      struct ipoque_id_struct         *dst=ipoque_struct->dst;


	IPQ_LOG(IPOQUE_PROTOCOL_SSL, ipoque_struct, IPQ_LOG_DEBUG, "search ssl\n");

	if (packet->payload_packet_len > 40 && flow->ssl_stage == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_SSL, ipoque_struct, IPQ_LOG_DEBUG, "first ssl packet\n");
		// SSLv2 Record
		if (packet->payload[2] == 0x01 && packet->payload[3] == 0x03
			&& (packet->payload[4] == 0x00 || packet->payload[4] == 0x01 || packet->payload[4] == 0x02)
			&& (packet->payload_packet_len - packet->payload[1] == 2)) {
			IPQ_LOG(IPOQUE_PROTOCOL_SSL, ipoque_struct, IPQ_LOG_DEBUG, "sslv2 len match\n");
			flow->ssl_stage = 1 + packet->packet_direction;
			return;
		}

		if (packet->payload[0] == 0x16 && packet->payload[1] == 0x03
			&& (packet->payload[2] == 0x00 || packet->payload[2] == 0x01 || packet->payload[2] == 0x02)
			&& (packet->payload_packet_len - ntohs(get_u16(packet->payload, 3)) == 5)) {
			// SSLv3 Record
			IPQ_LOG(IPOQUE_PROTOCOL_SSL, ipoque_struct, IPQ_LOG_DEBUG, "sslv3 len match\n");
			flow->ssl_stage = 1 + packet->packet_direction;
			return;
		}
	}

	if (packet->payload_packet_len > 40 &&
		flow->ssl_stage == 1 + packet->packet_direction
		&& flow->packet_direction_counter[packet->packet_direction] < 5) {
		return;
	}

	if (packet->payload_packet_len > 40 && flow->ssl_stage == 2 - packet->packet_direction) {
		IPQ_LOG(IPOQUE_PROTOCOL_SSL, ipoque_struct, IPQ_LOG_DEBUG, "second ssl packet\n");
		// SSLv2 Record
		if (packet->payload[2] == 0x01 && packet->payload[3] == 0x03
			&& (packet->payload[4] == 0x00 || packet->payload[4] == 0x01 || packet->payload[4] == 0x02)
			&& (packet->payload_packet_len - 2) >= packet->payload[1]) {
			IPQ_LOG(IPOQUE_PROTOCOL_SSL, ipoque_struct, IPQ_LOG_DEBUG, "sslv2 server len match\n");
			ssl_mark_and_payload_search_for_other_protocols(ipoque_struct);
			return;
		}

		if (1 == ipoque_search_sslv3_direction1(ipoque_struct)) {
			IPQ_LOG(IPOQUE_PROTOCOL_SSL, ipoque_struct, IPQ_LOG_DEBUG, "sslv3 server len match\n");
			ssl_mark_and_payload_search_for_other_protocols(ipoque_struct);
			return;
		}

		if (packet->payload_packet_len > 40 && flow->packet_direction_counter[packet->packet_direction] < 5) {
			IPQ_LOG(IPOQUE_PROTOCOL_SSL, ipoque_struct, IPQ_LOG_DEBUG, "need next packet\n");
			return;
		}
	}

	IPQ_LOG(IPOQUE_PROTOCOL_SSL, ipoque_struct, IPQ_LOG_DEBUG, "exclude ssl\n");
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_SSL);
	return;
}
#endif
